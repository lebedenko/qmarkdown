#!/usr/bin/env python3
"""Keep generated artifacts and oversized files out of repository snapshots."""
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
MAX_BYTES = 1024 * 1024
ARCHIVES = ('.gz', '.zip', '.tar', '.xz', '.bz2', '.zst', '.7z', '.rar')
COMPILED = ('.o', '.obj', '.a', '.so', '.dll', '.dylib', '.exe', '.bin', '.pyc')
CAPTURES = ('.png', '.jpg', '.jpeg', '.webp', '.gif', '.pdf', '.mp4', '.webm')
RAW_OUTPUT = ('.log', '.jsonl', '.stderr', '.stdout', '.build')


def violation(name, size, prefix=b''):
    """Return the storage-policy violation, or None for source/report assets."""
    lower = name.lower()
    if size > MAX_BYTES:
        return 'file exceeds 1 MiB; retain a concise summary or use external artifact storage'
    if lower.endswith(ARCHIVES):
        return 'generated archive belongs outside Git'
    if lower.endswith(COMPILED) or '.so.' in Path(lower).name or prefix.startswith(b'\x7fELF'):
        return 'compiled binary belongs in an ignored build directory'
    if lower.startswith('specs/') and lower.endswith(CAPTURES):
        return 'generated capture belongs outside Git'
    if lower.startswith('specs/') and '/evidence/' in lower and lower.endswith(RAW_OUTPUT):
        return 'raw execution output belongs outside Git'
    return None


def validate(root=ROOT, paths=None):
    if paths is None:
        paths = subprocess.check_output(
            ['git', 'ls-files', '--cached', '--others', '--exclude-standard', '-z'],
            cwd=root).decode().split('\0')
    errors = []
    for name in paths:
        path = root / name
        if not name or not path.is_file() or path.is_symlink():
            continue
        with path.open('rb') as stream:
            reason = violation(name, path.stat().st_size, stream.read(4))
        if reason:
            errors.append(f'{name}: {reason}')
    if errors:
        raise ValueError('\n'.join(errors))
    print('PASS: repository storage policy (no generated binaries/archives or oversized files)')


if __name__ == '__main__':
    try:
        validate()
    except ValueError as error:
        raise SystemExit(str(error))
