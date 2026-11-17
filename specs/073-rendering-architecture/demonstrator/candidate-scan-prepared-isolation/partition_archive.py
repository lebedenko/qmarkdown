#!/usr/bin/env python3
"""Repartition verified evidence using the historical 64 MiB member grouping rule."""
import gzip
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import run as p


def partition(source,destination):
    with p.a.no_launches():
        p.require(not destination.exists(),'fresh multipart destination required')
        index=json.loads((source/'archives.json').read_text())['archives']
        p.require(len(index)==1,'one source archive required')
        name,record=next(iter(index.items()))
        p.a.diagnosis.check_hash(source/name,record['sha256'])
        destination.mkdir(parents=True)
        records={}
        with tarfile.open(source/name,'r:gz') as original:
            members=original.getmembers()
            p.require({m.name for m in members}==set(record['files']),'source member set')
            groups=[];group=[];size=0
            for member in members:
                if group and size+member.size>64*1024**2:
                    groups.append(group);group=[];size=0
                group.append(member);size+=member.size
            if group:groups.append(group)
            for i,group in enumerate(groups,1):
                archive=destination/f'073-candidate-prepared-isolation-part-{i:03}.tar.gz'
                expected={m.name:record['files'][m.name] for m in group}
                with archive.open('wb') as raw,gzip.GzipFile(fileobj=raw,mode='wb',filename='',mtime=0) as compressed:
                    with tarfile.open(fileobj=compressed,mode='w') as tar:
                        for member in group:
                            with original.extractfile(member) as data:tar.addfile(member,data)
                        if i==len(groups):
                            tool=Path(__file__).resolve();data=tool.read_bytes()
                            member=tarfile.TarInfo('tool/'+tool.name);member.size=len(data);member.mode=0o644
                            tar.addfile(member,io.BytesIO(data));expected[member.name]=p.digest(tool)
                records[archive.name]=dict(sha256=p.digest(archive),bytes=archive.stat().st_size,files=expected)
        p.write(destination/'archives.json',dict(archives=records,sourceArchiveSha256=record['sha256']))
        with tempfile.TemporaryDirectory() as tmp:verified=p.a.diagnosis.verify_archives(destination,Path(tmp))
        p.require({k:v for k,v in verified.items() if k!='tool/'+Path(__file__).name}==record['files'],'partition changed evidence')
        print('Multipart read-back:',len(verified),'member hashes in',len(records),'archives; original evidence bytes unchanged.')


if __name__=='__main__':partition(Path(sys.argv[1]).resolve(),Path(sys.argv[2]).resolve())
