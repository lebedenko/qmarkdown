#pragma once
#include "inline.h"
#include <QString>
#include <QVector>

namespace QMarkdownPrivate {
enum class BlockKind { Paragraph, Heading, CodeBlock, ThematicBreak, List, ListItem, Quote, HtmlBlock };
struct Block {
    BlockKind kind;
    QString text;
    int level = 0;
    QVector<InlineRange> ranges;
    QVector<LinkSpan> links;
    QString infoString;
    QVector<Block> children;
    bool ordered = false;
    int start = 1;
    QChar delimiter = u'.';
    bool tight = true;
};
QVector<Block> parse(QString source);
}
