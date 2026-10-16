#include "private/formattedtext.h"
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QFontMetricsF>
#include <QMetaProperty>
#include <QWheelEvent>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
class CodeBlocksTest : public QObject {
    Q_OBJECT
    static QQuickItem *named(QQuickItem *root, const QString &name) {
        if (root->objectName() == name && root->isVisible()) return root;
        for (auto *child : root->childItems()) if (auto *found = named(child, name)) return found;
        return nullptr;
    }
    static FormattedText *native(QQuickItem *root) {
        if (auto *text = qobject_cast<FormattedText *>(root)) return text;
        for (auto *child : root->childItems()) if (auto *found = native(child)) return found;
        return nullptr;
    }
    static std::unique_ptr<QQuickItem> create(QQmlEngine &engine, bool legacy = false) {
        QQmlComponent component(&engine);
        const QByteArray header = legacy ? "" : "required property string language; required property string languageId; required property string languageName; required property string infoString; required property bool fenced;";
        component.setData(R"QML(import QtQuick; import QMarkdown; MarkdownView { width: 240;
            property Component hostBar: Component { Item {
                required property int orientation; required property real position; required property real size;
                required property bool active; required property var requestPosition;
                objectName: orientation === Qt.Vertical ? 'verticalBar' : 'horizontalBar'
                function move(value: real) { requestPosition(value) }
            } }
            property Component invalidBar: Component { Rectangle { objectName: 'invalidBar' } }
            function useBars() { codeBlockScrollBar = hostBar }
            function invalidBars() { codeBlockScrollBar = invalidBar }
            codeBlockActions: Item { objectName: 'header'; required property string codeText; required property var requestCopy; )QML" + header + " implicitHeight: 24; function copy() { requestCopy() } } }", {});
        auto result = std::unique_ptr<QQuickItem>(qobject_cast<QQuickItem *>(component.create()));
        if (!result) qWarning().noquote() << component.errorString();
        return result;
    }
private slots:
    void defaultsAndLegacyHeader() {
        QQmlEngine engine; auto view = create(engine, true); QVERIFY(view);
        QCOMPARE(view->property("codeBlockWrapEnabled").toBool(), true);
        QCOMPARE(view->property("codeBlockLineNumbersVisible").toBool(), false);
        QCOMPARE(view->property("codeBlockHeightLimitEnabled").toBool(), false);
        QCOMPARE(view->property("codeBlockViewportHeight").toReal(), 0.0);
        QCOMPARE(view->property("codeBlockHeightRatio").toReal(), 0.5);
        QCOMPARE(view->property("codeBlockMinimumLines").toInt(), 8);
        QCOMPARE(view->property("codeBlockMaximumLines").toInt(), 30);
        view->setProperty("markdown", "```CPP extra\nx\n```");
        QTRY_VERIFY(named(view.get(), "header"));
        QCOMPARE(named(view.get(), "header")->property("codeText").toString(), "x\n");
    }
    void metadataAndCopy_data() {
        QTest::addColumn<QString>("token"); QTest::addColumn<QString>("id"); QTest::addColumn<QString>("name");
        QTest::newRow("cpp") << "CPP" << "cpp" << "C++";
        QTest::newRow("python") << "Py" << "python" << "Python";
        QTest::newRow("shell") << "shell" << "bash" << "Bash";
        QTest::newRow("unknown") << "OddLang" << "OddLang" << "OddLang";
        QTest::newRow("empty") << "" << "" << "";
    }
    void metadataAndCopy() {
        QFETCH(QString, token); QFETCH(QString, id); QFETCH(QString, name);
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        const QString source = "\t😀é  \n\nlast\n";
        view->setProperty("markdown", "```" + token + "\n" + source + "```");
        QTRY_VERIFY(named(view.get(), "header"));
        auto *header = named(view.get(), "header");
        QCOMPARE(header->property("language").toString(), token);
        QCOMPARE(header->property("languageId").toString(), id);
        QCOMPARE(header->property("languageName").toString(), name);
        QCOMPARE(header->property("fenced").toBool(), true);
        QSignalSpy copy(view.get(), SIGNAL(codeBlockCopyRequested(QString)));
        QVERIFY(QMetaObject::invokeMethod(header, "copy"));
        QCOMPARE(copy.size(), 1); QCOMPARE(copy[0][0].toString(), source);
    }
    void lineGeometry_data() {
        QTest::addColumn<int>("count"); QTest::addColumn<bool>("wrap"); QTest::addColumn<bool>("highlight");
        for (int count : {1, 9, 10, 99, 100}) for (bool wrap : {false, true}) for (bool highlight : {false, true})
            QTest::newRow(qPrintable(QString::number(count) + (wrap ? "-wrap" : "-nowrap") + (highlight ? "-highlight" : "-plain"))) << count << wrap << highlight;
    }
    void lineGeometry() {
        QFETCH(int, count); QFETCH(bool, wrap); QFETCH(bool, highlight);
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.show();
        view->setProperty("codeBlockLineNumbersVisible", true);
        view->setProperty("codeBlockWrapEnabled", wrap);
        view->setProperty("syntaxHighlightingEnabled", highlight);
        QString source;
        for (int i = 0; i < count; ++i) source += (i % 2 ? QString() : QString(60, 'x')) + "\n";
        view->setProperty("markdown", "```cpp\n" + source + "```");
        QTRY_VERIFY(native(view.get()));
        auto *text = native(view.get());
        QTRY_COMPARE(text->sourceLines().size(), count);
        int number = 1;
        for (const auto &entry : text->sourceLines()) {
            const auto line = entry.toMap();
            QCOMPARE(line.value("number").toInt(), number++);
            auto row = text->layout()->lineForTextPosition(line.value("start").toInt());
            QVERIFY(row.isValid());
            QCOMPARE(line.value("baseline").toReal(), row.y() + row.ascent());
        }
        auto *style = view->property("style").value<QObject *>();
        auto *codeStyle = style->property("codeBlock").value<QObject *>(); QVERIFY(codeStyle);
        style->setProperty("codeBlockColor", QColor(20, 30, 40, 100));
        QCOMPARE(codeStyle->property("lineNumberColor").value<QColor>(), QColor(20, 30, 40, 60));
        QVERIFY(codeStyle->setProperty("lineNumberColor", QColor(Qt::red)));
        QCOMPARE(codeStyle->property("lineNumberColor").value<QColor>(), QColor(Qt::red));
        const auto property = codeStyle->metaObject()->property(codeStyle->metaObject()->indexOfProperty("lineNumberColor"));
        QVERIFY(property.reset(codeStyle));
        QCOMPARE(codeStyle->property("lineNumberColor").value<QColor>(), QColor(20, 30, 40, 60));
    }
    void scrollbarsAndShortBlocks() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(240, 600); window.show();
        view->setProperty("codeBlockWrapEnabled", false);
        view->setProperty("codeBlockLineNumbersVisible", true);
        view->setProperty("codeBlockHeightLimitEnabled", true);
        view->setProperty("codeBlockMinimumLines", 2);
        view->setProperty("codeBlockMaximumLines", 4);
        QString source; for (int i = 0; i < 20; ++i) source += QString(80, 'x') + "\n";
        view->setProperty("markdown", "```\n" + source + "```");
        QVERIFY(QMetaObject::invokeMethod(view.get(), "useBars"));
        QTRY_VERIFY(named(view.get(), "verticalBar")); QTRY_VERIFY(named(view.get(), "horizontalBar"));
        auto *viewport = named(view.get(), "codeViewport"); QVERIFY(viewport);
        const auto bar = named(view.get(), "verticalBar");
        QVERIFY(bar->property("size").toReal() < 1);
        QVERIFY(QMetaObject::invokeMethod(bar, "move", Q_ARG(double, 1.0)));
        QTRY_VERIFY(viewport->property("contentY").toReal() > 0);
        QTRY_COMPARE(viewport->property("contentY").toReal(), viewport->property("contentHeight").toReal() - viewport->height());
        view->setProperty("codeBlockWrapEnabled", true);
        QTRY_COMPARE(viewport->property("contentX").toReal(), 0.0);
        QVERIFY(QMetaObject::invokeMethod(view.get(), "invalidBars"));
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(!named(view.get(), "invalidBar"));
        view->setProperty("markdown", "```\nshort\n```");
        QTRY_VERIFY(native(view.get())); QTRY_VERIFY(native(view.get())->logicalHeight() > 0);
        viewport = named(view.get(), "codeViewport");
        QCOMPARE(viewport->height(), native(view.get())->logicalHeight());
    }
    void wrappingHeightGutterAndFind() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(240, 600); window.show();
        QString source;
        for (int i = 0; i < 100; ++i) source += QString::number(i) + "\t" + QString(80, 'x') + (i == 90 ? " needle" : "") + "\n";
        view->setProperty("markdown", "```cpp\n" + source + "```");
        view->setProperty("codeBlockWrapEnabled", false);
        view->setProperty("codeBlockLineNumbersVisible", true);
        view->setProperty("codeBlockHeightLimitEnabled", true);
        view->setProperty("codeBlockViewportHeight", 500);
        view->setProperty("codeBlockMinimumLines", 1);
        QTRY_VERIFY(native(view.get())); auto *text = native(view.get());
        QTRY_VERIFY(text->logicalWidth() > 240);
        QTRY_COMPARE(text->sourceLines().size(), 100);
        auto *viewport = named(view.get(), "codeViewport"); QVERIFY(viewport);
        QTRY_COMPARE(viewport->height(), 250.0);
        QCOMPARE(viewport->property("contentY").toReal(), 0.0);
        const auto natural = text->logicalHeight();
        view->setProperty("codeBlockWrapEnabled", true);
        QTRY_VERIFY(text->logicalHeight() > natural);
        QCOMPARE(text->sourceLines().size(), 100);
        QCOMPARE(viewport->property("contentX").toReal(), 0.0);
        view->setProperty("searchActive", true); view->setProperty("searchQuery", "needle");
        QTRY_COMPARE(view->property("searchResultCount").toInt(), 1);
        QTRY_VERIFY(viewport->property("contentY").toReal() > 0);
        viewport->setProperty("contentY", 0.0);
        QCoreApplication::processEvents();
        QCOMPARE(viewport->property("contentY").toReal(), 0.0);
        view->setProperty("selectionEnabled", true); QMetaObject::invokeMethod(view.get(), "selectAll");
        QCOMPARE(view->property("selectedText").toString(), source);
        view->setProperty("markdown", "```\n\n```");
        QTRY_VERIFY(native(view.get())); QTRY_COMPARE(native(view.get())->sourceLines().size(), 1);
        QCOMPARE(named(view.get(), "codeViewport")->property("contentY").toReal(), 0.0);
    }
};
QTEST_MAIN(CodeBlocksTest)
#include "tst_codeblocks.moc"
