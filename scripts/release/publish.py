#!/usr/bin/env python3
"""Publish checked distribution bytes; stop at the first failing stage."""
import argparse
import importlib.util
import json
from pathlib import Path
import subprocess

from eligibility import SOURCE, eligibility, gh_json

spec = importlib.util.spec_from_file_location('upload_assets', Path(__file__).with_name('upload-assets.py'))
assets = importlib.util.module_from_spec(spec)
spec.loader.exec_module(assets)


spec = importlib.util.spec_from_file_location('check_docs', SOURCE / 'scripts/check-docs.py')
docs = importlib.util.module_from_spec(spec)
spec.loader.exec_module(docs)


def publish(repository, number, directory):
    stage = 'eligibility'
    try:
        approved = eligibility(repository, number)
        tag, commit, version = approved['tag'], approved['commit'], approved['version']
        stage = 'documentation validation'
        docs.validate(SOURCE, release=True)
        stage = 'artifact validation'
        local = assets.select_assets(directory, version)
        notes = SOURCE / f'docs/release-notes-{version}.md'
        if not notes.is_file():
            raise ValueError('Missing finalized release notes')
        stage = 'tag creation'
        refs = gh_json(repository, f'git/matching-refs/tags/{tag}')
        if not any(ref['ref'] == f'refs/tags/{tag}' for ref in refs):
            subprocess.run(['gh', 'api', f'repos/{repository}/git/refs', '-X', 'POST',
                            '-f', f'ref=refs/tags/{tag}', '-f', f'sha={commit}'], check=True)
        stage = 'draft release creation'
        release_id = assets.find_release(repository, tag)
        if release_id is None:
            # Do not repeat this POST if the response is uncertain: a draft may exist.
            created = json.loads(assets.gh(
                'api', '-X', 'POST', '-f', f'tag_name={tag}', '-f', f'target_commitish={commit}',
                '-f', f'name=QMarkdown {version}', '-F', f'body=@{notes}',
                '-F', 'draft=true', '-F', 'prerelease=false', f'repos/{repository}/releases'))
            release_id = assets.validate_identity(created, tag)
            if not created['draft']:
                raise ValueError('Created release is not a draft')
        stage = 'asset upload'
        release = assets.upload(repository, tag, local, release_id=release_id)
        stage = 'final eligibility'
        if eligibility(repository, number) != approved:
            raise ValueError('Release verification changed during distribution checks')
        stage = 'stable/latest publication'
        if release['draft']:
            try:
                assets.gh('api', '-X', 'PATCH', '-F', 'draft=false', '-f', 'make_latest=true',
                          f'repos/{repository}/releases/{release_id}')
            except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as error:
                if not assets.transient(error):
                    raise
                # The PATCH may have succeeded; resolve through reads, never repeat it.
        stage = 'published download SHA-256 verification'
        assets.release_assets(repository, tag, release_id, local,
                              required=[path.name for path in local], published=True)
        print(f'PASS: published {tag}; all four downloaded SHA-256 values match')
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        raise SystemExit(f'Release stopped at {stage}: {error}. Explicit repair is required for published differences.') from error


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--pr', type=int, required=True)
    parser.add_argument('--assets', type=Path, required=True)
    args = parser.parse_args()
    publish(args.repository, args.pr, args.assets)


if __name__ == '__main__':
    main()
