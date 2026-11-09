#!/usr/bin/env python3
"""Bounded native placement gate; no visual/performance acceptance is inferred."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('executable', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--qt', required=True)
    parser.add_argument('--timeout', type=float, default=20)
    parser.add_argument('--compositor-scale', type=float, required=True)
    parser.add_argument('--font', type=Path,
                        default=Path('/usr/share/fonts/TTF/DejaVuSans.ttf'))
    args = parser.parse_args()
    exe = args.executable.resolve()
    args.output.mkdir(parents=True, exist_ok=False)
    results = []
    for backend in ('opengl', 'vulkan'):
        for renderer in ('distance', 'curve'):
            for dpr in (1, 1.25, 1.5, 2):
                folder = args.output / f'{backend}-{renderer}-{dpr}'
                folder.mkdir()
                env = dict(os.environ, QT_QPA_PLATFORM='wayland', QSG_RHI_BACKEND=backend,
                           QT_SCALE_FACTOR=str(dpr / args.compositor_scale), QSG_INFO='1',
                           QT_LOGGING_RULES='qt.scenegraph.general=true;qt.rhi.*=true')
                env.pop('QT_QUICK_BACKEND', None)
                command = [str(exe), renderer, str(folder.resolve()), str(args.font.resolve())]
                started = time.monotonic()
                with (folder / 'stdout.jsonl').open('w') as out, (folder / 'stderr.log').open('w') as err:
                    try:
                        result = subprocess.run(command, env=env, stdout=out, stderr=err,
                                                timeout=args.timeout, check=False)
                        code = result.returncode
                    except subprocess.TimeoutExpired:
                        code = 'timeout'
                records = [json.loads(line) for line in (folder / 'stdout.jsonl').read_text().splitlines()]
                identity = next((r for r in records if r['stage'] == 'capture-before'), {})
                layout = next((r for r in records if r['stage'] == 'layout'), {})
                gate = next((r for r in records if r['stage'] == 'placement-result'), {})
                passed = (code == 0 and layout.get('qt') == args.qt
                          and identity.get('api') == {'opengl': 3, 'vulkan': 5}[backend]
                          and identity.get('platform') == 'wayland'
                          and identity.get('dpr') == dpr and gate.get('adapterBlocked') is True)
                results.append(dict(backend=backend, renderer=renderer, requestedDpr=dpr,
                                    observed=identity, exit=code, seconds=time.monotonic() - started,
                                    lastStage=records[-1]['stage'] if records else 'no-output',
                                    placementGateVerified=passed))
                print(json.dumps(results[-1]), flush=True)
    files = [exe, args.font, *Path(__file__).parent.glob('*')]
    summary = dict(schema='qmarkdown-architecture-placement-v1',
                   identities={str(p): hashlib.sha256(p.read_bytes()).hexdigest()
                               for p in files if p.is_file()}, results=results,
                   architectureQualified=False)
    (args.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    return 0 if all(r['placementGateVerified'] for r in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
