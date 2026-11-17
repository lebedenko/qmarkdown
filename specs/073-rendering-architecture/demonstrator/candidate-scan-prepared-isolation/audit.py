"""Conservative audit of this compiler's final wrapper control flow."""
import re
from pathlib import Path
import prepare_build as b


def audit(text):
    parts=re.split(r'\n(?=[0-9a-f]+ <)',text)
    def function(name):
        found=[p for p in parts if 'TextWrapper::'+name+'(' in p.splitlines()[0]
               and '[clone' not in p.splitlines()[0]]
        assert len(found)==1, 'missing/duplicated wrapper'
        return found[0]
    wrapper=function('breakTextIntoLines');helper=function('breakPaddedTextIntoLines')
    for token in ('fegetround','stmxcsr','operator new','operator delete','Candidate','scan_bound'):
        assert token not in wrapper, 'candidate setup/cleanup in false wrapper: '+token
    assert 'fegetround' in helper and 'operator new' in helper and 'operator delete' in helper
    instructions=[]
    for line in wrapper.splitlines()[1:]:
        m=re.match(r'\s*([0-9a-f]+):\s+(?:[0-9a-f]{2} )+\s*([a-z][a-z0-9]*)\s*(.*)',line)
        if m:instructions.append((int(m[1],16),m[2],m[3]))
    pc=next(i for i,v in enumerate(instructions) if v[1]=='call')
    assert 'experimentalPadded() const' in instructions[pc][2], 'first call is not predicate'
    branch=next(i for i in range(pc+1,len(instructions)) if instructions[i][1].startswith('j'))
    assert instructions[branch][1]=='jne' and any(x[1]=='test' and x[2]=='%al,%al' for x in instructions[pc+1:branch]), 'predicate dispatch changed'
    true=int(instructions[branch][2].split()[0],16)
    target=next(i for i,v in enumerate(instructions) if v[0]==true)
    transfers=[i for i,v in enumerate(instructions) if 'breakPaddedTextIntoLines(' in v[2]]
    assert len(transfers)==1 and instructions[transfers[0]][1]=='jmp', 'helper not one out-of-line tail dispatch'
    assert target<=transfers[0] and transfers[0]-target<30, 'unexpected true branch structure'
    # Only the entry predicate may branch into the helper dispatch sequence.
    targets={v[0] for v in instructions[target:transfers[0]+1]}
    for i,v in enumerate(instructions):
        if v[1].startswith('j') and i not in range(target,transfers[0]+1):
            dest=v[2].split()[0]
            if re.fullmatch('[0-9a-f]+',dest) and int(dest,16) in targets:
                assert i==branch, 'false path reaches padded helper dispatch'
    assert instructions[target-1][1] in ('ret','jmp'), 'false fallthrough into helper dispatch'
    return dict(passed=True,entryPredicate=hex(instructions[pc][0]),trueBranch=hex(true),
                helperTransfer=hex(instructions[transfers[0]][0]),falseWrapperHasNoCandidateSetup=True,
                repeatedPredicateCalls=sum('experimentalPadded() const' in v[2] for v in instructions),
                wrapper=wrapper,helper=helper)


if __name__=='__main__':
    try:
        results={label:audit((b.WORK/(label+'-disassembly.txt')).read_text()) for label in ('original','prepared')}
        for label,result in results.items():
            for name in ('wrapper','helper'):
                (b.WORK/(label+'-'+name+'.asm')).write_text(result.pop(name))
        b.p.write(b.WORK/'compiled-audit.json',dict(passed=True,results=results,
            limitation='Source equality and final direct-control-flow audit; no claim of baseline instruction identity or isolated stack-only causality.'))
    except Exception as ex:
        b.p.write(b.WORK/'failure.json',dict(status='incomplete',stage='compiled-audit',error=str(ex),executableLaunches=0))
        raise
