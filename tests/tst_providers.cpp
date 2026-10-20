#include "private/formattedtext.h"
#include "syntaxhighlightingprovider.h"
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
class ProviderTest : public QObject {
    Q_OBJECT
    static QList<FormattedText *> painted(QQuickItem *root) {
        QList<FormattedText *> result;
        for (auto *child : root->childItems()) {
            if (auto *text = qobject_cast<FormattedText *>(child)) result.append(text);
            result.append(painted(child));
        }
        return result;
    }
    static QVariantList colors(QQuickItem *root) {
        QVariantList result;
        for (auto *text : painted(root)) result.append(text->foregroundRanges());
        return result;
    }
    static std::unique_ptr<QQuickItem> create(QQmlEngine &engine, bool custom = false);
private slots:
    void revisionsAndDefaults();
    void asynchronousQmlAndProjection();
    void customAndFootnoteFallback();
    void retainedCancellationAndDestruction();
};
std::unique_ptr<QQuickItem> ProviderTest::create(QQmlEngine &engine, bool custom) {
    QQmlComponent component(&engine);
    component.setData(R"(
import QtQuick
import QMarkdown 1.17
Item {
    id: root
    width: 420; height: 500
    property int calls: 0
    property var pending: []
    property alias view: preview
    property alias provider: host
    property bool customInitially: false
    property bool immediate: false
    SyntaxHighlightingProvider {
        id: host
        onHighlightRequested: request => {
            root.calls++
            root.pending = root.pending.concat([request])
            if (root.immediate) request.complete([{start:0, length:request.source.length, role:SyntaxHighlightingProvider.Keyword}])
        }
    }
    MarkdownView {
        id: preview
        width: parent.width
        selectionEnabled: !root.customInitially
        footnotesEnabled: true
        fencedBlockRenderers: root.customInitially ? ({HostOnly:root.custom}) : ({})
        syntaxHighlightingEnabled: true
        syntaxHighlightingProvider: host
        syntaxTheme: SyntaxTheme { keywordColor: "magenta" }
        markdown: "```HostOnly extra\nhello 😀\n```"
    }
    property Component custom: Component {
        Item { objectName: "hostVisual"; required property string codeText; required property string infoString; required property string language; required property var markdownStyle; implicitHeight: 40 }
    }
    function completeFirst() { pending[0].complete([{start:0,length:5,role:SyntaxHighlightingProvider.Keyword}]) }
    function completeLast() { let r = pending[pending.length-1]; r.complete([{start:0,length:r.source.length,role:SyntaxHighlightingProvider.Keyword}]) }
    function declineLast() { pending[pending.length-1].decline() }
    function invalidate() { host.invalidate() }
    function customOn() { preview.fencedBlockRenderers = {HostOnly:custom} }
    function replaceProvider() { preview.syntaxHighlightingProvider = null }
    function destroyView() { preview.destroy() }
    function collect() { gc() }
}
)", {});
    auto object = std::unique_ptr<QQuickItem>(qobject_cast<QQuickItem *>(component.createWithInitialProperties({{"customInitially", custom}})));
    if (!object) qWarning() << component.errorString();
    return object;
}
void ProviderTest::revisionsAndDefaults() {
    for (const auto version : {"1.16", "1.17"}) {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(QByteArray("import QMarkdown ") + version + "\nSyntaxHighlightingProvider {}", {});
        QCOMPARE(component.isReady(), QByteArray(version) == "1.17");
    }
    QQmlEngine engine; QQmlComponent component(&engine);
    component.setData("import QMarkdown 1.17\nMarkdownView {}", {});
    std::unique_ptr<QObject> view(component.create()); QVERIFY2(view, qPrintable(component.errorString()));
    QVERIFY(!view->property("syntaxHighlightingProvider").value<QObject *>());
    QVERIFY(!view->property("syntaxHighlightingEnabled").toBool());
    QCOMPARE(int(SyntaxHighlightingProvider::Error), 11);
}
void ProviderTest::asynchronousQmlAndProjection() {
    QQmlEngine engine; auto root = create(engine); QVERIFY(root);
    auto *view = qobject_cast<QQuickItem *>(root->property("view").value<QObject *>()); QVERIFY(view);
    QQuickWindow window; root->setParentItem(window.contentItem()); window.resize(440, 520); window.show();
    QTRY_COMPARE(root->property("calls").toInt(), 1);
    auto pending = root->property("pending").value<QJSValue>();
    QCOMPARE(pending.property(0).property("source").toString(), QString::fromUtf8("hello 😀\n"));
    QCOMPARE(pending.property(0).property("language").toString(), QString("HostOnly"));
    QVERIFY(colors(view).isEmpty());
    QMetaObject::invokeMethod(root.get(), "completeFirst");
    QTRY_COMPARE(colors(view).size(), 1);
    QCOMPARE(colors(view)[0].toMap()["color"].value<QColor>(), QColor(Qt::magenta));
    const auto height = view->property("contentHeight").toReal();
    auto *theme = view->property("syntaxTheme").value<QObject *>(); QVERIFY(theme);
    theme->setProperty("keywordColor", QColor(Qt::cyan));
    QTRY_COMPARE(colors(view)[0].toMap()["color"].value<QColor>(), QColor(Qt::cyan));
    QCOMPARE(root->property("calls").toInt(), 1);
    QCOMPARE(view->property("contentHeight").toReal(), height);
    QMetaObject::invokeMethod(view, "selectAll");
    const auto copy = view->property("selectedText").toString();
    QVERIFY(copy.contains(QString::fromUtf8("hello 😀\n")));
    view->setProperty("searchActive", true); view->setProperty("searchQuery", "hello");
    QTRY_COMPARE(view->property("searchResultCount").toInt(), 1);
    QCOMPARE(view->property("selectedText").toString(), copy);
    QMetaObject::invokeMethod(root.get(), "invalidate");
    QTRY_COMPARE(root->property("calls").toInt(), 2);
    QMetaObject::invokeMethod(root.get(), "completeLast"); QTRY_COMPARE(colors(view).size(), 1);
    QTRY_VERIFY(!window.grabWindow().isNull());
    QVERIFY(window.grabWindow().save("/tmp/063-providers.png"));
}
void ProviderTest::customAndFootnoteFallback() {
    QQmlEngine engine; auto root = create(engine, true); QVERIFY(root);
    auto *view = qobject_cast<QQuickItem *>(root->property("view").value<QObject *>()); QVERIFY(view);
    root->setProperty("immediate", true);
    QTest::qWait(20);
    QCOMPARE(root->property("calls").toInt(), 0);
    view->setProperty("searchActive", true);
    QTRY_COMPARE(root->property("calls").toInt(), 1); QTRY_VERIFY(!colors(view).isEmpty());
    view->setProperty("searchActive", false); QTRY_VERIFY(colors(view).isEmpty());
    view->setProperty("fencedBlockRenderers", QVariantMap{});
    view->setProperty("markdown", "text[^n]\n\n[^n]: ```HostOnly\n    note source\n    ```\n");
    QTRY_COMPARE(root->property("calls").toInt(), 2); QTRY_VERIFY(!colors(view).isEmpty());
    view->setProperty("selectionEnabled", true);
    QMetaObject::invokeMethod(view, "selectAll");
    QVERIFY(view->property("selectedText").toString().contains("1. note source"));
    view->setProperty("markdown", "    indented\n\n```\nunlabelled\n```\n");
    QTest::qWait(20); QCOMPARE(root->property("calls").toInt(), 2); QVERIFY(colors(view).isEmpty());
    view->setProperty("syntaxHighlightingEnabled", false);
    view->setProperty("markdown", "```HostOnly\ndisabled\n```\n");
    QTest::qWait(20); QCOMPARE(root->property("calls").toInt(), 2);
}
void ProviderTest::retainedCancellationAndDestruction() {
    QQmlEngine engine; auto root = create(engine); QVERIFY(root);
    auto *view = qobject_cast<QQuickItem *>(root->property("view").value<QObject *>()); QVERIFY(view);
    QTRY_COMPARE(root->property("calls").toInt(), 1);
    auto pending = root->property("pending").value<QJSValue>();
    const auto retained = pending.property(0);
    view->setProperty("markdown", "```HostOnly\nreplacement\n```\n");
    QTRY_COMPARE(root->property("calls").toInt(), 2);
    QVERIFY(retained.property("cancelled").toBool());
    engine.collectGarbage(); QVERIFY(retained.toQObject());
    auto *old = qobject_cast<SyntaxHighlightingRequest *>(retained.toQObject()); QVERIFY(old);
    old->complete(engine.evaluate("[{start:0,length:1,role:0}]")); QVERIFY(colors(view).isEmpty());
    auto last = root->property("pending").value<QJSValue>().property(1);
    QMetaObject::invokeMethod(root.get(), "replaceProvider");
    QTRY_VERIFY(last.property("cancelled").toBool());
    auto *provider = root->property("provider").value<QObject *>(); QVERIFY(provider);
    view->setProperty("syntaxHighlightingProvider", QVariant::fromValue(provider));
    QTRY_COMPARE(root->property("calls").toInt(), 3);
    last = root->property("pending").value<QJSValue>().property(2);
    delete provider;
    QTRY_VERIFY(last.property("cancelled").toBool());
    QVERIFY(!view->property("syntaxHighlightingProvider").value<QObject *>());
    auto *external = new SyntaxHighlightingProvider;
    QJSValue retainedAfterView;
    connect(external, &SyntaxHighlightingProvider::highlightRequested, external, [&](auto *r) { retainedAfterView = engine.newQObject(r); });
    view->setProperty("syntaxHighlightingProvider", QVariant::fromValue(external));
    QTRY_VERIFY(retainedAfterView.isQObject());
    delete view;
    QVERIFY(retainedAfterView.property("cancelled").toBool());
    delete external; engine.collectGarbage();
}
QTEST_MAIN(ProviderTest)
#include "tst_providers.moc"
