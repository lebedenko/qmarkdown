#!/usr/bin/env python3
"""Bounded coordinated study. No command changes the desktop scale."""
import argparse
import fcntl
import hashlib
import json
import math
import os
from pathlib import Path
import re
import signal
import subprocess
import time

HERE = Path(__file__).resolve().parent
SCALES = [1, 1.25, 1.5, 2]
TRANSITIONS = [2, 1.5, 1.25, 1, 1.25, 1.5, 2]
GIB = 1024**3


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def save(p, value):
    with p.open('x') as f:
        json.dump(value, f, indent=2, ensure_ascii=False)
        f.write('\n')
        f.flush()
        os.fsync(f.fileno())


def load(p):
    return json.loads(p.read_text())


def output_scale(outputs, requested, original=None):
    enabled = [o for o in outputs if o.get('enabled')]
    if len(enabled) != 1 or enabled[0].get('name') != 'DP-1':
        raise RuntimeError('unreliable output identity: expected DP-1 as sole enabled output')
    output = enabled[0]
    if original and any(output.get(k) != original.get(k) for k in ('name','make','model','serial')):
        raise RuntimeError('output identity changed')
    if output.get('scale') != requested:
        raise ValueError(f'wrong scale: requested {requested}, observed {output.get("scale")}')
    return output


def read_outputs():
    return json.loads(subprocess.check_output(['wlr-randr','--json'], timeout=10))


def environment(config):
    env = os.environ.copy()
    for key in list(env):
        if key.startswith(('QT_SCALE','QT_SCREEN_SCALE')) or key in (
                'QT_AUTO_SCREEN_SCALE_FACTOR','QT_ENABLE_HIGHDPI_SCALING','QT_FONT_DPI',
                'QT_DEVICE_PIXEL_RATIO','QT_QUICK_BACKEND'):
            del env[key]
    env.update(WAYLAND_DEBUG='1', QT_QPA_PLATFORM='wayland', SDL_VIDEODRIVER='wayland',
               QSG_RHI_BACKEND=config['api'])
    return env


def schedule(build, directory):
    fixtures = [f['id'] for f in load(build/'fixtures.json')['fixtures']]
    configs = [dict(id=f'{version}-{renderer}-{api}',role=role,qt=version,renderer=renderer,api=api)
               for role,version in [('minimum','6.8.0'),('development','6.12.0')]
               for renderer in ('distance','curve') for api in ('opengl','vulkan')]
    configs += [dict(id='skia-'+api,role='skia',renderer='skia',api=api) for api in ('opengl','vulkan')]
    cells = []
    for scale in SCALES:
        for config in configs:
            for fixture,cycle in [(f,0) for f in fixtures]+[('document',1),('document',2)]:
                cells.append(dict(scale=scale,config=config,fixture=fixture,cycle=cycle,kind='fresh'))
    cells += [dict(scale=2,config=c,fixture='bidi-objects',cycle=0,kind='live') for c in configs]
    for i,cell in enumerate(cells):
        cell['id'] = f'{i:03d}'
        c = cell['config']
        exe = build/c['role']/('skia-compare' if c['role']=='skia' else 'qt-compare')
        cell['command'] = [str(exe),str(build/'fixtures.json'),cell['fixture'],
            c['api'] if c['role']=='skia' else c['renderer'],str(directory/'attempts'/cell['id'])]
        if cell['kind']=='live':
            cell['command'].append('--scale-live')
        elif cell['cycle']==0:
            cell['command'].append('--motion-replay')
    assert len(cells)==690
    return cells


class Journal:
    def __init__(self, directory):
        self.directory = directory
        self.lock = (directory/'lock').open('a')
        fcntl.flock(self.lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        self.path = directory/'attempts.jsonl'
        self.events = []
        if self.path.exists():
            raw = self.path.read_bytes()
            if raw and not raw.endswith(b'\n'):
                raise RuntimeError('truncated attempt journal; refuse recovery')
            for line in raw.splitlines():
                row = json.loads(line)
                if row.get('sequence') != len(self.events):
                    raise RuntimeError('journal sequence mismatch')
                self.events.append(row)
        self.started = {r['id'] for r in self.events if r['event']=='start'}
        if len(self.started) != sum(r['event']=='start' for r in self.events):
            raise RuntimeError('duplicate attempt')
        self.finished = {r['id']:r for r in self.events if r['event']=='finish'}
        if len(self.finished)!=sum(r['event']=='finish' for r in self.events):
            raise RuntimeError('duplicate completion')
        for identifier,row in self.finished.items():
            if identifier not in self.started:
                raise RuntimeError('completion without attempt')
            result_path=directory/'attempts'/identifier/'result.json'
            if sha(result_path)!=row['resultHash']:
                raise RuntimeError('completed result changed')
            result=load(result_path)
            for name,expected in result.get('captures',{}).items():
                if sha(result_path.parent/name)!=expected:
                    raise RuntimeError('immutable capture changed')
        prior = load(directory/'freeze.json').get('priorAutomatedSeconds', 0) if (directory/'freeze.json').exists() else 0
        self.seconds = prior + sum(r.get('seconds',0) for r in self.events if r['event']=='finish')
        self.seconds += 120*len(self.started-self.finished.keys())
        if any(r['event']=='stop' for r in self.events):
            raise RuntimeError('study stopped terminally')

    def __del__(self):
        if hasattr(self,'lock'):self.lock.close()

    def append(self, event, **data):
        row = dict(sequence=len(self.events),event=event,**data)
        with self.path.open('a') as f:
            f.write(json.dumps(row,ensure_ascii=False)+'\n');f.flush();os.fsync(f.fileno())
        self.events.append(row)
        return row

    def start(self, cell):
        if cell['id'] in self.started:
            raise RuntimeError('attempt already launched')
        if len(self.started)>=690 or self.seconds>=14400:
            raise RuntimeError('whole-study budget exceeded')
        self.append('start',id=cell['id'],command=cell['command'],reservedSeconds=120)
        self.started.add(cell['id'])

    def finish(self, cell, result):
        row = self.append('finish',id=cell['id'],status=result['status'],seconds=result['seconds'],
                          resultHash=sha(self.directory/'attempts'/cell['id']/'result.json'))
        self.finished[cell['id']]=row
        self.seconds += result['seconds']


def rss(pid):
    try:
        for line in Path(f'/proc/{pid}/status').read_text().splitlines():
            if line.startswith('VmRSS:'):
                return int(line.split()[1])*1024
    except FileNotFoundError:
        return 0
    return 0


def limits(now, started, deadline, memory, absolute=True):
    if memory>4*GIB:
        return 'process RSS exceeded 4 GiB'
    if absolute and now-started>120:
        return 'absolute 120-second deadline'
    if now>deadline:
        return 'operation deadline'
    return None


def protocol_output(text):
    names = {}
    bindings = {}

    def name_output(output, name):
        if output in names and names[output] != name:
            raise RuntimeError('conflicting output names')
        names[output] = name

    for xdg, output in re.findall(
            r'get_xdg_output\(new id zxdg_output_v1[@#](\d+), wl_output[@#](\d+)\)', text):
        if xdg in bindings and bindings[xdg] != output:
            raise RuntimeError('conflicting xdg-output binding')
        bindings[xdg] = output
    for output, name in re.findall(r'wl_output[@#](\d+)\.name\("([^"]+)"\)', text):
        name_output(output, name)
    for xdg, name in re.findall(r'zxdg_output_v1[@#](\d+)\.name\("([^"]+)"\)', text):
        if xdg not in bindings:
            raise RuntimeError('xdg-output name without output binding')
        name_output(bindings[xdg], name)
    enters = re.findall(r'wl_surface[@#](\d+)\.enter\(wl_output[@#](\d+)\)', text)
    if not enters or any(names.get(output) != 'DP-1' for _, output in enters):
        raise RuntimeError('surface/output association missing or not DP-1')
    return dict(outputNames=names, xdgOutputBindings=bindings, surfaceEnters=enters,
        fractionalScale120ths=[int(n) for n in re.findall(r'preferred_scale\((\d+)\)', text)],
        viewportDestinations=re.findall(r'wp_viewport[@#]\d+\.set_destination\((\d+), (\d+)\)', text),
        bufferScales=re.findall(r'wl_surface[@#]\d+\.set_buffer_scale\((\d+)\)', text))


class Process:
    def __init__(self, cell, directory):
        self.cell=cell;self.folder=directory/'attempts'/cell['id']
        self.folder.mkdir(parents=True,exist_ok=False)
        self.out=(self.folder/'stdout.jsonl').open('xb')
        self.err=(self.folder/'wayland.log').open('xb')
        self.records=[];self.partial=b'';self.errors=[]
        self.started=time.monotonic();self.deadline=self.started+20;self.peak=0
        self.process=subprocess.Popen(cell['command'],env=environment(cell['config']),
            stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=self.err,start_new_session=True)
        os.set_blocking(self.process.stdout.fileno(),False)
        self.ready=False;self.eof=False;self.waiting=False;self.closed=False

    def pump(self):
        self.peak=max(self.peak,rss(self.process.pid))
        try:
            data=os.read(self.process.stdout.fileno(),65536)
        except BlockingIOError:
            return
        if not data:
            self.eof=True;return
        self.out.write(data);self.out.flush();self.partial+=data
        while b'\n' in self.partial:
            line,self.partial=self.partial.split(b'\n',1)
            try:
                r=json.loads(line)
                if not isinstance(r,dict) or not isinstance(r.get('stage'),str):
                    raise ValueError('missing stage')
                self.records.append(r)
                if r['stage']=='ready':
                    self.ready=True;self.deadline=time.monotonic()+10
                elif r['stage'] in ('capture','replay','shutdown','checkpoint') and self.ready:
                    self.deadline=time.monotonic()+10
            except (ValueError,UnicodeError) as exc:
                self.errors.append('malformed record: '+str(exc))

    def stop(self):
        if self.closed:return
        if self.process.poll() is None:
            os.killpg(self.process.pid,signal.SIGKILL)
        self.process.wait(timeout=10)
        self.ended=time.monotonic()
        self.pump()
        self.process.stdout.close();self.process.stdin.close();self.out.close();self.err.close();self.closed=True
        save(self.folder/'process-facts.json', dict(exit=self.process.returncode,
            seconds=self.ended-self.started, peakRSS=self.peak,
            captures={p.name:sha(p) for p in self.folder.glob('*.png')}))

    def send(self, command):
        self.process.stdin.write((command+'\n').encode());self.process.stdin.flush()
        self.deadline=time.monotonic()+10


def validate(process, manifest, live=False):
    rows=process.records;cell=process.cell;c=cell['config'];errors=process.errors.copy()
    ready=next((r for r in rows if r['stage']=='ready'),{})
    expected=dict(platform='wayland',api=c['api'] if c['role']=='skia' else {'opengl':3,'vulkan':5}[c['api']])
    if c['role']!='skia':
        expected.update(qt=c['qt'],dpr=cell['scale'])
    for k,v in expected.items():
        if ready.get(k)!=v:errors.append(f'identity {k}: {ready.get(k)!r} != {v!r}')
    checks=[r for r in rows if r['stage']=='study-check']
    fixture=next(f for f in manifest['fixtures'] if f['id']==cell['fixture'])
    source=next((b['text'] for b in fixture['blocks'] if b['text']),'')
    if not checks:errors.append('missing coordinate/source checks')
    for r in checks:
        if r.get('inverse') is not True or r.get('contained',True) is not True:
            errors.append('coordinate/containment assertion failed')
        if r.get('forward')!=source or r.get('reverse')!=source:
            errors.append('bidirectional source-copy mismatch')
    if not live:
        names={r['name'] for r in rows if r['stage']=='capture'}
        required={'stationary','selected','resize','reset','pose','replay','clip-entry','settled','zoom'}
        if not required<=names or any(not (process.folder/(n+'.png')).is_file() for n in required):
            errors.append('missing captures')
        if cell['cycle']==0 and (sum(r['stage']=='motion-frame' for r in rows)!=60 or
            not any(r.get('operation')=='captured-motion-pose' and r.get('exact') is True for r in rows)):
            errors.append('incomplete motion/replay')
        if not any(r['stage']=='selected-source' and r.get('source')==source for r in rows):
            errors.append('copied source mismatch')
        if not rows or rows[-1]['stage']!='exit':errors.append('missing exit record')
        if process.process.returncode!=0:errors.append('nonzero process exit')
    if process.partial:errors.append('truncated record')
    if any(r.get('exact') is False for r in rows):errors.append('replay mismatch')
    mapping=protocol_output((process.folder/'wayland.log').read_text(errors='replace')) if ready else None
    return dict(status='failed' if errors else 'passed',errors=errors,
        seconds=(process.ended if process.closed else time.monotonic())-process.started,peakRSS=process.peak,
        exit=process.process.returncode,configuration=expected,wayland=mapping,
        captures={p.name:sha(p) for p in process.folder.glob('*.png')})


def verify_identities(directory):
    frozen=load(directory/'freeze.json')
    for name,expected in frozen['identities'].items():
        if sha(Path(name))!=expected:
            raise RuntimeError('frozen input changed: '+name)
    if sha(directory/'schedule.json')!=frozen['scheduleHash']:
        raise RuntimeError('schedule changed')
    return frozen


def freeze(build, directory):
    load(build/'preflight.json')
    load(build/'layout-check.json')
    identities=load(build/'identities.json')
    identities[str(build/'preflight.json')]=sha(build/'preflight.json')
    for p in HERE.iterdir():
        if p.is_file():identities[str(p)]=sha(p)
    for name,h in identities.items():
        if sha(Path(name))!=h:raise RuntimeError('input changed: '+name)
    outputs=read_outputs()
    original=output_scale(outputs,outputs[0]['scale'])
    directory.mkdir(parents=True,exist_ok=False)
    save(directory/'original-output.json',outputs)
    save(directory/'schedule.json',schedule(build,directory))
    save(directory/'freeze.json',dict(build=str(build),identities=identities,
        scheduleHash=sha(directory/'schedule.json'),original=original,
        maxLaunches=690,totalAuthorizedLaunches=691,priorLaunches=1,
        priorAutomatedSeconds=120,automatedSeconds=14400,transitions=TRANSITIONS))


def confirm(directory, scale, label):
    frozen=verify_identities(directory)
    labels={f'fresh-{s:g}':s for s in SCALES}
    labels.update({f'live-{i}':s for i,s in enumerate(TRANSITIONS)})
    if labels.get(label)!=scale:raise ValueError('unexpected confirmation label/scale')
    outputs=read_outputs();output_scale(outputs,scale,frozen['original'])
    folder=directory/'confirmations';folder.mkdir(exist_ok=True)
    save(folder/(label+'.json'),dict(scale=scale,outputs=outputs,userConfirmed=True))


def persist_failure(directory, journal, cell, process, error):
    folder=directory/'attempts'/cell['id']
    folder.mkdir(parents=True,exist_ok=True)
    path=folder/'result.json'
    if path.exists():
        # A completed result is immutable; preserve a later failure separately.
        save(folder/'post-validation-failure.json',dict(error=str(error)))
        return
    facts=load(folder/'process-facts.json') if (folder/'process-facts.json').exists() else dict(
        exit=None,seconds=120,peakRSS=None,captures={},durationBasis='conservative interruption reservation')
    result=dict(facts,status='failed',errors=[str(error) or type(error).__name__],
        failureKind='interrupted' if isinstance(error,KeyboardInterrupt) else 'supervisor-or-validation')
    save(path,result)
    journal.finish(cell,result)


def fresh(directory, scale):
    frozen=verify_identities(directory);journal=Journal(directory)
    if scale not in SCALES:raise ValueError('unscheduled scale')
    if any(not (directory/f'fresh-{s:g}-complete.json').exists() for s in SCALES[:SCALES.index(scale)]):
        raise ValueError('previous scale set incomplete')
    confirmation=load(directory/'confirmations'/f'fresh-{scale:g}.json')
    if confirmation['scale']!=scale:raise ValueError('confirmation mismatch')
    output_scale(read_outputs(),scale,frozen['original'])
    cells=load(directory/'schedule.json');manifest=load(Path(frozen['build'])/'fixtures.json')
    for cell in cells:
        if cell['kind']!='fresh' or cell['scale']!=scale or cell['id'] in journal.started:continue
        if cell['cycle']:
            parent=next(c for c in cells if c['scale']==scale and c['config']==cell['config'] and c['fixture']=='document' and c['cycle']==cell['cycle']-1)
            if journal.finished.get(parent['id'],{}).get('status')!='passed':
                if not any(r['event']=='skip' and r['id']==cell['id'] for r in journal.events):
                    journal.append('skip',id=cell['id'],reason='preceding document attempt failed/interrupted')
                continue
        verify_identities(directory)
        output_scale(read_outputs(),scale,frozen['original'])
        journal.start(cell);p=None
        try:
            p=Process(cell,directory)
            while not (p.eof and p.process.poll() is not None):
                p.pump();now=time.monotonic()
                failure=limits(now,p.started,p.deadline,p.peak)
                if journal.seconds+now-p.started>14400:
                    raise RuntimeError('whole-study execution limit')
                if failure or p.errors:
                    p.errors.append(failure or 'malformed output');break
                time.sleep(.01)
            p.stop()
            result=validate(p,manifest)
            output_scale(read_outputs(),scale,frozen['original'])
            save(p.folder/'result.json',result);journal.finish(cell,result)
            print(cell['id'],cell['config']['id'],cell['fixture'],result['status'],result['errors'],flush=True)
        except BaseException as exc:
            if p:p.stop()
            persist_failure(directory, journal, cell, p, exc)
            if not isinstance(exc, KeyboardInterrupt):
                journal.append('stop',reason=str(exc),id=cell['id'])
            raise
    verify_identities(directory)
    marker=directory/f'fresh-{scale:g}-complete.json'
    if not marker.exists():save(marker,dict(scale=scale,finished=True))


def live(directory):
    frozen=verify_identities(directory);journal=Journal(directory)
    if not all((directory/f'fresh-{s:g}-complete.json').exists() for s in SCALES):
        raise RuntimeError('fresh matrix sets incomplete')
    output_scale(read_outputs(),2,frozen['original'])
    cells=[c for c in load(directory/'schedule.json') if c['kind']=='live']
    manifest=load(Path(frozen['build'])/'fixtures.json')
    processes=[];results={};states={};automatic=0.;active_start=time.monotonic()

    def tick(active):
        for proc in processes:
            if proc.closed:continue
            proc.pump()
            if proc.peak>4*GIB:raise RuntimeError('live per-process memory ceiling')
        if sum(rss(p.process.pid) for p in processes if not p.closed)>8*GIB:
            raise RuntimeError('live combined memory ceiling')
        if journal.seconds+automatic+(time.monotonic()-active_start if active else 0)>14400:
            raise RuntimeError('whole-study execution ceiling')
        time.sleep(.01)

    try:
        for cell in cells:
            if cell['id'] in journal.started:continue
            journal.start(cell);proc=Process(cell,directory);processes.append(proc)
            while not proc.ready and proc.process.poll() is None and not proc.errors:
                tick(True)
                failure=limits(time.monotonic(),proc.started,proc.deadline,proc.peak,False)
                if failure:proc.errors.append(failure);break
            if not proc.ready or proc.errors or proc.process.poll() is not None:
                proc.stop();results[cell['id']]=validate(proc,manifest,True)
                results[cell['id']]['status']='failed'
                results[cell['id']]['errors'].append('live startup failed')
        automatic+=time.monotonic()-active_start
        for index,scale in enumerate(TRANSITIONS):
            label=f'live-{index}'
            confirmation=directory/'confirmations'/(label+'.json')
            if index:
                print(json.dumps(dict(waitingFor=label,scale=scale)),flush=True)
                while not confirmation.exists():tick(False)
                if load(confirmation)['scale']!=scale:raise RuntimeError('live confirmation mismatch')
            active_start=time.monotonic()
            output_scale(read_outputs(),scale,frozen['original']);verify_identities(directory)
            for proc in processes:
                if proc.closed:continue
                if proc.process.poll() is not None:
                    proc.errors.append('window exited during human wait');proc.stop()
                    results[proc.cell['id']]=validate(proc,manifest,True)
                    results[proc.cell['id']]['status']='failed';continue
                before=len(proc.records);proc.send('checkpoint '+label)
                while not any(r['stage']=='checkpoint' and r.get('id')==label for r in proc.records[before:]):
                    tick(True)
                    failure=limits(time.monotonic(),proc.started,proc.deadline,proc.peak,False)
                    if failure or proc.process.poll() is not None or proc.errors:
                        proc.errors.append(failure or 'checkpoint process failure');break
                rows=proc.records[before:]
                checkpoint=next((r for r in rows if r['stage']=='checkpoint'),None)
                if checkpoint is None:
                    proc.stop();results[proc.cell['id']]=validate(proc,manifest,True)
                    results[proc.cell['id']]['status']='failed';continue
                mapping=next((r for r in rows if r['stage']=='mapping'),{})
                check=next((r for r in rows if r['stage']=='study-check'),{})
                path=proc.folder/('checkpoint-'+label+'.png')
                state=dict(mapping=mapping,check=check,capture=sha(path),scale=scale)
                prior=states.get((proc.cell['id'],scale))
                if prior:
                    # Remove only labels; geometry, selections, mappings and pixels must repeat.
                    clean=lambda r:{k:v for k,v in r.items() if k not in ('reason','checkpoint','monotonicMs')}
                    state['repeatPassed']=(clean(mapping)==clean(prior['mapping']) and
                        clean(check)==clean(prior['check']) and state['capture']==prior['capture'])
                    if not state['repeatPassed']:proc.errors.append('repeated-scale state mismatch')
                states[(proc.cell['id'],scale)]=state
                save(proc.folder/(label+'.json'),state)
                print(proc.cell['id'],label,'checkpoint',flush=True)
            output_scale(read_outputs(),scale,frozen['original'])
            automatic+=time.monotonic()-active_start
            journal.append('live-checkpoint',id=label,scale=scale,automatedSeconds=automatic)
        active_start=time.monotonic()
        for proc in processes:
            if proc.closed:continue
            proc.send('quit')
            while proc.process.poll() is None:
                tick(True)
                if time.monotonic()>proc.deadline:
                    proc.errors.append('shutdown deadline');break
            proc.stop();results[proc.cell['id']]=validate(proc,manifest,True)
            if proc.process.returncode!=0:
                results[proc.cell['id']]['status']='failed'
                results[proc.cell['id']]['errors'].append('nonzero shutdown')
        automatic+=time.monotonic()-active_start
    except BaseException as exc:
        for proc in processes:
            if not proc.closed:proc.errors.append(str(exc));proc.stop()
            if proc.cell['id'] not in results:
                results[proc.cell['id']]=dict(status='failed',errors=proc.errors)
        journal.append('stop',reason=str(exc),id='live')
        raise
    finally:
        for proc in processes:
            result=results.get(proc.cell['id'],dict(status='failed',errors=['interrupted']))
            facts=load(proc.folder/'process-facts.json')
            result={**facts,**result}
            result['processWallSeconds']=facts['seconds']
            result['seconds']=automatic/max(1,len(processes))
            result['humanWaitExcluded']=True
            if not (proc.folder/'result.json').exists():
                save(proc.folder/'result.json',result);journal.finish(proc.cell,result)
    save(directory/'live-complete.json',dict(automatedSeconds=automatic))


def main():
    p=argparse.ArgumentParser(description=__doc__);sub=p.add_subparsers(dest='action',required=True)
    f=sub.add_parser('freeze');f.add_argument('build',type=Path);f.add_argument('directory',type=Path)
    c=sub.add_parser('confirm');c.add_argument('directory',type=Path);c.add_argument('scale',type=float);c.add_argument('label')
    f=sub.add_parser('fresh');f.add_argument('directory',type=Path);f.add_argument('scale',type=float)
    f=sub.add_parser('live');f.add_argument('directory',type=Path)
    a=p.parse_args()
    if a.action=='freeze':freeze(a.build.resolve(),a.directory.resolve())
    elif a.action=='confirm':confirm(a.directory.resolve(),a.scale,a.label)
    elif a.action=='fresh':fresh(a.directory.resolve(),a.scale)
    elif a.action=='live':live(a.directory.resolve())


if __name__=='__main__':
    main()
