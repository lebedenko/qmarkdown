#include "document.h"
#include "inlineadapter.h"
#include "../../../third_party/cmark/symbols.h"
#include <cmark.h>

namespace QMarkdownPrivate {
namespace {
struct NodeOwner {
    cmark_node *node;
    ~NodeOwner() { if (node) cmark_node_free(node); }
};
QVector<Block> adaptBlocks(cmark_node *node)
{
    QVector<Block> blocks;
    for (; node; node = cmark_node_next(node)) {
        Block block{BlockKind::Paragraph, {}};
        switch (cmark_node_get_type(node)) {
        case CMARK_NODE_PARAGRAPH: break;
        case CMARK_NODE_HEADING:
            block.kind = BlockKind::Heading;
            block.level = cmark_node_get_heading_level(node);
            break;
        case CMARK_NODE_CODE_BLOCK:
            block.kind = BlockKind::CodeBlock;
            block.text = QString::fromUtf8(cmark_node_get_literal(node));
            block.infoString = QString::fromUtf8(cmark_node_get_fence_info(node));
            break;
        case CMARK_NODE_HTML_BLOCK:
            block.kind = BlockKind::HtmlBlock;
            block.text = QString::fromUtf8(cmark_node_get_literal(node));
            break;
        case CMARK_NODE_THEMATIC_BREAK: block.kind = BlockKind::ThematicBreak; break;
        case CMARK_NODE_LIST:
            block.kind = BlockKind::List;
            block.ordered = cmark_node_get_list_type(node) == CMARK_ORDERED_LIST;
            block.start = cmark_node_get_list_start(node);
            block.delimiter = cmark_node_get_list_delim(node) == CMARK_PAREN_DELIM ? u')' : u'.';
            block.tight = cmark_node_get_list_tight(node);
            break;
        case CMARK_NODE_ITEM: block.kind = BlockKind::ListItem; break;
        case CMARK_NODE_BLOCK_QUOTE: block.kind = BlockKind::Quote; break;
        default: continue;
        }
        if (block.kind == BlockKind::Paragraph || block.kind == BlockKind::Heading) {
            const auto content = adaptInlineNodes(cmark_node_first_child(node));
            block.text = content.text;
            block.ranges = content.ranges;
        } else if (block.kind == BlockKind::List || block.kind == BlockKind::ListItem || block.kind == BlockKind::Quote) {
            block.children = adaptBlocks(cmark_node_first_child(node));
        }
        blocks.append(std::move(block));
    }
    return blocks;
}
}
QVector<Block> parse(QString source)
{
    source.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    source.replace(u'\r', u'\n');
    source.replace(QChar(0), QChar(0xfffd));
    const auto input = source.toUtf8();
    NodeOwner document{cmark_parse_document(input.constData(), input.size(), CMARK_OPT_DEFAULT)};
    return document.node ? adaptBlocks(cmark_node_first_child(document.node)) : QVector<Block>{};
}
}
