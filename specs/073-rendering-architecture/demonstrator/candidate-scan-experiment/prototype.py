"""Apply only to fresh, ownership-verified and instrumented private sources."""
from pathlib import Path
import shutil
HERE = Path(__file__).resolve().parent


def apply(root):
    src = root/'modules/skparagraph/src'
    shutil.copy2(HERE/'ScanBound.h', src/'ScanBound.h')
    def change(name, before, after):
        p=src/name;s=p.read_text();assert s.count(before)==1,(name,before);p.write_text(s.replace(before,after))
    change('ParagraphImpl.h','    mutable PhaseWork phaseWork[2];', '''    // Private driver state: 0 oracle, 1 shadow, 2 prototype (historical gates).
    int scanMode=2;
    struct ScanWork {
        size_t preprocessing=0, trimming=0, suffixComparisons=0, evaluations=0,
               skipped=0, shadowChecks=0, shadowTrimming=0, scratchSize=0,
               scratchCapacity=0, scratchBytes=0, lines=0;
        size_t fallback[7]={};
    };
    mutable ScanWork scanWork;
    mutable PhaseWork phaseWork[2];''')
    change('ParagraphImpl.h','void resetPhaseWork() const {','void resetPhaseWork() const { scanWork={};')
    change('TextWrapper.cpp','#include "modules/skparagraph/src/TextWrapper.h"','#include "modules/skparagraph/src/TextWrapper.h"\n#include "modules/skparagraph/src/ScanBound.h"\n#include <cstdio>\n#include <cstdlib>')
    change('TextWrapper.cpp','    while (fEndLine.endCluster() != end) {', '''    struct Candidate { Cluster* end; Cluster* trimmedEnd; SkScalar trimmed, lower; bool eligible; };
    std::vector<Candidate> scratch; // Layout-local, released after wrapping.
    auto guard = scan_bound::platform<SkScalar>();
    SkScalar maxPad=0;
    if (!scan_bound::finite(maxWidth)) guard=scan_bound::width;
    for (const auto& s : parent->experimentalSpans()) {
        if (!scan_bound::finite(s.horizontalPadding) || s.horizontalPadding < 0) guard=scan_bound::padding;
        else maxPad=std::max(maxPad,s.horizontalPadding);
    }
    // Conservative envelope keeps every slice sum and required addition finite.
    // Each run emits at most one slice; at most two pads per slice. Factor four
    // slack covers rounding even for the largest representable vector size.
    const long double padEnvelope=4.L*maxPad*(parent->runs().size()+1.L);
    if (padEnvelope > std::numeric_limits<SkScalar>::max()/4.L) guard=scan_bound::intermediate;
    if (parent->experimentalPadded() && parent->scanMode && guard==scan_bound::safe) {
        scratch.reserve(span.size());
        parent->scanWork.scratchCapacity=scratch.capacity();
        parent->scanWork.scratchBytes=scratch.capacity()*sizeof(Candidate);
        if (scratch.capacity()>span.size()) { std::fprintf(stderr,"cutoff-proof scratch capacity exceeds paragraph\\n"); std::abort(); };
    }
    while (fEndLine.endCluster() != end) {''')
    path=src/'TextWrapper.cpp';s=path.read_text()
    a=s.index('            for (auto candidate = lineStart; candidate < end; ++candidate) {')
    b=s.index('            selectedEnd = legalEnd',a)
    original=s[a:b]
    # Keep oracle/fallback loop byte-for-byte except explicit evaluation counter.
    exhaustive=original.replace('                parent->phaseFitting = true;', '                ++parent->scanWork.evaluations;\n                parent->phaseFitting = true;')
    replacement='''            auto reason=guard;
            scratch.clear();
            if (parent->scanMode && reason==scan_bound::safe) {
                SkScalar sum=0;
                for (auto c=lineStart; c<end; ++c) {
                    ++parent->scanWork.preprocessing;
                    sum += c->width();
                    if (!scan_bound::finite(c->width()) || !scan_bound::finite(sum)) reason=scan_bound::intermediate;
                    auto t=c+1;
                    SkScalar trimmed=sum;
                    while (t>lineStart && (t-1)->isWhitespaceBreak()) {
                        ++parent->scanWork.trimming;
                        --t; trimmed -= t->width();
                        if (!scan_bound::finite(trimmed)) reason=scan_bound::intermediate;
                    }
                    if (!scan_bound::finite(trimmed) ||
                        std::abs(static_cast<long double>(trimmed)) + padEnvelope >
                            std::numeric_limits<SkScalar>::max()/2.L) reason=scan_bound::intermediate;
                    const auto& edges=parent->fExperimentalGraphemeEdges;
                    bool eligible=std::binary_search(edges.begin(),edges.end(),c->textRange().end);
                    if (eligible && !firstUnitEnd) firstUnitEnd=c+1;
                    scratch.push_back({c+1,t,trimmed,trimmed,eligible});
                    if (c->isHardBreak()) break;
                }
                parent->scanWork.scratchSize=std::max(parent->scanWork.scratchSize,scratch.size());
                if (reason==scan_bound::safe) {
                    scan_bound::suffix(scratch);
                    parent->scanWork.suffixComparisons+=scratch.empty()?0:scratch.size()-1;
                }
            }
            ++parent->scanWork.lines;
            if (parent->scanMode && reason!=scan_bound::safe) ++parent->scanWork.fallback[reason];
            if (parent->scanMode==2 && reason==scan_bound::safe) {
                for (size_t i=0;i<scratch.size();++i) {
                    if (scan_bound::skip(scratch[i].lower,maxWidth)) {
                        parent->scanWork.skipped+=scratch.size()-i;
                        break;
                    }
                    const auto& e=scratch[i];
                    ++parent->phaseCandidates;
                    ++parent->scanWork.evaluations;
                    TextRange range(lineStart->textRange().start,
                        e.trimmedEnd>lineStart ? (e.trimmedEnd-1)->textRange().end : lineStart->textRange().start);
                    parent->phaseFitting=true;
                    auto required=e.trimmed+parent->experimentalPadding(range);
                    parent->phaseFitting=false;
                    if (!scan_bound::finite(required)) { std::fprintf(stderr,"cutoff-proof nonfinite required\\n"); std::abort(); };
                    if (required<=maxWidth && e.eligible) {
                        fallbackEnd=e.end;
                        if ((e.end-1)->isSoftBreak() || (e.end-1)->isHardBreak() || e.end==end) legalEnd=e.end;
                    }
                }
            } else {
''' + exhaustive + '''            }
            if (parent->scanMode==1 && reason==scan_bound::safe) {
                bool skipping=false;
                for (const auto& e:scratch) {
                    skipping=skipping || scan_bound::skip(e.lower,maxWidth);
                    if (!skipping) continue;
                    ++parent->scanWork.skipped;
                    ++parent->scanWork.shadowChecks;
                    TextRange range(lineStart->textRange().start,
                        e.trimmedEnd>lineStart ? (e.trimmedEnd-1)->textRange().end : lineStart->textRange().start);
                    auto saved0=parent->phaseWork[0];auto saved1=parent->phaseWork[1];
                    auto savedLookups=parent->ownerLookups;
                    auto required=e.trimmed+parent->experimentalPadding(range);
                    parent->phaseWork[0]=saved0;parent->phaseWork[1]=saved1;parent->ownerLookups=savedLookups;
                    if (!scan_bound::finite(required) || !(required>maxWidth))
                        { std::fprintf(stderr,"cutoff-proof shadow failure\\n"); std::abort(); };
                }
            }
'''
    path.write_text(s[:a]+replacement+s[b:])
