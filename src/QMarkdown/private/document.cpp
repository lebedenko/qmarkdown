#include "document.h"
#include "../../../third_party/cmark/symbols.h"
#include <cmark.h>

namespace QMarkdownPrivate {
namespace {
bool space(QChar c) { return c == u' ' || c == u'\t'; }
QString trim(QString s)
{
    qsizetype first = 0, last = s.size();
    while (first < last && space(s[first])) ++first;
    while (last > first && space(s[last - 1])) --last;
    return s.mid(first, last - first);
}
qsizetype indentation(const QString &line)
{
    qsizetype i = 0;
    int column = 0;
    while (i < line.size() && space(line[i])) {
        column += line[i] == u'\t' ? 4 - column % 4 : 1;
        if (column > 3) return -1;
        ++i;
    }
    return i;
}
qsizetype run(const QString &s, qsizetype i, QChar marker)
{
    const auto start = i;
    while (i < s.size() && s[i] == marker) ++i;
    return i - start;
}
QString deindent(const QString &line, int columns)
{
    qsizetype i = 0;
    int column = 0;
    while (i < line.size() && column < columns && space(line[i])) {
        column += line[i] == u'\t' ? 4 - column % 4 : 1;
        ++i;
    }
    return QString(qMax(0, column - columns), u' ') + line.mid(i);
}
int setextLevel(const QString &line, qsizetype indent)
{
    if (indent < 0 || indent == line.size()) return 0;
    const auto marker = line[indent];
    if (marker != u'=' && marker != u'-') return 0;
    return trim(line.mid(indent + run(line, indent, marker))).isEmpty()
        ? (marker == u'=' ? 1 : 2) : 0;
}
bool thematicBreak(const QString &line, qsizetype indent)
{
    if (indent < 0 || indent == line.size()) return false;
    const auto marker = line[indent];
    if (marker != u'*' && marker != u'-' && marker != u'_') return false;
    qsizetype count = 0;
    for (qsizetype i = indent; i < line.size(); ++i) {
        if (line[i] == marker) ++count;
        else if (!space(line[i])) return false;
    }
    return count >= 3;
}
QString decodeInfo(const QString &source)
{
    const auto input = (QStringLiteral("~~~ ") + source + QStringLiteral("\n~~~\n")).toUtf8();
    struct NodeOwner {
        cmark_node *node;
        ~NodeOwner() { if (node) cmark_node_free(node); }
    };
    NodeOwner document{cmark_parse_document(input.constData(), input.size(), CMARK_OPT_DEFAULT)};
    if (!document.node || cmark_node_get_type(document.node) != CMARK_NODE_DOCUMENT) return source;
    auto *code = cmark_node_first_child(document.node);
    if (!code || cmark_node_get_type(code) != CMARK_NODE_CODE_BLOCK
        || cmark_node_next(code) || cmark_node_first_child(code)) return source;
    const auto *info = cmark_node_get_fence_info(code);
    return info ? QString::fromUtf8(info) : source;
}

}
QVector<Block> parse(QString source)
{
    source.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    source.replace(u'\r', u'\n');
    source.replace(QChar(0), QChar(0xfffd));
    QVector<Block> blocks;
    enum class State { Paragraph, FencedCode, IndentedCode };
    State state = State::Paragraph;
    QString paragraph, fence, info, indented, pendingBlanks;
    int fenceIndent = 0;
    QChar marker;
    qsizetype fenceLength = 0;
    auto flush = [&] {
        if (!paragraph.isEmpty()) blocks.append({BlockKind::Paragraph, paragraph});
        paragraph.clear();
    };
    const auto lines = source.split(u'\n');
    for (qsizetype n = 0; n < lines.size(); ++n) {
        const QString &line = lines[n];
        if (n + 1 == lines.size() && line.isEmpty()) break;
        const auto indent = indentation(line);
        if (state == State::FencedCode) {
            if (indent >= 0 && run(line, indent, marker) >= fenceLength
                && trim(line.mid(indent + run(line, indent, marker))).isEmpty()) {
                blocks.append({BlockKind::CodeBlock, fence, 0, {}, info});
                fence.clear();
                fenceLength = 0;
                state = State::Paragraph;
            } else {
                fence += deindent(line, fenceIndent) + u'\n';
            }
            continue;
        }
        const bool blank = trim(line).isEmpty();
        if (state == State::IndentedCode) {
            if (blank) {
                pendingBlanks += deindent(line, 4) + u'\n';
                continue;
            }
            if (indent < 0) {
                indented += pendingBlanks + deindent(line, 4) + u'\n';
                pendingBlanks.clear();
                continue;
            }
            blocks.append({BlockKind::CodeBlock, indented});
            indented.clear();
            pendingBlanks.clear();
            state = State::Paragraph;
            // The terminating line still needs ordinary block classification.
        }
        if (!blank && indent < 0 && paragraph.isEmpty()) {
            state = State::IndentedCode;
            indented = deindent(line, 4) + u'\n';
            continue;
        }
        const int level = setextLevel(line, indent);
        if (!paragraph.isEmpty() && level) {
            blocks.append({BlockKind::Heading, paragraph, level});
            paragraph.clear();
            continue;
        }
        if (thematicBreak(line, indent)) {
            flush();
            blocks.append({BlockKind::ThematicBreak, {}});
            continue;
        }
        if (indent >= 0 && indent < line.size()
            && (line[indent] == u'`' || line[indent] == u'~')) {
            const auto length = run(line, indent, line[indent]);
            if (length >= 3 && (line[indent] == u'~'
                || !line.mid(indent + length).contains(u'`'))) {
                flush();
                marker = line[indent];
                fenceLength = length;
                state = State::FencedCode;
                fenceIndent = int(indent);
                info = decodeInfo(trim(line.mid(indent + length)));
                fence.clear();
                continue;
            }
        }
        const auto hashes = indent >= 0 ? run(line, indent, u'#') : 0;
        const auto after = indent + hashes;
        if (hashes >= 1 && hashes <= 6
            && (after == line.size() || space(line[after]))) {
            flush();
            QString text = trim(line.mid(after));
            qsizetype end = text.size();
            while (end > 0 && text[end - 1] == u'#') --end;
            if (end < text.size() && ((end > 0 && space(text[end - 1]))
                || (end == 0 && after < line.size() && space(line[after]))))
                text = trim(text.left(end));
            blocks.append({BlockKind::Heading, text, int(hashes)});
        } else {
            const QString text = trim(line);
            if (text.isEmpty()) flush();
            else {
                if (!paragraph.isEmpty()) paragraph += u' ';
                paragraph += text;
            }
        }
    }
    flush();
    if (state == State::IndentedCode) blocks.append({BlockKind::CodeBlock, indented});
    if (state == State::FencedCode) blocks.append({BlockKind::CodeBlock, fence, 0, {}, info});
    for (auto &block : blocks) {
        if (block.kind == BlockKind::CodeBlock || block.kind == BlockKind::ThematicBreak) continue;
        const auto content = parseInline(block.text);
        block.text = content.text;
        block.ranges = content.ranges;
    }
    return blocks;
}
}
