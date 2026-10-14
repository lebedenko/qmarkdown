#include "presentation.h"

namespace QMarkdownPrivate {
namespace {
class TextProjection
{
public:
    QString text;
    void boundary(int lines) {
        int trailing = 0;
        for (int i = int(text.size()) - 1; i >= 0 && text[i] == u'\n'; --i) ++trailing;
        text += QString(qMax(0, lines - trailing), u'\n');
    }
    void sequence(QVector<PresentationBlock> &blocks, int depth) {
        for (int i = 0; i < blocks.size(); ++i) {
            if (i) boundary(2);
            block(blocks[i], depth);
        }
    }
    void block(PresentationBlock &value, int depth) {
        value.selectionOffset = int(text.size());
        if (value.kind == PresentationKind::List) {
            for (int i = 0; i < value.children.size(); ++i) {
                if (i) boundary(1);
                auto &item = value.children[i];
                item.selectionOffset = int(text.size());
                text += QString(depth * 2, u' ');
                text += value.ordered ? QString::number(qint64(value.start) + i) + value.delimiter : QString::fromUtf8("•");
                text += u' ';
                if (item.taskState != TaskState::Ordinary)
                    text += item.taskState == TaskState::Checked ? "[x] " : "[ ] ";
                for (int j = 0; j < item.children.size(); ++j) {
                    if (j) boundary(item.children[j].kind == PresentationKind::List ? 1 : 2);
                    block(item.children[j], depth + 1);
                }
                item.selectionLength = int(text.size()) - item.selectionOffset;
            }
        } else if (value.kind == PresentationKind::Table) {
            for (int row = 0; row < value.children.size(); ++row) {
                if (row) text += u'\n';
                auto &r = value.children[row];
                r.selectionOffset = int(text.size());
                for (int cell = 0; cell < r.children.size(); ++cell) {
                    if (cell) text += u'\t';
                    block(r.children[cell], depth);
                }
                r.selectionLength = int(text.size()) - r.selectionOffset;
            }
        } else if (!value.children.isEmpty()) {
            sequence(value.children, depth);
        } else {
            text += value.text;
        }
        value.selectionLength = int(text.size()) - value.selectionOffset;
    }
};
}
namespace {
QVector<PresentationBlock> present(const QVector<Block> &blocks)
{
    QVector<PresentationBlock> result;
    result.reserve(blocks.size());
    for (const auto &source : blocks) {
        PresentationBlock block{};
        switch (source.kind) {
        case BlockKind::Paragraph: block.kind = PresentationKind::Paragraph; break;
        case BlockKind::Heading: block.kind = PresentationKind::Heading; break;
        case BlockKind::CodeBlock: block.kind = PresentationKind::CodeBlock; break;
        case BlockKind::ThematicBreak: block.kind = PresentationKind::ThematicBreak; break;
        case BlockKind::List: block.kind = PresentationKind::List; break;
        case BlockKind::ListItem: block.kind = PresentationKind::ListItem; break;
        case BlockKind::Quote: block.kind = PresentationKind::Quote; break;
        case BlockKind::HtmlBlock: block.kind = PresentationKind::HtmlBlock; break;
        case BlockKind::Table: block.kind = PresentationKind::Table; break;
        case BlockKind::TableRow: block.kind = PresentationKind::TableRow; break;
        case BlockKind::TableCell: block.kind = PresentationKind::TableCell; break;
        }
        block.text = source.text;
        block.infoString = source.infoString;
        block.codeFenced = source.codeFenced;
        block.level = source.level;
        block.ranges = source.ranges;
        block.links = source.links;
        block.images = source.images;
        block.ordered = source.ordered;
        block.start = source.start;
        block.delimiter = source.delimiter;
        block.tight = source.tight;
        block.taskState = source.taskState;
        block.tableAlignment = source.tableAlignment;
        block.codes = source.codes;
        block.children = present(source.children);
        result.append(std::move(block));
    }
    return result;
}
}
Presentation projectDocument(const QVector<Block> &blocks)
{
    Presentation result;
    result.blocks = present(blocks);
    TextProjection projection;
    projection.sequence(result.blocks, 0);
    result.text = std::move(projection.text);
    return result;
}
namespace {
PresentationBlock textSlice(const PresentationBlock &block, int start, int end) {
    PresentationBlock result{block.kind, {}};
    result.level = block.level;
    result.selectionOffset = block.selectionOffset + start;
    result.selectionLength = end - start;
    result.text = block.text.mid(start, end - start);
    for (const auto &range : block.ranges) {
        const int a = qMax(start, range.start), b = qMin(end, range.start + range.length);
        if (a < b) result.ranges.append({a - start, b - a, range.flags});
    }
    for (const auto &code : block.codes) {
        const int a = qMax(start, code.start), b = qMin(end, code.start + code.length);
        if (a < b) result.codes.append({a - start, b - a});
    }
    for (const auto &link : block.links) {
        const int a = qMax(start, link.start), b = qMin(end, link.start + link.length);
        if (a < b) result.links.append({a - start, b - a, link.destination});
    }
    return result;
}
}
QVector<PresentationBlock> projectImages(const QVector<PresentationBlock> &blocks, const QHash<QUrl, QImage> &images, const QUrl &base)
{
    QVector<PresentationBlock> result;
    for (auto block : blocks) {
        block.children = projectImages(block.children, images, base);
        QVector<PresentationBlock> segments;
        int start = 0;
        for (const auto &span : block.images) {
            const auto key = base.resolved(QUrl(span.destination)).adjusted(QUrl::RemoveFragment);
            if (!images.contains(key)) continue;
            if (span.start > start) segments.append(textSlice(block, start, span.start));
            PresentationBlock image{PresentationKind::Image, {}};
            image.selectionOffset = block.selectionOffset + span.start;
            image.selectionLength = span.length;
            image.image = images.value(key); image.imageLink = span.enclosingLink; image.imageLinked = span.linked;
            segments.append(std::move(image));
            start = span.start + span.length;
        }
        if (!segments.isEmpty()) {
            if (start < block.text.size()) segments.append(textSlice(block, start, block.text.size()));
            if (block.kind != PresentationKind::TableCell) block.kind = PresentationKind::Segments;
            for (auto &segment : segments)
                if (segment.kind == PresentationKind::TableCell) segment.kind = PresentationKind::Paragraph;
            block.children = std::move(segments);
            block.images.clear(); block.text.clear(); block.ranges.clear(); block.links.clear();
            block.codes.clear();
        }
        result.append(std::move(block));
    }
    return result;
}
}
