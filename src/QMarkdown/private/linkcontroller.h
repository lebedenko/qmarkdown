#pragma once
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QQmlComponent>

class ViewState;
class LinkTargetItem;

// Logical links outlive presentation delegates, but never their source generation.
class LinkController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int focusedId READ focusedId NOTIFY focusChanged FINAL)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
public:
    explicit LinkController(ViewState *state);
    int focusedId() const;
    Q_INVOKABLE bool hasLinkFocus() const { return focusedId() >= 0; }
    bool enabled() const { return m_enabled; }
    void setEnabled(bool value);
    QColor color() const { return m_color; }
    void setColor(const QColor &value);
    void invalidate();
    void reconcile(const QHash<int, int> &identities);
    void detach();
    void refresh();
    void activate(LinkTargetItem *target);
    void focused(LinkTargetItem *target);
    quint64 generation() const { return m_generation; }
signals:
    void focusChanged();
    void enabledChanged();
    void colorChanged();
private:
    void updateTargets();
    ViewState *m_state;
    bool m_enabled = false, m_queued = false, m_streamingRefresh = false;
    QColor m_color;
    quint64 m_generation = 0;
    QQmlComponent *m_component = nullptr;
    QMap<int, LinkTargetItem *> m_targets;
    QPointer<LinkTargetItem> m_restoreFocus;
};

class LinkTargetItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label CONSTANT FINAL)
    Q_PROPERTY(QString destination READ destination CONSTANT FINAL)
public:
    explicit LinkTargetItem(QQuickItem *parent = nullptr);
    ~LinkTargetItem() override;
    QString label() const { return m_label; }
    QString destination() const { return m_destination; }
    void activate();
    QPointer<LinkController> controller;
    quint64 generation = 0;
    int identity = -1;
    QString m_label, m_destination;
    QVector<QRectF> fragments;
    bool announced = false;
    QRect accessibleRect;
    QRectF navigationRect;
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};
void registerLinkAccessibility();
