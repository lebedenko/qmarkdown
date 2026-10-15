"""Fake GitHub upload retries and byte verification without network writes."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location('upload', Path(__file__).resolve().parents[1] / 'scripts/release/upload-assets.py')
upload = importlib.util.module_from_spec(spec)
spec.loader.exec_module(upload)


class UploadTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        for name, data in (('qmarkdown-1.6.0.tar.gz', b'source'),
                           ('qmarkdown-1.6.0-1-x86_64.pkg.tar.zst', b'arch')):
            path = self.root / name
            path.write_bytes(data)
            path.with_name(name + '.sha256').write_text(hashlib.sha256(data).hexdigest() + '  ' + name + '\n')
        self.local = upload.select_assets(self.root, '1.6.0')
        self.remote = {path.name: path.read_bytes() for path in self.local}
        self.draft = True

    def gh(self, *args):
        endpoint = args[-1]
        if '/releases/tags/' in endpoint:
            raise subprocess.CalledProcessError(1, args)
        if endpoint.endswith('/releases?per_page=100'):
            return json.dumps([[{'tag_name': 'v1.6.0', 'id': 123}]]).encode()
        if endpoint.endswith('/releases/123'):
            return json.dumps({'id': 123, 'tag_name': 'v1.6.0', 'draft': self.draft, 'prerelease': False,
                               'assets': [{'name': name, 'id': name} for name in self.remote]}).encode()
        return self.remote[endpoint.split('/assets/')[1]]

    def upload_missing(self, command, **kwargs):
        for path in command[6:]:
            path = Path(path)
            self.remote[path.name] = path.read_bytes()
        self.assertNotIn('--clobber', command)

    def test_paginated_exact_match_refreshes_draft_and_published_state(self):
        for draft in (True, False):
            self.draft = draft

            def paginated(*args):
                if args[-1].endswith('/releases?per_page=100'):
                    self.assertEqual(args[:3], ('api', '--paginate', '--slurp'))
                    return json.dumps([[{'tag_name': 'v1.6.0-extra', 'id': 999}],
                                       [{'tag_name': 'v1.6.0', 'id': 123}]]).encode()
                return self.gh(*args)

            with patch.object(upload, 'gh', side_effect=paginated):
                release = upload.release_assets('owner/repo', 'v1.6.0')
            self.assertEqual(release['draft'], draft)
            self.assertEqual(len(release['assets']), 4)

    def test_missing_and_ambiguous_matches_stop_before_id_lookup(self):
        for pages in ([[]], [[{'tag_name': 'v1.6.0', 'id': 1}],
                              [{'tag_name': 'v1.6.0', 'id': 2}]]):
            with patch.object(upload, 'gh', return_value=json.dumps(pages)) as gh:
                with self.assertRaisesRegex(ValueError, 'exactly one'):
                    upload.release_assets('owner/repo', 'v1.6.0')
                self.assertEqual(gh.call_count, 1)

    def test_listing_and_id_api_errors_stop_without_upload(self):
        for failed in ('/releases?per_page=100', '/releases/123'):
            def failure(*args):
                if args[-1].endswith(failed):
                    raise subprocess.CalledProcessError(1, args)
                return self.gh(*args)

            with patch.object(upload, 'gh', side_effect=failure), patch.object(upload.subprocess, 'run') as run:
                with self.assertRaises(subprocess.CalledProcessError):
                    upload.upload('owner/repo', 'v1.6.0', self.local)
                run.assert_not_called()

    def test_refreshed_prerelease_or_changed_identity_is_rejected(self):
        for change in ({'prerelease': True}, {'tag_name': 'other'}, {'id': 456}):
            def changed(*args):
                data = self.gh(*args)
                if args[-1].endswith('/releases/123'):
                    return json.dumps({**json.loads(data), **change}).encode()
                return data

            with patch.object(upload, 'gh', side_effect=changed):
                with self.assertRaises(ValueError):
                    upload.release_assets('owner/repo', 'v1.6.0')

    def test_identical_assets_are_accepted_without_upload(self):
        with patch.object(upload, 'gh', side_effect=self.gh), patch.object(upload.subprocess, 'run') as run:
            upload.upload('owner/repo', 'v1.6.0', self.local)
            run.assert_not_called()

    def test_differing_existing_asset_blocks_all_uploads(self):
        self.remote = {self.local[-2].name: b'different'}
        with patch.object(upload, 'gh', side_effect=self.gh), patch.object(upload.subprocess, 'run') as run:
            with self.assertRaisesRegex(ValueError, 'SHA-256 mismatch'):
                upload.upload('owner/repo', 'v1.6.0', self.local)
            run.assert_not_called()

    def test_missing_draft_assets_are_uploaded_without_clobber(self):
        self.remote = {}
        with patch.object(upload, 'gh', side_effect=self.gh), patch.object(upload.subprocess, 'run', side_effect=self.upload_missing):
            upload.upload('owner/repo', 'v1.6.0', self.local)
        self.assertEqual(self.remote, {path.name: path.read_bytes() for path in self.local})

    def test_partial_upload_retry_refreshes_and_accepts_identical_bytes(self):
        self.remote = {}
        attempts = []

        def partial(command, **kwargs):
            attempts.append(command)
            if len(attempts) == 1:
                first = Path(command[6])
                self.remote[first.name] = first.read_bytes()
                raise subprocess.CalledProcessError(1, command)
            self.upload_missing(command, **kwargs)

        with patch.object(upload, 'gh', side_effect=self.gh), patch.object(upload.subprocess, 'run', side_effect=partial), patch.object(upload.time, 'sleep'):
            upload.upload('owner/repo', 'v1.6.0', self.local)
        self.assertEqual(len(attempts), 2)
        self.assertNotIn(attempts[0][6], attempts[1][6:])

    def test_retry_rejects_a_differing_asset(self):
        self.remote = {}

        def partial(command, **kwargs):
            self.remote[self.local[0].name] = b'other'
            raise subprocess.CalledProcessError(1, command)

        with patch.object(upload, 'gh', side_effect=self.gh), patch.object(upload.subprocess, 'run', side_effect=partial) as run, patch.object(upload.time, 'sleep'):
            with self.assertRaises(ValueError):
                upload.upload('owner/repo', 'v1.6.0', self.local)
            self.assertEqual(run.call_count, 1)

    def test_retries_are_bounded(self):
        self.remote = {}
        with patch.object(upload, 'gh', side_effect=self.gh), patch.object(upload.subprocess, 'run', side_effect=subprocess.CalledProcessError(1, 'gh')) as run, patch.object(upload.time, 'sleep'):
            with self.assertRaises(subprocess.CalledProcessError):
                upload.upload('owner/repo', 'v1.6.0', self.local)
            self.assertEqual(run.call_count, 3)

    def test_published_hash_mismatch_and_missing_download_are_rejected(self):
        self.draft = False
        for remote in ({self.local[0].name: b'corrupted'}, {}):
            self.remote = remote
            with patch.object(upload, 'gh', side_effect=self.gh):
                release = upload.release_assets('owner/repo', 'v1.6.0')
                with self.assertRaises(ValueError):
                    upload.compare_existing('owner/repo', release, self.local, require_all=True)

    def test_incomplete_published_release_requires_explicit_repair(self):
        self.draft = False
        self.remote = {}
        with patch.object(upload, 'gh', side_effect=self.gh), patch.object(upload.subprocess, 'run') as run:
            with self.assertRaisesRegex(ValueError, 'explicit repair'):
                upload.upload('owner/repo', 'v1.6.0', self.local)
            run.assert_not_called()

    def test_invalid_missing_extra_or_wrong_version_assets_are_rejected(self):
        with self.assertRaises(ValueError):
            upload.select_assets(self.root, '1.7.0')
        extra = self.root / 'summary.json'
        extra.write_text('{}')
        with self.assertRaises(ValueError):
            upload.select_assets(self.root)
        extra.unlink()
        checksum = self.local[-1]
        original = checksum.read_bytes()
        checksum.write_text('invalid')
        with self.assertRaises(ValueError):
            upload.select_assets(self.root)
        checksum.write_bytes(original)
        checksum.unlink()
        with self.assertRaises(ValueError):
            upload.select_assets(self.root)

    def test_unexpected_remote_assets_require_repair(self):
        self.remote['summary.json'] = b'{}'
        with patch.object(upload, 'gh', side_effect=self.gh):
            with self.assertRaisesRegex(ValueError, 'unexpected'):
                upload.upload('owner/repo', 'v1.6.0', self.local)


if __name__ == '__main__':
    unittest.main()
