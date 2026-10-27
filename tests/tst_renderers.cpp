#include "private/document.h"
#include "private/presentation.h"
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QPointer>
#include <QSignalSpy>
#include <QJSValue>
#include <QtQml/QQmlExtensionPlugin>
#include <QtTest/QTest>
#include <memory>
#include <cmath>
#include <limits>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

class RendererTest : public QObject
{
    Q_OBJECT
    static QList<QQuickItem *> named(QQuickItem *root, const QString &name);
    static std::unique_ptr<QQuickItem> create(QQmlEngine &engine, const QByteArray &extra = {});
private slots:
    void asynchronousDiagram();
    void streamingRetainsRendererAndAction();
    void originAndProjection();
    void matching_data();
    void matching();
    void geometryStyleActionsAndSelection();
    void replacementAndDestruction();
    void creationReplacement();
    void erroredComponent();
    void fallback_data();
    void fallback();
};

QList<QQuickItem *> RendererTest::named(QQuickItem *root, const QString &name)
{
    QList<QQuickItem *> result;
    for (auto *child : root->childItems()) {
        if (child->objectName() == name && child->isVisible()) result.append(child);
        result.append(named(child, name));
    }
    return result;
}
std::unique_ptr<QQuickItem> RendererTest::create(QQmlEngine &engine, const QByteArray &extra)
{
    QQmlComponent component(&engine);
    component.setData(R"(
pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown 1.7
MarkdownView {
id: view
width: 240
property Component renderer: Component {
    Item {
        objectName: "customCode"
        required property string codeText
        required property string infoString
        required property string language
        required property var markdownStyle
        property color foreground: markdownStyle.codeBlockColor
        implicitHeight: width / 2
    }
}
property Component alternative: Component {
    Item {
        objectName: "alternativeCode"
        required property string codeText
        required property string infoString
        required property string language
        required property var markdownStyle
        implicitHeight: 31
    }
}
property Component actions: Component {
    Item {
        objectName: "actions"
        required property string codeText
        required property var requestCopy
        implicitHeight: 19
        function copy() { requestCopy() }
    }
}
function useAlternative() { fencedBlockRenderers = { preview: alternative } }
function disableRenderers() { fencedBlockRenderers = {} }
function useInherited() { fencedBlockRenderers = Object.create({preview: renderer}) }
fencedBlockRenderers: ({preview: renderer})
codeBlockActions: actions
)" + extra + "\n}", {});
    auto object = std::unique_ptr<QQuickItem>(qobject_cast<QQuickItem *>(component.create()));
    if (!object) qWarning() << component.errorString();
    return object;
}

void RendererTest::streamingRetainsRendererAndAction()
{
    QQmlEngine engine; auto view = create(engine); QVERIFY(view);
    view->setProperty("markdown", "```preview\nretained\n```\n\n");
    QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 500); window.show();
    QTRY_COMPARE(named(view.get(), "customCode").size(), 1);
    QTRY_COMPARE(named(view.get(), "actions").size(), 1);
    QPointer<QQuickItem> renderer = named(view.get(), "customCode")[0], actions = named(view.get(), "actions")[0];
    const auto callback = actions->property("requestCopy").value<QJSValue>();
    QSignalSpy copied(view.get(), SIGNAL(codeBlockCopyRequested(QString)));
    view->setProperty("streamingEnabled", true);
    QVERIFY(QMetaObject::invokeMethod(view.get(), "appendMarkdown", Q_ARG(QString, "a new paragraph")));
    QVERIFY(QMetaObject::invokeMethod(view.get(), "flushMarkdown"));
    QVERIFY(renderer); QVERIFY(actions);
    QCOMPARE(named(view.get(), "customCode")[0], renderer.data());
    QCOMPARE(named(view.get(), "actions")[0], actions.data());
    auto retainedCallback = callback; retainedCallback.call(); QCOMPARE(copied.size(), 1);
    view->setProperty("markdown", "replacement");
    retainedCallback.call(); QCOMPARE(copied.size(), 1);
}
void RendererTest::asynchronousDiagram()
{
    QQmlEngine engine; QQuickWindow window;
    auto view = create(engine, R"QML(
    property Component diagram: Component {
        Rectangle {
            id: mock
            objectName: "asyncDiagram"
            required property string codeText
            required property string infoString
            required property string language
            required property var markdownStyle
            property bool ready: false
            property bool failed: false
            color: markdownStyle.codeBlockColor
            implicitHeight: ready && !failed ? width / 2 : source.implicitHeight
            Text { id: source; text: mock.codeText; visible: !mock.ready || mock.failed
                color: mock.markdownStyle.codeBlockColor }
            Timer { interval: 30; running: true; onTriggered: {
                mock.failed = mock.codeText.indexOf("fail") >= 0; mock.ready = true
            } }
        }
    }
    function useDiagram() { fencedBlockRenderers = {preview: diagram} }
    Component.onCompleted: useDiagram()
)QML");
    QVERIFY(view); view->setParentItem(window.contentItem()); window.show();
    view->setProperty("markdown", "```preview\ngraph A --> B\n```");
    QTRY_COMPARE(named(view.get(), "asyncDiagram").size(), 1);
    QPointer<QQuickItem> item = named(view.get(), "asyncDiagram")[0];
    QTRY_VERIFY(item->property("ready").toBool());
    QTRY_COMPARE(item->height(), item->width() / 2);
    view->setWidth(360); QTRY_COMPARE(item->height(), item->width() / 2);
    auto *style = view->property("style").value<QObject *>(); QVERIFY(style);
    style->setProperty("codeBlockColor", QColor("#123456"));
    QTRY_COMPARE(item->property("color").value<QColor>(), QColor("#123456"));
    view->setProperty("searchActive", true); QTRY_VERIFY(item.isNull());
    view->setProperty("searchQuery", "graph");
    QTRY_VERIFY(view->property("searchResultCount").toInt() > 0);
    view->setProperty("searchActive", false);
    QTRY_COMPARE(named(view.get(), "asyncDiagram").size(), 1);
    item = named(view.get(), "asyncDiagram")[0];
    view->setProperty("selectionEnabled", true); QTRY_VERIFY(item.isNull());
    QVERIFY(QMetaObject::invokeMethod(view.get(), "selectAll"));
    QCOMPARE(view->property("selectedText").toString(), "graph A --> B\n");
    view->setProperty("selectionEnabled", false);
    QTRY_COMPARE(named(view.get(), "asyncDiagram").size(), 1);
    item = named(view.get(), "asyncDiagram")[0];
    QVERIFY(QMetaObject::invokeMethod(view.get(), "useAlternative")); QTRY_VERIFY(item.isNull());
    QVERIFY(QMetaObject::invokeMethod(view.get(), "useDiagram"));
    view->setProperty("markdown", "```preview\nfail source\n```");
    QTRY_COMPARE(named(view.get(), "asyncDiagram").size(), 1);
    item = named(view.get(), "asyncDiagram")[0];
    QTRY_VERIFY(item->property("failed").toBool()); QVERIFY(item->height() > 0);
    view->setProperty("markdown", "```preview\npending\n```");
    QTRY_VERIFY(item.isNull());
    QTRY_COMPARE(named(view.get(), "asyncDiagram").size(), 1);
    item = named(view.get(), "asyncDiagram")[0];
    view.reset(); QVERIFY(item.isNull()); QTest::qWait(60);
}

void RendererTest::originAndProjection()
{
    using namespace QMarkdownPrivate;
    const auto blocks = parse("```\n```\n\n    preview\n\n> - ~~~ preview &amp; a\\*\n>   \tcode  \n>   ~~~\n");
    QCOMPARE(blocks.size(), 3);
    QVERIFY(blocks[0].codeFenced); QVERIFY(blocks[0].infoString.isEmpty()); QVERIFY(blocks[0].text.isEmpty());
    QVERIFY(!blocks[1].codeFenced);
    const auto &code = blocks[2].children[0].children[0].children[0];
    QVERIFY(code.codeFenced); QCOMPARE(code.infoString, "preview & a*");
    const auto projected = projectDocument(blocks);
    const auto &presented = projected.blocks[2].children[0].children[0].children[0];
    QVERIFY(presented.codeFenced); QCOMPARE(presented.infoString, code.infoString); QCOMPARE(presented.text, code.text);
    QCOMPARE(presented.language, "preview");
    for (const auto &info : {QString::fromUtf8("\xEF\xBB\xBF\xC2\xA0preview\xE2\x80\xA8options"),
                            QString("\tpreview extra"), QString("preview")}) {
        Block block{}; block.kind = BlockKind::CodeBlock; block.infoString = info;
        QCOMPARE(projectDocument({block}).blocks[0].language, "preview");
    }
    Block whitespace{}; whitespace.kind = BlockKind::CodeBlock; whitespace.infoString = " \t\n";
    QVERIFY(projectDocument({whitespace}).blocks[0].language.isEmpty());
}

void RendererTest::matching_data()
{
    QTest::addColumn<QString>("source"); QTest::addColumn<int>("count");
    QTest::newRow("decoded-info") << "~~~  pre&#118;iew\t&amp; a\\*\n  code\t \n\n~~~\n" << 1;
    QTest::newRow("case") << "```Preview\ncode\n```" << 0;
    QTest::newRow("unknown") << "```unknown\ncode\n```" << 0;
    QTest::newRow("empty-info") << "```\ncode\n```" << 0;
    QTest::newRow("indented") << "    preview\n    code\n" << 0;
    QTest::newRow("empty-code") << "```preview\n```" << 1;
    QTest::newRow("nested") << "> - ~~~preview\n>   nested\n>   ~~~\n\n- ```preview\n  list\n  ```" << 2;
}

void RendererTest::matching()
{
    QFETCH(QString, source); QFETCH(int, count);
    QQmlEngine engine; QQuickWindow window; auto view = create(engine); QVERIFY(view);
    view->setParentItem(window.contentItem()); window.show();
    view->setProperty("markdown", source);
    QTRY_COMPARE(named(view.get(), "customCode").size(), count);
    if (QString::fromLatin1(QTest::currentDataTag()) == "decoded-info") {
        auto *item = named(view.get(), "customCode")[0];
        QCOMPARE(item->property("language").toString(), "preview");
        QCOMPARE(item->property("infoString").toString(), "preview\t& a*");
        QCOMPARE(item->property("codeText").toString(), "  code\t \n\n");
    }
    if (!count) {
        QVERIFY(!named(view.get(), "codeSurface").isEmpty());
        QTRY_VERIFY(view->property("contentHeight").toDouble() > 0);
    }
}

void RendererTest::geometryStyleActionsAndSelection()
{
    QQmlEngine engine; QQuickWindow window; auto view = create(engine, R"(style: MarkdownStyle {
        codeBlock.horizontalPadding: 7; codeBlock.verticalPadding: 5
        codeBlock.backgroundColor: "#112233"
    })"); QVERIFY(view);
    view->setParentItem(window.contentItem()); window.show();
    view->setProperty("markdown", "```preview full info\n  source\t \n\n```");
    QTRY_COMPARE(named(view.get(), "customCode").size(), 1);
    auto *item = named(view.get(), "customCode")[0];
    auto *surface = named(view.get(), "codeSurface")[0];
    auto *action = named(view.get(), "actions")[0];
    QTRY_COMPARE(item->width(), 226.0);
    QTRY_COMPARE(surface->height(), 19 + 113 + 10.0);
    QCOMPARE(item->mapToItem(surface, QPointF{}).x(), 7.0); QCOMPARE(item->mapToItem(surface, QPointF{}).y(), 24.0);
    QCOMPARE(surface->property("color").value<QColor>(), QColor("#112233"));
    view->setWidth(100); QTRY_COMPARE(item->width(), 86.0); QTRY_COMPARE(surface->height(), 72.0);
    auto *style = view->property("style").value<QObject *>(); QVERIFY(style);
    style->setProperty("codeBlockColor", QColor("#abcdef"));
    QTRY_COMPARE(item->property("foreground").value<QColor>(), QColor("#abcdef"));
    QQmlComponent replacement(&engine); replacement.setData("import QMarkdown 1.7\nMarkdownStyle { codeBlockColor: '#334455' }", {});
    std::unique_ptr<QObject> newStyle(replacement.create()); QVERIFY(newStyle);
    view->setProperty("style", QVariant::fromValue(newStyle.get()));
    QTRY_COMPARE(item->property("markdownStyle").value<QObject *>(), newStyle.get());
    QTRY_COMPARE(item->property("foreground").value<QColor>(), QColor("#334455"));
    QSignalSpy codeCopy(view.get(), SIGNAL(codeBlockCopyRequested(QString)));
    QVERIFY(QMetaObject::invokeMethod(action, "copy")); QCOMPARE(codeCopy.size(), 1);
    QCOMPARE(codeCopy[0][0].toString(), "  source\t \n\n");
    QPointer<QQuickItem> custom(item);
    view->setProperty("selectionEnabled", true);
    QTRY_VERIFY(custom.isNull()); QVERIFY(named(view.get(), "customCode").isEmpty());
    QVERIFY(view->property("selectedText").toString().isEmpty());
    QVERIFY(QMetaObject::invokeMethod(view.get(), "selectAll"));
    QCOMPARE(view->property("selectedText").toString(), "  source\t \n\n");
    QSignalSpy copy(view.get(), SIGNAL(copyRequested(QString)));
    QVERIFY(QMetaObject::invokeMethod(view.get(), "requestCopy")); QCOMPARE(copy.size(), 1); QCOMPARE(copy[0][0], codeCopy[0][0]);
    view->setProperty("selectionEnabled", false); QTRY_COMPARE(named(view.get(), "customCode").size(), 1);
    item = named(view.get(), "customCode")[0];
    item->setImplicitHeight(-10); QTRY_COMPARE(item->height(), 0.0);
    item->setImplicitHeight(std::numeric_limits<double>::infinity()); QTRY_COMPARE(item->height(), 0.0);
    item->setImplicitHeight(std::numeric_limits<double>::quiet_NaN()); QTRY_COMPARE(item->height(), 0.0);
    item->setImplicitHeight(12); QTRY_COMPARE(item->height(), 12.0);
    view->setWidth(1); QVERIFY(std::isfinite(view->property("contentHeight").toDouble()));
}

void RendererTest::replacementAndDestruction()
{
    QQmlEngine engine; QQuickWindow window; auto view = create(engine); QVERIFY(view);
    view->setParentItem(window.contentItem()); window.show();
    view->setProperty("markdown", "```preview\none\n```");
    QTRY_COMPARE(named(view.get(), "customCode").size(), 1);
    QPointer<QQuickItem> old = named(view.get(), "customCode")[0];
    QVERIFY(QMetaObject::invokeMethod(view.get(), "useAlternative"));
    QTRY_VERIFY(old.isNull()); QTRY_COMPARE(named(view.get(), "alternativeCode").size(), 1);
    old = named(view.get(), "alternativeCode")[0];
    view->setProperty("markdown", "paragraph\n\n```preview\none\n```");
    QTRY_VERIFY(old.isNull()); QTRY_COMPARE(named(view.get(), "alternativeCode").size(), 1);
    old = named(view.get(), "alternativeCode")[0];
    QVERIFY(QMetaObject::invokeMethod(view.get(), "disableRenderers"));
    QTRY_VERIFY(old.isNull()); QVERIFY(named(view.get(), "alternativeCode").isEmpty());
    QVERIFY(QMetaObject::invokeMethod(view.get(), "useInherited")); QVERIFY(named(view.get(), "customCode").isEmpty());
    QVERIFY(QMetaObject::invokeMethod(view.get(), "useAlternative")); QTRY_COMPARE(named(view.get(), "alternativeCode").size(), 1);
    old = named(view.get(), "alternativeCode")[0]; view.reset(); QVERIFY(old.isNull());
    QQmlComponent defaults(&engine); defaults.setData("import QMarkdown 1.7\nMarkdownView {}", {});
    std::unique_ptr<QObject> empty(defaults.create()); QVERIFY(empty);
    QCOMPARE(empty->property("fencedBlockRenderers").value<QJSValue>().property("preview").isUndefined(), true);
}

void RendererTest::creationReplacement()
{
    QQmlEngine engine; QQuickWindow window;
    auto view = create(engine, R"QML(
    property Component replacing: Component {
        Item {
            objectName: "obsoleteRenderer"
            required property string codeText
            required property string infoString
            required property string language
            required property var markdownStyle
            implicitHeight: 20
            Component.onCompleted: view.useAlternative()
        }
    }
    Component.onCompleted: fencedBlockRenderers = {preview: replacing}
)QML");
    QVERIFY(view); view->setParentItem(window.contentItem()); window.show();
    view->setProperty("markdown", "```preview\none\n```");
    QTRY_COMPARE(named(view.get(), "alternativeCode").size(), 1);
    QVERIFY(named(view.get(), "obsoleteRenderer").isEmpty());
    // A host can replace the document while constructing the renderer.
    auto documentView = create(engine, R"QML(
    property Component replacing: Component {
        Item {
            objectName: "obsoleteRenderer"
            required property string codeText
            required property string infoString
            required property string language
            required property var markdownStyle
            implicitHeight: 20
            Component.onCompleted: view.markdown = "replacement paragraph"
        }
    }
    Component.onCompleted: fencedBlockRenderers = {preview: replacing}
)QML");
    QVERIFY(documentView); documentView->setParentItem(window.contentItem());
    documentView->setProperty("markdown", "```preview\none\n```");
    QTRY_COMPARE(documentView->property("markdown").toString(), "replacement paragraph");
    QTRY_VERIFY(named(documentView.get(), "obsoleteRenderer").isEmpty());
}

void RendererTest::erroredComponent()
{
    QQmlEngine engine; QQuickWindow window; auto view = create(engine); QVERIFY(view);
    view->setParentItem(window.contentItem()); window.show();
    view->setProperty("markdown", "```preview\nsource\n```");
    QTRY_COMPARE(named(view.get(), "customCode").size(), 1);
    QPointer<QQuickItem> old = named(view.get(), "customCode")[0];
    QQmlComponent error(&engine); error.setData("import QtQuick\nUnknownRenderer {}", {});
    QVERIFY(error.isError());
    auto map = engine.newObject(); map.setProperty("preview", engine.newQObject(&error));
    QVERIFY(view->setProperty("fencedBlockRenderers", QVariant::fromValue(map)));
    QTRY_VERIFY(old.isNull()); QVERIFY(named(view.get(), "customCode").isEmpty());
    QTRY_VERIFY(view->property("contentHeight").toDouble() > 19);
}

void RendererTest::fallback_data()
{
    QTest::addColumn<QByteArray>("entry");
    QTest::newRow("null") << QByteArray("fencedBlockRenderers: ({ preview: null })");
    QTest::newRow("not-component") << QByteArray("fencedBlockRenderers: ({ preview: 42 })");
    QTest::newRow("non-item") << QByteArray(R"(property Component bad: Component { QtObject {
        required property string codeText; required property string infoString; required property string language; required property var markdownStyle
    } }
    fencedBlockRenderers: ({preview: bad}))");
    QTest::newRow("missing-property") << QByteArray(R"(property Component bad: Component { Item {
        required property string codeText; required property string infoString; required property string language
    } }
    fencedBlockRenderers: ({preview: bad}))");
    QTest::newRow("wrong-type") << QByteArray(R"(property Component bad: Component { Item {
        required property var codeText; required property string infoString; required property string language; required property var markdownStyle
    } }
    fencedBlockRenderers: ({preview: bad}))");
    QTest::newRow("wrong-style-type") << QByteArray(R"(property Component bad: Component { Item {
        required property string codeText; required property string infoString; required property string language; required property string markdownStyle
    } }
    fencedBlockRenderers: ({preview: bad}))");
    QTest::newRow("creation-failure") << QByteArray(R"(property Component bad: Component { Item {
        required property string unavailable
        required property string codeText; required property string infoString; required property string language; required property var markdownStyle
    } }
    fencedBlockRenderers: ({preview: bad}))");
}

void RendererTest::fallback()
{
    QFETCH(QByteArray, entry);
    QQmlEngine engine; QQuickWindow window;
    // Replace the normal map declaration in the helper via an override binding.
    auto view = create(engine, entry.replace("fencedBlockRenderers:", "Component.onCompleted: fencedBlockRenderers =")); QVERIFY(view);
    view->setParentItem(window.contentItem()); window.show();
    view->setProperty("markdown", "```preview\nsource\n```");
    QTRY_VERIFY(view->property("contentHeight").toDouble() > 19);
    QVERIFY(named(view.get(), "customCode").isEmpty());
    QSignalSpy copy(view.get(), SIGNAL(codeBlockCopyRequested(QString)));
    QTRY_COMPARE(named(view.get(), "actions").size(), 1);
    QVERIFY(QMetaObject::invokeMethod(named(view.get(), "actions")[0], "copy"));
    QCOMPARE(copy.size(), 1); QCOMPARE(copy[0][0].toString(), "source\n");
}
QTEST_MAIN(RendererTest)
#include "tst_renderers.moc"
