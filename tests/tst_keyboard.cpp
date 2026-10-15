#include "private/linkcontroller.h"
#include "private/viewstate.h"
#include "private/formattedtext.h"
#include "markdownresourcepolicy.h"
#include <QAccessible>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QImage>
#include <QPainter>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

namespace {
struct AccessibleNotice { QPointer<QObject> object; QAccessible::Event type; };
QList<AccessibleNotice> notices;
void accessibleUpdate(QAccessibleEvent *event) { notices.append({event->object(), event->type()}); }
}

class KeyboardTest : public QObject
{
    Q_OBJECT
    static std::unique_ptr<QQuickItem> create(QQmlEngine &engine, QQuickWindow &window) {
        QQmlComponent component(&engine);
        component.setData(R"(
import QtQuick
import QMarkdown
Item {
    width: 420; height: 700
    Rectangle { objectName: "before"; width: 20; height: 20; activeFocusOnTab: true }
    MarkdownView {
        objectName: "view"; y: 30; width: 240
        codeBlockActions: Component {
            Item {
                required property string codeText
                required property var requestCopy
                implicitHeight: 24
                Rectangle { objectName: "action"; width: 20; height: 20; activeFocusOnTab: true }
            }
        }
    }
    Rectangle { objectName: "after"; y: 650; width: 20; height: 20; activeFocusOnTab: true }
}
)", {});
        std::unique_ptr<QQuickItem> result(qobject_cast<QQuickItem *>(component.create()));
        if (!result) qWarning() << component.errorString();
        if (result) { result->setParentItem(window.contentItem()); window.resize(420, 700); window.show(); window.requestActivate(); }
        return result;
    }
    static QList<LinkTargetItem *> links(QQuickItem *root) {
        QList<LinkTargetItem *> result;
        for (auto *item : root->childItems()) {
            if (auto *link = qobject_cast<LinkTargetItem *>(item); link && link->isVisible()) result.append(link);
            result.append(links(item));
        }
        std::sort(result.begin(), result.end(), [](auto *a, auto *b) { return a->identity < b->identity; });
        return result;
    }
    static QQuickItem *findItem(QQuickItem *root, const QString &name) {
        for (auto *item : root->childItems()) {
            if (item->objectName() == name) return item;
            if (auto *found = findItem(item, name)) return found;
        }
        return nullptr;
    }
private slots:
    void initTestCase() {
        QAccessible::setActive(true);
        QAccessible::installUpdateHandler(accessibleUpdate);
    }
    void cleanupTestCase() { QAccessible::installUpdateHandler(nullptr); }
    void traversalActivationAndSelection() {
        QQmlEngine engine; QQuickWindow window; auto root = create(engine, window); QVERIFY(root);
        auto *view = root->findChild<QQuickItem *>("view");
        auto *before = root->findChild<QQuickItem *>("before");
        auto *after = root->findChild<QQuickItem *>("after");
        QCOMPARE(view->property("keyboardNavigationEnabled").toBool(), false);
        view->setProperty("markdown", "[first](same) [second](same)");
        QTRY_COMPARE(links(root.get()).size(), 2);
        auto targets = links(root.get());
        QVERIFY(!targets[0]->activeFocusOnTab());
        before->forceActiveFocus(); QTest::keyClick(&window, Qt::Key_Tab); QTRY_VERIFY(after->hasActiveFocus());
        view->setProperty("keyboardNavigationEnabled", true);
        view->setProperty("selectionEnabled", true);
        QTRY_COMPARE(links(root.get()).size(), 2);
        QMetaObject::invokeMethod(view, "selectAll");
        const auto selection = view->property("selectedText").toString();
        QSignalSpy activated(view, SIGNAL(linkActivated(QString)));
        QSignalSpy navigation(view, SIGNAL(navigationRequested(QRectF)));
        before->forceActiveFocus(); QTest::keyClick(&window, Qt::Key_Tab);
        QTRY_VERIFY(targets[0]->hasActiveFocus());
        QVERIFY(!navigation.isEmpty());
        QSignalSpy copied(view, SIGNAL(copyRequested(QString)));
        QTest::keyClick(&window, Qt::Key_C, Qt::ControlModifier);
        QCOMPARE(copied.size(), 1);
        QCOMPARE(copied.first().first().toString(), selection);
        QTest::keyClick(&window, Qt::Key_Tab, Qt::ControlModifier);
        QVERIFY(targets[0]->hasActiveFocus());
        QTest::keyClick(&window, Qt::Key_Tab); QTRY_VERIFY(targets[1]->hasActiveFocus());
        QCOMPARE(view->property("selectedText").toString(), selection);
        QCOMPARE(activated.size(), 0);
        QTest::keyClick(&window, Qt::Key_Return); QCOMPARE(activated.size(), 1);
        QCOMPARE(activated.first().first().toString(), QString("same"));
        QKeyEvent repeat(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, {}, true);
        QGuiApplication::sendEvent(&window, &repeat); QCOMPARE(activated.size(), 1);
        QTest::keyClick(&window, Qt::Key_Enter, Qt::ControlModifier); QCOMPARE(activated.size(), 1);
        QTest::keyClick(&window, Qt::Key_Tab); QTRY_VERIFY(after->hasActiveFocus());
        QTest::keyClick(&window, Qt::Key_Tab, Qt::ShiftModifier); QTRY_VERIFY(targets[1]->hasActiveFocus());
        QTest::keyClick(&window, Qt::Key_Tab, Qt::ShiftModifier); QTRY_VERIFY(targets[0]->hasActiveFocus());
        QTest::keyClick(&window, Qt::Key_Tab, Qt::ShiftModifier); QTRY_VERIFY(before->hasActiveFocus());
        view->setProperty("markdown", "no links");
        QTRY_COMPARE(links(root.get()).size(), 0);
        before->forceActiveFocus(); QTest::keyClick(&window, Qt::Key_Tab); QTRY_VERIFY(after->hasActiveFocus());
    }
    void accessibleLinksAndInvalidation() {
        QQmlEngine engine; QQuickWindow window; auto root = create(engine, window); QVERIFY(root);
        auto *view = root->findChild<QQuickItem *>("view");
        view->setProperty("markdown", "# [A &amp; B](one)\n\n> - [nested](two)\n\n| a | b |\n| - | - |\n| [cell](three) | <https://example.com> |\n");
        view->setProperty("tablesEnabled", true);
        QTRY_COMPARE(links(root.get()).size(), 4);
        auto targets = links(root.get());
        QSignalSpy activated(view, SIGNAL(linkActivated(QString)));
        for (auto *target : targets) {
            auto *accessible = QAccessible::queryAccessibleInterface(target);
            QVERIFY(accessible); QCOMPARE(accessible->role(), QAccessible::Link);
            QCOMPARE(accessible->text(QAccessible::Value), target->destination());
            QVERIFY(!accessible->rect().isEmpty());
            QVERIFY(accessible->parent());
            QVERIFY(accessible->parent()->indexOfChild(accessible) >= 0);
            QVERIFY(!accessible->state().focusable);
            QVERIFY(accessible->actionInterface());
            accessible->actionInterface()->doAction(QAccessibleActionInterface::pressAction());
        }
        QCOMPARE(activated.size(), 4);
        QCOMPARE(QAccessible::queryAccessibleInterface(targets[0])->text(QAccessible::Name), QString("A & B"));
        view->setProperty("keyboardNavigationEnabled", true);
        targets[0]->forceActiveFocus();
        QVERIFY(QAccessible::queryAccessibleInterface(targets[0])->state().focused);
        view->setProperty("keyboardNavigationEnabled", false);
        QVERIFY(!targets[0]->hasActiveFocus());
        QVERIFY(!QAccessible::queryAccessibleInterface(targets[0])->state().focusable);
        QPointer<LinkTargetItem> old = targets[0];
        view->setProperty("markdown", "[replacement](new)");
        if (old) old->activate();
        QCOMPARE(activated.size(), 4);
        QTRY_COMPARE(links(root.get()).size(), 1);
        QCOMPARE(links(root.get())[0]->destination(), QString("new"));
        old = links(root.get())[0];
        view->setProperty("strikethroughEnabled", true);
        if (old) old->activate();
        QCOMPARE(activated.size(), 4);
        QTRY_COMPARE(links(root.get()).size(), 1);
    }
    void imagesWrappingAndFocusPersistence() {
        QTemporaryDir directory; QVERIFY(directory.isValid());
        QImage image(32, 24, QImage::Format_ARGB32); image.fill(Qt::green);
        QVERIFY(image.save(directory.filePath("image.png")));
        QQmlEngine engine; QQuickWindow window; auto root = create(engine, window); QVERIFY(root);
        auto *view = root->findChild<QQuickItem *>("view");
        view->setProperty("keyboardNavigationEnabled", true);
        view->setProperty("markdown", "[long wrapped link with עברית text ![image description](image.png) and tail](dest) [duplicate](dest) [![](image.png)](empty)[![](image.png)](empty)\n");
        QTRY_COMPARE(links(root.get()).size(), 2);
        auto *first = links(root.get())[0];
        QVERIFY(first->fragments.size() > 1);
        first->forceActiveFocus();
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        state->setBaseUrl(QUrl::fromLocalFile(directory.path() + "/"));
        state->resourcePolicy()->setAllowedFileRoots({QUrl::fromLocalFile(directory.path() + "/")});
        QTRY_COMPARE(links(root.get()).size(), 4);
        QCOMPARE(links(root.get())[0], first);
        QTRY_VERIFY(first->hasActiveFocus());
        auto *last = links(root.get())[2];
        QCOMPARE(last->label(), QString("empty"));
        QCOMPARE(links(root.get())[3]->destination(), QString("empty"));
        QVERIFY(last->identity != links(root.get())[3]->identity);
        view->setWidth(130);
        QTRY_VERIFY(first->fragments.size() >= 3);
        QCOMPARE(links(root.get())[0], first);
        view->setProperty("searchQuery", "wrapped"); view->setProperty("searchActive", true);
        QTRY_COMPARE(view->property("searchResultCount").toInt(), 1);
        QCOMPARE(links(root.get())[0], first);
        view->setProperty("keyboardFocusColor", QColor(Qt::magenta));
        auto *text = qobject_cast<FormattedText *>(first->parentItem()); QVERIFY(text);
        QImage paint(text->size().toSize(), QImage::Format_ARGB32); paint.fill(Qt::transparent);
        QPainter painter(&paint); text->paint(&painter); painter.end();
        bool colored = false;
        for (int y = 0; y < paint.height(); ++y) for (int x = 0; x < paint.width(); ++x)
            if (paint.pixelColor(x, y) == QColor(Qt::magenta)) colored = true;
        QVERIFY(colored);
    }
    void focusedGeometryAndColorFollowStyle() {
        QQmlEngine engine; QQuickWindow window; auto root = create(engine, window); QVERIFY(root);
        auto *view = root->findChild<QQuickItem *>("view");
        view->setProperty("keyboardNavigationEnabled", true);
        view->setProperty("markdown", "# Heading\n\n[first](one)\n\n[focused](two)");
        QTRY_COMPARE(links(root.get()).size(), 2);
        auto *target = links(root.get())[1]; target->forceActiveFocus();
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        QSignalSpy navigation(view, SIGNAL(navigationRequested(QRectF)));
        const auto previous = target->mapRectToItem(view, target->boundingRect());
        state->style()->setBlockSpacing(70);
        QTRY_VERIFY(target->mapRectToItem(view, target->boundingRect()).top() > previous.top());
        QTRY_VERIFY(!navigation.isEmpty());
        QCOMPARE(navigation.last().first().toRectF(), target->mapRectToItem(view, target->boundingRect()));
        state->style()->setLinkColor(QColor(Qt::cyan));
        QTRY_COMPARE(view->property("keyboardFocusColor").value<QColor>(), QColor(Qt::cyan));
        MarkdownStyle replacement;
        replacement.setLinkColor(QColor(Qt::darkGreen));
        replacement.setBlockSpacing(40);
        state->setStyle(&replacement);
        QTRY_COMPARE(view->property("keyboardFocusColor").value<QColor>(), QColor(Qt::darkGreen));
        QCOMPARE(links(root.get())[1], target);
        QTRY_VERIFY(target->hasActiveFocus());
    }
    void nativeFocusPresentation() {
        QTemporaryDir directory; QVERIFY(directory.isValid());
        QImage image(80, 48, QImage::Format_ARGB32); image.fill(Qt::green);
        QVERIFY(image.save(directory.filePath("image.png")));
        QQmlEngine engine; QQuickWindow window; auto root = create(engine, window); QVERIFY(root);
        auto *view = root->findChild<QQuickItem *>("view");
        view->setProperty("keyboardNavigationEnabled", true);
        view->setProperty("keyboardFocusColor", QColor(Qt::magenta));
        view->setProperty("markdown", "[Wrapped link with עברית text and an image ![image description](image.png) followed by text](dest) [another link](dest)");
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        state->setBaseUrl(QUrl::fromLocalFile(directory.path() + "/"));
        state->resourcePolicy()->setAllowedFileRoots({QUrl::fromLocalFile(directory.path() + "/")});
        QTRY_VERIFY(findItem(view, "markdownImage"));
        QTRY_COMPARE(links(root.get()).size(), 2);
        auto *target = links(root.get())[0]; target->forceActiveFocus();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto *imageItem = findItem(view, "markdownImage"); QVERIFY(imageItem);
        auto *outline = findItem(view, "markdownImageFocus"); QVERIFY(outline);
        QTRY_VERIFY(outline->isVisible());
        QImage frame;
        auto borderColor = [&] {
            frame = window.grabWindow();
            if (frame.isNull()) return QColor();
            const auto point = imageItem->mapToScene(QPointF(1, 1));
            const auto scale = qreal(frame.width()) / window.width();
            return frame.pixelColor(qRound(point.x() * scale), qRound(point.y() * scale));
        };
        QTRY_COMPARE(borderColor(), QColor(Qt::magenta));
        QVERIFY(frame.save(QStringLiteral(QMARKDOWN_KEYBOARD_CAPTURE)));
        const auto height = view->property("contentHeight").toDouble();
        root->findChild<QQuickItem *>("after")->forceActiveFocus();
        QTRY_VERIFY(!outline->isVisible());
        QCOMPARE(view->property("contentHeight").toDouble(), height);
    }
    void focusNotificationsAndHostFocusDuringDecode() {
        QTemporaryDir directory; QVERIFY(directory.isValid());
        QImage image(32, 24, QImage::Format_ARGB32); image.fill(Qt::green);
        QVERIFY(image.save(directory.filePath("image.png")));
        QQmlEngine engine; QQuickWindow window; auto root = create(engine, window); QVERIFY(root);
        auto *view = root->findChild<QQuickItem *>("view");
        view->setProperty("keyboardNavigationEnabled", true);
        view->setProperty("markdown", "[text ![description](image.png)](destination)");
        QTRY_COMPARE(links(root.get()).size(), 1);
        auto *target = links(root.get())[0];
        notices.clear();
        target->forceActiveFocus();
        QVERIFY(QAccessible::queryAccessibleInterface(target)->state().focused);
        bool notified = false;
        for (const auto &notice : notices)
            if (notice.object == target && notice.type == QAccessible::Focus) notified = true;
        QVERIFY(notified);
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        state->setBaseUrl(QUrl::fromLocalFile(directory.path() + "/"));
        state->resourcePolicy()->setAllowedFileRoots({QUrl::fromLocalFile(directory.path() + "/")});
        auto *after = root->findChild<QQuickItem *>("after"); after->forceActiveFocus();
        QTRY_VERIFY(findItem(view, "markdownImage"));
        QTRY_VERIFY(after->hasActiveFocus());
        QSignalSpy activated(view, SIGNAL(linkActivated(QString)));
        view->setProperty("markdown", "[replacement](new)");
        target->activate(); // Synchronous invalidation precedes deferred deletion.
        QCOMPARE(activated.size(), 0);
        QTRY_COMPARE(links(root.get()).size(), 1);
        links(root.get())[0]->forceActiveFocus();
        root.reset();
        QCoreApplication::processEvents();
    }
    void navigationHandlerCanReplaceSource() {
        QQmlEngine engine; QQuickWindow window; auto root = create(engine, window); QVERIFY(root);
        auto *view = root->findChild<QQuickItem *>("view");
        view->setProperty("keyboardNavigationEnabled", true);
        view->setProperty("markdown", "[old](old) [other](other)");
        QTRY_COMPARE(links(root.get()).size(), 2);
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        QSignalSpy activated(view, SIGNAL(linkActivated(QString)));
        bool replaced = false;
        connect(state, &ViewState::navigationRequested, view, [view, &replaced] {
            if (replaced) return;
            replaced = true;
            view->setProperty("markdown", "[replacement](new)");
        });
        QPointer<LinkTargetItem> old = links(root.get())[0];
        old->forceActiveFocus();
        QVERIFY(replaced);
        if (old) old->activate();
        QCOMPARE(activated.size(), 0);
        QTRY_COMPARE(links(root.get()).size(), 1);
        QCOMPARE(links(root.get())[0]->destination(), QString("new"));
    }
    void navigationHandlerCanRebuildLayout() {
        QQmlEngine engine; QQuickWindow window; auto root = create(engine, window); QVERIFY(root);
        auto *view = root->findChild<QQuickItem *>("view");
        view->setProperty("keyboardNavigationEnabled", true);
        view->setProperty("markdown", "# Heading\n\n[first](one)\n\n[second](two)");
        QTRY_COMPARE(links(root.get()).size(), 2);
        auto *target = links(root.get())[0]; target->forceActiveFocus();
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        bool rebuilt = false;
        connect(state, &ViewState::navigationRequested, view, [&] {
            if (rebuilt) return;
            rebuilt = true;
            state->setSelectionEnabled(true);
        });
        state->style()->setBlockSpacing(70);
        QTRY_VERIFY(rebuilt);
        QTRY_COMPARE(links(root.get()).size(), 2);
        QCOMPARE(links(root.get())[0], target);
        QTRY_VERIFY(target->hasActiveFocus());
    }
    void actionControlsStayInChain() {
        QQmlEngine engine; QQuickWindow window; auto root = create(engine, window); QVERIFY(root);
        auto *view = root->findChild<QQuickItem *>("view");
        view->setProperty("keyboardNavigationEnabled", true);
        view->setProperty("markdown", "[before code](one)\n\n```\ncode\n```\n\n[after code](two)");
        QTRY_COMPARE(links(root.get()).size(), 2);
        QTRY_VERIFY(findItem(root.get(), "action"));
        auto targets = links(root.get()); targets[0]->forceActiveFocus();
        QTest::keyClick(&window, Qt::Key_Tab);
        QTRY_VERIFY(findItem(root.get(), "action")->hasActiveFocus());
        QTest::keyClick(&window, Qt::Key_Tab); QTRY_VERIFY(targets[1]->hasActiveFocus());
    }
};
QTEST_MAIN(KeyboardTest)
#include "tst_keyboard.moc"
