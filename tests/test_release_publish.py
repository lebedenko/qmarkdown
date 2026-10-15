"""Exercise publication orchestration with a disposable fake GitHub service."""
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

from release_docs_fixture import write_docs

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts/release'))
spec = importlib.util.spec_from_file_location('publish', ROOT / 'scripts/release/publish.py')
publish = importlib.util.module_from_spec(spec)
spec.loader.exec_module(publish)


class PublishTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / 'assets'
        self.root.mkdir()
        self.source_root = Path(self.temp.name) / 'checkout'
        write_docs(self.source_root)
        self.commit = 'a' * 40
        self.head = 'b' * 40
        self.tree = 'c' * 40
        self.tag = None
        self.release = None
        self.remote = {}
        self.writes = []
        self.corrupt_published = False
        self.move_after_upload = False
        self.main = self.commit
        for name, data in (('qmarkdown-1.6.0.tar.gz', b'library source'),
                           ('qmarkdown-1.6.0-1-x86_64.pkg.tar.zst', b'arch package')):
            path = self.root / name
            path.write_bytes(data)
            path.with_name(name + '.sha256').write_text(hashlib.sha256(data).hexdigest() + '  ' + name + '\n')
        stream = io.BytesIO()
        with zipfile.ZipFile(stream, 'w') as archive:
            archive.writestr('gate.json', json.dumps({'full': True, 'pr': 42, 'head': self.head,
                'tree': self.tree, 'qt': json.loads((ROOT / 'scripts/ci-toolchain/qt-versions.json').read_text()), 'version': '1.6.0'}))
        self.gate_zip = stream.getvalue()

    def read(self, command, **kwargs):
        if command[0] == 'git':
            return self.commit + '\n'
        endpoint = command[-1].removeprefix('repos/owner/repo/')
        if endpoint == 'pulls/42':
            response = {'number': 42, 'merged': True, 'base': {'ref': 'main'},
                        'head': {'sha': self.head}, 'merge_commit_sha': self.commit}
        elif endpoint == 'git/ref/heads/main':
            response = {'object': {'sha': self.main}}
        elif endpoint == 'git/commits/' + self.commit:
            response = {'tree': {'sha': self.tree}}
        elif endpoint.startswith('actions/workflows/linux-qt.yml/runs?'):
            response = {'workflow_runs': [{'id': 7, 'event': 'pull_request', 'head_sha': self.head,
                'status': 'completed', 'conclusion': 'success', 'path': '.github/workflows/linux-qt.yml',
                'pull_requests': []}]}
        elif endpoint == 'actions/runs/7/artifacts':
            response = {'artifacts': [{'id': 8, 'name': 'release-gate', 'expired': False}]}
        elif endpoint == 'actions/artifacts/8/zip':
            return self.gate_zip
        elif endpoint.startswith('git/matching-refs/tags/'):
            response = [] if self.tag is None else [{'ref': 'refs/tags/v1.6.0', 'object': {'type': 'commit', 'sha': self.tag}}]
        elif endpoint == 'releases?per_page=100':
            response = [[self.release] if self.release else []]
        elif endpoint == 'releases/tags/v1.6.0':
            raise subprocess.CalledProcessError(1, command)
        elif endpoint == 'releases/123':
            response = {**self.release, 'assets': [{'id': name, 'name': name} for name in self.remote]}
        elif endpoint.startswith('releases/assets/'):
            if self.corrupt_published and not self.release['draft']:
                return b'corrupted download'
            return self.remote[endpoint.removeprefix('releases/assets/')]
        else:
            raise AssertionError(command)
        return json.dumps(response).encode()

    def write(self, command, **kwargs):
        self.writes.append(command)
        if command[1:3] == ['api', 'repos/owner/repo/git/refs']:
            self.tag = self.commit
        elif command[1:3] == ['release', 'create']:
            self.release = {'id': 123, 'tag_name': 'v1.6.0', 'draft': True, 'prerelease': False}
        elif command[1:3] == ['release', 'upload']:
            for name in command[6:]:
                path = Path(name)
                self.remote[path.name] = path.read_bytes()
            if self.move_after_upload:
                self.main = 'moved'
        elif command[1:3] == ['release', 'edit']:
            self.assertIn('--latest', command)
            self.release['draft'] = False
        else:
            raise AssertionError(command)
        return subprocess.CompletedProcess(command, 0)

    def invoke(self):
        with patch.object(sys.modules['eligibility'], 'SOURCE', self.source_root), patch.object(publish, 'SOURCE', self.source_root), patch.object(subprocess, 'check_output', side_effect=self.read), patch.object(subprocess, 'run', side_effect=self.write):
            publish.publish('owner/repo', 42, self.root)

    def test_invalid_documentation_causes_zero_remote_writes(self):
        path = self.source_root / 'docs/arch-install.md'
        path.write_text(path.read_text().replace('qmarkdown-1.6.0-1-', 'qmarkdown-1.5.0-1-'))
        with self.assertRaisesRegex(SystemExit, 'documentation validation.*docs/arch-install.md'):
            self.invoke()
        self.assertEqual(self.writes, [])
        self.assertIsNone(self.tag)
        self.assertIsNone(self.release)

    def test_tags_uploads_draft_then_publishes_and_verifies_hashes(self):
        self.invoke()
        self.assertEqual(self.tag, self.commit)
        self.assertFalse(self.release['draft'])
        self.assertEqual(set(self.remote), {path.name for path in self.root.iterdir()})
        self.assertEqual([command[2] for command in self.writes[1:]], ['create', 'upload', 'edit'])

    def test_missing_artifacts_fail_before_tag_or_release_writes(self):
        (self.root / 'qmarkdown-1.6.0.tar.gz').unlink()
        with self.assertRaisesRegex(SystemExit, 'artifact validation'):
            self.invoke()
        self.assertEqual(self.writes, [])

    def test_moved_main_and_mismatched_tag_fail_before_writes(self):
        self.main = 'moved'
        with self.assertRaisesRegex(SystemExit, 'eligibility'):
            self.invoke()
        self.main = self.commit
        self.tag = 'other'
        with self.assertRaisesRegex(SystemExit, 'eligibility'):
            self.invoke()
        self.assertEqual(self.writes, [])

    def test_download_mismatch_stops_without_claiming_completion(self):
        self.corrupt_published = True
        with self.assertRaisesRegex(SystemExit, 'published download SHA-256 verification'):
            self.invoke()
        self.assertFalse(self.release['draft'])
        self.assertEqual(self.tag, self.commit)

    def test_differing_existing_assets_never_get_overwritten(self):
        self.tag = self.commit
        self.release = {'id': 123, 'tag_name': 'v1.6.0', 'draft': False, 'prerelease': False}
        self.remote = {'qmarkdown-1.6.0.tar.gz': b'different'}
        with self.assertRaisesRegex(SystemExit, 'asset upload'):
            self.invoke()
        self.assertEqual(self.writes, [])

    def test_main_move_during_upload_leaves_draft_and_stops_publication(self):
        self.move_after_upload = True
        with self.assertRaisesRegex(SystemExit, 'final eligibility'):
            self.invoke()
        self.assertTrue(self.release['draft'])
        self.assertFalse(any(command[1:3] == ['release', 'edit'] for command in self.writes))

    def test_existing_identical_published_release_can_be_hash_verified(self):
        self.tag = self.commit
        self.release = {'id': 123, 'tag_name': 'v1.6.0', 'draft': False, 'prerelease': False}
        self.remote = {path.name: path.read_bytes() for path in self.root.iterdir()}
        self.invoke()
        self.assertEqual(self.writes, [])


if __name__ == '__main__':
    unittest.main()
