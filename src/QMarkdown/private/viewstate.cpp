#include "viewstate.h"
#include "formattedtext.h"
#include <QCoreApplication>
#include <QtQml/qqml.h>

int BlockModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_blocks.size();
}
QVariant BlockModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_blocks.size()) return {};
    const auto &block = m_blocks[index.row()];
    if (role == Qt::UserRole) return block.text;
    if (role == Qt::UserRole + 1) return block.level;
    if (role == Qt::UserRole + 2) return !block.ranges.isEmpty();
    if (role == Qt::UserRole + 3) {
        QVariantList ranges;
        for (const auto &range : block.ranges)
            ranges.append(QVariantMap{{"start", range.start}, {"length", range.length}, {"flags", range.flags}});
        return ranges;
    }
    if (role == Qt::UserRole + 5) return QVariant::fromValue(static_cast<QAbstractItemModel *>(m_children[index.row()]));
    if (role == Qt::UserRole + 6) return block.tight;
    if (role == Qt::UserRole + 7) {
        QStringList markers;
        for (qsizetype i = 0; i < block.children.size(); ++i)
            markers.append(block.ordered ? QString::number(qint64(block.start) + i) + block.delimiter : QString::fromUtf8("•"));
        return markers;
    }
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::List) return 4;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::Quote) return 5;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::ListItem) return 6;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::HtmlBlock) return 7;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::ThematicBreak) return 3;
    if (role == Qt::UserRole + 4) return block.kind == QMarkdownPrivate::BlockKind::CodeBlock ? 2 : (block.ranges.isEmpty() ? 0 : 1);
    return {};
}
QHash<int, QByteArray> BlockModel::roleNames() const
{
    return {{Qt::UserRole, "blockText"}, {Qt::UserRole + 1, "headingLevel"},
            {Qt::UserRole + 2, "formatted"}, {Qt::UserRole + 3, "formatRanges"}, {Qt::UserRole + 4, "renderKind"}, {Qt::UserRole + 5, "childBlocks"},
            {Qt::UserRole + 6, "tightList"}, {Qt::UserRole + 7, "markers"}};
}
void BlockModel::replace(QVector<QMarkdownPrivate::Block> blocks)
{
    beginResetModel();
    qDeleteAll(m_children);
    m_children.clear();
    m_blocks = std::move(blocks);
    for (const auto &block : m_blocks) {
        BlockModel *child = nullptr;
        if (block.kind == QMarkdownPrivate::BlockKind::List
            || block.kind == QMarkdownPrivate::BlockKind::ListItem
            || block.kind == QMarkdownPrivate::BlockKind::Quote) {
            child = new BlockModel(this);
            child->replace(block.children);
        }
        m_children.append(child);
    }
    endResetModel();
}
ViewState::ViewState(QObject *parent)
    : QObject(parent), m_blocks(this), m_default(this), m_style(&m_default)
{
}
void ViewState::setMarkdown(const QString &value)
{
    if (m_markdown == value) return;
    m_markdown = value;
    m_blocks.replace(QMarkdownPrivate::parse(value));
    emit markdownChanged();
}
void ViewState::setStyle(MarkdownStyle *value)
{
    if (!value) { resetStyle(); return; }
    if (m_style == value) return;
    disconnect(m_destroyed);
    m_style = value;
    if (value != &m_default)
        m_destroyed = connect(value, &QObject::destroyed, this, [this] { resetStyle(); });
    emit styleChanged();
}
void ViewState::resetStyle()
{
    m_default.restoreDefaults();
    setStyle(&m_default);
}

namespace {
void registerPrivateTypes()
{
    qmlRegisterType<FormattedText>("QMarkdown.Private", 0, 5, "FormattedText");
    qmlRegisterType<ViewState>("QMarkdown.Private", 0, 5, "ViewState");
}
}
Q_COREAPP_STARTUP_FUNCTION(registerPrivateTypes)
