import runpy
from pathlib import Path
import tempfile
import unittest

STORAGE = runpy.run_path(str(Path(__file__).resolve().parents[1] / 'scripts/check-storage.py'))


class RepositoryStorageTests(unittest.TestCase):
    def test_generated_formats_cannot_bypass_size_limit_by_splitting(self):
        for name in ('specs/073/evidence/part-001.tar.gz', 'output.zip',
                     'specs/073/evidence/capture.png', 'specs/073/evidence/run.jsonl',
                     'specs/073/evidence/build.log', 'libfoo.so.1', 'module.o'):
            with self.subTest(name=name):
                self.assertIsNotNone(STORAGE['violation'](name, 12))

    def test_extensionless_binary_and_oversized_text_are_rejected(self):
        self.assertIsNotNone(STORAGE['violation']('viewer', 128, b'\x7fELF'))
        self.assertIsNotNone(STORAGE['violation']('analysis.json', 1024 * 1024 + 1))
        self.assertIsNone(STORAGE['violation']('summary.json', 1024 * 1024))

    def test_source_fonts_images_and_small_reports_are_preserved(self):
        for name in ('src/main.cpp', 'third_party/fonts/math.ttf',
                     'examples/viewer/images/shapes.png',
                     'specs/073/evidence/summary.json', 'specs/073/results.md'):
            with self.subTest(name=name):
                self.assertIsNone(STORAGE['violation'](name, 100))

    def test_validate_checks_new_files_and_ignores_deleted_paths(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / 'summary.json').write_text('{}')
            STORAGE['validate'](root, ['summary.json', 'removed.tar.gz'])
            (root / 'new.zip').write_bytes(b'PK')
            with self.assertRaisesRegex(ValueError, 'new.zip'):
                STORAGE['validate'](root, ['summary.json', 'new.zip'])

    def test_all_policy_extensions(self):
        for category in ('ARCHIVES', 'COMPILED', 'CAPTURES', 'RAW_OUTPUT'):
            for extension in STORAGE[category]:
                name = 'specs/073/evidence/output' + extension.upper()
                with self.subTest(name=name):
                    self.assertIsNotNone(STORAGE['violation'](name, 12))
        self.assertIsNotNone(STORAGE['violation']('output.tgz', 12))

    def test_git_discovery_excludes_ignored_but_checks_untracked(self):
        import subprocess
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            subprocess.run(['git', 'init', '-q', root], check=True)
            (root / '.gitignore').write_text('*.tgz\nbuild*/\n')
            (root / 'output.tgz').write_bytes(b'archive')
            (root / 'build-local').mkdir()
            (root / 'build-local/viewer').write_bytes(b'\x7fELF')
            (root / 'summary.json').write_text('{}')
            STORAGE['validate'](root)
            (root / 'new.zip').write_bytes(b'PK')
            with self.assertRaisesRegex(ValueError, 'new.zip'):
                STORAGE['validate'](root)

    def test_force_added_archive_rejected_from_staged_snapshot(self):
        import subprocess
        from unittest.mock import patch
        from test_focused_verification import hooks
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            repo = root / 'repo'
            repo.mkdir()
            subprocess.run(['git', 'init', '-q', repo], check=True)
            (repo / '.gitignore').write_text('*.tgz\n')
            artifact = repo / 'output.tgz'
            artifact.write_bytes(b'archive')
            subprocess.run(['git', '-C', repo, 'add', '-f', 'output.tgz'], check=True)
            tree = subprocess.check_output(['git', '-C', repo, 'write-tree']).decode().strip()
            artifact.unlink()  # The staged blob must still be checked.
            snapshot = root / 'snapshot'
            with patch.object(hooks, 'ROOT', repo):
                hooks.export_tree(tree, snapshot)
            with self.assertRaisesRegex(ValueError, 'output.tgz'):
                STORAGE['validate'](snapshot, ['output.tgz'])
