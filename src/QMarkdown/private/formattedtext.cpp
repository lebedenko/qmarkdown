#include "formattedtext.h"
#include "inline.h"
#include "formatintervals.h"
#include <QFontMetricsF>
#include <QGlyphRun>
#include <QPainter>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QScreen>
#include <cmath>

namespace {
QFont layoutFont(QFont font)
{
    // Match native Qt Quick Text's half-point layout resolution while keeping
    // the public font and its resolve mask untouched.
    if (font.pointSizeF() > 0)
        font.setPointSizeF(qRound(font.pointSizeF() * 2) / qreal(2));
    return font;
}
}
FormattedText::FormattedText(QQuickItem *parent) : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    connect(this, &QQuickItem::windowChanged, this, &FormattedText::observeWindow);
    observeWindow(window());
}
void FormattedText::observeWindow(QQuickWindow *window)
{
    disconnect(m_windowScreenConnection);
    if (window)
        m_windowScreenConnection = connect(window, &QWindow::screenChanged, this,
                                           [this, window] { observeScreen(window); });
    observeScreen(window);
}
void FormattedText::observeScreen(QQuickWindow *window)
{
    disconnect(m_screenDpiConnection);
    auto *screen = window ? window->screen() : QGuiApplication::primaryScreen();
    if (screen)
        m_screenDpiConnection = connect(screen, &QScreen::logicalDotsPerInchChanged,
                                       this, [this] { polish(); });
    polish();
}
void FormattedText::itemChange(ItemChange change, const ItemChangeData &data)
{
    QQuickPaintedItem::itemChange(change, data);
    if (change == ItemDevicePixelRatioHasChanged) polish();
}
void FormattedText::setText(const QString &value)
{
    if (m_text == value) return;
    m_text = value; m_layout.reset(); emit textChanged(); polish();
}
void FormattedText::setFormatRanges(const QVariantList &value)
{
    if (m_ranges == value) return;
    m_ranges = value; emit formatRangesChanged(); polish();
}
void FormattedText::setFont(const QFont &value)
{
    if (m_font == value && m_font.resolveMask() == value.resolveMask()) return;
    // Native Text ignores resolve-mask-only changes and keeps its initial
    // application font unrounded until a different font is assigned.
    if (m_font != value) m_layoutFont = layoutFont(value);
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
        m_layout = std::make_unique<QTextLayout>(layoutText, m_layoutFont);
        m_layout->setCacheEnabled(true);
        QTextOption option;
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        option.setAlignment(Qt::AlignLeft);
        m_layout->setTextOption(option);
        QList<QTextLayout::FormatRange> formats;
        const auto intervals = QMarkdownPrivate::prepareFormatIntervals(int(m_text.size()), m_ranges, m_links);
        for (const auto &interval : intervals) {
            const int start = interval.start, length = interval.length, flags = interval.flags;
            const bool linked = interval.linked;
            QFont resolved = flags & QMarkdownPrivate::Code ? m_codeFont.resolve(m_layoutFont) : m_layoutFont;
            if ((flags & QMarkdownPrivate::Code) && (m_codeFont.resolveMask() & QFont::SizeResolved))
                resolved = layoutFont(resolved);
            if (flags & QMarkdownPrivate::Emphasis) resolved.setItalic(true);
            if (flags & QMarkdownPrivate::Strong) resolved.setWeight(qMax(resolved.weight(), QFont::Bold));
            QTextCharFormat format;
            format.setFont(resolved);
            if (linked) {
                format.setForeground(m_linkColor);
                format.setFontUnderline(m_linkUnderline);
            }
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
        if (m_text.isEmpty()) m_logicalHeight = QFontMetricsF(m_layoutFont).height();
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

void FormattedText::setLinkSpans(const QVariantList &value)
{
    if (m_links == value) return;
    m_links = value; m_layout.reset(); emit linkSpansChanged(); polish();
}
void FormattedText::setLinkColor(const QColor &value)
{
    if (m_linkColor == value) return;
    m_linkColor = value; emit linkColorChanged(); polish();
}
void FormattedText::setLinkUnderline(bool value)
{
    if (m_linkUnderline == value) return;
    m_linkUnderline = value; emit linkUnderlineChanged(); polish();
}
int FormattedText::linkAt(qreal x, qreal y) const
{
    if (!m_layout || m_layout->text().size() != m_text.size()) return -1;
    const QPointF point = QPointF(x, y) - m_paintOffset;
    for (int lineIndex = 0; lineIndex < m_layout->lineCount(); ++lineIndex) {
        const auto line = m_layout->lineAt(lineIndex);
        if (point.y() < line.y() || point.y() >= line.y() + line.height()) continue;
        // Character cells, rather than nearest-cursor rounding, exclude blank
        // line tails and retain disjoint visual runs in bidirectional labels.
        for (int pos = line.textStart(); pos < line.textStart() + line.textLength(); ++pos) {
            if (!m_layout->isValidCursorPosition(pos) || m_text[pos] == u'\n'
                || m_text[pos] == QChar::LineSeparator) continue;
            const qreal leading = line.cursorToX(pos, QTextLine::Leading);
            const qreal trailing = line.cursorToX(pos, QTextLine::Trailing);
            if (point.x() < qMin(leading, trailing) || point.x() >= qMax(leading, trailing)) continue;
            for (int identity = 0; identity < m_links.size(); ++identity) {
                const auto link = m_links[identity].toMap();
                const int start = link.value("start").toInt();
                if (pos >= start && pos < start + link.value("length").toInt()) return identity;
            }
            return -1;
        }
    }
    return -1;
}
QString FormattedText::linkDestination(int identity) const
{
    return identity >= 0 && identity < m_links.size()
        ? m_links[identity].toMap().value("destination").toString() : QString();
}
