"""Fake GitHub upload retries and byte verification without network writes."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch
from urllib.parse import parse_qs, urlsplit

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
        self.pages = [[{'tag_name': 'v1.6.0', 'id': 123, 'prerelease': False}]]
        self.change = {}
        self.writes = []
        self.reads = []
        self.errors = []
        self.stale = 0
        self.upload_delay = 0
        self.uncertain = False
        self.drop_upload = False
        self.corrupt_upload = False
        self.sleep = self.enterContext(patch.object(upload.time, 'sleep'))
        self.enterContext(patch.object(upload, 'gh', side_effect=self.gh))

    def gh(self, *args):
        endpoint = str(args[-1])
        if '-X' in args:
            self.assertIn('POST', args)
            self.assertTrue(endpoint.startswith('https://uploads.github.com/repos/owner/repo/releases/123/assets?'))
            path = Path(args[args.index('--input') + 1])
            name = parse_qs(urlsplit(endpoint).query)['name'][0]
            self.assertEqual(name, path.name)
            self.assertNotIn(name, self.remote, 'Never overwrite existing bytes')
            self.writes.append(name)
            if not self.drop_upload:
                self.remote[name] = b'corrupt' if self.corrupt_upload else path.read_bytes()
            self.stale = self.upload_delay
            if self.uncertain:
                raise subprocess.CalledProcessError(1, args, stderr=b'connection reset')
            return b'{}'
        self.reads.append(endpoint)
        self.assertIn('Cache-Control: no-cache', args)
        if self.errors:
            error = self.errors.pop(0)
            if error is not None:
                raise error
        if endpoint.endswith('/releases?per_page=100'):
            self.assertIn('--paginate', args)
            self.assertIn('--slurp', args)
            return json.dumps(self.pages).encode()
        if endpoint.endswith('/releases/123'):
            visible = list(self.remote)
            if self.stale:
                self.stale -= 1
                visible = [name for name in visible if name != self.writes[-1]]
            return json.dumps({'id': 123, 'tag_name': 'v1.6.0', 'draft': self.draft, 'prerelease': False,
                               'upload_url': 'https://uploads.github.com/repos/owner/repo/releases/123/assets{?name,label}',
                               'assets': [{'name': name, 'id': name} for name in visible], **self.change}).encode()
        if '/releases/assets/' in endpoint:
            return self.remote[endpoint.split('/assets/')[1]]
        raise AssertionError(args)

    def invoke(self):
        return upload.upload('owner/repo', 'v1.6.0', self.local)

    def test_paginated_exact_match_refreshes_draft_and_published_state(self):
        self.pages = [[{'tag_name': 'v1.6.0-extra', 'id': 999}], self.pages[0]]
        for draft in (True, False):
            self.draft = draft
            release = upload.release_assets('owner/repo', 'v1.6.0')
            self.assertEqual(release['draft'], draft)
            self.assertEqual(len(release['assets']), 4)

    def test_missing_and_ambiguous_matches_stop_before_id_lookup(self):
        for pages in ([[]], [self.pages[0], self.pages[0]]):
            self.pages = pages
            self.reads = []
            with self.assertRaisesRegex(ValueError, 'one release'):
                self.invoke()
            self.assertEqual(len(self.reads), 1)
        self.assertEqual(self.writes, [])

    def test_refreshed_prerelease_or_changed_identity_fails_immediately(self):
        for change in ({'prerelease': True}, {'tag_name': 'other'}, {'id': 456}, {'id': '123'}):
            self.change = change
            with self.assertRaises(ValueError):
                self.invoke()
        self.assertEqual(self.writes, [])
        self.sleep.assert_not_called()

    def test_identical_assets_are_accepted_without_upload(self):
        self.invoke()
        self.assertEqual(self.writes, [])

    def test_differing_asset_blocks_all_uploads_even_when_earlier_assets_missing(self):
        self.remote = {self.local[-2].name: b'different'}
        with self.assertRaisesRegex(ValueError, 'SHA-256 mismatch'):
            self.invoke()
        self.assertEqual(self.writes, [])
        self.sleep.assert_not_called()

    def test_missing_draft_assets_are_uploaded_without_overwrite(self):
        self.remote = {}
        self.invoke()
        self.assertEqual(self.remote, {path.name: path.read_bytes() for path in self.local})
        self.assertEqual(len(self.writes), 4)

    def test_stale_asset_lists_wait_without_reupload(self):
        self.remote = {}
        self.upload_delay = 3
        self.invoke()
        self.assertEqual(len(self.writes), 4)
        self.assertEqual([call.args[0] for call in self.sleep.call_args_list], [1, 2, 4] * 4)

    def test_partial_upload_uncertain_response_accepts_identical_bytes(self):
        self.remote = {self.local[0].name: self.local[0].read_bytes()}
        self.uncertain = True
        self.upload_delay = 2
        self.invoke()
        self.assertEqual(self.writes, [path.name for path in self.local[1:]])
        self.assertEqual(len(self.remote), 4)

    def test_uncertain_upload_with_differing_bytes_stops_immediately(self):
        self.remote = {}
        self.uncertain = True
        self.corrupt_upload = True
        with self.assertRaisesRegex(ValueError, 'SHA-256 mismatch'):
            self.invoke()
        self.assertEqual(len(self.writes), 1)
        self.sleep.assert_not_called()

    def test_upload_retry_exhaustion_waits_for_visibility_before_each_retry(self):
        self.remote = {}
        self.uncertain = True
        self.drop_upload = True
        with self.assertRaises(subprocess.CalledProcessError):
            self.invoke()
        self.assertEqual(len(self.writes), 3)
        self.assertEqual([call.args[0] for call in self.sleep.call_args_list], [1, 2, 4, 8, 16] * 3)

    def test_successful_upload_visibility_exhaustion_never_reuploads(self):
        self.remote = {}
        self.upload_delay = 6
        with self.assertRaisesRegex(ValueError, 'Missing expected assets'):
            self.invoke()
        self.assertEqual(len(self.writes), 1)
        self.assertEqual([call.args[0] for call in self.sleep.call_args_list], [1, 2, 4, 8, 16])

    def test_uncertain_upload_read_failure_does_not_authorize_another_write(self):
        self.remote = {}
        self.uncertain = True
        self.errors = [None, None] + [subprocess.CalledProcessError(1, 'gh', stderr=b'HTTP 503')] * 6
        with self.assertRaises(subprocess.CalledProcessError):
            self.invoke()
        self.assertEqual(len(self.writes), 1)

    def test_transient_discovery_and_download_failures_recover(self):
        for message in (b'HTTP 408', b'HTTP 429', b'HTTP 502', b'connection reset', b'network is unreachable'):
            self.errors = [subprocess.CalledProcessError(1, 'gh', stderr=message), None, None,
                           subprocess.CalledProcessError(1, 'gh', stderr=message)]
            self.invoke()
        self.assertEqual(self.writes, [])

    def test_permanent_http_failures_are_not_retried(self):
        for message in (b'HTTP 401', b'HTTP 403', b'HTTP 422'):
            self.errors = [subprocess.CalledProcessError(1, 'gh', stderr=message)]
            with self.assertRaises(subprocess.CalledProcessError):
                self.invoke()
        self.sleep.assert_not_called()
        self.assertEqual(self.writes, [])

    def test_refresh_exhaustion_does_not_upload(self):
        self.errors = [None] + [subprocess.CalledProcessError(1, 'gh', stderr=b'HTTP 404')] * 6
        with self.assertRaises(subprocess.CalledProcessError):
            self.invoke()
        self.assertEqual(self.writes, [])
        self.assertEqual([call.args[0] for call in self.sleep.call_args_list], [1, 2, 4, 8, 16])

    def test_transport_timeout_recovers_without_writes(self):
        self.errors = [subprocess.TimeoutExpired('gh', 30)]
        self.invoke()
        self.assertEqual(self.writes, [])
        self.sleep.assert_called_once_with(1)

    def test_discovery_exhaustion_cannot_be_treated_as_missing_release(self):
        self.errors = [subprocess.CalledProcessError(1, 'gh', stderr=b'HTTP 503')] * 6
        with self.assertRaises(subprocess.CalledProcessError):
            self.invoke()
        self.assertEqual(self.writes, [])
        self.assertEqual(len(self.reads), 6)

    def test_invalid_discovered_id_stops_before_refresh_or_upload(self):
        for release_id in (0, -1, '123', True):
            self.pages[0][0]['id'] = release_id
            with self.assertRaisesRegex(ValueError, 'positive integer'):
                self.invoke()
        self.assertEqual(self.writes, [])
        self.sleep.assert_not_called()

    def test_incomplete_published_release_requires_explicit_repair(self):
        self.draft = False
        self.remote = {}
        with self.assertRaisesRegex(ValueError, 'explicit repair'):
            self.invoke()
        self.assertEqual(self.writes, [])

    def test_published_hash_mismatch_and_missing_download_are_rejected(self):
        self.draft = False
        for remote in ({self.local[0].name: b'corrupted'}, {}):
            self.remote = remote
            release = upload.release_assets('owner/repo', 'v1.6.0')
            with self.assertRaises(ValueError):
                upload.compare_existing('owner/repo', release, self.local, require_all=True)

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

    def test_unexpected_and_duplicate_remote_assets_require_repair(self):
        self.remote['summary.json'] = b'{}'
        with self.assertRaisesRegex(ValueError, 'unexpected'):
            self.invoke()
        self.change = {'assets': [{'name': self.local[0].name, 'id': 1}] * 2}
        with self.assertRaisesRegex(ValueError, 'unexpected'):
            self.invoke()
        self.assertEqual(self.writes, [])
        self.sleep.assert_not_called()


if __name__ == '__main__':
    unittest.main()
