#pragma once
#include "viewstate.h"
#include <QQmlParserStatus>

// Declarative lifetime and metadata contract for selection and navigation geometry.
class SelectionRegistration : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ANONYMOUS
    Q_PROPERTY(ViewState *coordinator READ coordinator WRITE setCoordinator NOTIFY coordinatorChanged FINAL)
    Q_PROPERTY(QQuickItem *item READ item WRITE setItem NOTIFY itemChanged FINAL)
    Q_PROPERTY(ViewState::SelectionItemKind kind READ kind WRITE setKind NOTIFY metadataChanged FINAL)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY metadataChanged FINAL)
    Q_PROPERTY(int length READ length WRITE setLength NOTIFY metadataChanged FINAL)
    Q_PROPERTY(QString imageLink READ imageLink WRITE setImageLink NOTIFY metadataChanged FINAL)
    Q_PROPERTY(int imageLinkId READ imageLinkId WRITE setImageLinkId NOTIFY metadataChanged FINAL)
    Q_PROPERTY(bool imageLinked READ imageLinked WRITE setImageLinked NOTIFY metadataChanged FINAL)
public:
    explicit SelectionRegistration(QObject *parent = nullptr) : QObject(parent) {}
    ~SelectionRegistration() override;
    void classBegin() override {}
    void componentComplete() override;
    ViewState *coordinator() const { return m_coordinator; }
    QQuickItem *item() const { return m_item; }
    ViewState::SelectionItemKind kind() const { return m_kind; }
    int offset() const { return m_offset; }
    int length() const { return m_length; }
    QString imageLink() const { return m_imageLink; }
    int imageLinkId() const { return m_imageLinkId; }
    void setImageLinkId(int value);
    bool imageLinked() const { return m_imageLinked; }
    void setCoordinator(ViewState *value);
    void setItem(QQuickItem *value);
    void setKind(ViewState::SelectionItemKind value);
    void setOffset(int value);
    void setLength(int value);
    void setImageLink(const QString &value);
    void setImageLinked(bool value);
signals:
    void coordinatorChanged();
    void itemChanged();
    void metadataChanged();
private:
    void detach();
    void update();
    QPointer<ViewState> m_coordinator;
    QPointer<QQuickItem> m_item;
    QMetaObject::Connection m_coordinatorDestroyed, m_itemDestroyed;
    ViewState::SelectionItemKind m_kind = ViewState::TextSelection;
    int m_offset = 0, m_length = 0, m_imageLinkId = -1;
    QString m_imageLink;
    bool m_imageLinked = false, m_complete = false;
};
