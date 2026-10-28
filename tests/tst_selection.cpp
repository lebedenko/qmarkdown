#include "private/document.h"
#include "private/formattedtext.h"
#include "private/resourcecontroller.h"
#include "private/presentation.h"
#include "private/viewstate.h"
#include "private/selectionregistration.h"
#include "markdownstyle.h"
#include <QGuiApplication>
#include <QSignalSpy>
#include <QStyleHints>
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
    void registrationLifecycle() {
        ViewState first, second;
        first.setMarkdown("abcdef"); first.setSelectionEnabled(true);
        second.setMarkdown("abcdef"); second.setSelectionEnabled(true);
        QQuickItem view;
        auto image = std::make_unique<QQuickItem>(&view);
        image->setWidth(100); image->setHeight(30);
        auto registration = std::make_unique<SelectionRegistration>();
        registration->setCoordinator(&first); registration->setItem(image.get());
        registration->setKind(ViewState::ImageSelection);
        registration->setOffset(1); registration->setLength(3);
        registration->setImageLink("first"); registration->setImageLinked(true);
        QVERIFY(!first.selectionLink(&view, {10, 10}).isValid());
        registration->componentComplete();
        QCOMPARE(first.selectionLink(&view, {10, 10}).toString(), "first");
        QVERIFY(first.beginSelection(&view, {0, 10}, false));
        first.extendSelection(&view, {99, 10});
        QCOMPARE(first.selectedText(), "bcd");
        registration->setOffset(2); registration->setLength(2);
        registration->setImageLink("updated");
        QVERIFY(first.beginSelection(&view, {0, 10}, false));
        first.extendSelection(&view, {99, 10});
        QCOMPARE(first.selectedText(), "cd");
        QCOMPARE(first.selectionLink(&view, {10, 10}).toString(), "updated");
        registration->setImageLinked(false);
        QVERIFY(!first.selectionLink(&view, {10, 10}).isValid());
        registration->setImageLinked(true);
        registration->setCoordinator(&second);
        QVERIFY(!first.selectionLink(&view, {10, 10}).isValid());
        QCOMPARE(second.selectionLink(&view, {10, 10}).toString(), "updated");
        QQuickItem replacement(&view); replacement.setX(200);
        replacement.setWidth(100); replacement.setHeight(30);
        registration->setItem(&replacement);
        QVERIFY(!second.selectionLink(&view, {10, 10}).isValid());
        QCOMPARE(second.selectionLink(&view, {210, 10}).toString(), "updated");
        registration.reset();
        QVERIFY(!second.selectionLink(&view, {210, 10}).isValid());
        SelectionRegistration lifetime;
        lifetime.setCoordinator(&first); lifetime.setItem(image.get());
        lifetime.componentComplete(); image.reset();
        QVERIFY(!lifetime.item());
        auto temporary = std::make_unique<ViewState>();
        lifetime.setCoordinator(temporary.get()); lifetime.setItem(&replacement);
        temporary.reset(); QVERIFY(!lifetime.coordinator());
    }
    void atomicMathGeometry() {
        ViewState state;
        state.setMathEnabled(true);
        state.setMarkdown("$$\nx+y\nz+1\n$$");
        state.setSelectionEnabled(true);
        QQuickItem view, formula(&view);
        formula.setWidth(100); formula.setHeight(40);
        SelectionRegistration registration;
        registration.setCoordinator(&state); registration.setItem(&formula);
        registration.setKind(ViewState::MathSelection);
        registration.setOffset(0); registration.setLength(8);
        registration.componentComplete();
        QVERIFY(!state.selectionLink(&view, {10, 10}).isValid());
        QVERIFY(state.beginSelection(&view, {10, 10}, false));
        QCOMPARE(state.selectionStart(), 0); QCOMPARE(state.selectionEnd(), 0);
        state.extendSelection(&view, {90, 10});
        QCOMPARE(state.selectedText(), QString("x+y\nz+1\n"));
        state.clearSelection(); state.cancelSelectionGesture();
        // A fresh coordinate starts a click series; both repeated granularities
        // must include all lines and the retained terminal newline.
        for (int click = 0; click < 3; ++click) {
            QVERIFY(state.beginSelection(&view, {30, 30}, false));
            if (click > 0) QCOMPARE(state.selectedText(), QString("x+y\nz+1\n"));
        }
        state.cancelSelectionGesture();
        QVERIFY(state.beginSelection(&view, {90, 10}, false));
        state.extendSelection(&view, {10, 10});
        QCOMPARE(state.selectedText(), QString("x+y\nz+1\n"));
    }

    void exclusionRegistrations() {
        ViewState state; state.setSelectionEnabled(true); state.setMarkdown("abcdef");
        QQuickItem view, cell(&view), image(&view), action(&view);
        cell.setWidth(100); cell.setHeight(30);
        image.setX(100); image.setWidth(100); image.setHeight(30);
        action.setWidth(20); action.setHeight(20);
        SelectionRegistration cellRegistration, imageRegistration, actionRegistration;
        cellRegistration.setCoordinator(&state); cellRegistration.setItem(&cell);
        cellRegistration.setKind(ViewState::TableCellSelection); cellRegistration.componentComplete();
        imageRegistration.setCoordinator(&state); imageRegistration.setItem(&image);
        imageRegistration.setKind(ViewState::ImageSelection);
        imageRegistration.setOffset(2); imageRegistration.setLength(3); imageRegistration.componentComplete();
        actionRegistration.setCoordinator(&state); actionRegistration.setItem(&action);
        actionRegistration.setKind(ViewState::ActionSelection); actionRegistration.componentComplete();
        QVERIFY(!state.beginSelection(&view, {10, 10}, false));
        QVERIFY(state.beginSelection(&view, {50, 10}, false));
        QCOMPARE(state.selectionStart(), 0); // Empty cell cannot select a neighboring cell's text.
        actionRegistration.setItem(nullptr);
        QVERIFY(state.beginSelection(&view, {10, 10}, false));
        cellRegistration.setItem(nullptr);
        QVERIFY(state.beginSelection(&view, {150, 10}, false));
        QCOMPARE(state.selectionStart(), 5);
    }

    void repeatedSelection_data() {
        QTest::addColumn<QString>("source");
        QTest::addColumn<int>("leaf"); QTest::addColumn<int>("offset");
        QTest::addColumn<int>("clicks"); QTest::addColumn<QString>("expected");
        QTest::newRow("word") << "alpha beta" << 0 << 2 << 2 << "alpha";
        QTest::newRow("combining") << QString::fromUtf8("café next") << 0 << 3 << 2 << QString::fromUtf8("café");
        QTest::newRow("bidi") << QString::fromUtf8("abc שלום xyz") << 0 << 5 << 2 << QString::fromUtf8("שלום");
        QTest::newRow("punctuation") << "one!? next" << 0 << 3 << 2 << "!";
        QTest::newRow("emoji") << QString::fromUtf8("a 🙂 next") << 0 << 2 << 2 << QString::fromUtf8("🙂");
        QTest::newRow("whitespace") << "```\na   b\n```" << 0 << 2 << 2 << "   ";
        QTest::newRow("paragraph") << "long paragraph wraps over several visual lines\n\nsibling" << 0 << 7 << 3 << "long paragraph wraps over several visual lines";
        QTest::newRow("heading") << "# heading words\n\nsibling" << 0 << 3 << 3 << "heading words";
        QTest::newRow("nested") << "> - parent\n>   - child words\n>   - next" << 1 << 3 << 3 << "child words";
        QTest::newRow("cell") << "| left | right |\n|-|-|\n|body||" << 1 << 2 << 3 << "right";
        QTest::newRow("empty-cell") << "| left | right |\n|-|-|\n|body||" << 3 << 0 << 3 << "";
        QTest::newRow("code-line") << "```\nfirst\nsecond line\n```" << 0 << 9 << 3 << "second line\n";
        QTest::newRow("empty-code-line") << "```\nfirst\n\nlast\n```" << 0 << 6 << 3 << "\n";
        QTest::newRow("final-html-line") << "<div>\nraw text\n</div>" << 0 << 17 << 3 << "</div>\n";
        QTest::newRow("html-line") << "<div>\nraw text\n</div>" << 0 << 8 << 3 << "raw text\n";
    }
    void repeatedSelection() {
        QFETCH(QString, source); QFETCH(int, leaf); QFETCH(int, offset);
        QFETCH(int, clicks); QFETCH(QString, expected);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.7\nMarkdownView { width: 140; selectionEnabled: true; tablesEnabled: true }", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        view->setProperty("markdown", source);
        QQuickWindow window; window.resize(200, 500); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_VERIFY(leaves(view).size() > leaf);
        auto *text = leaves(view)[leaf]; QTRY_VERIFY(text->layout());
        const auto point = position(text, offset, window.contentItem());
        for (int i = 0; i < clicks; ++i) QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(view->property("selectedText").toString(), expected);
    }
    void repeatedClickThresholds() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.8\nMarkdownView { width: 300; selectionEnabled: true; markdown: 'first word next' }", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300, 140); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_COMPARE(leaves(view).size(), 1);
        QTRY_VERIFY(leaves(view)[0]->layout());
        const auto first = position(leaves(view)[0], 2, window.contentItem());
        const auto distant = position(leaves(view)[0], 12, window.contentItem());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, first);
        QTest::qWait(QGuiApplication::styleHints()->mouseDoubleClickInterval() + 30);
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, first);
        QVERIFY(view->property("selectedText").toString().isEmpty());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, distant);
        QVERIFY(view->property("selectedText").toString().isEmpty());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, distant);
        QCOMPARE(view->property("selectedText").toString(), "next");
    }
    void wholeUnitDragging() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.7\nMarkdownView { width: 300; selectionEnabled: true; markdown: 'one two three\\n\\nfour five' }", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300, 200); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_COMPARE(leaves(view).size(), 2);
        auto text = leaves(view); QTRY_VERIFY(text[0]->layout() && text[1]->layout());
        const auto start = position(text[0], 5, window.contentItem());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, start);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start);
        QCOMPARE(view->property("selectedText").toString(), "two");
        QTest::mouseMove(&window, position(text[0], 11, window.contentItem()));
        QTRY_COMPARE(view->property("selectedText").toString(), "two three");
        QTest::mouseMove(&window, position(text[0], 1, window.contentItem()));
        QTRY_COMPARE(view->property("selectedText").toString(), "one two");
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, position(text[0], 1, window.contentItem()));
        QVERIFY(QMetaObject::invokeMethod(view, "clearSelection"));
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, start);
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, start);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start);
        QCOMPARE(view->property("selectedText").toString(), "one two three");
        const auto end = position(text[1], 2, window.contentItem());
        QTest::mouseMove(&window, end);
        QTRY_COMPARE(view->property("selectedText").toString(), "one two three\n\nfour five");
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, end);
    }
    void edgeScrollSpeed_data() {
        QTest::addColumn<int>("y"); QTest::addColumn<qreal>("expectedSpeed");
        QTest::newRow("upper-half") << 16 << qreal(-300);
        QTest::newRow("lower-half") << 144 << qreal(300);
        QTest::newRow("above") << -10 << qreal(-600);
        QTest::newRow("below") << 170 << qreal(600);
    }
    void edgeScrollSpeed() {
        QFETCH(int, y); QFETCH(qreal, expectedSpeed);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.8
Item {
    id: viewport; width: 240; height: 160
    property alias document: document
    MarkdownView {
        id: document; width: 240; selectionEnabled: true; selectionViewport: viewport
        markdown: "paragraph words\n\n".repeat(20)
    }
})", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *viewport = qobject_cast<QQuickItem *>(object.get()); QVERIFY(viewport);
        auto *view = viewport->property("document").value<QQuickItem *>(); QVERIFY(view);
        QQuickWindow window; window.resize(240, 160); viewport->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_VERIFY(!leaves(view).isEmpty());
        QSignalSpy requests(view, SIGNAL(selectionScrollRequested(double)));
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(40, 80));
        QElapsedTimer elapsed; elapsed.start();
        QTest::mouseMove(&window, QPoint(40, y));
        QTRY_VERIFY(requests.size() >= 8);
        qreal displacement = 0;
        for (const auto &request : requests) { displacement += request[0].toDouble(); QVERIFY(qAbs(request[0].toDouble()) <= 30); }
        const qreal measuredSpeed = displacement * 1000 / elapsed.elapsed();
        QVERIFY2(qAbs(measuredSpeed - expectedSpeed) < qAbs(expectedSpeed) * .2,
            qPrintable(QString::number(measuredSpeed)));
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(40, y));
    }
    void partialCodeCopy_data() {
        QTest::addColumn<bool>("wrap");
        QTest::addColumn<bool>("numbers");
        QTest::addColumn<bool>("scrolled");
        for (bool wrap : {false, true})
            for (bool numbers : {false, true})
                for (bool scrolled : {false, true})
                    QTest::newRow(qPrintable(QString("wrap%1-numbers%2-scrolled%3").arg(wrap).arg(numbers).arg(scrolled)))
                        << wrap << numbers << scrolled;
    }
    void partialCodeCopy() {
        QFETCH(bool, wrap); QFETCH(bool, numbers); QFETCH(bool, scrolled);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.18
MarkdownView {
    width: 520; selectionEnabled: true
    syntaxHighlightingEnabled: true
    codeBlockMinimumLines: 8; codeBlockMaximumLines: 8
    codeBlockHeightLimitEnabled: true
})", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        view->setProperty("codeBlockWrapEnabled", wrap);
        view->setProperty("codeBlockLineNumbersVisible", numbers);
        const QString source = QString::fromUtf8("    while\tbeta\n\n  中文 😀 é tail\nnext line end\n")
            + QString("long_identifier ").repeated(15) + "\n" + QString("padding line\n").repeated(15);
        view->setProperty("markdown", "Before prose\n\n```cpp\n" + source + "```\n\nAfter prose");
        QQuickWindow window; window.resize(540, 900); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_COMPARE(leaves(view).size(), 3);
        auto *text = leaves(view)[1]; QTRY_VERIFY(text->layout());
        QCOMPARE(text->text(), source.chopped(1));
        if (view->property("syntaxHighlightingAvailable").toBool())
            QTRY_VERIFY(!text->foregroundRanges().isEmpty());
        auto *viewport = named(view, "codeViewport").value(0); QVERIFY(viewport);
        QVERIFY(!window.grabWindow().isNull());
        QSignalSpy copies(view, SIGNAL(copyRequested(QString)));
        struct Range { QString start; int skip; QString expected; };
        QList<Range> ranges = scrolled
            ? QList<Range>{{QString::fromUtf8("中文"), 0, QString::fromUtf8("中文 😀 é")},
                           {"next", 2, "xt line"}}
            : QList<Range>{{"while", 1, "hile\tbe"}, {"    while", 1, "   while\tbeta\n\n  "},
                           {QString::fromUtf8("中文"), 0, QString::fromUtf8("中文 😀 é tail\nnext li")}};
        if (wrap && !scrolled)
            ranges.append({"long_identifier", 32,
                "long_identifier long_identifier long_identifier long_identifier "});
        for (const auto &range : ranges) {
            const int start = source.indexOf(range.start) + range.skip;
            const int end = start + range.expected.size();
            for (bool reverse : {false, true}) {
                // Separate gestures so repeated-click granularity cannot affect the test.
                QTest::qWait(QGuiApplication::styleHints()->mouseDoubleClickInterval() + 1);
                if (scrolled) {
                    viewport->setProperty("contentY", text->layout()->lineAt(2).y());
                    if (!wrap) viewport->setProperty("contentX", 12.0);
                    QCoreApplication::processEvents();
                    QVERIFY(viewport->property("contentY").toDouble() > 0);
                    if (!wrap) QVERIFY(viewport->property("contentX").toDouble() > 0);
                }
                const auto a = position(text, reverse ? end : start, window.contentItem());
                const auto b = position(text, reverse ? start : end, window.contentItem());
                QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, a);
                QTest::mouseMove(&window, b);
                QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, b);
                QCOMPARE(view->property("selectionStart").toInt(), 14 + start);
                QCOMPARE(view->property("selectionEnd").toInt(), 14 + end);
                QCOMPARE(view->property("selectedText").toString(), range.expected);
                view->forceActiveFocus(); QTest::keyClick(&window, Qt::Key_C, Qt::ControlModifier);
                QCOMPARE(copies.size(), 1);
                QCOMPARE(copies.takeFirst()[0].toString(), range.expected);
            }
        }
    }
    void nestedCodeSelectionScrolling_data() {
        QTest::addColumn<bool>("overflow");
        QTest::newRow("natural-height") << false;
        QTest::newRow("bounded-height") << true;
    }
    void nestedCodeSelectionScrolling() {
        QFETCH(bool, overflow);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.18
Item {
    width: 400; height: 700
    property alias document: document
    MarkdownView {
        id: document; y: 180; width: 400; selectionEnabled: true
        selectionViewport: parent
        codeBlockMinimumLines: 3; codeBlockMaximumLines: 3
        codeBlockWrapEnabled: false
    }
})", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *view = host->property("document").value<QQuickItem *>(); QVERIFY(view);
        view->setProperty("codeBlockHeightLimitEnabled", overflow);
        view->setProperty("markdown", "```\n" + (QString("line with words ").repeated(8) + "\n").repeated(overflow ? 20 : 3) + "```");
        QQuickWindow window; window.resize(400, 700); host->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_COMPARE(named(view, "codeViewport").size(), 1);
        auto *viewport = named(view, "codeViewport")[0];
        QTRY_VERIFY(viewport->height() > 0); QTRY_VERIFY(leaves(view)[0]->layout());
        const auto center = viewport->mapToScene({80, viewport->height() / 2}).toPoint();
        const auto bottom = viewport->mapToScene({viewport->width() - 1, viewport->height() - 1}).toPoint();
        QSignalSpy requests(view, SIGNAL(selectionScrollRequested(double)));
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, center);
        QTest::mouseMove(&window, bottom);
        if (overflow) {
            QTRY_VERIFY(viewport->property("contentY").toDouble() > 0);
            const int end = view->property("selectionEnd").toInt();
            QTRY_VERIFY(view->property("selectionEnd").toInt() > end);
            QCOMPARE(requests.size(), 0);
            viewport->setProperty("contentY", viewport->property("contentHeight").toDouble() - viewport->height());
        }
        QTRY_VERIFY(viewport->property("contentX").toDouble() > 0);
        QTest::qWait(90); QCOMPARE(requests.size(), 0);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
        viewport->setProperty("contentY", 0);
        const auto top = viewport->mapToScene({100, 1}).toPoint();
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, center);
        QTest::mouseMove(&window, top);
        QTest::qWait(90); QCOMPARE(requests.size(), 0);
        QCOMPARE(viewport->property("contentY").toDouble(), 0.0);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, top);
        // The same exhausted source now overlaps the outer bottom edge.
        view->setY(700 - viewport->y() - viewport->height());
        const auto outerBottom = viewport->mapToScene({100, viewport->height() - 1}).toPoint();
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(80, 660));
        QTest::mouseMove(&window, outerBottom);
        if (overflow) {
            QTRY_VERIFY(viewport->property("contentY").toDouble() > 0);
            QCOMPARE(requests.size(), 0);
            viewport->setProperty("contentY", viewport->property("contentHeight").toDouble() - viewport->height());
        }
        QTRY_VERIFY(!requests.isEmpty());
        QVERIFY(requests.last()[0].toDouble() > 0);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, outerBottom);
        const int count = requests.size(); QTest::qWait(70); QCOMPARE(requests.size(), count);
    }
    void edgeAutoscrolling() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.7
Flickable {
    id: host
    width: 240; height: 160; clip: true; contentHeight: document.contentHeight
    property alias document: document
    MarkdownView {
        id: document; width: host.width; selectionEnabled: true; selectionViewport: host
        markdown: "paragraph words\n\n".repeat(40)
        onSelectionScrollRequested: function(deltaY) {
            host.contentY = Math.max(0, Math.min(host.contentHeight - host.height, host.contentY + deltaY))
        }
    }
})", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *view = host->property("document").value<QQuickItem *>(); QVERIFY(view);
        QQuickWindow window; window.resize(240, 160); host->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_VERIFY(leaves(view).size() == 40);
        QTRY_VERIFY(leaves(view)[0]->layout());
        QSignalSpy requests(view, SIGNAL(selectionScrollRequested(double)));
        const QPoint start = position(leaves(view)[0], 1, window.contentItem());
        const QPoint bottom(100, 159);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start);
        QTest::mouseMove(&window, bottom);
        QTRY_VERIFY(host->property("contentY").toDouble() > 40);
        const int initialEnd = view->property("selectionEnd").toInt();
        QTRY_VERIFY(view->property("selectionEnd").toInt() > initialEnd);
        QVERIFY(!requests.isEmpty());
        for (const auto &request : requests) { QVERIFY(request[0].toDouble() > 0); QVERIFY(request[0].toDouble() <= 30); }
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
        const int stopped = requests.size(); QTest::qWait(70); QCOMPARE(requests.size(), stopped);
        // A stationary pointer above the top edge scrolls upward, then clamps at zero.
        host->setProperty("contentY", 120);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(100, 80));
        QTest::mouseMove(&window, QPoint(100, -10));
        QTRY_VERIFY(host->property("contentY").toDouble() < 80);
        QTRY_COMPARE(host->property("contentY").toDouble(), 0.0);
        QVERIFY(requests.last()[0].toDouble() < 0);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(100, -10));
        requests.clear();
        // Requests at the lower document boundary leave the host clamped.
        const qreal maximum = host->property("contentHeight").toDouble() - host->height();
        host->setProperty("contentY", maximum - 5);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(100, 80));
        QTest::mouseMove(&window, bottom);
        QTRY_COMPARE(host->property("contentY").toDouble(), maximum);
        const int boundEnd = view->property("selectionEnd").toInt();
        QTest::qWait(70); QCOMPARE(host->property("contentY").toDouble(), maximum);
        QCOMPARE(view->property("selectionEnd").toInt(), boundEnd);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
        host->setProperty("contentY", 0);
        // Source reset and disabling selection stop an existing drag immediately.
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start); QTest::mouseMove(&window, bottom);
        QTRY_VERIFY(!requests.isEmpty());
        view->setProperty("markdown", "replacement");
        const int resetCount = requests.size(); QTest::qWait(70); QCOMPARE(requests.size(), resetCount);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
        view->setProperty("markdown", QString("paragraph words\n\n").repeated(40));
        host->setProperty("contentY", 0); QTRY_VERIFY(leaves(view)[0]->layout());
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start); QTest::mouseMove(&window, bottom);
        QTRY_VERIFY(requests.size() > resetCount);
        view->setProperty("selectionEnabled", false);
        const int disabledCount = requests.size(); QTest::qWait(70); QCOMPARE(requests.size(), disabledCount);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
        view->setProperty("selectionEnabled", true); host->setProperty("contentY", 0);
        QTRY_VERIFY(leaves(view)[0]->layout());
        auto viewport = std::make_unique<QQuickItem>(window.contentItem()); viewport->setSize({240, 160});
        view->setProperty("selectionViewport", QVariant::fromValue(viewport.get()));
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start); QTest::mouseMove(&window, bottom);
        QTRY_VERIFY(requests.size() > disabledCount);
        viewport.reset();
        QVERIFY(!view->property("selectionViewport").value<QQuickItem *>());
        const int destroyedCount = requests.size(); QTest::qWait(70); QCOMPARE(requests.size(), destroyedCount);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
        // Explicit viewport removal stops requests during the current drag.
        host->setProperty("contentY", 0);
        view->setProperty("selectionViewport", QVariant::fromValue(host));
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start); QTest::mouseMove(&window, bottom);
        QTRY_VERIFY(requests.size() > destroyedCount);
        view->setProperty("selectionViewport", QVariant::fromValue<QQuickItem *>(nullptr));
        const int removedCount = requests.size(); QTest::qWait(70); QCOMPARE(requests.size(), removedCount);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
        // Null viewport permits selection without requesting scrolling.
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start); QTest::mouseMove(&window, bottom);
        QTest::qWait(70); QCOMPARE(requests.size(), removedCount);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
        view->setProperty("selectionViewport", QVariant::fromValue(host));
        host->setProperty("contentY", 0);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start); QTest::mouseMove(&window, bottom);
        QTRY_VERIFY(requests.size() > removedCount);
        view->forceActiveFocus(); QTest::keyClick(&window, Qt::Key_Escape);
        const int escapedCount = requests.size(); QTest::qWait(70); QCOMPARE(requests.size(), escapedCount);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
        host->setProperty("contentY", 0);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, start); QTest::mouseMove(&window, bottom);
        QTRY_VERIFY(requests.size() > escapedCount);
        QQuickWindow other; other.resize(100, 100); other.show(); other.requestActivate();
        QTRY_VERIFY(!window.isActive());
        const QString retained = view->property("selectedText").toString();
        const int inactiveCount = requests.size(); QTest::qWait(70); QCOMPARE(requests.size(), inactiveCount);
        QCOMPARE(view->property("selectedText").toString(), retained);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, bottom);
    }
    void delayedLinks() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.7\nMarkdownView { width: 200; selectionEnabled: true; markdown: '[label](host:x)' }", {});
        std::unique_ptr<QObject> object(component.create()); QVERIFY2(object, qPrintable(component.errorString()));
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(200, 120); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window)); QTRY_COMPARE(leaves(view).size(), 1);
        QTRY_VERIFY(leaves(view)[0]->layout());
        const auto point = position(leaves(view)[0], 2, window.contentItem());
        QSignalSpy links(view, SIGNAL(linkActivated(QString)));
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(links.size(), 0); QTRY_COMPARE(links.size(), 1);
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(view->property("selectedText").toString(), "label");
        QTest::qWait(QGuiApplication::styleHints()->mouseDoubleClickInterval() + 30);
        QCOMPARE(links.size(), 1);
        QVERIFY(QMetaObject::invokeMethod(view, "clearSelection"));
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
        view->setProperty("markdown", "replacement");
        QTest::qWait(QGuiApplication::styleHints()->mouseDoubleClickInterval() + 30);
        QCOMPARE(links.size(), 1);
        view->setProperty("markdown", "[label](host:x)"); QTRY_VERIFY(leaves(view)[0]->layout());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
        view->setProperty("selectionEnabled", false);
        QTest::qWait(QGuiApplication::styleHints()->mouseDoubleClickInterval() + 30);
        QCOMPARE(links.size(), 1);
        view->setProperty("selectionEnabled", true); QTRY_VERIFY(leaves(view)[0]->layout());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
        QQuickWindow other; other.resize(100, 100); other.show(); other.requestActivate();
        QTRY_VERIFY(!window.isActive());
        QTest::qWait(QGuiApplication::styleHints()->mouseDoubleClickInterval() + 30);
        QCOMPARE(links.size(), 1);
    }
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
        QCOMPARE(QMarkdownPrivate::projectDocument(blocks).text, expected);
        auto check = [&](auto &&self, const QVector<QMarkdownPrivate::PresentationBlock> &values) -> void {
            for (const auto &block : values) {
                QVERIFY(block.selectionOffset >= 0);
                QVERIFY(block.selectionOffset + block.selectionLength <= expected.size());
                if (block.children.isEmpty()) QCOMPARE(expected.mid(block.selectionOffset, block.selectionLength), block.text);
                self(self, block.children);
            }
        };
        check(check, QMarkdownPrivate::projectDocument(blocks).blocks);
    }
    void resourceRanges() {
        auto blocks = QMarkdownPrivate::parse("before ![alt](shape.png) after");
        QCOMPARE(QMarkdownPrivate::projectDocument(blocks).text, "before alt after");
        QImage image(10, 10, QImage::Format_ARGB32); image.fill(Qt::red);
        const auto projected = QMarkdownPrivate::projectImages(QMarkdownPrivate::projectDocument(blocks).blocks, {{QUrl("file:///shape.png"), image}}, QUrl("file:///"));
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
        auto *style = view->property("style").value<QObject *>(); QVERIFY(style);
        auto *table = style->property("table").value<QObject *>(); QVERIFY(table);
        QSignalSpy copies(view, SIGNAL(copyRequested(QString)));
        for (int mode : {0, 1, 2, 3}) {
            table->setProperty("borderMode", mode);
            table->setProperty("borderWidth", 2);
            table->setProperty("alternateRowBackground", QColor(Qt::yellow));
            view->setWidth(100 + mode * 30);
            QCoreApplication::processEvents();
            QCOMPARE(view->property("selectedText").toString(), "left\tright\nbody\t");
            QVERIFY(QMetaObject::invokeMethod(view, "requestCopy"));
            QCOMPARE(copies.takeFirst()[0].toString(), QString("left\tright\nbody\t"));
        }

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
        // Resource completion creates the item before the containing Row/Column
        // has polished its position. Hit-test coordinates need a completed frame.
        QVERIFY(!window.grabWindow().isNull());
        const auto center = paint->mapToScene({paint->width() / 2, paint->height() / 2}).toPoint();
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, center);
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, center);
        QCOMPARE(view->property("selectedText").toString(), "alt");
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, center);
        QCOMPARE(view->property("selectedText").toString(), "before alt after");
        QVERIFY(QMetaObject::invokeMethod(view, "clearSelection"));
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
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point); QTRY_COMPARE(link.size(), 1);
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
        QTRY_COMPARE(link.size(), 1); QCOMPARE(link[0][0].toString(), QString());
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
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, a); QTRY_COMPARE(link.size(), 1);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, a);
        QTest::mouseMove(&window, b); QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, b);
        QTRY_COMPARE(link.size(), 1); QCOMPARE(view->property("selectedText").toString(), "irst block\n\nsecond");
        QTest::keyClick(&window, Qt::Key_C, Qt::ControlModifier);
        QCOMPARE(copy.size(), 1); QCOMPARE(copy[0][0].toString(), view->property("selectedText").toString());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::ShiftModifier, position(text[0], 4, window.contentItem()));
        QCOMPARE(view->property("selectedText").toString(), "irs"); QTRY_COMPARE(link.size(), 1);
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
