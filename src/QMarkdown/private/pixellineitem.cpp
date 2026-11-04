#include "pixellineitem.h"
#include <QHash>
#include <QSet>
#include <QPointer>
#include <QQuickWindow>
#include <QSGRectangleNode>
#include <QSGRenderNode>
#include <QSGRendererInterface>
#include <QPainter>
#include <QtMath>
#include <cmath>

namespace {
// The software renderer's opaque-region calculation uses whole logical pixels.
// A snapped device pixel can be thinner than that, so paint without claiming an
// opaque logical region. GPU backends keep the ordinary rectangle node.
class SoftwareLineNode : public QSGRenderNode
{
public:
    explicit SoftwareLineNode(QQuickWindow *window) : m_window(window) {}
    QRectF rect() const override { return rectangle; }
    RenderingFlags flags() const override { return BoundedRectRendering | NoExternalRendering; }
    void render(const RenderState *state) override
    {
        auto *painter = static_cast<QPainter *>(m_window->rendererInterface()->getResource(
            m_window, QSGRendererInterface::PainterResource));
        if (!painter) return;
        painter->setWorldTransform(QTransform());
        if (const auto *clip = state->clipRegion()) painter->setClipRegion(*clip, Qt::ReplaceClip);
        painter->setWorldTransform(matrix()->toTransform());
        painter->setOpacity(inheritedOpacity());
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter->fillRect(rectangle, color);
    }
    QRectF rectangle;
    QColor color;
private:
    QQuickWindow *m_window;
};

// One tracker per window: avoids per-ancestor connections for every line.
class LineTracker : public QObject
{
public:
    explicit LineTracker(QQuickWindow *window) : QObject(window)
    {
        connect(window, &QObject::destroyed, this, [window] { registry().remove(window); });
        connect(window, &QQuickWindow::afterAnimating, this, [this] {
            const auto copy = m_lines;
            for (PixelLineItem *line : copy)
                if (line->isVisible()) line->revalidate();
        });
    }
    static LineTracker *of(QQuickWindow *window)
    {
        auto *&tracker = registry()[window];
        if (!tracker) tracker = new LineTracker(window);
        return tracker;
    }
    static LineTracker *existing(QQuickWindow *window) { return registry().value(window); }
    static QHash<QQuickWindow *, LineTracker *> &registry()
    {
        static QHash<QQuickWindow *, LineTracker *> lines;
        return lines;
    }
    void add(PixelLineItem *line) { m_lines.insert(line); }
    void remove(PixelLineItem *line) { m_lines.remove(line); }
private:
    QSet<PixelLineItem *> m_lines;
};

// Thin axis: fixed nearest-pixel thickness from a rounded start edge. Long axis:
// both edges rounded. Both are in device pixels.
// Thickness derives from products such as 1 * 0.99999994 * 1.5; a tolerance keeps a
// mathematically exact half pixel from rounding down on some scroll offsets.
qreal roundPosition(qreal position) { return std::floor(position + 0.5 + 1e-4); }

qreal roundThickness(qreal extent) { return std::floor(extent + 0.5 + 1e-4); }

void snapAxis(qreal start, qreal extent, bool thin, qreal &outStart, qreal &outExtent)
{
    outStart = roundPosition(start);
    if (thin) {
        outExtent = std::max<qreal>(1, roundThickness(extent));
    } else {
        const qreal end = roundPosition(start + extent);
        outExtent = std::max<qreal>(end - outStart, extent > 0 ? 1 : 0);
    }
}
}

PixelLineItem::PixelLineItem(QQuickItem *parent) : QQuickItem(parent)
{
    setFlag(ItemHasContents);
}

PixelLineItem::~PixelLineItem() { untrack(); }

void PixelLineItem::setStartInset(qreal value)
{
    if (m_startInset == value) return;
    m_startInset = value;
    update();
    emit joinsChanged();
}

void PixelLineItem::setEndThickness(qreal value)
{
    if (m_endThickness == value) return;
    m_endThickness = value;
    update();
    emit joinsChanged();
}

void PixelLineItem::setColor(const QColor &value)
{
    if (m_color == value) return;
    m_color = value;
    update();
    emit colorChanged();
}

QRectF PixelLineItem::snappedRect() const
{
    if (!(width() > 0 && height() > 0) || m_color.alpha() == 0 || !window()) return {};
    const QRectF logical(0, 0, width(), height());
    const QPointF origin = mapToScene(QPointF(0, 0));
    const QPointF right = mapToScene(QPointF(width(), 0));
    const QPointF bottom = mapToScene(QPointF(0, height()));
    // Only translation plus positive axis scale can be snapped; otherwise paint as laid out.
    constexpr qreal eps = 1e-6;
    if (std::abs(right.y() - origin.y()) > eps || std::abs(bottom.x() - origin.x()) > eps) return logical;
    const qreal sx = (right.x() - origin.x()) / width();
    const qreal sy = (bottom.y() - origin.y()) / height();
    if (!(sx > 0 && sy > 0)) return logical;
    const qreal dpr = window()->effectiveDevicePixelRatio();
    if (!(dpr > 0)) return logical;
    const qreal devX = origin.x() * dpr, devY = origin.y() * dpr;
    const qreal devW = width() * sx * dpr, devH = height() * sy * dpr;
    const bool thinX = devW <= devH;
    qreal x, w, y, h;
    snapAxis(devX, devW, thinX, x, w);
    snapAxis(devY, devH, !thinX, y, h);
    // Join the long axis to the same snapped edges as the perpendicular borders.
    // The start follows a preceding border; the end includes a final border.
    const qreal scale = (thinX ? sy : sx) * dpr;
    const qreal start = thinX ? devY : devX;
    const qreal extent = thinX ? devH : devW;
    qreal &longStart = thinX ? y : x;
    qreal &longExtent = thinX ? h : w;
    qreal end = longStart + longExtent;
    if (m_startInset > 0)
        longStart = roundPosition(start - m_startInset * scale)
            + std::max<qreal>(1, roundThickness(m_startInset * scale));
    if (m_endThickness > 0)
        end = roundPosition(start + extent - m_endThickness * scale)
            + std::max<qreal>(1, roundThickness(m_endThickness * scale));
    longExtent = std::max<qreal>(0, end - longStart);
    const QRectF result((x - devX) / (sx * dpr), (y - devY) / (sy * dpr),
                        w / (sx * dpr), h / (sy * dpr));
    return result.isEmpty() ? QRectF() : result;
}

void PixelLineItem::revalidate()
{
    if (snappedRect() != m_painted) update();
}

QSGNode *PixelLineItem::updatePaintNode(QSGNode *old, UpdatePaintNodeData *)
{
    m_painted = snappedRect();
    if (m_painted.isEmpty()) {
        delete old;
        m_painted = {};
        return nullptr;
    }
    if (window()->rendererInterface()->graphicsApi() == QSGRendererInterface::Software) {
        auto *node = static_cast<SoftwareLineNode *>(old);
        if (!node) node = new SoftwareLineNode(window());
        node->rectangle = m_painted;
        node->color = m_color;
        node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
        return node;
    }
    auto *node = static_cast<QSGRectangleNode *>(old);
    if (!node) node = window()->createRectangleNode();
    node->setRect(m_painted);
    node->setColor(m_color);
    return node;
}

void PixelLineItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);
    if (change == ItemSceneChange) {
        untrack();
        if (value.window) {
            m_tracked = value.window;
            LineTracker::of(value.window)->add(this);
        }
    } else if (change == ItemVisibleHasChanged || change == ItemParentHasChanged) {
        update();
    }
}

void PixelLineItem::untrack()
{
    if (m_tracked) {
        if (auto *tracker = LineTracker::existing(m_tracked)) tracker->remove(this);
    }
    m_tracked = nullptr;
}
