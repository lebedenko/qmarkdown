#!/usr/bin/env python3
"""Create isolated diagnostic sources/builds; root must not exist."""
import argparse
import hashlib
import json
import re
from pathlib import Path
import shutil
import shlex
import subprocess

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[2]


def run(*args, **kwargs):
    subprocess.run(args, check=True, **kwargs)


def replace(path, before, after):
    text = path.read_text()
    if text.count(before) != 1:
        raise RuntimeError(f'Expected exactly one instrumentation anchor in {path}: {before}')
    path.write_text(text.replace(before, after))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--viewer', type=Path, required=True)
    p.add_argument('--root', type=Path, required=True)
    a = p.parse_args()
    a.root = a.root.resolve()
    if a.root.parent != Path('/tmp'):
        p.error('Use a fresh direct child of /tmp')
    a.root.mkdir(exist_ok=False)
    manifest = {'viewer_revision': subprocess.check_output(['git', '-C', str(a.viewer), 'rev-parse', 'HEAD'], text=True).strip(),
                'viewer_status': subprocess.check_output(['git', '-C', str(a.viewer), 'status', '--short'], text=True),
                'qt': subprocess.check_output(['qmake6', '-query', 'QT_VERSION'], text=True).strip()}
    manifest['library_revisions'] = {name: subprocess.check_output(
        ['git', '-C', str(REPO), 'rev-parse', rev], text=True).strip()
        for name, rev in [('released', 'b22d519'), ('fixed', 'e399594')]}
    (a.root / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    snapshot = a.root / 'viewer-source'
    shutil.copytree(a.viewer, snapshot, ignore=shutil.ignore_patterns(
        '.git', 'build*', '.env', '.env.*', '.serena', '.aws', '.codex', '.agents'))
    for label, revision in [('released', 'b22d519'), ('fixed', 'e399594')]:
        base = a.root / label
        source = base / 'qmarkdown'
        source.mkdir(parents=True)
        archive = base / 'source.tar'
        run('git', '-C', str(REPO), 'archive', '--format=tar', '-o', str(archive), revision)
        run('tar', '-xf', str(archive), '-C', str(source))
        prefix = base / 'prefix'
        run('cmake', '-S', str(source), '-B', str(base / 'library-build'), '-G', 'Ninja',
            '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_SHARED_LIBS=ON', '-DBUILD_TESTING=ON',
            '-DQMARKDOWN_WITH_MATH=ON', '-DQMARKDOWN_WITH_KSYNTAXHIGHLIGHTING=ON',
            f'-DCMAKE_INSTALL_PREFIX={prefix}')
        run('cmake', '--build', str(base / 'library-build'), '--target', 'QMarkdownPlugin',
            'qmarkdown-selection-test', 'qmarkdown-codeblocks-test', '--parallel', '2')
        run('cmake', '--install', str(base / 'library-build'))
        viewer = base / 'viewer'
        shutil.copytree(snapshot, viewer)
        hashes = {str(f.relative_to(viewer)): hashlib.sha256(f.read_bytes()).hexdigest()
                  for f in sorted(viewer.rglob('*')) if f.is_file()}
        (base / 'viewer-snapshot.json').write_text(json.dumps(hashes, indent=2) + '\n')
        shutil.copy(HERE / 'trace.h', viewer / 'src/diagnostictrace.h')
        replace(viewer / 'src/clipboard.h', '  Q_INVOKABLE static void copyText', '''  Q_INVOKABLE static void traceCopy(const QString &kind, const QString &text, int start, int end, const QString &selected);
  Q_INVOKABLE static void copyText''')
        replace(viewer / 'src/clipboard.cpp', '#include "clipboard.h"', '#include "clipboard.h"\n#include "diagnostictrace.h"')
        replace(viewer / 'src/clipboard.cpp', 'void Clipboard::copyText(const QString& text) { QGuiApplication::clipboard()->setText(text); }', '''void Clipboard::traceCopy(const QString &kind, const QString &text, int start, int end, const QString &selected) {
  ++Diagnostic::action;
  Diagnostic::record(kind, text, {{"selectionStart", start}, {"selectionEnd", end}});
  Diagnostic::record("selectedText", selected);
}
void Clipboard::copyText(const QString& text) {
  Diagnostic::record("writer", text);
  QGuiApplication::clipboard()->setText(text);
  Diagnostic::record("readback", QGuiApplication::clipboard()->text());
}''')
        for signal, kind in [('CopyRequested', 'partial'), ('CodeBlockCopyRequested', 'whole-code')]:
            path = viewer / 'qml/ReaderPane.qml'
            pattern = rf'on{signal}:\s*function\s*\(text\)\s*{{\s*root\.copyRequested\(text\);?\s*}}'
            replacement = f'on{signal}: function(text) {{ Clipboard.traceCopy("{kind}", text, document.selectionStart, document.selectionEnd, document.selectedText); root.copyRequested(text) }}'
            content, count = re.subn(pattern, replacement, path.read_text())
            if count != 1:
                raise RuntimeError(f'Expected one public copy handler: {signal}')
            path.write_text(content)
        replace(viewer / 'src/main.cpp', '#include <QCommandLineParser>', '#include <QCommandLineParser>\n#include "diagnostictrace.h"')
        replace(viewer / 'src/main.cpp', '  engine.loadFromModule', '''  QFile diagnosticQml(":/qt/qml/QMarkdown/MarkdownView.qml");
  if (!diagnosticQml.open(QIODevice::ReadOnly)) qFatal("Cannot read provider QML resource");
  Diagnostic::record("qml-resource", QString::fromUtf8(diagnosticQml.readAll()));
  Diagnostic::record("startup", QString(), {{"qt", qVersion()}, {"backend", QGuiApplication::platformName()}, {"importPaths", QJsonArray::fromStringList(engine.importPathList())}});
  engine.loadFromModule''')
        build = base / 'app-build'
        run('cmake', '-S', str(viewer), '-B', str(build), '-G', 'Ninja', '-DBUILD_TESTING=OFF',
            '-DCMAKE_BUILD_TYPE=Release', f'-DCMAKE_PREFIX_PATH={prefix}')
        run('cmake', '--build', str(build), '--parallel', '2')
        config = base / 'config/md-viewer/config.toml'
        config.parent.mkdir(parents=True)
        config.write_text('# Isolated native diagnostic preferences\n')
        env = ' '.join(shlex.quote(value) for value in [f'XDG_CONFIG_HOME={base}/config', f'MDV_TRACE={base}/trace.jsonl'])
        (base / 'launch.sh').write_text(f'#!/bin/sh\nexec env {env} {shlex.quote(str(build / "mdv"))} "$@"\n')
        (base / 'launch.sh').chmod(0o755)
        with (base / 'ldd.txt').open('w') as out:
            run('ldd', str(build / 'mdv'), stdout=out)
        if str(prefix) not in (base / 'ldd.txt').read_text():
            raise RuntimeError('Selected provider absent from ldd')
    (a.root / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')


if __name__ == '__main__':
    main()
