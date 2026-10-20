#include "private/document.h"
#include "private/presentation.h"
#include "private/viewstate.h"
#include "private/formattedtext.h"
#include "private/linkcontroller.h"
#include <QAccessible>
#include <QAccessibleActionInterface>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QSignalSpy>
#include <QTemporaryDir>
#include "private/imageitem.h"
#include "markdownresourcepolicy.h"
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
using namespace QMarkdownPrivate;
class FootnotesTest : public QObject {
    Q_OBJECT
    static QList<LinkTargetItem *> links(QQuickItem *root) {
        QList<LinkTargetItem *> result;
        for (auto child : root->childItems()) {
            if (auto link = qobject_cast<LinkTargetItem *>(child); link && link->isVisible()) result.append(link);
            result.append(links(child));
        }
        std::sort(result.begin(), result.end(), [](auto a, auto b) { return a->identity < b->identity; });
        return result;
    }
    static QList<FormattedText *> texts(QQuickItem *root) {
        QList<FormattedText *> result;
        for (auto child : root->childItems()) {
            if (auto text = qobject_cast<FormattedText *>(child)) result.append(text);
            result.append(texts(child));
        }
        return result;
    }
    static QList<QQuickItem *> descendants(QQuickItem *root) {
        QList<QQuickItem *> result;
        for (auto child : root->childItems()) { result.append(child); result.append(descendants(child)); }
        return result;
    }
private slots:
    void richBodyPolicies();
    void projection();
    void nativeNavigationAndAccessibility();
};
void FootnotesTest::projection() {
        ViewState state; QVERIFY(!state.footnotesEnabled());
        state.setFootnotesEnabled(true); state.setSelectionEnabled(true);
        state.setMarkdown("A[^b] B[^a] C[^b]\n\n[^a]: Alpha\n\n    Second\n\n[^b]: Beta\n\n[^unused]: hidden\n");
        state.selectAll(); QCOMPARE(state.selectedText(), "A[1] B[2] C[1]\n\n1. Beta\n\n2. Alpha\n\nSecond");
        state.setSearchActive(true); state.setSearchQuery("[1]"); QCOMPARE(state.searchResultCount(), 2);
        state.setSearchQuery("Back to"); QCOMPARE(state.searchResultCount(), 0);
        state.setSearchQuery("hidden"); QCOMPARE(state.searchResultCount(), 0);
        state.setSearchQuery("Second"); QCOMPARE(state.searchResultCount(), 1);
        state.setSearchActive(true); state.setSearchActive(false);
        QCOMPARE(state.selectedText(), "A[1] B[2] C[1]\n\n1. Beta\n\n2. Alpha\n\nSecond");
        state.setFootnotesEnabled(false); state.selectAll(); QVERIFY(state.selectedText().contains("^a"));
    }
void FootnotesTest::nativeNavigationAndAccessibility() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"QML(
import QtQuick
import QMarkdown 1.16
MarkdownView {
    width: 360; footnotesEnabled: true; inlineMathEnabled: true
    selectionEnabled: true; keyboardNavigationEnabled: true
    markdown: "# fn 1\n\n# fnref 1 1\n\nA[^n] B[^n]\n\n[^n]: # Note\n\n    Text [link](https://example.invalid) and $x^2$"
}
)QML", {});
        QQuickWindow window; window.setColor(Qt::white); window.resize(380, 360);
        std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
        QVERIFY2(view, qPrintable(component.errorString())); view->setParentItem(window.contentItem()); window.show();
        QAccessible::setActive(true);
        QTRY_COMPARE(links(view.get()).size(), 5);
        auto targets = links(view.get());
        QCOMPARE(targets[0]->label(), "Footnote 1"); QCOMPARE(targets[0]->destination(), "#fn-1-1");
        QCOMPARE(targets[1]->destination(), "#fn-1-1"); QCOMPARE(targets[2]->label(), "link");
        QCOMPARE(targets[3]->destination(), "#fnref-1-1-1"); QCOMPARE(targets[4]->destination(), "#fnref-1-2");
        for (auto target : targets) {
            auto accessible = QAccessible::queryAccessibleInterface(target); QVERIFY(accessible);
            QCOMPARE(accessible->role(), QAccessible::Link); QCOMPARE(accessible->text(QAccessible::Name), target->label());
            QVERIFY(!accessible->rect().isEmpty());
        }
        QSignalSpy activated(view.get(), SIGNAL(linkActivated(QString)));
        targets[0]->forceActiveFocus(); QTest::keyClick(&window, Qt::Key_Return);
        QTRY_COMPARE(activated.size(), 1); QCOMPARE(activated[0][0].toString(), "#fn-1-1");
        QTest::keyClick(&window, Qt::Key_Tab); QTRY_VERIFY(targets[1]->hasActiveFocus());
        QTest::keyClick(&window, Qt::Key_Backtab); QTRY_VERIFY(targets[0]->hasActiveFocus());
        auto backlink = QAccessible::queryAccessibleInterface(targets[3]);
        backlink->actionInterface()->doAction(QAccessibleActionInterface::pressAction());
        QTRY_COMPARE(activated.size(), 2); QCOMPARE(activated[1][0].toString(), "#fnref-1-1-1");
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        QSignalSpy navigated(state, &ViewState::navigationRequested);
        QVERIFY(state->navigateToFragment("#fn-1-1")); QTRY_VERIFY(!navigated.isEmpty());
        QVERIFY(state->navigateToFragment("#fnref-1-2")); QVERIFY(!state->navigateToFragment("#missing"));
        QCOMPARE(state->headings().size(), 3);
        QCOMPARE(state->headings()[0].toMap().value("fragmentId").toString(), "fn-1");
        state->selectAll();
        const auto expected = QString("fn 1\n\nfnref 1 1\n\nA[1] B[1]\n\n1. Note\n\nText link and x^2");
        QCOMPARE(state->selectedText(), expected);
        FormattedText *referenceText = nullptr;
        for (auto text : texts(view.get())) if (!text->referenceSpans().isEmpty()) referenceText = text;
        QVERIFY(referenceText); QTRY_VERIFY(referenceText->layout());
        QCOMPARE(referenceText->layout()->text().count(QChar::ObjectReplacementCharacter), 2);
        const auto rect = referenceText->rangeRectangles(1, 4); QCOMPARE(rect.size(), 1);
        QCOMPARE(referenceText->rangeRectangles(2, 3), rect);
        const auto point = referenceText->mapToItem(view.get(), rect[0].center() + referenceText->paintOffset());
        QVERIFY(state->beginSelection(view.get(), point, false));
        state->beginSelection(view.get(), point, false); QCOMPARE(state->selectedText(), "[1]");
        state->selectAll(); state->setSearchActive(true); state->setSearchQuery("[1]"); QCOMPARE(state->searchResultCount(), 2);
        state->setSearchActive(false); QCOMPARE(state->selectedText(), expected);
        const auto capture = qEnvironmentVariable("QMARKDOWN_FOOTNOTE_CAPTURE");
        if (!capture.isEmpty()) { QTest::qWait(30); QVERIFY(window.grabWindow().save(capture)); }
        state->setMarkdown("replacement"); QTRY_COMPARE(links(view.get()).size(), 0);
    }
void FootnotesTest::richBodyPolicies() {
    QTemporaryDir directory; QVERIFY(directory.isValid());
    QImage image(20, 12, QImage::Format_RGB32); image.fill(Qt::red);
    QVERIFY(image.save(directory.filePath("note.png")));
    QQmlEngine engine; QQmlComponent component(&engine);
    component.setData(R"QML(
import QtQuick
import QMarkdown 1.16
MarkdownView {
    width: 360; footnotesEnabled: true; syntaxHighlightingEnabled: true
    markdown: "A[^n]\n\n[^n]: Before ![alt](note.png) after\n\n    ```demo\n    source\n    ```\n\n    ```cpp\n    int x;\n    ```"
    fencedBlockRenderers: ({demo: custom})
    property Component custom: Component { Item {
        objectName: "noteCustom"
        required property string codeText; required property string infoString
        required property string language; required property var markdownStyle
        implicitHeight: 23
    } }
}
)QML", {});
    QQuickWindow window; window.resize(400, 360);
    std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
    QVERIFY2(view, qPrintable(component.errorString())); view->setParentItem(window.contentItem()); window.show();
    view->setProperty("baseUrl", QUrl::fromLocalFile(directory.filePath("doc.md")));
    const auto hasCustom = [&] { for (auto item : descendants(view.get())) if (item->objectName() == "noteCustom") return true; return false; };
    const auto imageCount = [&] { int n = 0; for (auto item : descendants(view.get())) if (qobject_cast<ImageItem *>(item)) ++n; return n; };
    QTRY_VERIFY(hasCustom());
    QCOMPARE(imageCount(), 0);
    auto policy = qobject_cast<MarkdownResourcePolicy *>(view->property("resourcePolicy").value<QObject *>()); QVERIFY(policy);
    policy->setAllowedFileRoots({QUrl::fromLocalFile(directory.path() + '/')});
    QTRY_COMPARE(imageCount(), 1);
    view->setProperty("selectionEnabled", true);
    QMetaObject::invokeMethod(view.get(), "selectAll");
    const auto copied = view->property("selectedText").toString();
    QCOMPARE(copied, "A[1]\n\n1. Before alt after\n\nsource\n\nint x;\n");
    policy->setAllowedFileRoots({}); QTRY_COMPARE(imageCount(), 0);
    QCOMPARE(view->property("selectedText").toString(), copied);
}
QTEST_MAIN(FootnotesTest)
#include "tst_footnotes.moc"
