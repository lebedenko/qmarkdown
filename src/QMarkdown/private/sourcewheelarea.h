#pragma once
#include <QQuickItem>
#include <QCoreApplication>
#include <QWheelEvent>
#include <algorithm>

// QML WheelHandler blocking accepts the event point before emitting wheel;
// rejecting its WheelEvent cannot undo that point acceptance on supported Qt.
// Native item delivery lets acceptance follow the actual viewport movement.
class SourceWheelArea : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *sourceViewport MEMBER m_viewport NOTIFY viewportChanged)
    Q_PROPERTY(qreal lineHeight MEMBER m_lineHeight NOTIFY lineHeightChanged)
public:
    using QQuickItem::QQuickItem;
signals:
    void viewportChanged();
    void lineHeightChanged();
protected:
    void wheelEvent(QWheelEvent *event) override
    {
        if (!m_viewport) { event->ignore(); return; }
        const auto pixels = event->pixelDelta(), angles = event->angleDelta();
        const qreal dx = pixels.x() ? -pixels.x() : -angles.x() / 120.0 * m_lineHeight * 3;
        const qreal dy = pixels.y() ? -pixels.y() : -angles.y() / 120.0 * m_lineHeight * 3;
        const qreal oldX = m_viewport->property("contentX").toReal();
        const qreal oldY = m_viewport->property("contentY").toReal();
        const qreal maxX = std::max(qreal(0), m_viewport->property("contentWidth").toReal() - m_viewport->width());
        const qreal maxY = std::max(qreal(0), m_viewport->property("contentHeight").toReal() - m_viewport->height());
        const bool shift = event->modifiers().testFlag(Qt::ShiftModifier);
        const qreal x = std::clamp(oldX + dx + (shift ? dy : 0), qreal(0), maxX);
        const qreal y = shift ? oldY : std::clamp(oldY + dy, qreal(0), maxY);
        m_viewport->setProperty("contentX", x);
        m_viewport->setProperty("contentY", y);
        const bool moved = x != oldX || y != oldY;
        // A consumed ScrollBegin must still initialize the enclosing native
        // Flickable's gesture state, without forwarding any movement. Otherwise
        // a later unconsumed ScrollUpdate cannot start scrolling that reader.
        if (moved && event->phase() == Qt::ScrollBegin) {
            for (auto *ancestor = parentItem(); ancestor; ancestor = ancestor->parentItem()) {
                QWheelEvent begin(ancestor->mapFromScene(event->scenePosition()), event->globalPosition(),
                    {}, {}, event->buttons(), event->modifiers(), Qt::ScrollBegin,
                    event->inverted(), event->source(), event->pointingDevice());
                begin.setTimestamp(event->timestamp());
                QCoreApplication::sendEvent(ancestor, &begin);
                if (begin.isAccepted()) break;
            }
        }
        event->setAccepted(moved);
        event->point(0).setAccepted(moved);
    }
private:
    QQuickItem *m_viewport = nullptr;
    qreal m_lineHeight = 1;
};
