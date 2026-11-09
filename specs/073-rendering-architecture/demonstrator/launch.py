#!/usr/bin/env python3
"""Launch an independent actual-source window, or generate a browser reference."""
import argparse
import os
from pathlib import Path
import subprocess
from reference import generate

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer",choices=["distance","curve","skia","browser"],default="curve")
    parser.add_argument("--backend",choices=["opengl","vulkan"],default="opengl")
    parser.add_argument("--fixture",default="document")
    parser.add_argument("--sdk",choices=["minimum","development"],default="development")
    parser.add_argument("--scale",type=float,default=1,help="Qt window scale multiplier; not document zoom or a compositor setting")
    parser.add_argument("--output",type=Path,default=Path("/tmp/073-review"))
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=True)
    if args.renderer=="browser":
        output=args.output/"reference.html"
        generate(HERE/"fixtures.json",output)
        print(output.resolve())
        return
    env=os.environ.copy()
    if args.renderer=="skia":
        exe=ROOT/"build-ci/073-skia-demo/skia-compare"
        renderer=args.backend
        env["SDL_VIDEODRIVER"]="wayland"
        if args.scale!=1:
            parser.error("SDL window DPR is compositor-controlled; +/- changes document zoom")
    else:
        exe=ROOT/f"build-ci/073-demo-{args.sdk}/qt-compare"
        renderer=args.renderer
        env.update(QT_QPA_PLATFORM="wayland",QSG_RHI_BACKEND=args.backend,QT_SCALE_FACTOR=str(args.scale))
    command=[str(exe),str(HERE/"fixtures.json"),args.fixture,renderer,str(args.output.resolve()),"--interactive"]
    print("Launching",command,flush=True)
    return subprocess.call(command,env=env)


if __name__=="__main__":
    raise SystemExit(main())
