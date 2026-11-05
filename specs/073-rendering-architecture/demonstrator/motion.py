#!/usr/bin/env python3
"""Serial rendered-frame slow/flick/reversal and captured-pose replay checks."""
import argparse
import json
import os
from pathlib import Path
from run import HERE, supervise

ROOT=HERE.parents[2]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output",type=Path)
    parser.add_argument("--fixtures",default="padded,objects,document")
    args=parser.parse_args()
    args.output.mkdir(parents=True,exist_ok=False)
    manifest=HERE/"fixtures.json"
    data=json.loads(manifest.read_text())
    fonts=list(map(Path,data["fonts"].values()))
    results=[]
    controls=[(sdk,version,renderer,backend,dpr)
              for sdk,version in (("development","6.12.0"),("minimum","6.8.0"))
              for renderer in ("distance","curve") for backend in ("opengl","vulkan")
              for dpr in (1,1.25,1.5,2)]
    controls += [("skia",None,"skia",backend,1) for backend in ("opengl","vulkan")]
    for sdk,version,renderer,backend,dpr in controls:
        exe=ROOT/"build-ci"/("073-skia-demo" if sdk=="skia" else f"073-demo-{sdk}")/("skia-compare" if sdk=="skia" else "qt-compare")
        for fixture in args.fixtures.split(","):
            folder=args.output/f"{sdk}-{renderer}-{backend}-{dpr}-{fixture}"
            env=os.environ.copy()
            expected=dict(platform="wayland",dpr=dpr)
            if sdk=="skia":
                env["SDL_VIDEODRIVER"]="wayland"
                expected["api"]=backend
                sources=[HERE/"skia/main.cpp",HERE/"skia/CMakeLists.txt",HERE/"skia-pin.json"]
            else:
                env.update(QT_QPA_PLATFORM="wayland",QSG_RHI_BACKEND=backend,QT_SCALE_FACTOR=str(dpr/1.25))
                expected.update(qt=version,api={"opengl":3,"vulkan":5}[backend])
                sources=[HERE/"qt.cpp",HERE/"stage.py",*exe.parent.rglob("libQMarkdown.so*")]
            command=[str(exe),str(manifest),fixture,backend if sdk=="skia" else renderer,
                     str(folder.resolve()),"--motion-replay"]
            result=supervise(command,folder,env,expected,[exe,manifest,HERE/"motion.py",HERE/"run.py",*sources,*fonts])
            rows=[]
            for line in (folder/"stdout.jsonl").read_text(errors="replace").splitlines():
                try:
                    rows.append(json.loads(line))
                except ValueError:
                    pass
            count=sum(r.get("stage")=="motion-frame" for r in rows)
            replay=any(r.get("operation")=="captured-motion-pose" and r.get("exact") is True for r in rows)
            result.update(motionFrames=count,capturedMotionReplay=replay)
            if count!=60 or not replay:
                result["passed"]=False
                result["errors"].append("missing complete motion sequence or captured-pose replay")
            (folder/"result.json").write_text(json.dumps(result,indent=2)+"\n")
            results.append(result)
            print(json.dumps({k:v for k,v in result.items() if k!="inputsAfter"}),flush=True)
    (args.output/"summary.json").write_text(json.dumps(results,indent=2)+"\n")
    return 0 if all(r["passed"] for r in results) else 1


if __name__=="__main__":
    raise SystemExit(main())
