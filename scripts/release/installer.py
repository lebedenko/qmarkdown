#!/usr/bin/env python3
"""Install only validated, tracked QMarkdown files; Python 3.9+ standard library."""
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import platform
import shutil
import stat
import subprocess

RECORD = 'usr/share/qmarkdown/installed.json'


def entries_valid(entries, require_targets=False):
    if not isinstance(entries, dict) or not entries:
        raise ValueError('Empty or invalid manifest')
    for name, entry in entries.items():
        path = PurePosixPath(name)
        if (not name.startswith('usr/') or str(path) != name or '..' in path.parts
                or name == RECORD or any(ord(c) < 32 for c in name)):
            raise ValueError(f'Unsafe path: {name}')
        if entry.get('kind') == 'link':
            target = entry['target']
            if not target or PurePosixPath(target).is_absolute():
                raise ValueError(f'Unsafe link: {name}')
            normalized = os.path.normpath(str(path.parent / target))
            if not normalized.startswith('usr/') or (require_targets and normalized not in entries):
                raise ValueError(f'Escaping link: {name}')
        elif entry.get('kind') == 'file':
            if entry.get('mode') not in (0o644, 0o755):
                raise ValueError(f'Unsafe permissions: {name}')
            digest = entry.get('sha256', '')
            if len(digest) != 64 or any(c not in '0123456789abcdef' for c in digest):
                raise ValueError(f'Invalid digest: {name}')
        else:
            raise ValueError(f'Unsupported entry: {name}')
    for name in entries:
        if any(str(parent) in entries for parent in PurePosixPath(name).parents):
            raise ValueError(f'Manifest ancestor is a file: {name}')


def describe(path):
    if path.is_symlink():
        return {'kind': 'link', 'target': os.readlink(path)}
    if path.is_file():
        return {'kind': 'file', 'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
                'mode': stat.S_IMODE(path.stat().st_mode)}
    return None


def safe_destination(root, name):
    path = root / name
    for parent in reversed(path.parents):
        if parent == root or root in parent.parents:
            if parent.is_symlink() or (parent.exists() and not parent.is_dir()):
                raise ValueError(f'Unsafe destination ancestor: {parent}')
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('operation', choices=('install', 'uninstall'))
    parser.add_argument('--destdir', type=Path)
    args = parser.parse_args()
    if args.destdir and not args.destdir.is_absolute():
        parser.error('--destdir must be absolute')
    root = args.destdir or Path('/')
    if root.is_symlink() or root.resolve() != root:
        parser.error('Destination root must be canonical and not a symlink')
    real = root == Path('/')
    if real and os.geteuid() != 0:
        parser.error('Real installation requires root; use sudo')
    record = safe_destination(root, RECORD)
    if record.is_symlink() or (record.exists() and not record.is_file()):
        raise ValueError('Unsafe installer record')
    temporary = record.with_suffix('.tmp')
    if temporary.exists() or temporary.is_symlink():
        raise ValueError('Conflicting temporary installer record')
    old = json.loads(record.read_text()) if record.exists() else {}
    if old:
        entries_valid(old)
    package = Path(__file__).resolve().parent
    new = {}
    if args.operation == 'install':
        metadata = json.loads((package / 'metadata.json').read_text())
        if metadata['architecture'] != 'x86_64' or platform.machine() not in ('x86_64', 'AMD64'):
            raise ValueError('Package requires x86_64')
        new = json.loads((package / 'manifest.json').read_text())
        entries_valid(new, require_targets=True)
        payload = package / 'payload'
        actual = {p.relative_to(payload).as_posix(): describe(p)
                  for p in payload.rglob('*') if not p.is_dir() or p.is_symlink()}
        if actual != new:
            raise ValueError('Payload differs from manifest')
        for name in new:
            source = safe_destination(payload, name)
            resolved = source.resolve(strict=True)
            if payload.resolve() not in resolved.parents:
                raise ValueError(f'Escaping payload link: {name}')
        # Preflight every old and new path before changing any files.
        for name, entry in old.items():
            path = safe_destination(root, name)
            if describe(path) != entry:
                raise ValueError(f'Owned file missing or modified: {name}')
        for name in new:
            path = safe_destination(root, name)
            if name not in old and (path.exists() or path.is_symlink()):
                raise ValueError(f'Unrelated conflict: {name}')
        for name in old.keys() - new.keys():
            (root / name).unlink()
        for name, entry in new.items():
            target = root / name
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.exists() or target.is_symlink():
                target.unlink()
            if entry['kind'] == 'link':
                target.symlink_to(entry['target'])
            else:
                shutil.copyfile(payload / name, target)
                target.chmod(entry['mode'])
            if real:
                os.chown(target, 0, 0, follow_symlinks=False)
    else:
        # Preflight unsafe parents before any removals.
        for name in old:
            safe_destination(root, name)
        for name, entry in old.items():
            path = root / name
            if describe(path) == entry:
                path.unlink()
            elif path.exists() or path.is_symlink():
                print(f'Preserving modified file: {name}')
                new[name] = entry
    record.parent.mkdir(parents=True, exist_ok=True)
    if new:
        temporary = record.with_suffix('.tmp')
        if temporary.exists() or temporary.is_symlink():
            raise ValueError('Conflicting temporary installer record')
        with temporary.open('x') as stream:
            json.dump(new, stream, indent=2, sort_keys=True)
            stream.write('\n')
        temporary.chmod(0o644)
        if real:
            os.chown(temporary, 0, 0)
        temporary.replace(record)
    elif record.exists():
        record.unlink()
    if real:
        subprocess.run(['ldconfig'], check=True)
    print(f'PASS: {args.operation} in {root}')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, KeyError) as error:
        raise SystemExit(str(error))
