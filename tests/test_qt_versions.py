"""Validate reproducible CI roles and reject floating or malformed SDK targets."""

import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

HELPER = Path(__file__).resolve().parents[1] / 'scripts/ci-toolchain/qt_versions.py'
spec = importlib.util.spec_from_file_location('qt_versions', HELPER)
versions = importlib.util.module_from_spec(spec)
spec.loader.exec_module(versions)


class QtVersionsTest(unittest.TestCase):
    def test_exact_targets_and_matrix(self):
        self.assertEqual(versions.load_versions(), {'minimum': '6.8.0', 'development': '6.12.0'})
        for role, version in versions.load_versions().items():
            result = subprocess.run([sys.executable, HELPER, '--role', role],
                                    check=True, capture_output=True, text=True)
            self.assertEqual(result.stdout.strip(), version)
        result = subprocess.run([sys.executable, HELPER, '--matrix'],
                                check=True, capture_output=True, text=True)
        self.assertEqual(json.loads(result.stdout), {'include': [
            {'role': 'minimum', 'qt': '6.8.0'}, {'role': 'development', 'qt': '6.12.0'}]})

    def test_rejects_unconfigured_or_floating_versions(self):
        for version in ('6.11.3', 'latest', '6.12', ''):
            with self.subTest(version=version):
                result = subprocess.run([sys.executable, HELPER, '--validate-version', version],
                                        capture_output=True, text=True)
                self.assertEqual(result.returncode, 2)
        for version in versions.load_versions().values():
            subprocess.run([sys.executable, HELPER, '--validate-version', version], check=True)

    def test_rejects_invalid_configuration(self):
        invalid = [[], {}, {'minimum': '6.8.0'},
                   {'minimum': '6.8.0', 'development': 'latest'},
                   {'minimum': '6.8.0', 'development': 6.12},
                   {'minimum': '6.8.0', 'development': '6.8.0'},
                   {'minimum': '6.12.0', 'development': '6.8.0'},
                   {'minimum': '6.8.0', 'development': '6.12.0', 'extra': '7.0.0'}]
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'versions.json'
            for value in invalid:
                with self.subTest(value=value):
                    path.write_text(json.dumps(value))
                    with self.assertRaises(ValueError):
                        versions.load_versions(path)
            path.write_text('invalid json')
            with self.assertRaises(ValueError):
                versions.load_versions(path)


if __name__ == '__main__':
    unittest.main()
