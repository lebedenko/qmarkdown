#!/usr/bin/env python3
"""Archive bounded isolation evidence, including terminal runs; read back all bytes."""
import gzip
import io
import json
import shutil
import hashlib
from pathlib import Path
import sys
import tarfile
import tempfile
import run as p
import report


def package(output,destination):
    with p.a.no_launches():
        result=report.report(output)
        if destination.exists():
            p.require({x.name for x in destination.iterdir()}=={'decision.json','report.md'},'only staged reports permitted')
            for name in ('decision.json','report.md'):
                p.require((destination/name).read_bytes()==(output/name).read_bytes(),'staged report differs')
        files={str(x.relative_to(output)):x for x in output.rglob('*')
               if x.is_file() and '/inputs/' not in str(x.relative_to(output)) and '/refinement/' not in str(x.relative_to(output))}
        for x in p.WORK.iterdir():
            if x.is_file():files['build/'+x.name]=x
        for label,path in p.ORIGINAL.items():files['binary/'+label+'-original']=path
        for label,path in p.PREPARED.items():files['binary/'+label+'-prepared']=path
        for label in ('C','I'):
            for name in ('scan-tests','numeric-tests','fast-math-tests'):
                files['binary/'+label+'-'+name]=p.WORK/(label+'-diagnostic-build')/name
        for folder in ('isolated-sources','C-diagnostic-driver','I-diagnostic-driver'):
            for x in (p.WORK/folder).rglob('*'):
                if x.is_file() and '__pycache__' not in x.parts:files['build/'+str(x.relative_to(p.WORK))]=x
        for folder in ('isolated','C-diagnostic','I-diagnostic'):
            root=p.WORK/folder
            for x in (root/'modules/skparagraph/src').iterdir():
                if x.is_file():files['build/'+folder+'/sources/'+x.name]=x
            for name in ('libskparagraph.a','args.gn','build.ninja','toolchain.ninja','obj/modules/skparagraph/skparagraph.ninja'):
                x=root/'out/comparison'/name
                if x.is_file():files['build/'+folder+'/recipe/'+name]=x
        for folder in ('C-prepared-build','I-prepared-build','I-original-build','C-diagnostic-build','I-diagnostic-build'):
            for name in ('CMakeCache.txt','build.ninja','CMakeFiles/rules.ninja'):
                x=p.WORK/folder/name
                if x.is_file():files['build/'+folder+'/'+name]=x
        files.update({'tool/'+x.name:x for x in p.HERE.iterdir() if x.is_file()})
        for name in ('requirements.md','design.md','tasks.md','verification.md','candidate-scan-recommendation.md','prepared-path-isolation-proposal.md'):
            files['specification/'+name]=p.FEATURE/name
        destination.mkdir(parents=True,exist_ok=True)
        archive=destination/'073-candidate-prepared-isolation.tar.gz'
        hashes={name:p.digest(path) for name,path in sorted(files.items())}
        with archive.open('wb') as raw,gzip.GzipFile(fileobj=raw,mode='wb',filename='',mtime=0) as zipped:
            with tarfile.open(fileobj=zipped,mode='w') as tar:
                for name,path in sorted(files.items()):
                    data=path.read_bytes();member=tarfile.TarInfo(name);member.size=len(data);member.mode=0o644
                    tar.addfile(member,io.BytesIO(data))
        p.write(destination/'archives.json',dict(archives={archive.name:dict(sha256=p.digest(archive),bytes=archive.stat().st_size,files=hashes)}))
        with tempfile.TemporaryDirectory() as tmp:members=p.a.diagnosis.verify_archives(destination,Path(tmp))
        for name in ('decision.json','report.md'):(destination/name).write_bytes((output/name).read_bytes())
        with (output/'analysis.json').open('rb') as source, (destination/'analysis.json.gz').open('wb') as raw:
            with gzip.GzipFile(fileobj=raw,mode='wb',filename='',mtime=0) as compressed:
                shutil.copyfileobj(source,compressed)
        h=hashlib.sha256()
        with gzip.open(destination/'analysis.json.gz','rb') as stream:
            for chunk in iter(lambda:stream.read(1024*1024),b''):h.update(chunk)
        p.require(h.hexdigest()==p.digest(output/'analysis.json'),'compressed analysis read-back differs')
        p.a.diagnosis.verify_identities(json.loads((p.WORK/'input-identities.json').read_text()))
        print('Archive read-back:',len(members),'member hashes verified; historical inputs unchanged; complete:',result['complete'])


if __name__=='__main__':package(Path(sys.argv[1]).resolve(),Path(sys.argv[2]).resolve())
