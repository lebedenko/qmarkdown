"""Small, complete checkout documentation for offline release regressions."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
CURRENT = ('README.md', 'specs/README.md', 'specs/overview.md', 'specs/roadmap.md',
           'docs/gfm.md', 'docs/release-install.md', 'docs/arch-install.md')


def write_docs(root, version='1.6.0'):
    files = (*CURRENT, 'CHANGELOG.md', 'docs/release-notes-1.7.0.md')
    for name in files:
        content = (ROOT / name).read_text().replace('1.7.0', version).replace('QML 1.7', 'QML ' + version.rsplit('.', 1)[0]).replace('import QMarkdown 1.7', 'import QMarkdown ' + version.rsplit('.', 1)[0]).replace('find_package(QMarkdown 1.7 ', 'find_package(QMarkdown ' + version.rsplit('.', 1)[0] + ' ').replace('accepts compatible 1.0–1.7 requests', 'accepts compatible 1.0–' + version.rsplit('.', 1)[0] + ' requests')
        target = root / name.replace('1.7.0', version)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(content)
    (root / 'CMakeLists.txt').write_text(f'project(QMarkdown VERSION {version})')
    (root / 'src/QMarkdown').mkdir(parents=True, exist_ok=True)
    (root / 'src/QMarkdown/CMakeLists.txt').write_text('VERSION ' + version.rsplit('.', 1)[0])
    # Link targets need only exist; feature records are outside release summaries.
    for name in files:
        path = root / name.replace('1.7.0', version)
        for link in re.findall(r'\]\(([^)]+)\)', path.read_text()):
            link = link.split('#')[0]
            if not link or '://' in link or link.startswith(('/', 'mailto:')):
                continue
            target = path.parent / link
            target.parent.mkdir(parents=True, exist_ok=True)
            if not target.exists():
                target.touch()
