#!/usr/bin/env python3
"""Completed-run recovery with fatal launch sentinels."""
from pathlib import Path
import sys
from unittest.mock import patch
import run


def check(prior, output):
    with run.recovery.execution_lock(run.ROOT / 'build-ci/073-performance-recovery/execution.lock'):
        with patch.object(run.supervisor, 'supervise', side_effect=AssertionError('forbidden launch')), \
             patch.object(run.supervisor.subprocess, 'Popen', side_effect=AssertionError('forbidden Popen')):
            records = run.resume(prior, output)
            run.require(len(records) == 82, 'completed run required')
            run.execute(output, records)
    print('Completed-run resume: 82 validated records, zero launches; fatal supervisor/Popen sentinels intact.')


if __name__ == '__main__':
    check(Path(sys.argv[1]).resolve(), Path(sys.argv[2]).resolve())
