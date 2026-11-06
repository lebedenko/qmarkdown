"""Private ownership diagnostics, applied after the checked optimization patch."""
from pathlib import Path

FIELDS = ('calls', 'runVisits', 'ownerComparisons', 'reorderCalls', 'slices')


def instrument(root):
    src = Path(root)/'modules/skparagraph/src'
    def change(name, before, after):
        path = src/name
        text = path.read_text()
        assert text.count(before) == 1, (name, before)
        path.write_text(text.replace(before, after))
    change('ParagraphImpl.h', 'size_t experimentalShapeGeneration() const',
           'struct PhaseWork { size_t calls=0, runVisits=0, ownerComparisons=0, reorderCalls=0, slices=0; };\n'
           '    mutable PhaseWork phaseWork[2];\n'
           '    mutable size_t phaseCandidates=0, phaseWhitespace=0;\n'
           '    mutable bool phaseFitting=false;\n'
           '    mutable size_t ownerBuilds=0, ownerExaminations=0, ownerLookups=0;\n'
           '    size_t ownerCapacity() const { return fRunOwners.capacity(); }\n'
           '    size_t ownerPayload() const { return fRunOwners.size()*sizeof(size_t); }\n'
           '    void resetPhaseWork() const { phaseWork[0]={}; phaseWork[1]={}; phaseCandidates=phaseWhitespace=0; ownerBuilds=ownerExaminations=ownerLookups=0; }\n'
           '    size_t experimentalShapeGeneration() const')
    path = src/'ParagraphImpl.cpp'
    text = path.read_text()
    start = text.index('ParagraphImpl::experimentalVisualSlices(TextRange range) const {')
    end = text.index('\nSkScalar ParagraphImpl::experimentalPadding', start)
    body = text[start:end]
    replacements = {
        '    std::vector<size_t> indexes;': '    auto& work = phaseWork[phaseFitting ? 0 : 1];\n    ++work.calls;\n    std::vector<size_t> indexes;',
        'for (const auto& run : fRuns) {': 'for (const auto& run : fRuns) {\n        ++work.runVisits;',
        'if (!order.empty()) fUnicode->reorderVisual(levels.data(), levels.size(), order.data());': 'if (!order.empty()) { ++work.reorderCalls; fUnicode->reorderVisual(levels.data(), levels.size(), order.data()); }',
        'slices.push_back({index, identity, 0, 0});': '++work.slices;\n        slices.push_back({index, identity, 0, 0});'}
    for before, after in replacements.items():
        assert body.count(before) == 1
        body = body.replace(before, after)
    path.write_text(text[:start]+body+text[end:])
    change('TextWrapper.cpp', 'for (auto candidate = lineStart; candidate < end; ++candidate) {',
           'for (auto candidate = lineStart; candidate < end; ++candidate) {\n                ++parent->phaseCandidates;')
    change('TextWrapper.cpp', '                    --trimmedEnd;',
           '                    ++parent->phaseWhitespace;\n                    --trimmedEnd;')
    change('TextWrapper.cpp', 'auto required = trimmedWidth + parent->experimentalPadding(range);',
           'parent->phaseFitting = true;\n                auto required = trimmedWidth + parent->experimentalPadding(range);\n                parent->phaseFitting = false;')

    change('ParagraphImpl.cpp', '    std::vector<size_t> owners(fRuns.size(), EMPTY_INDEX);',
           '    ++ownerBuilds;\n    std::vector<size_t> owners(fRuns.size(), EMPTY_INDEX);')
    change('ParagraphImpl.cpp', '            const auto& span = fExperimentalSpans[j];',
           '            ++ownerExaminations;\n            const auto& span = fExperimentalSpans[j];')
    change('ParagraphImpl.cpp', '    if (fOwnersValid && fOwnerRevision == fRunRevision &&',
           '    ++ownerLookups;\n    if (fOwnersValid && fOwnerRevision == fRunRevision &&')
    change('ParagraphImpl.cpp', '        const auto& span = fExperimentalSpans[j];\n        if (span.source.start',
           '        ++phaseWork[phaseFitting ? 0 : 1].ownerComparisons;\n        const auto& span = fExperimentalSpans[j];\n        if (span.source.start')
