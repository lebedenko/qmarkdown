"""Read-only final preflight and identities; no benchmark launches."""
import json
from pathlib import Path
import prepare_build as b
import audit


def freeze():
    with b.p.a.no_launches():
        b.p.require(not (b.WORK/'new-identities.json').exists(),'already frozen')
        b.p.require(not (b.WORK/'failure.json').exists(),'terminal preflight failure')
        ids=json.loads((b.WORK/'input-identities.json').read_text())
        b.p.a.diagnosis.verify_identities(ids)
        src='modules/skparagraph/src'
        b.split.audit(b.SOURCE/'baseline'/src,b.SOURCE/'retained'/src,b.WORK/'isolated'/src)
        for label in ('original','prepared'):audit.audit((b.WORK/(label+'-disassembly.txt')).read_text())
        # Predicate and zero option are inspected together with actual measured
        # fixtures; metadata mutation changes identity only, preserving padding.
        predicate='''bool ParagraphImpl::experimentalPadded() const {
    for (const auto& span : fExperimentalSpans)
        if (span.horizontalPadding || span.verticalPadding) return true;
    return false;
}'''
        for path in (b.SOURCE/'baseline',b.SOURCE/'retained',b.WORK/'isolated'):
            assert predicate in (path/src/'ParagraphImpl.cpp').read_text()
        trace=(b.HERE.parent/'candidate-scan-continuation/trace.cpp').read_text()
        assert 'if(std::string(argv[2])=="zero")m["padding"]={0,0};' in trace
        assert 'copies.back()[0].identity+=copies.back().size();' in trace
        original=(b.HERE.parent/'boundary-experiment/boundary.h')
        headers=list((b.HERE.parent/'boundary-experiment').glob('*.h'))
        assert any('manifest.at("padding")[0],manifest.at("padding")[1]' in x.read_text() for x in headers)
        fixtures={}
        for case in ('prose-480-4096','dense-240-4096'):
            path=b.p.r.HISTORICAL/(case+'.json')
            blocks=json.loads(path.read_text())['fixtures'][0]['blocks']
            counts=[len(x['spans']) for x in blocks]
            assert (all(n==0 for n in counts) if case.startswith('prose') else all(n>0 for n in counts))
            fixtures[case]=dict(sha256=b.p.digest(path),spanCounts=counts,effectivePadding=[0,0],predicate=False)
        b.p.write(b.WORK/'padding-audit.json',dict(predicate=predicate,fixtures=fixtures,
            metadata='only span identity mutates; zero horizontal and vertical padding preserved',
            sourceProof=True,executableLaunches=0))
        for path in b.WORK.rglob('*'):
            if not path.is_file() or '__pycache__' in path.parts or 'preflight' in path.parts:continue
            if path.suffix in ('.log',) or path.name.endswith('disassembly.txt'):continue
            ids[str(path)]=b.p.digest(path)
        # Include executable and resolved compiler/runtime dependencies from all
        # historical identity sets; they have just been revalidated unchanged.
        for path in b.HERE.iterdir():
            if path.is_file():ids[str(path)]=b.p.digest(path)
        b.p.write(b.WORK/'new-identities.json',ids)
        print('Frozen identities:',len(ids),'measured predicates: false; executable launches: 0')


if __name__=='__main__':freeze()
