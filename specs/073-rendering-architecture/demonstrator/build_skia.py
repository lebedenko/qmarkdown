#!/usr/bin/env python3
"""Build only the recorded Skia revision; all outputs stay in an external checkout."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import time

HERE = Path(__file__).resolve().parent


def run(args, cwd):
    subprocess.run(args, cwd=cwd, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("checkout", type=Path)
    args = parser.parse_args()
    checkout = args.checkout.resolve()
    pin = json.loads((HERE / "skia-pin.json").read_text())
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=checkout, text=True).strip()
    if revision != pin["commit"]:
        raise SystemExit("checkout differs from pinned revision")
    run(["git", "diff", "--quiet", "HEAD", "--"], checkout)
    started = time.monotonic()
    deps = {}
    # All other used dependencies come from explicitly recorded system packages.
    for name in ("vulkanmemoryallocator", "vulkan-headers", "highway"):
        match = re.search(r'"third_party/externals/' + name + r'"\s*:\s*"([^"]+)"', (checkout / "DEPS").read_text())
        url, commit = match[1].rsplit("@", 1)
        target = checkout / "third_party/externals" / name
        if not target.exists():
            run(["git", "init", str(target)], checkout)
            run(["git", "remote", "add", "origin", url], target)
        run(["git", "fetch", "--depth", "1", "origin", commit], target)
        run(["git", "checkout", "--detach", commit], target)
        deps[name] = commit
    run(["python3", "bin/fetch-gn"], checkout)
    gn = ('is_official_build=true is_debug=false skia_enable_tools=false '
          'skia_enable_skparagraph=true skia_enable_skshaper=true '
          'skia_use_gl=true skia_use_vulkan=true skia_use_system_freetype2=true '
          'skia_use_system_harfbuzz=true skia_use_system_icu=true '
          'skia_use_partition_alloc=false skia_use_perfetto=false '
          'skia_use_wuffs=false skia_use_piex=false skia_enable_pdf=false '
          'skia_enable_skottie=false skia_enable_svg=false '
          'skia_use_libjpeg_turbo_decode=false skia_use_libjpeg_turbo_encode=false '
          'skia_use_libwebp_decode=false skia_use_libwebp_encode=false')
    run(["bin/gn", "gen", "out/comparison", "--args=" + gn], checkout)
    run(["ninja", "-C", "out/comparison", "-j2", "skia", "skparagraph", "skshaper", "skunicode"], checkout)
    versions = subprocess.check_output(["pkg-config", "--modversion", "sdl2", "freetype2", "harfbuzz", "icu-uc", "libpng", "vulkan"], text=True)
    manifest = dict(commit=revision, dependencies=deps, systemPackageVersions=versions.splitlines(),
                    gn=gn, seconds=time.monotonic()-started,
                    gnSha256=hashlib.sha256((checkout/"bin/gn").read_bytes()).hexdigest(),
                    libraries={p.name:dict(bytes=p.stat().st_size, sha256=hashlib.sha256(p.read_bytes()).hexdigest())
                               for p in (checkout/"out/comparison").glob("*.a")})
    (checkout/"comparison-build.json").write_text(json.dumps(manifest, indent=2)+"\n")


if __name__ == "__main__":
    main()
