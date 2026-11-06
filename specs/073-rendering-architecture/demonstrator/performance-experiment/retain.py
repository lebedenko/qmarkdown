#!/usr/bin/env python3
"""Retain compact raw archives and hashes without overwriting earlier evidence."""
import argparse
import hashlib
from datetime import datetime, timezone
import json
from pathlib import Path
import platform
import subprocess
import tarfile
from build import digest


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('output', type=Path)
    p.add_argument('inputs', type=Path, nargs='+')
    p.add_argument('--part-bytes', type=int, default=64*1024**2, help='uncompressed bytes per archive part')
    p.add_argument('--environment-cohort', default='archive-time', help='When archive environment facts were observed')
    args = p.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    archives = {}
    for source in args.inputs:
        files = sorted(f for f in source.rglob('*') if f.is_file()
                       and not any(part in ('build', 'control-build', 'gates') for part in f.relative_to(source).parts))
        groups, current, size = [], [], 0
        for f in files:
            if current and size+f.stat().st_size > args.part_bytes:
                groups.append(current)
                current, size = [], 0
            current.append(f)
            size += f.stat().st_size
        if current:
            groups.append(current)
        for index, group in enumerate(groups, 1):
            suffix = f'-part-{index:03}' if len(groups) > 1 else ''
            target = args.output/(source.name+suffix+'.tar.gz')
            assert not target.exists(), 'refusing to overwrite '+str(target)
            with tarfile.open(target, 'w:gz', compresslevel=6, dereference=True) as archive:
                for f in group:
                    archive.add(f, arcname=str(f.relative_to(source)))
            with tarfile.open(target, 'r:gz') as archive:
                for member in archive.getmembers():
                    assert member.isfile(), member.name
                    assert hashlib.sha256(archive.extractfile(member).read()).hexdigest() == digest(source/member.name), member.name
            archives[target.name] = dict(sourceDirectory=source.name, sha256=digest(target), bytes=target.stat().st_size,
                files={str(f.relative_to(source)):digest(f) for f in group})
    info = dict(environmentCohort=args.environment_cohort, recordedAtUtc=datetime.now(timezone.utc).isoformat(), kernel=platform.platform(),
        cpu=subprocess.check_output(['lscpu'], text=True), archives=archives)
    driver = Path('/proc/driver/nvidia/version')
    info['nvidiaDriver'] = driver.read_text() if driver.exists() else None
    info['cpuGovernors'] = {str(f):f.read_text().strip() for f in Path('/sys/devices/system/cpu').glob('cpu*/cpufreq/scaling_governor')}
    (args.output/'archives.json').write_text(json.dumps(info, indent=2)+'\n')


if __name__ == '__main__':
    main()
