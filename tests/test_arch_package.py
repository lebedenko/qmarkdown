"""Validate generated Arch recipe inputs without provisioning containers."""
import importlib.util
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile
import unittest
from unittest.mock import patch

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
            'qt6-base>=6.12.1', 'qt6-declarative>=6.12.1', 'a' * 64, '!debug'])
        self.assertIn('-DCMAKE_SKIP_INSTALL_RPATH=ON', rendered)
        self.assertNotIn('@VERSION@', rendered)

    def test_archive_uses_selected_commit_not_working_tree(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, work = root / 'source', root / 'artifacts'
            source.mkdir()
            subprocess.run(['git', 'init', '-q', source], check=True)
            cmake = source / 'CMakeLists.txt'
            committed = 'project(QMarkdown VERSION 2.3.4 LANGUAGES CXX)\n'
            cmake.write_text(committed)
            subprocess.run(['git', '-C', str(source), 'add', 'CMakeLists.txt'], check=True)
            subprocess.run(['git', '-C', str(source), '-c', 'user.name=Test', '-c',
                'user.email=test@example.invalid', 'commit', '-qm', 'fixture'], check=True)
            cmake.write_text('project(QMarkdown VERSION 9.9.9 LANGUAGES CXX)\n')
            (source / 'untracked.txt').write_text('excluded')
            real_run = subprocess.run

            def run_without_docker(command, **kwargs):
                if command[0] == 'docker':
                    raise RuntimeError('Provisioning boundary')
                return real_run(command, **kwargs)

            with patch.object(arch, 'SOURCE', source), patch.object(sys, 'argv',
                    ['runner', '--work-dir', str(work)]), patch.object(arch.subprocess,
                    'run', side_effect=run_without_docker):
                with self.assertRaisesRegex(RuntimeError, 'Provisioning boundary'):
                    arch.main()
            with tarfile.open(work / 'qmarkdown-source.tar.gz') as archive:
                self.assertEqual(archive.getnames(), ['qmarkdown', 'qmarkdown/CMakeLists.txt'])
                self.assertEqual(archive.extractfile('qmarkdown/CMakeLists.txt').read().decode(), committed)

    def test_payload_accepts_cmake_installed_private_metadata_only(self):
        for name in ('/usr/lib/libQMarkdown.so.1.1.0',
                '/usr/lib/qt6/qml/QMarkdown/MarkdownView.qml',
                '/usr/lib/qt6/qml/QMarkdown/Private/qmldir',
                '/usr/lib/qt6/qml/QMarkdown/Private/private.qmltypes',
                '/usr/lib/cmake/QMarkdown/QMarkdownConfig.cmake',
                '/usr/share/licenses/qmarkdown/cmark/COPYING'):
            self.assertTrue(verify_arch.allowed_payload(Path(name)), name)
        for name in ('/usr/bin/test', '/usr/lib/libQt6Core.so',
                '/usr/lib/tests/test', '/usr/share/qmarkdown/installed.json',
                '/usr/lib/qt6/qml/QMarkdown/Private/test', '/usr/lib/libQMarkdown.a'):
            self.assertFalse(verify_arch.allowed_payload(Path(name)), name)

    def test_invalid_inputs_are_rejected_before_shell_execution(self):
        for version, revision, qt, checksum in (
                ('1.0;id', '1', '6.8.0', 'a' * 64), ('1.0.0', '0', '6.8.0', 'a' * 64),
                ('1.0.0', '1', '6.8', 'a' * 64), ('1.0.0', '1', '6.8.0', 'SKIP')):
            with self.assertRaises(ValueError):
                arch.recipe('', version, revision, qt, checksum)


if __name__ == '__main__':
    unittest.main()
