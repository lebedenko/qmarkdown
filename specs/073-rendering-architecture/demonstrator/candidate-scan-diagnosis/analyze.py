#!/usr/bin/env python3
"""Read-only diagnosis of the closed candidate study; never launches study binaries."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
from statistics import median
import subprocess
import tarfile

HERE = Path(__file__).resolve().parent
FEATURE = HERE.parents[1]
ROOT = FEATURE.parents[1]


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def write(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + '\n')


def check_hash(path, expected):
    require(path.is_file(), f'missing evidence: {path}')
    require(digest(path) == expected, f'hash mismatch: {path}')


def verify_archives(folder, destination):
    manifest = json.loads((folder / 'archives.json').read_text())
    members = {}
    for name, record in manifest['archives'].items():
        check_hash(folder / name, record['sha256'])
        require((folder / name).stat().st_size == record['bytes'], f'archive size: {name}')
        actual = {}
        with tarfile.open(folder / name) as archive:
            for member in archive:
                relative = Path(member.name)
                require(member.isfile() and not relative.is_absolute() and '..' not in relative.parts,
                        f'unsafe archive member: {member.name}')
                require(member.name not in members and member.name not in actual,
                        f'duplicate archive member: {member.name}')
                data = archive.extractfile(member).read()
                actual[member.name] = hashlib.sha256(data).hexdigest()
                require(actual[member.name] == record['files'].get(member.name),
                        f'member hash mismatch: {member.name}')
                target = destination / member.name
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
        require(actual == record['files'], f'archive member set: {name}')
        members.update(actual)
    return members


def verify_identities(identities):
    for name, expected in identities.items():
        check_hash(Path(name), expected)


def verify_records(root, schedule, records):
    require(len(records) == len(schedule), 'missing scheduled records')
    require(len({r['id'] for r in records}) == len(records), 'duplicate records')
    for entry, row in zip(schedule, records):
        require(all(row.get(k) == v for k, v in entry.items()), 'schedule identity mismatch')
        require(row['status'] == 'completed', 'incomplete record')
        check_hash(root / row['raw'], row['rawSha256'])
        check_hash(root / (row['case'] + '.json'), row['fixtureSha256'])
    for first, second in zip(records[::2], records[1::2]):
        require(first['pair'] == second['pair'] and first['order'] == 0 and second['order'] == 1
                and {first['label'], second['label']} == {'baseline', 'retained'}, 'invalid adjacent pair')
        expected = 'baseline' if first['sample'] % 2 == 0 else 'retained'
        require(first['label'] == expected, 'nonalternating pair')


def describe(values):
    center = median(values)
    return dict(median=center, mad=median([abs(x - center) for x in values]),
                minimum=min(values), maximum=max(values),
                positive=sum(x > 0 for x in values), negative=sum(x < 0 for x in values),
                zero=values.count(0))


def pair_value(a, b):
    require(a['applicableParagraphs'] == b['applicableParagraphs'], 'paragraph mismatch')
    n = a['applicableParagraphs']
    if a['elapsedNs'] is None or b['elapsedNs'] is None:
        require(a['elapsedNs'] is None and b['elapsedNs'] is None and n == 0, 'invalid null operation')
        return None, None
    require(n > 0, 'invalid paragraph count')
    delta = b['elapsedNs'] - a['elapsedNs']
    return delta, delta / n


def cell_analysis(cell, pairs):
    result = dict(case=cell['case'], variant=cell['variant'], operation=cell['operation'],
                  original=cell, pairs=pairs)
    require([p['sample'] for p in pairs] == list(range(20)), 'missing/duplicate cell pair')
    if not cell['applicable']:
        require(all(p['differenceNs'] is None for p in pairs), 'nonnull inapplicable cell')
        result['descriptive'] = None
        return result
    stats = {key: describe([p[key] for p in pairs])
             for key in ('differenceNs', 'overheadPerParagraphNs')}
    stats['orderMedians'] = {order: median([p['overheadPerParagraphNs'] for p in pairs
                                          if p['armOrder'][0] == order])
                             for order in ('baseline', 'retained')}
    stats['orderArmMedians'] = {order: {arm: median([p[arm]['elapsedNs'] for p in pairs
        if p['armOrder'][0] == order]) for arm in ('baseline', 'retained')}
        for order in ('baseline', 'retained')}
    stats['halves'] = {}
    for name, rows in [('firstTen', pairs[:10]), ('lastTen', pairs[10:])]:
        stats['halves'][name] = dict(
            baselineMedianNs=median([p['baseline']['elapsedNs'] for p in rows]),
            retainedMedianNs=median([p['retained']['elapsedNs'] for p in rows]),
            overheadPerParagraphNs=median([p['overheadPerParagraphNs'] for p in rows]))
    result['descriptive'] = stats
    if cell['gate'] == 'regression-fail':
        result['upperBoundExcessNs'] = cell['intervals95']['overheadPerParagraphNs'][1] - cell['overheadLimitPerParagraphNs']
    return result


def matrix(root, records, summary):
    timing = {(r['case'], r['variant'], r['sample'], r['label']): r
              for r in records if r['mode'] == 'timing'}
    require(len(timing) == 1760, 'timing matrix incomplete')
    cells = []
    for cell in summary['cells']:
        pairs = []
        for sample in range(20):
            pair = dict(sample=sample, timerLimited=cell['timerLimited'])
            rows = [timing[cell['case'], cell['variant'], sample, arm] for arm in ('baseline', 'retained')]
            pair['armOrder'] = [r['label'] for r in sorted(rows, key=lambda r: r['order'])]
            for row in rows:
                raw = [json.loads(line) for line in (root / row['raw']).read_text().splitlines()]
                require([r['phase'] for r in raw] == ['initial', 'reflow', 'repeat', 'metadata'], 'operation order')
                phase = next(r for r in raw if r['phase'] == cell['operation'])
                pair[row['label']] = dict(**phase, wallSeconds=row['wallSeconds'],
                    sampledRssBytes=row['peakObservedRssBytes'], raw=row['raw'], rawSha256=row['rawSha256'])
            pair['differenceNs'], pair['overheadPerParagraphNs'] = pair_value(pair['baseline'], pair['retained'])
            pairs.append(pair)
        cells.append(cell_analysis(cell, pairs))
    require(len(cells) == 176, 'cell matrix incomplete')
    return cells


def load_report():
    spec = importlib.util.spec_from_file_location('historical_report', HERE.parent / 'candidate-scan-continuation/report.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def compare_summary(actual, expected):
    require(actual == expected, 'archived summary disagreement')
    require(sum(c['gate'] == 'regression-fail' for c in actual['cells']) == 8, 'expected eight failures')


def inspect_binaries(workspace, output):
    commands = []
    for tool in ('nm', 'objdump'):
        command = [tool, '--version']
        value = subprocess.run(command, check=True, capture_output=True, text=True).stdout
        (output / (tool + '-version.txt')).write_text(value)
        commands.append(command)
    for arm in ('baseline', 'retained'):
        binary = workspace / (arm + '-build/runtime-trace')
        command = ['nm', '-S', '-C', '--defined-only', str(binary)]
        symbols = subprocess.run(command, check=True, capture_output=True, text=True).stdout
        selected = '\n'.join(line for line in symbols.splitlines() if any(s in line for s in
            ('TextWrapper::breakTextIntoLines', 'ParagraphImpl::experimentalPadded',
             'ParagraphImpl::layout(', 'ParagraphImpl::experimentalSetSpans', 'ParagraphImpl::breakShapedTextIntoLines')))
        (output / (arm + '-symbols.txt')).write_text(selected + '\n')
        commands.append(command)
        # Address ranges come only from nm, avoiding dependence on mangled spellings.
        for i, line in enumerate(selected.splitlines()):
            address, size, *_ = line.split()
            command = ['objdump', '-d', '-C', '--start-address=0x' + address,
                       '--stop-address=' + hex(int(address, 16) + int(size, 16)), str(binary)]
            value = subprocess.run(command, check=True, capture_output=True, text=True).stdout
            (output / f'{arm}-{i:02}.asm').write_text(value)
            commands.append(command)
    write(output / 'commands.json', commands)


def run(output):
    require(not output.exists(), 'output must be a new directory')
    output.mkdir(parents=True)
    historical = FEATURE / 'evidence/candidate-scan-continuation'
    inputs = output / 'inputs'
    inputs.mkdir()
    members = verify_archives(historical, inputs)
    for name, expected in members.items():
        check_hash(ROOT / 'build-ci/073-candidate-continuation-evidence' / name, expected)
    identities = json.loads((inputs / 'identities.json').read_text())
    verify_identities(identities)
    verify_identities(json.loads((inputs / 'supplemental-identities.json').read_text()))
    schedule = json.loads((inputs / 'schedule.json').read_text())
    records = [json.loads(line) for line in (inputs / 'index.jsonl').read_text().splitlines()]
    require(len(schedule) == 1848, 'schedule size')
    verify_records(inputs, schedule, records)
    # Compare the persisted schedule with the unchanged generating implementation.
    spec = importlib.util.spec_from_file_location('historical_runner', HERE.parent / 'candidate-scan-continuation/run.py')
    runner = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(runner)
    require(schedule == runner.schedule(), 'generated schedule mismatch')
    for name in ('summary.json', 'report.md'):
        require((inputs / name).read_bytes() == (historical / name).read_bytes(), f'published {name} mismatch')
    report_source = HERE.parent / 'candidate-scan-continuation/report.py'
    require(report_source.read_bytes() == (inputs / 'reproduction/candidate-scan-continuation/report.py').read_bytes(), 'report implementation mismatch')
    expected = json.loads((inputs / 'summary.json').read_text())
    report_root = output / 'recomputed'
    shutil.copytree(inputs, report_root)
    actual = load_report().report(report_root)
    compare_summary(actual, expected)
    for name in ('summary.json', 'report.md'):
        require((report_root / name).read_bytes() == (inputs / name).read_bytes(), f'nonidentical recomputed {name}')
    cells = matrix(inputs, records, actual)
    write(output / 'cells.json', cells)
    # A second independent construction checks deterministic serialization without resampling again.
    require(json.dumps(cells, sort_keys=True) == json.dumps(matrix(inputs, records, actual), sort_keys=True), 'nondeterministic analysis')
    binary_output = output / 'binary'
    binary_output.mkdir()
    inspect_binaries(ROOT / 'build-ci/073-candidate-continuation', binary_output)
    write(output / 'integrity.json', dict(archiveIndexSha256=digest(historical / 'archives.json'),
          members=members, identities=identities, records=len(records), cells=len(cells),
          summaryExact=True, reportExact=True, deterministicMatrix=True))
    preserve_and_present(output)
    print('Verified archives, identities, schedule, raw hashes and exact report; wrote 176 cells / 3520 pairs.')


def preserve_and_present(output):
    from present import render
    integrity = json.loads((output / 'integrity.json').read_text())
    historical = ROOT / 'build-ci/073-candidate-continuation-evidence'
    for name, expected in integrity['members'].items():
        check_hash(historical / name, expected)
    verify_identities(integrity['identities'])
    sources = output / 'sources'
    for arm in ('baseline', 'retained'):
        folder = ROOT / 'build-ci/073-candidate-continuation' / arm / 'modules/skparagraph/src'
        for name in ('TextWrapper.cpp', 'ParagraphImpl.cpp', 'ParagraphImpl.h'):
            target = sources / arm / name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(folder / name, target)
    shutil.copyfile(ROOT / 'build-ci/073-candidate-continuation/retained/modules/skparagraph/src/ScanBound.h',
                    sources / 'retained/ScanBound.h')
    require((sources / 'baseline/ParagraphImpl.cpp').read_bytes() ==
            (sources / 'retained/ParagraphImpl.cpp').read_bytes(), 'ParagraphImpl.cpp differs')
    require((sources / 'baseline/ParagraphImpl.h').read_bytes() ==
            (sources / 'retained/ParagraphImpl.h').read_bytes(), 'ParagraphImpl.h differs')
    for name in ('candidate-scan-continuation/trace.cpp', 'boundary-experiment/layout.h'):
        target = sources / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(HERE.parent / name, target)
    render(output)
    first = {name: digest(output / name) for name in ('report.md', 'failed-pairs.md', 'context.md')}
    render(output)
    require(first == {name: digest(output / name) for name in first}, 'nondeterministic presentation')
    write(output / 'preservation.json', dict(historicalMembersUnchanged=len(integrity['members']),
          identitiesUnchanged=len(integrity['identities']), deterministicPresentation=True))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('output', type=Path)
    run(parser.parse_args().output.resolve())
