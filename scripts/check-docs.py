#!/usr/bin/env python3
"""Validate current release wording, versions and local documentation links."""
import argparse
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
CURRENT = ('README.md', 'specs/README.md', 'specs/overview.md', 'specs/roadmap.md', 'docs/gfm.md', 'docs/release-install.md', 'docs/arch-install.md')


START = '<!-- current-release:start -->'
END = '<!-- current-release:end -->'
RELEASE_URL = 'https://github.com/lebedenko/qmarkdown/releases'


def current_section(content, name):
    if content.count(START) != 1 or content.count(END) != 1:
        raise ValueError(f'Missing or duplicate current-release boundaries: {name}')
    if content.index(START) > content.index(END):
        raise ValueError(f'Reversed current-release boundaries: {name}')
    return content.split(START, 1)[1].split(END, 1)[0]


def validate_current(current, name, expected):
    def fail(reason):
        raise ValueError(f'{name}: {reason}')

    prose = re.sub(r'\]\([^)]+\)', ']', current)
    if re.search(r'\b(candidate|preparation|pending|unpublished)\b', prose, re.I):
        fail('Stale current-release wording')
    if re.search(r'publication.{0,60}(separate|authorization|excluded)|'
                 r'(future|forthcoming|planned).{0,30}(download|release asset)|'
                 r'(downloads?|assets?).{0,30}(will be|not yet)', ' '.join(prose.split()), re.I):
        fail('Stale publication disclaimer or future-download wording')
    expected_qml = expected.rsplit('.', 1)[0]
    if expected not in current:
        fail(f'Current version missing: {expected}')
    # Historical references and compatibility ranges are allowed; assertions about
    # the current package/module and concrete download bytes must agree.
    for match in re.finditer(r'\b(?:QMarkdown|package|published|released|current)\s+[*`]*(\d+\.\d+\.\d+)', current, re.I):
        line_end = current.find('\n', match.end())
        line = current[current.rfind('\n', 0, match.start()) + 1:
                       line_end if line_end >= 0 else len(current)]
        if match[1] != expected and not re.search(r'\bhistorical\b', line, re.I):
            fail(f'Stale current package version: {match[1]} (expected {expected})')
    for match in re.finditer(r'\bQML\s+[*`]*(\d+\.\d+)\b', current):
        value = match[1]
        prefix = current[current.rfind('\n', 0, match.start()) + 1:match.start()]
        if value != expected_qml and not re.search(r'\bhistorical\b', prefix, re.I):
            fail(f'Stale current QML version: {value} (expected {expected_qml})')
    for upper in re.findall(r'accepts compatible \d+\.\d+[–-](\d+\.\d+) requests', current):
        if upper != expected_qml:
            fail(f'Stale compatibility upper bound: {upper} (expected {expected_qml})')
    if name in ('docs/gfm.md', 'docs/release-install.md', 'docs/arch-install.md'):
        for value in re.findall(r'(?:import QMarkdown |find_package\(QMarkdown )(\d+\.\d+(?:\.\d+)?)', current):
            if value not in (expected_qml, expected):
                fail(f'Stale installation import/package request: {value} (expected {expected_qml})')
    for tag in re.findall(r'/releases/(?:tag|download)/v([^/\s)]+)', current):
        if tag != expected:
            fail(f'Wrong release URL version: {tag} (expected {expected})')
    for asset in re.findall(r'qmarkdown-[^\s`/()]+', current):
        if not re.fullmatch(r'qmarkdown-' + re.escape(expected) +
                            r'(?:\.tar\.gz(?:\.sha256)?|-1-x86_64\.pkg\.tar\.zst(?:\.sha256)?)?', asset):
            fail(f'Obsolete or incorrect release asset: {asset}')
    for url in re.findall(r'https?://[^\s)`]+', current):
        if '/releases/' in url and not url.startswith(RELEASE_URL + '/'):
            fail(f'Wrong release repository URL: {url}')
    if name != 'docs/gfm.md' and RELEASE_URL + '/tag/v' + expected not in current:
        fail(f'Current release download link missing: {expected}')


def validate(root=ROOT, release=False):
    version = re.search(r'project\(QMarkdown VERSION (\d+\.\d+\.\d+)', (root / 'CMakeLists.txt').read_text())[1]
    qml_version = re.search(r'\bVERSION (\d+\.\d+)\b', (root / 'src/QMarkdown/CMakeLists.txt').read_text())[1]
    if qml_version != version.rsplit('.', 1)[0]:
        raise ValueError('Package and QML module versions differ')
    published = re.search(r'\bQMarkdown\s+(\d+\.\d+\.\d+)',
                          current_section((root / 'README.md').read_text(), 'README.md'))
    if not published:
        raise ValueError('README.md: Current package version missing')
    expected = version if release else published[1]
    if tuple(map(int, expected.split('.'))) > tuple(map(int, version.split('.'))):
        raise ValueError('README.md: Published summary is newer than the package')
    for name in (*CURRENT, f'docs/release-notes-{version}.md'):
        content = (root / name).read_text()
        validate_current(current_section(content, name), name,
                         version if name == f'docs/release-notes-{version}.md' else expected)
    section = (root / 'CHANGELOG.md').read_text().split('\n## ', 2)[1]
    if not re.match(re.escape(version) + r' — \d{4}-\d{2}-\d{2}\n', section):
        raise ValueError('Current changelog must have a finalized release date')
    if re.search(r'\b(candidate|preparation|pending)\b', section, re.I):
        raise ValueError('Stale current changelog wording')
    files = [root / 'README.md', root / 'CHANGELOG.md', *sorted((root / 'docs').glob('*.md')),
             *(root / name for name in CURRENT[1:])]
    for path in files:
        content = re.sub(r'```.*?```', '', path.read_text(), flags=re.S)
        for target in re.findall(r'\]\(([^)]+)\)', content):
            target = target.split('#')[0]
            if not target or '://' in target or target.startswith(('mailto:', '/')):
                continue
            if not (path.parent / target).exists():
                raise ValueError(f'Broken local link in {path.relative_to(root)}: {target}')
    print(f'PASS: current {version} documentation and local links')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--release', action='store_true', help='Require documentation for the package being published')
    args = parser.parse_args()
    try:
        validate(release=args.release)
    except (ValueError, OSError) as error:
        parser.exit(1, f'Documentation validation failed: {error}\n')
