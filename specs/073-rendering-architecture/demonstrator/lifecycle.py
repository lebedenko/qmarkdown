#!/usr/bin/env python3
"""Serial empty-window variants and bounded debugger evidence on exact SDKs."""
import argparse
import json
import os
from pathlib import Path
import signal
import subprocess
from run import HERE, supervise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    results = []
    for sdk, version in [('minimum', '6.8.0'), ('development', '6.12.0')]:
        build = Path('build-ci', '073-demo-'+sdk).resolve()
        for loop in ['default', 'basic']:
            env = dict(os.environ, QT_QPA_PLATFORM='wayland', QSG_RHI_BACKEND='vulkan', QT_SCALE_FACTOR='0.8')
            env.pop('QT_QUICK_BACKEND', None)
            env.pop('QSG_RENDER_LOOP', None)
            if loop == 'basic':
                env['QSG_RENDER_LOOP'] = 'basic'
            for variant in ['existing-empty', 'standalone', 'release-live']:
                exe = build/('qt-compare' if variant == 'existing-empty' else 'qt-lifecycle')
                for cycle in range(3):
                    folder = args.output/f'{sdk}-{loop}-{variant}-{cycle}'
                    command = [str(exe)]
                    if variant == 'existing-empty':
                        command += [str(HERE/'fixtures.json'), 'bidi-objects', 'distance', str(folder.resolve()), '--case=empty']
                    elif variant == 'release-live':
                        command += ['--release-live']
                    result = supervise(command, folder, env,
                        dict(qt=version, api=5, platform='wayland', dpr=1),
                        [exe, HERE/'lifecycle.cpp', HERE/'qt.cpp', HERE/'fixtures.json'])
                    result.update(sdk=sdk, loop=loop, variant=variant, cycle=cycle)
                    results.append(result)
                    print(sdk, loop, variant, cycle, result['passed'], flush=True)
                    if cycle == 0:
                        debug = ['gdb', '-batch', '-ex', 'set pagination off', '-ex', 'run',
                                 '-ex', 'thread apply all bt 20', '--args', *command]
                        with (folder/'backtrace.log').open('wb') as log:
                            process = subprocess.Popen(debug, env=env, stdout=log, stderr=subprocess.STDOUT, start_new_session=True)
                            try:
                                process.wait(timeout=30)
                            except subprocess.TimeoutExpired:
                                os.killpg(process.pid, signal.SIGKILL)
                                process.wait()
                                log.write(b'\nDebugger deadline exceeded\n')
    (args.output/'summary.json').write_text(json.dumps(results, indent=2)+'\n')
    return 0 if all(r['passed'] for r in results) else 1


if __name__ == '__main__':
    raise SystemExit(main())
