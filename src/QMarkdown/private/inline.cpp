#include "inlineadapter.h"
#include "../../../third_party/cmark/symbols.h"
#include <cmark.h>
#include <QStringDecoder>
#include <limits>

namespace QMarkdownPrivate {
namespace {
const QByteArray prefix("QMarkdownInline ");
bool boundary(const QByteArray &source, int offset)
{
    return offset >= 0 && offset <= source.size()
        && (offset == source.size() || (static_cast<unsigned char>(source[offset]) & 0xc0) != 0x80);
}
struct NodeOwner {
    cmark_node *node;
    ~NodeOwner() { if (node) cmark_node_free(node); }
};
struct Adapter {
    const QByteArray &source;
    InlineContent output;
    bool first = true;
    int preservedEnd = 0;

    void append(const QString &text, int flags)
    {
        if (text.isEmpty()) return;
        const int start = output.text.size();
        output.text += text;
        if (!flags) return;
        if (!output.ranges.isEmpty() && output.ranges.last().flags == flags
            && output.ranges.last().start + output.ranges.last().length == start)
            output.ranges.last().length += text.size();
        else output.ranges.append({start, int(text.size()), flags});
    }
    bool adapt(cmark_node *node, int flags = 0, int depth = 0)
    {
        if (depth > 512) return false;
        for (; node; node = cmark_node_next(node)) {
            const auto type = cmark_node_get_type(node);
            if (first && type != CMARK_NODE_TEXT) return false;
            if (type == CMARK_NODE_TEXT || type == CMARK_NODE_CODE) {
                if (cmark_node_first_child(node)) return false;
                const char *literal = cmark_node_get_literal(node);
                if (!literal) return false;
                QString text = QString::fromUtf8(literal);
                if (first) {
                    if (!text.startsWith(QString::fromLatin1(prefix))) return false;
                    text.remove(0, prefix.size());
                    first = false;
                }
                append(text, flags | (type == CMARK_NODE_CODE ? Code : 0));
            } else if (type == CMARK_NODE_EMPH || type == CMARK_NODE_STRONG) {
                if (!cmark_node_first_child(node)
                    || !adapt(cmark_node_first_child(node), flags | (type == CMARK_NODE_EMPH ? Emphasis : Strong), depth + 1))
                    return false;
            } else if (type == CMARK_NODE_LINK || type == CMARK_NODE_IMAGE || type == CMARK_NODE_HTML_INLINE) {
                QByteArray literal;
                const int start = cmark_node_get_start_column(node) - 1;
                const int end = cmark_node_get_end_column(node);
                if (start < preservedEnd || !sourceSpan(source, cmark_node_get_start_line(node),
                    start + 1, cmark_node_get_end_line(node), end, prefix.size(), &literal)) return false;
                preservedEnd = end;
                append(QString::fromUtf8(literal), flags);
            } else return false;
        }
        return true;
    }
};
}
bool sourceSpan(const QByteArray &source, int startLine, int startColumn,
                int endLine, int endColumn, int minimum, QByteArray *result)
{
    if (startColumn < 1 || minimum < 0) return false;
    const int start = startColumn - 1;
    const int end = endColumn; // cmark's columns are inclusive, one-based byte offsets.
    if (!result || source.contains('\n') || startLine != 1 || endLine != 1
        || start < minimum || end <= start || end > source.size()
        || !boundary(source, start) || !boundary(source, end)) return false;
    const auto span = source.mid(start, end - start);
    QStringDecoder decoder(QStringDecoder::Utf8, QStringDecoder::Flag::Stateless);
    const QString decoded = decoder.decode(span);
    Q_UNUSED(decoded);
    if (decoder.hasError()) return false;
    *result = span;
    return true;
}
InlineContent parseInline(const QString &source)
{
    const InlineContent fallback{source, {}};
    if (source.isEmpty()) return fallback;
    const auto bytes = source.toUtf8();
    if (bytes.size() > std::numeric_limits<int>::max() - prefix.size()) return fallback;
    const auto input = prefix + bytes;
    NodeOwner root{cmark_parse_document(input.constData(), input.size(), CMARK_OPT_DEFAULT)};
    return adaptInlineDocument(source, root.node);
}
InlineContent adaptInlineDocument(const QString &source, cmark_node *document)
{
    const InlineContent fallback{source, {}};
    const auto input = prefix + source.toUtf8();
    if (!document || cmark_node_get_type(document) != CMARK_NODE_DOCUMENT) return fallback;
    auto *paragraph = cmark_node_first_child(document);
    if (!paragraph || cmark_node_get_type(paragraph) != CMARK_NODE_PARAGRAPH || cmark_node_next(paragraph))
        return fallback;
    Adapter adapter{input};
    if (!adapter.adapt(cmark_node_first_child(paragraph)) || adapter.first) return fallback;
    return adapter.output;
}
}
