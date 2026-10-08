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
void adapt(cmark_node *node, InlineContent &output, int flags = 0, bool inImage = false, const QString &enclosingLink = {}, bool linked = false)
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
        case CMARK_NODE_EMPH: adapt(cmark_node_first_child(node), output, flags | Emphasis, inImage, enclosingLink, linked); break;
        case CMARK_NODE_STRONG: adapt(cmark_node_first_child(node), output, flags | Strong, inImage, enclosingLink, linked); break;
        case CMARK_NODE_LINK: {
            const int start = output.text.size();
            adapt(cmark_node_first_child(node), output, flags, inImage,
                  inImage ? enclosingLink : QString::fromUtf8(cmark_node_get_url(node)), !inImage || linked);
            if (!inImage && output.text.size() > start)
                output.links.append({start, int(output.text.size()) - start,
                                     QString::fromUtf8(cmark_node_get_url(node))});
            break;
        }
        case CMARK_NODE_IMAGE: {
            const int start = output.text.size();
            adapt(cmark_node_first_child(node), output, flags, true);
            if (!inImage) output.images.append({start, int(output.text.size()) - start,
                QString::fromUtf8(cmark_node_get_url(node)), QString::fromUtf8(cmark_node_get_title(node)), enclosingLink, linked});
            break;
        }
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
    for (auto &link : result.links) link.start -= 16;
    for (auto &image : result.images) image.start -= 16;
    return result;
}
}
