"""Exact pinned-source diagnostic patch; no layout decisions are changed."""
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
           '    void resetPhaseWork() const { phaseWork[0]={}; phaseWork[1]={}; phaseCandidates=phaseWhitespace=0; }\n'
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
        'for (const auto& span : fExperimentalSpans) {': 'for (const auto& span : fExperimentalSpans) {\n            ++work.ownerComparisons;',
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
