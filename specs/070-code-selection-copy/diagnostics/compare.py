#!/usr/bin/env python3
"""Compare one expected UTF-8 file with correlated trace and paste payloads."""
import argparse
import json
from pathlib import Path


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('expected', type=Path)
    p.add_argument('trace', type=Path)
    p.add_argument('--pid', type=int, required=True)
    p.add_argument('--action', type=int, required=True)
    p.add_argument('--local', type=Path, required=True)
    p.add_argument('--remote', type=Path, required=True)
    a = p.parse_args()
    expected = a.expected.read_bytes()
    expected.decode('utf-8')
    rows = [json.loads(line) for line in a.trace.read_text().splitlines()]
    rows = [r for r in rows if (r['pid'], r['action']) == (a.pid, a.action)]
    kinds = [r['stage'] for r in rows]
    if kinds not in [['partial', 'selectedText', 'writer', 'readback'],
                     ['whole-code', 'selectedText', 'writer', 'readback']]:
        p.error(f'Incomplete or duplicate action stages: {kinds}')
    ordered = [rows[1], rows[0], *rows[2:]] if kinds[0] == 'partial' else [rows[0], *rows[2:]]
    stages = [(r['stage'], bytes.fromhex(r['hex'])) for r in ordered]
    for name, path in [('local', a.local), ('remote', a.remote)]:
        capture = json.loads(path.read_text())
        if capture['action'] != f'{a.pid}:{a.action}':
            p.error(f'{name} capture belongs to a different action')
        stages.append((name, bytes.fromhex(capture['payload']['hex'])))
    for stage, data in stages:
        if data != expected:
            print(f'FAIL: earliest divergent stage: {stage}; expected={expected.hex()} actual={data.hex()}')
            return 1
    print('PASS: every captured payload matches expected bytes; inspect wire framing separately')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
