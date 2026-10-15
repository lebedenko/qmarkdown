#!/usr/bin/env python3
"""Upload missing assets, refusing any differing bytes, and verify downloads."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time
from urllib.parse import urlencode


def gh(*args):
    return subprocess.check_output(['gh', *map(str, args)], stderr=subprocess.PIPE)


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


class PendingState(ValueError):
    """The expected remote transition is not visible yet."""


class MissingAssets(PendingState):
    pass


def transient(error, visibility=False):
    if isinstance(error, subprocess.TimeoutExpired):
        return True
    message = error.stderr or b''
    if isinstance(message, bytes):
        message = message.decode(errors='replace')
    status = re.search(r'HTTP (\d{3})', message)
    if status:
        code = int(status[1])
        return code in (408, 429) or code >= 500 or (visibility and code == 404)
    # gh reports transport failures without an HTTP status.
    return any(word in message.lower() for word in
               ('timeout', 'timed out', 'connection', 'eof', 'no such host', 'network', 'tls handshake'))


def retry_refresh(read, visibility=False):
    for attempt in range(6):
        try:
            return read()
        except PendingState:
            if attempt == 5:
                raise
        except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as error:
            if attempt == 5 or not transient(error, visibility):
                raise
        time.sleep(2 ** attempt)


def read_api(endpoint, *options):
    return gh('api', '-H', 'Cache-Control: no-cache', *options, endpoint)


def validate_identity(release, tag, release_id=None):
    if type(release.get('id')) is not int or release['id'] <= 0:
        raise ValueError('Release ID must be a positive integer')
    if release['tag_name'] != tag or (release_id is not None and release['id'] != release_id):
        raise ValueError('Release identity changed during lookup')
    if release['prerelease'] is not False:
        raise ValueError('Prerelease publication is not supported')
    return release['id']


def find_release(repository, tag):
    # Listing includes drafts; a tag lookup does not reliably expose them.
    pages = retry_refresh(lambda: json.loads(read_api(
        f'repos/{repository}/releases?per_page=100', '--paginate', '--slurp')))
    matching = [release for page in pages for release in page if release['tag_name'] == tag]
    if len(matching) > 1:
        raise ValueError(f'Expected at most one release for {tag}; found {len(matching)}')
    if not matching:
        return None
    return validate_identity(matching[0], tag)


def release_assets(repository, tag, release_id=None, local=None, required=(), published=False):
    if release_id is None:
        release_id = find_release(repository, tag)
        if release_id is None:
            raise ValueError(f'Expected exactly one release for {tag}; found 0')

    def refresh():
        release = json.loads(read_api(f'repos/{repository}/releases/{release_id}'))
        validate_identity(release, tag, release_id)
        if local is not None:
            missing = compare_existing(repository, release, local)
            if {path.name for path in missing} & set(required):
                raise MissingAssets('Missing expected assets; release state has not converged')
        if published and release['draft']:
            raise PendingState('Release is still a draft')
        return release

    return retry_refresh(refresh, visibility=True)


def compare_existing(repository, release, local, require_all=False):
    existing = {asset['name']: asset for asset in release['assets']}
    if len(existing) != len(release['assets']) or set(existing) - {path.name for path in local}:
        raise ValueError('Release contains unexpected assets; explicit repair required')
    missing = []
    for path in local:
        if path.name not in existing:
            missing.append(path)
            continue
        data = read_api(f'repos/{repository}/releases/assets/{existing[path.name]["id"]}',
                        '-H', 'Accept: application/octet-stream')
        if hashlib.sha256(data).digest() != hashlib.sha256(path.read_bytes()).digest():
            raise ValueError(f'Refusing differing downloaded asset (SHA-256 mismatch): {path.name}')
    if require_all and missing:
        raise MissingAssets(f'Missing published asset: {missing[0].name}')
    return missing


def upload(repository, tag, local, attempts=3, release_id=None):
    if release_id is None:
        release_id = find_release(repository, tag)
        if release_id is None:
            raise ValueError(f'Expected exactly one release for {tag}; found 0')
    confirmed = set()
    for path in local:
        for attempt in range(attempts):
            release = release_assets(repository, tag, release_id, local, required=confirmed)
            if path.name in {asset['name'] for asset in release['assets']}:
                confirmed.add(path.name)
                break
            if not release['draft']:
                raise ValueError('Incomplete published release requires explicit repair')
            endpoint = release['upload_url'].split('{', 1)[0] + '?' + urlencode({'name': path.name})
            uncertain = None
            try:
                gh('api', '-X', 'POST', '-H', 'Content-Type: application/octet-stream',
                   '--input', path, endpoint)
            except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as error:
                # Even an HTTP conflict can mean an earlier upload became visible.
                uncertain = error
            try:
                release_assets(repository, tag, release_id, local, required=confirmed | {path.name})
            except MissingAssets:
                if uncertain is None:
                    raise  # Successful upload did not become visible; do not write again.
                if attempt + 1 == attempts or not transient(uncertain):
                    raise uncertain
                continue
            confirmed.add(path.name)
            break
    return release_assets(repository, tag, release_id, local, required=[path.name for path in local])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--tag', required=True)
    parser.add_argument('--assets', type=Path, required=True)
    args = parser.parse_args()
    upload(args.repository, args.tag, select_assets(args.assets, args.tag.removeprefix('v')))


if __name__ == '__main__':
    main()
