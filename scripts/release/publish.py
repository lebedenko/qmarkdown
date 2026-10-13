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


def publish(repository, number, directory):
    stage = 'eligibility'
    try:
        approved = eligibility(repository, number)
        tag, commit, version = approved['tag'], approved['commit'], approved['version']
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
        # Listing avoids interpreting transient API failures as a missing release.
        releases = json.loads(assets.gh('api', '--paginate', '--slurp', f'repos/{repository}/releases?per_page=100'))
        matching = [release for page in releases for release in page if release['tag_name'] == tag]
        if not matching:
            subprocess.run(['gh', 'release', 'create', tag, '--repo', repository,
                            '--verify-tag', '--draft', '--title', f'QMarkdown {version}',
                            '--notes-file', str(notes)], check=True)
        stage = 'asset upload'
        assets.upload(repository, tag, local)
        stage = 'final eligibility'
        if eligibility(repository, number) != approved:
            raise ValueError('Release verification changed during distribution checks')
        stage = 'stable/latest publication'
        release = assets.release_assets(repository, tag)
        if release['draft']:
            subprocess.run(['gh', 'release', 'edit', tag, '--repo', repository,
                            '--draft=false', '--latest'], check=True)
        stage = 'published download SHA-256 verification'
        published = assets.release_assets(repository, tag)
        if published['draft']:
            raise ValueError('Release is still a draft')
        assets.compare_existing(repository, published, local, require_all=True)
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
