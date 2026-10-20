#pragma once
#include <QString>
#include <QVector>

namespace QMarkdownPrivate {
enum InlineFlag { Emphasis = 1, Strong = 2, Code = 4, Strikethrough = 8 };
struct InlineRange {
    int start = 0;
    int length = 0;
    int flags = 0;
};
struct CodeSpan {
    int start = 0;
    int length = 0;
};
struct LinkSpan {
    int start = 0;
    int length = 0;
    QString destination;
    int logicalId = -1;
    QString label;
    int noteNumber = 0, noteOccurrence = 0;
    bool backlink = false;
};
struct ImageSpan {
    int start = 0;
    int length = 0;
    QString destination;
    QString title;
    QString enclosingLink;
    bool linked = false;
    int logicalId = -1;
};
struct FormulaSpan {
    int start = 0, length = 0;
    int sourceLine = 0, sourceColumn = 0, sourceEndColumn = 0;
};
struct ReferenceSpan { int start = 0, length = 0, number = 0, occurrence = 0; };
enum class InlineKind { Text, Code, Emphasis, Strong, SoftBreak, HardBreak, Link, Image, Html, Strikethrough, Formula, FootnoteReference };
// Owned source semantics; presentation spans deliberately retain effective styles.
struct InlineNode {
    InlineKind kind;
    QString literal;
    QString destination;
    QString title;
    QVector<InlineNode> children;
    int noteNumber = 0, noteOccurrence = 0;
    QString filteredHtml;
    int sourceLine = 0, sourceColumn = 0, sourceEndColumn = 0;
};
struct InlineContent {
    QString text;
    QVector<InlineRange> ranges;
    QVector<LinkSpan> links;
    QVector<ImageSpan> images;
    QVector<InlineNode> nodes;
    QVector<CodeSpan> codes;
    QVector<FormulaSpan> formulas;
    QVector<ReferenceSpan> references;
    int nextLinkIdentity = 0;
};
QString filterRawHtml(const QString &literal);
InlineContent projectInlineNodes(const QVector<InlineNode> &nodes);
InlineContent parseInline(const QString &source);
}
