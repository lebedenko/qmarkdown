#include "private/formattedtext.h"
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QPainter>
#include <QElapsedTimer>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
class SyntaxViewTest : public QObject {
    Q_OBJECT
    static std::unique_ptr<QQuickItem> create(QQmlEngine &engine);
    static QQuickItem *named(QQuickItem *root, const QString &name) {
        for (auto *child : root->childItems()) {
            if (child->objectName() == name && child->isVisible()) return child;
            if (auto *found = named(child, name)) return found;
        }
        return nullptr;
    }
    static FormattedText *text(QQuickItem *root) {
        for (auto *child : root->childItems()) {
            if (auto *formatted = qobject_cast<FormattedText *>(child)) return formatted;
            if (auto *found = text(child)) return found;
        }
        return nullptr;
    }
private slots:
    void publicDefaults() {
        QQmlEngine engine; QQmlComponent c(&engine);
        c.setData("import QtQuick; import QMarkdown 1.11; MarkdownView {}", {});
        std::unique_ptr<QObject> view(c.create()); QVERIFY2(view, qPrintable(c.errorString()));
        QCOMPARE(view->property("syntaxHighlightingEnabled").toBool(), false);
        QCOMPARE(view->property("syntaxHighlightingAvailable").toBool(), bool(QMARKDOWN_EXPECT_SYNTAX));
        QVERIFY(view->property("syntaxTheme").value<QObject *>());
    }
    void colorsThemeAndSource() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(240, 240); window.show();
        const bool available = view->property("syntaxHighlightingAvailable").toBool();
        if (!available) { QCoreApplication::processEvents(); QVERIFY(!text(view.get())); return; }
        QTRY_VERIFY(text(view.get()));
        auto *formatted = text(view.get());
        QTRY_VERIFY(formatted->logicalHeight() > 0);
        QCOMPARE(formatted->text(), "const int count = 42;");
        QVERIFY(!formatted->foregroundRanges().isEmpty());
        QImage image(200, 80, QImage::Format_ARGB32_Premultiplied); image.fill(Qt::white);
        QPainter painter(&image); formatted->paint(&painter); painter.end();
        int magenta = 0;
        for (int y = 0; y < image.height(); ++y) for (int x = 0; x < image.width(); ++x) {
            auto c = image.pixelColor(x, y); if (c.red() > 180 && c.blue() > 180 && c.green() < 120) ++magenta;
        }
        QVERIFY(magenta > 5);
        QVERIFY(QMetaObject::invokeMethod(view.get(), "magentaTheme"));
        QTRY_VERIFY(formatted->foregroundRanges().last().toMap().value("color").value<QColor>() == QColor(Qt::magenta));
        QVERIFY(QMetaObject::invokeMethod(view.get(), "resetTheme"));
        QTRY_VERIFY(formatted->foregroundRanges().first().toMap().value("color").value<QColor>() != QColor(Qt::magenta));
        QObject *fallback = view->property("syntaxTheme").value<QObject *>();
        QVERIFY(QMetaObject::invokeMethod(view.get(), "temporaryTheme"));
        QTRY_COMPARE(view->property("syntaxTheme").value<QObject *>(), fallback);
        view->setProperty("selectionEnabled", true); QVERIFY(QMetaObject::invokeMethod(view.get(), "selectAll"));
        QVERIFY(view->property("selectedText").toString().contains("const int count = 42;"));
        view->setProperty("searchActive", true); view->setProperty("searchQuery", "count");
        QTRY_COMPARE(view->property("searchResultCount").toInt(), 1);
        QTRY_VERIFY(text(view.get()) && !text(view.get())->foregroundRanges().isEmpty());
        view->setProperty("markdown", "```unknown\nconst int other = 7;\n```");
        QTRY_VERIFY(text(view.get()) && text(view.get())->foregroundRanges().isEmpty());
    }
    void layoutFallbacksAndExactCopy() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 300); window.show();
        const QString code = "\tconst char *s = \"😀é\";  \n\n";
        view->setProperty("markdown", "> ```cpp extra\n> " + QString(code).replace("\n", "\n> ") + "```\n");
        if (view->property("syntaxHighlightingAvailable").toBool()) {
            QTRY_VERIFY(text(view.get()) && !text(view.get())->foregroundRanges().isEmpty());
            auto *formatted = text(view.get());
            QTRY_VERIFY(formatted->logicalHeight() > 0);
            const auto height = view->property("contentHeight").toReal();
            formatted->setHorizontalAlignment(Qt::AlignRight);
            QTRY_VERIFY(formatted->logicalHeight() > 0);
            view->setProperty("syntaxHighlightingEnabled", false);
            QTRY_VERIFY(!text(view.get()));
            QCOMPARE(view->property("contentHeight").toReal(), height);
        }
        view->setProperty("selectionEnabled", true);
        QMetaObject::invokeMethod(view.get(), "selectAll");
        QVERIFY(view->property("selectedText").toString().contains(code));
        QSignalSpy copy(view.get(), SIGNAL(copyRequested(QString)));
        QMetaObject::invokeMethod(view.get(), "requestCopy");
        QCOMPARE(copy.size(), 1); QVERIFY(copy.first().first().toString().contains(code));
        view->setProperty("markdown", "```cpp\n```\n");
        QTRY_VERIFY(view->property("contentHeight").toReal() > 0);
        view->setProperty("markdown", "```\nint n = 42;\n```\n");
        QCoreApplication::processEvents();
        if (text(view.get())) QVERIFY(text(view.get())->foregroundRanges().isEmpty());
    }
    void advisoryPerformance() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 300); window.show();
        if (!view->property("syntaxHighlightingAvailable").toBool()) return;
        QString document;
        for (int i = 0; i < 40; ++i) document += QString("```cpp\nconst int value%1 = %1;\n```\n\n").arg(i);
        QElapsedTimer timer; timer.start();
        view->setProperty("markdown", document);
        QTRY_VERIFY(text(view.get()) && !text(view.get())->foregroundRanges().isEmpty());
        const auto dozens = timer.elapsed();
        timer.restart();
        view->setProperty("markdown", "```cpp\n" + QString("int n = 42;\n").repeated(4000) + "```\n");
        // Source must be present immediately, irrespective of resource fallback.
        QTRY_VERIFY(view->property("contentHeight").toReal() > 0);
        const auto large = timer.elapsed();
        timer.restart();
        for (int i = 0; i < 100; ++i) view->setProperty("markdown", QString("```js\nconst n = %1;\n```\n").arg(i));
        QTRY_VERIFY(text(view.get()) && !text(view.get())->foregroundRanges().isEmpty());
        const auto replacements = timer.elapsed();
        timer.restart();
        for (int i = 0; i < 100; ++i) view->property("syntaxTheme").value<QObject *>()->setProperty("preset", i % 2);
        qInfo() << "Advisory milliseconds: 40 blocks first result" << dozens << "large block initial layout" << large
                << "100 replacements" << replacements << "100 palette assignments" << timer.elapsed();
    }
    void customPrecedenceAndFallback() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QVERIFY(QMetaObject::invokeMethod(view.get(), "useCustom"));
        QTRY_VERIFY(named(view.get(), "customVisual"));
        QVERIFY(!text(view.get()));
        view->setProperty("searchActive", true);
        if (view->property("syntaxHighlightingAvailable").toBool()) QTRY_VERIFY(text(view.get()) && !text(view.get())->foregroundRanges().isEmpty());
        view->setProperty("searchActive", false);
        QTRY_VERIFY(named(view.get(), "customVisual"));
        QVERIFY(QMetaObject::invokeMethod(view.get(), "clearCustom"));
        if (view->property("syntaxHighlightingAvailable").toBool()) QTRY_VERIFY(text(view.get()));
        view->setProperty("markdown", "    const int count = 42;\n");
        QTRY_VERIFY(!text(view.get()));
    }
};
std::unique_ptr<QQuickItem> SyntaxViewTest::create(QQmlEngine &engine) {
        QQmlComponent component(&engine);
        component.setData(R"(
import QtQuick
import QMarkdown 1.11
MarkdownView {
    id: root
    width: 180
    syntaxHighlightingEnabled: true
    syntaxTheme: SyntaxTheme { keywordColor: "#ff00ff"; typeColor: "#ff00ff" }
    markdown: "```CPP extra fields\nconst int count = 42;\n```"
    property Component custom: Component {
        Item { objectName: "customVisual"; required property string codeText; required property string infoString; required property string language; required property var markdownStyle; implicitHeight: 37 }
    }
    function useCustom() { fencedBlockRenderers = { CPP: custom } }
    function clearCustom() { fencedBlockRenderers = {} }
    function resetTheme() { syntaxTheme = null }
    function darkTheme() { syntaxTheme.preset = SyntaxTheme.Dark }
    function magentaTheme() { syntaxTheme.numberColor = "#ff00ff" }
    function temporaryTheme() { let t = Qt.createQmlObject('import QMarkdown 1.11; SyntaxTheme { preset: SyntaxTheme.Dark }', root); syntaxTheme = t; t.destroy() }
}
)", {});
        auto result = std::unique_ptr<QQuickItem>(qobject_cast<QQuickItem *>(component.create()));
        if (!result) qWarning() << component.errorString();
        return result;
    }

QTEST_MAIN(SyntaxViewTest)
#include "tst_syntaxview.moc"
