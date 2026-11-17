"""Mechanical PI2 split; only the wrapper implementation/declaration may change."""
import hashlib

SIGNATURE = 'void TextWrapper::breakTextIntoLines('
DISPATCH = '''
    if (parent->experimentalPadded()) {
        breakPaddedTextIntoLines(parent, maxWidth, addLine);
        return;
    }
'''


def body(text, signature=SIGNATURE):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return start, opening, end


def split(baseline, candidate):
    bs, bo, be = body(baseline)
    cs, co, ce = body(candidate)
    padded = candidate[cs:ce].replace(SIGNATURE,
        '__attribute__((noinline)) void TextWrapper::breakPaddedTextIntoLines(', 1)
    # Remove only the redundant pre-loop predicate, retaining the inner scope and
    # every operation, local lifetime and repeated per-line predicate verbatim.
    preloop = padded.index('    while (fEndLine.endCluster() != end) {')
    prefix = padded[:preloop]
    assert prefix.count('if (parent->experimentalPadded()) {') == 1
    padded = prefix.replace('if (parent->experimentalPadded()) {', '{', 1) + padded[preloop:]
    wrapper = baseline[bs:bo+1] + DISPATCH + baseline[bo+1:be]
    return candidate[:cs] + wrapper + '\n\n' + padded + candidate[ce:]


def apply(baseline, candidate, target):
    target.write_text(split(baseline.read_text(), candidate.read_text()))
    header = target.with_suffix('.h')
    text = header.read_text()
    assert text.count('\nprivate:\n') == 1
    text = text.replace('\nprivate:\n', '''
private:
    __attribute__((noinline)) void breakPaddedTextIntoLines(
        ParagraphImpl* parent, SkScalar maxWidth, const AddLineToParagraph& addLine);
''', 1)
    header.write_text(text)
    return audit(baseline.parent, candidate.parent, target.parent)


def audit(baseline, candidate, isolated):
    b, c, i = [(p/'TextWrapper.cpp').read_text() for p in (baseline, candidate, isolated)]
    assert i == split(b, c), 'source split differs from exact PI2 transformation'
    bs, bo, be = body(b); ins, ino, ine = body(i)
    assert i[ino+1:ine].removeprefix(DISPATCH) == b[bo+1:be], 'false body differs from B'
    assert i[ino+1:ine].startswith(DISPATCH), 'missing entry dispatch'
    expected = (candidate/'TextWrapper.h').read_text().replace('\nprivate:\n', '''
private:
    __attribute__((noinline)) void breakPaddedTextIntoLines(
        ParagraphImpl* parent, SkScalar maxWidth, const AddLineToParagraph& addLine);
''', 1)
    assert (isolated/'TextWrapper.h').read_text() == expected, 'private declaration changed'
    for path in candidate.iterdir():
        if path.is_file() and path.name not in ('TextWrapper.cpp', 'TextWrapper.h'):
            assert path.read_bytes() == (isolated/path.name).read_bytes(), str(path)
    return dict(falseBodyEqualsBaseline=True, paddedBodyEqualsCandidate=True,
                removedOuterPredicateOnly=True, scanBoundUnchanged=True,
                baselineBodySha256=hashlib.sha256(b[bo+1:be].encode()).hexdigest(),
                isolation='private noinline helper; source equality does not prove compiled isolation')
