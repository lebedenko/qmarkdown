#!/usr/bin/env python3
"""Reproduce the bounded geometry scan or serial native correctness matrix."""
import argparse
import gzip
import json
import os
from pathlib import Path
import subprocess
from run import HERE, hashes, supervise


def geometry(output):
    data = json.loads((HERE/'fixtures.json').read_text())
    scan = dict(data, fixtures=[])
    for fixture in data['fixtures']:
        widths = (range(60, 241) if fixture['id'] in
                  ('wrapping', 'adjacent', 'bidi-objects', 'unicode-code') else
                  (fixture['width']-1, fixture['width'], fixture['width']+1))
        scan['fixtures'].extend(dict(fixture, id=fixture['id']+'@'+str(w), width=w) for w in widths)
    manifest = output/'geometry-fixtures.json'
    manifest.write_text(json.dumps(scan, ensure_ascii=False, indent=2)+'\n')
    results = []
    for name in ('qt-development', 'qt-minimum', 'skia', 'skia-trial'):
        exe = Path('build-ci', '073-demo-'+name[3:], 'qt-selection') if name.startswith('qt-') else Path('build-ci/073-skia-demo/skia-selection')
        command = [str(exe.resolve()), str(manifest.resolve())]
        if name == 'skia-trial':
            command.append('--fragment-trial')
        inputs = [exe, manifest, *map(Path, data['fonts'].values()), HERE/'prepared-cursor.patch',
                  *exe.parent.rglob('libQMarkdown.so*')]
        before = hashes(inputs)
        process = subprocess.run(command, env=dict(os.environ, QT_QPA_PLATFORM='offscreen'), capture_output=True, timeout=180)
        with gzip.open(output/(name+'-geometry.jsonl.gz'), 'wb') as stream:
            stream.write(process.stdout)
        (output/(name+'-geometry.stderr')).write_bytes(process.stderr)
        records = [json.loads(line) for line in process.stdout.splitlines()]
        failures = [r for r in records if r.get('passed') is False or r.get('status') in ('cycle', 'non-convergent')]
        (output/(name+'-failures.json')).write_text(json.dumps(failures, indent=2)+'\n')
        results.append(dict(name=name, command=command, exit=process.returncode, failures=len(failures),
                            passed=process.returncode == 0 and not failures and before == hashes(inputs),
                            inputsBefore=before, inputsAfter=hashes(inputs)))
        print(name, process.returncode, len(failures), flush=True)
    return results


def native(output):
    manifest = HERE/'fixtures.json'
    data = json.loads(manifest.read_text())
    results = []
    # Full matrix once; affected composition fixtures get three recreations and
    # a separate rendered-motion pass. Failed Skia composition remains diagnostic.
    affected = {'padded', 'adjacent', 'wrapping', 'bidi-objects', 'unicode-code', 'objects', 'document'}
    for sdk, version in [('development', '6.12.0'), ('minimum', '6.8.0'), ('skia', None)]:
        exe = Path('build-ci', '073-skia-demo' if sdk == 'skia' else '073-demo-'+sdk,
                   'skia-compare' if sdk == 'skia' else 'qt-compare').resolve()
        sources = [p for p in HERE.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
        inputs = [exe, manifest, *sources, *exe.parent.rglob('libQMarkdown.so*'),
                  Path('build-ci/073-correctness-prepared/demo-source.json'), *map(Path, data['fonts'].values())]
        for backend in ('opengl', 'vulkan'):
            for renderer in (('skia',) if sdk == 'skia' else ('distance', 'curve')):
                for dpr in ((1,) if sdk == 'skia' else (1, 1.25, 1.5, 2)):
                    for fixture in data['fixtures']:
                        for cycle in range(4 if fixture['id'] in affected else 1):
                            motion = cycle == 3
                            folder = output/f"{sdk}-{backend}-{renderer}-{dpr}-{fixture['id']}-{cycle}"
                            command = [str(exe), str(manifest), fixture['id'], backend if sdk == 'skia' else renderer, str(folder.resolve())]
                            if motion:
                                command.append('--motion-replay')
                            env = dict(os.environ)
                            env.pop('QSG_RENDER_LOOP', None)
                            env.pop('QT_QUICK_BACKEND', None)
                            expected = dict(platform='wayland', dpr=dpr)
                            if sdk == 'skia':
                                env['SDL_VIDEODRIVER'] = 'wayland'
                                expected['api'] = backend
                            else:
                                env.update(QT_QPA_PLATFORM='wayland', QSG_RHI_BACKEND=backend, QT_SCALE_FACTOR=str(dpr/1.25))
                                expected.update(qt=version, api={'opengl': 3, 'vulkan': 5}[backend])
                            result = supervise(command, folder, env, expected, inputs)
                            rows = []
                            for line in (folder/'stdout.jsonl').read_text(errors='replace').splitlines():
                                try:
                                    row = json.loads(line)
                                    if isinstance(row, dict):
                                        rows.append(row)
                                except ValueError:
                                    pass  # supervise retains and rejects malformed output.
                            names = {r.get('name') for r in rows if r.get('stage') == 'capture'}
                            required = {'stationary', 'selected', 'resize', 'reset', 'pose', 'replay'}
                            complete = required <= names and all((folder/(name+'.png')).is_file() for name in required)
                            if not complete:
                                result['errors'].append('missing required captures')
                            if complete and (folder/'selected.png').read_bytes() == (folder/'reset.png').read_bytes():
                                result['errors'].append('selection capture unchanged')
                            copied = next((r['source'] for r in rows if r.get('stage') == 'selected-source'), None)
                            expected_source = next((b['text'] for b in fixture['blocks'] if b['text']), '')
                            if copied != expected_source:
                                result['errors'].append('selected source mismatch')
                            if motion and (sum(r.get('stage') == 'motion-frame' for r in rows) != 60 or
                                           not any(r.get('operation') == 'captured-motion-pose' and r.get('exact') is True for r in rows)):
                                result['errors'].append('incomplete motion/replay')
                            result['passed'] &= not result['errors']
                            result.update(sdk=sdk, fixture=fixture['id'], cycle=cycle, motion=motion)
                            (folder/'result.json').write_text(json.dumps(result, indent=2)+'\n')
                            results.append(result)
                            print(folder.name, result['passed'], flush=True)
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mode', choices=['geometry', 'native'])
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    results = (geometry if args.mode == 'geometry' else native)(args.output)
    (args.output/'summary.json').write_text(json.dumps(results, indent=2)+'\n')
    return 0 if all(r.get('passed', r['exit'] == 0) for r in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
