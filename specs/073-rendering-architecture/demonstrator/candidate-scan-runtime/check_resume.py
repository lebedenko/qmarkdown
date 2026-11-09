#!/usr/bin/env python3
"""Completed-run resume must not execute any child, including historical gates."""
import argparse
import json
from pathlib import Path
from unittest.mock import patch
import run as runner


def check(workspace, prior, output):
    def fatal(*args, **kwargs):
        raise AssertionError('completed resume attempted a process launch')
    with runner.recovery.execution_lock(runner.ROOT/'build-ci/073-performance-recovery/execution.lock'):
        with patch.object(runner.supervisor,'supervise',side_effect=fatal), \
             patch.object(runner.subprocess,'run',side_effect=fatal), \
             patch.object(runner.subprocess,'Popen',side_effect=fatal), \
             patch.object(runner.subprocess,'check_output',side_effect=fatal):
            records=runner.resume(prior,output)
            assert len(records)==1848
            runner.execute(workspace,output,records)
    for name in ('index.jsonl','build-commands.jsonl','execution-commands.jsonl','gate-commands.jsonl'):
        assert (prior/name).read_bytes()==(output/name).read_bytes(),name
    return dict(records=len(records),newProcessLaunches=0,journalsIdentical=True)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('workspace',type=Path);p.add_argument('prior',type=Path);p.add_argument('output',type=Path);a=p.parse_args()
    print(json.dumps(check(a.workspace.resolve(),a.prior.resolve(),a.output.resolve()),indent=2))
