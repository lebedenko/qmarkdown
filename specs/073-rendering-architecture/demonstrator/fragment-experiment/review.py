#!/usr/bin/env python3
"""Create an interactive, offline image review of the blocked CPU probe."""
import argparse
import base64
from pathlib import Path
import webbrowser

HERE = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--evidence', type=Path, default=HERE.parents[1] / 'evidence/fragment-experiment')
    parser.add_argument('--output', type=Path, default=Path('build-ci/073-fragment-review.html'))
    parser.add_argument('--open', action='store_true')
    args = parser.parse_args()
    labels = [('adjacent-lam-alef', 'Patched preflight: adjacent spans, unchanged ligature'),
              ('whole-lam-alef', 'Control geometry: whole span, same ligature'),
              ('reshaped-lam', 'Counterfactual: lam shaped alone'),
              ('reshaped-alef', 'Counterfactual: alef shaped alone')]
    cards = []
    for name, label in labels:
        encoded = base64.b64encode((args.evidence / 'images' / (name+'-0.png')).read_bytes()).decode()
        cards.append(f'<section><h2>{label}</h2><div class="crop"><img alt="{label}" src="data:image/png;base64,{encoded}"></div></section>')
    html = '''<!doctype html><html lang="en"><meta charset="utf-8"><title>073 blocked fragment experiment</title>
<style>body{font:16px system-ui;margin:32px;max-width:1000px;color:#182533}h2{font-size:18px}
.crop{height:180px;overflow:hidden;border:1px solid #ccd4dd}img{transform:scale(var(--zoom,3));transform-origin:top left;image-rendering:pixelated}
section{margin:24px 0}button,input{margin:8px}</style>
<h1>Fragment experiment: blocked</h1><p>These are CPU diagnostic images, not padded-layout success or native visual acceptance.
Two adjacent code spans in Arabic lam–alef require an edge inside a single glyph.
The last two images show different glyphs after separate shaping; that is outside approved scope.</p>
<label>Magnification <input id="zoom" type="range" min="1" max="4" value="3" step=".5"></label>
<button id="counter">Hide/show reshaping counterfactuals</button>'''+''.join(cards)+'''
<script>document.querySelector('#zoom').oninput=e=>document.body.style.setProperty('--zoom',e.target.value);
document.querySelector('#counter').onclick=()=>document.querySelectorAll('section:nth-of-type(n+3)').forEach(e=>e.hidden=!e.hidden);</script></html>'''
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(html)
    print(args.output.resolve())
    if args.open:
        webbrowser.open(args.output.resolve().as_uri())


if __name__ == '__main__':
    main()
