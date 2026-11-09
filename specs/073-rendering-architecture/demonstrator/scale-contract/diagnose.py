#!/usr/bin/env python3
"""Bounded serial diagnostic launches; never retry a recorded attempt."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import resource
import subprocess
import time

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[3]
spec=importlib.util.spec_from_file_location('previous',HERE.parent/'scale-study-v2/study.py')
previous=importlib.util.module_from_spec(spec);spec.loader.exec_module(previous)
spec=importlib.util.spec_from_file_location('handoff',HERE.parent/'scale-study-live-recovery/handoff.py')
handoff=importlib.util.module_from_spec(spec);spec.loader.exec_module(handoff)

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def save(path,data):
    with path.open('x') as f:
        json.dump(data,f,indent=2);f.write('\n');f.flush();os.fsync(f.fileno())
def limited():resource.setrlimit(resource.RLIMIT_AS,(4*previous.GIB,4*previous.GIB))

def run(directory,identifier,scale,hypothesis,expected,command):
    directory.mkdir(parents=True,exist_ok=True)
    import fcntl
    with (directory/'lock').open('a') as lock:
        fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        journal=directory/'attempts.jsonl'
        events=[json.loads(line) for line in journal.read_text().splitlines()] if journal.exists() else []
        starts=[r for r in events if r['event']=='start']
        if any(r['id']==identifier for r in starts):raise ValueError('duplicate diagnostic')
        if len(starts)>=60:raise ValueError('diagnostic ceiling; final 690 reserved')
        consumed=sum(r['seconds'] for r in events if r['event']=='finish')
        consumed+=120*(len(starts)-sum(r['event']=='finish' for r in events))
        if consumed+120>14400:raise ValueError('automated deadline')
        outputs=previous.read_outputs()
        original_path=directory/'original-output.json'
        if original_path.exists():original=json.loads(original_path.read_text())[0]
        else:save(original_path,outputs);original=outputs[0]
        previous.output_scale(outputs,scale,original)
        folder=directory/identifier;folder.mkdir(exist_ok=False)
        identities={str(p):sha(p) for p in HERE.iterdir() if p.is_file()}
        executable=Path(command[0]).resolve();identities[str(executable)]=sha(executable)
        linked=subprocess.check_output(['ldd',str(executable)],text=True)
        (folder/'ldd.txt').write_text(linked)
        for line in linked.splitlines():
            for word in line.split():
                if word.startswith('/') and Path(word).is_file():identities[word]=sha(Path(word))
        save(folder/'intent.json',dict(hypothesis=hypothesis,expected=expected,command=command,
             revision=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip(),
             identities=identities,outputs=outputs))
        def append(row):
            with journal.open('a') as f:
                f.write(json.dumps(row)+'\n');f.flush();os.fsync(f.fileno())
        append(dict(event='start',id=identifier,reservedSeconds=120))
        started=time.monotonic();peak=0;error=None;process=None
        try:
            with (folder/'records.jsonl').open('xb') as out,(folder/'wayland.log').open('xb') as err:
                env=previous.environment(dict(api='opengl'));env['SDL_VIDEO_DRIVER']='wayland'
                process=subprocess.Popen(command,stdout=out,stderr=err,env=env,preexec_fn=limited)
                while process.poll() is None:
                    peak=max(peak,previous.rss(process.pid))
                    failure=previous.limits(time.monotonic(),started,started+20,peak)
                    if failure:raise RuntimeError(failure)
                    time.sleep(.02)
        except BaseException as exc:
            error=repr(exc)
            if process and process.poll() is None:
                process.terminate()
                try:process.wait(timeout=10)
                except subprocess.TimeoutExpired:process.kill();process.wait(timeout=10)
        finally:
            result=dict(exit=process.returncode if process else None,error=error,peakRSS=peak,
                        seconds=time.monotonic()-started)
            save(folder/'result.json',result)
            append(dict(event='finish',id=identifier,**result))
        if error:raise RuntimeError(error)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('directory',type=Path);p.add_argument('identifier')
    p.add_argument('scale',type=float);p.add_argument('hypothesis');p.add_argument('expected')
    p.add_argument('command',nargs=argparse.REMAINDER);a=p.parse_args()
    run(a.directory,a.identifier,a.scale,a.hypothesis,a.expected,a.command)
