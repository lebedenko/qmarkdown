#pragma once
#include "document.h"
#include <QImage>
#include <QHash>
#include <QUrl>

namespace QMarkdownPrivate {
enum class PresentationKind { Paragraph, Heading, CodeBlock, ThematicBreak, List, ListItem, Quote, HtmlBlock, Table, TableRow, TableCell, Image, Segments, Backlinks };
struct PresentationBlock {
    PresentationKind kind;
    QString text;
    QString infoString;
    QString language;
    bool codeFenced = false;
    bool displayMath = false;
    int level = 0;
    QVector<InlineRange> ranges;
    QVector<LinkSpan> links;
    QVector<LinkSpan> logicalLinks;
    QVector<ImageSpan> images;
    QImage image;
    QString imageLink;
    bool imageLinked = false;
    int imageLinkId = -1;
    QVector<PresentationBlock> children;
    bool ordered = false;
    int start = 1;
    QChar delimiter = u'.';
    bool tight = true;
    TaskState taskState = TaskState::Ordinary;
    int tableAlignment = 0;
    QVector<CodeSpan> codes;
    QVector<FormulaSpan> formulas;
    QVector<ReferenceSpan> references;
    int noteNumber = 0;
    int selectionOffset = 0;
    int selectionLength = 0;
};
// Offsets are positional metadata, not semantic block identity.
inline bool operator==(const PresentationBlock &a, const PresentationBlock &b)
{
    return std::tie(a.kind, a.text, a.infoString, a.language, a.codeFenced, a.displayMath, a.level, a.ranges, a.links, a.logicalLinks, a.images, a.image, a.imageLink, a.imageLinked, a.children, a.ordered, a.start, a.delimiter, a.tight, a.taskState, a.tableAlignment, a.codes, a.formulas, a.references, a.noteNumber)
        == std::tie(b.kind, b.text, b.infoString, b.language, b.codeFenced, b.displayMath, b.level, b.ranges, b.links, b.logicalLinks, b.images, b.image, b.imageLink, b.imageLinked, b.children, b.ordered, b.start, b.delimiter, b.tight, b.taskState, b.tableAlignment, b.codes, b.formulas, b.references, b.noteNumber);
}
struct Presentation {
    QString text;
    QVector<PresentationBlock> blocks;
};
Presentation projectDocument(const QVector<Block> &blocks);
QVector<PresentationBlock> projectImages(const QVector<PresentationBlock> &blocks, const QHash<QUrl, QImage> &images, const QUrl &base);
}
