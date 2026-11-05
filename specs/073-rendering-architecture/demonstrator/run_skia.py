#!/usr/bin/env python3
"""Run independent native Skia cases even when Qt cases have failed."""
import argparse
import json
import os
from pathlib import Path
from run import HERE, supervise


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable",type=Path)
    parser.add_argument("output",type=Path)
    parser.add_argument("--dpr",type=float,default=1.25)
    parser.add_argument("--fixtures",default="padded,objects,document")
    parser.add_argument("--cycles",type=int,default=1)
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=False)
    manifest=HERE/"fixtures.json"
    data=json.loads(manifest.read_text())
    inputs=[args.executable.resolve(),*(HERE/"skia").glob("*"), *[HERE/name for name in ("fixtures.json","skia-pin.json","build_skia.py","run.py","run_skia.py")],*map(Path,data["fonts"].values())]
    results=[]
    for backend in ("opengl","vulkan"):
        for fixture in args.fixtures.split(","):
            for cycle in range(args.cycles):
                folder=args.output/f"{backend}-{fixture}-{cycle}"
                command=[str(args.executable.resolve()),str(manifest),fixture,backend,str(folder.resolve())]
                result=supervise(command,folder,dict(os.environ,SDL_VIDEODRIVER="wayland"),
                                 dict(api=backend,platform="wayland",dpr=args.dpr),inputs)
                results.append(result)
                print(json.dumps({k:v for k,v in result.items() if k!="inputsAfter"}),flush=True)
    (args.output/"summary.json").write_text(json.dumps(dict(results=results,
        dprCoverage={str(dpr):"observed" if any(r["observed"].get("dpr")==dpr for r in results)
                    else "unavailable in current compositor configuration; not emulated by zoom"
                    for dpr in (1,1.25,1.5,2)}),indent=2)+"\n")
    return 0 if all(r["passed"] for r in results) else 1


if __name__=="__main__":
    raise SystemExit(main())
