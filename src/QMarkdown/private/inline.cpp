#include "inlineadapter.h"
#include "../../../third_party/cmark/symbols.h"
#include <cmark.h>

namespace QMarkdownPrivate {
namespace {
struct NodeOwner {
    cmark_node *node;
    ~NodeOwner() { if (node) cmark_node_free(node); }
};
void append(InlineContent &output, const QString &text, int flags)
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
void adapt(cmark_node *node, InlineContent &output, int flags = 0)
{
    for (; node; node = cmark_node_next(node)) {
        switch (cmark_node_get_type(node)) {
        case CMARK_NODE_TEXT:
        case CMARK_NODE_HTML_INLINE:
            append(output, QString::fromUtf8(cmark_node_get_literal(node)), flags); break;
        case CMARK_NODE_CODE:
            append(output, QString::fromUtf8(cmark_node_get_literal(node)), flags | Code); break;
        case CMARK_NODE_SOFTBREAK: append(output, QStringLiteral(" "), flags); break;
        case CMARK_NODE_LINEBREAK: append(output, QStringLiteral("\n"), flags); break;
        case CMARK_NODE_EMPH: adapt(cmark_node_first_child(node), output, flags | Emphasis); break;
        case CMARK_NODE_STRONG: adapt(cmark_node_first_child(node), output, flags | Strong); break;
        case CMARK_NODE_LINK:
        case CMARK_NODE_IMAGE: adapt(cmark_node_first_child(node), output, flags); break;
        default: break;
        }
    }
}
}
InlineContent adaptInlineNodes(cmark_node *node)
{
    InlineContent output;
    adapt(node, output);
    return output;
}
// Private inline fixture helper; production parsing adapts the single document tree.
InlineContent parseInline(const QString &source)
{
    const auto input = QByteArray("QMarkdownInline ") + source.toUtf8();
    NodeOwner root{cmark_parse_document(input.constData(), input.size(), CMARK_OPT_DEFAULT)};
    if (!root.node) return {source, {}};
    auto result = adaptInlineNodes(cmark_node_first_child(cmark_node_first_child(root.node)));
    result.text.remove(0, 16);
    for (auto &range : result.ranges) range.start -= 16;
    return result;
}
}
