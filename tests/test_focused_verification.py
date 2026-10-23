"""Profile and immutable-hook regression tests using isolated Git fixtures."""
import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


scope = load('scope066', 'scripts/ci-toolchain/check-scope.py')
hooks = load('hooks066', 'scripts/git-hooks.py')


class Profiles(unittest.TestCase):
    def test_profiles_and_additive_features(self):
        cases = [(['docs/a.md', 'specs/065/experiment.cpp'], 'light'),
                 (['src/QMarkdown/parser.cpp'], 'core'),
                 (['tests/test_ci_runner.py'], 'full'),
                 (['CMakeLists.txt'], 'full'), (['src/QMarkdown/CMakeLists.txt'], 'full'),
                 (['tests/CMakeLists.txt'], 'full'), (['specs/066/CMakeLists.txt'], 'light'), (['unknown'], 'full'),
                 (['docs/release-notes-1.17.1.md'], 'full'),
                 (['docs/a.md', 'src/QMarkdown/math.cpp'], 'core'),
                 (['tests/benchmark.cpp'], 'full'), (['tests/inlinebenchmark.cpp'], 'full'),
                 (['tests/installed-consumer/main.cpp'], 'full')]
        for paths, expected in cases:
            with self.subTest(paths=paths):
                self.assertEqual(scope.classify(paths)['profile'], expected)
        self.assertEqual(scope.classify(['docs/a.md'], True)['profile'], 'full')
        result = scope.classify(['src/QMarkdown/providers.cpp', 'tests/math.cpp'])
        self.assertTrue(result['syntax'])
        self.assertTrue(result['math'])

    def test_push_refs_skip_deletions_deduplicate_and_include_non_head(self):
        zero = '0' * 40
        lines = [f'refs/heads/a abc refs/heads/a {zero}',
                 'refs/heads/b def refs/heads/b old',
                 'refs/heads/c abc refs/heads/c old',
                 f'(delete) {zero} refs/heads/d old']
        self.assertEqual(hooks.push_commits(lines), ['abc', 'def'])


class Snapshots(unittest.TestCase):
    def test_staged_snapshot_preserves_timestamps_and_excludes_working_edits(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            repo = root / 'repo'
            repo.mkdir()
            subprocess.run(['git', 'init', '-q', repo], check=True)
            (repo / 'fixture').write_text('staged')
            subprocess.run(['git', '-C', repo, 'add', '.'], check=True)
            tree = subprocess.check_output(['git', '-C', repo, 'write-tree']).decode().strip()
            (repo / 'fixture').write_text('unstaged')
            target = root / 'snapshot'
            with patch.object(hooks, 'ROOT', repo):
                hooks.export_tree(tree, target)
                first = (target / 'fixture').stat().st_mtime_ns
                hooks.export_tree(tree, target)
                self.assertEqual((target / 'fixture').read_text(), 'staged')
                self.assertEqual((target / 'fixture').stat().st_mtime_ns, first)
                (target / 'obsolete').write_text('old')
                hooks.export_tree(tree, target)
                self.assertFalse((target / 'obsolete').exists())
            self.assertEqual((repo / 'fixture').read_text(), 'unstaged')

    def test_missing_qt_fails_without_download(self):
        with tempfile.TemporaryDirectory() as temporary:
            cache = Path(temporary)
            source = cache / 'source/scripts/ci-toolchain'
            source.mkdir(parents=True)
            (source / 'qt_versions.py').write_text("def load_versions(): return {'development': '6.12.0'}")
            with patch.object(hooks, 'export_tree'), patch.object(hooks, 'run'), patch.object(hooks.shutil, 'which', return_value=None):
                with self.assertRaisesRegex(RuntimeError, 'Development Qt missing'):
                    hooks.verify('head', cache, True)

    def test_rename_and_deletion_cover_both_paths(self):
        with tempfile.TemporaryDirectory() as temporary:
            repo = Path(temporary)
            subprocess.run(['git', 'init', '-q', repo], check=True)
            (repo / 'src').mkdir()
            (repo / 'src/a.cpp').write_text('fixture')
            subprocess.run(['git', '-C', repo, 'add', '.'], check=True)
            subprocess.run(['git', '-C', repo, '-c', 'user.name=Test', '-c',
                            'user.email=test@example.invalid', 'commit', '-qm', 'initial'], check=True)
            base = subprocess.check_output(['git', '-C', repo, 'rev-parse', 'HEAD']).decode().strip()
            (repo / 'specs').mkdir()
            (repo / 'src/a.cpp').rename(repo / 'specs/a.cpp')
            subprocess.run(['git', '-C', repo, 'add', '-A'], check=True)
            tree = subprocess.check_output(['git', '-C', repo, 'write-tree']).decode().strip()
            # scope uses subprocess cwd; isolate by wrapping the observed invocation.
            original = scope.subprocess.check_output
            with patch.object(scope.subprocess, 'check_output', side_effect=lambda command: original(command, cwd=repo)):
                names = scope.changed_paths(base, tree)
            self.assertEqual(set(names), {'src/a.cpp', 'specs/a.cpp'})
            self.assertEqual(scope.classify(names)['profile'], 'core')

    def test_missing_base_selects_build_and_duplicate_refs_run_once(self):
        with tempfile.TemporaryDirectory() as temporary:
            with patch.object(hooks, 'ROOT', Path(temporary)), patch.object(hooks, 'verify') as verify:
                def git(*args):
                    if args[0] == 'merge-base':
                        raise subprocess.CalledProcessError(1, 'git')
                    return b'head\n'
                with patch.object(hooks, 'git', side_effect=git), patch('sys.argv', ['hooks', 'check-push']):
                    hooks.main()
                self.assertTrue(verify.call_args.args[2])

    def test_concurrent_hook_fails_before_snapshot_export(self):
        import fcntl
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            cache = root / 'build-ci/hooks'
            cache.mkdir(parents=True)
            with (cache / 'lock').open('w') as lock:
                fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
                with patch.object(hooks, 'ROOT', root), patch('sys.argv', ['hooks', 'pre-commit']), patch.object(hooks, 'verify') as verify:
                    with self.assertRaisesRegex(RuntimeError, 'Another hook'):
                        hooks.main()
                    verify.assert_not_called()

    def test_unrelated_hook_configuration_is_not_replaced(self):
        from types import SimpleNamespace
        with patch.object(hooks.subprocess, 'run', return_value=SimpleNamespace(stdout='/other/hooks\n')) as process:
            with self.assertRaisesRegex(RuntimeError, 'Existing core.hooksPath'):
                hooks.install()
            self.assertEqual(process.call_count, 1)


class EvidenceSyntax(unittest.TestCase):
    def test_malformed_compressed_jsonl_is_rejected(self):
        import gzip
        checks = load('checks066', 'scripts/check.py')
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with gzip.open(root / 'evidence.jsonl.gz', 'wt') as output:
                output.write('{"valid": true}\n{broken}\n')
            with patch.object(checks, 'ROOT', root), patch.object(checks.subprocess, 'run'), patch('sys.argv', ['check', '--all-files']):
                import json
                with self.assertRaises(json.JSONDecodeError):
                    checks.main()
