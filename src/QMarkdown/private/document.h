#pragma once
#include "inline.h"
#include <QString>
#include <QImage>
#include <QVector>
#include <utility>

namespace QMarkdownPrivate {
enum class BlockKind { Paragraph, Heading, CodeBlock, ThematicBreak, List, ListItem, Quote, HtmlBlock, Image, Segments, Table, TableRow, TableCell };
enum class TaskState { Ordinary, Unchecked, Checked };
struct Block {
    BlockKind kind;
    QString text;
    int level = 0;
    QVector<InlineRange> ranges;
    QVector<LinkSpan> links;
    QVector<ImageSpan> images;
    QImage image;
    QString imageLink;
    bool imageLinked = false;
    QString infoString;
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
    int selectionOffset = 0;
    int selectionLength = 0;
};
struct ParseOptions {
    bool taskLists = false;
    bool strikethrough = false;
    bool extendedAutolinks = false;
    bool tables = false;
    bool tagFilter = false;
};
QVector<Block> parse(QString source, ParseOptions options = {});
QString projectText(QVector<Block> &blocks);
// Compatibility helper for existing private fixtures; production uses ParseOptions.
inline QVector<Block> parse(QString source, bool tasks, bool strike = false, bool autolinks = false)
{ return parse(std::move(source), ParseOptions{tasks, strike, autolinks}); }
}
