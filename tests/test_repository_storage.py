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
