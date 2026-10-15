#include "viewstate.h"
#include "formattedtext.h"
#include "imageitem.h"
#include <QCoreApplication>
#include <QMetaProperty>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QStyleHints>
#include <QTextBoundaryFinder>
#include <QSet>
#include <limits>
#include <tuple>
#include <QtQml/qqml.h>

namespace {
constexpr int selectionScrollInterval = 16;
constexpr qint64 selectionMaximumTick = 50;
constexpr qreal selectionEdgeDistance = 32;
constexpr qreal selectionMaximumSpeed = 600;
}

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
    if (role == CodeInfoRole) return block.infoString;
    if (role == CodeFencedRole) return block.codeFenced;
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
            {TaskStateRole, "taskState"}, {TableAlignmentRole, "tableAlignment"}, {CodeSpansRole, "codeSpans"}, {SelectionOffsetRole, "selectionOffset"}, {SelectionLengthRole, "selectionLength"}, {SelectionKindRole, "selectionKind"}, {CodeInfoRole, "codeInfo"}, {CodeFencedRole, "codeFenced"}};
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
    m_linkTimer.setSingleShot(true);
    connect(&m_linkTimer, &QTimer::timeout, this, [this] {
        const auto link = m_pressedLink;
        m_pressedLink = {};
        if (m_selectionEnabled && link.isValid()) emit selectionLinkActivated(link.toString());
    });
    m_scrollTimer.setInterval(selectionScrollInterval);
    connect(&m_scrollTimer, &QTimer::timeout, this, &ViewState::scrollSelection);
    connect(&m_blocks, &QAbstractItemModel::modelAboutToBeReset, this, [this] {
        m_linkTimer.stop(); m_pressedLink = {}; m_clickElapsed.invalidate();
        emit presentationReset();
    });
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
    const bool semanticChange = flags(options) != flags(m_parsedOptions) || m_sourceChanged;
    if (flags(options) != flags(m_parsedOptions)) clearSelection();
    m_sourceChanged = false;
    m_parsedOptions = options;
    m_sourceBlocks = QMarkdownPrivate::parse(m_markdown, options);
    auto presentation = QMarkdownPrivate::projectDocument(m_sourceBlocks);
    m_selectionUnits.clear();
    auto collect = [this](auto &&self, const auto &blocks) -> void {
        using Kind = QMarkdownPrivate::PresentationKind;
        for (const auto &block : blocks) {
            if (block.kind == Kind::Paragraph || block.kind == Kind::Heading
                || block.kind == Kind::TableCell || block.kind == Kind::CodeBlock || block.kind == Kind::HtmlBlock)
                m_selectionUnits.append({block.selectionOffset, block.selectionOffset + block.selectionLength,
                    block.kind == Kind::CodeBlock || block.kind == Kind::HtmlBlock});
            self(self, block.children);
        }
    };
    collect(collect, presentation.blocks);
    m_projection = std::move(presentation.text);
    m_presentationBlocks = std::move(presentation.blocks);
    if (semanticChange) {
        m_targetStart = -1;
        rebuildHeadings();
        rebuildSearch();
    }
    reloadResources();
}
void ViewState::setMarkdown(const QString &value)
{
    if (m_markdown == value) return;
    clearSelection();
    m_markdown = value;
    m_sourceChanged = true;
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

bool ViewState::validFencedRenderer(QObject *object) const
{
    if (!qobject_cast<QQuickItem *>(object)) return false;
    const auto *meta = object->metaObject();
    for (const char *name : {"codeText", "infoString", "language", "markdownStyle"}) {
        const int index = meta->indexOfProperty(name);
        if (index < 0) return false;
        const auto property = meta->property(index);
        if (!property.isWritable()) return false;
        if (QByteArray(name) != "markdownStyle" && property.metaType() != QMetaType::fromType<QString>()) return false;
    }
    return object->property("markdownStyle").value<QObject *>() == m_style;
}

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
    cancelSelectionGesture();
    m_hasAnchor = false;
    setSelection(0, 0);
}
void ViewState::requestCopy()
{
    if (m_selectionEnabled && selectionStart() != selectionEnd()) emit copyRequested(selectedText());
}
void ViewState::registerSelectionItem(QQuickItem *item, int kind)
{
    if (!item) return;
    unregisterSelectionItem(item);
    m_selectionItems.append({item, kind});
    connect(item, &QQuickItem::xChanged, this, &ViewState::refreshNavigation);
    connect(item, &QQuickItem::yChanged, this, &ViewState::refreshNavigation);
    connect(item, &QQuickItem::widthChanged, this, &ViewState::refreshNavigation);
    connect(item, &QQuickItem::heightChanged, this, &ViewState::refreshNavigation);
    if (auto *text = qobject_cast<::FormattedText *>(item))
        connect(text, &::FormattedText::layoutChanged, this, &ViewState::refreshNavigation);
    for (auto *parent = item->parentItem(); parent && parent != m_navigationView; parent = parent->parentItem()) {
        connect(parent, &QQuickItem::xChanged, this, &ViewState::refreshNavigation, Qt::UniqueConnection);
        connect(parent, &QQuickItem::yChanged, this, &ViewState::refreshNavigation, Qt::UniqueConnection);
    }
    refreshNavigation();
}
void ViewState::unregisterSelectionItem(QQuickItem *item)
{
    if (item) disconnect(item, nullptr, this, nullptr);
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
    if (!m_selectionEnabled || !view || selectionExcluded(view, point)) {
        cancelSelectionGesture();
        return false;
    }
    observeSelectionWindow(view);
    m_gestureActive = true;
    m_linkTimer.stop();
    m_shiftClick = extend;
    m_dragging = false;
    const auto scene = view->mapToScene(point);
    const auto *hints = QGuiApplication::styleHints();
    const auto distance = scene - m_lastClickScene;
    const bool repeated = !extend && m_clickElapsed.isValid()
        && m_clickElapsed.elapsed() <= hints->mouseDoubleClickInterval()
        && qAbs(distance.x()) <= hints->mouseDoubleClickDistance()
        && qAbs(distance.y()) <= hints->mouseDoubleClickDistance();
    m_clickCount = repeated ? m_clickCount % 3 + 1 : 1;
    m_clickElapsed.start();
    m_lastClickScene = scene;
    m_granularity = extend ? 1 : m_clickCount;
    m_pressedLink = !extend && m_clickCount == 1 ? selectionLink(view, point) : QVariant();
    if (m_granularity > 1) {
        const auto unit = selectionUnitAt(view, point, m_granularity);
        m_unitStart = unit.first; m_unitEnd = unit.second;
        setSelection(m_unitStart, m_unitEnd);
    } else {
        const int position = selectionPosition(view, point);
        setSelection(extend && m_hasAnchor ? m_anchor : position, position);
    }
    m_hasAnchor = true;
    return true;
}
void ViewState::extendSelection(QQuickItem *view, QPointF point)
{
    if (!m_selectionEnabled || !m_hasAnchor || !view) return;
    if (m_granularity == 1) {
        setSelection(m_anchor, selectionPosition(view, point));
    } else {
        const auto unit = selectionUnitAt(view, point, m_granularity);
        if (unit.first < m_unitStart) setSelection(m_unitEnd, unit.first);
        else setSelection(m_unitStart, qMax(m_unitEnd, unit.second));
    }
}
QPair<int, int> ViewState::selectionUnitAt(QQuickItem *view, QPointF point, int granularity) const
{
    auto *item = selectionItemAt(view, point);
    if (!item) return {0, 0};
    const int offset = item->property("projectionOffset").toInt();
    const int length = item->property("projectionLength").toInt();
    int position = offset;
    if (auto *text = qobject_cast<::FormattedText *>(item)) {
        const auto local = item->mapFromItem(view, point);
        position += text->characterAt(local.x(), local.y());
    } else if (granularity == 2) return {offset, offset + length};
    // Empty image/cell ranges can share an offset with a preceding terminal
    // newline. Prefer the later semantic unit at such boundaries.
    for (auto it = m_selectionUnits.crbegin(); it != m_selectionUnits.crend(); ++it) {
        const auto &unit = *it;
        if (offset < unit.start || offset + length > unit.end) continue;
        position = qBound(unit.start, position, qMax(unit.start, unit.end - 1));
        if (granularity == 3) {
            if (!unit.lines) return {unit.start, unit.end};
            int start = position, end = position;
            while (start > unit.start && m_projection[start - 1] != u'\n') --start;
            while (end < unit.end && m_projection[end] != u'\n') ++end;
            if (end < unit.end) ++end;
            return {start, end};
        }
        if (unit.start == unit.end) return {unit.start, unit.end};
        const auto text = m_projection.mid(unit.start, unit.end - unit.start);
        const int local = position - unit.start;
        if (text[local].isSpace()) {
            int start = local, end = local + 1;
            while (start > 0 && text[start - 1].isSpace()) --start;
            while (end < text.size() && text[end].isSpace()) ++end;
            return {unit.start + start, unit.start + end};
        }
        QTextBoundaryFinder words(QTextBoundaryFinder::Word, text);
        words.toStart();
        int start = -1;
        do {
            const auto reasons = words.boundaryReasons();
            if (reasons & QTextBoundaryFinder::StartOfItem) start = words.position();
            if ((reasons & QTextBoundaryFinder::EndOfItem) && start >= 0
                && local >= start && local < words.position())
                return {unit.start + start, unit.start + words.position()};
        } while (words.toNextBoundary() >= 0);
        QTextBoundaryFinder graphemes(QTextBoundaryFinder::Grapheme, text);
        graphemes.setPosition(local);
        const int startGrapheme = graphemes.isAtBoundary() ? local : graphemes.toPreviousBoundary();
        graphemes.setPosition(local);
        const int endGrapheme = graphemes.toNextBoundary();
        return {unit.start + startGrapheme, unit.start + endGrapheme};
    }
    return {offset, offset + length};
}
void ViewState::observeSelectionWindow(QQuickItem *view)
{
    disconnect(m_windowActive); disconnect(m_windowChanged);
    m_windowChanged = connect(view, &QQuickItem::windowChanged, this, &ViewState::cancelSelectionGesture);
    if (auto *window = view->window())
        m_windowActive = connect(window, &QWindow::activeChanged, this, [this, window] {
            if (!window->isActive()) cancelSelectionGesture();
        });
}
void ViewState::finishSelectionTap(QQuickItem *view, QPointF point)
{
    if (!m_selectionEnabled || m_dragging || m_shiftClick || m_clickCount != 1) return;
    if (m_pressedLink.isValid() && selectionLink(view, point) == m_pressedLink)
        m_linkTimer.start(QGuiApplication::styleHints()->mouseDoubleClickInterval());
}
void ViewState::dragSelection(QQuickItem *view, QPointF point)
{
    if (!m_selectionEnabled || !m_hasAnchor || !m_gestureActive || !view) return;
    m_dragging = true;
    m_linkTimer.stop(); m_pressedLink = {};
    m_clickElapsed.invalidate();
    m_dragView = view;
    m_pointerScene = view->mapToScene(point);
    extendSelection(view, point);
    if (m_selectionViewport && !m_scrollTimer.isActive()) {
        m_scrollElapsed.start(); m_scrollTimer.start();
    }
}
void ViewState::endSelectionDrag()
{
    m_scrollTimer.stop(); m_dragView = nullptr;
    m_gestureActive = false;
}
void ViewState::cancelSelectionGesture()
{
    endSelectionDrag();
    m_linkTimer.stop(); m_pressedLink = {};
    m_clickElapsed.invalidate(); m_clickCount = 0;
    m_granularity = 1; m_dragging = false;
}
void ViewState::setSelectionViewport(QQuickItem *item)
{
    if (m_selectionViewport == item) return;
    m_scrollTimer.stop(); m_dragView = nullptr;
    disconnect(m_viewportDestroyed);
    m_selectionViewport = item;
    if (item) m_viewportDestroyed = connect(item, &QObject::destroyed, this, [this] {
        m_scrollTimer.stop(); m_dragView = nullptr; emit selectionViewportChanged();
    });
    emit selectionViewportChanged();
}
void ViewState::scrollSelection()
{
    if (!m_selectionEnabled || !m_dragView || !m_selectionViewport
        || !m_dragView->window() || m_selectionViewport->window() != m_dragView->window()) {
        endSelectionDrag(); return;
    }
    const qreal elapsed = qMin(selectionMaximumTick, m_scrollElapsed.restart()) / 1000.0;
    const qreal y = m_selectionViewport->mapFromScene(m_pointerScene).y();
    const qreal height = m_selectionViewport->height();
    const qreal top = y, bottom = height - y;
    qreal speed = 0;
    if (height > 0 && qMin(top, bottom) < selectionEdgeDistance && top != bottom)
        speed = (top < bottom ? -1 : 1) * selectionMaximumSpeed * qBound(qreal(0),
            (selectionEdgeDistance - qMin(top, bottom)) / selectionEdgeDistance, qreal(1));
    if (speed != 0) emit selectionScrollRequested(speed * elapsed);
    // Hosts may synchronously remove the viewport or replace the source.
    if (m_dragView && m_selectionViewport)
        extendSelection(m_dragView, m_dragView->mapFromScene(m_pointerScene));
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

void ViewState::setSearchActive(bool value)
{
    if (m_searchActive == value) return;
    m_searchActive = value;
    emit searchActiveChanged();
    rebuildSearch();
}
void ViewState::setSearchQuery(const QString &value)
{
    if (m_searchQuery == value) return;
    m_searchQuery = value;
    emit searchQueryChanged();
    rebuildSearch();
}
void ViewState::setSearchCaseSensitive(bool value)
{
    if (m_searchCaseSensitive == value) return;
    m_searchCaseSensitive = value;
    emit searchCaseSensitiveChanged();
    rebuildSearch();
}
QVariantList ViewState::searchMatches() const
{
    QVariantList result;
    for (int i = 0; i < m_matches.size(); ++i)
        result.append(QVariantMap{{"start", m_matches[i].start}, {"length", m_matches[i].length}, {"current", i == m_currentMatch}});
    return result;
}
void ViewState::rebuildSearch()
{
    if (m_targetSearch) m_targetStart = -1;
    m_matches.clear();
    if (m_searchActive && !m_searchQuery.isEmpty()) {
        const auto mode = m_searchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        int start = 0;
        while ((start = m_projection.indexOf(m_searchQuery, start, mode)) >= 0) {
            m_matches.append({start, int(m_searchQuery.size())});
            start += int(m_searchQuery.size());
        }
    }
    m_currentMatch = m_matches.isEmpty() ? -1 : 0;
    emit searchChanged();
    if (m_currentMatch >= 0) {
        m_targetSearch = true;
        m_targetStart = m_matches[0].start;
        m_targetLength = m_matches[0].length;
        m_lastTarget = {};
        refreshNavigation();
    }
}
bool ViewState::advanceSearch(int direction)
{
    if (m_matches.isEmpty()) return false;
    m_currentMatch = (m_currentMatch + direction + m_matches.size()) % m_matches.size();
    m_targetStart = m_matches[m_currentMatch].start;
    m_targetLength = m_matches[m_currentMatch].length;
    m_targetSearch = true;
    m_lastTarget = {};
    emit searchChanged();
    refreshNavigation();
    return true;
}
bool ViewState::searchNext() { return advanceSearch(1); }
bool ViewState::searchPrevious() { return advanceSearch(-1); }

void ViewState::rebuildHeadings()
{
    m_headings.clear(); m_headingRanges.clear();
    QSet<QString> used;
    auto collect = [this, &used](auto &&self, const auto &blocks) -> void {
        for (const auto &block : blocks) {
            if (block.kind == QMarkdownPrivate::PresentationKind::Heading) {
                QString base;
                bool space = false;
                for (char32_t ch : block.text.toLower().toUcs4()) {
                    const auto category = QChar::category(ch);
                    if (category >= QChar::Punctuation_Connector && category <= QChar::Punctuation_Other) continue;
                    if (QChar::isSpace(ch)) { space = !base.isEmpty(); continue; }
                    if (space) base += u'-';
                    space = false;
                    base += QString::fromUcs4(&ch, 1);
                }
                if (base.isEmpty()) base = QStringLiteral("section");
                QString id = base;
                for (int suffix = 1; used.contains(id); ++suffix) id = base + u'-' + QString::number(suffix);
                used.insert(id);
                m_headings.append(QVariantMap{{"level", block.level}, {"title", block.text}, {"fragmentId", id}});
                m_headingRanges.append({block.selectionOffset, block.selectionLength});
            }
            self(self, block.children);
        }
    };
    collect(collect, m_presentationBlocks);
    emit headingsChanged();
}
bool ViewState::navigateToHeading(int index)
{
    m_targetStart = -1;
    if (index < 0 || index >= m_headingRanges.size()) return false;
    m_targetSearch = false;
    m_targetStart = m_headingRanges[index].start;
    m_targetLength = m_headingRanges[index].length;
    m_lastTarget = {};
    refreshNavigation();
    return true;
}
bool ViewState::navigateToFragment(const QString &fragment)
{
    m_targetStart = -1;
    if (!fragment.startsWith(u'#')) return false;
    const auto id = QUrl::fromPercentEncoding(fragment.mid(1).toUtf8());
    if (id.isEmpty()) return false;
    for (int i = 0; i < m_headings.size(); ++i)
        if (m_headings[i].toMap().value("fragmentId").toString() == id) return navigateToHeading(i);
    return false;
}
void ViewState::setNavigationView(QQuickItem *view)
{
    if (m_navigationView == view) return;
    m_navigationView = view;
    emit navigationViewChanged();
    refreshNavigation();
}
void ViewState::refreshNavigation()
{
    if (m_navigationQueued || m_targetStart < 0) return;
    m_navigationQueued = true;
    QTimer::singleShot(0, this, [this] {
        m_navigationQueued = false;
        emitNavigation();
    });
}
void ViewState::emitNavigation()
{
    if (!m_navigationView || m_navigationView->width() <= 0 || m_targetStart < 0) return;
    // Resolve pending native text and positioner polish before mapping geometry.
    auto polish = [](auto &&self, QQuickItem *item) -> void {
        item->ensurePolished();
        const auto children = item->childItems();
        for (auto *child : children) self(self, child);
        item->ensurePolished();
    };
    polish(polish, m_navigationView);
    QRectF target, nearest;
    int distance = std::numeric_limits<int>::max();
    for (const auto &entry : m_selectionItems) {
        auto *item = entry.item.data();
        if (!item || !item->isVisible() || item->width() <= 0
            || (entry.kind != TextSelection && entry.kind != ImageSelection)) continue;
        const int offset = item->property("projectionOffset").toInt();
        const int length = item->property("projectionLength").toInt();
        const auto bounds = item->mapRectToItem(m_navigationView, item->boundingRect());
        const int d = qMax(0, qMax(offset - m_targetStart, m_targetStart - offset - length));
        if (d < distance) { distance = d; nearest = bounds; }
        if (offset > m_targetStart + m_targetLength || offset + length < m_targetStart) continue;
        if (m_targetLength > 0 && (offset >= m_targetStart + m_targetLength || offset + length <= m_targetStart)) continue;
        if (auto *text = qobject_cast<::FormattedText *>(item)) {
            for (const auto &rect : text->rangeRectangles(m_targetStart - offset, m_targetStart + m_targetLength - offset)) {
                const auto mapped = item->mapRectToItem(m_navigationView, rect.translated(text->paintOffset()));
                target = target.isNull() ? mapped : target.united(mapped);
            }
        } else target = target.isNull() ? bounds : target.united(bounds);
    }
    if (target.isNull()) target = nearest;
    if (!target.isEmpty() && target != m_lastTarget) {
        m_lastTarget = target;
        emit navigationRequested(target);
    }
}
