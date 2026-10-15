#pragma once
#include <QQuickPaintedItem>
#include <QPainter>
#include <QPainterPath>

class TaskIndicatorItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY checkedChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
public:
    explicit TaskIndicatorItem(QQuickItem *parent = nullptr) : QQuickPaintedItem(parent) {}
    bool checked() const { return m_checked; }
    QColor color() const { return m_color; }
    void setChecked(bool value) {
        if (m_checked == value) return;
        m_checked = value; update(); emit checkedChanged();
    }
    void setColor(const QColor &value) {
        if (m_color == value) return;
        m_color = value; update(); emit colorChanged();
    }
    void paint(QPainter *painter) override {
        const qreal size = qMin(width(), height());
        if (size <= 1) return;
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(m_color, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(QRectF(0.5, 0.5, size - 1, size - 1), 3, 3);
        if (!m_checked) return;
        QPainterPath path;
        path.moveTo(size * 0.23, size * 0.51);
        path.lineTo(size * 0.43, size * 0.71);
        path.lineTo(size * 0.78, size * 0.29);
        painter->setPen(QPen(m_color, qMax(1.5, size * 0.1), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(path);
    }
signals:
    void checkedChanged();
    void colorChanged();
private:
    bool m_checked = false;
    QColor m_color = Qt::black;
};

// A distinct host type lets accessibility omit Qt Quick's default checkbox actions.
class TaskIndicatorHostItem : public QQuickItem
{
    Q_OBJECT
public:
    using QQuickItem::QQuickItem;
};

void registerTaskIndicatorAccessibility();
