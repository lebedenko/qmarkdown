#include "private/document.h"
#include "private/mathitem.h"
#include "private/viewstate.h"
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QProcess>
#include <QWheelEvent>
#include <QPointingDevice>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#include <limits>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
#ifdef QMARKDOWN_MATH_TEST_BACKEND
static void removeMathResources() { Q_CLEANUP_RESOURCE(math_resources); }
#endif
using namespace QMarkdownPrivate;
class MathTest : public QObject {
    Q_OBJECT
    static QList<MathItem *> formulas(QQuickItem *root) {
        QList<MathItem *> found;
        for (auto child : root->childItems()) {
            if (auto formula = qobject_cast<MathItem *>(child)) found.append(formula);
            found.append(formulas(child));
        }
        return found;
    }
private slots:
    void parser_data() {
        QTest::addColumn<QString>("source"); QTest::addColumn<QString>("body");
        QTest::newRow("simple") << "$$\nx^2\n$$\n" << "x^2\n";
        QTest::newRow("whitespace") << "   $$  \n\\frac{a}{b}\n  $$\t\n" << "\\frac{a}{b}\n";
        QTest::newRow("blank") << "$$\na\n\nb\n$$" << "a\n\nb\n";
        QTest::newRow("quote") << "> $$\n> x^2\n> $$\n" << "x^2\n";
        QTest::newRow("list") << "- $$\n  x^2\n  $$\n" << "x^2\n";
        QTest::newRow("list-blank-lines") << "- $$\n  a\n\n\n  b\n  $$\n" << "a\n\n\nb\n";
        QTest::newRow("nested") << "> - $$\n>   x^2\n>   $$\n" << "x^2\n";
    }
    void parser() {
        QFETCH(QString, source); QFETCH(QString, body);
        ParseOptions options; options.math = true;
        auto blocks = parse(source, options);
        while (blocks.size() == 1 && !blocks[0].children.isEmpty()) blocks = blocks[0].children;
        QCOMPARE(blocks.size(), 1); QCOMPARE(blocks[0].kind, BlockKind::MathBlock); QCOMPARE(blocks[0].text, body);
        ViewState view; view.setMathEnabled(true); view.setMarkdown(source);
        view.setSearchActive(true); view.setSearchQuery(body.trimmed());
        QVERIFY(view.searchResultCount() > 0);
    }
    void protectedSyntax() {
        const QStringList ordinary = {"$$\nx", "paragraph\n$$\nx\n$$", "$$x$$", "$x$", "\\[x\\]", "    $$\n    x\n    $$", "```\n$$\nx\n$$\n```", "<pre>\n$$\nx\n$$\n</pre>", "> $$\n> x\n\n$$"};
        ParseOptions options; options.math = true;
        for (const auto &source : ordinary) {
            const auto blocks = parse(source, options);
            const auto contains = [](auto &&self, const QVector<Block> &values) -> bool {
                for (const auto &b : values) if (b.kind == BlockKind::MathBlock || self(self, b.children)) return true;
                return false;
            };
            QVERIFY2(!contains(contains, blocks), qPrintable(source));
        }
        QCOMPARE(parse("$$\nx\n$$")[0].kind, BlockKind::Paragraph);
    }
    void formulas_data() {
        QTest::addColumn<QString>("source");
        for (const auto &s : QStringList{"x^2+y_1", "\\frac{1}{2}", "\\sqrt{x}", "\\alpha+\\Gamma", "\\sum_{i=0}^{n}i", "\\int_0^1 x dx", "\\left(\\frac{a}{b}\\right)", "\\begin{pmatrix}a&b\\\\c&d\\end{pmatrix}", "\\begin{aligned}a&=b\\\\c&=d\\end{aligned}"}) QTest::newRow(qPrintable(s)) << s;
    }
    void formulas() {
        QFETCH(QString, source);
        MathItem item; item.setSource(source); item.setMathEnabled(true);
        if (!mathAvailable()) { QVERIFY(!item.ready()); return; }
        QTRY_VERIFY_WITH_TIMEOUT(item.ready(), 5000);
        QVERIFY(item.implicitWidth() > 0); QVERIFY(item.implicitHeight() > 0);
        item.setColor(Qt::red); QTRY_VERIFY(item.ready());
        const auto width = item.implicitWidth(); item.setTextSize(32); QTRY_VERIFY(item.ready()); QVERIFY(item.implicitWidth() > width);
    }
    void rejectedAndLifetime() {
        for (const auto &source : QStringList{"\\unknown{x}", "\\newcommand{\\a}{x}\\a", "\\includegraphics{a}", "{x", "x}", QString(17000, u'x'), QString(65, u'{') + "x" + QString(65, u'}')}) {
            MathItem item; item.setSource(source); item.setMathEnabled(true); QVERIFY(!item.ready());
        }
        for (int i = 0; i < 100; ++i) { auto item = std::make_unique<MathItem>(); item->setSource("\\frac{x}{y}"); item->setMathEnabled(true); }
        MathItem item; item.setSource("x"); item.setMathEnabled(true); item.setSource("\\unknown");
        QTest::qWait(100); QVERIFY(!item.ready());
        MathItem other; other.setSource("x"); other.setMathEnabled(true);
        if (mathAvailable()) QTRY_VERIFY(other.ready());
    }
    void backendFailures() {
        if (!mathAvailable()) return;
        for (const auto &source : QStringList{"\\frac{1}", "\\left(x", "x^{", "\\begin{matrix}a&b\\end{aligned}"}) {
            MathItem item; item.setSource(source);
            QSignalSpy completed(&item, &MathItem::resultChanged);
            item.setMathEnabled(true);
            // Preflight may reject immediately; backend failures emit again on completion.
            QTRY_VERIFY(!item.pending());
            QVERIFY2(!item.ready(), qPrintable(source));
        }
        MathItem bounded;
        bounded.setSource(QString(6000, u'x')); bounded.setMathEnabled(true);
        QTRY_VERIFY(!bounded.pending()); QVERIFY(!bounded.ready());
    }
    void sourceProjectionAndExtensions() {
        for (int mask = 0; mask < 32; ++mask) {
            ParseOptions opts{bool(mask&1), bool(mask&2), bool(mask&4), bool(mask&8), bool(mask&16), true};
            const auto blocks = parse("$$\n*raw* & \\alpha\n$$\n\n- [x] task\n", opts);
            QCOMPARE(blocks[0].kind, BlockKind::MathBlock);
            QCOMPARE(blocks[0].text, "*raw* & \\alpha\n");
            QVERIFY(blocks[0].inlines.isEmpty());
        }
        ViewState state; state.setMathEnabled(true); state.setMarkdown("$$\nx^2\n$$\n\n```math\ny_1\n```\n");
        state.setSelectionEnabled(true); state.selectAll();
        QCOMPARE(state.selectedText(), "x^2\n\ny_1\n");
        state.setSearchActive(true); state.setSearchQuery("$$"); QCOMPARE(state.searchResultCount(), 0);
        state.setSearchQuery("x^2"); QCOMPARE(state.searchResultCount(), 1);
        state.setMathEnabled(false); state.setSearchQuery("$$"); QCOMPARE(state.searchResultCount(), 2);
    }
    void customPrecedenceAndOverflow() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(
import QtQuick
import QMarkdown
MarkdownView {
    width: 80; mathEnabled: true
    markdown: "```math\nx+x+x+x+x+x+x+x+x+x+x+x+x+x+x\n```"
    fencedBlockRenderers: ({math: custom})
    property Component custom: Component { Item {
        required property string codeText; required property string infoString
        required property string language; required property var markdownStyle
        implicitHeight: 23
    } }
}
)", QUrl());
        std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
        QVERIFY2(view, qPrintable(component.errorString()));
        QTRY_COMPARE(formulas(view.get()).size(), 1);
        QVERIFY(!formulas(view.get())[0]->mathEnabled());
        view->setProperty("fencedBlockRenderers", QVariantMap{});
        if (!mathAvailable()) return;
        auto item = formulas(view.get())[0]; QTRY_VERIFY(item->ready());
        QVERIFY(item->implicitWidth() > view->width());
        auto viewport = item->parentItem()->parentItem();
        QVERIFY(viewport->property("contentWidth").toReal() > viewport->width());
        QVERIFY(viewport->setProperty("contentX", 20)); QCOMPARE(viewport->property("contentX").toReal(), 20);
    }
    void capture() {
        if (!mathAvailable()) return;
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(
import QtQuick
import QMarkdown
MarkdownView {
    width: 480; mathEnabled: true
    style: MarkdownStyle { bodyFont: Qt.font({pixelSize: 24}) }
    markdown: "$$\n\\frac{1}{2}+\\sqrt{x^2+y^2}\n$$\n\n$$\n\\sum_{i=0}^{n}i + \\int_0^1 x dx\n$$\n\n$$\n\\begin{pmatrix}a&b\\\\c&d\\end{pmatrix}\n$$\n\n$$\n\\begin{aligned}a&=b+c\\\\d&=e+f\\end{aligned}\n$$"
}
)", QUrl());
        QQuickWindow window; window.setColor(Qt::white); window.resize(500, 500);
        std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
        QVERIFY2(view, qPrintable(component.errorString())); view->setParentItem(window.contentItem());
        window.show(); QTest::qWait(50);
        QTRY_COMPARE(formulas(view.get()).size(), 4);
        for (auto f : formulas(view.get())) QTRY_VERIFY(f->ready());
        QTest::qWait(50);
        const auto image = window.grabWindow(); QVERIFY(!image.isNull());
        int ink = 0;
        for (int y = 0; y < image.height(); ++y) for (int x = 0; x < image.width(); ++x)
            if (image.pixelColor(x,y).lightness() < 180) ++ink;
        // Bundled fonts and a fixed 24 px fixture: the former QPicture text
        // replay lost entire glyphs and produced fewer than 1000 ink pixels at 1x.
        const auto scale = window.devicePixelRatio();
        QVERIFY(ink > 1500 * scale * scale);
        for (auto f : formulas(view.get())) { QVERIFY(f->antialiasing()); QVERIFY(!f->smooth()); }
        const auto destination = qEnvironmentVariable("QMARKDOWN_MATH_CAPTURE");
        if (!destination.isEmpty()) QVERIFY(image.save(destination));
    }
    void wheelHandoff_data() {
        QTest::addColumn<bool>("pixel");
        QTest::newRow("wheel") << false; QTest::newRow("trackpad") << true;
    }
    void wheelHandoff() {
        QFETCH(bool, pixel);
        if (!mathAvailable()) return;
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(
import QtQuick
import QMarkdown
Flickable {
    width: 120; height: 300; contentWidth: width; contentHeight: 1500
    MarkdownView { width: 120; mathEnabled: true; markdown: "```math\n" + "x+".repeat(60) + "y\n```" }
}
)", QUrl());
        std::unique_ptr<QQuickItem> outer(qobject_cast<QQuickItem *>(component.create()));
        QVERIFY2(outer, qPrintable(component.errorString()));
        QQuickWindow window; window.resize(120,300); outer->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QTRY_COMPARE(formulas(outer.get()).size(), 1);
        auto formula = formulas(outer.get())[0]; QTRY_VERIFY(formula->ready());
        auto viewport = formula->parentItem()->parentItem();
        QTRY_VERIFY(viewport->property("contentWidth").toReal() > viewport->width());
        const auto point = viewport->mapToScene(QPointF(20, viewport->height()/2));
        QPointingDevice trackpad("math touchpad", 42, QInputDevice::DeviceType::TouchPad,
            QPointingDevice::PointerType::Finger, QInputDevice::Capability::Position | QInputDevice::Capability::Scroll, 1, 0);
        ulong timestamp = 100;
        const auto wheel = [&](Qt::KeyboardModifiers modifiers, Qt::ScrollPhase phase) {
            QWheelEvent event(point, window.mapToGlobal(point.toPoint()), pixel ? QPoint(0,-40) : QPoint(), QPoint(0,-120),
                Qt::NoButton, modifiers, phase, false,
                pixel ? Qt::MouseEventSynthesizedBySystem : Qt::MouseEventNotSynthesized,
                pixel ? &trackpad : QPointingDevice::primaryPointingDevice());
            event.setTimestamp(timestamp += 16); QCoreApplication::sendEvent(&window, &event);
        };
        if (pixel) wheel(Qt::ShiftModifier, Qt::ScrollBegin);
        wheel(Qt::ShiftModifier, pixel ? Qt::ScrollUpdate : Qt::NoScrollPhase);
        QTRY_VERIFY(viewport->property("contentX").toReal() > 0);
        QCOMPARE(outer->property("contentY").toReal(), 0.0);
        wheel(Qt::NoModifier, pixel ? Qt::ScrollUpdate : Qt::NoScrollPhase);
        if (pixel) wheel(Qt::NoModifier, Qt::ScrollUpdate);
        QTRY_VERIFY(outer->property("contentY").toReal() > 0);
    }
    void missingResources() {
#ifdef QMARKDOWN_MATH_TEST_BACKEND
        if (qEnvironmentVariableIsSet("QMARKDOWN_MATH_MISSING_CHILD")) {
            removeMathResources();
            MathItem item; item.setSource("x"); item.setMathEnabled(true);
            QVERIFY(!item.ready()); QVERIFY(!item.pending());
            return;
        }
        QProcess process;
        auto environment = QProcessEnvironment::systemEnvironment();
        environment.insert("QMARKDOWN_MATH_MISSING_CHILD", "1"); process.setProcessEnvironment(environment);
        process.start(QCoreApplication::applicationFilePath(), {"missingResources"});
        QVERIFY(process.waitForFinished(10000));
        QVERIFY2(process.exitCode() == 0, process.readAllStandardOutput().constData());
#endif
    }
    void mathStyle() {
        MarkdownStyle style;
        QCOMPARE(style.math()->scale(), 1.0); QCOMPARE(style.math()->color(), style.bodyColor());
        style.setBodyColor(Qt::red); QCOMPARE(style.math()->color(), QColor(Qt::red));
        style.math()->setColor(Qt::blue); style.setBodyColor(Qt::green); QCOMPARE(style.math()->color(), QColor(Qt::blue));
        style.math()->resetColor(); QCOMPARE(style.math()->color(), QColor(Qt::green));
        style.math()->setScale(2); QCOMPARE(style.math()->scale(), 2.0);
        style.math()->setScale(-1); QCOMPARE(style.math()->scale(), 1.0);
        style.math()->setScale(std::numeric_limits<qreal>::quiet_NaN()); QCOMPARE(style.math()->scale(), 1.0);
        style.math()->setScale(2); style.resetOverrides(); QCOMPARE(style.math()->scale(), 1.0);
    }
    void qmlModes() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(
import QtQuick
import QMarkdown
MarkdownView { width: 120; markdown: "```math\n\\frac{1}{2}\n```\n\n$$\nx^2\n$$" }
)", QUrl());
        std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create())); QVERIFY2(view, qPrintable(component.errorString()));
        QCOMPARE(view->property("mathEnabled").toBool(), false); QCOMPARE(view->property("mathAvailable").toBool(), mathAvailable());
        view->setProperty("mathEnabled", true);
        QTRY_COMPARE(formulas(view.get()).size(), 2);
        auto items = formulas(view.get());
        if (mathAvailable()) for (auto f : items) QTRY_VERIFY(f->ready());
        view->setProperty("selectionEnabled", true);
        for (auto f : formulas(view.get())) QVERIFY(!f->ready());
        view->setProperty("selectionEnabled", false);
        if (mathAvailable()) for (auto f : formulas(view.get())) QTRY_VERIFY(f->ready());
        view->setProperty("searchActive", true);
        for (auto f : formulas(view.get())) QVERIFY(!f->ready());
    }
};
QTEST_MAIN(MathTest)
#include "tst_math.moc"
