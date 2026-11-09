from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parent
repo = root.parents[1]
scripts = repo/'specs/073-rendering-architecture/demonstrator/performance-experiment'
sys.path.insert(0, str(scripts))
from recovery import phase_complete
assert phase_complete(root/'diagnostics', 'finished')
build = repo/'build-ci/073-performance-final'
commands = [
    [sys.executable, str(scripts/'run.py'), str(build), str(root/'scale'), '--mode', 'scale'],
    [sys.executable, str(scripts/'retry.py'), str(build), str(repo/'build-ci/073-performance-native'), str(root/'native-retry'), '--cohort', 'post-reboot'],
    [sys.executable, str(scripts/'run.py'), str(build), str(root/'cpu-smoke'), '--mode', 'cpu', '--case', 'document-480-10'],
]
for command in commands:
    print('Running:', command, flush=True)
    subprocess.run(command, check=True, cwd=repo)
with (root/'scale.json').open('w') as output:
    subprocess.run([sys.executable, str(scripts/'scale.py'), str(root/'scale')], check=True, stdout=output, cwd=repo)
