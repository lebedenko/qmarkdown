#include "document.h"
#include "inlineadapter.h"
#include "../../../third_party/cmark/symbols.h"
#include <cmark.h>
#include <memory>
#include <QHash>

namespace QMarkdownPrivate {
namespace {
QVector<Block> adaptBlocks(cmark_node *node, bool tagFilter)
{
    QVector<Block> blocks;
    for (; node; node = cmark_node_next(node)) {
        Block block{BlockKind::Paragraph, {}};
        block.sourceLine = cmark_node_get_start_line(node); block.sourceColumn = cmark_node_get_start_column(node);
        block.sourceEndLine = cmark_node_get_end_line(node); block.sourceEndColumn = cmark_node_get_end_column(node);
        switch (cmark_node_get_type(node)) {
        case CMARK_NODE_QMARKDOWN_FOOTNOTE_DEFINITION:
            block.kind = BlockKind::FootnoteDefinition;
            block.noteLabel = QString::fromUtf8(cmark_node_get_footnote_label(node));
            block.text = QString::fromUtf8(cmark_node_get_literal(node));
            block.children = adaptBlocks(cmark_node_first_child(node), tagFilter);
            break;
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
        case CMARK_NODE_QMARKDOWN_MATH:
            block.kind = BlockKind::MathBlock;
            block.text = QString::fromUtf8(cmark_node_get_literal(node));
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
            block.formulas = content.formulas;
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
namespace {
void applyInline(Block &block) {
    const auto content = projectInlineNodes(block.inlines);
    block.text = content.text; block.ranges = content.ranges; block.links = content.links;
    block.images = content.images; block.codes = content.codes; block.formulas = content.formulas;
    block.references = content.references;
}
void resolveFootnotes(QVector<Block> &blocks) {
    QHash<QString, Block> definitions;
    auto extract = [&](auto &&self, QVector<Block> &values) -> void {
        QVector<Block> retained;
        for (auto block : values) {
            if (block.kind == BlockKind::FootnoteDefinition) {
                if (!definitions.contains(block.noteLabel)) { definitions.insert(block.noteLabel, block); continue; }
                InlineNode marker{InlineKind::Text}; marker.literal = block.text + u' ';
                if (!block.children.isEmpty() && block.children.first().kind == BlockKind::Paragraph) {
                    block.children.first().inlines.prepend(marker); applyInline(block.children.first());
                } else {
                    Block literal{BlockKind::Paragraph}; literal.inlines.append(marker); applyInline(literal);
                    block.children.prepend(literal);
                }
                retained += block.children;
            } else {
                const bool hadChildren = !block.children.isEmpty();
                self(self, block.children);
                if (hadChildren && block.children.isEmpty()
                    && (block.kind == BlockKind::List || block.kind == BlockKind::ListItem || block.kind == BlockKind::Quote)) continue;
                retained.append(std::move(block));
            }
        }
        values = std::move(retained);
    };
    extract(extract, blocks);
    QHash<QString, int> numbers, occurrences;
    QStringList order;
    auto resolveNodes = [&](auto &&self, QVector<InlineNode> &nodes) -> void {
        for (auto &node : nodes) {
            if (node.kind == InlineKind::FootnoteReference) {
                if (!definitions.contains(node.destination)) { node.kind = InlineKind::Text; continue; }
                if (!numbers.contains(node.destination)) { order.append(node.destination); numbers[node.destination] = order.size(); }
                node.noteNumber = numbers[node.destination]; node.noteOccurrence = ++occurrences[node.destination];
                node.literal = QStringLiteral("[%1]").arg(node.noteNumber);
            }
            self(self, node.children);
        }
    };
    auto resolveBlocks = [&](auto &&self, QVector<Block> &values) -> void {
        for (auto &block : values) {
            if (!block.inlines.isEmpty()) { resolveNodes(resolveNodes, block.inlines); applyInline(block); }
            self(self, block.children);
        }
    };
    resolveBlocks(resolveBlocks, blocks);
    for (const auto &label : order) {
        auto definition = definitions.take(label);
        Block note{BlockKind::Endnote}; note.noteNumber = numbers[label]; note.noteOccurrences = occurrences[label];
        note.sourceLine = definition.sourceLine; note.sourceColumn = definition.sourceColumn;
        note.sourceEndLine = definition.sourceEndLine; note.sourceEndColumn = definition.sourceEndColumn;
        note.children = std::move(definition.children);
        Block backlinks{BlockKind::Backlinks}; backlinks.noteNumber = note.noteNumber; backlinks.noteOccurrences = note.noteOccurrences;
        note.children.append(backlinks); blocks.append(note);
    }
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
        | (options.tables ? CMARK_OPT_QMARKDOWN_TABLES : 0)
        | (options.math ? CMARK_OPT_QMARKDOWN_MATH : 0)
        | (options.inlineMath ? CMARK_OPT_QMARKDOWN_INLINE_MATH : 0)
        | (options.footnotes ? CMARK_OPT_QMARKDOWN_FOOTNOTES : 0)), &cmark_node_free};
    auto blocks = document.get() ? adaptBlocks(cmark_node_first_child(document.get()), options.tagFilter) : QVector<Block>{};
    if (options.footnotes) resolveFootnotes(blocks);
    return blocks;
}

}
