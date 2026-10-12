#pragma once
#include "inline.h"
#include <QString>
#include <QImage>
#include <QVector>

namespace QMarkdownPrivate {
enum class BlockKind { Paragraph, Heading, CodeBlock, ThematicBreak, List, ListItem, Quote, HtmlBlock, Image, Segments };
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
};
QVector<Block> parse(QString source, bool taskListsEnabled = false);
}
