#!/usr/bin/env python3
"""Require the final release PR's full CI run and the unchanged merged tree."""
import argparse
import io
import json
from pathlib import Path
import subprocess
import zipfile

SOURCE = Path(__file__).resolve().parents[2]


def gh_json(repository, endpoint):
    return json.loads(subprocess.check_output(['gh', 'api', f'repos/{repository}/{endpoint}']))


def validate(pr, run, gate, main, merged_tree, versions, version, tag_commit=None):
    if not pr['merged'] or pr['base']['ref'] != 'main':
        raise ValueError('Release PR must be merged into main')
    commit = pr['merge_commit_sha']
    if main != commit:
        raise ValueError('main moved; obtain fresh verification through an updated PR')
    if (run['event'] != 'pull_request' or run['head_sha'] != pr['head']['sha']
            or run['status'] != 'completed' or run['conclusion'] != 'success'
            or run['path'] != '.github/workflows/linux-qt.yml'):
        raise ValueError('Final PR CI run did not pass')
    if (gate.get('schema') != 'qmarkdown-ci-v2' or gate.get('profile') != 'full'
            or gate.get('full') is not True or gate.get('head') != pr['head']['sha']
            or gate.get('pr') != pr['number']):
        raise ValueError('Release requires full verification of the final PR head')
    if gate.get('tree') != merged_tree:
        raise ValueError('Tested Git tree differs from the merged release tree')
    if gate.get('qt') != versions:
        raise ValueError('Verified Qt targets differ from configuration')
    if gate.get('version') != version:
        raise ValueError('Verified package version differs from merged source')
    if tag_commit is not None and tag_commit != commit:
        raise ValueError('Existing version tag points to a different commit')
    return commit


def eligibility(repository, number):
    pr = gh_json(repository, f'pulls/{number}')
    if not pr['merged'] or pr['base']['ref'] != 'main':
        raise ValueError('Release PR must be merged into main')
    main = gh_json(repository, 'git/ref/heads/main')['object']['sha']
    if main != pr['merge_commit_sha']:
        raise ValueError('main moved; obtain fresh verification through an updated PR')
    merged = gh_json(repository, f'git/commits/{pr["merge_commit_sha"]}')
    runs = gh_json(repository, 'actions/workflows/linux-qt.yml/runs?event=pull_request&head_sha='
                   + pr['head']['sha'] + '&per_page=100')['workflow_runs']
    # GitHub removes PR associations from completed runs after merge. The gate
    # artifact pins the PR number; workflow/event/final head still select the run.
    relevant = [run for run in runs if run['head_sha'] == pr['head']['sha']]
    if not relevant:
        raise ValueError('No final PR CI run found')
    run = max(relevant, key=lambda item: item['id'])
    if run['status'] != 'completed' or run['conclusion'] != 'success':
        raise ValueError('Latest final PR CI run did not pass')
    artifacts = gh_json(repository, f'actions/runs/{run["id"]}/artifacts')['artifacts']
    gates = [item for item in artifacts if item['name'] == 'release-gate' and not item['expired']]
    if len(gates) != 1:
        raise ValueError('Missing full CI release-gate artifact')
    data = subprocess.check_output(['gh', 'api', f'repos/{repository}/actions/artifacts/{gates[0]["id"]}/zip'])
    with zipfile.ZipFile(io.BytesIO(data)) as archive:
        gate = json.loads(archive.read('gate.json'))
    # The caller must check out the merged commit, including its exact tooling.
    local_commit = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=SOURCE, text=True).strip()
    if local_commit != pr['merge_commit_sha']:
        raise ValueError('Checkout does not match merged release commit')
    import sys
    sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'ci-toolchain'))
    from qt_versions import load_versions
    import re
    version = re.search(r'project\(QMarkdown VERSION (\d+\.\d+\.\d+)', (SOURCE / 'CMakeLists.txt').read_text())[1]
    refs = gh_json(repository, f'git/matching-refs/tags/v{version}')
    exact = [ref for ref in refs if ref['ref'] == f'refs/tags/v{version}']
    tag_commit = None
    if exact:
        obj = exact[0]['object']
        while obj['type'] == 'tag':
            obj = gh_json(repository, f'git/tags/{obj["sha"]}')['object']
        tag_commit = obj['sha']
    commit = validate(pr, run, gate, main, merged['tree']['sha'], load_versions(), version, tag_commit)
    return {'commit': commit, 'version': version, 'tag': 'v' + version, 'run': run['id']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repository', required=True)
    parser.add_argument('--pr', type=int, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    args.output.write_text(json.dumps(eligibility(args.repository, args.pr), indent=2) + '\n')


if __name__ == '__main__':
    main()
