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
QVector<InlineNode> retain(cmark_node *node)
{
    QVector<InlineNode> result;
    for (; node; node = cmark_node_next(node)) {
        InlineNode value{InlineKind::Text, {}, {}, {}, {}};
        switch (cmark_node_get_type(node)) {
        case CMARK_NODE_TEXT: value.kind = InlineKind::Text; break;
        case CMARK_NODE_CODE: value.kind = InlineKind::Code; break;
        case CMARK_NODE_HTML_INLINE: value.kind = InlineKind::Html; break;
        case CMARK_NODE_SOFTBREAK: value.kind = InlineKind::SoftBreak; break;
        case CMARK_NODE_LINEBREAK: value.kind = InlineKind::HardBreak; break;
        case CMARK_NODE_EMPH: value.kind = InlineKind::Emphasis; break;
        case CMARK_NODE_STRONG: value.kind = InlineKind::Strong; break;
        case CMARK_NODE_LINK: value.kind = InlineKind::Link; break;
        case CMARK_NODE_IMAGE: value.kind = InlineKind::Image; break;
        default: continue;
        }
        if (value.kind == InlineKind::Text || value.kind == InlineKind::Code || value.kind == InlineKind::Html)
            value.literal = QString::fromUtf8(cmark_node_get_literal(node));
        if (value.kind == InlineKind::Link || value.kind == InlineKind::Image) {
            value.destination = QString::fromUtf8(cmark_node_get_url(node));
            value.title = QString::fromUtf8(cmark_node_get_title(node));
        }
        // cmark can leave an empty Text leaf after trimming line-end spaces.
        // It carries no source semantics; empty link/image containers still survive.
        if (value.kind == InlineKind::Text && value.literal.isEmpty()) continue;
        value.children = retain(cmark_node_first_child(node));
        result.append(std::move(value));
    }
    return result;
}
void project(const QVector<InlineNode> &nodes, InlineContent &output, int flags = 0,
             bool inImage = false, const QString &enclosingLink = {}, bool linked = false)
{
    for (const auto &node : nodes) {
        switch (node.kind) {
        case InlineKind::Text:
        case InlineKind::Html: append(output, node.literal, flags); break;
        case InlineKind::Code: append(output, node.literal, flags | Code); break;
        case InlineKind::SoftBreak: append(output, QStringLiteral(" "), flags); break;
        case InlineKind::HardBreak: append(output, QStringLiteral("\n"), flags); break;
        case InlineKind::Emphasis: project(node.children, output, flags | Emphasis, inImage, enclosingLink, linked); break;
        case InlineKind::Strong: project(node.children, output, flags | Strong, inImage, enclosingLink, linked); break;
        case InlineKind::Link: {
            const int start = output.text.size();
            project(node.children, output, flags, inImage,
                    inImage ? enclosingLink : node.destination, !inImage || linked);
            if (!inImage && output.text.size() > start)
                output.links.append({start, int(output.text.size()) - start, node.destination});
            break;
        }
        case InlineKind::Image: {
            const int start = output.text.size();
            project(node.children, output, flags, true);
            if (!inImage) output.images.append({start, int(output.text.size()) - start,
                node.destination, node.title, enclosingLink, linked});
            break;
        }
        }
    }
}
}
InlineContent adaptInlineNodes(cmark_node *node)
{
    InlineContent output;
    output.nodes = retain(node);
    project(output.nodes, output);
    return output;
}
// Private inline fixture helper; production parsing adapts the single document tree.
InlineContent parseInline(const QString &source)
{
    const auto input = QByteArray("QMarkdownInline ") + source.toUtf8();
    NodeOwner root{cmark_parse_document(input.constData(), input.size(), CMARK_OPT_DEFAULT)};
    if (!root.node) return {source, {}};
    auto result = adaptInlineNodes(cmark_node_first_child(cmark_node_first_child(root.node)));
    // The synthetic prefix belongs only to the fixture parser, never its tree.
    if (!result.nodes.isEmpty()) {
        result.nodes.first().literal.remove(0, 16);
        if (result.nodes.first().literal.isEmpty()) result.nodes.removeFirst();
    }
    result.text.remove(0, 16);
    for (auto &range : result.ranges) range.start -= 16;
    for (auto &link : result.links) link.start -= 16;
    for (auto &image : result.images) image.start -= 16;
    return result;
}
}
