#!/usr/bin/env python3
"""One persistent diagnostic; operator handoffs are read back, never rendered."""
import fcntl
import json
import os
from pathlib import Path
import select
import subprocess
import sys
import time
import diagnose

def run(directory,identifier,command):
    directory.mkdir(parents=True,exist_ok=True)
    with (directory/'lock').open('a') as lock:
        fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
        journal=directory/'attempts.jsonl'
        events=[json.loads(line) for line in journal.read_text().splitlines()]
        starts=[r for r in events if r['event']=='start']
        if len(starts)>=60 or any(r['id']==identifier for r in starts):raise ValueError('budget/duplicate')
        outputs=diagnose.previous.read_outputs()
        original=json.loads((directory/'original-output.json').read_text())[0]
        diagnose.previous.output_scale(outputs,original['scale'],original)
        folder=directory/identifier;folder.mkdir(exist_ok=False)
        identities={str(p):diagnose.sha(p) for p in diagnose.HERE.iterdir() if p.is_file()}
        identities[command[0]]=diagnose.sha(Path(command[0]))
        diagnose.save(folder/'intent.json',dict(hypothesis='Mapped SDL3 window receives live fractional updates',
            expected='DP-1 scale changes update preferred_scale, pixel density and buffer without recreation',
            command=command,identities=identities,outputs=outputs))
        def append(row):
            with journal.open('a') as f:f.write(json.dumps(row)+'\n');f.flush();os.fsync(f.fileno())
        append(dict(event='start',id=identifier,reservedSeconds=120))
        process=None;error=None;peak=0;active=0;checkpoints=0;started=time.monotonic()
        try:
            with (folder/'records.jsonl').open('xb') as out,(folder/'wayland.log').open('xb') as err:
                process=subprocess.Popen(command,stdin=subprocess.PIPE,stdout=out,stderr=err,
                    env=diagnose.previous.environment(dict(api='opengl')))
                def pump(seconds):
                    nonlocal peak,active
                    until=time.monotonic()+seconds
                    while time.monotonic()<until:
                        peak=max(peak,diagnose.previous.rss(process.pid))
                        if process.poll() is not None:raise RuntimeError('probe exited prematurely')
                        if peak>4*diagnose.previous.GIB:raise RuntimeError('RSS limit')
                        time.sleep(.02)
                    active+=seconds
                pump(2)
                print('READY: persistent probe; send scale NUMBER after owner handoff, or quit',flush=True)
                while True:
                    peak=max(peak,diagnose.previous.rss(process.pid))
                    if peak>4*diagnose.previous.GIB or process.poll() is not None:raise RuntimeError('probe/resource failure')
                    if not select.select([sys.stdin],[],[],.1)[0]:continue
                    line=sys.stdin.readline().strip()
                    if not line or line=='quit':break
                    parts=line.split()
                    if len(parts)!=2 or parts[0]!='scale':raise ValueError('expected scale NUMBER')
                    scale=float(parts[1]);outputs=diagnose.previous.read_outputs()
                    diagnose.previous.output_scale(outputs,scale,original)
                    pump(1)
                    diagnose.handoff.save(folder/f'checkpoint-{checkpoints}.json',dict(
                        scale=scale,outputs=outputs,wallNs=time.time_ns(),monotonicNs=time.monotonic_ns()))
                    checkpoints+=1
                    print('CHECKPOINT',scale,'recorded; same process',process.pid,flush=True)
                    if active>120:raise RuntimeError('diagnostic active deadline')
                process.stdin.write(b'quit\n');process.stdin.flush();process.wait(timeout=10)
                active+=time.monotonic()-started if active==0 else 0
        except BaseException as exc:error=repr(exc)
        finally:
            if process and process.poll() is None:
                process.terminate()
                try:process.wait(timeout=10)
                except subprocess.TimeoutExpired:process.kill();process.wait(timeout=10)
            result=dict(exit=process.returncode if process else None,error=error,peakRSS=peak,
                        seconds=active,wallSeconds=time.monotonic()-started,checkpoints=checkpoints)
            diagnose.save(folder/'result.json',result);append(dict(event='finish',id=identifier,**result))
        if error:raise RuntimeError(error)

if __name__=='__main__':run(Path(sys.argv[1]),sys.argv[2],sys.argv[3:])
