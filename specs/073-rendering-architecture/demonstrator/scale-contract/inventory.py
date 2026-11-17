#!/usr/bin/env python3
"""Read-only runtime inventory; no environment dump or configuration writes."""
import json
from pathlib import Path
import subprocess
import sys
from diagnose import sha,save,previous
out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=False)
commands={
 'packages':['pacman','-Q'],
 'kernel':['uname','-a'],
 'outputs':['wlr-randr','--json'],
 'processes':['ps','-eo','pid,comm'],
 'gpu':['nvidia-smi','--query-gpu=name,driver_version','--format=csv,noheader'],
}
results={}
for name,command in commands.items():
    p=subprocess.run(command,capture_output=True,text=True,timeout=10)
    results[name]=dict(command=command,exit=p.returncode,stdout=p.stdout,stderr=p.stderr)
save(out/'inventory.json',results)
identities={}
for line in results['processes']['stdout'].splitlines()[1:]:
    fields=line.split()
    if len(fields)!=2:continue
    pid,comm=fields
    if any(name in comm.lower() for name in ('kwin','labwc','sway','niri','mango','hypr','weston','wayfire','miracle','cosmic-comp','dwl')):
        executable=Path('/proc')/pid/'exe'
        try:
            real=executable.resolve(strict=True);identities[str(real)]=sha(real)
            linked=subprocess.run(['ldd',str(real)],capture_output=True,text=True,timeout=10)
            (out/f'{comm}-{pid}-ldd.txt').write_text(linked.stdout+linked.stderr)
            for word in linked.stdout.split():
                if word.startswith('/') and Path(word).is_file():identities[word]=sha(Path(word))
        except (OSError,subprocess.SubprocessError) as error:identities[str(executable)]=dict(error=str(error))
save(out/'compositor-identities.json',identities)
