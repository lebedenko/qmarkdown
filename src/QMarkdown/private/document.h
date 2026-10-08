#pragma once
#include "inline.h"
#include <QString>
#include <QVector>

namespace QMarkdownPrivate {
enum class BlockKind { Paragraph, Heading, CodeBlock, ThematicBreak };
struct Block {
    BlockKind kind;
    QString text;
    int level = 0;
    QVector<InlineRange> ranges;
    QString infoString;
};
QVector<Block> parse(QString source);
}
