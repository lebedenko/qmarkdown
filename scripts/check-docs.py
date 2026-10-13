#!/usr/bin/env python3
"""Validate current release wording, versions and local documentation links."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
CURRENT = ('README.md', 'specs/README.md', 'specs/overview.md', 'specs/roadmap.md', 'docs/gfm.md', 'docs/release-install.md', 'docs/arch-install.md')


def validate(root=ROOT):
    version = re.search(r'project\(QMarkdown VERSION (\d+\.\d+\.\d+)', (root / 'CMakeLists.txt').read_text())[1]
    qml_version = re.search(r'\bVERSION (\d+\.\d+)\b', (root / 'src/QMarkdown/CMakeLists.txt').read_text())[1]
    if qml_version != version.rsplit('.', 1)[0]:
        raise ValueError('Package and QML module versions differ')
    for name in (*CURRENT, f'docs/release-notes-{version}.md'):
        content = (root / name).read_text()
        # Validate the current summary; detailed historical records remain intact.
        current = (content.split('Historical records below', 1)[0] if name == 'specs/README.md'
                   else content.split('\n## ', 1)[0] if name in CURRENT else content)
        current = re.sub(r'\]\([^)]+\)', ']', current)
        if re.search(r'\b(candidate|preparation|pending|unpublished)\b', current, re.I):
            raise ValueError(f'Stale current-release wording: {name}')
        if version not in current:
            raise ValueError(f'Current version missing: {name}')
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
    validate()
