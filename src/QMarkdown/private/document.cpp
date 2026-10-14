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
    NodeOwner document{cmark_parse_document(input.constData(), input.size(),
        (options.taskLists ? CMARK_OPT_QMARKDOWN_TASK_LISTS : 0)
        | (options.strikethrough ? CMARK_OPT_QMARKDOWN_STRIKETHROUGH : 0)
        | (options.extendedAutolinks ? CMARK_OPT_QMARKDOWN_EXTENDED_AUTOLINKS : 0)
        | (options.tables ? CMARK_OPT_QMARKDOWN_TABLES : 0))};
    return document.node ? adaptBlocks(cmark_node_first_child(document.node), options.tagFilter) : QVector<Block>{};
}
namespace {
class TextProjection
{
public:
    QString text;
    void boundary(int lines) {
        int trailing = 0;
        for (int i = int(text.size()) - 1; i >= 0 && text[i] == u'\n'; --i) ++trailing;
        text += QString(qMax(0, lines - trailing), u'\n');
    }
    void sequence(QVector<Block> &blocks, int depth) {
        for (int i = 0; i < blocks.size(); ++i) {
            if (i) boundary(2);
            block(blocks[i], depth);
        }
    }
    void block(Block &value, int depth) {
        value.selectionOffset = int(text.size());
        if (value.kind == BlockKind::List) {
            for (int i = 0; i < value.children.size(); ++i) {
                if (i) boundary(1);
                auto &item = value.children[i];
                item.selectionOffset = int(text.size());
                text += QString(depth * 2, u' ');
                text += value.ordered ? QString::number(qint64(value.start) + i) + value.delimiter : QString::fromUtf8("•");
                text += u' ';
                if (item.taskState != TaskState::Ordinary)
                    text += item.taskState == TaskState::Checked ? "[x] " : "[ ] ";
                for (int j = 0; j < item.children.size(); ++j) {
                    if (j) boundary(item.children[j].kind == BlockKind::List ? 1 : 2);
                    block(item.children[j], depth + 1);
                }
                item.selectionLength = int(text.size()) - item.selectionOffset;
            }
        } else if (value.kind == BlockKind::Table) {
            for (int row = 0; row < value.children.size(); ++row) {
                if (row) text += u'\n';
                auto &r = value.children[row];
                r.selectionOffset = int(text.size());
                for (int cell = 0; cell < r.children.size(); ++cell) {
                    if (cell) text += u'\t';
                    block(r.children[cell], depth);
                }
                r.selectionLength = int(text.size()) - r.selectionOffset;
            }
        } else if (!value.children.isEmpty()) {
            sequence(value.children, depth);
        } else {
            text += value.text;
        }
        value.selectionLength = int(text.size()) - value.selectionOffset;
    }
};
}
QString projectText(QVector<Block> &blocks)
{
    TextProjection projection;
    projection.sequence(blocks, 0);
    return projection.text;
}

}
