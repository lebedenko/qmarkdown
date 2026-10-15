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
#include <limits>
#include <algorithm>

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
void FormattedText::setCodeSpans(const QVariantList &value)
{
    if (m_codes == value) return;
    m_codes = value; m_layout.reset(); emit codeSpansChanged(); polish();
}
void FormattedText::setCodeHorizontalPadding(qreal value)
{
    value = std::isfinite(value) ? qMax(qreal(0), value) : 0;
    if (m_codeHorizontalPadding == value) return;
    m_codeHorizontalPadding = value; emit codeHorizontalPaddingChanged(); polish();
}
void FormattedText::setCodeVerticalPadding(qreal value)
{
    value = std::isfinite(value) ? qMax(qreal(0), value) : 0;
    if (m_codeVerticalPadding == value) return;
    m_codeVerticalPadding = value; emit codeVerticalPaddingChanged(); polish();
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
void FormattedText::setCodeForeground(const QColor &value)
{
    if (m_codeForeground == value) return;
    m_codeForeground = value; emit codeForegroundChanged(); polish();
}
void FormattedText::setCodeBackground(const QColor &value)
{
    if (m_codeBackground == value) return;
    m_codeBackground = value; emit codeBackgroundChanged(); update();
}
void FormattedText::setCodeCornerRadius(qreal value)
{
    value = std::isfinite(value) ? qMax(qreal(0), value) : 0;
    if (m_codeCornerRadius == value) return;
    m_codeCornerRadius = value; emit codeCornerRadiusChanged(); update();
}
void FormattedText::setHorizontalAlignment(int value)
{
    if (value != Qt::AlignLeft && value != Qt::AlignHCenter && value != Qt::AlignRight) value = Qt::AlignLeft;
    if (m_alignment == value) return;
    m_alignment = value; emit horizontalAlignmentChanged(); polish();
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
    m_codeRects.clear();
    m_cursorCells.clear();
    m_cursorLines.clear();
    m_paintedRuns.clear();
    m_hitRegions.clear();
    m_padded = false;
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
        m_padded = (m_codeHorizontalPadding > 0 || m_codeVerticalPadding > 0)
            && std::any_of(m_ranges.cbegin(), m_ranges.cend(), [](const QVariant &range) {
                return range.toMap().value("flags").toInt() & QMarkdownPrivate::Code;
            });
        option.setAlignment(m_padded ? Qt::AlignLeft : Qt::Alignment(m_alignment));
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
            if (flags & QMarkdownPrivate::Strikethrough) resolved.setStrikeOut(true);
            QTextCharFormat format;
            format.setFont(resolved);
            if ((flags & QMarkdownPrivate::Code) && m_codeForeground.isValid())
                format.setForeground(m_codeForeground);
            if (linked) {
                format.setForeground(m_linkColor);
                format.setFontUnderline(m_linkUnderline);
            }
            formats.append({start, length, format});
        }
        QVector<int> codes(m_text.size(), -1), links(m_text.size(), -1), formatIdentities(m_text.size(), -1);
        int formatEnd = 0, formatIdentity = 0;
        for (const auto &interval : intervals) {
            for (int pos = formatEnd; pos < interval.start; ++pos)
                formatIdentities[pos] = formatIdentity;
            ++formatIdentity;
            formatEnd = interval.start + interval.length;
            for (int pos = interval.start; pos < formatEnd; ++pos)
                formatIdentities[pos] = formatIdentity;
            ++formatIdentity;
        }
        for (int pos = formatEnd; pos < m_text.size(); ++pos)
            formatIdentities[pos] = formatIdentity;
        auto mapSpan = [&](const QVariantMap &span, QVector<int> &map, int identity) {
            const qint64 start = span.value("start").toLongLong();
            const qint64 length = span.value("length").toLongLong();
            if (start < 0 || length <= 0 || start > map.size() || length > map.size() - start) return;
            const qint64 end = start + length;
            for (qint64 pos = qMax(qint64(0), start); pos < qMin(qint64(map.size()), end); ++pos)
                map[int(pos)] = identity;
        };
        for (int i = 0; i < m_codes.size(); ++i) mapSpan(m_codes[i].toMap(), codes, i);
        // Direct private consumers may provide only effective ranges.
        if (m_codes.isEmpty()) for (int i = 0; i < m_ranges.size(); ++i)
            if (m_ranges[i].toMap().value("flags").toInt() & QMarkdownPrivate::Code)
                mapSpan(m_ranges[i].toMap(), codes, i);
        for (int i = 0; i < m_links.size(); ++i) mapSpan(m_links[i].toMap(), links, i);
        // A custom format property preserves individual code shaping boundaries
        // even when their effective fonts and colors are identical.
        if (m_padded && !m_codes.isEmpty()) {
            QList<QTextLayout::FormatRange> split;
            for (const auto &format : formats) {
                for (int start = format.start; start < format.start + format.length;) {
                    int end = start + 1;
                    while (end < format.start + format.length && codes[end] == codes[start]) ++end;
                    auto part = format.format;
                    if (codes[start] >= 0) part.setProperty(QTextFormat::UserProperty, codes[start]);
                    split.append({start, end - start, part});
                    start = end;
                }
            }
            formats = split;
        }
        m_layout->setFormats(formats);
        struct Cell { int source, code; QRectF rect; qreal shift = 0; bool reversed = false; };
        auto cellsFor = [&](QTextLine line) {
            // cursorToX clamps overflowing glyph cells to the assigned width.
            // Qt's natural width is the minimum preserving this break position.
            const qreal originalWidth = line.width();
            if (line.naturalTextWidth() > originalWidth) line.setLineWidth(line.naturalTextWidth());
            const qreal alignmentShift = m_padded ? 0 : (originalWidth - line.width())
                * (m_alignment == Qt::AlignRight ? 1 : m_alignment == Qt::AlignHCenter ? .5 : 0);
            QVector<Cell> cells;
            for (int pos = line.textStart(); pos < line.textStart() + line.textLength(); ++pos) {
                if (!m_layout->isValidCursorPosition(pos) || layoutText[pos] == QChar::LineSeparator) continue;
                const qreal a = line.cursorToX(pos, QTextLine::Leading), b = line.cursorToX(pos, QTextLine::Trailing);
                if (a != b) cells.append({pos, codes[pos], QRectF(qMin(a, b) + alignmentShift, line.y(), std::abs(a - b), line.height()), 0, a > b});
            }
            if (!m_padded && line.width() != originalWidth) line.setLineWidth(originalWidth);
            std::stable_sort(cells.begin(), cells.end(), [](const Cell &a, const Cell &b) { return a.rect.left() < b.rect.left(); });
            return cells;
        };
        auto startsFragment = [](const QVector<Cell> &cells, int i) {
            return cells[i].code >= 0 && (i == 0 || cells[i - 1].code != cells[i].code
                || cells[i].rect.left() > cells[i - 1].rect.right() + .01);
        };
        struct Gutter { int left, right; qreal boundary; };
        auto cacheLine = [&](QTextLine line, const QVector<qreal> &shifts, qreal align,
                             const QVector<Gutter> &gutters = {}) {
            struct Strike { int fragment = -1; QLineF line; qreal thickness = 0; };
            QHash<int, Strike> strikes;
            // Retrieve each shaped line once, then partition glyphs by paint
            // format, color and padding shift. Qt retains fallback fonts and decoration flags.
            const auto runs = line.glyphRuns(-1, -1, QTextLayout::RetrieveGlyphIndexes
                | QTextLayout::RetrieveGlyphPositions | QTextLayout::RetrieveStringIndexes);
            for (const auto &run : runs) {
                const auto positions = run.positions(); const auto indexes = run.stringIndexes();
                const auto glyphs = run.glyphIndexes();
                QList<quint32> paintedGlyphs;
                auto paintedIndexes = indexes; paintedIndexes.clear();
                QList<QPointF> paintedPositions;
                QColor color;
                qreal fragmentShift = 0;
                int fragmentFormat = -1;
                auto flush = [&] {
                    if (paintedGlyphs.isEmpty()) return;
                    auto painted = run;
                    painted.setGlyphIndexes(paintedGlyphs);
                    // Keep the shaped glyph coordinates and painter transform;
                    // decorations use logical font metrics in the same space.
                    qreal left = paintedPositions.first().x();
                    for (const auto &position : paintedPositions) left = qMin(left, position.x());
                    const QPointF origin(left, 0);
                    for (auto &position : paintedPositions) position -= origin;
                    painted.setPositions(paintedPositions);
                    painted.setStringIndexes(paintedIndexes);
                    QRectF ink;
                    for (int i = 0; i < paintedGlyphs.size(); ++i)
                        ink = ink.united(run.rawFont().boundingRect(paintedGlyphs[i]).translated(paintedPositions[i]));
                    QVector<Decoration> decorations;
                    if (run.underline() || run.strikeOut() || run.overline()) {
                        const auto font = run.rawFont();
                        const auto advances = font.advancesForGlyphIndexes(paintedGlyphs);
                        qreal right = 0;
                        for (int i = 0; i < paintedGlyphs.size(); ++i)
                            right = qMax(right, paintedPositions[i].x() + advances[i].x());
                        const qreal baseline = paintedPositions.first().y();
                        const qreal thickness = font.lineThickness();
                        auto includeDecoration = [&](qreal y) {
                            decorations.append({QLineF(0, y, right, y), thickness});
                            ink = ink.united(QRectF(0, y - thickness / 2, right, thickness));
                        };
                        if (run.underline()) includeDecoration(baseline + font.underlinePosition());
                        if (run.strikeOut()) {
                            includeDecoration(baseline - font.ascent() / 3);
                            const Strike strike{int(m_paintedRuns.size()),
                                decorations.last().line.translated(origin), thickness};
                            for (auto source : paintedIndexes) {
                                while (source > quint32(line.textStart()) && !m_layout->isValidCursorPosition(source)) --source;
                                strikes.insert(int(source), strike);
                            }
                        }
                        if (run.overline()) includeDecoration(baseline - font.ascent());
                    }
                    painted.setBoundingRect(ink);
                    m_paintedRuns.append({painted, color, origin, decorations});
                    paintedGlyphs.clear(); paintedPositions.clear(); paintedIndexes.clear();
                };
                for (int i = 0; i < positions.size(); ++i) {
                    int source = int(indexes[i]);
                    while (source > line.textStart() && !m_layout->isValidCursorPosition(source)) --source;
                    const QColor foreground = links[source] >= 0 ? m_linkColor
                        : codes[source] >= 0 && m_codeForeground.isValid() ? m_codeForeground : m_color;
                    const qreal glyphShift = shifts[source - line.textStart()];
                    if (foreground != color || glyphShift != fragmentShift || formatIdentities[source] != fragmentFormat) {
                        flush(); color = foreground; fragmentShift = glyphShift; fragmentFormat = formatIdentities[source];
                    }
                    auto position = positions[i];
                    position.rx() += shifts[source - line.textStart()] + align;
                    paintedGlyphs.append(glyphs[i]); paintedPositions.append(position); paintedIndexes.append(indexes[i]);
                }
                flush();
            }
            for (const auto &gutter : gutters) {
                if (!strikes.contains(gutter.left) || !strikes.contains(gutter.right)) continue;
                const auto left = strikes.value(gutter.left), right = strikes.value(gutter.right);
                const QLineF connector(left.line.p2(), right.line.p1());
                if (connector.dx() <= 0) continue;
                auto append = [&](const Strike &style, QPointF from, QPointF to) {
                    auto &fragment = m_paintedRuns[style.fragment];
                    const QLineF segment(from - fragment.origin, to - fragment.origin);
                    fragment.decorations.append({segment, style.thickness});
                    const qreal half = style.thickness / 2;
                    fragment.glyphs.setBoundingRect(fragment.glyphs.boundingRect().united(
                        QRectF(segment.p1(), segment.p2()).normalized().adjusted(-half, -half, half, half)));
                };
                if (codes[gutter.left] >= 0 && codes[gutter.right] >= 0) {
                    const QPointF boundary = connector.pointAt(qBound(qreal(0),
                        (gutter.boundary - connector.x1()) / connector.dx(), qreal(1)));
                    append(left, connector.p1(), boundary);
                    append(right, boundary, connector.p2());
                } else append(codes[gutter.left] >= 0 ? left : right, connector.p1(), connector.p2());
            }
        };
        m_layout->beginLayout();
        while (true) {
            auto line = m_layout->createLine();
            if (!line.isValid()) break;
            line.setLineWidth(m_layoutWidth);
            if (!m_padded) {
                line.setPosition(QPointF(0, m_logicalHeight));
                for (const auto &cell : cellsFor(line)) {
                    m_cursorCells.append({cell.source, m_layout->nextCursorPosition(cell.source), cell.rect, cell.reversed});
                }
                m_cursorLines.append({line.textStart(), line.y(), line.y() + line.height()});
                cacheLine(line, QVector<qreal>(line.textLength(), 0), 0);
                m_logicalHeight += line.height();
                continue;
            }
            int budget = 0, count = 0;
            QVector<Cell> cells;
            while (true) {
                cells = cellsFor(line); count = 0;
                for (int i = 0; i < cells.size(); ++i) if (startsFragment(cells, i)) ++count;
                if (m_codeHorizontalPadding == 0 || count <= budget) break;
                budget = count;
                line.setLineWidth(qMax(qreal(.01), m_layoutWidth - 2 * m_codeHorizontalPadding * budget));
            }
            const qreal vertical = count ? m_codeVerticalPadding : 0;
            const qreal previousY = line.y();
            line.setPosition({0, m_logicalHeight + vertical});
            // Moving the baseline does not change shaping or cursor advances.
            for (auto &cell : cells) cell.rect.translate(0, line.y() - previousY);
            qreal shift = 0;
            QVector<qreal> shifts(line.textLength(), 0);
            QVector<QRectF> fragments;
            for (int i = 0; i < cells.size(); ++i) {
                auto &cell = cells[i];
                const bool first = startsFragment(cells, i);
                const bool last = cell.code >= 0 && (i + 1 == cells.size() || cells[i + 1].code != cell.code
                    || cells[i + 1].rect.left() > cell.rect.right() + .01);
                // Decide boundaries before translating cells.
                if (first) shift += m_codeHorizontalPadding;
                cell.shift = shift;
                shifts[cell.source - line.textStart()] = shift;
                if (cell.code >= 0) {
                    const QRectF rect = cell.rect.translated(shift, 0).adjusted(
                        first ? -m_codeHorizontalPadding : 0, -vertical,
                        last ? m_codeHorizontalPadding : 0, vertical);
                    if (first) fragments.append(rect);
                    else fragments.last() = fragments.last().united(rect);
                    m_hitRegions.append({rect, links[cell.source]});
                } else m_hitRegions.append({cell.rect.translated(shift, 0), links[cell.source]});
                if (last) shift += m_codeHorizontalPadding;
            }
            const qreal contentWidth = line.naturalTextWidth() + shift;
            const qreal align = m_alignment == Qt::AlignRight ? m_layoutWidth - contentWidth
                : m_alignment == Qt::AlignHCenter ? (m_layoutWidth - contentWidth) / 2 : 0;
            for (auto &rect : fragments) { rect.translate(align, 0); m_codeRects.append(rect); }
            for (int i = 0; i < cells.size(); ++i) {
                auto &region = m_hitRegions[m_hitRegions.size() - cells.size() + i];
                region.rect.translate(align, 0);
                const int pos = cells[i].source;
                m_cursorCells.append({pos, m_layout->nextCursorPosition(pos), region.rect, cells[i].reversed});
            }
            QVector<Gutter> gutters;
            for (int i = 1; i < cells.size(); ++i) {
                const auto &left = cells[i - 1], &right = cells[i];
                if (right.shift > left.shift && qAbs(right.rect.left() - left.rect.right()) < .01)
                    gutters.append({left.source, right.source,
                        left.rect.right() + left.shift + align
                            + (left.code >= 0 ? m_codeHorizontalPadding : 0)});
            }
            cacheLine(line, shifts, align, gutters);
            m_cursorLines.append({line.textStart(), line.y() - vertical, line.y() + line.height() + vertical});
            m_logicalHeight += line.height() + 2 * vertical;
        }
        m_layout->endLayout();
        // Use the same visual character cells as link hit testing, including
        // disjoint bidi runs and complete grapheme cells. No layout padding.
        qsizetype intervalIndex = 0;
        const bool hasCode = std::any_of(intervals.cbegin(), intervals.cend(), [](const auto &interval) {
            return interval.flags & QMarkdownPrivate::Code;
        });
        for (int i = 0; !m_padded && hasCode && i < m_layout->lineCount(); ++i) {
            const auto line = m_layout->lineAt(i);
            QVector<QRectF> cells;
            for (int pos = line.textStart(); pos < line.textStart() + line.textLength(); ++pos) {
                if (!m_layout->isValidCursorPosition(pos) || m_text[pos] == u'\n') continue;
                while (intervalIndex < intervals.size()
                       && pos >= intervals[intervalIndex].start + intervals[intervalIndex].length)
                    ++intervalIndex;
                if (intervalIndex == intervals.size() || pos < intervals[intervalIndex].start
                    || !(intervals[intervalIndex].flags & QMarkdownPrivate::Code)) continue;
                const qreal a = line.cursorToX(pos, QTextLine::Leading);
                const qreal b = line.cursorToX(pos, QTextLine::Trailing);
                if (a != b) cells.append(QRectF(qMin(a, b), line.y(), std::abs(b - a), line.height()));
            }
            std::sort(cells.begin(), cells.end(), [](const QRectF &a, const QRectF &b) { return a.left() < b.left(); });
            for (const auto &cell : cells) {
                if (!m_codeRects.isEmpty() && m_codeRects.last().top() == cell.top()
                    && cell.left() <= m_codeRects.last().right() + 0.01)
                    m_codeRects.last() = m_codeRects.last().united(cell);
                else m_codeRects.append(cell);
            }
        }
        if (m_text.isEmpty()) m_logicalHeight = QFontMetricsF(m_layoutFont).height();
        // Raster bounds include ink overhang; wrapping and parent geometry retain
        // the host's logical width and native line height.
        QRectF bounds(0, 0, m_layoutWidth, m_logicalHeight);
        for (const auto &run : m_paintedRuns) bounds = bounds.united(run.glyphs.boundingRect().translated(run.origin));
        for (const auto &rect : m_codeRects) bounds = bounds.united(rect);
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
    painter->save();
    painter->translate(m_paintOffset);
    painter->setPen(Qt::NoPen);
    painter->setBrush(m_codeBackground);
    for (const auto &rect : m_codeRects)
        painter->drawRoundedRect(rect, m_codeCornerRadius, m_codeCornerRadius);
    for (bool current : {false, true}) {
        painter->setBrush(current ? m_currentSearchColor : m_searchColor);
        for (const auto &value : m_searchRanges) {
            const auto range = value.toMap();
            if (range.value("current").toBool() != current) continue;
            const int start = range.value("start").toInt();
            for (const auto &rect : rangeRectangles(start, start + range.value("length").toInt())) painter->drawRect(rect);
        }
    }
    painter->setBrush(m_selectionColor);
    for (const auto &rect : selectionRectangles()) painter->drawRect(rect);
    painter->setPen(m_color);
    for (const auto &run : m_paintedRuns) {
        painter->setPen(run.color);
        auto glyphs = run.glyphs;
        // Qt's raster glyph path derives decorations from pretransformed
        // positions. Paint them explicitly to avoid applying scale twice,
        // including independently rounded Qt Quick texture dimensions.
        glyphs.setUnderline(false);
        glyphs.setStrikeOut(false);
        glyphs.setOverline(false);
        painter->drawGlyphRun(run.origin, glyphs);
        for (const auto &decoration : run.decorations) {
            painter->setPen(QPen(run.color, decoration.thickness, Qt::SolidLine, Qt::FlatCap));
            painter->drawLine(decoration.line.translated(run.origin));
        }
    }
    if (m_focusedLink >= 0) {
        painter->setPen(QPen(m_focusColor, 2));
        painter->setBrush(Qt::NoBrush);
        for (const auto &value : m_links) {
            const auto link = value.toMap();
            if (link.value("logicalId", -1).toInt() != m_focusedLink) continue;
            const int start = link.value("start").toInt();
            for (const auto &rect : rangeRectangles(start, start + link.value("length").toInt()))
                painter->drawRect(rect.adjusted(1, 1, -1, -1));
        }
    }
    painter->restore();
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
    if (m_padded) {
        for (const auto &region : m_hitRegions)
            if (point.x() >= region.rect.left() && point.x() < region.rect.right()
                && point.y() >= region.rect.top() && point.y() < region.rect.bottom()) return region.link;
        return -1;
    }
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

void FormattedText::setSelectionStart(int value)
{
    if (m_selectionStart == value) return;
    m_selectionStart = value; emit selectionChanged(); update();
}
void FormattedText::setSelectionEnd(int value)
{
    if (m_selectionEnd == value) return;
    m_selectionEnd = value; emit selectionChanged(); update();
}
void FormattedText::setSelectionColor(const QColor &value)
{
    if (m_selectionColor == value) return;
    m_selectionColor = value; emit selectionChanged(); update();
}
QVector<QRectF> FormattedText::selectionRectangles() const
{
    return rangeRectangles(m_selectionStart, m_selectionEnd);
}
QVector<QRectF> FormattedText::rangeRectangles(int start, int end) const
{
    QVector<QRectF> result;
    if (!m_layout || start >= end) return result;
    QVector<QRectF> cells;
    for (const auto &cell : m_cursorCells)
        if (cell.start < end && cell.end > start) cells.append(cell.rect);
    // Logical source order can run backwards or jump across other text in bidi
    // lines. Merge only adjacent visual cells, retaining disjoint link fragments.
    std::sort(cells.begin(), cells.end(), [](const auto &a, const auto &b) {
        return a.top() == b.top() ? a.left() < b.left() : a.top() < b.top();
    });
    for (const auto &rect : cells) {
        if (!result.isEmpty() && result.last().top() == rect.top()
            && rect.left() <= result.last().right() + .01)
            result.last() = result.last().united(rect);
        else result.append(rect);
    }
    return result;
}
int FormattedText::cursorAt(qreal x, qreal y) const
{
    if (!m_layout || m_layout->text().size() != m_text.size()) return 0;
    const QPointF p = QPointF(x, y) - m_paintOffset;
    if (p.y() < 0) return 0;
    if (p.y() >= m_logicalHeight) return int(m_text.size());
    qreal nearest = std::numeric_limits<qreal>::max();
    int position = 0;
    // Prefer the nearest visual line, then its nearest cursor edge. Cursor
    // cells retain source direction after bidi shaping and padding translation.
    qreal lineDistance = std::numeric_limits<qreal>::max();
    const CursorLine *line = nullptr;
    for (const auto &candidate : m_cursorLines) {
        const qreal distance = qMax(qreal(0), qMax(candidate.top - p.y(), p.y() - candidate.bottom));
        if (distance < lineDistance) { lineDistance = distance; line = &candidate; }
    }
    if (!line) return 0;
    position = line->start;
    for (const auto &cell : m_cursorCells) {
        const qreal dy = qMax(qreal(0), qMax(cell.rect.top() - p.y(), p.y() - cell.rect.bottom()));
        if (dy > lineDistance || cell.rect.top() < line->top || cell.rect.bottom() > line->bottom) continue;
        const qreal left = std::abs(p.x() - cell.rect.left()), right = std::abs(p.x() - cell.rect.right());
        const qreal distance = qMin(left, right);
        if (distance < nearest) {
            nearest = distance;
            position = (left <= right) != cell.reversed ? cell.start : cell.end;
        }
    }
    return position;
}

int FormattedText::characterAt(qreal x, qreal y) const
{
    const QPointF point = QPointF(x, y) - m_paintOffset;
    for (const auto &cell : m_cursorCells)
        if (cell.rect.contains(point)) return cell.start;
    return cursorAt(x, y);
}

void FormattedText::setSearchRanges(const QVariantList &value)
{
    if (m_searchRanges == value) return;
    m_searchRanges = value; emit searchPaintingChanged(); update();
}
void FormattedText::setSearchHighlightColor(const QColor &value)
{
    if (m_searchColor == value) return;
    m_searchColor = value; emit searchPaintingChanged(); update();
}
void FormattedText::setCurrentSearchHighlightColor(const QColor &value)
{
    if (m_currentSearchColor == value) return;
    m_currentSearchColor = value; emit searchPaintingChanged(); update();
}
