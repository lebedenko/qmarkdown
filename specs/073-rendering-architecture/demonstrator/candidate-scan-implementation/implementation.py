"""Checked clean transformation and a separate, reproducible diagnostic overlay."""
from pathlib import Path
import difflib
import importlib.util
import shutil
import tempfile

HERE = Path(__file__).resolve().parent


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def diff(before, after):
    result = []
    for path in sorted(after.iterdir()):
        if not path.is_file():
            continue
        prior = before/path.name
        a = prior.read_text() if prior.exists() else ''
        b = path.read_text()
        if a != b:
            relative = 'modules/skparagraph/src/'+path.name
            result.extend(difflib.unified_diff(a.splitlines(True), b.splitlines(True),
                          fromfile='a/'+relative, tofile='b/'+relative))
    return ''.join(result)


def diagnostic(root):
    """Proof overlay with guarded-path placement and recoverable final addition."""
    load('owner_overlay', HERE.parent/'ownership-experiment/instrument.py').instrument(root)
    load('proof_overlay', HERE.parent/'candidate-scan-experiment/prototype.py').apply(root)
    src = root/'modules/skparagraph/src'
    path = src/'TextWrapper.cpp'
    text = path.read_text()
    a = text.index('    auto guard = scan_bound::platform<SkScalar>();')
    b = text.index('    while (fEndLine.endCluster() != end) {', a)
    guards = text[a:b]
    guards = guards.replace('    auto guard = scan_bound::platform<SkScalar>();',
        '        ++parent->scanWork.guardVisits;\n        guard = scan_bound::platform<SkScalar>();')
    guards = guards.replace('parent->experimentalPadded() && parent->scanMode && ', 'parent->scanMode && ')
    guards = guards.replace('        if (scratch.capacity()>span.size()) { std::fprintf(stderr,"cutoff-proof scratch capacity exceeds paragraph\\n"); std::abort(); };\n', '')
    text = text[:a]+'''    auto guard = scan_bound::safe;
    long double padEnvelope = 0;
    if (parent->experimentalPadded()) {
'''+guards.replace('    const long double padEnvelope=', '    padEnvelope=')+'    }\n'+text[b:]
    text = text.replace('if (!scan_bound::finite(required)) { std::fprintf(stderr,"cutoff-proof nonfinite required\\n"); std::abort(); };',
        'if (!scan_bound::finite(required)) { reason=scan_bound::intermediate; break; }')
    # Late fallback is executable even though the conservative envelope prevents it
    # on supported inputs. No partially selected endpoint may escape.
    text = text.replace('''            } else {
            for (auto candidate''', '''            }
            if (!parent->scanMode || reason!=scan_bound::safe || parent->scanMode==1) {
                bare=0;
                legalEnd=fallbackEnd=firstUnitEnd=nullptr;
            for (auto candidate''')
    text = text.replace('            if (parent->scanMode && reason!=scan_bound::safe) ++parent->scanWork.fallback[reason];\n', '')
    text = text.replace('                bare=0;', '                if (parent->scanMode && reason!=scan_bound::safe) ++parent->scanWork.fallback[reason];\n                bare=0;')
    # Keep the padded-only preparation block readable in both artifacts.
    start = text.index('    SkScalar maxPad=0;')
    stop = text.index('    while (fEndLine.endCluster() != end)', start)
    block = text[start:stop]
    lines = block.splitlines(True)
    text = text[:start]+''.join('    '+line for line in lines[:-1])+lines[-1]+text[stop:]
    path.write_text(text)
    path = src/'ParagraphImpl.h'
    path.write_text(path.read_text().replace('size_t preprocessing=0,', 'size_t guardVisits=0, preprocessing=0,'))


def apply(root):
    """Retain the proven algorithm, removing every ownership/scan diagnostic."""
    src = root/'modules/skparagraph/src'
    # Generate diagnostic code from the verified baseline, then retain only the
    # scanner body. ParagraphImpl and its ownership implementation stay untouched.
    with tempfile.TemporaryDirectory() as temporary:
        stage = Path(temporary)
        target = stage/'modules/skparagraph/src'
        shutil.copytree(src, target)
        diagnostic(stage)
        text = (target/'TextWrapper.cpp').read_text()
    text = text.replace('#include <cstdio>\n#include <cstdlib>\n', '')
    text = text.replace('        ++parent->scanWork.guardVisits;\n', '')
    text = text.replace('parent->scanMode && ', '')
    text = text.replace('''        parent->scanWork.scratchCapacity=scratch.capacity();
        parent->scanWork.scratchBytes=scratch.capacity()*sizeof(Candidate);
''', '')
    text = text.replace('''                    ++parent->scanWork.preprocessing;
''', '').replace('''                        ++parent->scanWork.trimming;
''', '')
    text = text.replace('''                parent->scanWork.scratchSize=std::max(parent->scanWork.scratchSize,scratch.size());
''', '').replace('''                    parent->scanWork.suffixComparisons+=scratch.empty()?0:scratch.size()-1;
''', '')
    text = text.replace('''            ++parent->scanWork.lines;
            if (parent->scanMode && reason!=scan_bound::safe) ++parent->scanWork.fallback[reason];
''', '')
    # The earlier selector removal also changes the fallback counter condition.
    text = text.replace('''            ++parent->scanWork.lines;
            if (reason!=scan_bound::safe) ++parent->scanWork.fallback[reason];
''', '')
    text = text.replace('                if (reason!=scan_bound::safe) ++parent->scanWork.fallback[reason];\n', '')
    text = text.replace('parent->scanMode==2 && ', '')
    text = text.replace('''                        parent->scanWork.skipped+=scratch.size()-i;
''', '')
    for line in ('                    ++parent->phaseCandidates;\n',
                 '                    ++parent->scanWork.evaluations;\n',
                 '                    parent->phaseFitting=true;\n',
                 '                    parent->phaseFitting=false;\n',
                 '                ++parent->phaseCandidates;\n',
                 '                    ++parent->phaseWhitespace;\n',
                 '                ++parent->scanWork.evaluations;\n',
                 '                parent->phaseFitting = true;\n',
                 '                parent->phaseFitting = false;\n'):
        text = text.replace(line, '')
    text = text.replace('!parent->scanMode || reason!=scan_bound::safe || parent->scanMode==1', 'reason!=scan_bound::safe')
    a = text.index('            if (parent->scanMode==1 && reason==scan_bound::safe) {')
    b = text.index('            selectedEnd = legalEnd', a)
    text = text[:a]+text[b:]
    text = ''.join(line for line in text.splitlines(True) if 'parent->scanWork.' not in line)
    for forbidden in ('scanMode', 'scanWork', 'phaseWork', 'phaseCandidates', 'phaseFitting', 'phaseWhitespace', 'cutoff-proof', 'abort(', 'fprintf('):
        assert forbidden not in text, forbidden
    (src/'TextWrapper.cpp').write_text(text)
    shutil.copy2(HERE/'ScanBound.h', src/'ScanBound.h')
