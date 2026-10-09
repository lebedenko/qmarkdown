#pragma once
#include <QString>
#include <QVector>

namespace QMarkdownPrivate {
enum InlineFlag { Emphasis = 1, Strong = 2, Code = 4 };
struct InlineRange {
    int start = 0;
    int length = 0;
    int flags = 0;
};
struct LinkSpan {
    int start = 0;
    int length = 0;
    QString destination;
};
struct ImageSpan {
    int start = 0;
    int length = 0;
    QString destination;
    QString title;
    QString enclosingLink;
    bool linked = false;
};
enum class InlineKind { Text, Code, Emphasis, Strong, SoftBreak, HardBreak, Link, Image, Html };
// Owned source semantics; presentation spans deliberately retain effective styles.
struct InlineNode {
    InlineKind kind;
    QString literal;
    QString destination;
    QString title;
    QVector<InlineNode> children;
};
struct InlineContent {
    QString text;
    QVector<InlineRange> ranges;
    QVector<LinkSpan> links;
    QVector<ImageSpan> images;
    QVector<InlineNode> nodes;
};
InlineContent parseInline(const QString &source);
}
