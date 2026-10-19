#include "private/formattedtext.h"
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QFontMetricsF>
#include <QMetaProperty>
#include <QWheelEvent>
#include <QPointingDevice>
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
    void gutterTracksFontChanges() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.show();
        view->setProperty("codeBlockLineNumbersVisible", true);
        view->setProperty("markdown", "```\n" + QString("x\n").repeated(10) + "```");
        auto *style = view->property("style").value<QObject *>(); QVERIFY(style);
        auto font = style->property("codeBlockFont").value<QFont>();
        font.setPixelSize(12); style->setProperty("codeBlockFont", font);
        auto *surface = named(view.get(), "codeSurface"); QVERIFY(surface);
        QTRY_VERIFY(surface->property("gutterWidth").toReal() > 16);
        const auto before = surface->property("gutterWidth").toReal();
        font.setPixelSize(28); style->setProperty("codeBlockFont", font);
        QTRY_VERIFY(surface->property("gutterWidth").toReal() > before);
    }
    void wheelBoundaryHandoff_data() {
        QTest::addColumn<bool>("pixel");
        QTest::addColumn<bool>("parentHandler");
        QTest::newRow("wheel") << false << false;
        QTest::newRow("wheel-parent-handler") << false << true;
        QTest::newRow("trackpad") << true << false;
        QTest::newRow("trackpad-parent-handler") << true << true;
    }
    void wheelBoundaryHandoff() {
        QFETCH(bool, pixel);
        QFETCH(bool, parentHandler);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(
import QtQuick
import QMarkdown
Flickable {
    id: outer
    property bool parentHandler: false
    property int observedWheels: 0
    WheelHandler {
        enabled: outer.parentHandler
        blocking: false
        onWheel: function(event) { outer.observedWheels++; event.accepted = false }
    }
    width: 240; height: 300; contentWidth: width; contentHeight: 1500
    MarkdownView {
        objectName: "reader"; width: 240; y: 100
        codeBlockWrapEnabled: false
        codeBlockHeightLimitEnabled: true; codeBlockMinimumLines: 1; codeBlockMaximumLines: 2
        markdown: "```\n" + ("long source line ".repeat(20) + "\n").repeat(40) + "```"
    }
}
)", {});
        std::unique_ptr<QQuickItem> outer(qobject_cast<QQuickItem *>(component.create()));
        QVERIFY2(outer, qPrintable(component.errorString()));
        outer->setProperty("parentHandler", parentHandler);
        QQuickWindow window; window.resize(240, 300); outer->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        auto *viewport = named(outer.get(), "codeViewport"); QVERIFY(viewport);
        QTRY_VERIFY(viewport->height() > 0); QCoreApplication::processEvents();
        auto point = viewport->mapToScene(QPointF(20, viewport->height() / 2));
        QPointingDevice trackpad("test touchpad", 42, QInputDevice::DeviceType::TouchPad,
            QPointingDevice::PointerType::Finger, QInputDevice::Capability::Position | QInputDevice::Capability::Scroll, 1, 0);
        if (pixel) {
            QWheelEvent begin(point, window.mapToGlobal(point.toPoint()), QPoint(0,-40), QPoint(0,-120),
                Qt::NoButton, Qt::NoModifier, Qt::ScrollBegin, false, Qt::MouseEventSynthesizedBySystem, &trackpad);
            begin.setTimestamp(100);
            QCoreApplication::sendEvent(&window, &begin);
        }
        ulong timestamp = 100;
        auto wheel = [&](int direction = -1, bool horizontal = false, bool shift = false) {
            point = viewport->mapToScene(QPointF(20, viewport->height() / 2));
            // High-resolution Qt wheel events carry both pixel and angle deltas.
            QWheelEvent event(point, window.mapToGlobal(point.toPoint()), pixel ? (horizontal ? QPoint(direction * 40, 0) : QPoint(0, direction * 40)) : QPoint(),
                horizontal ? QPoint(direction * 120, 0) : QPoint(0, direction * 120),
                Qt::NoButton, shift ? Qt::ShiftModifier : Qt::NoModifier, pixel ? Qt::ScrollUpdate : Qt::NoScrollPhase, false,
                pixel ? Qt::MouseEventSynthesizedBySystem : Qt::MouseEventNotSynthesized,
                pixel ? &trackpad : QPointingDevice::primaryPointingDevice());
            timestamp += 16;
            event.setTimestamp(timestamp);
            QCoreApplication::sendEvent(&window, &event);
        };
        wheel();
        QTRY_VERIFY(viewport->property("contentY").toReal() > 0);
        QCOMPARE(outer->property("contentY").toReal(), 0.0);
        QCOMPARE(outer->property("observedWheels").toInt(), 0);
        const auto internalY = viewport->property("contentY");
        wheel(-1, true);
        QVERIFY(viewport->property("contentX").toReal() > 0);
        QCOMPARE(viewport->property("contentY"), internalY);
        const auto internalX = viewport->property("contentX").toReal();
        wheel(-1, false, true);
        QVERIFY(viewport->property("contentX").toReal() > internalX);
        QCOMPARE(viewport->property("contentY"), internalY);
        const auto edge = viewport->property("contentHeight").toReal() - viewport->height();
        viewport->setProperty("contentY", edge - 5);
        wheel();
        QCOMPARE(viewport->property("contentY").toReal(), edge);
        QCOMPARE(outer->property("contentY").toReal(), 0.0);
        wheel();
        wheel(); wheel();
        QTRY_VERIFY(outer->property("contentY").toReal() > 0);
        QMetaObject::invokeMethod(outer.get(), "cancelFlick");
        outer->setProperty("contentY", 0);
        const auto bottom = viewport->property("contentY").toReal();
        wheel(1);
        QVERIFY(viewport->property("contentY").toReal() < bottom);
        QCOMPARE(outer->property("contentY").toReal(), 0.0);
        viewport->setProperty("contentY", 5);
        wheel(1);
        QCOMPARE(viewport->property("contentY").toReal(), 0.0);
        QCOMPARE(outer->property("contentY").toReal(), 0.0);
        QMetaObject::invokeMethod(outer.get(), "cancelFlick");
        QTest::qWait(300); timestamp += 1000;
        outer->setProperty("contentY", 20);
        QCoreApplication::processEvents();
        wheel(1); wheel(1); wheel(1);
        QTRY_VERIFY(outer->property("contentY").toReal() < 20);
        QMetaObject::invokeMethod(outer.get(), "cancelFlick");
        outer->setProperty("contentY", 0);
        auto *reader = named(outer.get(), "reader"); QVERIFY(reader);
        reader->setProperty("markdown", "```\nshort\n```");
        viewport = named(outer.get(), "codeViewport"); QVERIFY(viewport);
        QTRY_VERIFY(viewport->property("contentHeight").toReal() <= viewport->height());
        wheel(); wheel(); wheel();
        QTRY_VERIFY(outer->property("contentY").toReal() > 0);
    }
    void blockingObserverPreventsNativeHandoff()
    {
        // Reproduces the sibling viewer's default-blocking observer independently
        // of QMarkdown. Such observers must set blocking: false in the host.
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(
import QtQuick
Flickable {
    width: 240; height: 300; contentHeight: 1500
    WheelHandler { onWheel: function(event) { event.accepted = false } }
}
)", {});
        std::unique_ptr<QQuickItem> outer(qobject_cast<QQuickItem *>(component.create()));
        QVERIFY2(outer, qPrintable(component.errorString()));
        QQuickWindow window; window.resize(240,300); outer->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QWheelEvent event(QPointF(40,40), window.mapToGlobal(QPoint(40,40)), {}, QPoint(0,-120),
            Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
        QCoreApplication::sendEvent(&window, &event);
        QTest::qWait(150);
        QCOMPARE(outer->property("contentY").toReal(), 0.0);
    }
    void plainSourceScrolling() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(240, 600); window.show();
        QString source; for (int i = 0; i < 20; ++i) source += QString(80, 'x') + "\n";
        view->setProperty("codeBlockWrapEnabled", false);
        view->setProperty("codeBlockHeightLimitEnabled", true);
        view->setProperty("codeBlockMinimumLines", 1);
        view->setProperty("codeBlockMaximumLines", 3);
        view->setProperty("markdown", "```\n" + source + "```");
        auto *viewport = named(view.get(), "codeViewport"); QVERIFY(viewport);
        QTRY_VERIFY(viewport->property("contentWidth").toReal() > viewport->width());
        QVERIFY(!native(view.get()));
        QTRY_VERIFY(viewport->height() > 0);
        QCoreApplication::processEvents();
        viewport->setProperty("contentY", viewport->height() * 2);
        QCoreApplication::processEvents();
        const auto before = viewport->property("contentY").toReal(); QVERIFY(before > 0);
        view->setProperty("codeBlockWrapEnabled", true);
        QTRY_VERIFY(viewport->property("contentY").toReal() > before);
        QCOMPARE(viewport->property("contentX").toReal(), 0.0);
        QVERIFY(!native(view.get()));
        view->setProperty("codeBlockWrapEnabled", false);
        QTRY_VERIFY(qAbs(viewport->property("contentY").toReal() - before) < 1);
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
        view->setProperty("codeBlockViewportHeight", 1);
        const auto effectiveLineHeight = text->sourceLines()[0].toMap().value("height").toReal();
        QTRY_COMPARE(viewport->height(), effectiveLineHeight);
        view->setProperty("codeBlockViewportHeight", 500);
        QTRY_COMPARE(viewport->height(), 250.0);
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
