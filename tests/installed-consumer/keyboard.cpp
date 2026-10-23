#include <QAccessible>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

class KeyboardConsumer : public QObject
{
    Q_OBJECT
    static QList<QAccessibleInterface *> links(QAccessibleInterface *parent) {
        QList<QAccessibleInterface *> result;
        if (!parent) return result;
        for (int i = 0; i < parent->childCount(); ++i) {
            auto *child = parent->child(i);
            if (!child) continue;
            if (child->role() == QAccessible::Link) result.append(child);
            result.append(links(child));
        }
        return result;
    }
private slots:
    void installedKeyboardAndAccessibility() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import QMarkdown 1.17
Item {
    width: 300; height: 200
    Item { objectName: "before"; activeFocusOnTab: true }
    MarkdownView {
        objectName: "reader"; width: 200
        keyboardNavigationEnabled: true
        keyboardFocusColor: "magenta"
        markdown: "[one](first) [two](second)"
    }
    Item { objectName: "after"; activeFocusOnTab: true }
}
)QML", {});
        std::unique_ptr<QQuickItem> root(qobject_cast<QQuickItem *>(component.create()));
        QVERIFY2(root, qPrintable(component.errorString()));
        QQuickWindow window; root->setParentItem(window.contentItem());
        window.resize(300, 200); window.show(); window.requestActivate();
        auto *reader = root->findChild<QQuickItem *>("reader"); QVERIFY(reader);
        auto *before = root->findChild<QQuickItem *>("before"); QVERIFY(before);
        auto *after = root->findChild<QQuickItem *>("after"); QVERIFY(after);
        QSignalSpy activated(reader, SIGNAL(linkActivated(QString)));
        QTRY_COMPARE(links(QAccessible::queryAccessibleInterface(&window)).size(), 2);
        before->forceActiveFocus(); QTest::keyClick(&window, Qt::Key_Tab);
        QTRY_VERIFY(window.activeFocusItem() != before);
        auto *first = QAccessible::queryAccessibleInterface(window.activeFocusItem());
        QVERIFY(first); QCOMPARE(first->role(), QAccessible::Link);
        QCOMPARE(first->text(QAccessible::Name), QString("one"));
        QCOMPARE(first->text(QAccessible::Value), QString("first"));
        QVERIFY(!first->rect().isEmpty());
        QTest::keyClick(&window, Qt::Key_Return);
        QCOMPARE(activated.size(), 1); QCOMPARE(activated.first().first().toString(), QString("first"));
        QTest::keyClick(&window, Qt::Key_Tab);
        auto *second = QAccessible::queryAccessibleInterface(window.activeFocusItem());
        QVERIFY(second); QCOMPARE(second->text(QAccessible::Value), QString("second"));
        QTest::keyClick(&window, Qt::Key_Tab); QVERIFY(after->hasActiveFocus());
        reader->setProperty("keyboardNavigationEnabled", false);
        first->actionInterface()->doAction(QAccessibleActionInterface::pressAction());
        QCOMPARE(activated.size(), 2);
        before->forceActiveFocus(); QTest::keyClick(&window, Qt::Key_Tab); QVERIFY(after->hasActiveFocus());
        root.reset();
    }
};
QTEST_MAIN(KeyboardConsumer)
#include "keyboard.moc"
