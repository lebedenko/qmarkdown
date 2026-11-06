#!/usr/bin/env python3
"""Run native gates serially only after the matching CPU geometry gate passes."""
import argparse
import json
import os
from pathlib import Path
import sys

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
from run import supervise, hashes


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build', type=Path)
    parser.add_argument('geometry', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    gate = json.loads((args.geometry/'summary.json').read_text())
    if gate['outcome'] != 'geometry-scan-passed' or gate['failures']:
        parser.error('geometry has not passed')
    if any(hashes([Path(p)]).get(p) != value for p,value in gate['inputsAfter'].items()):
        parser.error('geometry inputs changed; rerun CPU gates')
    args.output.mkdir(parents=True, exist_ok=False)
    data = json.loads((HERE.parent/'fixtures.json').read_text())
    minimal = json.loads((HERE/'minimal.json').read_text())
    data['fixtures'] += minimal['fixtures'][:2]
    manifest = args.output/'fixtures.json'
    manifest.write_text(json.dumps(data, ensure_ascii=False, indent=2)+'\n')
    executable = (args.build/'patched/skia-compare').resolve()
    inputs = [executable, manifest, *[p for p in HERE.iterdir() if p.is_file()],
              args.geometry/'summary.json', args.build/'build.json',
              HERE.parent/'run.py', *map(Path,data['fonts'].values())]
    results = []
    for backend in ('opengl','vulkan'):
        for fixture in data['fixtures']:
            for cycle in range(4):
                folder = args.output/f"{backend}-{fixture['id']}-{cycle}"
                command = [str(executable),str(manifest.resolve()),fixture['id'],backend,str(folder.resolve())]
                if cycle == 3:
                    command.append('--motion-replay')
                result = supervise(command,folder,dict(os.environ,SDL_VIDEODRIVER='wayland'),
                                   dict(api=backend,platform='wayland'),inputs)
                records = []
                for line in (folder/'stdout.jsonl').read_text().splitlines():
                    try:
                        records.append(json.loads(line))
                    except ValueError:
                        pass  # Supervisor retains/rejects malformed records.
                names = {r.get('name') for r in records if r.get('stage')=='capture'}
                required = {'stationary','selected','resize','reset','pose','replay'}
                if not required <= names or not all((folder/(n+'.png')).exists() for n in required):
                    result['errors'].append('missing capture')
                copied = next((r.get('source') for r in records if r.get('stage')=='selected-source'),None)
                expected = next((b['text'] for b in fixture['blocks'] if b['text']),'')
                if copied != expected:
                    result['errors'].append('copied source differs')
                if cycle == 3 and (sum(r.get('stage')=='motion-frame' for r in records)!=60 or
                    not any(r.get('operation')=='captured-motion-pose' and r.get('exact') is True for r in records)):
                    result['errors'].append('incomplete motion/replay')
                result['passed'] &= not result['errors']
                result.update(fixture=fixture['id'],cycle=cycle)
                (folder/'result.json').write_text(json.dumps(result,indent=2)+'\n')
                results.append(result)
                print(folder.name,result['passed'],result['errors'],flush=True)
                if not result['passed']:
                    break  # Preserve minimal native failure; no dependent recreation claims.
            if not results[-1]['passed']:
                break
        if not results[-1]['passed']:
            break
    observed = sorted({r['observed'].get('dpr') for r in results if 'dpr' in r['observed']})
    summary = dict(outcome='passed' if all(r['passed'] for r in results) else 'blocked',results=results,
                   observedDprs=observed,unavailableDprs=[d for d in (1,1.25,1.5,2) if d not in observed],
                   ownerVisual='pending',geometrySummary=str(args.geometry/'summary.json'))
    (args.output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    return summary['outcome'] != 'passed'


if __name__ == '__main__':
    raise SystemExit(main())
