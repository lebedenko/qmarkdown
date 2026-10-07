#include "formattedtext.h"
#include "inline.h"
#include <QFontMetricsF>
#include <QGlyphRun>
#include <QPainter>
#include <cmath>

FormattedText::FormattedText(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
}
void FormattedText::setText(const QString &value)
{
    if (m_text == value) return;
    m_text = value; emit textChanged(); polish();
}
void FormattedText::setFormatRanges(const QVariantList &value)
{
    if (m_ranges == value) return;
    m_ranges = value; emit formatRangesChanged(); polish();
}
void FormattedText::setFont(const QFont &value)
{
    if (m_font == value && m_font.resolveMask() == value.resolveMask()) return;
    m_font = value; emit fontChanged(); polish();
}
void FormattedText::setCodeFont(const QFont &value)
{
    if (m_codeFont == value && m_codeFont.resolveMask() == value.resolveMask()) return;
    m_codeFont = value; emit codeFontChanged(); polish();
}
void FormattedText::setColor(const QColor &value)
{
    if (m_color == value) return;
    m_color = value; emit colorChanged(); polish();
}
void FormattedText::setLayoutWidth(qreal value)
{
    value = std::isfinite(value) ? qMax(qreal(0), value) : 0;
    if (m_layoutWidth == value) return;
    m_layoutWidth = value; emit layoutWidthChanged(); polish();
}
void FormattedText::updatePolish()
{
    m_layout.reset();
    m_logicalHeight = 0;
    m_paintOffset = {};
    if (m_layoutWidth > 0) {
        QString layoutText = m_text;
        // QTextLayout is a paragraph layout: use its native line separator for
        // decoded LF characters, keeping every UTF-16 range offset unchanged.
        layoutText.replace(u'\n', QChar::LineSeparator);
        m_layout = std::make_unique<QTextLayout>(layoutText, m_font);
        m_layout->setCacheEnabled(true);
        QTextOption option;
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        option.setAlignment(Qt::AlignLeft);
        m_layout->setTextOption(option);
        QList<QTextLayout::FormatRange> formats;
        int previousEnd = 0;
        for (const auto &value : m_ranges) {
            const auto range = value.toMap();
            const int start = range.value("start").toInt();
            const int length = range.value("length").toInt();
            const int flags = range.value("flags").toInt();
            if (start < previousEnd || length <= 0 || start > m_text.size()
                || length > m_text.size() - start) continue;
            previousEnd = start + length;
            QFont resolved = flags & QMarkdownPrivate::Code ? m_codeFont.resolve(m_font) : m_font;
            if (flags & QMarkdownPrivate::Emphasis) resolved.setItalic(true);
            if (flags & QMarkdownPrivate::Strong) resolved.setWeight(qMax(resolved.weight(), QFont::Bold));
            QTextCharFormat format;
            format.setFont(resolved);
            formats.append({start, length, format});
        }
        m_layout->setFormats(formats);
        m_layout->beginLayout();
        while (true) {
            auto line = m_layout->createLine();
            if (!line.isValid()) break;
            line.setLineWidth(m_layoutWidth);
            line.setPosition(QPointF(0, m_logicalHeight));
            m_logicalHeight += line.height();
        }
        m_layout->endLayout();
        if (m_text.isEmpty()) m_logicalHeight = QFontMetricsF(m_font).height();
        // Raster bounds include ink overhang; wrapping and parent geometry retain
        // the host's logical width and native line height.
        QRectF bounds(0, 0, m_layoutWidth, m_logicalHeight);
        for (const auto &run : m_layout->glyphRuns()) bounds = bounds.united(run.boundingRect());
        const qreal left = std::floor(bounds.left()) - 1;
        const qreal top = std::floor(bounds.top()) - 1;
        m_paintOffset = QPointF(-left, -top);
        setX(left); setY(top);
        setWidth(std::ceil(bounds.right()) - left + 1);
        setHeight(std::ceil(bounds.bottom()) - top + 1);
    } else {
        setX(0); setY(0); setWidth(0); setHeight(0);
    }
    emit layoutChanged();
    update();
}
void FormattedText::paint(QPainter *painter)
{
    if (!m_layout) return;
    painter->setPen(m_color);
    m_layout->draw(painter, m_paintOffset);
}
