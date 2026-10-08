#include "private/formattedtext.h"
#include "private/inline.h"
#include <QGuiApplication>
#include <QFontMetricsF>
#include <QFontDatabase>
#include <QSignalSpy>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWindow>
#include <QNetworkAccessManager>
#include <QQmlNetworkAccessManagerFactory>
#include <QtQml/QQmlExtensionPlugin>
#include <QtTest/QTest>
#include <memory>
#include <limits>
#include <cmath>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

class RequestManager : public QNetworkAccessManager
{
public:
    RequestManager(int *requests, QObject *parent) : QNetworkAccessManager(parent), m_requests(requests) {}
protected:
    QNetworkReply *createRequest(Operation operation, const QNetworkRequest &request, QIODevice *data) override
    {
        ++*m_requests;
        return QNetworkAccessManager::createRequest(operation, request, data);
    }
private:
    int *m_requests;
};
class RequestFactory : public QQmlNetworkAccessManagerFactory
{
public:
    int requests = 0;
    QNetworkAccessManager *create(QObject *parent) override { return new RequestManager(&requests, parent); }
};

// The test host observes rendered Text items rather than private model identities.
class ViewTest : public QObject
{
    Q_OBJECT
    static QList<QQuickItem *> texts(QQuickItem *item)
    {
        QList<QQuickItem *> result;
        for (auto *child : item->childItems()) {
            if (child->metaObject()->indexOfProperty("textFormat") >= 0) result.append(child);
            result.append(texts(child));
        }
        return result;
    }
    static QList<FormattedText *> painted(QQuickItem *item)
    {
        QList<FormattedText *> result;
        for (auto *child : item->childItems()) {
            if (auto *text = qobject_cast<FormattedText *>(child)) result.append(text);
            result.append(painted(child));
        }
        return result;
    }
    static double content(QQuickItem *view) { return view->property("contentHeight").toDouble(); }
private slots:
    void init() { QTest::failOnWarning(); }
    void leafLayoutAndStyle()
    {
        RequestFactory factory;
        QQmlEngine engine; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\nimport QMarkdown 0.4\n"
            "Item { MarkdownStyle { id: shared; objectName: 'shared' }"
            " MarkdownStyle { id: other; objectName: 'other'; thematicBreakThickness: 9 }"
            " MarkdownView { objectName: 'a'; width: 240; style: shared }"
            " MarkdownView { objectName: 'b'; width: 240; style: shared } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *a = host->findChild<QQuickItem *>("a"); auto *b = host->findChild<QQuickItem *>("b");
        auto *shared = host->findChild<QObject *>("shared"); auto *other = host->findChild<QObject *>("other");
        QVERIFY(a && b && shared && other);
        const QString source = "Title\n===\n*Second*\n---\n***\n    <b> &amp; [x](https://example.invalid) ![x](https://example.invalid/x)  \n\n      residual\n\n___\nafter";
        a->setProperty("markdown", source); b->setProperty("markdown", source);
        QQuickWindow window; host->setParentItem(window.contentItem()); window.resize(600, 600); window.show();
        QTRY_COMPARE(texts(a).size(), 3); QTRY_COMPARE(painted(a).size(), 1);
        QTRY_VERIFY(content(a) > 0);
        const auto children = texts(a)[0]->parentItem()->childItems();
        // Column also owns the Repeater; the six block delegates precede it.
        QList<QQuickItem *> blocks;
        for (auto *child : children) if (child->width() == a->width()) blocks.append(child);
        QCOMPARE(blocks.size(), 6);
        auto *rule = blocks[2]; auto *rule2 = blocks[4];
        QTRY_COMPARE(rule->height(), 1.0); QCOMPARE(rule->width(), 240.0);
        QCOMPARE(rule->property("color").value<QColor>(), QColor("#202020"));
        QCOMPARE(texts(a)[0]->property("text").toString(), QString("Title"));
        QCOMPARE(texts(a)[0]->property("font").value<QFont>(), shared->property("h1Font").value<QFont>());
        QCOMPARE(painted(a)[0]->text(), QString("Second"));
        QCOMPARE(painted(a)[0]->font(), shared->property("h2Font").value<QFont>());
        auto *code = texts(a)[1];
        QCOMPARE(code->property("text").toString(), QString("<b> &amp; [x](https://example.invalid) ![x](https://example.invalid/x)  \n\n  residual"));
        QCOMPARE(code->property("textFormat").toInt(), 0);
        QCOMPARE(code->property("font").value<QFont>(), shared->property("codeBlockFont").value<QFont>());
        for (int i = 1; i < blocks.size(); ++i)
            QTRY_COMPARE(blocks[i]->y(), blocks[i-1]->y() + blocks[i-1]->height() + 8);
        QTRY_COMPARE(content(a), blocks.last()->y() + blocks.last()->height());
        const auto initial = content(a);
        a->setWidth(90); QTRY_VERIFY(content(a) > initial); QTRY_COMPARE(rule->width(), 90.0);
        a->setWidth(240); QTRY_COMPARE(content(a), initial);
        QSignalSpy thickness(shared, SIGNAL(thematicBreakThicknessChanged()));
        QSignalSpy colors(shared, SIGNAL(thematicBreakColorChanged()));
        QVERIFY(thickness.isValid() && colors.isValid());
        shared->setProperty("thematicBreakThickness", 4.5);
        shared->setProperty("thematicBreakColor", QColor("#123456"));
        QTRY_COMPARE(rule->height(), 4.5); QTRY_COMPARE(rule2->height(), 4.5);
        QTRY_COMPARE(content(a), initial + 7); QTRY_COMPARE(content(b), content(a));
        QTRY_COMPARE(rule->property("color").value<QColor>(), QColor("#123456"));
        shared->setProperty("thematicBreakThickness", 4.5);
        shared->setProperty("thematicBreakColor", QColor("#123456"));
        QCOMPARE(thickness.count(), 1); QCOMPARE(colors.count(), 1);
        for (const qreal value : {qreal(-1), qreal(0), std::numeric_limits<qreal>::infinity(),
                                 -std::numeric_limits<qreal>::infinity(), std::numeric_limits<qreal>::quiet_NaN()}) {
            shared->setProperty("thematicBreakThickness", value);
            QTRY_COMPARE(rule->height(), std::isfinite(value) ? 0.0 : 1.0);
            QTRY_COMPARE(rule2->height(), rule->height());
            if (rule->height() > 0) {
                for (int i = 1; i < blocks.size(); ++i)
                    QTRY_COMPARE(blocks[i]->y(), blocks[i-1]->y() + blocks[i-1]->height() + 8);
                QTRY_COMPARE(content(a), initial);
            } else {
                QTRY_COMPARE(blocks[3]->y(), blocks[1]->y() + blocks[1]->height() + 8);
                QTRY_COMPARE(blocks[5]->y(), blocks[3]->y() + blocks[3]->height() + 8);
                QTRY_COMPARE(content(a), initial - 18);
            }
        }
        const int notifications = thickness.count();
        shared->setProperty("thematicBreakThickness", std::numeric_limits<qreal>::quiet_NaN());
        QCOMPARE(thickness.count(), notifications);
        a->setProperty("style", QVariant::fromValue(other)); QTRY_COMPARE(rule->height(), 9.0);
        shared->setProperty("thematicBreakThickness", 2); QTRY_COMPARE(rule->height(), 9.0);
        delete other; QTRY_COMPARE(rule->height(), 1.0);
        QTRY_COMPARE(rule->property("color").value<QColor>(), QColor("#202020"));
        auto *defaults = a->property("style").value<QObject *>();
        defaults->setProperty("thematicBreakThickness", 7);
        defaults->setProperty("thematicBreakColor", QColor(Qt::red));
        QTRY_COMPARE(rule->height(), 7.0);
        QVERIFY(QQmlProperty(a, "style", &engine).reset());
        QTRY_COMPARE(rule->height(), 1.0);
        QTRY_COMPARE(rule->property("color").value<QColor>(), QColor("#202020"));
        QTRY_COMPARE(content(a), initial);
        QSignalSpy activation(code, SIGNAL(linkActivated(QString))); QVERIFY(activation.isValid());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, code->mapToScene(QPointF(20, 10)).toPoint());
        QCOMPARE(activation.count(), 0); QCOMPARE(factory.requests, 0);
        for (const int width : {0, -1}) {
            a->setWidth(width); QTRY_COMPARE(content(a), 0.0);
            QTRY_VERIFY(texts(a).isEmpty());
            a->setWidth(240); QTRY_COMPARE(content(a), initial);
        }
        a->setWidth(0); QTRY_COMPARE(content(a), 0.0);
        a->setProperty("markdown", "---"); a->setWidth(240); QTRY_COMPARE(content(a), 1.0);
        QTRY_VERIFY(texts(a).isEmpty()); QTRY_VERIFY(painted(a).isEmpty());
        a->setProperty("markdown", "replacement"); QTRY_COMPARE(texts(a).size(), 1);
        QTRY_COMPARE(texts(a)[0]->property("text").toString(), QString("replacement"));
        a->setProperty("markdown", ""); QTRY_COMPARE(content(a), 0.0);
        QCOMPARE(factory.requests, 0);
    }
    void fencedLayoutAndStyle()
    {
        RequestFactory factory;
        QQmlEngine engine; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\nimport QMarkdown 0.4\n"
            "Item { MarkdownStyle { id: shared; objectName: 'shared' }"
            " MarkdownStyle { id: other; objectName: 'other'; codeBlockFont.pixelSize: 40 }"
            " MarkdownView { objectName: 'a'; width: 240; style: shared }"
            " MarkdownView { objectName: 'b'; width: 240; style: shared }"
            " function editCode() { shared.codeBlockFont.pixelSize = 30; shared.codeBlockColor = '#123456' } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *a = host->findChild<QQuickItem *>("a"); auto *b = host->findChild<QQuickItem *>("b");
        auto *shared = host->findChild<QObject *>("shared"); auto *other = host->findChild<QObject *>("other");
        QVERIFY(a && b && shared && other);
        const QString source = "before\n``` metadata\n<b>*literal* &amp; [x](https://example.invalid) ![x](https://example.invalid/x)\n```\n~~~\n~~~\n```\n\n\n```\n# after";
        a->setProperty("markdown", source); b->setProperty("markdown", source);
        QQuickWindow window; host->setParentItem(window.contentItem()); window.resize(600,600); window.show();
        QTRY_COMPARE(texts(a).size(), 5); QTRY_VERIFY(content(a) > 0);
        auto items = texts(a);
        QCOMPARE(items[1]->property("text").toString(), QString("<b>*literal* &amp; [x](https://example.invalid) ![x](https://example.invalid/x)"));
        QCOMPARE(items[1]->property("textFormat").toInt(), 0);
        QVERIFY(painted(a).isEmpty());
        const auto font = shared->property("codeBlockFont").value<QFont>();
        QCOMPARE(font.pixelSize(), 16); QCOMPARE(font.weight(), QFont::Normal);
        QCOMPARE(font.family(), QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
        QCOMPARE(shared->property("codeBlockColor").value<QColor>(), QColor("#202020"));
        QTRY_COMPARE(items[2]->height(), QFontMetricsF(font).height());
        QTRY_VERIFY(qAbs(items[3]->height() - 2 * items[2]->height()) < 2);
        for (int i = 1; i < items.size(); ++i) QTRY_COMPARE(items[i]->y(), items[i-1]->y() + items[i-1]->height() + 8);
        QFont body = shared->property("bodyFont").value<QFont>();
        QFont inlineFont = shared->property("inlineCodeFont").value<QFont>();
        QFont independent = body; independent.setPixelSize(48);
        shared->setProperty("bodyFont", independent); shared->setProperty("inlineCodeFont", independent);
        QTRY_COMPARE(items[0]->property("font").value<QFont>().pixelSize(), 48);
        QCOMPARE(items[1]->property("font").value<QFont>(), font);
        shared->setProperty("bodyFont", body); shared->setProperty("inlineCodeFont", inlineFont);
        QTRY_COMPARE(items[0]->property("font").value<QFont>(), body);
        QSignalSpy activation(items[1], SIGNAL(linkActivated(QString)));
        QVERIFY(activation.isValid());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(30, 40));
        QCOMPARE(activation.count(), 0);
        const auto initial = content(a);
        a->setWidth(90); QTRY_VERIFY(content(a) > initial);
        QTRY_VERIFY(texts(a)[1]->property("contentWidth").toDouble() <= 90);
        a->setWidth(240); QTRY_COMPARE(content(a), initial);
        QSignalSpy fonts(shared, SIGNAL(codeBlockFontChanged()));
        QSignalSpy colors(shared, SIGNAL(codeBlockColorChanged()));
        QVERIFY(QMetaObject::invokeMethod(host, "editCode"));
        QTRY_VERIFY(content(a) > initial); QTRY_COMPARE(content(a), content(b));
        QCOMPARE(fonts.count(), 1); QCOMPARE(colors.count(), 1);
        QCOMPARE(shared->property("bodyFont").value<QFont>().pixelSize(), 16);
        QCOMPARE(shared->property("inlineCodeFont").value<QFont>().resolveMask(), QFont(QFontDatabase::systemFont(QFontDatabase::FixedFont).family()).resolveMask());
        QTRY_COMPARE(texts(a)[1]->property("color").value<QColor>(), QColor("#123456"));
        a->setProperty("style", QVariant::fromValue(other));
        QTRY_COMPARE(texts(a)[1]->property("font").value<QFont>().pixelSize(), 40);
        QFont changed = font; changed.setPixelSize(20); shared->setProperty("codeBlockFont", changed);
        QTRY_COMPARE(texts(b)[1]->property("font").value<QFont>().pixelSize(), 20);
        QCOMPARE(fonts.count(), 2);
        shared->setProperty("codeBlockFont", changed); QCOMPARE(fonts.count(), 2);
        QCOMPARE(texts(a)[1]->property("font").value<QFont>().pixelSize(), 40);
        delete other; QTRY_COMPARE(content(a), initial);
        delete shared; QTRY_COMPARE(content(b), initial);
        auto *defaults = a->property("style").value<QObject *>();
        defaults->setProperty("codeBlockFont", changed); defaults->setProperty("codeBlockColor", QColor(Qt::red));
        QTRY_VERIFY(content(a) > initial);
        QVERIFY(QQmlProperty(a, "style", &engine).reset()); QTRY_COMPARE(content(a), initial);
        QCOMPARE(defaults->property("codeBlockColor").value<QColor>(), QColor("#202020"));
        a->setWidth(0); QTRY_COMPARE(content(a), 0.0);
        a->setWidth(-1); QTRY_COMPARE(content(a), 0.0);
        a->setWidth(240); QTRY_COMPARE(content(a), initial);
        a->setProperty("markdown", "~~~\nreplacement\n~~~"); QTRY_COMPARE(texts(a).size(), 1);
        QTRY_COMPARE(texts(a)[0]->property("text").toString(), QString("replacement"));
        a->setProperty("markdown", ""); QTRY_COMPARE(content(a), 0.0);
        QCOMPARE(factory.requests, 0);
    }
    void inlineCodeFontOverlay()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\nimport QMarkdown\n"
            "MarkdownView { width: 200; function editCode() { style.inlineCodeFont.pixelSize = 23 } function resetCode() { style = null } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> view(component.create());
        QVERIFY(view);
        auto *style = view->property("style").value<QObject *>();
        QVERIFY(style);
        const auto initial = style->property("inlineCodeFont").value<QFont>();
        QCOMPARE(initial.family(), QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
        QFont heading;
        heading.setPixelSize(37);
        heading.setWeight(QFont::Black);
        heading.setItalic(true);
        const auto resolved = initial.resolve(heading);
        QCOMPARE(resolved.pixelSize(), 37);
        QCOMPARE(resolved.weight(), QFont::Black);
        QVERIFY(resolved.italic());
        QSignalSpy changes(style, SIGNAL(inlineCodeFontChanged()));
        QVERIFY(changes.isValid());
        QVERIFY(QMetaObject::invokeMethod(view.get(), "editCode"));
        QCOMPARE(style->property("inlineCodeFont").value<QFont>().pixelSize(), 23);
        QCOMPARE(changes.count(), 1);
        QFont custom;
        custom.setFamily("serif");
        custom.setPointSizeF(19);
        QVERIFY(style->setProperty("inlineCodeFont", custom));
        QCOMPARE(style->property("inlineCodeFont").value<QFont>(), custom);
        QCOMPARE(changes.count(), 2);
        QVERIFY(QMetaObject::invokeMethod(view.get(), "resetCode"));
        QCOMPARE(style->property("inlineCodeFont").value<QFont>(), initial);
        QCOMPARE(style->property("inlineCodeFont").value<QFont>().resolveMask(), initial.resolveMask());
        QCOMPARE(changes.count(), 3);
        QFont explicitWeight = initial;
        explicitWeight.setWeight(initial.weight());
        QVERIFY(explicitWeight.resolveMask() != initial.resolveMask());
        QVERIFY(style->setProperty("inlineCodeFont", explicitWeight));
        QCOMPARE(changes.count(), 4);
        QCOMPARE(style->property("inlineCodeFont").value<QFont>().resolve(heading).weight(), initial.weight());
    }
    void formattedFontAndLayout()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(
            "\n"
            "            import QtQuick\n"
            "            import QMarkdown 0.4\n"
            "            MarkdownView {\n"
            "                width: 260\n"
            "                markdown: \"# ***`Heading`***\\n\\n*Italic* **bold** `code with spaces` שלום é 日本語 😀 longunbrokenword\\n\\n##\"\n"
            "                function editCode() { style.inlineCodeFont.pixelSize = 45 }\n"
            "            }\n"
            "        "
            , {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window;
        window.resize(400, 500); view->setParentItem(window.contentItem()); window.show();
        QTRY_COMPARE(painted(view).size(), 2);
        auto *heading = painted(view)[0];
        auto *body = painted(view)[1];
        QTRY_VERIFY(heading->layout() && body->layout());
        QTRY_VERIFY(content(view) > 0);
        const auto headingFormat = heading->layout()->formats()[0].format.font();
        QCOMPARE(headingFormat.pixelSize(), 32);
        QVERIFY(headingFormat.italic());
        QCOMPARE(headingFormat.weight(), QFont::Bold);
        QCOMPARE(headingFormat.family(), QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
        QCOMPARE(body->layout()->formats().size(), 3);
        bool hasRtlRun = false;
        for (const auto &run : body->layout()->glyphRuns()) hasRtlRun |= run.isRightToLeft();
        QVERIFY(hasRtlRun);
        QVERIFY(!body->layout()->isValidCursorPosition(body->text().indexOf(QChar(0x301))));
        QVERIFY(body->layout()->formats()[0].format.font().italic());
        QCOMPARE(body->layout()->formats()[1].format.font().weight(), QFont::Bold);
        QCOMPARE(body->layout()->formats()[2].format.font().pixelSize(), 16);
        QCOMPARE(body->layout()->text(), QString::fromUtf8("Italic bold code with spaces שלום é 日本語 😀 longunbrokenword"));
        QTRY_COMPARE(texts(view).size(), 1);
        auto *empty = texts(view)[0];
        QTRY_COMPARE(empty->height(), QFontMetricsF(empty->property("font").value<QFont>()).height());
        QTRY_COMPARE(body->parentItem()->y(), heading->logicalHeight() + 8);
        QTRY_COMPARE(empty->y(), body->parentItem()->y() + body->logicalHeight() + 8);
        QTRY_COMPARE(content(view), empty->y() + empty->height());
        QFont heavy = body->font(); heavy.setWeight(QFont::Black);
        auto *effectiveStyle = view->property("style").value<QObject *>();
        effectiveStyle->setProperty("bodyFont", heavy);
        QTRY_COMPARE(body->layout()->formats()[1].format.font().weight(), QFont::Black);
        QVERIFY(QQmlProperty(view, "style", &engine).reset());
        QTRY_COMPARE(body->layout()->formats()[1].format.font().weight(), QFont::Bold);
        const auto wide = content(view);
        view->setWidth(90); QTRY_VERIFY(content(view) > wide);
        view->setWidth(260); QTRY_COMPARE(content(view), wide);
        QVERIFY(QMetaObject::invokeMethod(view, "editCode"));
        QTRY_COMPARE(body->layout()->formats()[2].format.font().pixelSize(), 45);
        QTRY_VERIFY(content(view) > wide);
        auto *style = view->property("style").value<QObject *>();
        QFont code = style->property("inlineCodeFont").value<QFont>();
        code.setWeight(QFont::Light); code.setItalic(false);
        style->setProperty("inlineCodeFont", code);
        QTRY_COMPARE(body->layout()->formats()[2].format.font().weight(), QFont::Light);
        QTRY_COMPARE(heading->layout()->formats()[0].format.font().weight(), QFont::Bold);
        QVERIFY(heading->layout()->formats()[0].format.font().italic());
        style->setProperty("bodyColor", QColor(Qt::transparent));
        QTRY_COMPARE(body->color(), QColor(Qt::transparent));
        style->setProperty("blockSpacing", -1);
        QTRY_COMPARE(body->parentItem()->y(), heading->logicalHeight());
        QVERIFY(QQmlProperty(view, "style", &engine).reset());
        QTRY_COMPARE(content(view), wide);
        view->setWidth(1);
        QTRY_COMPARE(body->layoutWidth(), 1.0);
        QTRY_VERIFY(body->width() > body->layoutWidth());
        QTRY_VERIFY(body->logicalHeight() > 0);
        QTRY_COMPARE(body->layout()->lineAt(0).width(), 1.0);
        // Verify native ink/cluster overhang is inside the raster item's bounds.
        for (const auto &run : body->layout()->glyphRuns()) {
            const auto bounds = run.boundingRect().translated(-body->x(), -body->y());
            QVERIFY(bounds.left() >= 0 && bounds.top() >= 0);
            QVERIFY(bounds.right() <= body->width() && bounds.bottom() <= body->height());
        }
        view->setWidth(0); QTRY_COMPARE(content(view), 0.0);
        view->setWidth(-1); QTRY_COMPARE(content(view), 0.0);
        view->setProperty("markdown", "**replacement**");
        view->setWidth(260);
        QTRY_COMPARE(painted(view).size(), 1);
        QTRY_VERIFY(painted(view)[0]->layout());
        QTRY_COMPARE(painted(view)[0]->layout()->text(), QString("replacement"));
        const auto shortHeight = content(view);
        view->setProperty("markdown", "**replacement**"); QTRY_COMPARE(content(view), shortHeight);
        view->setProperty("markdown", "plain"); QTRY_COMPARE(painted(view).size(), 0);
        QTRY_COMPARE(texts(view).size(), 1);
        view->setProperty("markdown", ""); QTRY_COMPARE(content(view), 0.0);
    }
    void codeInEveryHeading()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\nimport QMarkdown\n"
            "MarkdownView { width: 300; markdown: '# `one`\\n## `two`\\n### `three`\\n#### `four`\\n##### `five`\\n###### `six`'; "
            "function editHeading() { style.h3Font.pixelSize = 39 } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(400,400); window.show();
        QTRY_COMPARE(painted(view).size(), 6);
        QTRY_VERIFY(content(view) > 0);
        const int sizes[] = {32,28,24,20,18,16};
        for (int i = 0; i < 6; ++i) {
            QTRY_VERIFY(painted(view)[i]->layout());
            QCOMPARE(painted(view)[i]->layout()->formats()[0].format.font().pixelSize(), sizes[i]);
            QCOMPARE(painted(view)[i]->layout()->formats()[0].format.font().weight(), QFont::Bold);
        }
        QVERIFY(QMetaObject::invokeMethod(view, "editHeading"));
        QTRY_COMPARE(painted(view)[2]->layout()->formats()[0].format.font().pixelSize(), 39);
        QCOMPARE(painted(view)[1]->layout()->formats()[0].format.font().pixelSize(), 28);
    }
    void decodedLineSeparators()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown\nMarkdownView { width: 300; markdown: '*one&NewLine;two*\\n\\none&NewLine;two' }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(400,400); window.show();
        QTRY_VERIFY(content(view) > 0);
        QTRY_COMPARE(painted(view).size(), 1);
        QTRY_COMPARE(texts(view).size(), 1);
        QTRY_VERIFY(painted(view)[0]->layout());
        QCOMPARE(painted(view)[0]->text(), QString("one\ntwo"));
        QCOMPARE(texts(view)[0]->property("lineCount").toInt(), 2);
        QCOMPARE(painted(view)[0]->layout()->lineCount(), 2);
        QCOMPARE(painted(view)[0]->logicalHeight(), texts(view)[0]->height());
    }
    void formattedStyleLifecycle()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(
            "\n"
            "            import QtQuick\n"
            "            import QMarkdown\n"
            "            Item {\n"
            "                MarkdownStyle { id: shared; objectName: \"shared\" }\n"
            "                MarkdownStyle { id: other; objectName: \"other\"; inlineCodeFont.pixelSize: 40 }\n"
            "                MarkdownView { objectName: \"a\"; width: 180; markdown: \"`code`\"; style: shared }\n"
            "                MarkdownView { objectName: \"b\"; width: 180; markdown: \"`code`\"; style: shared }\n"
            "                function editShared() { shared.inlineCodeFont.pixelSize = 30 }\n"
            "            }\n"
            "        "
            , {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *a = host->findChild<QQuickItem *>("a"); auto *b = host->findChild<QQuickItem *>("b");
        auto *shared = host->findChild<QObject *>("shared"); auto *other = host->findChild<QObject *>("other");
        QVERIFY(a && b && shared && other);
        QQuickWindow window; host->setParentItem(window.contentItem()); window.resize(400,400); window.show();
        QTRY_VERIFY(content(a) > 0 && content(b) > 0);
        const auto initial = content(a);
        QVERIFY(QMetaObject::invokeMethod(host, "editShared"));
        QTRY_VERIFY(content(a) > initial); QTRY_COMPARE(content(a), content(b));
        a->setProperty("style", QVariant::fromValue(other));
        QTRY_COMPARE(painted(a)[0]->layout()->formats()[0].format.font().pixelSize(), 40);
        const auto replacementHeight = content(a);
        QFont code = shared->property("inlineCodeFont").value<QFont>(); code.setPixelSize(20);
        shared->setProperty("inlineCodeFont", code);
        QTRY_COMPARE(painted(b)[0]->layout()->formats()[0].format.font().pixelSize(), 20);
        QCOMPARE(content(a), replacementHeight);
        delete other; QTRY_COMPARE(content(a), initial);
        delete shared; QTRY_COMPARE(content(b), initial);
        auto *defaults = a->property("style").value<QObject *>();
        code.setPixelSize(50); defaults->setProperty("inlineCodeFont", code);
        QTRY_VERIFY(content(a) > initial);
        QVERIFY(QQmlProperty(a, "style", &engine).reset()); QTRY_COMPARE(content(a), initial);
    }
    void nativeLayoutAndReplacement()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\n"
            "import QMarkdown\n"
            "MarkdownView { width: 300; markdown: '# Title\\n\\nlong long long long long long long long long long\\n\\n##' }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(400, 400); view->setParentItem(window.contentItem()); window.show();
        QTRY_VERIFY(content(view) > 0);
        QTRY_COMPARE(texts(view).size(), 3);
        auto items = texts(view);
        QTRY_COMPARE(items[2]->height(), QFontMetricsF(items[2]->property("font").value<QFont>()).height());
        QTRY_COMPARE(items[1]->y(), items[0]->height() + 8);
        QTRY_COMPARE(items[2]->y(), items[1]->y() + items[1]->height() + 8);
        QTRY_COMPARE(content(view), items[2]->y() + items[2]->height());
        QCOMPARE(view->implicitWidth(), 0.0);
        QCOMPARE(view->implicitHeight(), content(view));
        const double wide = content(view);
        view->setWidth(70);
        QTRY_VERIFY(content(view) > wide);
        view->setWidth(300); QTRY_COMPARE(content(view), wide);
        view->setHeight(1); QCOMPARE(content(view), wide);
        view->setWidth(0); QTRY_COMPARE(content(view), 0.0);
        view->setWidth(-1); QTRY_COMPARE(content(view), 0.0);
        view->setProperty("markdown", "replacement");
        view->setWidth(300); QTRY_COMPARE(texts(view).size(), 1);
        QTRY_COMPARE(texts(view)[0]->property("text").toString(), QString("replacement"));
        const double shortHeight = content(view);
        view->setProperty("markdown", "replacement"); QTRY_COMPARE(content(view), shortHeight);
        view->setProperty("markdown", " \t\r\n\r\t "); QTRY_COMPARE(content(view), 0.0);
        QTRY_COMPARE(texts(view).size(), 0);
        view->setProperty("markdown", QString(QChar(0xa0))); QTRY_VERIFY(content(view) > 0);
        view->setProperty("markdown", "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
        view->setWidth(40); QTRY_VERIFY(content(view) > shortHeight);
        QTRY_VERIFY(texts(view)[0]->property("contentWidth").toDouble() <= 40);
        view->setWidth(1); QTRY_VERIFY(content(view) > 0);
    }
    void styleLifecycle()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\n"
            "import QMarkdown\n"
            "Item {\n"
            "    width: 400\n"
            "    MarkdownStyle { id: shared; objectName: \"shared\" }\n"
            "    MarkdownStyle { id: other; objectName: \"other\" }\n"
            "    MarkdownView { id: first; objectName: \"a\"; width: 250; markdown: \"# title\\n\\nbody\"; style: shared }\n"
            "    MarkdownView { objectName: \"b\"; width: 250; markdown: \"# title\\n\\nbody\"; style: shared }\n"
            "    function editFont() { shared.bodyFont.pixelSize = 40 }\n"
            "    function resetNull() { first.style = null }\n"
            "    function resetUndefined() { first.style = undefined }\n"
            "}", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *a = host->findChild<QQuickItem *>("a"); auto *b = host->findChild<QQuickItem *>("b");
        auto *shared = host->findChild<QObject *>("shared"); auto *other = host->findChild<QObject *>("other");
        QVERIFY(a && b && shared && other);
        QQuickWindow window; host->setParentItem(window.contentItem()); window.resize(400,400); window.show();
        QTRY_VERIFY(content(a) > 0);
        QCOMPARE(shared->parent(), host);
        QCOMPARE(shared->property("bodyFont").value<QFont>().pixelSize(), 16);
        QCOMPARE(shared->property("bodyFont").value<QFont>().weight(), QFont::Normal);
        const int sizes[] = {32,28,24,20,18,16};
        for (int i=1; i<=6; ++i) {
            const auto prefix = QString("h%1").arg(i);
            QCOMPARE(shared->property(qPrintable(prefix+"Font")).value<QFont>().pixelSize(), sizes[i-1]);
            QCOMPARE(shared->property(qPrintable(prefix+"Font")).value<QFont>().weight(), QFont::Bold);
            QCOMPARE(shared->property(qPrintable(prefix+"Color")).value<QColor>(), QColor("#202020"));
        }
        const double initial = content(a);
        QVERIFY(QMetaObject::invokeMethod(host, "editFont"));
        QTRY_VERIFY(content(a) > initial); QTRY_COMPARE(content(b), content(a));
        QCOMPARE(shared->property("h1Font").value<QFont>().pixelSize(), 32);
        shared->setProperty("bodyColor", QColor(Qt::transparent));
        QTRY_COMPARE(texts(a)[1]->property("color").value<QColor>(), QColor(Qt::transparent));
        shared->setProperty("blockSpacing", -5); QTRY_COMPARE(texts(a)[1]->y(), texts(a)[0]->height());
        shared->setProperty("blockSpacing", std::numeric_limits<double>::infinity());
        QTRY_COMPARE(texts(a)[1]->y(), texts(a)[0]->height()+8);
        a->setProperty("style", QVariant::fromValue(other)); QTRY_COMPARE(content(a), initial);
        shared->setProperty("blockSpacing", 90); QTRY_COMPARE(content(a), initial);
        delete other; QTRY_COMPARE(content(a), initial);
        auto *defaults = a->property("style").value<QObject *>(); QVERIFY(defaults);
        QFont font = defaults->property("bodyFont").value<QFont>(); font.setPixelSize(48);
        defaults->setProperty("bodyFont", font); QTRY_VERIFY(content(a) > initial);
        QVERIFY(QQmlProperty(a, "style", &engine).reset()); QTRY_COMPARE(content(a), initial);
        defaults->setProperty("blockSpacing", 70); QTRY_VERIFY(content(a) > initial);
        QVERIFY(QMetaObject::invokeMethod(host, "resetNull")); QTRY_COMPARE(content(a), initial);
        defaults->setProperty("blockSpacing", 70); QTRY_VERIFY(content(a) > initial);
        QVERIFY(QMetaObject::invokeMethod(host, "resetUndefined")); QTRY_COMPARE(content(a), initial);
        shared->setProperty("blockSpacing", std::numeric_limits<double>::quiet_NaN());
        QTRY_COMPARE(texts(b)[1]->y(), texts(b)[0]->height() + 8);
        QCOMPARE(texts(a)[0]->property("text").toString(), QString("title"));
    }
#ifdef QMARKDOWN_VIEWER_SOURCE
    void viewerTheme()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl::fromLocalFile(QStringLiteral(QMARKDOWN_VIEWER_SOURCE)));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *window = qobject_cast<QQuickWindow *>(object.get()); QVERIFY(window);
        auto *preview = window->findChild<QQuickItem *>("preview"); QVERIFY(preview);
        auto *editor = window->findChild<QObject *>("editor"); QVERIFY(editor);
        auto *panel = window->findChild<QObject *>("stylePanel"); QVERIFY(panel);
        auto *style = preview->property("style").value<QObject *>(); QVERIFY(style);
        const auto bodyFont = style->property("bodyFont").value<QFont>();
        const auto inlineFont = style->property("inlineCodeFont").value<QFont>();
        const auto spacing = style->property("blockSpacing");
        const QStringList roles = {"body", "h1", "h2", "h3", "h4", "h5", "h6", "codeBlock", "thematicBreak"};
        const auto color = [style](const QString &role) {
            return style->property((role + "Color").toUtf8().constData()).value<QColor>();
        };
        const auto setPalette = [window](bool dark) {
            const QColor surface(dark ? "#18212b" : "#ffffff");
            const QColor text(dark ? "#eeeeee" : "#202020");
            bool ok = true;
            for (const auto &role : {"base", "window", "button"})
                ok = QQmlProperty::write(window, QString("palette.") + role, surface) && ok;
            for (const auto &role : {"text", "windowText", "buttonText"})
                ok = QQmlProperty::write(window, QString("palette.") + role, text) && ok;
            return ok;
        };
        QVERIFY(setPalette(true));
        QTRY_COMPARE(color("body"), QColor("#eeeeee"));
        auto *surface = window->findChild<QObject *>("previewBackground"); QVERIFY(surface);
        auto *label = window->findChild<QObject *>("previewLabel"); QVERIFY(label);
        editor->setProperty("text", "# H1\n## H2\n### H3\n#### H4\n##### H5\n###### H6\n\nBody\n\n*Emphasis* and `code`\n\n```\nfenced\n```\n");
        QTRY_COMPARE(texts(preview).size(), 8);
        QTRY_COMPARE(painted(preview).size(), 1);
        for (const bool dark : {false, true}) {
            QVERIFY(setPalette(dark));
            const QColor expected(dark ? "#eeeeee" : "#202020");
            QTRY_COMPARE(surface->property("color").value<QColor>(), QColor(dark ? "#18212b" : "#ffffff"));
            QTRY_COMPARE(label->property("color").value<QColor>(), expected);
            for (const auto &role : roles) QTRY_COMPARE(color(role), expected);
            for (auto *text : texts(preview)) QTRY_COMPARE(text->property("color").value<QColor>(), expected);
            QTRY_COMPARE(painted(preview)[0]->color(), expected);
        }
        panel->setProperty("selectedRole", 0);
        QVariant accepted;
        QVERIFY(QMetaObject::invokeMethod(panel, "applyColor", Q_RETURN_ARG(QVariant, accepted), Q_ARG(QVariant, "invalid-color")));
        QVERIFY(!accepted.toBool());
        QVERIFY(setPalette(false));
        QTRY_COMPARE(color("body"), QColor("#202020"));
        QVERIFY(QMetaObject::invokeMethod(panel, "applyColor", Q_RETURN_ARG(QVariant, accepted), Q_ARG(QVariant, "#123456")));
        QVERIFY(accepted.toBool());
        QVERIFY(setPalette(true));
        QTRY_COMPARE(color("body"), QColor("#123456"));
        for (const auto &role : roles.mid(1)) QTRY_COMPARE(color(role), QColor("#eeeeee"));
        QTRY_COMPARE(painted(preview)[0]->color(), QColor("#123456"));
        panel->setProperty("selectedRole", 7);
        QVERIFY(QMetaObject::invokeMethod(panel, "editFont", Q_ARG(QVariant, "pixelSize"), Q_ARG(QVariant, 25)));
        style->setProperty("blockSpacing", 30);
        QVERIFY(QMetaObject::invokeMethod(window, "resetStyle"));
        QCOMPARE(preview->property("style").value<QObject *>(), style);
        QCOMPARE(style->property("bodyFont").value<QFont>(), bodyFont);
        QCOMPARE(style->property("inlineCodeFont").value<QFont>().resolveMask(), inlineFont.resolveMask());
        QCOMPARE(style->property("blockSpacing"), spacing);
        QVERIFY(setPalette(false));
        for (const auto &role : roles) QTRY_COMPARE(color(role), QColor("#202020"));
        QVERIFY(QMetaObject::invokeMethod(window, "switchStyle"));
        QTRY_COMPARE(color("body"), QColor("#194c39"));
        QTRY_COMPARE(color("h1"), QColor("#743a86"));
        QTRY_COMPARE(color("codeBlock"), QColor("#305b9c"));
        QTRY_COMPARE(color("thematicBreak"), QColor("#743a86"));
        QCOMPARE(style->property("thematicBreakThickness").toDouble(), 3.0);
        QVERIFY(setPalette(true));
        QTRY_COMPARE(color("body"), QColor("#82cba7"));
        for (int i = 1; i <= 6; ++i) QTRY_COMPARE(color("h" + QString::number(i)), QColor("#d8a0e5"));
        QTRY_COMPARE(color("codeBlock"), QColor("#8db9f2"));
        QTRY_COMPARE(color("thematicBreak"), QColor("#d8a0e5"));
        panel->setProperty("selectedRole", 1);
        QVERIFY(QMetaObject::invokeMethod(panel, "applyColor", Q_RETURN_ARG(QVariant, accepted), Q_ARG(QVariant, "#abcdef")));
        QVERIFY(accepted.toBool());
        QVERIFY(setPalette(false));
        QTRY_COMPARE(color("h1"), QColor("#abcdef"));
        QTRY_COMPARE(color("h2"), QColor("#743a86"));
        QVERIFY(QMetaObject::invokeMethod(panel, "applyPreset", Q_ARG(QVariant, false)));
        QVERIFY(setPalette(true));
        for (const auto &role : roles) QTRY_COMPARE(color(role), QColor("#eeeeee"));
        QCOMPARE(style->property("bodyFont").value<QFont>(), bodyFont);
        QCOMPARE(style->property("blockSpacing"), spacing);
        QVERIFY(QMetaObject::invokeMethod(panel, "applyRuleColor", Q_RETURN_ARG(QVariant, accepted), Q_ARG(QVariant, "invalid-color")));
        QVERIFY(!accepted.toBool());
        QVERIFY(QMetaObject::invokeMethod(panel, "applyRuleColor", Q_RETURN_ARG(QVariant, accepted), Q_ARG(QVariant, "#456789")));
        QVERIFY(accepted.toBool());
        style->setProperty("thematicBreakThickness", 6);
        QVERIFY(setPalette(false)); QTRY_COMPARE(color("thematicBreak"), QColor("#456789"));
        QVERIFY(setPalette(true)); QTRY_COMPARE(color("thematicBreak"), QColor("#456789"));
        QCOMPARE(style->property("thematicBreakThickness").toDouble(), 6.0);
        QVERIFY(QMetaObject::invokeMethod(window, "resetStyle"));
        QTRY_COMPARE(color("thematicBreak"), QColor("#eeeeee"));
        QCOMPARE(style->property("thematicBreakThickness").toDouble(), 1.0);
        if (qEnvironmentVariableIsSet("QMARKDOWN_CAPTURE_THEME")) {
            window->setProperty("sampleIndex", 6);
            window->findChild<QObject *>("styleToggle")->setProperty("checked", true);
            QVERIFY(QMetaObject::invokeMethod(window, "loadSample"));
            for (const bool dark : {false, true}) {
                QVERIFY(setPalette(dark));
                for (const bool alternate : {false, true}) {
                    QVERIFY(QMetaObject::invokeMethod(panel, "applyPreset", Q_ARG(QVariant, alternate)));
                    QSignalSpy frames(window, &QQuickWindow::frameSwapped);
                    window->update();
                    QTRY_VERIFY(!frames.isEmpty());
                    const auto path = qEnvironmentVariable("QMARKDOWN_CAPTURE_THEME")
                        + (dark ? ".dark" : ".light") + (alternate ? ".alternate.png" : ".neutral.png");
                    QVERIFY(window->grabWindow().save(path));
                }
            }
        }
    }
    void viewer()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl::fromLocalFile(QStringLiteral(QMARKDOWN_VIEWER_SOURCE)));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *window = qobject_cast<QQuickWindow *>(object.get()); QVERIFY(window);
        auto *preview = window->findChild<QQuickItem *>("preview"); QVERIFY(preview);
        auto *editor = window->findChild<QObject *>("editor"); QVERIFY(editor);
        QTRY_VERIFY(content(preview) > 0);
        window->setHeight(600);
        QTRY_VERIFY(content(preview) > 0);
        window->setHeight(700);
        const double initial = content(preview);
        QVERIFY(QMetaObject::invokeMethod(window, "switchStyle"));
        QTRY_VERIFY(content(preview) > initial);
        QVERIFY(QMetaObject::invokeMethod(window, "resetStyle"));
        QTRY_COMPARE(content(preview), initial);
        QVERIFY(QMetaObject::invokeMethod(window, "clearSource"));
        QTRY_COMPARE(content(preview), 0.0);
        QVERIFY(QMetaObject::invokeMethod(window, "loadSample"));
        QTRY_COMPARE(content(preview), initial);
        editor->setProperty("text", "# Edited\n\nLive preview");
        QTRY_COMPARE(texts(preview).size(), 2);
        QTRY_COMPARE(texts(preview)[1]->property("text").toString(), QString("Live preview"));
        QVERIFY(QMetaObject::invokeMethod(window, "loadSample"));
        QTRY_COMPARE(content(preview), initial);
        window->setWidth(800);
        QTRY_VERIFY(content(preview) > initial);
        window->setWidth(1000);
        QTRY_COMPARE(content(preview), initial);
        auto *panel = window->findChild<QObject *>("stylePanel"); QVERIFY(panel);
        auto *style = preview->property("style").value<QObject *>(); QVERIFY(style);
        auto *sourceScroll = window->findChild<QQuickItem *>("sourceScroll"); QVERIFY(sourceScroll);
        auto *previewScroll = window->findChild<QQuickItem *>("previewScroll"); QVERIFY(previewScroll);
        for (int sample = 0; sample < 7; ++sample) {
            window->setProperty("sampleIndex", sample);
            QTRY_COMPARE(editor->property("text"), window->property("sample"));
            QVERIFY(!editor->property("text").toString().isEmpty());
            QTRY_COMPARE(sourceScroll->property("contentY").toDouble(), 0.0);
            QTRY_COMPARE(previewScroll->property("contentY").toDouble(), 0.0);
        }
        window->setProperty("sampleIndex", 4);
        QTRY_VERIFY(content(preview) > previewScroll->height());
        const auto wideHeight = content(preview);
        window->setProperty("previewWidth", 240);
        QTRY_COMPARE(preview->width(), 240.0);
        QTRY_VERIFY(content(preview) > wideHeight);
        window->setProperty("previewWidth", 720);
        QTRY_COMPARE(preview->width(), previewScroll->width());
        window->setProperty("previewWidth", 0);
        sourceScroll->setProperty("contentY", 30);
        previewScroll->setProperty("contentY", 30);
        QVERIFY(QMetaObject::invokeMethod(window, "loadSample"));
        QTRY_COMPARE(sourceScroll->property("contentY").toDouble(), 0.0);
        QTRY_COMPARE(previewScroll->property("contentY").toDouble(), 0.0);
        panel->setProperty("selectedRole", 7);
        const auto inlineFont = style->property("inlineCodeFont").value<QFont>();
        QVERIFY(QMetaObject::invokeMethod(panel, "editFont", Q_ARG(QVariant, "family"), Q_ARG(QVariant, "monospace")));
        QCOMPARE(style->property("inlineCodeFont").value<QFont>().pixelSize(), inlineFont.pixelSize());
        QVERIFY(QMetaObject::invokeMethod(panel, "editFont", Q_ARG(QVariant, "pixelSize"), Q_ARG(QVariant, 25)));
        QCOMPARE(style->property("inlineCodeFont").value<QFont>().pixelSize(), 25);
        panel->setProperty("selectedRole", 0);
        const auto oldColor = style->property("bodyColor");
        QVariant accepted;
        QVERIFY(QMetaObject::invokeMethod(panel, "applyColor", Q_RETURN_ARG(QVariant, accepted), Q_ARG(QVariant, "invalid-color")));
        QVERIFY(!accepted.toBool());
        QCOMPARE(style->property("bodyColor"), oldColor);
        QVERIFY(QMetaObject::invokeMethod(panel, "applyColor", Q_RETURN_ARG(QVariant, accepted), Q_ARG(QVariant, "#123456")));
        QVERIFY(accepted.toBool());
        QCOMPARE(style->property("bodyColor").value<QColor>(), QColor("#123456"));
        panel->setProperty("selectedRole", 1);
        QCOMPARE(style->property("bodyColor").value<QColor>(), QColor("#123456"));
        QVERIFY(QMetaObject::invokeMethod(window, "resetStyle"));
        QCOMPARE(style->property("inlineCodeFont").value<QFont>().resolveMask(), inlineFont.resolveMask());
        QCOMPARE(preview->property("style").value<QObject *>(), style);
        window->setProperty("sampleIndex", 0);
        window->resize(800, 600);
        auto *toggle = window->findChild<QObject *>("styleToggle"); QVERIFY(toggle);
        toggle->setProperty("checked", true);
        QTRY_VERIFY(previewScroll->height() > 150);
        auto *clear = window->findChild<QQuickItem *>("clearButton"); QVERIFY(clear);
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, clear->mapToScene(QPointF(clear->width()/2, clear->height()/2)).toPoint());
        QTRY_COMPARE(content(preview), 0.0);
        auto *editItem = qobject_cast<QQuickItem *>(editor); QVERIFY(editItem);
        editItem->forceActiveFocus();
        for (const QChar ch : QString("Live keyboard edit")) QTest::keyClick(window, ch.toLatin1());
        QTRY_COMPARE(preview->property("markdown").toString(), QString("Live keyboard edit"));
        auto *reload = window->findChild<QQuickItem *>("reloadButton"); QVERIFY(reload);
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, reload->mapToScene(QPointF(reload->width()/2, reload->height()/2)).toPoint());
        QTRY_COMPARE(editor->property("text"), window->property("sample"));
        auto *alternateButton = window->findChild<QQuickItem *>("alternateButton"); QVERIFY(alternateButton);
        alternateButton->forceActiveFocus();
        QTest::keyClick(window, Qt::Key_Space);
        QTRY_COMPARE(style->property("blockSpacing").toDouble(), 16.0);
        QVERIFY(QMetaObject::invokeMethod(window, "resetStyle"));
        if (qEnvironmentVariableIsSet("QMARKDOWN_CAPTURE_VIEWER")) {
            QTest::qWait(100);
            QVERIFY(window->grabWindow().save(qEnvironmentVariable("QMARKDOWN_CAPTURE_VIEWER") + ".minimum.png"));
        }
        const auto paneWidth = sourceScroll->width();
        const QPoint divider = sourceScroll->mapToScene(QPointF(sourceScroll->width() + 10, sourceScroll->height()/2)).toPoint();
        QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, divider);
        QTest::mouseMove(window, divider + QPoint(60, 0), 30);
        QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, divider + QPoint(60, 0));
        QTRY_VERIFY(sourceScroll->width() != paneWidth);
        toggle->setProperty("checked", false);
        window->resize(1000, 700);
        QVERIFY(QMetaObject::invokeMethod(window, "loadSample"));
        if (qEnvironmentVariableIsSet("QMARKDOWN_CAPTURE_VIEWER")) {
            QSignalSpy firstFrames(window, &QQuickWindow::frameSwapped);
            window->update();
            QTRY_VERIFY(!firstFrames.isEmpty());
            const auto image = window->grabWindow();
            QVERIFY(!image.isNull());
            const auto path = qEnvironmentVariable("QMARKDOWN_CAPTURE_VIEWER");
            QVERIFY(image.save(path));
            QVERIFY(QMetaObject::invokeMethod(window, "switchStyle"));
            QTRY_VERIFY(content(preview) > 0);
            // The glyph draw follows polish; wait for a frame before grabbing.
            QSignalSpy frames(window, &QQuickWindow::frameSwapped);
            window->update();
            QTRY_VERIFY(!frames.isEmpty());
            QVERIFY(window->grabWindow().save(path + ".alternate.png"));
            window->setProperty("sampleIndex", 3);
            for (int preset = 0; preset < 2; ++preset) {
                QVERIFY(QMetaObject::invokeMethod(panel, "applyPreset", Q_ARG(QVariant, bool(preset))));
                QTest::qWait(100);
                QVERIFY(window->grabWindow().save(path + QString(".fenced.%1.png").arg(preset)));
            }
        }
    }
#endif
    void plainTextAndPrivateApi()
    {
        RequestFactory factory;
        QQmlEngine engine; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\n"
            "import QMarkdown\n"
            "MarkdownView { width: 300; markdown: \"<img src='https://example.invalid/x'> ![x](https://example.invalid/x) [link](https://example.invalid) &amp; *literal*\" }\n", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QTRY_COMPARE(painted(view).size(), 1);
        QVERIFY(painted(view)[0]->text().contains("<img"));
        QVERIFY(painted(view)[0]->text().contains("![x](https://example.invalid/x)"));
        QVERIFY(painted(view)[0]->text().contains("[link](https://example.invalid)"));
        QVERIFY(!painted(view)[0]->text().contains("&amp;"));
        QCOMPARE(painted(view)[0]->metaObject()->indexOfSignal("linkActivated(QString)"), -1);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(400, 400); window.show();
        QTRY_VERIFY(content(view) > 0);
        QCOMPARE(factory.requests, 0);
        view->setProperty("markdown", "<a href='https://example.invalid'>link</a> ![image](https://example.invalid/x)");
        QTRY_COMPARE(painted(view).size(), 0);
        QTRY_COMPARE(texts(view).size(), 1);
        QTRY_VERIFY(content(view) > 0);
        QCOMPARE(texts(view)[0]->property("textFormat").toInt(), 0);
        QSignalSpy activation(texts(view)[0], SIGNAL(linkActivated(QString)));
        QVERIFY(activation.isValid());
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(30, 10));
        QCOMPARE(activation.count(), 0);
        QCOMPARE(factory.requests, 0);
        for (const char *type : {"ViewState", "BlockModel", "ModuleAnchor", "FormattedText"}) {
            QQmlComponent hidden(&engine);
            hidden.setData(QByteArray("import QMarkdown\n") + type + " {}", {});
            QVERIFY(hidden.isError());
        }
    }
};
QTEST_MAIN(ViewTest)
#include "tst_view.moc"
