"""Validate generated Arch recipe inputs without provisioning containers."""
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('arch', ROOT / 'scripts/build-arch-package.py')
arch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(arch)

verify_spec = importlib.util.spec_from_file_location('verify_arch', ROOT / 'scripts/arch/verify.py')
verify_arch = importlib.util.module_from_spec(verify_spec)
verify_spec.loader.exec_module(verify_arch)


class ArchRecipeTest(unittest.TestCase):
    def test_version_comes_from_selected_cmake_content(self):
        self.assertEqual(arch.version_from_cmake('project(QMarkdown VERSION 2.3.4 LANGUAGES CXX)'), '2.3.4')
        for text in ('project(Other VERSION 1.0.0)', 'project(QMarkdown VERSION 1.0)'):
            with self.assertRaises(ValueError):
                arch.version_from_cmake(text)

    def test_recipe_evaluates_to_one_system_package(self):
        rendered = arch.recipe((ROOT / 'scripts/arch/PKGBUILD.in').read_text(), '2.3.4', '2', '6.12.1', 'a' * 64)
        with tempfile.TemporaryDirectory() as work:
            path = Path(work) / 'PKGBUILD'
            path.write_text(rendered)
            result = subprocess.check_output(['bash', '-c',
                'source "$1"; printf "%s\\n" "$pkgname-$pkgver-$pkgrel-${arch[0]}" "${depends[@]}" "${sha256sums[@]}" "${options[@]}"',
                'recipe', str(path)], text=True).splitlines()
        self.assertEqual(result, ['qmarkdown-2.3.4-2-x86_64', 'glibc', 'gcc-libs',
            'qt6-base>=6.12.1', 'qt6-declarative>=6.12.1', 'syntax-highlighting>=6.10.0', 'a' * 64, '!debug'])
        self.assertIn('-DCMAKE_SKIP_INSTALL_RPATH=ON', rendered)
        self.assertNotIn('@VERSION@', rendered)

    def test_recipe_builds_distribution_without_full_suite(self):
        rendered = arch.recipe((ROOT / 'scripts/arch/PKGBUILD.in').read_text(), '2.3.4', '1', '6.12.0', 'a' * 64)
        self.assertIn("source=('qmarkdown-2.3.4.tar.gz')", rendered)
        self.assertIn('-DBUILD_TESTING=OFF', rendered)
        self.assertIn('-DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING=ON', rendered)
        self.assertNotIn('check()', rendered)
        self.assertNotIn('ctest', rendered)

    def test_payload_accepts_cmake_installed_private_metadata_only(self):
        for name in ('/usr/lib/libQMarkdown.so.1.1.0',
                '/usr/lib/qt6/qml/QMarkdown/MarkdownView.qml',
                '/usr/lib/qt6/qml/QMarkdown/Private/qmldir',
                '/usr/lib/qt6/qml/QMarkdown/Private/private.qmltypes',
                '/usr/lib/qt6/qml/QMarkdown/private/Layout.js',
                '/usr/lib/cmake/QMarkdown/QMarkdownConfig.cmake',
                '/usr/share/licenses/qmarkdown/cmark/COPYING'):
            self.assertTrue(verify_arch.allowed_payload(Path(name)), name)
        for name in ('/usr/bin/test', '/usr/lib/libQt6Core.so',
                '/usr/lib/tests/test', '/usr/share/qmarkdown/installed.json',
                '/usr/lib/qt6/qml/QMarkdown/Private/test', '/usr/lib/libQMarkdown.a',
                '/usr/lib/qt6/qml/QMarkdown/private/test.qml',
                '/usr/lib/qt6/qml/QMarkdown/private/test.js',
                '/usr/lib/qt6/qml/QMarkdown/private/document.h'):
            self.assertFalse(verify_arch.allowed_payload(Path(name)), name)

    def test_invalid_inputs_are_rejected_before_shell_execution(self):
        for version, revision, qt, checksum in (
                ('1.0;id', '1', '6.8.0', 'a' * 64), ('1.0.0', '0', '6.8.0', 'a' * 64),
                ('1.0.0', '1', '6.8', 'a' * 64), ('1.0.0', '1', '6.8.0', 'SKIP')):
            with self.assertRaises(ValueError):
                arch.recipe('', version, revision, qt, checksum)


if __name__ == '__main__':
    unittest.main()
