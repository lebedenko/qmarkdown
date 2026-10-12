"""Verify publication reruns using fake GitHub responses, without network writes."""
import importlib.util
import hashlib
import json
from pathlib import Path
import sys
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
        self.archive = self.root / 'package.tar.gz'
        self.archive.write_bytes(b'archive')
        self.checksum = self.root / 'package.tar.gz.sha256'
        self.checksum.write_text(hashlib.sha256(b'archive').hexdigest() + '  ' + self.archive.name + '\n')
        self.arch = self.root / 'qmarkdown-1.0.0-1-x86_64.pkg.tar.zst'
        self.arch.write_bytes(b'arch package')
        self.arch_checksum = self.root / (self.arch.name + '.sha256')
        self.arch_checksum.write_text(hashlib.sha256(b'arch package').hexdigest() + '  ' + self.arch.name + '\n')
        self.release = {'draft': False, 'prerelease': False, 'assets': [
            {'name': path.name, 'id': number} for number, path in enumerate(
                (self.archive, self.checksum, self.arch, self.arch_checksum), 1)]}
        self.responses = [json.dumps(self.release).encode(), *[path.read_bytes() for path in
            (self.archive, self.checksum, self.arch, self.arch_checksum)]]

    def invoke(self):
        with patch.object(sys, 'argv', ['upload-assets.py', '--repository', 'owner/repo',
                                      '--tag', 'v1.0.0', '--assets', str(self.root)]):
            upload.main()

    def test_identical_assets_are_accepted_without_upload(self):
        with patch.object(upload, 'gh', side_effect=self.responses), patch.object(upload.subprocess, 'run') as run:
            self.invoke()
            run.assert_not_called()

    def test_differing_asset_blocks_all_uploads(self):
        self.responses[1] = b'other archive'
        with patch.object(upload, 'gh', side_effect=self.responses), patch.object(upload.subprocess, 'run') as run:
            with self.assertRaisesRegex(SystemExit, 'Refusing to overwrite'):
                self.invoke()
            run.assert_not_called()

    def test_missing_assets_are_uploaded_without_clobber(self):
        self.release['assets'] = []
        with patch.object(upload, 'gh', return_value=json.dumps(self.release).encode()), patch.object(upload.subprocess, 'run') as run:
            self.invoke()
            command = run.call_args.args[0]
            self.assertIn(str(self.archive), command)
            self.assertIn(str(self.checksum), command)
            self.assertIn(str(self.arch), command)
            self.assertIn(str(self.arch_checksum), command)
            self.assertNotIn('--clobber', command)

    def test_invalid_checksum_prevents_remote_access(self):
        self.arch.write_bytes(b'corrupted')
        with patch.object(upload, 'gh') as gh:
            with self.assertRaises(SystemExit):
                self.invoke()
            gh.assert_not_called()

    def test_missing_arch_pair_is_rejected(self):
        self.arch_checksum.unlink()
        with self.assertRaises(ValueError):
            upload.select_assets(self.root)

    def test_extra_archive_is_rejected(self):
        (self.root / 'extra.pkg.tar.zst').write_bytes(b'extra')
        with self.assertRaises(ValueError):
            upload.select_assets(self.root)

    def test_checksum_filename_must_match(self):
        self.arch_checksum.write_text(hashlib.sha256(self.arch.read_bytes()).hexdigest() + '  other.pkg.tar.zst\n')
        with self.assertRaises(ValueError):
            upload.select_assets(self.root)

    def test_differing_arch_asset_blocks_missing_uploads(self):
        self.release['assets'] = [{'name': self.arch.name, 'id': 3}]
        with patch.object(upload, 'gh', side_effect=[json.dumps(self.release).encode(), b'different']), patch.object(upload.subprocess, 'run') as run:
            with self.assertRaisesRegex(SystemExit, 'Refusing to overwrite'):
                self.invoke()
            run.assert_not_called()

    def test_prerelease_is_rejected_without_upload(self):
        self.release['prerelease'] = True
        with patch.object(upload, 'gh', return_value=json.dumps(self.release).encode()), patch.object(upload.subprocess, 'run') as run:
            with self.assertRaises(SystemExit):
                self.invoke()
            run.assert_not_called()


if __name__ == '__main__':
    unittest.main()
