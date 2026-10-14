#include "viewstate.h"
#include "formattedtext.h"
#include "imageitem.h"
#include <QCoreApplication>
#include <limits>
#include <tuple>
#include <QtQml/qqml.h>

int BlockModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_blocks.size();
}
QVariant BlockModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_blocks.size()) return {};
    const auto &block = m_blocks[index.row()];
    if (role == Qt::UserRole + 17) {
        const int kind = data(index, Qt::UserRole + 4).toInt();
        return kind == 0 ? 11 : kind == 2 ? 12 : kind == 7 ? 13 : kind;
    }
    if (role == Qt::UserRole + 15) return block.selectionOffset;
    if (role == Qt::UserRole + 16) return block.selectionLength;
    if (role == Qt::UserRole + 14) {
        QVariantList codes;
        for (const auto &code : block.codes)
            codes.append(QVariantMap{{"start", code.start}, {"length", code.length}});
        return codes;
    }
    if (role == Qt::UserRole + 13) return block.tableAlignment;
    if (role == Qt::UserRole + 12) return static_cast<int>(block.taskState);
    if (role == Qt::UserRole + 9) return block.image;
    if (role == Qt::UserRole + 10) return block.imageLink;
    if (role == Qt::UserRole + 11) return block.imageLinked;
    if (role == Qt::UserRole) return block.text;
    if (role == Qt::UserRole + 1) return block.level;
    if (role == Qt::UserRole + 2) return (!block.ranges.isEmpty() || !block.links.isEmpty());
    if (role == Qt::UserRole + 3) {
        QVariantList ranges;
        for (const auto &range : block.ranges)
            ranges.append(QVariantMap{{"start", range.start}, {"length", range.length}, {"flags", range.flags}});
        return ranges;
    }
    if (role == Qt::UserRole + 8) {
        QVariantList links;
        for (const auto &link : block.links)
            links.append(QVariantMap{{"start", link.start}, {"length", link.length}, {"destination", link.destination}});
        return links;
    }
    if (role == Qt::UserRole + 5) return QVariant::fromValue(static_cast<QAbstractItemModel *>(m_children[index.row()]));
    if (role == Qt::UserRole + 6) return block.tight;
    if (role == Qt::UserRole + 7) {
        QStringList markers;
        for (qsizetype i = 0; i < block.children.size(); ++i)
            markers.append(block.ordered ? QString::number(qint64(block.start) + i) + block.delimiter : QString::fromUtf8("•"));
        return markers;
    }
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::Table) return 10;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::Image) return 8;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::Segments) return 9;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::List) return 4;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::Quote) return 5;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::ListItem) return 6;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::HtmlBlock) return 7;
    if (role == Qt::UserRole + 4 && block.kind == QMarkdownPrivate::BlockKind::ThematicBreak) return 3;
    if (role == Qt::UserRole + 4) return block.kind == QMarkdownPrivate::BlockKind::CodeBlock ? 2 : (block.ranges.isEmpty() && block.links.isEmpty() ? 0 : 1);
    return {};
}
QHash<int, QByteArray> BlockModel::roleNames() const
{
    return {{Qt::UserRole, "blockText"}, {Qt::UserRole + 1, "headingLevel"},
            {Qt::UserRole + 2, "formatted"}, {Qt::UserRole + 3, "formatRanges"}, {Qt::UserRole + 4, "renderKind"}, {Qt::UserRole + 5, "childBlocks"},
            {Qt::UserRole + 6, "tightList"}, {Qt::UserRole + 7, "markers"}, {Qt::UserRole + 8, "linkSpans"}, {Qt::UserRole + 9, "imageData"},
            {Qt::UserRole + 10, "imageLink"}, {Qt::UserRole + 11, "imageLinked"},
            {Qt::UserRole + 12, "taskState"}, {Qt::UserRole + 13, "tableAlignment"}, {Qt::UserRole + 14, "codeSpans"}, {Qt::UserRole + 15, "selectionOffset"}, {Qt::UserRole + 16, "selectionLength"}, {Qt::UserRole + 17, "selectionKind"}};
}
void BlockModel::replace(QVector<QMarkdownPrivate::Block> blocks)
{
    beginResetModel();
    qDeleteAll(m_children);
    m_children.clear();
    m_blocks = std::move(blocks);
    for (const auto &block : m_blocks) {
        BlockModel *child = nullptr;
        if (block.kind == QMarkdownPrivate::BlockKind::Segments
            || block.kind == QMarkdownPrivate::BlockKind::List
            || block.kind == QMarkdownPrivate::BlockKind::ListItem
            || block.kind == QMarkdownPrivate::BlockKind::Quote
            || block.kind == QMarkdownPrivate::BlockKind::Table
            || block.kind == QMarkdownPrivate::BlockKind::TableRow
            || block.kind == QMarkdownPrivate::BlockKind::TableCell) {
            child = new BlockModel(this);
            if (block.kind == QMarkdownPrivate::BlockKind::TableCell && block.children.isEmpty()) {
                auto content = block;
                content.kind = QMarkdownPrivate::BlockKind::Paragraph;
                child->replace({std::move(content)});
            } else child->replace(block.children);
        }
        m_children.append(child);
    }
    endResetModel();
}
ViewState::ViewState(QObject *parent)
    : QObject(parent), m_defaultPolicy(this), m_policy(&m_defaultPolicy), m_resources(this), m_blocks(this), m_default(this), m_style(&m_default)
{
    connect(&m_blocks, &QAbstractItemModel::modelAboutToBeReset, this, &ViewState::presentationReset);
    m_policyChanged = connect(m_policy, &MarkdownResourcePolicy::changed, this, &ViewState::reloadResources);
    connect(&m_resources, &ResourceController::changed, this, [this] {
        m_blocks.replace(QMarkdownPrivate::projectImages(m_sourceBlocks, m_resources.images(), m_baseUrl));
    });
}
void ViewState::setBaseUrl(const QUrl &value) {
    if (m_baseUrl == value) return;
    m_baseUrl = value; reloadResources(); emit baseUrlChanged();
}
void ViewState::setResourcePolicy(MarkdownResourcePolicy *value) {
    if (!value) { resetResourcePolicy(); return; }
    if (m_policy == value) return;
    disconnect(m_policyDestroyed); disconnect(m_policyChanged);
    m_policy = value;
    if (value != &m_defaultPolicy)
        m_policyDestroyed = connect(value, &QObject::destroyed, this, &ViewState::resetResourcePolicy);
    m_policyChanged = connect(value, &MarkdownResourcePolicy::changed, this, &ViewState::reloadResources);
    reloadResources(); emit resourcePolicyChanged();
}
void ViewState::resetResourcePolicy() {
    m_defaultPolicy.restoreDefaults(); setResourcePolicy(&m_defaultPolicy);
}
void ViewState::reloadResources() {
    m_resources.restart(m_sourceBlocks, m_baseUrl, m_policy);
    m_blocks.replace(m_sourceBlocks);
}
void ViewState::reparse()
{
    const QMarkdownPrivate::ParseOptions options{
        m_gfmEnabled || m_taskListsEnabled, m_gfmEnabled || m_strikethroughEnabled,
        m_gfmEnabled || m_extendedAutolinksEnabled, m_gfmEnabled || m_tablesEnabled,
        m_gfmEnabled || m_tagFilterEnabled};
    auto flags = [](const auto &o) { return std::make_tuple(o.taskLists, o.strikethrough, o.extendedAutolinks, o.tables, o.tagFilter); };
    if (flags(options) != flags(m_parsedOptions)) clearSelection();
    m_parsedOptions = options;
    m_sourceBlocks = QMarkdownPrivate::parse(m_markdown, options);
    m_projection = QMarkdownPrivate::projectText(m_sourceBlocks);
    reloadResources();
}
void ViewState::setMarkdown(const QString &value)
{
    if (m_markdown == value) return;
    clearSelection();
    m_markdown = value;
    reparse();
    emit markdownChanged();
}
void ViewState::setTaskListsEnabled(bool value)
{
    if (m_taskListsEnabled == value) return;
    m_taskListsEnabled = value;
    reparse();
    emit taskListsEnabledChanged();
}
void ViewState::setStrikethroughEnabled(bool value)
{
    if (m_strikethroughEnabled == value) return;
    m_strikethroughEnabled = value;
    reparse();
    emit strikethroughEnabledChanged();
}
void ViewState::setExtendedAutolinksEnabled(bool value)
{
    if (m_extendedAutolinksEnabled == value) return;
    m_extendedAutolinksEnabled = value;
    reparse();
    emit extendedAutolinksEnabledChanged();
}
void ViewState::setTablesEnabled(bool value)
{
    if (m_tablesEnabled == value) return;
    m_tablesEnabled = value;
    reparse();
    emit tablesEnabledChanged();
}
void ViewState::setTagFilterEnabled(bool value)
{
    if (m_tagFilterEnabled == value) return;
    m_tagFilterEnabled = value;
    reparse();
    emit tagFilterEnabledChanged();
}
void ViewState::setGfmEnabled(bool value)
{
    if (m_gfmEnabled == value) return;
    m_gfmEnabled = value;
    reparse();
    emit gfmEnabledChanged();
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
    qmlRegisterType<FormattedText>("QMarkdown.Private", 1, 0, "FormattedText");
    qmlRegisterType<ImageItem>("QMarkdown.Private", 1, 0, "ImageItem");
    qmlRegisterType<ViewState>("QMarkdown.Private", 1, 0, "ViewState");
}
}
Q_COREAPP_STARTUP_FUNCTION(registerPrivateTypes)

void ViewState::setSelectionEnabled(bool value)
{
    if (m_selectionEnabled == value) return;
    if (!value) clearSelection();
    m_selectionEnabled = value;
    emit selectionEnabledChanged();
    m_blocks.replace(QMarkdownPrivate::projectImages(m_sourceBlocks, m_resources.images(), m_baseUrl));
}
void ViewState::setSelection(int anchor, int active)
{
    anchor = qBound(0, anchor, int(m_projection.size()));
    active = qBound(0, active, int(m_projection.size()));
    if (m_anchor == anchor && m_active == active) return;
    m_anchor = anchor; m_active = active;
    emit selectionChanged();
}
void ViewState::selectAll()
{
    if (!m_selectionEnabled) return;
    m_hasAnchor = true;
    setSelection(0, int(m_projection.size()));
}
void ViewState::clearSelection()
{
    m_hasAnchor = false;
    setSelection(0, 0);
}
void ViewState::requestCopy()
{
    if (m_selectionEnabled && selectionStart() != selectionEnd()) emit copyRequested(selectedText());
}
void ViewState::registerSelectionItem(QQuickItem *item, int kind)
{
    if (!m_selectionEnabled || !item) return;
    unregisterSelectionItem(item);
    m_selectionItems.append({item, kind});
}
void ViewState::unregisterSelectionItem(QQuickItem *item)
{
    m_selectionItems.removeIf([item](const auto &entry) { return !entry.item || entry.item == item; });
}
bool ViewState::selectionExcluded(QQuickItem *view, QPointF point) const
{
    for (const auto &entry : m_selectionItems)
        if (entry.item && entry.kind == 3 && entry.item->isVisible()
            && entry.item->contains(entry.item->mapFromItem(view, point))) return true;
    return false;
}
QQuickItem *ViewState::selectionItemAt(QQuickItem *view, QPointF point) const
{
    QQuickItem *cell = nullptr;
    for (const auto &entry : m_selectionItems)
        if (entry.item && entry.kind == 2 && entry.item->isVisible()
            && entry.item->contains(entry.item->mapFromItem(view, point))) { cell = entry.item; break; }
    QQuickItem *nearest = nullptr;
    qreal distance = std::numeric_limits<qreal>::max();
    int offset = std::numeric_limits<int>::max();
    for (const auto &entry : m_selectionItems) {
        auto *item = entry.item.data();
        if (!item || entry.kind > 1 || !item->isVisible() || item->width() <= 0) continue;
        if (cell && item != cell && !cell->isAncestorOf(item)) continue;
        const auto p = item->mapFromItem(view, point);
        const qreal dx = qMax(qreal(0), qMax(-p.x(), p.x() - item->width()));
        const qreal dy = qMax(qreal(0), qMax(-p.y(), p.y() - item->height()));
        const qreal d = dx * dx + dy * dy;
        const int start = item->property("projectionOffset").toInt();
        if (d < distance || (d == distance && start < offset)) {
            nearest = item; distance = d; offset = start;
        }
    }
    return nearest;
}
int ViewState::selectionPosition(QQuickItem *view, QPointF point) const
{
    auto *item = selectionItemAt(view, point);
    if (!item) return 0;
    const auto p = item->mapFromItem(view, point);
    const int offset = item->property("projectionOffset").toInt();
    if (auto *text = qobject_cast<FormattedText *>(item)) {
        if (p.y() >= item->height()) return offset + item->property("projectionLength").toInt();
        return offset + text->cursorAt(p.x(), p.y());
    }
    return offset + ((p.y() >= item->height() || (p.y() >= 0 && p.x() >= item->width() / 2))
        ? item->property("projectionLength").toInt() : 0);
}
bool ViewState::beginSelection(QQuickItem *view, QPointF point, bool extend)
{
    if (!m_selectionEnabled || !view || selectionExcluded(view, point)) return false;
    const int position = selectionPosition(view, point);
    setSelection(extend && m_hasAnchor ? m_anchor : position, position);
    m_hasAnchor = true;
    return true;
}
void ViewState::extendSelection(QQuickItem *view, QPointF point)
{
    if (m_selectionEnabled && m_hasAnchor && view) setSelection(m_anchor, selectionPosition(view, point));
}
QVariant ViewState::selectionLink(QQuickItem *view, QPointF point) const
{
    if (!view || selectionExcluded(view, point)) return {};
    auto *item = selectionItemAt(view, point);
    if (!item) return {};
    const auto p = item->mapFromItem(view, point);
    if (!item->contains(p)) return {};
    if (auto *text = qobject_cast<FormattedText *>(item)) {
        const int link = text->linkAt(p.x(), p.y());
        return link >= 0 ? QVariant(text->linkDestination(link)) : QVariant();
    }
    return item->property("imageLinked").toBool() ? item->property("imageLink") : QVariant();
}
