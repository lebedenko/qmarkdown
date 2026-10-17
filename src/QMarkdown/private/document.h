#pragma once
#include "inline.h"
#include <QString>
#include <QVector>
#include <utility>

namespace QMarkdownPrivate {
// Preserve semantic fixture identifiers; 8/9 were former presentation-only kinds.
enum class BlockKind { Paragraph, Heading, CodeBlock, ThematicBreak, List, ListItem, Quote, HtmlBlock, Table = 10, TableRow, TableCell, MathBlock };
enum class TaskState { Ordinary, Unchecked, Checked };
struct Block {
    BlockKind kind;
    QString text;
    int level = 0;
    QVector<InlineRange> ranges;
    QVector<LinkSpan> links;
    QVector<ImageSpan> images;
    QString infoString;
    bool codeFenced = false;
    QVector<Block> children;
    bool ordered = false;
    int start = 1;
    QChar delimiter = u'.';
    bool tight = true;
    QVector<InlineNode> inlines;
    TaskState taskState = TaskState::Ordinary;
    int tableAlignment = 0;
    QString filteredHtml;
    QVector<CodeSpan> codes;
};
struct ParseOptions {
    bool taskLists = false;
    bool strikethrough = false;
    bool extendedAutolinks = false;
    bool tables = false;
    bool tagFilter = false;
    bool math = false;
};
QVector<Block> parse(QString source, ParseOptions options = {});
// Compatibility helper for existing private fixtures; production uses ParseOptions.
inline QVector<Block> parse(QString source, bool tasks, bool strike = false, bool autolinks = false)
{ return parse(std::move(source), ParseOptions{tasks, strike, autolinks}); }
}
