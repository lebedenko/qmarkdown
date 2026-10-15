#pragma once
#include "document.h"
#include <QImage>
#include <QHash>
#include <QUrl>

namespace QMarkdownPrivate {
enum class PresentationKind { Paragraph, Heading, CodeBlock, ThematicBreak, List, ListItem, Quote, HtmlBlock, Table, TableRow, TableCell, Image, Segments };
struct PresentationBlock {
    PresentationKind kind;
    QString text;
    QString infoString;
    QString language;
    bool codeFenced = false;
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
    int selectionOffset = 0;
    int selectionLength = 0;
};
struct Presentation {
    QString text;
    QVector<PresentationBlock> blocks;
};
Presentation projectDocument(const QVector<Block> &blocks);
QVector<PresentationBlock> projectImages(const QVector<PresentationBlock> &blocks, const QHash<QUrl, QImage> &images, const QUrl &base);
}
