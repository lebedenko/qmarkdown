#include "document.h"
#include "inlineadapter.h"
#include "../../../third_party/cmark/symbols.h"
#include <cmark.h>
#include <memory>

namespace QMarkdownPrivate {
namespace {
QVector<Block> adaptBlocks(cmark_node *node, bool tagFilter)
{
    QVector<Block> blocks;
    for (; node; node = cmark_node_next(node)) {
        Block block{BlockKind::Paragraph, {}};
        switch (cmark_node_get_type(node)) {
        case CMARK_NODE_PARAGRAPH: break;
        case CMARK_NODE_QMARKDOWN_TABLE: block.kind = BlockKind::Table; break;
        case CMARK_NODE_QMARKDOWN_TABLE_ROW: block.kind = BlockKind::TableRow; break;
        case CMARK_NODE_QMARKDOWN_TABLE_CELL:
            block.kind = BlockKind::TableCell;
            block.tableAlignment = cmark_node_get_table_alignment(node);
            break;
        case CMARK_NODE_HEADING:
            block.kind = BlockKind::Heading;
            block.level = cmark_node_get_heading_level(node);
            break;
        case CMARK_NODE_CODE_BLOCK:
            block.kind = BlockKind::CodeBlock;
            block.text = QString::fromUtf8(cmark_node_get_literal(node));
            block.infoString = QString::fromUtf8(cmark_node_get_fence_info(node));
            block.codeFenced = cmark_node_get_code_fenced(node);
            break;
        case CMARK_NODE_HTML_BLOCK:
            block.kind = BlockKind::HtmlBlock;
            block.text = QString::fromUtf8(cmark_node_get_literal(node));
            block.filteredHtml = tagFilter ? filterRawHtml(block.text) : block.text;
            break;
        case CMARK_NODE_THEMATIC_BREAK: block.kind = BlockKind::ThematicBreak; break;
        case CMARK_NODE_LIST:
            block.kind = BlockKind::List;
            block.ordered = cmark_node_get_list_type(node) == CMARK_ORDERED_LIST;
            block.start = cmark_node_get_list_start(node);
            block.delimiter = cmark_node_get_list_delim(node) == CMARK_PAREN_DELIM ? u')' : u'.';
            block.tight = cmark_node_get_list_tight(node);
            break;
        case CMARK_NODE_ITEM:
            block.kind = BlockKind::ListItem;
            block.taskState = static_cast<TaskState>(cmark_node_get_task_state(node));
            break;
        case CMARK_NODE_BLOCK_QUOTE: block.kind = BlockKind::Quote; break;
        default: continue;
        }
        if (block.kind == BlockKind::Paragraph || block.kind == BlockKind::Heading || block.kind == BlockKind::TableCell) {
            const auto content = adaptInlineNodes(cmark_node_first_child(node), tagFilter);
            block.text = content.text;
            block.ranges = content.ranges;
            block.links = content.links;
            block.codes = content.codes;
            block.images = content.images;
            block.inlines = content.nodes;
        } else if (block.kind == BlockKind::List || block.kind == BlockKind::ListItem || block.kind == BlockKind::Quote
                   || block.kind == BlockKind::Table || block.kind == BlockKind::TableRow) {
            block.children = adaptBlocks(cmark_node_first_child(node), tagFilter);
        }
        blocks.append(std::move(block));
    }
    return blocks;
}
}
QVector<Block> parse(QString source, ParseOptions options)
{
    source.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    source.replace(u'\r', u'\n');
    source.replace(QChar(0), QChar(0xfffd));
    const auto input = source.toUtf8();
    std::unique_ptr<cmark_node, decltype(&cmark_node_free)> document{cmark_parse_document(input.constData(), input.size(),
        (options.taskLists ? CMARK_OPT_QMARKDOWN_TASK_LISTS : 0)
        | (options.strikethrough ? CMARK_OPT_QMARKDOWN_STRIKETHROUGH : 0)
        | (options.extendedAutolinks ? CMARK_OPT_QMARKDOWN_EXTENDED_AUTOLINKS : 0)
        | (options.tables ? CMARK_OPT_QMARKDOWN_TABLES : 0)), &cmark_node_free};
    return document.get() ? adaptBlocks(cmark_node_first_child(document.get()), options.tagFilter) : QVector<Block>{};
}

}
