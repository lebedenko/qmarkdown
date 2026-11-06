"""Private append-only recovery primitives; original evidence is never modified."""
from contextlib import contextmanager
import fcntl
import json
import os
from pathlib import Path
import shutil
from build import digest

LOCK_FDS = ()


def durable_json(path, value):
    temporary = path.with_suffix(path.suffix + '.pending')
    with temporary.open('w') as stream:
        json.dump(value, stream, indent=2)
        stream.write('\n')
        stream.flush()
        os.fsync(stream.fileno())
    temporary.replace(path)
    fd = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY)
    try:
        os.fsync(fd)
    finally:
        os.close(fd)


@contextmanager
def execution_lock(path):
    global LOCK_FDS
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('a') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        previous = LOCK_FDS
        LOCK_FDS = (lock.fileno(),)
        try:
            yield
        finally:
            # Closing, not explicit unlocking: a live child retains the lock.
            LOCK_FDS = previous


def rows(path):
    if not path.exists():
        return []
    result = []
    lines = path.read_bytes().splitlines(keepends=True)
    for i, line in enumerate(lines):
        try:
            value = json.loads(line)
        except (json.JSONDecodeError, UnicodeDecodeError):
            if i == len(lines)-1 and not line.endswith(b'\n'):
                break
            raise
        if not line.endswith(b'\n'):
            break  # unacknowledged final record
        result.append(value)
    return result


def checkpoint(output, phase, artifacts):
    identities = {}
    for path in artifacts:
        if path.suffix in ('.json', '.jsonl'):
            if path.suffix == '.json':
                json.loads(path.read_text())
            else:
                for line in path.read_text().splitlines():
                    json.loads(line)
        with path.open('rb') as stream:
            os.fsync(stream.fileno())
        identities[str(path.relative_to(output))] = digest(path)
    durable_json(output/(phase+'-checkpoint.json'), identities)


def phase_complete(output, phase):
    path = output/(phase+'-checkpoint.json')
    if not path.exists():
        return False
    for name, expected in json.loads(path.read_text()).items():
        if digest(output/name) != expected:
            raise ValueError('checkpoint hash mismatch: '+name)
    return True


def preserve_interrupted(path, directory):
    directory.mkdir(parents=True, exist_ok=True)
    target = directory/path.name
    attempt = 0
    while target.exists():
        attempt += 1
        target = directory/(path.name+f'.{attempt}')
    path.rename(target)
    return target


def import_prior(prior, output, relocations):
    if output.exists():
        raise ValueError('resume requires a new output directory')
    for name, expected in json.loads((prior/'resume-identities.json').read_text()).items():
        if digest(Path(relocations.get(name, name))) != expected:
            raise ValueError('identity hash mismatch: '+name)
    for path in prior.glob('*-checkpoint.json'):
        phase_complete(prior, path.name.removesuffix('-checkpoint.json'))
    gates = prior/'full-geometry-gates/summary.json'
    if gates.exists():
        summary = json.loads(gates.read_text())
        if summary['outcome'] != 'geometry-scan-passed' or summary['failures']:
            raise ValueError('prior geometry gates did not pass')
        for name, expected in summary['outputHashes'].items():
            if digest(gates.parent/name) != expected:
                raise ValueError('geometry gate hash mismatch: '+name)
        if summary['inputsBefore'] != summary['inputsAfter']:
            raise ValueError('geometry inputs changed during historical gates')
        for name, expected in summary['inputsAfter'].items():
            if digest(Path(relocations.get(name, name))) != expected:
                raise ValueError('geometry input hash mismatch: '+name)
    records = rows(prior/'index.jsonl')
    acknowledged = set()
    for row in records:
        if row.get('raw'):
            raw = prior/row['raw']
            if row.get('rawSha256') and digest(raw) != row['rawSha256']:
                raise ValueError('raw hash mismatch: '+str(raw))
            if row.get('fixtureSha256') and digest(prior/(row['case']+'.json')) != row['fixtureSha256']:
                raise ValueError('fixture hash mismatch: '+row['case'])
            if row['status'] == 'completed':
                data = json.loads(raw.read_text())
                if 'geometry' not in data:
                    raise ValueError('missing geometry: '+str(raw))
            acknowledged.add(row['raw'])
    for filename, variant in [('equivalence.jsonl', None), ('control-equivalence.jsonl', 'upstream')]:
        for row in rows(prior/filename):
            key = (row['case'], variant or row['variant'])
            pair = [r for r in records if (r['case'], r['variant']) == key and r['status'] == 'completed']
            by_label = {r['label']:json.loads((prior/r['raw']).read_text()) for r in pair}
            if set(by_label) != {'instrumented', 'headline'} or not row['equal'] or by_label['instrumented']['geometry'] != by_label['headline']['geometry']:
                raise ValueError('invalid comparison: '+str(key))
    shutil.copytree(prior, output, symlinks=False)
    # Keep the original index byte-for-byte, including an interrupted last line.
    shutil.copy2(output/'index.jsonl', output/'inherited-index.jsonl')
    with (output/'index.jsonl').open('w') as stream:
        for row in records:
            stream.write(json.dumps(row)+'\n')
    for filename in ('equivalence.jsonl', 'control-equivalence.jsonl'):
        path = output/filename
        if path.exists():
            shutil.copy2(path, path.with_suffix('.inherited.jsonl'))
            path.write_text(''.join(json.dumps(r)+'\n' for r in rows(path)))
    interrupted = output/'interrupted'
    for path in output.glob('*.jsonl'):
        if path.name.endswith(('-instrumented.jsonl', '-headline.jsonl')) and path.name not in acknowledged:
            preserve_interrupted(path, interrupted)
            stderr = path.with_suffix('.stderr')
            if stderr.exists():
                preserve_interrupted(stderr, interrupted)
    durable_json(output/'import.json', dict(prior=str(prior.resolve()),
        files={str(p.relative_to(prior)):digest(p) for p in prior.rglob('*') if p.is_file()},
        relocations=relocations))
