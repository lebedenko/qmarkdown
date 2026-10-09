#!/usr/bin/env python3
"""Upload missing assets only after comparing every existing asset byte-for-byte."""
import argparse
import json
from pathlib import Path
import subprocess


def gh(*args):
    return subprocess.check_output(['gh', *map(str, args)])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--tag', required=True)
    parser.add_argument('--assets', type=Path, required=True)
    args = parser.parse_args()
    release = json.loads(gh('api', f'repos/{args.repository}/releases/tags/{args.tag}'))
    if release['draft'] or release['prerelease']:
        parser.error('Publication requires a stable published release')
    local = sorted(args.assets.glob('*.tar.gz*'))
    if len(local) != 2 or not any(p.name.endswith('.tar.gz.sha256') for p in local):
        parser.error('Expected one archive and its checksum')
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
