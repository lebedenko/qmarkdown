#!/usr/bin/env python3
"""Generate browser visual references from the shared manifest (not pixel oracles)."""
import argparse
import base64
import html
import json
from pathlib import Path


def generate(manifest, output):
    data = json.loads(manifest.read_text())
    fonts = []
    for role, filename in data["fonts"].items():
        encoded = base64.b64encode(Path(filename).read_bytes()).decode()
        fonts.append(f"@font-face{{font-family:{role};src:url(data:font/ttf;base64,{encoded})}}")
    h, v = data["padding"]
    content = ["<!doctype html><meta charset=utf-8><title>073 browser visual references</title>",
               "<style>" + "".join(fonts) +
               f"body{{font: {data['fontSize']}px regular,cjk,emoji;background:{data['background']};margin:24px}}"
               f"code{{font-family:mono;background:#eee;padding:{v}px {h}px;border-radius:4px;box-decoration-break:clone;-webkit-box-decoration-break:clone}}"
               "strong{font-family:bold}em{font-family:italic}p{white-space:pre-wrap;margin:0 0 18px}"
               ".object{display:inline-block;background:#b9eadb;border:1px solid #176b58;border-radius:4px;box-sizing:border-box}"
               ".table{border-bottom:1px solid #cbd5df}section{margin-bottom:48px}header{font:16px sans-serif;color:#456}</style>",
               "<header>Browser visual reference only. Native controls and acceptance are separate.</header>"]
    for fixture in data["fixtures"]:
        content.append(f"<section id='{html.escape(fixture['id'])}' style='width:{fixture['width']}px'><h2>{html.escape(fixture['id'])}</h2>")
        for block in fixture["blocks"]:
            if block.get("kind") == "rule":
                content.append("<hr>")
                continue
            units = block["text"].encode("utf-16-le")
            starts, ends = {}, {}
            for span in block["spans"]:
                tag = dict(code="code", bold="strong", italic="em", strike="s")[span["kind"]]
                starts.setdefault(span["start"], []).append("<"+tag+">")
                ends.setdefault(span["start"]+span["length"], []).insert(0, "</"+tag+">")
            objects = {o["start"]: o for o in block["objects"]}
            rendered = []
            i = 0
            while i <= len(units)//2:
                rendered.extend(ends.get(i, []))
                rendered.extend(starts.get(i, []))
                if i == len(units)//2:
                    break
                if i in objects:
                    o = objects[i]
                    source = units[2*i:2*(i+o["length"])].decode("utf-16-le")
                    rendered.append(f'<span class="object" title="{html.escape(source)}" data-source="{html.escape(source)}" style="width:{o["width"]}px;height:{o["ascent"]+o["descent"]}px;vertical-align:-{o["descent"]}px"></span>')
                    i += o["length"]
                else:
                    count = 2 if 0xD800 <= int.from_bytes(units[2*i:2*i+2], "little") <= 0xDBFF else 1
                    rendered.append(html.escape(units[2*i:2*(i+count)].decode("utf-16-le")))
                    i += count
            content.append('<p class="'+block.get("kind","")+'">'+"".join(rendered)+"</p>")
        content.append("</section>")
    output.write_text("\n".join(content)+"\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    generate(args.manifest, args.output)
