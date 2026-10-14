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
    if (role == SelectionKindRole) {
        const int kind = data(index, RenderKindRole).toInt();
        return kind == ViewState::PlainText ? ViewState::SelectedText
            : kind == ViewState::CodeBlock ? ViewState::SelectedCode
            : kind == ViewState::HtmlBlock ? ViewState::SelectedHtml : kind;
    }
    if (role == SelectionOffsetRole) return block.selectionOffset;
    if (role == SelectionLengthRole) return block.selectionLength;
    if (role == CodeSpansRole) {
        QVariantList codes;
        for (const auto &code : block.codes)
            codes.append(QVariantMap{{"start", code.start}, {"length", code.length}});
        return codes;
    }
    if (role == TableAlignmentRole) return block.tableAlignment;
    if (role == TaskStateRole) return static_cast<int>(block.taskState);
    if (role == ImageDataRole) return block.image;
    if (role == ImageLinkRole) return block.imageLink;
    if (role == ImageLinkedRole) return block.imageLinked;
    if (role == BlockTextRole) return block.text;
    if (role == HeadingLevelRole) return block.level;
    if (role == FormattedRole) return (!block.ranges.isEmpty() || !block.links.isEmpty());
    if (role == FormatRangesRole) {
        QVariantList ranges;
        for (const auto &range : block.ranges)
            ranges.append(QVariantMap{{"start", range.start}, {"length", range.length}, {"flags", range.flags}});
        return ranges;
    }
    if (role == LinkSpansRole) {
        QVariantList links;
        for (const auto &link : block.links)
            links.append(QVariantMap{{"start", link.start}, {"length", link.length}, {"destination", link.destination}});
        return links;
    }
    if (role == ChildBlocksRole) return QVariant::fromValue(static_cast<QAbstractItemModel *>(m_children[index.row()]));
    if (role == TightListRole) return block.tight;
    if (role == MarkersRole) {
        QStringList markers;
        for (qsizetype i = 0; i < block.children.size(); ++i)
            markers.append(block.ordered ? QString::number(qint64(block.start) + i) + block.delimiter : QString::fromUtf8("•"));
        return markers;
    }
    if (role == RenderKindRole && block.kind == QMarkdownPrivate::PresentationKind::Table) return ViewState::Table;
    if (role == RenderKindRole && block.kind == QMarkdownPrivate::PresentationKind::Image) return ViewState::Image;
    if (role == RenderKindRole && block.kind == QMarkdownPrivate::PresentationKind::Segments) return ViewState::Segments;
    if (role == RenderKindRole && block.kind == QMarkdownPrivate::PresentationKind::List) return ViewState::List;
    if (role == RenderKindRole && block.kind == QMarkdownPrivate::PresentationKind::Quote) return ViewState::Quote;
    if (role == RenderKindRole && block.kind == QMarkdownPrivate::PresentationKind::ListItem) return ViewState::ListItem;
    if (role == RenderKindRole && block.kind == QMarkdownPrivate::PresentationKind::HtmlBlock) return ViewState::HtmlBlock;
    if (role == RenderKindRole && block.kind == QMarkdownPrivate::PresentationKind::ThematicBreak) return ViewState::ThematicBreak;
    if (role == RenderKindRole) return block.kind == QMarkdownPrivate::PresentationKind::CodeBlock ? ViewState::CodeBlock : (block.ranges.isEmpty() && block.links.isEmpty() ? ViewState::PlainText : ViewState::FormattedText);
    return {};
}
QHash<int, QByteArray> BlockModel::roleNames() const
{
    return {{BlockTextRole, "blockText"}, {HeadingLevelRole, "headingLevel"},
            {FormattedRole, "formatted"}, {FormatRangesRole, "formatRanges"}, {RenderKindRole, "renderKind"}, {ChildBlocksRole, "childBlocks"},
            {TightListRole, "tightList"}, {MarkersRole, "markers"}, {LinkSpansRole, "linkSpans"}, {ImageDataRole, "imageData"},
            {ImageLinkRole, "imageLink"}, {ImageLinkedRole, "imageLinked"},
            {TaskStateRole, "taskState"}, {TableAlignmentRole, "tableAlignment"}, {CodeSpansRole, "codeSpans"}, {SelectionOffsetRole, "selectionOffset"}, {SelectionLengthRole, "selectionLength"}, {SelectionKindRole, "selectionKind"}};
}
void BlockModel::replace(QVector<QMarkdownPrivate::PresentationBlock> blocks)
{
    beginResetModel();
    qDeleteAll(m_children);
    m_children.clear();
    m_blocks = std::move(blocks);
    for (const auto &block : m_blocks) {
        BlockModel *child = nullptr;
        if (block.kind == QMarkdownPrivate::PresentationKind::Segments
            || block.kind == QMarkdownPrivate::PresentationKind::List
            || block.kind == QMarkdownPrivate::PresentationKind::ListItem
            || block.kind == QMarkdownPrivate::PresentationKind::Quote
            || block.kind == QMarkdownPrivate::PresentationKind::Table
            || block.kind == QMarkdownPrivate::PresentationKind::TableRow
            || block.kind == QMarkdownPrivate::PresentationKind::TableCell) {
            child = new BlockModel(this);
            if (block.kind == QMarkdownPrivate::PresentationKind::TableCell && block.children.isEmpty()) {
                auto content = block;
                content.kind = QMarkdownPrivate::PresentationKind::Paragraph;
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
        m_blocks.replace(QMarkdownPrivate::projectImages(m_presentationBlocks, m_resources.images(), m_baseUrl));
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
    m_resources.restart(m_sourceBlocks, m_baseUrl, *m_policy);
    m_blocks.replace(m_presentationBlocks);
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
    auto presentation = QMarkdownPrivate::projectDocument(m_sourceBlocks);
    m_projection = std::move(presentation.text);
    m_presentationBlocks = std::move(presentation.blocks);
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
    qmlRegisterType<::FormattedText>("QMarkdown.Private", 1, 0, "FormattedText");
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
    m_blocks.replace(QMarkdownPrivate::projectImages(m_presentationBlocks, m_resources.images(), m_baseUrl));
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
        if (entry.item && entry.kind == ActionSelection && entry.item->isVisible()
            && entry.item->contains(entry.item->mapFromItem(view, point))) return true;
    return false;
}
QQuickItem *ViewState::selectionItemAt(QQuickItem *view, QPointF point) const
{
    QQuickItem *cell = nullptr;
    for (const auto &entry : m_selectionItems)
        if (entry.item && entry.kind == TableCellSelection && entry.item->isVisible()
            && entry.item->contains(entry.item->mapFromItem(view, point))) { cell = entry.item; break; }
    QQuickItem *nearest = nullptr;
    qreal distance = std::numeric_limits<qreal>::max();
    int offset = std::numeric_limits<int>::max();
    for (const auto &entry : m_selectionItems) {
        auto *item = entry.item.data();
        if (!item || (entry.kind != TextSelection && entry.kind != ImageSelection) || !item->isVisible() || item->width() <= 0) continue;
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
    if (auto *text = qobject_cast<::FormattedText *>(item)) {
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
    if (auto *text = qobject_cast<::FormattedText *>(item)) {
        const int link = text->linkAt(p.x(), p.y());
        return link >= 0 ? QVariant(text->linkDestination(link)) : QVariant();
    }
    return item->property("imageLinked").toBool() ? item->property("imageLink") : QVariant();
}
