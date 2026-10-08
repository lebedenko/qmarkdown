#pragma once
#include "../markdownstyle.h"
#include "document.h"
#include <QAbstractListModel>
#include <QPointer>

class BlockModel : public QAbstractListModel
{
    Q_OBJECT
public:
    using QAbstractListModel::QAbstractListModel;
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void replace(QVector<QMarkdownPrivate::Block> blocks);
private:
    QVector<QMarkdownPrivate::Block> m_blocks;
    QVector<BlockModel *> m_children;
};

class ViewState : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QString markdown READ markdown WRITE setMarkdown NOTIFY markdownChanged FINAL)
    Q_PROPERTY(MarkdownStyle *style READ style WRITE setStyle RESET resetStyle NOTIFY styleChanged FINAL)
    Q_PROPERTY(QAbstractItemModel *blocks READ blocks CONSTANT FINAL)
public:
    explicit ViewState(QObject *parent = nullptr);
    QString markdown() const { return m_markdown; }
    void setMarkdown(const QString &value);
    MarkdownStyle *style() const { return m_style; }
    void setStyle(MarkdownStyle *value);
    void resetStyle();
    QAbstractItemModel *blocks() { return &m_blocks; }
signals:
    void markdownChanged();
    void styleChanged();
private:
    QString m_markdown;
    BlockModel m_blocks;
    MarkdownStyle m_default;
    QPointer<MarkdownStyle> m_style;
    QMetaObject::Connection m_destroyed;
};
