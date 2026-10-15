#include "taskindicatoritem.h"
#include <QAccessibleObject>
#include <QCoreApplication>
#include <QQuickWindow>

namespace {
class TaskIndicatorAccessible : public QAccessibleObject
{
public:
    explicit TaskIndicatorAccessible(TaskIndicatorHostItem *item) : QAccessibleObject(item) {}
    QQuickItem *item() const { return static_cast<QQuickItem *>(object()); }
    QWindow *window() const override { return item()->window(); }
    int childCount() const override { return 0; }
    QAccessibleInterface *child(int) const override { return nullptr; }
    int indexOfChild(const QAccessibleInterface *) const override { return -1; }
    QAccessibleInterface *parent() const override {
        for (auto *ancestor = item()->parentItem(); ancestor; ancestor = ancestor->parentItem()) {
            auto *accessible = QAccessible::queryAccessibleInterface(ancestor);
            if (accessible && accessible->indexOfChild(this) >= 0) return accessible;
        }
        return QAccessible::queryAccessibleInterface(window());
    }
    QRect rect() const override {
        if (!window() || !item()->isVisible()) return {};
        const auto scene = item()->mapRectToScene(item()->boundingRect());
        return QRectF(window()->mapToGlobal(scene.topLeft().toPoint()), scene.size()).toAlignedRect();
    }
    QAccessible::Role role() const override { return QAccessible::CheckBox; }
    QAccessible::State state() const override {
        QAccessible::State result;
        result.checkable = true;
        result.checked = item()->property("checked").toBool();
        result.readOnly = true;
        result.disabled = !item()->isEnabled();
        result.invisible = !item()->isVisible();
        result.offscreen = item()->width() <= 0 || item()->height() <= 0;
        return result;
    }
    QString text(QAccessible::Text kind) const override {
        if (kind == QAccessible::Name)
            return QCoreApplication::translate("TaskIndicatorHost", state().checked ? "Completed task" : "Incomplete task");
        if (kind == QAccessible::Description)
            return QCoreApplication::translate("TaskIndicatorHost", "Read-only Markdown task");
        return {};
    }
    // No action interface: task state belongs exclusively to the Markdown source.
};
}

void registerTaskIndicatorAccessibility()
{
    QAccessible::installFactory([](const QString &, QObject *object) -> QAccessibleInterface * {
        if (auto *item = qobject_cast<TaskIndicatorHostItem *>(object))
            return new TaskIndicatorAccessible(item);
        return nullptr;
    });
}
