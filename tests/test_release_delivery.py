"""Offline distribution and release-gate regressions; never publish a release."""
import importlib.util
import re
from pathlib import Path
import subprocess
import tarfile
import tempfile
import unittest
from unittest.mock import patch

from release_docs_fixture import write_docs

ROOT = Path(__file__).resolve().parents[1]


def module(name, path):
    spec = importlib.util.spec_from_file_location(name, ROOT / path)
    loaded = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(loaded)
    return loaded


source = module('source', 'scripts/build-source-archive.py')
eligible = module('eligible', 'scripts/release/eligibility.py')
docs = module('docs', 'scripts/check-docs.py')
scope = module('scope', 'scripts/ci-toolchain/check-scope.py')


class SourceArchiveTest(unittest.TestCase):
    def test_arch_upload_patterns_exclude_namcap_logs(self):
        workflow = (ROOT / '.github/workflows/release.yml').read_text()
        configuration = workflow.split('name: arch-distribution\n', 1)[1].split('if-no-files-found:', 1)[0]
        patterns = re.findall(r'\$\{\{ runner\.temp \}\}/qmarkdown-arch/(\S+)', configuration)
        self.assertTrue(patterns)
        package = 'qmarkdown-1.8.0-1-x86_64.pkg.tar.zst'
        expected = {package, package + '.sha256'}
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in (*expected, package + '.namcap.log'):
                (root / name).write_text('fixture')
            selected = {path.name for pattern in patterns for path in root.glob(pattern)}
        self.assertEqual(selected, expected)

    def test_archive_is_deterministic_library_only_with_licenses(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            first = source.build_archive(ROOT, root / 'first')
            second = source.build_archive(ROOT, root / 'second')
            self.assertEqual(first.read_bytes(), second.read_bytes())
            with tarfile.open(first) as archive:
                names = archive.getnames()
                prefix = 'qmarkdown-' + source.project_version((ROOT / 'CMakeLists.txt').read_text()) + '/'
                for name in ('CMakeLists.txt', 'LICENSE', 'INSTALL.md', 'third_party/cmark/COPYING',
                             'third_party/cmark/PROVENANCE.md', 'src/QMarkdown/CMakeLists.txt',
                             'third_party/microtex/LICENSE', 'third_party/microtex/PROVENANCE.md',
                             'third_party/microtex/res/fonts/licences/OFL.txt', 'third_party/tinyxml2/LICENSE.txt'):
                    self.assertIn(prefix + name, names)
                for name in ('LICENSE', 'third_party/cmark/COPYING', 'third_party/cmark/PROVENANCE.md'):
                    self.assertEqual(archive.extractfile(prefix + name).read(), (ROOT / name).read_bytes())
                for member in archive.getmembers():
                    self.assertEqual((member.uid, member.gid, member.mtime, member.mode), (0, 0, 0, 0o644))
                    self.assertNotIn(member.name.split('/')[1],
                                     {'tests', 'examples', 'scripts', 'specs', '.github', 'benchmarks'})
            with self.assertRaisesRegex(ValueError, 'already exists'):
                source.build_archive(ROOT, root / 'first')

    def test_selected_commit_excludes_working_edits(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, content in {
                'CMakeLists.txt': 'project(QMarkdown VERSION 2.3.4 LANGUAGES CXX)',
                'LICENSE': 'license', 'third_party/cmark/COPYING': 'cmark license',
                'third_party/cmark/PROVENANCE.md': 'provenance',
                'docs/source-install.md': 'standalone instructions',
            }.items():
                path = root / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(content)
            allowlist = root / 'scripts/release/source-files.txt'
            allowlist.parent.mkdir(parents=True)
            allowlist.write_text('CMakeLists.txt\nLICENSE\nthird_party/cmark/COPYING\nthird_party/cmark/PROVENANCE.md\ndocs/source-install.md\n')
            subprocess.run(['git', 'init', '-q', root], check=True)
            subprocess.run(['git', '-C', root, 'add', '.'], check=True)
            subprocess.run(['git', '-C', root, '-c', 'user.name=Test', '-c',
                            'user.email=test@example.invalid', 'commit', '-qm', 'fixture'], check=True)
            (root / 'CMakeLists.txt').write_text('project(QMarkdown VERSION 9.9.9)')
            archive = source.build_archive(root, root / 'output', 'HEAD')
            self.assertEqual(archive.name, 'qmarkdown-2.3.4.tar.gz')


class EligibilityTest(unittest.TestCase):
    def setUp(self):
        self.pr = {'number': 42, 'merged': True, 'base': {'ref': 'main'},
                   'head': {'sha': 'head'}, 'merge_commit_sha': 'merged'}
        self.run = {'id': 7, 'event': 'pull_request', 'head_sha': 'head', 'status': 'completed',
                    'conclusion': 'success', 'path': '.github/workflows/linux-qt.yml',
                    'pull_requests': [{'number': 42}]}
        self.versions = {'minimum': '6.8.0', 'development': '6.12.0'}
        self.gate = {'full': True, 'pr': 42, 'head': 'head', 'tree': 'tree',
                     'qt': self.versions, 'version': '1.6.0'}

    def validate(self, **kwargs):
        args = dict(pr=self.pr, run=self.run, gate=self.gate, main='merged',
                    merged_tree='tree', versions=self.versions, version='1.6.0')
        args.update(kwargs)
        return eligible.validate(**args)

    def test_accepts_tested_tree_after_squash_merge(self):
        self.assertEqual(self.validate(), 'merged')
        self.assertEqual(self.validate(tag_commit='merged'), 'merged')
        self.assertEqual(self.validate(run={**self.run, 'pull_requests': []}), 'merged')

    def test_failed_stale_wrong_or_unfinished_ci_is_rejected(self):
        for field, value in (('conclusion', 'failure'), ('status', 'in_progress'),
                             ('head_sha', 'previous'), ('event', 'push'),
                             ('path', '.github/workflows/release.yml')):
            run = {**self.run, field: value}
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(run=run)

    def test_tree_qt_version_and_full_gate_must_match(self):
        for field, value in (('tree', 'other'), ('qt', {'minimum': '6.8.0', 'development': '6.13.0'}),
                             ('pr', 43), ('version', '1.5.0'), ('full', False), ('head', 'previous')):
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(gate={**self.gate, field: value})

    def test_moved_main_different_tag_and_unmerged_pr_are_rejected(self):
        for kwargs in ({'main': 'moved'}, {'tag_commit': 'other'},
                       {'pr': {**self.pr, 'merged': False}},
                       {'pr': {**self.pr, 'base': {'ref': 'other'}}}):
            with self.subTest(kwargs=kwargs), self.assertRaises(ValueError):
                self.validate(**kwargs)

    def test_latest_failed_run_cannot_fall_back_to_previous_success(self):
        responses = [self.pr, {'object': {'sha': 'merged'}}, {'tree': {'sha': 'tree'}},
                     {'workflow_runs': [self.run, {**self.run, 'id': 8, 'conclusion': 'failure'}]}]
        with patch.object(eligible, 'gh_json', side_effect=responses):
            with self.assertRaisesRegex(ValueError, 'Latest'):
                eligible.eligibility('owner/repo', 42)

    def test_missing_gate_artifacts_are_rejected(self):
        responses = [self.pr, {'object': {'sha': 'merged'}}, {'tree': {'sha': 'tree'}},
                     {'workflow_runs': [self.run]}, {'artifacts': []}]
        with patch.object(eligible, 'gh_json', side_effect=responses):
            with self.assertRaisesRegex(ValueError, 'Missing'):
                eligible.eligibility('owner/repo', 42)


class DocumentationTest(unittest.TestCase):
    def test_current_docs_pass_with_historical_candidates_retained(self):
        docs.validate(ROOT)

    def test_release_docs_pass_and_historical_records_remain_allowed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            write_docs(root)
            docs.validate(root, release=True)

    def test_invalid_complete_current_sections_are_rejected(self):
        cases = (
            ('README.md', '\n## Downloads\nPublished package 1.5.0 / QML 1.5 is current.'),
            ('docs/release-install.md', '\nsha256sum -c qmarkdown-1.5.0.tar.gz.sha256'),
            ('docs/arch-install.md', '\nqmarkdown-1.6.0-2-x86_64.pkg.tar.zst'),
            ('docs/arch-install.md', '\nimport QMarkdown 1.5'),
            ('docs/arch-install.md', '\nfind_package(QMarkdown 1.5 CONFIG REQUIRED)'),
            ('README.md', '\nPublication requires separate authorization.'),
            ('README.md', '\nPublication requires separate\nauthorization.'),
            ('README.md', '\nPublication is a separate delivery action.'),
            ('docs/release-install.md', '\nFuture release assets are qmarkdown-X.Y.Z.tar.gz.'),
            ('docs/release-install.md', '\nDownloads will be available after publication.'),
            ('README.md', '\nhttps://github.com/lebedenko/qmarkdown/releases/tag/v1.5.0'),
            ('README.md', '\nhttps://github.com/other/project/releases/tag/v1.6.0'),
            ('README.md', '\nCurrent package 1.6.0 / QML 1.5'),
            ('specs/overview.md', '\nPackage 1.6.0 accepts compatible 1.0–1.5 requests.'),
            ('README.md', '\nPublished 1.5.0 is available.'),
        )
        for name, bad in cases:
            with self.subTest(name=name, bad=bad), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                write_docs(root)
                path = root / name
                path.write_text(path.read_text().replace(docs.END, bad + '\n' + docs.END))
                with self.assertRaisesRegex(ValueError, name):
                    docs.validate(root, release=True)

    def test_stale_current_release_wording_is_rejected(self):
        for wording in ('release candidate', 'release preparation', 'pending-publication', 'unpublished'):
            with self.subTest(wording=wording), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                write_docs(root)
                path = root / 'README.md'
                path.write_text(path.read_text().replace(docs.END, wording + '\n' + docs.END))
                with self.assertRaisesRegex(ValueError, 'Stale'):
                    docs.validate(root)

    def test_development_can_advance_but_release_requires_published_version(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            write_docs(root)
            (root / 'CMakeLists.txt').write_text('project(QMarkdown VERSION 1.7.0)')
            (root / 'src/QMarkdown/CMakeLists.txt').write_text('VERSION 1.7')
            path = root / 'README.md'
            path.write_text(path.read_text() + '\n## Development after the release tag\nUnpublished custom renderers; publication requires authorization.\n')
            docs.validate(root)
            with self.assertRaisesRegex(ValueError, 'README.md'):
                docs.validate(root, release=True)

    def test_missing_boundaries_fail_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            write_docs(root)
            path = root / 'README.md'
            path.write_text(path.read_text().replace(docs.END, ''))
            with self.assertRaisesRegex(ValueError, 'boundaries.*README.md'):
                docs.validate(root, release=True)

    def test_only_prose_changes_get_lightweight_jobs(self):
        self.assertFalse(scope.full_verification(['README.md', 'docs/gfm.md', 'specs/039/verification.md']))
        for names in (['CHANGELOG.md'], ['docs/release-notes-1.7.0.md'], ['src/QMarkdown/MarkdownView.qml'],
                      ['scripts/ci-toolchain/qt-versions.json'], ['CMakeLists.txt'], ['LICENSE']):
            self.assertTrue(scope.full_verification(names))


if __name__ == '__main__':
    unittest.main()
