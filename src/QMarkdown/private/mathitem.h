#pragma once
#include <QQuickPaintedItem>
#include <QPicture>
#include <atomic>
#include <memory>

namespace QMarkdownPrivate {
bool mathAvailable();
struct MathRecording { QPicture picture; QSizeF size; };
// Immutable recordings cross the worker boundary; the engine never touches an Item.
class MathItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY inputChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY inputChanged FINAL)
    Q_PROPERTY(qreal textSize READ textSize WRITE setTextSize NOTIFY inputChanged FINAL)
    Q_PROPERTY(bool enabled READ mathEnabled WRITE setMathEnabled NOTIFY inputChanged FINAL)
    Q_PROPERTY(bool pending READ pending NOTIFY resultChanged FINAL)
    Q_PROPERTY(bool ready READ ready NOTIFY resultChanged FINAL)
public:
    explicit MathItem(QQuickItem *parent = nullptr);
    ~MathItem() override;
    QString source() const { return m_source; }
    QColor color() const { return m_color; }
    qreal textSize() const { return m_size; }
    bool mathEnabled() const { return m_enabled; }
    bool pending() const { return m_pending; }
    bool ready() const { return bool(m_recording); }
    void setSource(const QString &value);
    void setColor(const QColor &value);
    void setTextSize(qreal value);
    void setMathEnabled(bool value);
    void paint(QPainter *painter) override;
signals:
    void inputChanged();
    void resultChanged();
private:
    void refresh();
    QString m_source;
    QColor m_color = Qt::black;
    qreal m_size = 16;
    bool m_enabled = false;
    bool m_pending = false;
    quint64 m_generation = 0;
    std::shared_ptr<std::atomic_bool> m_cancelled;
    std::shared_ptr<const MathRecording> m_recording;
};
}
