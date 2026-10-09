"""Exercise installer preflight without root or a Qt build."""
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

SOURCE = Path(__file__).resolve().parents[1]


class InstallerTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.work = Path(self.temp.name)
        self.package = self.work / 'package'
        self.package.mkdir()
        shutil.copy2(SOURCE / 'scripts/release/installer.py', self.package / 'installer.py')
        self.root = self.work / 'root'
        self.root.mkdir()
        self.name = 'usr/lib/x86_64-linux-gnu/libQMarkdown.so.1.0.0'
        self.file = self.package / 'payload' / self.name
        self.file.parent.mkdir(parents=True)
        self.file.write_bytes(b'package data')
        self.file.chmod(0o644)
        self.manifest = {self.name: {'kind': 'file', 'mode': 0o644,
                                   'sha256': hashlib.sha256(b'package data').hexdigest()}}
        (self.package / 'metadata.json').write_text(json.dumps({'architecture': 'x86_64'}))
        self.save_manifest()

    def save_manifest(self):
        (self.package / 'manifest.json').write_text(json.dumps(self.manifest))

    def invoke(self, operation='install', success=False):
        result = subprocess.run([sys.executable, self.package / 'installer.py', operation,
                                 '--destdir', self.root], capture_output=True, text=True)
        self.assertEqual(result.returncode == 0, success, result.stdout + result.stderr)
        return result

    def test_corrupt_payload_is_rejected_without_creating_destination_files(self):
        self.file.write_bytes(b'corrupt')
        self.invoke()
        self.assertEqual(list(self.root.iterdir()), [])

    def test_traversal_manifest_cannot_write_outside_root(self):
        self.manifest['usr/../../escape'] = self.manifest[self.name]
        self.save_manifest()
        self.invoke()
        self.assertFalse((self.work / 'escape').exists())
        self.assertEqual(list(self.root.iterdir()), [])

    def test_escaping_symlink_is_rejected(self):
        link = self.file.parent / 'libQMarkdown.so'
        link.symlink_to('../../../../escape')
        self.manifest[link.relative_to(self.package / 'payload').as_posix()] = {
            'kind': 'link', 'target': '../../../../escape'}
        self.save_manifest()
        self.invoke()
        self.assertEqual(list(self.root.iterdir()), [])

    def test_payload_symlink_ancestor_is_rejected(self):
        directory = self.package / 'payload/usr/lib'
        outside = self.work / 'outside'
        directory.rename(outside)
        directory.symlink_to(outside)
        self.invoke()
        self.assertEqual(list(self.root.iterdir()), [])

    def test_record_temporary_conflict_prevents_upgrade_mutation(self):
        self.invoke(success=True)
        installed = self.root / self.name
        original = installed.read_bytes()
        temporary = self.root / 'usr/share/qmarkdown/installed.tmp'
        temporary.write_text('unrelated')
        self.file.write_bytes(b'new release')
        self.manifest[self.name]['sha256'] = hashlib.sha256(b'new release').hexdigest()
        self.save_manifest()
        self.invoke()
        self.assertEqual(installed.read_bytes(), original)
        self.assertEqual(temporary.read_text(), 'unrelated')

    def test_modified_symlink_is_preserved_on_uninstall_and_blocks_upgrade(self):
        link_name = 'usr/lib/x86_64-linux-gnu/libQMarkdown.so'
        (self.package / 'payload' / link_name).symlink_to('libQMarkdown.so.1.0.0')
        self.manifest[link_name] = {'kind': 'link', 'target': 'libQMarkdown.so.1.0.0'}
        self.save_manifest()
        self.invoke(success=True)
        installed = self.root / link_name
        installed.unlink()
        installed.symlink_to('local.so')
        self.invoke()
        self.assertTrue((self.root / self.name).exists())
        result = self.invoke('uninstall', success=True)
        self.assertIn('Preserving modified file', result.stdout)
        self.assertEqual(os.readlink(installed), 'local.so')
        self.assertFalse((self.root / self.name).exists())
        self.invoke('uninstall', success=True)

    def test_wrong_architecture_is_rejected_before_copying(self):
        (self.package / 'metadata.json').write_text(json.dumps({'architecture': 'aarch64'}))
        self.invoke()
        self.assertEqual(list(self.root.iterdir()), [])

    def test_relative_staging_root_is_rejected(self):
        self.root = Path('relative')
        self.invoke()


if __name__ == '__main__':
    unittest.main()
