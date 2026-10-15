#!/usr/bin/env python3
"""Upload missing assets, refusing any differing bytes, and verify downloads."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time


def gh(*args):
    return subprocess.check_output(['gh', *map(str, args)])


def select_assets(directory, version=None):
    local = []
    for suffix in ('.tar.gz', '.pkg.tar.zst'):
        archives = sorted(directory.glob('*' + suffix))
        checksums = sorted(directory.glob('*' + suffix + '.sha256'))
        if len(archives) != 1 or len(checksums) != 1:
            raise ValueError(f'Expected one {suffix} archive and its checksum')
        archive, checksum = archives[0], checksums[0]
        fields = checksum.read_text().split()
        if (checksum.name != archive.name + '.sha256' or len(fields) != 2
                or fields[1] != archive.name
                or fields[0] != hashlib.sha256(archive.read_bytes()).hexdigest()):
            raise ValueError(f'Invalid checksum: {checksum.name}')
        if version:
            pattern = (re.escape(f'qmarkdown-{version}.tar.gz') if suffix == '.tar.gz' else
                       rf'qmarkdown-{re.escape(version)}-[1-9]\d*-x86_64\.pkg\.tar\.zst')
            if not re.fullmatch(pattern, archive.name):
                raise ValueError('Artifact name does not match release version')
        local.extend((archive, checksum))
    if set(path.name for path in directory.iterdir()) != {path.name for path in local}:
        raise ValueError('Only source/Arch deliverables and checksums may be uploaded')
    return local


def release_assets(repository, tag):
    # Tag lookup does not reliably expose drafts; listing includes them.
    pages = json.loads(gh('api', '--paginate', '--slurp',
                          f'repos/{repository}/releases?per_page=100'))
    matching = [release for page in pages for release in page if release['tag_name'] == tag]
    if len(matching) != 1:
        raise ValueError(f'Expected exactly one release for {tag}; found {len(matching)}')
    release = json.loads(gh('api', f'repos/{repository}/releases/{matching[0]["id"]}'))
    if release['id'] != matching[0]['id'] or release['tag_name'] != tag:
        raise ValueError('Release identity changed during lookup')
    if release['prerelease']:
        raise ValueError('Prerelease publication is not supported')
    return release


def compare_existing(repository, release, local, require_all=False):
    existing = {asset['name']: asset for asset in release['assets']}
    if set(existing) - {path.name for path in local}:
        raise ValueError('Release contains unexpected assets; explicit repair required')
    missing = []
    for path in local:
        if path.name not in existing:
            if require_all:
                raise ValueError(f'Missing published asset: {path.name}')
            missing.append(path)
            continue
        data = gh('api', '-H', 'Accept: application/octet-stream',
                  f'repos/{repository}/releases/assets/{existing[path.name]["id"]}')
        if hashlib.sha256(data).digest() != hashlib.sha256(path.read_bytes()).digest():
            raise ValueError(f'Refusing differing downloaded asset (SHA-256 mismatch): {path.name}')
    return missing


def upload(repository, tag, local, attempts=3):
    for attempt in range(attempts):
        # Refresh after partial upload failure; never clobber even on retries.
        release = release_assets(repository, tag)
        missing = compare_existing(repository, release, local)
        if not missing:
            return
        if not release['draft']:
            raise ValueError('Incomplete published release requires explicit repair')
        try:
            subprocess.run(['gh', 'release', 'upload', tag, '--repo', repository,
                            *map(str, missing)], check=True)
            compare_existing(repository, release_assets(repository, tag), local, require_all=True)
            return
        except subprocess.CalledProcessError:
            if attempt + 1 == attempts:
                raise
            time.sleep(2)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--tag', required=True)
    parser.add_argument('--assets', type=Path, required=True)
    args = parser.parse_args()
    upload(args.repository, args.tag, select_assets(args.assets, args.tag.removeprefix('v')))


if __name__ == '__main__':
    main()
