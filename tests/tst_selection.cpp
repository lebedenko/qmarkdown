#include "private/document.h"
#include "private/formattedtext.h"
#include "private/resourcecontroller.h"
#include "private/viewstate.h"
#include "markdownstyle.h"
#include <QGuiApplication>
#include <QSignalSpy>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QTextBoundaryFinder>
#include <QTemporaryDir>
#include <QWheelEvent>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

class SelectionTest : public QObject
{
    Q_OBJECT
    static QList<FormattedText *> leaves(QQuickItem *item) {
        QList<FormattedText *> result;
        for (auto *child : item->childItems()) {
            if (auto *text = qobject_cast<FormattedText *>(child)) result.append(text);
            result.append(leaves(child));
        }
        return result;
    }
    static QPoint position(FormattedText *text, int offset, QQuickItem *root) {
        const auto line = text->layout()->lineForTextPosition(offset);
        // Cancel the renderer's raster overhang, preserving logical cursor geometry.
        return text->mapToItem(root, QPointF(line.cursorToX(offset) - text->x() + .5,
            line.y() + line.height() / 2 - text->y())).toPoint();
    }
    static QList<QQuickItem *> named(QQuickItem *item, const QString &name) {
        QList<QQuickItem *> result;
        for (auto *child : item->childItems()) {
            if (child->objectName() == name) result.append(child);
            result.append(named(child, name));
        }
        return result;
    }
private slots:
    void projection_data() {
        QTest::addColumn<QString>("source"); QTest::addColumn<QString>("expected");
        QTest::newRow("empty") << "" << "";
        QTest::newRow("decoded") << "# A &amp; B\n\n**strong** [label](host:x) ![alt *text*](missing.png)" << "A & B\n\nstrong label alt text";
        QTest::newRow("nested") << "> 3) [x] first\n>    - child\n> 4) [ ] next" << "3) [x] first\n  • child\n4) [ ] next";
        QTest::newRow("quotes") << "> one\n>\n> > two\n\nlast" << "one\n\ntwo\n\nlast";
        QTest::newRow("code-html") << "```\n a  \n\n```\n\n<div>\n raw  \n</div>\n\nlast" << " a  \n\n<div>\n raw  \n</div>\n\nlast";
        QTest::newRow("tables") << "| a | b |\n| - | - |\n| c | |\n| | d |" << "a\tb\nc\t\n\td";
        QTest::newRow("break") << "before\n\n---\n\nafter" << "before\n\nafter";
        QTest::newRow("mixed") << "- [ ] ~~word~~ www.a.b `a  b`\n- ![🙂](missing.png)" << "• [ ] word www.a.b a  b\n• 🙂";
        QTest::newRow("empty-cells") << "| | |\n|-|-|\n|||" << "\t\n\t";
        QTest::newRow("terminal-code") << "```\nline\n```" << "line\n";
    }
    void projection() {
        QFETCH(QString, source); QFETCH(QString, expected);
        auto blocks = QMarkdownPrivate::parse(source, {true, true, true, true, true});
        QCOMPARE(QMarkdownPrivate::projectText(blocks), expected);
        auto check = [&](auto &&self, const QVector<QMarkdownPrivate::Block> &values) -> void {
            for (const auto &block : values) {
                QVERIFY(block.selectionOffset >= 0);
                QVERIFY(block.selectionOffset + block.selectionLength <= expected.size());
                if (block.children.isEmpty()) QCOMPARE(expected.mid(block.selectionOffset, block.selectionLength), block.text);
                self(self, block.children);
            }
        };
        check(check, blocks);
    }
    void resourceRanges() {
        auto blocks = QMarkdownPrivate::parse("before ![alt](shape.png) after");
        QCOMPARE(QMarkdownPrivate::projectText(blocks), "before alt after");
        QImage image(10, 10, QImage::Format_ARGB32); image.fill(Qt::red);
        const auto projected = QMarkdownPrivate::projectImages(blocks, {{QUrl("file:///shape.png"), image}}, QUrl("file:///"));
        QCOMPARE(projected[0].children.size(), 3);
        QCOMPARE(projected[0].children[0].selectionOffset, 0);
        QCOMPARE(projected[0].children[0].selectionLength, 7);
        QCOMPARE(projected[0].children[1].selectionOffset, 7);
        QCOMPARE(projected[0].children[1].selectionLength, 3);
        QCOMPARE(projected[0].children[2].selectionOffset, 10);
        QCOMPARE(projected[0].children[2].selectionLength, 6);
    }
    void stateLifecycle() {
        ViewState state; state.setMarkdown("one\n\ntwo");
        QSignalSpy copy(&state, &ViewState::copyRequested);
        state.selectAll(); state.requestCopy(); QCOMPARE(copy.size(), 0);
        QVERIFY(state.selectedText().isEmpty());
        state.setSelectionEnabled(true); state.selectAll(); state.requestCopy();
        QCOMPARE(state.selectedText(), "one\n\ntwo"); QCOMPARE(state.selectionStart(), 0); QCOMPARE(state.selectionEnd(), 8);
        QCOMPARE(copy.takeFirst()[0].toString(), state.selectedText());
        state.setBaseUrl(QUrl("file:///tmp/"));
        state.resourcePolicy()->setAllowQrc(true);
        QCOMPARE(state.selectedText(), "one\n\ntwo");
        state.setGfmEnabled(true); QVERIFY(state.selectedText().isEmpty());
        state.selectAll(); state.setTablesEnabled(true); QCOMPARE(state.selectedText(), "one\n\ntwo");
        state.setSelectionEnabled(false); QVERIFY(state.selectedText().isEmpty());
        state.requestCopy(); QCOMPARE(copy.size(), 0);
        state.setSelectionEnabled(true); state.selectAll(); state.setMarkdown("new");
        QVERIFY(state.selectedText().isEmpty());
        state.requestCopy(); QCOMPARE(copy.size(), 0);
        state.selectAll(); state.clearSelection(); QCOMPARE(state.selectionStart(), state.selectionEnd());
        state.setMarkdown(""); state.selectAll(); state.requestCopy(); QCOMPARE(copy.size(), 0);
    }
    void geometry_data() {
        QTest::addColumn<QString>("text"); QTest::addColumn<bool>("padded"); QTest::addColumn<int>("alignment");
        for (const auto &text : {QString::fromUtf8("a🙂é z"), QString::fromUtf8("abc שלום مرحبا xyz"), QString("long code words wrap here")})
            for (bool padded : {false, true}) for (int alignment : {int(Qt::AlignLeft), int(Qt::AlignHCenter), int(Qt::AlignRight)})
                QTest::newRow(qPrintable(text + QString::number(padded) + QString::number(alignment))) << text << padded << alignment;
    }
    void geometry() {
        QFETCH(QString, text); QFETCH(bool, padded); QFETCH(int, alignment);
        FormattedText item;
        QFont font; font.setPixelSize(18); item.setFont(font); item.setCodeFont(font);
        item.setText(text); item.setLayoutWidth(100); item.setHorizontalAlignment(alignment);
        if (padded) {
            item.setFormatRanges({QVariantMap{{"start", 0}, {"length", text.size()}, {"flags", 4}}});
            item.setCodeSpans({QVariantMap{{"start", 0}, {"length", text.size()}}});
        }
        item.ensurePolished(); QVERIFY(item.layout());
        const auto height = item.logicalHeight();
        item.setSelectionStart(0); item.setSelectionEnd(text.size());
        const auto rects = item.selectionRectangles(); QVERIFY(!rects.isEmpty());
        QCOMPARE(item.logicalHeight(), height);
        QTextBoundaryFinder boundaries(QTextBoundaryFinder::Grapheme, text);
        for (int y = 1; y < height; y += 3) for (int x = -5; x < 110; x += 3) {
            const int cursor = item.cursorAt(x, y);
            boundaries.setPosition(cursor); QVERIFY2(boundaries.isAtBoundary(), qPrintable(QString::number(cursor)));
        }
        for (const auto &rect : rects) { QVERIFY(rect.width() > 0); QVERIFY(rect.height() > 0); }
        item.setSelectionEnd(0); QVERIFY(item.selectionRectangles().isEmpty());
    }

    void emptyLinesAndPaddedHits() {
        FormattedText item; QFont font; font.setPixelSize(18); item.setFont(font); item.setCodeFont(font);
        item.setText("a\n\nb"); item.setLayoutWidth(200); item.ensurePolished();
        const auto empty = item.layout()->lineAt(1);
        QCOMPARE(item.cursorAt(20, empty.y() + empty.height() / 2 + 1), 2);
        item.setText("before code after");
        item.setFormatRanges({QVariantMap{{"start", 7}, {"length", 4}, {"flags", 4}}});
        item.setCodeSpans({QVariantMap{{"start", 7}, {"length", 4}}}); item.ensurePolished();
        QCOMPARE(item.codeRectangles().size(), 1);
        const auto code = item.codeRectangles()[0];
        QCOMPARE(item.cursorAt(code.left() + 1.1, code.center().y() + 1), 7);
        QCOMPARE(item.cursorAt(code.right() + .9, code.center().y() + 1), 11);
        item.setSelectionStart(7); item.setSelectionEnd(11);
        QCOMPARE(item.selectionRectangles().size(), 1);
        QCOMPARE(item.selectionRectangles()[0], code);
        item.setSelectionStart(8); item.setSelectionEnd(9);
        QVERIFY(item.selectionRectangles()[0].width() < code.width());
    }

    void indivisibleGlyphAtNarrowWidth() {
        for (int alignment : {int(Qt::AlignLeft), int(Qt::AlignHCenter), int(Qt::AlignRight)}) {
            FormattedText item; QFont font; font.setPixelSize(18); item.setFont(font);
            item.setText("W"); item.setLayoutWidth(1); item.setHorizontalAlignment(alignment); item.ensurePolished();
            const qreal width = item.layout()->lineAt(0).width();
            item.setSelectionStart(0); item.setSelectionEnd(1);
            const auto cells = item.selectionRectangles(); QCOMPARE(cells.size(), 1);
            QVERIFY(cells[0].width() > 10);
            QCOMPARE(item.layout()->lineAt(0).width(), width);
            const QPointF origin = item.position();
            QCOMPARE(item.cursorAt(cells[0].left() - origin.x() + 1,
                cells[0].center().y() - origin.y()), 0);
            QCOMPARE(item.cursorAt(cells[0].right() - origin.x() - 1,
                cells[0].center().y() - origin.y()), 1);
        }
    }
    void enabledRenderingParity() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.7\nMarkdownView { width: 160; markdown: '# \\n\\nplain body wraps over several lines\\n\\n```\\n code  \\n\\n```\\n\\n<div>\\n html\\n</div>'; style.bodyFont.pixelSize: 18; style.codeBlockFont.pixelSize: 18 }", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(200, 400); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_VERIFY(view->property("contentHeight").toDouble() > 100);
        const auto height = view->property("contentHeight").toDouble();
        view->setProperty("selectionEnabled", true); QTRY_COMPARE(leaves(view).size(), 4);
        QTRY_COMPARE(view->property("contentHeight").toDouble(), height);
    }
    void style() {
        MarkdownStyle style;
        const auto light = style.selectionBackgroundColor(); QVERIFY(light.alpha() > 0 && light.alpha() < 255);
        style.setPreset(MarkdownStyle::PolishedDark); const auto dark = style.selectionBackgroundColor(); QVERIFY(dark != light);
        style.setSelectionBackgroundColor(Qt::red); style.setPreset(MarkdownStyle::Legacy);
        QCOMPARE(style.selectionBackgroundColor(), QColor(Qt::red));
        style.resetSelectionBackgroundColor(); QCOMPARE(style.selectionBackgroundColor(), light);
        style.setSelectionBackgroundColor(Qt::green); style.resetOverrides(); QCOMPARE(style.selectionBackgroundColor(), light);
    }

    void tableAndImageLifecycle() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.7\nMarkdownView { width: 260; selectionEnabled: true; tablesEnabled: true; markdown: '| left | right |\\n| :- | -: |\\n| body | |' }", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300, 250); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_COMPARE(named(view, "tableCell").size(), 4);
        const auto cells = named(view, "tableCell");
        // Right header padding must hit the right cell's end, not a nearby left glyph.
        const QPoint end = cells[1]->mapToScene({cells[1]->width() - 1, cells[1]->height() / 2}).toPoint();
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, end);
        QCOMPARE(view->property("selectionStart").toInt(), 10);
        const QPoint empty = cells[3]->mapToScene({cells[3]->width() / 2, cells[3]->height() / 2}).toPoint();
        QTest::mouseClick(&window, Qt::LeftButton, Qt::ShiftModifier, empty);
        QCOMPARE(view->property("selectedText").toString(), "\nbody\t");
        QVERIFY(QMetaObject::invokeMethod(view, "selectAll"));
        QCOMPARE(view->property("selectedText").toString(), "left\tright\nbody\t");
        QTemporaryDir dir; QVERIFY(dir.isValid());
        QImage image(40, 30, QImage::Format_ARGB32); image.fill(Qt::red);
        QVERIFY(image.save(dir.filePath("shape.png")));
        view->setProperty("baseUrl", QUrl::fromLocalFile(dir.path() + "/"));
        view->setProperty("markdown", "before ![alt](shape.png) after");
        QVERIFY(QMetaObject::invokeMethod(view, "selectAll"));
        auto *policy = view->property("resourcePolicy").value<MarkdownResourcePolicy *>(); QVERIFY(policy);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(dir.path() + "/")});
        QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        QCOMPARE(view->property("selectedText").toString(), "before alt after");
        auto *paint = named(view, "markdownImage")[0];
        QVERIFY(!paint->childItems().isEmpty()); QVERIFY(paint->childItems()[0]->isVisible());
        const auto left = paint->mapToScene({1, paint->height() / 2}).toPoint();
        const auto right = paint->mapToScene({paint->width() - 1, paint->height() / 2}).toPoint();
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, left);
        QTest::mouseMove(&window, right); QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, right);
        QCOMPARE(view->property("selectedText").toString(), "alt");
        policy->setAllowedFileRoots({}); QTRY_VERIFY(named(view, "markdownImage").isEmpty());
        QCOMPARE(view->property("selectedText").toString(), "alt");
        QTRY_COMPARE(leaves(view).size(), 1); QTRY_VERIFY(leaves(view)[0]->layout());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, position(leaves(view)[0], 8, window.contentItem()));
        QCOMPARE(view->property("selectionStart").toInt(), 8);
        QCOMPARE(view->property("selectionEnd").toInt(), 8);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(dir.path() + "/")});
        QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        QVERIFY(view->property("selectedText").toString().isEmpty());
        QVERIFY(!named(view, "markdownImage")[0]->childItems()[0]->isVisible());
    }
    void actionsAndScrolling() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.7
Flickable {
    id: scrollHost
    WheelHandler {
        target: null
        onWheel: function(event) { scrollHost.contentY = Math.max(0, scrollHost.contentY - event.angleDelta.y / 2) }
    }
    width: 260; height: 160; contentHeight: view.contentHeight; clip: true
    property int actions: 0
    property alias document: view
    MarkdownView {
        id: view; width: 260; selectionEnabled: true
        markdown: "```\nline\n```\n\n" + "paragraph words\n\n".repeat(30)
        codeBlockActions: Rectangle {
            required property string codeText
            required property var requestCopy
            objectName: "actionControl"; implicitHeight: 30
            TapHandler { onTapped: { parent.requestCopy(); } }
        }
    }
})", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *scroll = qobject_cast<QQuickItem *>(object.get()); QVERIFY(scroll);
        auto *view = scroll->property("document").value<QQuickItem *>(); QVERIFY(view);
        QQuickWindow window; window.resize(260, 160); scroll->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_VERIFY(!leaves(view).isEmpty());
        QTRY_VERIFY(scroll->property("contentHeight").toDouble() > 300);
        QSignalSpy codeCopy(view, SIGNAL(codeBlockCopyRequested(QString)));
        QMetaObject::invokeMethod(view, "selectAll"); const auto all = view->property("selectedText").toString();
        const auto controls = named(view, "actionControl"); QCOMPARE(controls.size(), 1);
        const auto point = controls[0]->mapToScene({20, 15}).toPoint();
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(codeCopy.size(), 1); QCOMPARE(codeCopy[0][0].toString(), "line\n");
        QCOMPARE(view->property("selectedText").toString(), all);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, point);
        QTest::mouseMove(&window, QPoint(100, 80)); QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(100, 80));
        QCOMPARE(view->property("selectedText").toString(), all);
        scroll->setProperty("contentY", 0);
        QTRY_VERIFY(!scroll->property("moving").toBool());
        QTRY_COMPARE(scroll->property("contentY").toDouble(), 0.0);
        auto text = leaves(view); QTRY_VERIFY(text[1]->layout() && text[2]->layout());
        const auto from = position(text[1], 0, window.contentItem()), to = position(text[2], 14, window.contentItem());
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(&window, to);
        QTRY_COMPARE(view->property("selectedText").toString(), "paragraph words\n\nparagraph word");
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, to);
        QCOMPARE(view->property("selectedText").toString(), "paragraph words\n\nparagraph word");
        QCOMPARE(scroll->property("contentY").toDouble(), 0.0);
        QMetaObject::invokeMethod(view, "selectAll");
        scroll->setProperty("contentY", 0);
        QWheelEvent wheel(QPointF(100, 100), window.mapToGlobal(QPoint(100, 100)), QPoint(0, -60), QPoint(0, -120),
            Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
        QCoreApplication::sendEvent(&window, &wheel);
        QTRY_VERIFY(scroll->property("contentY").toDouble() > 0);
        QCOMPARE(view->property("selectedText").toString(), all);
        scroll->setProperty("contentY", 0);
        auto *device = QTest::createTouchDevice();
        QTest::touchEvent(&window, device).press(0, QPoint(100, 140), &window);
        QTest::touchEvent(&window, device).move(0, QPoint(100, 100), &window);
        QTest::touchEvent(&window, device).move(0, QPoint(100, 40), &window);
        QTest::touchEvent(&window, device).release(0, QPoint(100, 40), &window);
        QTRY_VERIFY(scroll->property("contentY").toDouble() > 0);
        QCOMPARE(view->property("selectedText").toString(), all);
    }


    void resourceResetCancelsPendingLinkTap() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.7\nMarkdownView { width: 200; selectionEnabled: true; markdown: '[label](host:x)' }", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(200, 120); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_COMPARE(leaves(view).size(), 1);
        QTRY_VERIFY(leaves(view)[0]->layout());
        const auto point = position(leaves(view)[0], 2, window.contentItem());
        QSignalSpy link(view, SIGNAL(linkActivated(QString)));
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, point);
        auto *policy = view->property("resourcePolicy").value<MarkdownResourcePolicy *>(); QVERIFY(policy);
        policy->setAllowQrc(true);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(link.size(), 0); QCOMPARE(view->property("selectionStart").toInt(), 2);
        QCOMPARE(view->property("selectionEnd").toInt(), 2);
        QTRY_VERIFY(leaves(view)[0]->layout());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point); QCOMPARE(link.size(), 1);
    }
    void emptyDestinationLink() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.7\nMarkdownView { width: 200; selectionEnabled: true; markdown: '[empty]()' }", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(200, 120); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_COMPARE(leaves(view).size(), 1);
        auto *text = leaves(view)[0]; QTRY_VERIFY(text->layout());
        QSignalSpy link(view, SIGNAL(linkActivated(QString)));
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, position(text, 2, window.contentItem()));
        QCOMPARE(link.size(), 1); QCOMPARE(link[0][0].toString(), QString());
    }
    void mouseAndKeyboard() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.7\nMarkdownView { width: 300; selectionEnabled: true; markdown: '[first](host:first) block\\n\\nsecond block' }", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300, 220); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_COMPARE(leaves(view).size(), 2);
        auto text = leaves(view); QTRY_VERIFY(text[0]->layout() && text[1]->layout());
        QSignalSpy link(view, SIGNAL(linkActivated(QString))); QSignalSpy copy(view, SIGNAL(copyRequested(QString)));
        const auto a = position(text[0], 1, window.contentItem()), b = position(text[1], 6, window.contentItem());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, a); QCOMPARE(link.size(), 1);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, a);
        QTest::mouseMove(&window, b); QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, b);
        QCOMPARE(link.size(), 1); QCOMPARE(view->property("selectedText").toString(), "irst block\n\nsecond");
        QTest::keyClick(&window, Qt::Key_C, Qt::ControlModifier);
        QCOMPARE(copy.size(), 1); QCOMPARE(copy[0][0].toString(), view->property("selectedText").toString());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::ShiftModifier, position(text[0], 4, window.contentItem()));
        QCOMPARE(view->property("selectedText").toString(), "irs"); QCOMPARE(link.size(), 1);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, b);
        QTest::mouseMove(&window, a); QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, a);
        QCOMPARE(view->property("selectedText").toString(), "irst block\n\nsecond");
        QTest::keyClick(&window, Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(view->property("selectedText").toString(), "first block\n\nsecond block");
        QQuickItem hostCopy(window.contentItem()); hostCopy.setFlag(QQuickItem::ItemIsFocusScope); hostCopy.forceActiveFocus();
        QCOMPARE(view->property("selectedText").toString(), "first block\n\nsecond block");
        QVERIFY(QMetaObject::invokeMethod(view, "requestCopy")); QCOMPARE(copy.size(), 2);
        view->setWidth(20); QTRY_VERIFY(text[0]->layout()->lineCount() > 1);
        QCOMPARE(view->property("selectedText").toString(), "first block\n\nsecond block");
        view->setWidth(0); QTRY_VERIFY(leaves(view).isEmpty()); view->setWidth(300); QTRY_COMPARE(leaves(view).size(), 2);
        QCOMPARE(view->property("selectedText").toString(), "first block\n\nsecond block");
        view->forceActiveFocus(); QTest::keyClick(&window, Qt::Key_Escape); QVERIFY(view->property("selectedText").toString().isEmpty());
    }
};
QTEST_MAIN(SelectionTest)
#include "tst_selection.moc"
