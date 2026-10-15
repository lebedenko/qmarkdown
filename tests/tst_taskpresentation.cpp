#include <QAccessible>
#include <QDir>
#include <QImage>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QtQml/QQmlExtensionPlugin>
#include <QtTest/QTest>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

static QList<QQuickItem *> named(QQuickItem *root, const QString &name)
{
    QList<QQuickItem *> result;
    for (auto *child : root->childItems()) {
        if (child->objectName() == name) result.append(child);
        result.append(named(child, name));
    }
    return result;
}

class TaskPresentationTest : public QObject
{
    Q_OBJECT
private slots:
    void styles()
    {
        QTest::failOnWarning();
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl::fromLocalFile(QString(QMARKDOWN_TASK_FIXTURE)));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *window = qobject_cast<QQuickWindow *>(object.get()); QVERIFY(window);
        QVERIFY(QTest::qWaitForWindowExposed(window));
        auto views = named(window->contentItem(), "hostView"); QCOMPARE(views.size(), 1);
        auto *view = views[0];
        QTRY_COMPARE(named(view, "taskIndicator").size(), 2);
        const auto hosts = named(view, "taskIndicator");
        const auto source = view->property("markdown");
        for (auto *host : hosts) {
            auto *accessible = QAccessible::queryAccessibleInterface(host); QVERIFY(accessible);
            QCOMPARE(accessible->role(), QAccessible::CheckBox);
            QVERIFY(accessible->state().readOnly); QVERIFY(!accessible->state().focusable);
            QCOMPARE(accessible->childCount(), 0);
            QVERIFY(!accessible->actionInterface());
            QVERIFY(accessible->parent());
            QVERIFY(accessible->parent()->indexOfChild(accessible) >= 0);
            QVERIFY(!accessible->rect().isEmpty());
            const auto checked = host->property("checked");
            const auto point = host->mapToScene(QPointF(host->width() / 2, host->height() / 2)).toPoint();
            QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, point);
            auto *device = QTest::createTouchDevice();
            QTest::touchEvent(window, device).press(0, point, window);
            QTest::touchEvent(window, device).release(0, point, window);
            QTest::keyClick(window, Qt::Key_Space);
            if (auto *actions = accessible->actionInterface()) actions->doAction(QAccessibleActionInterface::toggleAction());
            QCOMPARE(host->property("checked"), checked); QVERIFY(!host->hasActiveFocus());
        }
        QCOMPARE(view->property("markdown"), source);
        const QString directory = QString(QMARKDOWN_TASK_CAPTURES) + "/" + qEnvironmentVariable("QT_QUICK_CONTROLS_STYLE", "Fusion") + "/" + qEnvironmentVariable("QT_SCALE_FACTOR", "1");
        QVERIFY(QDir().mkpath(directory));
        for (bool dark : {false, true}) {
            window->setProperty("dark", dark);
            // Wait for style transitions and scene-graph rendering before capture.
            QTest::qWait(250);
            auto *ordinary = named(window->contentItem(), "ordinaryChecked").value(0); QVERIFY(ordinary);
            auto *backing = named(view, "viewerTaskControl").value(0); QVERIFY(backing);
            auto *ordinaryIndicator = ordinary->property("indicator").value<QQuickItem *>(); QVERIFY(ordinaryIndicator);
            auto *hostIndicator = backing->property("indicator").value<QQuickItem *>(); QVERIFY(hostIndicator);
            QCOMPARE(hostIndicator->size(), ordinaryIndicator->size());
            auto expected = ordinaryIndicator->grabToImage(); QVERIFY(expected);
            auto actual = hostIndicator->grabToImage(); QVERIFY(actual);
            QTRY_VERIFY(!expected->image().isNull() && !actual->image().isNull());
            QCOMPARE(actual->image(), expected->image());
            const auto image = window->grabWindow(); QVERIFY(!image.isNull());
            QVERIFY(image.save(directory + (dark ? "/dark.png" : "/light.png")));
        }
    }
};
QTEST_MAIN(TaskPresentationTest)
#include "tst_taskpresentation.moc"
