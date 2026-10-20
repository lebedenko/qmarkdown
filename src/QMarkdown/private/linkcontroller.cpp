#include "linkcontroller.h"
#include "viewstate.h"
#include "selectionregistration.h"
#include "formattedtext.h"
#include <QAccessibleObject>
#include <QAccessibleActionInterface>
#include <QKeyEvent>
#include <QQuickWindow>
#include <QTimer>
#include <QtQml/qqml.h>
#include <utility>

LinkTargetItem::LinkTargetItem(QQuickItem *parent) : QQuickItem(parent)
{
    setObjectName(QStringLiteral("markdownLink"));
    setAcceptedMouseButtons(Qt::NoButton);
    connect(this, &QQuickItem::activeFocusChanged, this, [this] {
        if (controller) emit controller->focusChanged();
        if (hasActiveFocus() && controller) controller->focused(this);
        QAccessible::State changed;
        changed.focused = true;
        QAccessibleStateChangeEvent state(this, changed);
        QAccessible::updateAccessibility(&state);
        if (hasActiveFocus()) {
            QAccessibleEvent event(this, QAccessible::Focus);
            QAccessible::updateAccessibility(&event);
        }
    });
}
LinkTargetItem::~LinkTargetItem()
{
    if (announced) {
        QAccessibleEvent event(this, QAccessible::ObjectDestroyed);
        QAccessible::updateAccessibility(&event);
    }
}
void LinkTargetItem::activate() { if (controller) controller->activate(this); }
void LinkTargetItem::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        && event->modifiers() == Qt::NoModifier) {
        event->accept();
        if (!event->isAutoRepeat()) activate();
    } else QQuickItem::keyPressEvent(event);
}
void LinkTargetItem::keyReleaseEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        && event->modifiers() == Qt::NoModifier) event->accept();
    else QQuickItem::keyReleaseEvent(event);
}
LinkController::LinkController(ViewState *state) : QObject(state), m_state(state) {}
int LinkController::focusedId() const
{
    if (m_enabled)
        for (auto *target : m_targets) if (target->hasActiveFocus()) return target->identity;
    return -1;
}
void LinkController::setEnabled(bool value)
{
    if (m_enabled == value) return;
    m_enabled = value;
    m_restoreFocus = nullptr;
    const auto targets = m_targets;
    for (auto *target : targets) {
        if (!value) target->setFocus(false);
        target->setActiveFocusOnTab(value && !target->fragments.isEmpty());
        QAccessible::State changed;
        changed.focusable = true;
        QAccessibleStateChangeEvent event(target, changed);
        QAccessible::updateAccessibility(&event);
        target->update();
    }
    emit focusChanged();
    emit enabledChanged();
}
void LinkController::setColor(const QColor &value)
{
    if (m_color == value) return;
    m_color = value;
    for (auto *target : m_targets) target->update();
    emit colorChanged();
}
void LinkController::invalidate()
{
    ++m_generation;
    m_restoreFocus = nullptr;
    const auto old = std::exchange(m_targets, {});
    for (auto *target : old) {
        target->controller = nullptr;
        target->setFocus(false);
        target->setActiveFocusOnTab(false);
        target->setVisible(false);
        target->setParentItem(nullptr);
        target->deleteLater();
    }
    emit focusChanged();
}
void LinkController::detach()
{
    const auto targets = m_targets;
    for (auto *target : targets) {
        if (target->hasActiveFocus()) {
            m_restoreFocus = target;
            if (m_state->navigationView()) m_state->navigationView()->forceActiveFocus(Qt::OtherFocusReason);
        }
        target->setFocus(false);
        target->setParentItem(m_state->navigationView());
        target->setActiveFocusOnTab(false);
        target->setVisible(false);
        QAccessibleEvent event(target, QAccessible::ObjectHide);
        QAccessible::updateAccessibility(&event);
    }
    refresh();
}
void LinkController::refresh()
{
    if (m_queued) return;
    m_queued = true;
    QTimer::singleShot(0, this, [this] { m_queued = false; updateTargets(); });
}
void LinkController::activate(LinkTargetItem *target)
{
    if (!target || target->controller != this || target->generation != m_generation
        || m_targets.value(target->identity) != target || !target->isVisible()
        || !target->isEnabled() || target->fragments.isEmpty()) return;
    emit m_state->selectionLinkActivated(target->destination());
}
void LinkController::focused(LinkTargetItem *target)
{
    if (target->generation != m_generation || !m_state->navigationView()) return;
    target->navigationRect = target->mapRectToItem(m_state->navigationView(), target->boundingRect());
    emit m_state->navigationRequested(target->navigationRect);
}
void LinkController::updateTargets()
{
    auto *view = m_state->navigationView();
    if (!view || !qmlEngine(view)) return;
    if (!m_component) {
        m_component = new QQmlComponent(qmlEngine(view), this);
        m_component->setData(R"(
import QtQuick
import QMarkdown.Private
LinkTargetItem {
    id: linkTarget
    Accessible.role: Accessible.Link
    Accessible.name: linkTarget.label
    Accessible.focusable: linkTarget.activeFocusOnTab
}
)", QUrl());
    }
    struct Geometry { QQuickItem *owner = nullptr; QVector<QRectF> rectangles; };
    QMap<int, Geometry> geometry;
    auto add = [&](int id, QQuickItem *owner, const QRectF &rect) {
        if (id < 0 || rect.isEmpty()) return;
        auto &value = geometry[id];
        if (!value.owner) value.owner = owner;
        value.rectangles.append(rect);
    };
    for (const auto &entry : m_state->m_selectionItems) {
        auto *item = entry.item.data();
        if (!item || !item->isVisible() || item->width() <= 0) continue;
        if (entry.kind == ViewState::TextSelection) {
            auto *text = qobject_cast<FormattedText *>(item);
            if (!text) continue;
            for (const auto &value : text->linkSpans()) {
                const auto link = value.toMap();
                const int start = link.value("start").toInt();
                for (auto rect : text->rangeRectangles(start, start + link.value("length").toInt()))
                    add(link.value("logicalId", -1).toInt(), item,
                        item->mapRectToItem(view, rect.translated(text->paintOffset())));
            }
        } else if (entry.kind == ViewState::DecorativeLinkSelection || (entry.kind == ViewState::ImageSelection && entry.imageLinked)) {
            add(entry.registration->imageLinkId(), item, item->mapRectToItem(view, item->boundingRect()));
        }
    }
    auto collect = [&](auto &&self, const auto &blocks) -> void {
        for (const auto &block : blocks) {
            for (const auto &link : block.logicalLinks) {
                if (m_targets.contains(link.logicalId) || !geometry.contains(link.logicalId)) continue;
                auto *object = m_component->beginCreate(qmlContext(view));
                auto *target = qobject_cast<LinkTargetItem *>(object);
                if (!target) { delete object; continue; }
                target->m_label = link.label.isEmpty() ? link.destination : link.label;
                target->m_destination = link.destination;
                target->identity = link.logicalId;
                target->generation = m_generation;
                target->controller = this;
                target->setParent(this);
                m_component->completeCreate();
                m_targets.insert(link.logicalId, target);
            }
            self(self, block.children);
        }
    };
    collect(collect, m_state->m_presentationBlocks);
    // Iterate by logical ID, preserving source order for targets sharing an owner.
    const auto targets = m_targets;
    const auto generation = m_generation;
    for (auto *target : targets) {
        if (generation != m_generation) return;
        const auto value = geometry.value(target->identity);
        const bool drawable = value.owner && !value.rectangles.isEmpty();
        const bool wasVisible = target->isVisible();
        if (!drawable) target->setFocus(false);
        target->setActiveFocusOnTab(m_enabled && drawable);
        target->setVisible(drawable);
        if (!drawable) { target->fragments.clear(); continue; }
        QRectF bounds;
        for (const auto &rect : value.rectangles) bounds = bounds.isNull() ? rect : bounds.united(rect);
        const auto previous = target->navigationRect;
        target->navigationRect = bounds;
        const bool wasFocused = target->hasActiveFocus();
        target->setParentItem(value.owner);
        if (wasFocused && !target->hasActiveFocus()) {
            auto *current = view->window() ? view->window()->activeFocusItem() : nullptr;
            if (!current || current == view || current == view->window()->contentItem()
                || current->isAncestorOf(target)) target->forceActiveFocus(Qt::OtherFocusReason);
        }
        if (generation != m_generation) return;
        const auto local = value.owner->mapRectFromItem(view, bounds);
        target->setPosition(local.topLeft());
        target->setSize(local.size());
        target->setZ(2);
        target->fragments.clear();
        for (const auto &rect : value.rectangles)
            target->fragments.append(target->mapRectFromItem(view, rect));
        target->update();
        if (!target->announced || !wasVisible) {
            QAccessibleEvent event(target, target->announced ? QAccessible::ObjectShow : QAccessible::ObjectCreated);
            target->announced = true;
            QAccessible::updateAccessibility(&event);
        }
        const auto scene = target->mapRectToScene(target->boundingRect());
        const auto screen = target->window()
            ? QRectF(target->window()->mapToGlobal(scene.topLeft().toPoint()), scene.size()).toAlignedRect() : QRect();
        if (screen != target->accessibleRect) {
            target->accessibleRect = screen;
            QAccessibleEvent event(target, QAccessible::LocationChanged);
            QAccessible::updateAccessibility(&event);
        }
        if (previous != bounds && target->hasActiveFocus()) focused(target);
    }
    if (m_restoreFocus && m_restoreFocus->isVisible()) {
        auto *target = m_restoreFocus.data();
        auto *current = view->window() ? view->window()->activeFocusItem() : nullptr;
        // Qt temporarily focuses the surviving Loader/FocusScope during reset.
        // Wait for drawable geometry, and never reclaim focus from a host control.
        if (current && current != view && current != view->window()->contentItem()
            && current != target && !current->isAncestorOf(target)) {
            m_restoreFocus = nullptr;
        } else if (m_enabled) {
            m_restoreFocus = nullptr;
            target->forceActiveFocus(Qt::OtherFocusReason);
        }
    }
}

namespace {
class LinkAccessible : public QAccessibleObject, public QAccessibleActionInterface
{
public:
    explicit LinkAccessible(LinkTargetItem *item) : QAccessibleObject(item) {}
    LinkTargetItem *item() const { return static_cast<LinkTargetItem *>(object()); }
    QWindow *window() const override { return item()->window(); }
    QAccessibleInterface *parent() const override {
        for (auto *ancestor = item()->parentItem(); ancestor; ancestor = ancestor->parentItem()) {
            auto *accessible = QAccessible::queryAccessibleInterface(ancestor);
            if (accessible && accessible->indexOfChild(this) >= 0) return accessible;
        }
        return QAccessible::queryAccessibleInterface(window());
    }
    int childCount() const override { return 0; }
    QAccessibleInterface *child(int) const override { return nullptr; }
    int indexOfChild(const QAccessibleInterface *) const override { return -1; }
    QRect rect() const override {
        if (!window() || !item()->isVisible()) return {};
        const auto scene = item()->mapRectToScene(item()->boundingRect());
        return QRectF(window()->mapToGlobal(scene.topLeft().toPoint()), scene.size()).toAlignedRect();
    }
    QAccessible::Role role() const override { return QAccessible::Link; }
    QAccessible::State state() const override {
        QAccessible::State result;
        result.focusable = item()->activeFocusOnTab();
        result.focused = item()->hasActiveFocus();
        result.disabled = !item()->isEnabled();
        result.invisible = !item()->isVisible();
        return result;
    }
    QString text(QAccessible::Text kind) const override {
        if (kind == QAccessible::Name) return item()->label();
        if (kind == QAccessible::Value) return item()->destination();
        return {};
    }
    void *interface_cast(QAccessible::InterfaceType type) override {
        if (type == QAccessible::ActionInterface) return static_cast<QAccessibleActionInterface *>(this);
        return QAccessibleObject::interface_cast(type);
    }
    QStringList actionNames() const override { return {pressAction()}; }
    void doAction(const QString &name) override { if (name == pressAction()) item()->activate(); }
    QStringList keyBindingsForAction(const QString &name) const override {
        return name == pressAction() && item()->activeFocusOnTab() ? QStringList{QStringLiteral("Enter")} : QStringList{};
    }
};
}
void registerLinkAccessibility()
{
    QAccessible::installFactory([](const QString &, QObject *object) -> QAccessibleInterface * {
        if (auto *item = qobject_cast<LinkTargetItem *>(object)) return new LinkAccessible(item);
        return nullptr;
    });
}
