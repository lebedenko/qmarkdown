"""Deterministic complete-grapheme atoms; all ranges use UTF-16 units."""
import copy
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent


def units(text):
    return len(text.encode('utf-16-le')) // 2


def block(kind, size, spans=None):
    # Every atom starts and ends at a grapheme boundary; never truncate an atom.
    atom = {'prose': 'The quick brown fox reads a paragraph. ',
            'dense': 'alpha beta gamma delta ',
            'mixed': 'Latin e\u0301 العربية אבג 漢字 👩\u200d💻 ',
            'long': 'abcdefghijklmno', 'objects': 'Text [object] next words. '}[kind]
    text = atom * max(1, size // units(atom))
    text += 'x' * (size - units(text))
    n = units(text)
    codes, objects = [], []
    if spans is not None:
        # ASCII input in the independent span sweep: exactly 50% code coverage.
        text = 'a ' * (size // 2)
        n = units(text)
        chunk = n // spans
        codes = [dict(start=i * chunk, length=chunk // 2, kind='code') for i in range(spans)]
    elif kind == 'long':
        codes = [dict(start=0, length=n, kind='code')]
    elif kind in ('dense', 'mixed', 'objects'):
        step = units(atom)
        codes = [dict(start=i, length=5 if kind != 'objects' else 13, kind='code')
                 for i in range(0, n - step + 1, step)]
    if kind == 'objects':
        objects = [dict(start=i+5, length=8, width=38, ascent=25, descent=8)
                   for i in range(0, n-units(atom)+1, units(atom))]
    return dict(text=text, spans=codes, objects=objects)


def cases():
    for kind in ('prose', 'dense', 'mixed', 'long', 'objects'):
        for width in (240, 480, 960):
            for size in (1024, 4096, 16384, 65536, 262144):
                yield dict(id=f'{kind}-{width}-{size}', series=f'{kind}-{width}', size=size,
                           width=width, blocks=[block(kind, size)])
    for width in (240, 480, 960):
        for count in (1, 8, 32, 128):
            yield dict(id=f'spans-{width}-{count}', series=f'spans-{width}', size=count,
                       width=width, blocks=[block('dense', 16384, count)])
    for width in (240, 480, 960):
        for count in (10, 100, 1000):
            yield dict(id=f'document-{width}-{count}', series=f'document-{width}', size=count,
                       width=width, blocks=[block('objects', 256) for _ in range(count)])


def manifest(case):
    data = json.loads((HERE.parent/'boundary-experiment/minimal.json').read_text())
    data['fixtures'] = [copy.deepcopy(case)]
    data['exactUtf16Units'] = sum(units(b['text']) for b in case['blocks'])
    data['codeUnits'] = sum(s['length'] for b in case['blocks'] for s in b['spans'])
    return data


def native_cases():
    for name, kind, size, count in [('typical', 'objects', 256, 10),
                                     ('span-dense', 'dense', 1024, 10),
                                     ('large-document', 'objects', 256, 1000)]:
        yield dict(id=name, width=752, blocks=[block(kind, size) for _ in range(count)])
