#!/usr/bin/env python3
"""Upload missing assets only after comparing every existing asset byte-for-byte."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess


def gh(*args):
    return subprocess.check_output(['gh', *map(str, args)])


def select_assets(directory):
    local = []
    for suffix in ('.tar.gz', '.pkg.tar.zst'):
        archives = sorted(directory.glob('*' + suffix))
        checksums = sorted(directory.glob('*' + suffix + '.sha256'))
        if len(archives) != 1 or len(checksums) != 1:
            raise ValueError(f'Expected one {suffix} archive and its checksum')
        archive = archives[0]
        checksum = checksums[0]
        fields = checksum.read_text().split()
        if (checksum.name != archive.name + '.sha256' or len(fields) != 2
                or fields[1] != archive.name
                or fields[0] != hashlib.sha256(archive.read_bytes()).hexdigest()):
            raise ValueError(f'Invalid checksum: {checksum.name}')
        local.extend((archive, checksum))
    return local


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--tag', required=True)
    parser.add_argument('--assets', type=Path, required=True)
    args = parser.parse_args()
    try:
        local = select_assets(args.assets)
    except ValueError as error:
        parser.error(str(error))
    release = json.loads(gh('api', f'repos/{args.repository}/releases/tags/{args.tag}'))
    if release['draft'] or release['prerelease']:
        parser.error('Publication requires a stable published release')
    existing = {asset['name']: asset for asset in release['assets']}
    missing = []
    for path in local:
        if path.name not in existing:
            missing.append(path)
            continue
        data = gh('api', '-H', 'Accept: application/octet-stream',
                  f'repos/{args.repository}/releases/assets/{existing[path.name]["id"]}')
        if data != path.read_bytes():
            raise SystemExit(f'Refusing to overwrite differing published asset: {path.name}')
        print(f'Identical published asset: {path.name}')
    if missing:
        subprocess.run(['gh', 'release', 'upload', args.tag, '--repo', args.repository,
                        *map(str, missing)], check=True)


if __name__ == '__main__':
    main()
