#pragma once
#include <QColor>
#include <QPointer>
#include <QQuickWindow>
#include <QQuickItem>

// Axis-aligned decorative line whose painted rectangle is snapped to the physical
// pixel grid of the final window transform. Layout geometry stays logical.
class PixelLineItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(qreal startInset READ startInset WRITE setStartInset NOTIFY joinsChanged FINAL)
    Q_PROPERTY(qreal endThickness READ endThickness WRITE setEndThickness NOTIFY joinsChanged FINAL)
public:
    explicit PixelLineItem(QQuickItem *parent = nullptr);
    ~PixelLineItem() override;
    QColor color() const { return m_color; }
    void setColor(const QColor &value);
    // Long-axis joins: preceding border thickness and final border thickness.
    qreal startInset() const { return m_startInset; }
    void setStartInset(qreal value);
    qreal endThickness() const { return m_endThickness; }
    void setEndThickness(qreal value);
    // Snapped item-local rectangle for the current window transform; empty when unset.
    QRectF snappedRect() const;
    // Called by the per-window tracker after animations advance.
    void revalidate();
signals:
    void colorChanged();
    void joinsChanged();
protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;
private:
    void untrack();
    QColor m_color = Qt::transparent;
    qreal m_startInset = 0;
    qreal m_endThickness = 0;
    QRectF m_painted;
    QPointer<QQuickWindow> m_tracked;
};
