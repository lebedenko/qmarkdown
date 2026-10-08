#pragma once
#include <QObject>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

class MarkdownResourcePolicy : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QList<QUrl> allowedFileRoots READ allowedFileRoots WRITE setAllowedFileRoots NOTIFY changed FINAL)
    Q_PROPERTY(QList<QUrl> allowedHttpsOrigins READ allowedHttpsOrigins WRITE setAllowedHttpsOrigins NOTIFY changed FINAL)
    Q_PROPERTY(bool allowQrc READ allowQrc WRITE setAllowQrc NOTIFY changed FINAL)
public:
    explicit MarkdownResourcePolicy(QObject *parent = nullptr) : QObject(parent) {}
    QList<QUrl> allowedFileRoots() const { return m_roots; }
    QList<QUrl> allowedHttpsOrigins() const { return m_origins; }
    bool allowQrc() const { return m_qrc; }
    void setAllowedFileRoots(const QList<QUrl> &value) { if (m_roots == value) return; m_roots = value; emit changed(); }
    void setAllowedHttpsOrigins(const QList<QUrl> &value) { if (m_origins == value) return; m_origins = value; emit changed(); }
    void setAllowQrc(bool value) { if (m_qrc == value) return; m_qrc = value; emit changed(); }
    void restoreDefaults() { m_roots.clear(); m_origins.clear(); m_qrc = false; emit changed(); }
signals:
    void changed();
private:
    QList<QUrl> m_roots, m_origins;
    bool m_qrc = false;
};
