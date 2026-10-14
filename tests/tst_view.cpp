#include "private/formattedtext.h"
#include "private/viewstate.h"
#include <QAccessible>
#include "markdownstyle.h"
#include "markdownresourcepolicy.h"
#include <QTemporaryDir>
#include <QImage>
#include <QScreen>
#include "private/inline.h"
#include "private/resourcecontroller.h"
#include <QSemaphore>
#include <atomic>
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFontMetricsF>
#include <QFontDatabase>
#include <QSignalSpy>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QJSValue>
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

#ifdef QMARKDOWN_VIEWER_SOURCE
void qml_register_types_QMarkdownViewer_Tools();
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
    static QList<QQuickItem *> named(QQuickItem *item, const QString &name)
    {
        QList<QQuickItem *> result;
        for (auto *child : item->childItems()) {
            if (child->objectName() == name) result.append(child);
            result.append(named(child, name));
        }
        return result;
    }
    static double content(QQuickItem *view) { return view->property("contentHeight").toDouble(); }
    QFont savedApplicationFont;
private slots:
    void gfmSwitchCombinations()
    {
        const char *properties[]{"taskListsEnabled", "strikethroughEnabled", "extendedAutolinksEnabled", "tablesEnabled", "tagFilterEnabled", "gfmEnabled"};
        const QString source = "- [x] task\n\n~~strike~~ www.a.b\n\n| head |\n| - |\nbody\n\ntext <script>";
        for (int mask = 0; mask < 64; ++mask) {
            ViewState state;
            for (const char *property : properties) QVERIFY(!state.property(property).toBool());
            for (int bit=0; bit<6; ++bit) QVERIFY(state.setProperty(properties[bit], bool(mask & (1 << bit))));
            state.setMarkdown(source);
            auto *model = state.blocks();
            auto children = [](QAbstractItemModel *m, int row) { return m->data(m->index(row,0),BlockModel::ChildBlocksRole).value<QAbstractItemModel *>(); };
            const bool aggregate = mask & 32;
            auto *items = children(model,0); QVERIFY(items);
            QCOMPARE(items->data(items->index(0,0),BlockModel::TaskStateRole).toInt(), (aggregate || (mask & 1)) ? 2 : 0);
            auto ranges=model->data(model->index(1,0),BlockModel::FormatRangesRole).toList();
            QCOMPARE(!ranges.isEmpty(), aggregate || bool(mask & 2));
            auto links=model->data(model->index(1,0),BlockModel::LinkSpansRole).toList();
            QCOMPARE(!links.isEmpty(), aggregate || bool(mask & 4));
            QCOMPARE(model->data(model->index(2,0),BlockModel::RenderKindRole).toInt() == ViewState::Table, aggregate || bool(mask & 8));
            // Property names and signals are checked independently of parser flags.
            QSignalSpy markdownChanges(&state,&ViewState::markdownChanged);
            QSignalSpy tasks(&state,&ViewState::taskListsEnabledChanged), strike(&state,&ViewState::strikethroughEnabledChanged);
            QSignalSpy autolinks(&state,&ViewState::extendedAutolinksEnabledChanged), tables(&state,&ViewState::tablesEnabledChanged);
            QSignalSpy tags(&state,&ViewState::tagFilterEnabledChanged), gfm(&state,&ViewState::gfmEnabledChanged);
            QSignalSpy resets(model,&QAbstractItemModel::modelReset);
            for (int bit=0; bit<6; ++bit) state.setProperty(properties[bit], bool(mask & (1 << bit)));
            QCOMPARE(resets.size(),0); QCOMPARE(gfm.size(),0);
            state.setGfmEnabled(!aggregate); QCOMPARE(gfm.size(),1); QCOMPARE(resets.size(),1);
            for(int bit=0;bit<5;++bit) QCOMPARE(state.property(properties[bit]).toBool(),bool(mask & (1 << bit)));
            state.setGfmEnabled(aggregate); QCOMPARE(gfm.size(),2); QCOMPARE(resets.size(),2);
            QCOMPARE(model->data(model->index(2,0),BlockModel::RenderKindRole).toInt()==ViewState::Table, aggregate || bool(mask & 8));
            QCOMPARE(tasks.size()+strike.size()+autolinks.size()+tables.size()+tags.size(),0);
            QCOMPARE(state.markdown(),source); QCOMPARE(markdownChanges.size(),0);
            // Changing a masked individual property still follows the rebuild contract.
            state.setGfmEnabled(true); resets.clear(); gfm.clear();
            state.setTablesEnabled(!(mask & 8)); QCOMPARE(tables.size(),1); QCOMPARE(resets.size(),1); QCOMPARE(gfm.size(),0);
            state.setTagFilterEnabled(!(mask & 16)); QCOMPARE(tags.size(),1); QCOMPARE(resets.size(),2);
            state.setMarkdown("replacement"); QCOMPARE(markdownChanges.size(),1); QCOMPARE(model->rowCount(),1);
            QCOMPARE(model->data(model->index(0,0),BlockModel::BlockTextRole).toString(),"replacement");
        }
    }
    void playgroundGfmControls()
    {
#ifdef QMARKDOWN_VIEWER_SOURCE
        QQmlEngine engine; QQmlComponent component(&engine,QUrl::fromLocalFile(QStringLiteral(QMARKDOWN_VIEWER_SOURCE)));
        QVERIFY2(component.isReady(),qPrintable(component.errorString())); std::unique_ptr<QObject> object(component.create());
        auto *window=qobject_cast<QQuickWindow *>(object.get()); QVERIFY(window);
        auto *preview=window->findChild<QQuickItem *>("preview"); QVERIFY(preview);
        auto *tables=window->findChild<QQuickItem *>("tablesToggle"); auto *tags=window->findChild<QQuickItem *>("tagFilterToggle");
        auto *gfm=window->findChild<QQuickItem *>("gfmToggle"); QVERIFY(tables); QVERIFY(tags); QVERIFY(gfm);
        window->setProperty("sampleIndex",13); QTRY_VERIFY(content(preview)>0); const auto source=preview->property("markdown");
        QVERIFY(named(preview,"markdownTable").isEmpty()); gfm->setProperty("checked",true);
        QTRY_COMPARE(named(preview,"markdownTable").size(),2); QVERIFY(!tables->property("checked").toBool());
        tables->setProperty("checked",true); tags->setProperty("checked",true); gfm->setProperty("checked",false);
        QTRY_COMPARE(named(preview,"markdownTable").size(),2); QVERIFY(preview->property("tagFilterEnabled").toBool());
        tables->setProperty("checked",false); QTRY_VERIFY(named(preview,"markdownTable").isEmpty());
        QCOMPARE(preview->property("markdown"),source);
#else
        QSKIP("Playground is not built in this configuration");
#endif
    }
    void nativeTables()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.6\nMarkdownView { width: 240; gfmEnabled: true }",{});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create()); auto *view=qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        auto *state=view->findChild<ViewState *>(); QVERIFY(state);
        state->setMarkdown("| left | center | right |\n| :- | :-: | -: |\n| wrapping words wrapping words | [**link**](host:table) | ~~strike~~ |\n\n> | quote |\n> | - |\n> | `code` |\n\n- | list |\n  | - |\n  | body |");
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(360,640); window.show();
        QTRY_COMPARE(named(view,"markdownTable").size(),3); QTRY_COMPARE(named(view,"tableCell").size(),10);
        const auto cells=named(view,"tableCell"); const auto rows=named(view,"tableRow");
        QCOMPARE(cells[0]->width(),80.0); QCOMPARE(cells[1]->x(),80.0); QCOMPARE(cells[2]->x(),160.0);
        QTRY_VERIFY(cells[3]->height() > cells[0]->height());
        QCOMPARE(cells[3]->height(),cells[4]->height()); QCOMPARE(cells[4]->height(),cells[5]->height());
        QVERIFY(texts(cells[0])[0]->property("font").value<QFont>().bold());
        QCOMPARE(texts(cells[1])[0]->property("horizontalAlignment").toInt(),int(Qt::AlignHCenter));
        QCOMPARE(texts(cells[2])[0]->property("horizontalAlignment").toInt(),int(Qt::AlignRight));
        auto *link=painted(cells[4])[0]; QTRY_VERIFY(link->layout()); QCOMPARE(link->horizontalAlignment(),int(Qt::AlignHCenter));
        QCOMPARE(link->linkDestination(0),"host:table"); QVERIFY(link->layout()->formats()[0].format.font().bold());
        const auto line=link->layout()->lineAt(0); const QPointF offset(-link->x(),-link->y());
        const auto point=QPointF((line.cursorToX(0)+line.cursorToX(1))/2,line.y()+line.height()/2)+offset;
        QCOMPARE(link->linkAt(point.x(),point.y()),0);
        QSignalSpy activated(view,SIGNAL(linkActivated(QString)));
        QTest::mouseClick(&window,Qt::LeftButton,Qt::NoModifier,link->mapToScene(point).toPoint());
        QCOMPARE(activated.size(),1); QCOMPARE(activated[0][0].toString(),"host:table");
        state->style()->table()->setHeaderForeground(Qt::red); state->style()->table()->setHeaderBackground(Qt::yellow);
        state->style()->table()->setBodyBackground(Qt::cyan); state->style()->table()->setBorderColor(Qt::blue);
        state->style()->table()->setBorderWidth(1); state->style()->table()->setHorizontalPadding(8);
        QTRY_COMPARE(texts(cells[0])[0]->property("color").value<QColor>(),QColor(Qt::red));
        QCOMPARE(cells[0]->property("color").value<QColor>(),QColor(Qt::yellow));
        QCOMPARE(cells[3]->property("color").value<QColor>(),QColor(Qt::cyan));
        state->style()->table()->resetHeaderForeground(); state->style()->setBodyColor(Qt::green);
        QTRY_COMPARE(texts(cells[0])[0]->property("color").value<QColor>(),QColor(Qt::green));
        state->style()->table()->setHorizontalPadding(std::numeric_limits<qreal>::infinity());
        state->style()->table()->setVerticalPadding(std::numeric_limits<qreal>::quiet_NaN());
        state->style()->table()->setBorderWidth(-2);
        QTRY_COMPARE(cells[0]->property("padX").toDouble(),0.0);
        QCOMPARE(cells[0]->property("padY").toDouble(),0.0); QCOMPARE(cells[0]->property("edge").toDouble(),0.0);
        state->style()->table()->resetHorizontalPadding(); state->style()->table()->resetVerticalPadding(); state->style()->table()->resetBorderWidth();
        view->setWidth(60); QTRY_COMPARE(cells[0]->width(),20.0);
        QVERIFY(cells[3]->height() > 0); view->setWidth(1); QTRY_VERIFY(content(view)>0);
        state->setTablesEnabled(true); state->setGfmEnabled(false); QTRY_COMPARE(named(view,"markdownTable").size(),3);
        state->setTablesEnabled(false); QTRY_VERIFY(named(view,"markdownTable").isEmpty());
    }
    void tableImagesAndCancellation_data()
    {
        QTest::addColumn<QString>("property");
        for (const auto &name : {"tablesEnabled","tagFilterEnabled","gfmEnabled"}) QTest::newRow(name) << QString(name);
    }
    void tableImagesAndCancellation()
    {
        QFETCH(QString,property);
        QTemporaryDir temp; QImage image(20,10,QImage::Format_RGB32); image.fill(Qt::green); QVERIFY(image.save(temp.filePath("image.png")));
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.6\nMarkdownView { width: 200; tablesEnabled: true }",{});
        QVERIFY2(component.isReady(),qPrintable(component.errorString())); std::unique_ptr<QObject> object(component.create());
        auto *view=qobject_cast<QQuickItem *>(object.get()); QVERIFY(view); auto *controller=view->findChild<ResourceController *>(); QVERIFY(controller);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300,300); window.show();
        view->setProperty("baseUrl",QUrl::fromLocalFile(temp.filePath("document.md")));
        const QString source="| image |\n| - |\n| [![alt](image.png)](host:image) |";
        view->setProperty("markdown",source); QTRY_COMPARE(named(view,"tableCell").size(),2);
        QVERIFY(named(view,"markdownImage").isEmpty());
        const auto deniedCell=named(view,"tableCell")[1]; QTRY_COMPARE(painted(deniedCell).size(),1);
        QCOMPARE(painted(deniedCell)[0]->text(),"alt");
        auto *policy=view->property("resourcePolicy").value<MarkdownResourcePolicy *>(); QVERIFY(policy);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())}); QTRY_COMPARE(named(view,"markdownImage").size(),1);
        const auto imageCell=named(view,"tableCell")[1]; QVERIFY(named(imageCell,"markdownImage").size()==1);
        QSignalSpy activated(view,SIGNAL(linkActivated(QString)));
        auto *imageItem=named(view,"markdownImage")[0]; QTest::mouseClick(&window,Qt::LeftButton,Qt::NoModifier,imageItem->mapToScene(QPointF(5,5)).toPoint());
        QCOMPARE(activated.size(),1); QCOMPARE(activated[0][0].toString(),"host:image");
        struct Gate { QSemaphore entered, release, completed; std::atomic<bool> timedOut{false}; };
        auto gate=std::make_shared<Gate>(); struct Release { std::shared_ptr<Gate> gate; ~Release(){ gate->release.release(2); } } cleanup{gate};
        controller->setDecoder([gate](QByteArray bytes){ gate->entered.release(); if(!gate->release.tryAcquire(1,5000))gate->timedOut=true; auto result=QMarkdownPrivate::decodeImage(std::move(bytes)); gate->completed.release(); return result; });
        view->setProperty("markdown",source+"\n| again |"); QVERIFY(gate->entered.tryAcquire(1,2000));
        const auto key=property.toUtf8(); view->setProperty(key.constData(),!view->property(key.constData()).toBool());
        QVERIFY(gate->entered.tryAcquire(1,2000)); QSignalSpy published(controller,&ResourceController::changed);
        view->setProperty("markdown","replacement"); gate->release.release(2); QVERIFY(gate->completed.tryAcquire(2,2000));
        QCoreApplication::processEvents(); QTRY_VERIFY(named(view,"markdownImage").isEmpty()); QCOMPARE(published.size(),0); QVERIFY(!gate->timedOut);
    }
    void extendedAutolinksToggleAndRendering()
    {
        QQmlEngine engine; RequestFactory factory; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.5\nMarkdownView { width: 150; taskListsEnabled: true; strikethroughEnabled: true }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create()); auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        state->setMarkdown("# www.a.b\n\n~~**http://a.b/wrapping-path**~~ *foo@a.b*\n\n- [x] www.task.test\n  - xmpp:foo@a.b/res\n\n![www.inert.test](missing.png)");
        const auto source = state->markdown();
        QSignalSpy markdownChanges(view, SIGNAL(markdownChanged())); QSignalSpy changes(view, SIGNAL(extendedAutolinksEnabledChanged()));
        QSignalSpy taskChanges(view, SIGNAL(taskListsEnabledChanged())); QSignalSpy strikeChanges(view, SIGNAL(strikethroughEnabledChanged()));
        QSignalSpy resets(state->blocks(), &QAbstractItemModel::modelReset); QSignalSpy links(view, SIGNAL(linkActivated(QString)));
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(320, 650); window.show();
        QTRY_VERIFY(content(view) > 0); QVERIFY(!view->property("extendedAutolinksEnabled").toBool());
        for (auto *item : painted(view)) QVERIFY(item->linkSpans().isEmpty());
        QVERIFY(view->setProperty("extendedAutolinksEnabled", true)); QTRY_COMPARE(painted(view).size(), 4);
        const auto items = painted(view);
        const QStringList destinations{"http://www.a.b", "http://a.b/wrapping-path", "http://www.task.test", "xmpp:foo@a.b/res"};
        for (int i = 0; i < 4; ++i) {
            QTRY_VERIFY(items[i]->layout() && !items[i]->layout()->formats().isEmpty());
            QCOMPARE(items[i]->linkDestination(0), destinations[i]);
            const auto line = items[i]->layout()->lineAt(0);
            const qreal x = (line.cursorToX(0) + line.cursorToX(1)) / 2, y = line.y() + line.height() / 2;
            QCOMPARE(items[i]->linkAt(x, y), 0);
            const QPoint hit = items[i]->mapToScene(QPointF(x, y)).toPoint();
            QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, hit);
            QCOMPARE(links.size(), i + 1); QCOMPARE(links.last()[0].toString(), destinations[i]);
        }
        auto *body = items[1]; QVERIFY(body->layout()->lineCount() > 1);
        QCOMPARE(body->linkDestination(1), "mailto:foo@a.b");
        QVERIFY(body->layout()->formats()[0].format.font().strikeOut());
        QVERIFY(body->layout()->formats()[0].format.font().weight() >= QFont::Bold);
        QVERIFY(body->layout()->formats().last().format.font().italic());
        QVERIFY(texts(view).last()->property("text").toString().contains("www.inert.test"));
        const auto firstLine = body->layout()->lineAt(0);
        const QPoint hit = body->mapToScene(QPointF((firstLine.cursorToX(0) + firstLine.cursorToX(1)) / 2, firstLine.height() / 2)).toPoint();
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, hit);
        QTest::mouseMove(&window, hit + QPoint(40, 30));
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, hit + QPoint(40, 30)); QCOMPARE(links.size(), 4);
        QFont font = state->style()->bodyFont(); font.setPixelSize(23); state->style()->setBodyFont(font);
        state->style()->setBodyColor(Qt::red); state->style()->setLinkColor(Qt::green);
        QTRY_COMPARE(body->layout()->formats()[0].format.font().pixelSize(), 23);
        QCOMPARE(body->color(), QColor(Qt::red)); QCOMPARE(body->layout()->formats()[0].format.foreground().color(), QColor(Qt::green));
        QVERIFY(body->layout()->formats()[0].format.font().strikeOut());
        const int resetCount = resets.size(); view->setProperty("extendedAutolinksEnabled", true);
        QCOMPARE(resets.size(), resetCount); QCOMPARE(changes.size(), 1);
        view->setProperty("extendedAutolinksEnabled", false); QCOMPARE(changes.size(), 2);
        QTRY_COMPARE(painted(view).size(), 1); QVERIFY(painted(view)[0]->linkSpans().isEmpty());
        QCOMPARE(state->markdown(), source); QCOMPARE(markdownChanges.size(), 0); QCOMPARE(taskChanges.size(), 0); QCOMPARE(strikeChanges.size(), 0);
        view->setProperty("extendedAutolinksEnabled", true); view->setProperty("markdown", "mailto:new@a.b");
        QTRY_COMPARE(painted(view).size(), 1); QCOMPARE(painted(view)[0]->linkDestination(0), "mailto:new@a.b");
        QCOMPARE(markdownChanges.size(), 1); QCOMPARE(changes.size(), 3); QCOMPARE(factory.requests, 0);
    }
    void extendedAutolinksResourceCancellation()
    {
        QTemporaryDir temp; QImage image(20,10,QImage::Format_RGB32); image.fill(Qt::green); QVERIFY(image.save(temp.filePath("image.png")));
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.5\nMarkdownView { width: 200 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString())); std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        auto *controller = view->findChild<ResourceController *>(); QVERIFY(controller);
        struct Gate { QSemaphore entered, release, completed; std::atomic<bool> timedOut{false}; };
        auto gate = std::make_shared<Gate>();
        struct Release { std::shared_ptr<Gate> gate; ~Release() { gate->release.release(2); } } cleanup{gate};
        controller->setDecoder([gate](QByteArray bytes) {
            gate->entered.release(); if (!gate->release.tryAcquire(1, 5000)) gate->timedOut = true;
            auto result = QMarkdownPrivate::decodeImage(std::move(bytes)); gate->completed.release(); return result;
        });
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300,300); window.show();
        view->setProperty("baseUrl", QUrl::fromLocalFile(temp.filePath("document.md")));
        auto *policy = view->property("resourcePolicy").value<MarkdownResourcePolicy *>(); QVERIFY(policy);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        view->setProperty("markdown", "![www.a.b](image.png)"); QVERIFY(gate->entered.tryAcquire(1, 2000));
        view->setProperty("extendedAutolinksEnabled", true); QVERIFY(gate->entered.tryAcquire(1, 2000));
        QSignalSpy published(controller, &ResourceController::changed);
        view->setProperty("markdown", "www.replacement.test"); gate->release.release(2);
        QVERIFY(gate->completed.tryAcquire(2, 2000)); QCoreApplication::processEvents();
        QTRY_COMPARE(painted(view).size(), 1); QCOMPARE(painted(view)[0]->linkDestination(0), "http://www.replacement.test");
        QVERIFY(named(view, "markdownImage").isEmpty()); QCOMPARE(published.size(), 0); QVERIFY(!gate->timedOut);
        controller->setDecoder(QMarkdownPrivate::decodeImage);
        view->setProperty("markdown", "![www.a.b](image.png)"); QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        view->setProperty("extendedAutolinksEnabled", false); QTRY_COMPARE(named(view, "markdownImage").size(), 1);
    }
    void playgroundExtendedAutolinks()
    {
#ifdef QMARKDOWN_VIEWER_SOURCE
        QQmlEngine engine; QQmlComponent component(&engine, QUrl::fromLocalFile(QStringLiteral(QMARKDOWN_VIEWER_SOURCE)));
        QVERIFY2(component.isReady(), qPrintable(component.errorString())); std::unique_ptr<QObject> object(component.create());
        auto *window = qobject_cast<QQuickWindow *>(object.get()); QVERIFY(window);
        auto *preview = window->findChild<QQuickItem *>("preview"); auto *toggle = window->findChild<QQuickItem *>("autolinkToggle"); QVERIFY(preview); QVERIFY(toggle);
        window->setProperty("sampleIndex", 12); QTRY_VERIFY(content(preview) > 0);
        QVERIFY(!preview->property("extendedAutolinksEnabled").toBool()); const auto source = preview->property("markdown");
        toggle->setProperty("checked", true); QTRY_VERIFY(preview->property("extendedAutolinksEnabled").toBool());
        QTRY_VERIFY(!painted(preview).isEmpty()); QCOMPARE(painted(preview)[0]->linkDestination(0), "http://www.commonmark.org");
        const auto screenshot = qEnvironmentVariable("QMARKDOWN_AUTOLINK_SCREENSHOT");
        if (!screenshot.isEmpty()) { QImage image; QTRY_VERIFY(!(image = window->grabWindow()).isNull()); QVERIFY(image.save(screenshot)); }
        toggle->setProperty("checked", false); QTRY_VERIFY(!preview->property("extendedAutolinksEnabled").toBool()); QCOMPARE(preview->property("markdown"), source);
#else
        QSKIP("Playground is not built in this configuration");
#endif
    }

    void strikethroughToggleAndRendering()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.4\nMarkdownView { width: 150; markdown: '# ~~Heading~~\\n\\n~~**bold** *italic* `code` [link](strike:target) wrapping words wrapping words~~\\n\\n- [x] ~~Task~~\\n  - ~~Nested~~\\n\\n![~~fallback~~](missing.png)' }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create()); auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        const auto source = state->markdown();
        QSignalSpy markdownChanges(view, SIGNAL(markdownChanged()));
        QSignalSpy optionChanges(view, SIGNAL(strikethroughEnabledChanged()));
        QSignalSpy resets(state->blocks(), &QAbstractItemModel::modelReset);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 600); window.show();
        QTRY_VERIFY(content(view) > 0); QVERIFY(!view->property("strikethroughEnabled").toBool());
        QVERIFY(view->setProperty("strikethroughEnabled", true));
        QTRY_COMPARE(painted(view).size(), 5);
        auto items = painted(view);
        QCOMPARE(items[0]->text(), "Heading"); QCOMPARE(items[2]->text(), "[x] Task");
        QCOMPARE(items[3]->text(), "Nested"); QCOMPARE(items[4]->text(), "fallback");
        for (auto *item : items) {
            QTRY_VERIFY(item->layout() && !item->layout()->formats().isEmpty());
            for (const auto &format : item->layout()->formats()) QVERIFY(format.format.font().strikeOut());
        }
        auto *body = items[1]; QVERIFY(body->layout()->lineCount() > 1);
        bool bold = false, italic = false, code = false;
        for (const auto &format : body->layout()->formats()) {
            bold |= format.format.font().weight() >= QFont::Bold;
            italic |= format.format.font().italic();
            code |= format.start == 12 && format.length == 4;
        }
        QVERIFY(bold); QVERIFY(italic); QVERIFY(code);
        QSignalSpy links(view, SIGNAL(linkActivated(QString)));
        bool clicked = false;
        for (int i = 0; i < body->layout()->lineCount(); ++i) {
            const auto line = body->layout()->lineAt(i);
            if (line.textStart() <= 17 && 17 < line.textStart() + line.textLength()) {
                const qreal x = (line.cursorToX(17) + line.cursorToX(18)) / 2, y = line.y() + line.height() / 2;
                QCOMPARE(body->linkAt(x, y), 0);
                QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, body->mapToScene(QPointF(x, y)).toPoint());
                clicked = true; break;
            }
        }
        QVERIFY(clicked); QCOMPARE(links.size(), 1); QCOMPARE(links[0][0].toString(), "strike:target");
        QFont font = state->style()->bodyFont(); font.setPixelSize(23); state->style()->setBodyFont(font);
        state->style()->setBodyColor(Qt::red); state->style()->setLinkColor(Qt::green);
        QTRY_COMPARE(body->layout()->formats()[0].format.font().pixelSize(), 23);
        QCOMPARE(body->color(), QColor(Qt::red));
        for (const auto &format : body->layout()->formats()) {
            QVERIFY(format.format.font().strikeOut());
            if (format.start == 17) QCOMPARE(format.format.foreground().color(), QColor(Qt::green));
        }
        const int resetCount = resets.size();
        view->setProperty("strikethroughEnabled", true);
        QCOMPARE(resets.size(), resetCount); QCOMPARE(optionChanges.size(), 1);
        QCOMPARE(state->markdown(), source); QCOMPARE(markdownChanges.size(), 0);
        view->setProperty("taskListsEnabled", true);
        QTRY_COMPARE(painted(view)[2]->text(), "Task");
        QTRY_VERIFY(painted(view)[2]->layout() && !painted(view)[2]->layout()->formats().isEmpty());
        QVERIFY(painted(view)[2]->layout()->formats()[0].format.font().strikeOut());
        view->setProperty("strikethroughEnabled", false);
        QTRY_VERIFY(texts(view)[0]->property("text").toString().contains("~~Heading~~"));
        QCOMPARE(optionChanges.size(), 2); QCOMPARE(state->markdown(), source); QCOMPARE(markdownChanges.size(), 0);
        view->setProperty("strikethroughEnabled", true);
        view->setProperty("markdown", "~~Replacement~~");
        QTRY_COMPARE(painted(view).size(), 1); QCOMPARE(painted(view)[0]->text(), "Replacement");
        QTRY_VERIFY(painted(view)[0]->layout() && !painted(view)[0]->layout()->formats().isEmpty());
        QVERIFY(painted(view)[0]->layout()->formats()[0].format.font().strikeOut()); QCOMPARE(markdownChanges.size(), 1);
    }
    void playgroundStrikethrough()
    {
#ifdef QMARKDOWN_VIEWER_SOURCE
        QQmlEngine engine; QQmlComponent component(&engine, QUrl::fromLocalFile(QStringLiteral(QMARKDOWN_VIEWER_SOURCE)));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create()); auto *window = qobject_cast<QQuickWindow *>(object.get()); QVERIFY(window);
        auto *preview = window->findChild<QQuickItem *>("preview"); QVERIFY(preview);
        auto *toggle = window->findChild<QQuickItem *>("strikeToggle"); QVERIFY(toggle);
        window->setProperty("sampleIndex", 11); QTRY_VERIFY(content(preview) > 0);
        QVERIFY(!preview->property("strikethroughEnabled").toBool());
        const auto source = preview->property("markdown"); toggle->setProperty("checked", true);
        QTRY_VERIFY(preview->property("strikethroughEnabled").toBool());
        QTRY_VERIFY(!painted(preview).isEmpty());
        QTRY_VERIFY(painted(preview)[0]->layout() && !painted(preview)[0]->layout()->formats().isEmpty());
        QVERIFY(painted(preview)[0]->layout()->formats()[0].format.font().strikeOut());
        const auto screenshot = qEnvironmentVariable("QMARKDOWN_STRIKE_SCREENSHOT");
        if (!screenshot.isEmpty()) {
            QImage image; QTRY_VERIFY(!(image = window->grabWindow()).isNull()); QVERIFY(image.save(screenshot));
        }
        toggle->setProperty("checked", false); QTRY_VERIFY(!preview->property("strikethroughEnabled").toBool());
        QCOMPARE(preview->property("markdown"), source);
#else
        QSKIP("Playground is not built in this configuration");
#endif
    }

    void taskListsToggleAndLayout()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.3\nMarkdownView {\n    width: 240\n    markdown: \"9) [x] **Done** [link](task:link)\\n10) Plain item\\n11) [ ] Pending\\n    - [X] Nested\\n\\n    Second paragraph\\n\\n12) [ ] \"\n}", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        const auto source = state->markdown();
        QSignalSpy markdownChanges(view, SIGNAL(markdownChanged()));
        QSignalSpy optionChanges(view, SIGNAL(taskListsEnabledChanged()));
        QSignalSpy resets(state->blocks(), &QAbstractItemModel::modelReset);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 600); window.show();
        QTRY_VERIFY(content(view) > 0);
        QVERIFY(!view->property("taskListsEnabled").toBool());
        for (auto *indicator : named(view, "taskIndicator")) QVERIFY(!indicator->isVisible());
        QVERIFY(view->setProperty("taskListsEnabled", true));
        QTRY_COMPARE(optionChanges.size(), 1);
        QCOMPARE(state->markdown(), source); QCOMPARE(markdownChanges.size(), 0);
        QTRY_COMPARE(named(view, "listMarker").size(), 5);
        QStringList markers;
        for (auto *marker : named(view, "listMarker")) markers.append(marker->property("text").toString());
        QCOMPARE(markers, QStringList({"9)", "10)", "11)", QString::fromUtf8("•"), "12)"}));
        QList<QQuickItem *> indicators;
        for (auto *indicator : named(view, "taskIndicator"))
            if (indicator->isVisible()) indicators.append(indicator);
        QCOMPARE(indicators.size(), 4);
        QCOMPARE(indicators[0]->property("checked").toBool(), true);
        QCOMPARE(indicators[1]->property("checked").toBool(), false);
        QCOMPARE(indicators[2]->property("checked").toBool(), true);
        QCOMPARE(indicators[3]->property("checked").toBool(), false);
        QVERIFY(!painted(view).isEmpty());
        QCOMPARE(painted(view)[0]->property("text").toString(), "Done link");
        for (auto *item : texts(view)) QVERIFY(!item->property("text").toString().contains("[x]"));
        auto *indicator = indicators[0];
        auto *accessible = QAccessible::queryAccessibleInterface(indicator); QVERIFY(accessible);
        QCOMPARE(accessible->role(), QAccessible::CheckBox);
        QVERIFY(accessible->state().checked); QVERIFY(!accessible->state().focusable);
        QCOMPARE(accessible->text(QAccessible::Description), "Read-only Markdown task");
        auto *uncheckedAccessible = QAccessible::queryAccessibleInterface(indicators[1]); QVERIFY(uncheckedAccessible);
        QVERIFY(!uncheckedAccessible->state().checked);
        QSignalSpy links(view, SIGNAL(linkActivated(QString)));
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
            indicator->mapToScene(QPointF(indicator->width() / 2, indicator->height() / 2)).toPoint());
        QTest::keyClick(&window, Qt::Key_Space);
        QCOMPARE(indicator->property("checked").toBool(), true);
        QVERIFY(!indicator->hasActiveFocus()); QCOMPARE(links.size(), 0); QCOMPARE(state->markdown(), source);
        if (auto *actions = accessible->actionInterface())
            actions->doAction(QAccessibleActionInterface::toggleAction());
        QCOMPARE(indicator->property("checked").toBool(), true); QCOMPARE(state->markdown(), source);
        auto *label = painted(view)[0];
        QTRY_VERIFY(label->layout() && label->layout()->lineCount() > 0);
        const auto line = label->layout()->lineAt(0);
        const qreal linkX = (line.cursorToX(5) + line.cursorToX(6)) / 2;
        const qreal linkY = line.y() + line.height() / 2;
        QCOMPARE(label->linkAt(linkX, linkY), 0);
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
            label->mapToScene(QPointF(linkX, linkY)).toPoint());
        QCOMPARE(links.size(), 1); QCOMPARE(links[0][0].toString(), "task:link");
        QVERIFY(indicators.last()->parentItem()->height() >= QFontMetricsF(state->style()->bodyFont()).height());
        const auto size = indicator->height();
        QFont font = state->style()->bodyFont(); font.setPixelSize(24); state->style()->setBodyFont(font);
        state->style()->setBodyColor(Qt::red);
        QTRY_VERIFY(indicator->height() > size);
        const auto boxes = named(view, "taskBox"); QVERIFY(!boxes.isEmpty());
        QCOMPARE(QQmlProperty::read(boxes[0], "border.color").value<QColor>(), QColor(Qt::red));
        const int resetCount = resets.size();
        view->setProperty("taskListsEnabled", true);
        QCOMPARE(resets.size(), resetCount); QCOMPARE(optionChanges.size(), 1);
        view->setWidth(1);
        QTRY_VERIFY(content(view) > 0);
        for (auto *loader : named(view, "listItemContent")) {
            QCOMPARE(loader->x(), 0); QCOMPARE(loader->width(), 1);
        }
        for (auto *item : named(view, "taskIndicator")) QCOMPARE(item->width(), 0);
        view->setWidth(0); QTRY_COMPARE(content(view), 0);
        view->setWidth(-1); QTRY_COMPARE(content(view), 0);
        view->setWidth(240); QTRY_VERIFY(content(view) > 0);
        view->setProperty("taskListsEnabled", false);
        QTRY_COMPARE(optionChanges.size(), 2);
        for (auto *item : named(view, "taskIndicator")) QVERIFY(!item->isVisible());
        QCOMPARE(state->markdown(), source); QCOMPARE(markdownChanges.size(), 0);
        view->setProperty("taskListsEnabled", true);
        view->setProperty("markdown", "- [ ] Replacement");
        QTRY_COMPARE(named(view, "listMarker").size(), 1);
        QTRY_VERIFY(!named(view, "taskIndicator").isEmpty());
        QCOMPARE(named(view, "taskIndicator")[0]->property("checked").toBool(), false);
        QCOMPARE(texts(view).last()->property("text").toString(), "Replacement");
    }
    void taskListsImageProjection()
    {
        QTemporaryDir temp; QVERIFY(temp.isValid());
        QImage image(30, 20, QImage::Format_ARGB32); image.fill(Qt::green);
        QVERIFY(image.save(temp.filePath("image.png")));
        QQmlEngine engine; RequestFactory factory; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.3
MarkdownView {
    width: 200
    taskListsEnabled: true
    markdown: "- [x] Before ![*alt*](image.png) after"
})", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        auto *state = view->findChild<ViewState *>(); QVERIFY(state);
        view->setProperty("baseUrl", QUrl::fromLocalFile(temp.filePath("document.md")));
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300,300); window.show();
        QTRY_VERIFY(!named(view, "taskIndicator").isEmpty());
        QVERIFY(named(view, "markdownImage").isEmpty()); QCOMPARE(factory.requests, 0);
        const auto source = state->markdown();
        state->resourcePolicy()->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        QCOMPARE(named(view, "taskIndicator")[0]->property("checked").toBool(), true);
        QCOMPARE(named(view, "markdownImage")[0]->height(), 20);
        QSignalSpy resets(state->blocks(), &QAbstractItemModel::modelReset);
        view->setProperty("taskListsEnabled", true);
        QCOMPARE(resets.size(), 0); QCOMPARE(named(view, "markdownImage").size(), 1);
        view->setProperty("taskListsEnabled", false);
        QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        QVERIFY(!named(view, "taskIndicator")[0]->isVisible());
        view->setProperty("taskListsEnabled", true);
        QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        QCOMPARE(named(view, "taskIndicator")[0]->property("checked").toBool(), true);
        QCOMPARE(state->markdown(), source);
    }
    void playgroundTaskLists()
    {
#ifdef QMARKDOWN_VIEWER_SOURCE
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl::fromLocalFile(QStringLiteral(QMARKDOWN_VIEWER_SOURCE)));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *window = qobject_cast<QQuickWindow *>(object.get()); QVERIFY(window);
        auto *preview = window->findChild<QQuickItem *>("preview"); QVERIFY(preview);
        auto *toggle = window->findChild<QQuickItem *>("taskToggle"); QVERIFY(toggle);
        window->setProperty("sampleIndex", 10);
        QTRY_VERIFY(content(preview) > 0);
        QVERIFY(!preview->property("taskListsEnabled").toBool());
        const auto source = preview->property("markdown");
        toggle->setProperty("checked", true);
        QTRY_VERIFY(preview->property("taskListsEnabled").toBool());
        QList<QQuickItem *> tasks;
        for (auto *item : named(preview, "taskIndicator")) if (item->isVisible()) tasks.append(item);
        QCOMPARE(tasks.size(), 8);
        QCOMPARE(tasks[0]->property("checked").toBool(), true);
        QCOMPARE(preview->property("markdown"), source);
        const auto screenshot = qEnvironmentVariable("QMARKDOWN_TASKS_SCREENSHOT");
        if (!screenshot.isEmpty()) {
            QImage image;
            QTRY_VERIFY(!(image = window->grabWindow()).isNull());
            QVERIFY(image.save(screenshot));
        }
        toggle->setProperty("checked", false);
        QTRY_VERIFY(!preview->property("taskListsEnabled").toBool());
        for (auto *item : named(preview, "taskIndicator")) QVERIFY(!item->isVisible());
        QCOMPARE(preview->property("markdown"), source);
#else
        QSKIP("Playground is not built in this configuration");
#endif
    }
    void codeActionsText_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("expected");
        QTest::newRow("fence") << "```cpp ignored\n \t<literal>  \n\n```" << " \t<literal>  \n\n";
        QTest::newRow("indented") << "    one  \n    \ttwo\n" << "one  \n\ttwo\n";
        QTest::newRow("nested-fence") << "> - ```\n>   nested\n>   ```" << "nested\n";
        QTest::newRow("nested-indented") << ">     nested  \n" << "nested  \n";
        QTest::newRow("empty") << "```\n```" << "";
        QTest::newRow("unicode") << QString::fromUtf8("~~~\nλ 😀 é\n~~~") << QString::fromUtf8("λ 😀 é\n");
    }
    void codeActionsText()
    {
        QFETCH(QString, source); QFETCH(QString, expected);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.2
MarkdownView {
    width: 300
    codeBlockActions: Item {
        required property string codeText
        required property var requestCopy
        objectName: "actions"
        implicitHeight: 24
        function activate() { requestCopy() }
    }
})", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QSignalSpy requests(view, SIGNAL(codeBlockCopyRequested(QString)));
        view->setProperty("markdown", source);
        QTRY_COMPARE(named(view, "actions").size(), 1);
        auto *action = named(view, "actions")[0];
        QCOMPARE(action->property("codeText").toString(), expected);
        QVERIFY(QMetaObject::invokeMethod(action, "activate"));
        QCOMPARE(requests.size(), 1);
        QCOMPARE(requests[0][0].toString(), expected);
    }
    void codeActionsLifecycleAndGeometry()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.2
Item {
    property Component first: Component {
        Item {
            required property string codeText
            required property var requestCopy
            objectName: "actions"
            implicitHeight: width < 100 ? 40 : 20
            function activate() { requestCopy() }
        }
    }
    property Component second: Component {
        Item {
            required property string codeText
            required property var requestCopy
            objectName: "actions"
            implicitHeight: 30
        }
    }
    MarkdownView {
        objectName: "view"; width: 300
        markdown: "```\none\n```\n\n```\ntwo\n```"
        style: MarkdownStyle { codeBlock.horizontalPadding: 10; codeBlock.verticalPadding: 7 }
    }
    MarkdownView { objectName: "other"; width: 200; markdown: "```\nother\n```" }
})", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create()); QVERIFY(object);
        auto *view = object->findChild<QQuickItem *>("view"); QVERIFY(view);
        auto *other = object->findChild<QQuickItem *>("other"); QVERIFY(other);
        QQuickWindow window; window.resize(400, 600);
        view->setParentItem(window.contentItem()); other->setParentItem(window.contentItem()); window.show();
        QSignalSpy requests(view, SIGNAL(codeBlockCopyRequested(QString)));
        QSignalSpy otherRequests(other, SIGNAL(codeBlockCopyRequested(QString)));
        QTRY_COMPARE(named(view, "codeSurface").size(), 2);
        QTRY_VERIFY(content(view) > 0);
        const auto baseline = content(view);
        const auto first = object->property("first");
        view->setProperty("codeBlockActions", first);
        other->setProperty("codeBlockActions", first);
        QTRY_COMPARE(named(view, "actions").size(), 2);
        QTRY_COMPARE(content(view), baseline + 40);
        const auto actions = named(view, "actions");
        for (int i = 0; i < actions.size(); ++i) {
            auto *action = actions[i];
            QCOMPARE(action->width(), 280.0); QCOMPARE(action->height(), 20.0);
            QCOMPARE(action->parentItem()->y(), 7.0);
            auto *surface = action->parentItem()->parentItem();
            auto *code = texts(surface)[0];
            QCOMPARE(code->mapToItem(surface, QPointF()).y(), 27.0);
            QCOMPARE(surface->height(), code->height() + 34);
            QVERIFY(QMetaObject::invokeMethod(action, "activate"));
            QCOMPARE(requests.size(), i + 1);
            QCOMPARE(requests[i][0], action->property("codeText"));
        }
        QTRY_COMPARE(named(other, "actions").size(), 1);
        QVERIFY(QMetaObject::invokeMethod(named(other, "actions")[0], "activate"));
        QCOMPARE(otherRequests.size(), 1); QCOMPARE(requests.size(), 2);
        auto stale = actions[0]->property("requestCopy").value<QJSValue>();
        QPointer<QQuickItem> removed = actions[0];
        view->setProperty("codeBlockActions", object->property("second"));
        stale.call(); QCOMPARE(requests.size(), 2);
        QTRY_VERIFY(removed.isNull());
        QTRY_COMPARE(content(view), baseline + 60);
        view->setProperty("codeBlockActions", first);
        QTRY_COMPARE(named(view, "actions").size(), 2);
        view->setWidth(60);
        QTRY_COMPARE(named(view, "actions")[0]->width(), 40.0);
        QTRY_COMPARE(named(view, "actions")[0]->height(), 40.0);
        auto *action = named(view, "actions")[0];
        for (double invalid : {-10.0, std::numeric_limits<double>::infinity(),
                               std::numeric_limits<double>::quiet_NaN()}) {
            action->setImplicitHeight(invalid);
            QTRY_COMPARE(action->height(), 0.0);
            auto *surface = action->parentItem()->parentItem();
            QCOMPARE(texts(surface)[0]->mapToItem(surface, QPointF()).y(), 7.0);
        }
        stale = action->property("requestCopy").value<QJSValue>();
        view->setWidth(0); QTRY_COMPARE(content(view), 0);
        QTRY_VERIFY(named(view, "actions").isEmpty());
        stale.call(); QCOMPARE(requests.size(), 2);
        view->setWidth(1); QTRY_COMPARE(named(view, "actions").size(), 2);
        QCOMPARE(named(view, "actions")[0]->width(), 1.0);
        stale = named(view, "actions")[0]->property("requestCopy").value<QJSValue>();
        view->setProperty("markdown", "```\nreplacement\n```");
        QTRY_COMPARE(named(view, "actions").size(), 1);
        QTRY_COMPARE(named(view, "actions")[0]->property("codeText").toString(), QString("replacement\n"));
        stale.call(); QCOMPARE(requests.size(), 2);
        view->setProperty("markdown", "```\none\n```\n\n```\ntwo\n```");
        view->setWidth(300); QTRY_COMPARE(named(view, "actions").size(), 2);
        QTRY_COMPARE(named(view, "actions")[0]->property("codeText").toString(), QString("one\n"));
        stale = named(view, "actions")[0]->property("requestCopy").value<QJSValue>();
        QVERIFY(view->setProperty("codeBlockActions", QVariant::fromValue<QQmlComponent *>(nullptr)));
        QVERIFY(!view->property("codeBlockActions").value<QQmlComponent *>());
        stale.call(); QCOMPARE(requests.size(), 2);
        QTRY_COMPARE(content(view), baseline);
        view->setProperty("codeBlockActions", first);
        QTRY_COMPARE(named(view, "actions").size(), 2);
        stale = named(view, "actions")[0]->property("requestCopy").value<QJSValue>();
        delete view;
        QVERIFY(!stale.call().isError());
    }
    void codeActionsCreationFailure()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.2
MarkdownView {
    width: 200; markdown: "```\nstill rendered\n```"
    codeBlockActions: QtObject {
        required property string codeText
        required property var requestCopy
    }
})", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QTest::ignoreMessage(QtWarningMsg, "QMarkdown: codeBlockActions must create an Item with codeText and requestCopy properties");
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QTRY_COMPARE(texts(view).size(), 1);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 300); window.show();
        QCOMPARE(texts(view)[0]->property("text").toString(), QString("still rendered"));
        QTRY_VERIFY(content(view) > 0);
    }
    void codeActionsMissingRequiredProperty()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(R"(import QtQuick
import QMarkdown 1.2
MarkdownView {
    width: 200; markdown: "```\nstill rendered\n```"
    codeBlockActions: Item {
        required property string codeText
        required property var requestCopy
        required property int extra
    }
})", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*Required property extra was not initialized.*"));
        QTest::ignoreMessage(QtWarningMsg, "QMarkdown: codeBlockActions must create an Item with codeText and requestCopy properties");
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 300); window.show();
        QTRY_COMPARE(texts(view).size(), 1);
        QCOMPARE(texts(view)[0]->property("text").toString(), QString("still rendered"));
        QTRY_VERIFY(content(view) > 0);
    }
    void retainedSemanticsPresentation_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("display");
        QTest::addColumn<bool>("formatted");
        QTest::addColumn<int>("lines");
        QTest::newRow("soft-break") << "*one\ntwo*" << "one two" << true << 1;
        QTest::newRow("hard-break") << "*one  \ntwo*" << "one\ntwo" << true << 2;
        QTest::newRow("escaped-entity") << "\\*literal\\* &amp; &#65;" << "*literal* & A" << false << 1;
        QTest::newRow("nested-emphasis") << "***nested***" << "nested" << true << 1;
        QTest::newRow("link-title") << "[label](custom:target \"private title\")" << "label" << true << 1;
        QTest::newRow("empty-link") << "before [](custom:target) after" << "before  after" << false << 1;
        QTest::newRow("inline-html") << "*before* <b>&amp;</b> after" << "before <b>&</b> after" << true << 1;
        QTest::newRow("html-block") << "<div>\n*literal* &amp;\n</div>" << "<div>\n*literal* &amp;\n</div>" << false << 3;
    }
    void retainedSemanticsPresentation()
    {
        QFETCH(QString, source); QFETCH(QString, display); QFETCH(bool, formatted); QFETCH(int, lines);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nMarkdownView { width: 480 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(500, 300); window.show();
        view->setProperty("markdown", source);
        QTRY_VERIFY(content(view) > 0);
        if (formatted) {
            QTRY_COMPARE(painted(view).size(), 1);
            auto *item = painted(view)[0]; QTRY_VERIFY(item->layout());
            QCOMPARE(item->text(), display); QCOMPARE(item->layout()->lineCount(), lines);
            if (source == "***nested***") {
                QCOMPARE(item->layout()->formats().size(), 1);
                QVERIFY(item->layout()->formats()[0].format.font().italic());
                QCOMPARE(item->layout()->formats()[0].format.font().weight(), QFont::Bold);
            }
        } else {
            QTRY_COMPARE(texts(view).size(), 1);
            QCOMPARE(texts(view)[0]->property("text").toString(), display);
            QCOMPARE(texts(view)[0]->property("textFormat").toInt(), 0);
            QCOMPARE(texts(view)[0]->property("lineCount").toInt(), lines);
        }
        QCOMPARE(view->implicitHeight(), content(view));
    }
    void mixedDocumentHiddenTransitions()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nMarkdownView { width: 300 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(400, 600); window.show();
        view->setProperty("markdown", "> - **old**\n\n# old heading");
        QTRY_VERIFY(content(view) > 0);
        view->setWidth(0); QTRY_COMPARE(content(view), 0); QTRY_COMPARE(view->implicitHeight(), 0);
        QTRY_VERIFY(texts(view).isEmpty() && painted(view).isEmpty());
        auto font = style->bodyFont(); font.setPixelSize(23); style->setBodyFont(font);
        style->setBlockSpacing(13);
        view->setProperty("markdown", "> - **new words that wrap at narrow widths**\n>\n>   tail\n\n# new heading\n\n```\nnew code\n```");
        view->setWidth(300);
        QTRY_COMPARE(painted(view).size(), 1); QTRY_COMPARE(texts(view).size(), 4);
        QTRY_COMPARE(painted(view)[0]->text(), "new words that wrap at narrow widths");
        QTRY_COMPARE(painted(view)[0]->font().pixelSize(), 23);
        QTRY_COMPARE(named(view, "quoteRule").size(), 1);
        QTRY_VERIFY(content(view) > 0); QTRY_COMPARE(view->implicitHeight(), content(view));
        QStringList displayed;
        for (auto *item : texts(view)) displayed.append(item->property("text").toString());
        QCOMPARE(displayed, QStringList({"•", "tail", "new heading", "new code"}));
        QTRY_COMPARE(content(view), texts(view).last()->mapToItem(view, QPointF(0, texts(view).last()->height())).y());
        const auto wide = content(view);
        view->setWidth(50); QTRY_VERIFY(content(view) > wide); QTRY_COMPARE(view->implicitHeight(), content(view));
        view->setWidth(300); QTRY_COMPARE(content(view), wide);
        view->setProperty("markdown", "replacement");
        QTRY_VERIFY(painted(view).isEmpty() && named(view, "quoteRule").isEmpty());
        QTRY_COMPARE(texts(view).size(), 1); QTRY_COMPARE(texts(view)[0]->property("text").toString(), "replacement");
        QTRY_COMPARE(texts(view)[0]->property("font").value<QFont>().pixelSize(), 23);
        view->setProperty("markdown", ""); QTRY_COMPARE(content(view), 0); QTRY_COMPARE(view->implicitHeight(), 0);
    }
    void activeDecodeTeardown_data()
    {
        QTest::addColumn<bool>("destroyEngine");
        QTest::newRow("view") << false;
        QTest::newRow("engine") << true;
    }
    void activeDecodeTeardown()
    {
        QFETCH(bool, destroyEngine);
        QTemporaryDir temp; QVERIFY(temp.isValid());
        QImage image(20, 10, QImage::Format_ARGB32); image.fill(Qt::green);
        QVERIFY(image.save(temp.filePath("image.png")));
        auto engine = std::make_unique<QQmlEngine>();
        auto component = std::make_unique<QQmlComponent>(engine.get());
        component->setData("import QtQuick\nimport QMarkdown 1.0\nMarkdownView { width: 240 }", {});
        QVERIFY2(component->isReady(), qPrintable(component->errorString()));
        std::unique_ptr<QObject> object(component->create()); QVERIFY(object);
        QPointer<ResourceController> controller = object->findChild<ResourceController *>(); QVERIFY(controller);
        struct Gate { QSemaphore entered, release, completed; std::atomic<bool> timedOut{false}; };
        auto gate = std::make_shared<Gate>();
        struct Cleanup { std::shared_ptr<Gate> gate; ~Cleanup() { gate->release.release(); } } cleanup{gate};
        controller->setDecoder([gate](QByteArray bytes) {
            gate->entered.release();
            if (!gate->release.tryAcquire(1, 5000)) gate->timedOut = true;
            auto result = QMarkdownPrivate::decodeImage(std::move(bytes));
            gate->completed.release(); return result;
        });
        object->setProperty("baseUrl", QUrl::fromLocalFile(temp.filePath("document.md")));
        auto *policy = object->property("resourcePolicy").value<MarkdownResourcePolicy *>(); QVERIFY(policy);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        int publications = 0;
        connect(controller, &ResourceController::changed, this, [&] { ++publications; });
        object->setProperty("markdown", "![alt](image.png)");
        QVERIFY(gate->entered.tryAcquire(1, 2000));
        component.reset();
        QElapsedTimer elapsed; elapsed.start();
        if (destroyEngine) {
            object->setParent(engine.get()); // The engine owns this host-created root explicitly.
            object.release();
            engine.reset();
        } else object.reset();
        QVERIFY2(elapsed.elapsed() < 1000, "Native teardown waited for the held decoder");
        QVERIFY(controller.isNull()); QCOMPARE(gate->completed.available(), 0);
        gate->release.release(); QVERIFY(gate->completed.tryAcquire(1, 2000));
        QCoreApplication::processEvents(); QCOMPARE(publications, 0); QVERIFY(!gate->timedOut);
    }
    void staleImageCompletionPresentation_data()
    {
        QTest::addColumn<bool>("revoke");
        QTest::newRow("replacement") << false;
        QTest::newRow("policy-revocation") << true;
    }
    void staleImageCompletionPresentation()
    {
        QFETCH(bool, revoke);
        QTemporaryDir temp; QVERIFY(temp.isValid());
        QImage image(20, 10, QImage::Format_ARGB32); image.fill(Qt::green);
        QVERIFY(image.save(temp.filePath("image.png")));
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nMarkdownView { width: 240 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        auto *controller = object->findChild<ResourceController *>(); QVERIFY(controller);
        struct Gate { QSemaphore entered, release, completed; std::atomic<bool> timedOut{false}; };
        auto gate = std::make_shared<Gate>();
        // Release before view destruction even when an assertion returns early.
        struct Release { std::shared_ptr<Gate> gate; ~Release() { gate->release.release(); } } cleanup{gate};
        controller->setDecoder([gate](QByteArray bytes) {
            gate->entered.release();
            if (!gate->release.tryAcquire(1, 5000)) gate->timedOut = true;
            auto result = QMarkdownPrivate::decodeImage(std::move(bytes));
            gate->completed.release(); return result;
        });
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300, 300); window.show();
        view->setProperty("baseUrl", QUrl::fromLocalFile(temp.filePath("document.md")));
        auto *policy = view->property("resourcePolicy").value<MarkdownResourcePolicy *>(); QVERIFY(policy);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        view->setProperty("markdown", "![*alt*](image.png)");
        QVERIFY(gate->entered.tryAcquire(1, 2000));
        QTRY_COMPARE(painted(view).size(), 1); QTRY_COMPARE(painted(view)[0]->text(), "alt");
        if (revoke) policy->setAllowedFileRoots({});
        else view->setProperty("markdown", "**replacement**");
        QSignalSpy published(controller, &ResourceController::changed);
        gate->release.release();
        // Drain the worker before settling the view, so stale completion was exercised.
        QVERIFY(gate->completed.tryAcquire(1, 2000));
        QTRY_COMPARE(painted(view).size(), 1);
        QTRY_COMPARE(painted(view)[0]->text(), revoke ? "alt" : "replacement");
        QCoreApplication::processEvents();
        QVERIFY(named(view, "markdownImage").isEmpty()); QCOMPARE(published.size(), 0);
        QVERIFY(!gate->timedOut);
        controller->setDecoder(QMarkdownPrivate::decodeImage);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        view->setProperty("markdown", "![new](image.png)");
        QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        QCOMPARE(published.size(), 1);
    }
    void imageRowsAndPolicyLifecycle() {
        QTemporaryDir temp; QVERIFY(temp.isValid());
        QImage image(120, 60, QImage::Format_ARGB32); image.fill(Qt::green);
        QVERIFY(image.save(temp.filePath("image.png")));
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nMarkdownView { width: 240 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create()); auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300, 500); view->setParentItem(window.contentItem()); window.show();
        QSignalSpy links(view, SIGNAL(linkActivated(QString)));
        view->setProperty("markdown", "before **![*alt*](image.png)** after");
        QTRY_VERIFY(content(view) > 0); QVERIFY(named(view, "markdownImage").isEmpty());
        QCOMPARE(painted(view).size(), 1); QCOMPARE(painted(view)[0]->text(), "before alt after");
        QTRY_VERIFY(painted(view)[0]->layout());
        const auto fallbackFormats = painted(view)[0]->layout()->formats();
        QVERIFY(!fallbackFormats.isEmpty());
        bool formattedAlt = false;
        for (const auto &range : fallbackFormats)
            if (range.start <= 7 && range.start + range.length >= 10)
                formattedAlt |= range.format.font().italic() && range.format.font().weight() >= QFont::Bold;
        QVERIFY(formattedAlt);
        view->setProperty("baseUrl", QUrl::fromLocalFile(temp.filePath("document.md")));
        auto policy = std::make_unique<MarkdownResourcePolicy>();
        view->setProperty("resourcePolicy", QVariant::fromValue(policy.get()));
        QVERIFY(named(view, "markdownImage").isEmpty());
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        auto *row = named(view, "markdownImage")[0]; QTRY_COMPARE(row->width(), 120); QTRY_COMPARE(row->height(), 60);
        QCOMPARE(texts(view).size(), 2); QCOMPARE(texts(view)[0]->property("text").toString(), "before ");
        QCOMPARE(texts(view)[1]->property("text").toString(), " after");
        // Resource arrival creates rows before their enclosing Column is polished.
        QTRY_VERIFY(texts(view)[0]->height() > 0 && texts(view)[1]->height() > 0);
        QTRY_COMPARE(content(view), texts(view)[0]->height() + 60 + texts(view)[1]->height() + 16);
        const auto initialHeight = content(view);
        view->setWidth(40); QTRY_COMPARE(named(view, "markdownImage")[0]->width(), 40);
        QTRY_COMPARE(named(view, "markdownImage")[0]->height(), 20);
        QTRY_VERIFY(content(view) > 20); view->setWidth(0); QTRY_COMPARE(content(view), 0); QVERIFY(named(view, "markdownImage").isEmpty());
        view->setWidth(240); QTRY_COMPARE(content(view), initialHeight);
        policy->setAllowedFileRoots({}); QTRY_VERIFY(named(view, "markdownImage").isEmpty());
        QTRY_COMPARE(painted(view).size(), 1); QTRY_VERIFY(painted(view)[0]->layout());
        QCOMPARE(painted(view)[0]->text(), "before alt after");
        const auto restoredFormats = painted(view)[0]->layout()->formats();
        QCOMPARE(restoredFormats.size(), fallbackFormats.size());
        for (int i = 0; i < fallbackFormats.size(); ++i) {
            QCOMPARE(restoredFormats[i].start, fallbackFormats[i].start);
            QCOMPARE(restoredFormats[i].length, fallbackFormats[i].length);
            QCOMPARE(restoredFormats[i].format, fallbackFormats[i].format);
        }
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        view->setProperty("markdown", "[![](image.png)](outer:link)");
        QTRY_COMPARE(named(view, "markdownImage").size(), 1); QTRY_COMPARE(content(view), 60);
        row = named(view, "markdownImage")[0]; const QPoint hit = row->mapToScene(QPointF(20,20)).toPoint();
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, hit); QTRY_COMPARE(links.size(), 1); QCOMPARE(links[0][0].toString(), "outer:link");
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(200,20)); QCOMPARE(links.size(), 1);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, hit);
        view->setWidth(10); QTRY_COMPARE(row->width(), 10);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, hit); QCOMPARE(links.size(), 1);
        view->setWidth(240); QTRY_COMPARE(row->width(), 120);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, hit); QTest::mouseMove(&window, hit + QPoint(30,30));
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, hit + QPoint(30,30)); QCOMPARE(links.size(), 1);
        view->setProperty("markdown", "# before ![alt](image.png) after\n\n> - ![alt](image.png)![](image.png)");
        QTRY_COMPARE(named(view, "markdownImage").size(), 3);
        const auto rows = named(view, "markdownImage"); QCOMPARE(rows[1]->parentItem()->parentItem(), rows[2]->parentItem()->parentItem());
        QTRY_COMPARE(rows[2]->parentItem()->y(), rows[1]->parentItem()->y() + rows[1]->height());
        policy->setAllowedFileRoots({}); QTRY_VERIFY(named(view, "markdownImage").isEmpty());
        QTRY_COMPARE(texts(view).size(), 3);
        QCOMPARE(texts(view)[0]->property("text").toString(), "before alt after");
        QCOMPARE(texts(view)[1]->property("text").toString(), "•");
        QCOMPARE(texts(view)[2]->property("text").toString(), "alt");
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())}); QTRY_COMPARE(named(view, "markdownImage").size(), 3);
        policy.reset(); QTRY_VERIFY(named(view, "markdownImage").isEmpty());
        auto *defaults = view->property("resourcePolicy").value<MarkdownResourcePolicy *>(); QVERIFY(defaults);
        QVERIFY(defaults->allowedFileRoots().isEmpty()); QVERIFY(!defaults->allowQrc());
        defaults->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())}); QTRY_COMPARE(named(view, "markdownImage").size(), 3);
        QVERIFY(QQmlProperty(view, "resourcePolicy", &engine).reset()); QTRY_VERIFY(named(view, "markdownImage").isEmpty());
        defaults->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        view->setProperty("resourcePolicy", QVariant::fromValue<MarkdownResourcePolicy *>(nullptr)); QTRY_VERIFY(named(view, "markdownImage").isEmpty());
        view->setProperty("markdown", ""); QTRY_COMPARE(content(view), 0);
    }
    void linkedImageTouchAndScrolling() {
        QTemporaryDir temp; QImage image(120,300,QImage::Format_RGB32); image.fill(Qt::blue); QVERIFY(image.save(temp.filePath("image.png")));
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nFlickable { width: 180; height: 100; contentHeight: view.contentHeight; clip: true; MarkdownView { id: view; objectName: 'view'; width: 180; markdown: '[![alt](image.png)](image:link)' } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString())); std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); auto *view = object->findChild<QQuickItem *>("view"); QVERIFY(host); QVERIFY(view);
        QQuickWindow window; window.resize(200,120); host->setParentItem(window.contentItem()); window.show();
        view->setProperty("baseUrl", QUrl::fromLocalFile(temp.filePath("document.md")));
        auto *policy = view->property("resourcePolicy").value<MarkdownResourcePolicy *>(); QVERIFY(policy);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())}); QTRY_COMPARE(content(view), 300);
        QSignalSpy activation(view, SIGNAL(linkActivated(QString)));
        auto *device = QTest::createTouchDevice();
        QTest::touchEvent(&window, device).press(0, QPoint(20,20), &window);
        QTest::touchEvent(&window, device).release(0, QPoint(20,20), &window);
        QTRY_COMPARE(activation.size(), 1); QCOMPARE(activation.takeFirst()[0].toString(), "image:link");
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(20,80));
        QTest::mouseMove(&window, QPoint(20,55), 20); QTest::mouseMove(&window, QPoint(20,20), 20);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(20,20));
        QTRY_VERIFY(host->property("contentY").toDouble() > 0); QCOMPARE(activation.size(), 0);
        QVERIFY(QMetaObject::invokeMethod(host, "cancelFlick")); host->setProperty("contentY", 0);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(20,20));
        policy->setAllowedFileRoots({}); QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(20,20));
        QCOMPARE(activation.size(), 0); QTRY_VERIFY(named(view,"markdownImage").isEmpty());
    }
    void sharedImagePolicy() {
        QTemporaryDir temp; QImage image(80,40,QImage::Format_RGB32); image.fill(Qt::red); QVERIFY(image.save(temp.filePath("image.png")));
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nItem { width: 300; height: 300; MarkdownResourcePolicy { id: p; objectName: \"policy\" } MarkdownView { objectName: \"a\"; width: 150; resourcePolicy: p; markdown: \"![alt](image.png)\" } MarkdownView { objectName: \"b\"; width: 30; resourcePolicy: p; markdown: \"![alt](image.png)\" } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString())); std::unique_ptr<QObject> object(component.create());
        QQuickWindow window; window.resize(300,300); qobject_cast<QQuickItem *>(object.get())->setParentItem(window.contentItem()); window.show();
        auto *policy = object->findChild<MarkdownResourcePolicy *>("policy"); QVERIFY(policy);
        auto *a = object->findChild<QQuickItem *>("a"), *b = object->findChild<QQuickItem *>("b"); QVERIFY(a); QVERIFY(b);
        const auto base = QUrl::fromLocalFile(temp.filePath("document.md")); a->setProperty("baseUrl", base); b->setProperty("baseUrl", base);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        QTRY_COMPARE(named(a,"markdownImage").size(), 1); QTRY_COMPARE(named(b,"markdownImage").size(), 1);
        QTRY_COMPARE(content(a), 40); QTRY_COMPARE(content(b), 15);
        a->setProperty("baseUrl", QUrl()); QTRY_VERIFY(named(a,"markdownImage").isEmpty()); QCOMPARE(named(b,"markdownImage").size(), 1);
        policy->setAllowedFileRoots({}); QTRY_VERIFY(named(b,"markdownImage").isEmpty());
    }
    void initTestCase() {
#ifdef QMARKDOWN_VIEWER_SOURCE
        qml_register_types_QMarkdownViewer_Tools();
#endif
    }
    void init() {
        QTest::failOnWarning();
        savedApplicationFont = QGuiApplication::font();
        QFont font = savedApplicationFont; font.setPixelSize(16);
        QGuiApplication::setFont(font);
    }
    void cleanup() { QGuiApplication::setFont(savedApplicationFont); }
    void linkInteraction_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("destination");
        QTest::newRow("paragraph") << "[**link** `code`](custom:x&amp;y)" << "custom:x&y";
        QTest::newRow("heading") << "# [link](#part)" << "#part";
        QTest::newRow("nested") << "> - [link](../relative)" << "../relative";
        QTest::newRow("empty") << "[link]()" << "";
        QTest::newRow("autolink") << "<https://example.invalid/path>" << "https://example.invalid/path";
        QTest::newRow("image-in-link") << "[![link](https://example.invalid/img)](outer)" << "outer";
    }
    void linkInteraction()
    {
        QFETCH(QString, source); QFETCH(QString, destination);
        QQmlEngine engine;
        RequestFactory factory; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nMarkdownView { width: 250 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300, 300); view->setParentItem(window.contentItem()); window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        view->setProperty("markdown", source);
        QTRY_COMPARE(painted(view).size(), 1);
        auto *item = painted(view)[0]; QTRY_VERIFY(item->logicalHeight() > 0);
        QSignalSpy activation(view, SIGNAL(linkActivated(QString))); QVERIFY(activation.isValid());
        QTRY_VERIFY(item->layout() && item->layout()->lineCount() > 0);
        // Hit the first text line, including labels that wrap on desktop fonts.
        const auto line = item->layout()->lineAt(0);
        const QPoint point = item->mapToScene(QPointF(5, line.y() + line.height()/2)).toPoint();
        QCOMPARE(item->linkAt(5, line.y() + line.height()/2), 0);
        QTest::mouseMove(&window, QPoint(240, 250));
        QTest::mouseMove(&window, point);
        QTRY_COMPARE(item->property("hoveredLink").toInt(), 0);
        QTRY_COMPARE(window.cursor().shape(), Qt::PointingHandCursor);
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(activation.size(), 1); QCOMPARE(activation.takeFirst()[0].toString(), destination);
        QTest::mouseClick(&window, Qt::RightButton, Qt::NoModifier, point);
        QCOMPARE(activation.size(), 0);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, point);
        QTest::mouseMove(&window, point + QPoint(80, 50));
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(activation.size(), 0);
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(240, 10));
        QCOMPARE(activation.size(), 0);
        QTest::mouseMove(&window, QPoint(240, 10));
        QTRY_COMPARE(item->property("hoveredLink").toInt(), -1);
        QTRY_COMPARE(window.cursor().shape(), Qt::ArrowCursor);
        auto *device = QTest::createTouchDevice();
        QTest::touchEvent(&window, device).press(0, point, &window);
        QTest::touchEvent(&window, device).release(0, point, &window);
        QTRY_COMPARE(activation.size(), 1); QCOMPARE(activation.takeFirst()[0].toString(), destination);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, point);
        view->setProperty("markdown", "[replacement](other)");
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(activation.size(), 0);
        QTRY_COMPARE(painted(view).size(), 1);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, point);
        view->setProperty("markdown", "");
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, point);
        QCOMPARE(activation.size(), 0);
        QCOMPARE(factory.requests, 0);
    }
    void inactiveLinksAndScrolling()
    {
        QQmlEngine engine; RequestFactory factory; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\n"
                          "Flickable { width: 240; height: 100; contentHeight: view.contentHeight; clip: true; "
                          "MarkdownView { id: view; objectName: 'view'; width: 240 } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *view = host->findChild<QQuickItem *>("view"); QVERIFY(view);
        QQuickWindow window; window.resize(300, 200); host->setParentItem(window.contentItem()); window.show();
        QSignalSpy activation(view, SIGNAL(linkActivated(QString)));
        const QStringList inertSources = {"ordinary text", "![**image** [inner](inert)](https://example.invalid/image)",
                                         "`[code](inert)`", "    [code](inert)", "<a href='inert'>HTML</a>"};
        for (const auto &source : inertSources) {
            view->setProperty("markdown", source);
            QTRY_VERIFY(content(view) > 0);
            QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier, QPoint(10, 10));
            QCOMPARE(activation.size(), 0);
        }
        view->setProperty("markdown", "[" + QString("scrolling link words ").repeated(80) + "](scroll:link)");
        QTRY_COMPARE(painted(view).size(), 1); QTRY_VERIFY(content(view) > 100);
        QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, QPoint(20, 80));
        QTest::mouseMove(&window, QPoint(20, 55), 20);
        QTest::mouseMove(&window, QPoint(20, 20), 20);
        QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
        QTRY_VERIFY(host->property("contentY").toDouble() > 0);
        QCOMPARE(activation.size(), 0);
        QCOMPARE(factory.requests, 0);
    }
    void linkHitBoundariesAndLiveHover()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nMarkdownView { width: 240 }", {});
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300, 200); view->setParentItem(window.contentItem()); window.show();
        view->setProperty("markdown", "[abc  \ndef](one)[ghi](one)");
        QTRY_COMPARE(painted(view).size(), 1);
        auto *item = painted(view)[0]; QTRY_VERIFY(item->layout() && item->layout()->lineCount() == 2);
        const auto first = item->layout()->lineAt(0);
        const qreal y = first.height()/2 - item->y();
        QCOMPARE(item->linkAt(-0.1-item->x(), y), -1);
        QCOMPARE(item->linkAt(0.1-item->x(), y), 0);
        // A hard-break separator occupies no clickable cell.
        const qreal end = first.cursorToX(3);
        QCOMPARE(item->linkAt(end+0.1-item->x(), y), -1);
        QCOMPARE(item->linkAt(1-item->x(), -0.1-item->y()), -1);
        const auto second = item->layout()->lineAt(1);
        const qreal boundary = second.cursorToX(7);
        QCOMPARE(item->linkAt(boundary-0.1-item->x(), second.y()+second.height()/2-item->y()), 0);
        QCOMPARE(item->linkAt(boundary+0.1-item->x(), second.y()+second.height()/2-item->y()), 1);
        QSignalSpy activation(view, SIGNAL(linkActivated(QString)));
        QTest::mouseClick(&window, Qt::LeftButton, Qt::NoModifier,
                          item->mapToScene(QPointF(boundary+2-item->x(), second.y()+second.height()/2-item->y())).toPoint());
        QCOMPARE(activation.size(), 1); QCOMPARE(activation.takeFirst()[0].toString(), "one");
        view->setProperty("markdown", "[abc](one) plain");
        QTRY_COMPARE(painted(view).size(), 1); item = painted(view)[0];
        QTRY_VERIFY(item->layout());
        const QPoint hoverPoint(70, 8);
        QTest::mouseMove(&window, hoverPoint); QTRY_COMPARE(item->property("hoveredLink").toInt(), -1);
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        auto font = style->bodyFont(); font.setPixelSize(60); style->setBodyFont(font);
        QTRY_COMPARE(item->property("hoveredLink").toInt(), 0);
        font.setPixelSize(16); style->setBodyFont(font);
        QTRY_COMPARE(item->property("hoveredLink").toInt(), -1);
    }
    void linkStyleLifecycle()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\n"
                          "Item { MarkdownStyle { id: shared; objectName: 'shared' } "
                          "MarkdownStyle { id: other; objectName: 'other'; linkColor: '#112233'; linkUnderline: false } "
                          "MarkdownView { objectName: 'a'; width: 200; markdown: '[a](x)'; style: shared } "
                          "MarkdownView { objectName: 'b'; width: 200; markdown: '[b](x)'; style: shared } }", {});
        std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *a = host->findChild<QQuickItem *>("a"), *b = host->findChild<QQuickItem *>("b");
        auto *shared = host->findChild<MarkdownStyle *>("shared"), *other = host->findChild<MarkdownStyle *>("other");
        QVERIFY(a && b && shared && other);
        QQuickWindow window; host->setParentItem(window.contentItem()); window.show();
        QTRY_VERIFY(!painted(a).isEmpty() && !painted(b).isEmpty());
        QTRY_VERIFY(painted(a)[0]->layout() && painted(b)[0]->layout());
        shared->setLinkColor(QColor("#998877")); shared->setLinkUnderline(false);
        for (auto *view : {a, b}) {
            QTRY_COMPARE(painted(view)[0]->layout()->formats()[0].format.foreground().color(), QColor("#998877"));
            QVERIFY(!painted(view)[0]->layout()->formats()[0].format.fontUnderline());
        }
        a->setProperty("style", QVariant::fromValue(other));
        QTRY_COMPARE(painted(a)[0]->linkColor(), QColor("#112233"));
        delete other;
        QTRY_COMPARE(painted(a)[0]->linkColor(), QColor("#0066cc"));
        QTRY_VERIFY(painted(a)[0]->linkUnderline());
        delete shared;
        QTRY_COMPARE(painted(b)[0]->linkColor(), QColor("#0066cc"));
        QTRY_VERIFY(painted(b)[0]->linkUnderline());
    }
    void paddedCodePointerInteraction()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.6\nMarkdownView { width: 180; markdown: 'before [`long code words`](host:code) after' }", {});
        std::unique_ptr<QObject> object(component.create());
        auto *view=qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300,300); view->setParentItem(window.contentItem()); window.show();
        QTRY_COMPARE(painted(view).size(),1); auto *item=painted(view)[0];
        QTRY_VERIFY(item->layout() && !item->codeRectangles().isEmpty());
        QCOMPARE(item->codeHorizontalPadding(),4.0); QCOMPARE(item->codeVerticalPadding(),2.0);
        QSignalSpy activated(view,SIGNAL(linkActivated(QString)));
        for(const auto &rect : item->codeRectangles()) {
            for(qreal x : {rect.left()+1,rect.right()-1}) {
                const QPoint point=item->mapToScene(QPointF(x-item->x(),rect.center().y()-item->y())).toPoint();
                QTest::mouseMove(&window,point); QTRY_COMPARE(item->property("hoveredLink").toInt(),0);
                QTest::mouseClick(&window,Qt::LeftButton,Qt::NoModifier,point);
                QVERIFY(!activated.isEmpty()); QCOMPARE(activated.takeFirst()[0].toString(),"host:code");
            }
        }
        const auto rect=item->codeRectangles().first();
        const QPoint point=item->mapToScene(QPointF(rect.left()+1-item->x(),rect.center().y()-item->y())).toPoint();
        QTest::mousePress(&window,Qt::LeftButton,Qt::NoModifier,point);
        QTest::mouseMove(&window,point+QPoint(60,50));
        QTest::mouseRelease(&window,Qt::LeftButton,Qt::NoModifier,point+QPoint(60,50));
        QCOMPARE(activated.size(),0);
        auto *style=view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        style->inlineCode()->setHorizontalPadding(12); QTRY_COMPARE(item->codeHorizontalPadding(),12.0);
        style->inlineCode()->resetHorizontalPadding(); QTRY_COMPARE(item->codeHorizontalPadding(),4.0);
    }
    void linkLayout_data()
    {
        QTest::addColumn<bool>("points");
        QTest::newRow("pixels") << false;
        QTest::newRow("points") << true;
    }
    void linkLayout()
    {
        QFETCH(bool, points);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nMarkdownView { width: 110 }", {});
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.show();
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        // This test deliberately verifies the historical zero-padding cursor geometry.
        style->inlineCode()->setHorizontalPadding(0); style->inlineCode()->setVerticalPadding(0);
        auto font = style->bodyFont();
        if (points) font.setPointSizeF(13.25); else font.setPixelSize(18);
        style->setBodyFont(font);
        view->setProperty("markdown", QString::fromUtf8("plain [**long** *link* `code` אבג 😀 more words](one)[next](one)"));
        QTRY_COMPARE(painted(view).size(), 1);
        auto *item = painted(view)[0]; QTRY_VERIFY(item->layout() && item->layout()->lineCount() > 1);
        const auto checkHits = [&] {
            const auto *layout = item->layout();
            const auto links = item->linkSpans();
            for (int i = 0; i < layout->lineCount(); ++i) {
                const auto line = layout->lineAt(i);
                for (int pos = line.textStart(); pos < line.textStart() + line.textLength(); ++pos) {
                    if (!layout->isValidCursorPosition(pos) || item->text()[pos] == u'\n') continue;
                    const qreal a = line.cursorToX(pos, QTextLine::Leading), b = line.cursorToX(pos, QTextLine::Trailing);
                    if (qAbs(a - b) < 0.1) continue;
                    int expected = -1;
                    for (int k = 0; k < links.size(); ++k) {
                        const auto link = links[k].toMap();
                        if (pos >= link["start"].toInt() && pos < link["start"].toInt() + link["length"].toInt()) expected = k;
                    }
                    QCOMPARE(item->linkAt((a+b)/2 - item->x(), line.y()+line.height()/2 - item->y()), expected);
                }
                QCOMPARE(item->linkAt(line.naturalTextWidth()+10-item->x(), line.y()+line.height()/2-item->y()), -1);
            }
        };
        checkHits();
        bool strong = false, emphasis = false, code = false;
        for (const auto &format : item->layout()->formats()) {
            QCOMPARE(format.format.foreground().color(), style->linkColor());
            QVERIFY(format.format.fontUnderline());
            strong |= format.format.font().weight() >= QFont::Bold;
            emphasis |= format.format.font().italic();
            code |= format.format.font().family() == style->inlineCodeFont().family();
        }
        QVERIFY(strong && emphasis && code);
        style->setLinkColor(QColor("#cc3322")); style->setLinkUnderline(false);
        QTRY_COMPARE(item->layout()->formats()[0].format.foreground().color(), QColor("#cc3322"));
        QVERIFY(!item->layout()->formats()[0].format.fontUnderline());
        view->setWidth(240); QTRY_COMPARE(item->layoutWidth(), 240); QTRY_VERIFY(item->width() >= 240);
        checkHits();
        QSignalSpy colorSpy(style, &MarkdownStyle::linkColorChanged), underlineSpy(style, &MarkdownStyle::linkUnderlineChanged);
        style->setLinkColor(style->linkColor()); style->setLinkUnderline(false);
        QCOMPARE(colorSpy.size(), 0); QCOMPARE(underlineSpy.size(), 0);
        style->restoreDefaults(); QCOMPARE(style->linkColor(), QColor("#0066cc")); QVERIFY(style->linkUnderline());
        QCOMPARE(colorSpy.size(), 1); QCOMPARE(underlineSpy.size(), 1);
    }
    void applicationTypography_data()
    {
        QTest::addColumn<bool>("pixels");
        QTest::newRow("pixels") << true;
        QTest::newRow("fractional-points") << false;
    }
    void applicationTypography()
    {
        QFETCH(bool, pixels);
        QFont application = QGuiApplication::font();
        if (pixels) application.setPixelSize(13);
        else application.setPointSizeF(10.25);
        application.setWeight(QFont::Black);
        QGuiApplication::setFont(application);
        MarkdownStyle style;
        QCOMPARE(style.bodyFont().weight(), QFont::Normal);
        QCOMPARE(style.bodyFont().pixelSize(), application.pixelSize());
        QCOMPARE(style.bodyFont().pointSizeF(), application.pointSizeF());
        const qreal ratios[] = {2, 1.75, 1.5, 1.25, 1.125, 1};
        const QFont headings[] = {style.h1Font(), style.h2Font(), style.h3Font(),
                                  style.h4Font(), style.h5Font(), style.h6Font()};
        for (int i = 0; i < 6; ++i) {
            QCOMPARE(headings[i].weight(), QFont::Bold);
            if (pixels) QCOMPARE(headings[i].pixelSize(), qRound(13 * ratios[i]));
            else QCOMPARE(headings[i].pointSizeF(), 10.25 * ratios[i]);
        }
        QCOMPARE(style.codeBlockFont().pixelSize(), application.pixelSize());
        QCOMPARE(style.codeBlockFont().pointSizeF(), application.pointSizeF());
        QCOMPARE(style.codeBlockFont().weight(), QFont::Normal);
        QCOMPARE(style.codeBlockFont().family(), QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
        for (const auto &block : {style.bodyFont(), style.h1Font()}) {
            const auto code = style.inlineCodeFont().resolve(block);
            QCOMPARE(code.pixelSize(), block.pixelSize());
            QCOMPARE(code.pointSizeF(), block.pointSizeF());
        }
        const auto snapshot = style.bodyFont();
        QSignalSpy changes(&style, &MarkdownStyle::bodyFontChanged);
        QFont edited = snapshot; edited.setPointSizeF(23.75);
        style.setBodyFont(edited); style.setBodyFont(edited);
        QCOMPARE(changes.count(), 1);
        QCOMPARE(style.h1Font(), headings[0]);
        QCOMPARE(style.codeBlockFont().pointSizeF(), application.pointSizeF());
        QCOMPARE(style.codeBlockFont().pixelSize(), application.pixelSize());
        edited.setPixelSize(27); style.setBodyFont(edited);
        QCOMPARE(changes.count(), 2);
        application.setPointSizeF(41); QGuiApplication::setFont(application);
        style.restoreDefaults();
        QCOMPARE(style.bodyFont(), snapshot);
        QCOMPARE(changes.count(), 3);
        MarkdownStyle fresh;
        QCOMPARE(fresh.bodyFont().pointSizeF(), 41.0);
    }
    void nativePointLayout_data()
    {
        QTest::addColumn<qreal>("points");
        QTest::newRow("eighth") << qreal(13.125);
        QTest::newRow("quarter") << qreal(13.25);
        QTest::newRow("near-half") << qreal(13.49);
    }
    void nativePointLayout()
    {
        QFETCH(qreal, points);
        QFont application = QGuiApplication::font(); application.setPointSizeF(points);
        QGuiApplication::setFont(application);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown\nMarkdownView { width: 150; markdown: 'same words wrap over several lines here\\n\\n*same words wrap over several lines here*' }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.show();
        QTRY_COMPARE(painted(view).size(), 1);
        auto *inlineText = painted(view)[0]; inlineText->setFormatRanges({});
        QTRY_VERIFY(inlineText->layout());
        QTRY_COMPARE(inlineText->logicalHeight(), texts(view)[0]->height());
        qreal lineWidth = 0;
        for (int i = 0; i < inlineText->layout()->lineCount(); ++i)
            lineWidth = qMax(lineWidth, inlineText->layout()->lineAt(i).naturalTextWidth());
        QCOMPARE(lineWidth, texts(view)[0]->property("contentWidth").toDouble());
        // Returning to the application font after another assignment goes
        // through native Text's setter and therefore does round to half points.
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        QFont edited = style->bodyFont(); edited.setPointSizeF(24.75); style->setBodyFont(edited);
        QTRY_COMPARE(inlineText->layout()->font().pointSizeF(), 25.0);
        edited.setPointSizeF(points); style->setBodyFont(edited);
        QTRY_COMPARE(inlineText->logicalHeight(), texts(view)[0]->height());
        QCOMPARE(inlineText->font().pointSizeF(), points);
    }
    void pointRenderingAndRuntimeUnits()
    {
        QFont application = QGuiApplication::font(); application.setPointSizeF(13.25);
        QGuiApplication::setFont(application);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\nimport QMarkdown\n"
            "Item { width: 250; height: 600\n"
            " MarkdownStyle { id: shared; objectName: 'shared'; bodyFont.pointSize: 13.25; h1Font.pointSize: 19.5 }\n"
            " MarkdownView { id: a; objectName: 'a'; width: 180; style: shared; markdown: 'same words wrap over several lines here\\n\\n*same words wrap over several lines here*\\n\\n# `heading`\\n\\n`body`' }\n"
            " MarkdownView { objectName: 'b'; width: 180; style: shared; markdown: a.markdown }\n"
            " function points() { shared.bodyFont.pointSize = 24.75 }\n"
            " function pixels() { shared.bodyFont.pixelSize = 12 }\n"
            " MarkdownStyle { id: other; objectName: 'pointReplacement'; bodyFont.pointSize: 17.5 }\n"
            " function replace() { a.style = other }\n"
            " function reset() { a.style = null }\n"
            "}", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *a = host->findChild<QQuickItem *>("a");
        auto *b = host->findChild<QQuickItem *>("b");
        auto *style = host->findChild<MarkdownStyle *>("shared"); QVERIFY(a && b && style);
        QQuickWindow window; host->setParentItem(window.contentItem()); window.show();
        QTRY_COMPARE(painted(a).size(), 3);
        QTRY_VERIFY(painted(a)[0]->layout());
        // Italic can change widths; compare unformatted native Text against a
        // formatted renderer with the same font and content directly.
        auto *formatted = painted(a)[0];
        const auto originalRanges = formatted->formatRanges();
        formatted->setFormatRanges({});
        QTRY_COMPARE(formatted->logicalHeight(), texts(a)[0]->height());
        formatted->setFormatRanges(originalRanges);
        QTRY_COMPARE(content(a), content(b));
        const auto headingCode = painted(a)[1]->layout()->formats()[0].format.font();
        QCOMPARE(headingCode.pointSizeF(), 19.5);
        QCOMPARE(painted(a)[2]->layout()->formats()[0].format.font().pointSizeF(), 13.25);
        const auto initial = content(a);
        QSignalSpy notifications(style, &MarkdownStyle::bodyFontChanged);
        QVERIFY(QMetaObject::invokeMethod(host, "points"));
        QTRY_VERIFY(content(a) > initial); QTRY_COMPARE(content(a), content(b));
        QCOMPARE(style->bodyFont().pointSizeF(), 24.75);
        QCOMPARE(style->bodyFont().pixelSize(), -1);
        QCOMPARE(notifications.count(), 1);
        QTest::ignoreMessage(QtWarningMsg, "Both point size and pixel size set. Using pixel size.");
        QVERIFY(QMetaObject::invokeMethod(host, "pixels"));
        QTRY_VERIFY(content(a) < initial); QTRY_COMPARE(content(a), content(b));
        QCOMPARE(style->bodyFont().pixelSize(), 12);
        QCOMPARE(style->bodyFont().pointSizeF(), -1.0);
        QCOMPARE(notifications.count(), 2);
        QTest::ignoreMessage(QtWarningMsg, "Both point size and pixel size set. Using pixel size.");
        QVERIFY(QMetaObject::invokeMethod(host, "points"));
        QCOMPARE(style->bodyFont().pixelSize(), 12);
        QCOMPARE(notifications.count(), 2);
        QFont whole = style->bodyFont(); whole.setPointSizeF(31.5);
        style->setBodyFont(whole);
        QTRY_VERIFY(content(a) > initial);
        // Screen/DPI notifications must invalidate cached private layouts.
        QSignalSpy layoutChanges(formatted, &FormattedText::layoutChanged);
        QVERIFY(QMetaObject::invokeMethod(window.screen(), "logicalDotsPerInchChanged", Q_ARG(qreal, window.screen()->logicalDotsPerInch())));
        QTRY_VERIFY(layoutChanges.count() > 0);
        QVERIFY(QMetaObject::invokeMethod(host, "reset"));
        QTRY_COMPARE(a->property("style").value<QObject *>()->property("bodyFont").value<QFont>().pointSizeF(), 13.25);
        QTRY_VERIFY(content(a) < content(b));
        QVERIFY(QMetaObject::invokeMethod(host, "replace"));
        QTRY_COMPARE(a->property("style").value<QObject *>()->property("bodyFont").value<QFont>().pointSizeF(), 17.5);
        delete host->findChild<MarkdownStyle *>("pointReplacement");
        QTRY_COMPARE(a->property("style").value<QObject *>()->property("bodyFont").value<QFont>().pointSizeF(), 13.25);
        const auto beforeScreenChange = layoutChanges.count();
        QVERIFY(QMetaObject::invokeMethod(&window, "screenChanged", Q_ARG(QScreen *, window.screen())));
        QTRY_VERIFY(layoutChanges.count() > beforeScreenChange);

    }
    void containerLayoutAndStyle()
    {
        RequestFactory factory;
        QQmlEngine engine; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.0\nMarkdownView { width: 280 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        auto *style = view->property("style").value<QObject *>(); QVERIFY(style);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(400, 600); window.show();
        auto find = [&](const QString &text) -> QQuickItem * {
            for (auto *item : texts(view)) if (item->property("text").toString() == text) return item;
            return nullptr;
        };
        view->setProperty("markdown", "9) first\n10) second");
        QTRY_VERIFY(find("9)") && find("10)") && find("first") && find("second"));
        QTRY_VERIFY(content(view) > 0);
        auto *first = find("first"); auto *second = find("second");
        auto *row1 = first->parentItem()->parentItem()->parentItem();
        auto *row2 = second->parentItem()->parentItem()->parentItem();
        QTRY_COMPARE(row2->y(), row1->height());
        QCOMPARE(find("9)")->y(), 0.0);
        QCOMPARE(first->parentItem()->parentItem()->x(), qMax(24.0, QFontMetricsF(style->property("bodyFont").value<QFont>()).horizontalAdvance("10)") + 8));
        const auto bodyFont = style->property("bodyFont").value<QFont>();
        auto largerFont = bodyFont; largerFont.setPixelSize(40);
        style->setProperty("bodyFont", largerFont);
        QTRY_COMPARE(first->parentItem()->parentItem()->x(), QFontMetricsF(largerFont).horizontalAdvance("10)") + 8);
        style->setProperty("bodyFont", bodyFont);
        view->setProperty("markdown", "- first\n\n- second");
        QTRY_VERIFY(find("first") && find("second"));
        first = find("first"); second = find("second");
        row1 = first->parentItem()->parentItem()->parentItem(); row2 = second->parentItem()->parentItem()->parentItem();
        QTRY_COMPARE(row2->y(), row1->height() + 8);
        view->setProperty("markdown", "- outer\n  - nestedA\n\n  - nestedB\n- end");
        QTRY_VERIFY(find("nestedA") && find("nestedB"));
        first = find("nestedA"); second = find("nestedB");
        row1 = first->parentItem()->parentItem()->parentItem(); row2 = second->parentItem()->parentItem()->parentItem();
        QTRY_COMPARE(row2->y(), row1->height() + 8);
        view->setProperty("markdown", "> quoted\n>\n> another");
        QTRY_VERIFY(find("quoted") && find("another"));
        QTRY_COMPARE(named(view, "quoteRule").size(), 1);
        auto *rule = named(view, "quoteRule")[0];
        QTRY_COMPARE(rule->width(), 2.0);
        QTRY_COMPARE(rule->height(), content(view));
        QTRY_COMPARE(find("another")->y(), find("quoted")->height() + 8);
        QCOMPARE(find("quoted")->parentItem()->parentItem()->x(), 16.0);
        QSignalSpy indent(style, SIGNAL(quoteIndentChanged()));
        QSignalSpy thickness(style, SIGNAL(quoteRuleThicknessChanged()));
        QSignalSpy listIndent(style, SIGNAL(listIndentChanged()));
        QSignalSpy color(style, SIGNAL(quoteRuleColorChanged()));
        style->setProperty("quoteIndent", 32); style->setProperty("quoteIndent", 32);
        QCOMPARE(indent.count(), 1);
        QTRY_COMPARE(find("quoted")->parentItem()->parentItem()->x(), 32.0);
        style->setProperty("quoteRuleThickness", 4); style->setProperty("quoteRuleColor", QColor("#abcdef"));
        QCOMPARE(thickness.count(), 1); QCOMPARE(color.count(), 1);
        QTRY_COMPARE(rule->width(), 4.0); QCOMPARE(rule->property("color").value<QColor>(), QColor("#abcdef"));
        const auto nan = std::numeric_limits<double>::quiet_NaN();
        style->setProperty("quoteIndent", nan); style->setProperty("quoteIndent", nan);
        QCOMPARE(indent.count(), 2);
        QTRY_COMPARE(find("quoted")->parentItem()->parentItem()->x(), 16.0);
        style->setProperty("quoteRuleThickness", -2); QTRY_COMPARE(rule->width(), 0.0);
        style->setProperty("quoteRuleThickness", std::numeric_limits<double>::infinity()); QTRY_COMPARE(rule->width(), 2.0);
        view->setWidth(1); QTRY_COMPARE(find("quoted")->width(), 1.0);
        QTRY_COMPARE(rule->width(), 0.0);
        view->setWidth(0); QTRY_COMPARE(content(view), 0.0); QTRY_VERIFY(texts(view).isEmpty());
        view->setWidth(280); QTRY_VERIFY(find("quoted"));
        view->setProperty("markdown", ">");
        QTRY_COMPARE(content(view), QFontMetricsF(style->property("bodyFont").value<QFont>()).height());
        view->setProperty("markdown", "-\n-");
        QTRY_COMPARE(content(view), 2 * QFontMetricsF(style->property("bodyFont").value<QFont>()).height());
        view->setProperty("markdown", ">\n\n-\n- > nested\n  > - item");
        QTRY_VERIFY(find("nested") && find("item")); QTRY_VERIFY(content(view) > 0);
        style->setProperty("listIndent", 50); style->setProperty("listIndent", 50); QCOMPARE(listIndent.count(), 1);
        view->setWidth(2); QTRY_VERIFY(content(view) > 0);
        view->setWidth(280);
        view->setProperty("markdown", "- **one**  \n  two\n- ![*image*](https://example.invalid/x)\n\n> [link](https://example.invalid)");
        QTRY_COMPARE(painted(view).size(), 3); QCOMPARE(painted(view)[2]->text(), "link");
        QTRY_VERIFY(painted(view)[0]->logicalHeight() > QFontMetricsF(style->property("bodyFont").value<QFont>()).height());
        QCOMPARE(factory.requests, 0);
        view->setProperty("markdown", "replacement"); QTRY_COMPARE(texts(view).size(), 1);
        QTRY_VERIFY(named(view, "quoteRule").isEmpty());
        view->setProperty("markdown", ""); QTRY_COMPARE(content(view), 0.0);
        view->setProperty("markdown", "> first\n>\n> second");
        QTRY_VERIFY(find("first"));
        QQmlComponent styleComponent(&engine);
        styleComponent.setData("import QMarkdown 1.0\nMarkdownStyle { quoteIndent: 48; quoteRuleThickness: 5 }", {});
        std::unique_ptr<QObject> shared(styleComponent.create()); QVERIFY(shared);
        std::unique_ptr<QObject> secondObject(component.create());
        auto *other = qobject_cast<QQuickItem *>(secondObject.get()); QVERIFY(other);
        other->setParentItem(window.contentItem());
        other->setProperty("markdown", "> other");
        view->setProperty("style", QVariant::fromValue(shared.get()));
        other->setProperty("style", QVariant::fromValue(shared.get()));
        QTRY_COMPARE(find("first")->parentItem()->parentItem()->x(), 48.0);
        QTRY_COMPARE(named(other, "quoteRule").size(), 1);
        QTRY_COMPARE(named(other, "quoteRule")[0]->width(), 5.0);
        shared->setProperty("quoteRuleThickness", 7);
        QTRY_COMPARE(named(view, "quoteRule")[0]->width(), 7.0);
        QTRY_COMPARE(named(other, "quoteRule")[0]->width(), 7.0);
        QVERIFY(QQmlProperty(view, "style", &engine).reset());
        QTRY_COMPARE(find("first")->parentItem()->parentItem()->x(), 16.0);
        QTRY_COMPARE(named(view, "quoteRule")[0]->width(), 2.0);
        shared.reset(); QTRY_COMPARE(named(other, "quoteRule")[0]->width(), 2.0);
        QCOMPARE(other->property("style").value<QObject *>()->property("listIndent").toDouble(), 24.0);
    }
    void leafLayoutAndStyle()
    {
        RequestFactory factory;
        QQmlEngine engine; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData(
            "import QtQuick\nimport QMarkdown 1.0\n"
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
            "import QtQuick\nimport QMarkdown 1.0\n"
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
        for (int i = 1; i < items.size(); ++i)
            QTRY_COMPARE(items[i]->mapToItem(a, QPointF()).y(),
                         items[i-1]->mapToItem(a, QPointF()).y() + items[i-1]->height() + 8);
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
    void combinedFormatsAndLiveRanges()
    {
        FormattedText item;
        QQuickWindow window; item.setParentItem(window.contentItem());
        QFont body("Noto Sans"); body.setPixelSize(18); item.setFont(body);
        QFont code("Noto Sans Mono"); code.setPixelSize(20); item.setCodeFont(code);
        item.setText(QString::fromUtf8("a😀b code")); item.setLayoutWidth(100);
        const auto span = [](int start, int length, int flags = 0) {
            return QVariantMap{{"start", start}, {"length", length}, {"flags", flags}};
        };
        item.setFormatRanges({span(1, 2, 1), span(1, 2, 2), span(1, 2, 4)});
        item.setLinkSpans({span(0, 4)}); item.setLinkColor(QColor("#123456"));
        item.ensurePolished();
        QVERIFY(item.layout()); QCOMPARE(item.layout()->formats().size(), 3);
        const auto combined = item.layout()->formats()[1];
        QCOMPARE(combined.start, 1); QCOMPARE(combined.length, 2);
        QCOMPARE(combined.format.font().family(), code.family());
        QCOMPARE(combined.format.font().pixelSize(), 20);
        QVERIFY(combined.format.font().italic());
        QCOMPARE(combined.format.font().weight(), QFont::Bold);
        QCOMPARE(combined.format.foreground().color(), QColor("#123456"));
        QVERIFY(combined.format.fontUnderline());
        item.setFormatRanges({span(5, 4, 2)}); item.setLinkSpans({span(5, 4)});
        item.setLinkUnderline(false); item.ensurePolished();
        QCOMPARE(item.layout()->formats().size(), 1);
        const auto replacement = item.layout()->formats()[0];
        QCOMPARE(replacement.start, 5); QCOMPARE(replacement.length, 4);
        QCOMPARE(replacement.format.font().family(), body.family());
        QVERIFY(!replacement.format.font().italic());
        QCOMPARE(replacement.format.font().weight(), QFont::Bold);
        QVERIFY(!replacement.format.fontUnderline());
        QCOMPARE(replacement.format.foreground().color(), QColor("#123456"));
        item.setFormatRanges({}); item.setLinkSpans({}); item.ensurePolished();
        QVERIFY(item.layout()->formats().isEmpty());
        QVERIFY(item.logicalHeight() > 0);
    }

    void formattedFontAndLayout()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData(
            "\n"
            "            import QtQuick\n"
            "            import QMarkdown 1.0\n"
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
        QTRY_VERIFY(body->layout()->lineAt(0).width() >= body->layout()->lineAt(0).naturalTextWidth());
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
        const auto documentTexts = [preview] { return painted(preview); };
        QTRY_COMPARE(documentTexts().size(), 9);
        for (const bool dark : {false, true}) {
            QVERIFY(setPalette(dark));
            const QColor expected(dark ? "#eeeeee" : "#202020");
            QTRY_COMPARE(surface->property("color").value<QColor>(), QColor(dark ? "#18212b" : "#ffffff"));
            QTRY_COMPARE(label->property("color").value<QColor>(), expected);
            for (const auto &role : roles) QTRY_COMPARE(color(role), expected);
            for (auto *text : documentTexts()) QTRY_COMPARE(text->property("color").value<QColor>(), expected);
            QTRY_COMPARE(painted(preview)[7]->color(), expected);
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
        QTRY_COMPARE(painted(preview)[7]->color(), QColor("#123456"));
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
        QVERIFY(QMetaObject::invokeMethod(panel, "applyQuoteColor", Q_RETURN_ARG(QVariant, accepted), Q_ARG(QVariant, "invalid-color")));
        QVERIFY(!accepted.toBool());
        QVERIFY(QMetaObject::invokeMethod(panel, "applyQuoteColor", Q_RETURN_ARG(QVariant, accepted), Q_ARG(QVariant, "#345678")));
        QVERIFY(accepted.toBool());
        QVERIFY(setPalette(false)); QTRY_COMPARE(color("quoteRule"), QColor("#345678"));
        style->setProperty("listIndent", 60); style->setProperty("quoteIndent", 40); style->setProperty("quoteRuleThickness", 6);
        QVERIFY(QMetaObject::invokeMethod(window, "resetStyle"));
        QCOMPARE(style->property("listIndent").toDouble(), 24.0);
        QCOMPARE(style->property("quoteIndent").toDouble(), 16.0);
        QCOMPARE(style->property("quoteRuleThickness").toDouble(), 2.0);
        QTRY_COMPARE(color("quoteRule"), QColor("#707070"));
        QVERIFY(setPalette(true)); QTRY_COMPARE(color("quoteRule"), QColor("#a0a0a0"));
        QVERIFY(QMetaObject::invokeMethod(window, "switchStyle"));
        QTRY_COMPARE(color("quoteRule"), QColor("#d8a0e5"));
        QCOMPARE(style->property("listIndent").toDouble(), 32.0);
        QCOMPARE(style->property("quoteIndent").toDouble(), 24.0);
        QCOMPARE(style->property("quoteRuleThickness").toDouble(), 3.0);
        QVERIFY(QMetaObject::invokeMethod(window, "resetStyle"));
        window->setProperty("sampleIndex", 8);
        QTRY_VERIFY(!painted(preview).isEmpty());
        auto *linkedHeading = painted(preview)[0];
        QTRY_VERIFY(linkedHeading->logicalHeight() > 0);
        auto *destinationLabel = window->findChild<QObject *>("lastDestination"); QVERIFY(destinationLabel);
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
                          linkedHeading->mapToScene(QPointF(8-linkedHeading->x(), 12-linkedHeading->y())).toPoint());
        QTRY_COMPARE(destinationLabel->property("text").toString(), "Last activated: #heading");
        QCOMPARE(destinationLabel->property("textFormat").toInt(), 0);
        QVERIFY(QQmlProperty::write(window, "palette.link", QColor("#2299bb")));
        QTRY_COMPARE(color("link"), QColor("#2299bb"));
        auto *linkField = window->findChild<QObject *>("linkColorField"); QVERIFY(linkField);
        linkField->setProperty("text", "invalid-color");
        QVERIFY(QMetaObject::invokeMethod(linkField, "editingFinished"));
        QCOMPARE(color("link"), QColor("#2299bb"));
        linkField->setProperty("text", "#334455");
        QVERIFY(QMetaObject::invokeMethod(linkField, "editingFinished"));
        QCOMPARE(color("link"), QColor("#334455"));
        QVERIFY(QMetaObject::invokeMethod(window, "switchStyle"));
        QVERIFY(!style->property("linkUnderline").toBool());
        QVERIFY(setPalette(false)); QTRY_COMPARE(color("link"), QColor("#305b9c"));
        QVERIFY(setPalette(true)); QTRY_COMPARE(color("link"), QColor("#8db9f2"));
        QVERIFY(QMetaObject::invokeMethod(window, "resetStyle"));
        QTRY_COMPARE(color("link"), QColor("#2299bb")); QVERIFY(style->property("linkUnderline").toBool());
        if (qEnvironmentVariableIsSet("QMARKDOWN_CAPTURE_THEME")) {
            window->resize(1000, 1250);
            window->setProperty("sampleIndex", 9);
            window->findChild<QObject *>("styleToggle")->setProperty("checked", true);
            QVERIFY(QMetaObject::invokeMethod(window, "loadSample"));
            QTRY_COMPARE(named(preview, "markdownImage").size(), 2);
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
    void viewerFontUnits()
    {
        QFont application = QGuiApplication::font(); application.setPointSizeF(11.25);
        QGuiApplication::setFont(application);
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl::fromLocalFile(QStringLiteral(QMARKDOWN_VIEWER_SOURCE)));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *window = qobject_cast<QQuickWindow *>(object.get()); QVERIFY(window);
        auto *panel = window->findChild<QObject *>("stylePanel"); QVERIFY(panel);
        auto *preview = window->findChild<QQuickItem *>("preview"); QVERIFY(preview);
        auto *style = preview->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        auto *units = window->findChild<QObject *>("unitSelector"); QVERIFY(units);
        auto *size = window->findChild<QObject *>("sizeField"); QVERIFY(size);
        QTRY_COMPARE(units->property("currentIndex").toInt(), 1);
        QCOMPARE(size->property("value").toInt(), 1125);
        const auto initialHeight = content(preview);
        QFont decorated = style->bodyFont();
        decorated.setUnderline(true); decorated.setLetterSpacing(QFont::AbsoluteSpacing, 1.25);
        style->setBodyFont(decorated);
        const auto mask = decorated.resolveMask();
        const auto dpi = panel->property("logicalDpi").toDouble(); QVERIFY(dpi > 0);
        QVERIFY(QMetaObject::invokeMethod(panel, "changeUnit", Q_ARG(QVariant, false)));
        QTRY_COMPARE(units->property("currentIndex").toInt(), 0);
        QCOMPARE(style->bodyFont().pixelSize(), qRound(11.25 * dpi / 72));
        QVERIFY(style->bodyFont().underline());
        QCOMPARE(style->bodyFont().letterSpacing(), 1.25);
        QCOMPARE(style->bodyFont().resolveMask(), mask);
        QVERIFY(QMetaObject::invokeMethod(panel, "changeUnit", Q_ARG(QVariant, true)));
        QCOMPARE(style->bodyFont().pointSizeF(), qRound(11.25 * dpi / 72) * 72.0 / dpi);
        QCOMPARE(style->bodyFont().resolveMask(), mask);
        QTRY_COMPARE(units->property("currentIndex").toInt(), 1);
        // Exercise the SpinBox's fractional conversion and actual user input.
        const auto parser = engine.newQObject(size).property("valueFromText");
        const auto parsed = parser.call({QJSValue("23.75"), engine.evaluate("Qt.locale('en_US')")});
        QVERIFY2(!parsed.isError(), qPrintable(parsed.toString()));
        QCOMPARE(parsed.toInt(), 2375);
        auto *sizeInput = size->property("contentItem").value<QQuickItem *>(); QVERIFY(sizeInput);
        sizeInput->forceActiveFocus();
        QVERIFY(QMetaObject::invokeMethod(sizeInput, "selectAll"));
        for (const QChar ch : QString("23.75")) QTest::keyClick(window, ch.toLatin1());
        QTest::keyClick(window, Qt::Key_Return);
        QTRY_COMPARE(size->property("value").toInt(), 2375);
        QTRY_COMPARE(style->bodyFont().pointSizeF(), 23.75);
        QTRY_VERIFY(content(preview) > initialHeight);
        const auto inherited = style->inlineCodeFont();
        panel->setProperty("selectedRole", 7);
        QTRY_COMPARE(units->property("currentIndex").toInt(), 1);
        QCOMPARE(panel->property("fontSize").toDouble(), 23.75);
        QVERIFY(QMetaObject::invokeMethod(panel, "changeUnit", Q_ARG(QVariant, true)));
        QCOMPARE(style->inlineCodeFont().resolveMask(), inherited.resolveMask());
        QVERIFY(QMetaObject::invokeMethod(panel, "editFont", Q_ARG(QVariant, "family"), Q_ARG(QVariant, "serif")));
        QCOMPARE(style->inlineCodeFont().resolveMask(), inherited.resolveMask());
        QVERIFY(QMetaObject::invokeMethod(panel, "changeUnit", Q_ARG(QVariant, false)));
        QCOMPARE(style->inlineCodeFont().pixelSize(), qRound(23.75 * dpi / 72));
        QCOMPARE(style->inlineCodeFont().resolveMask(), inherited.resolveMask() | QFont::SizeResolved);
        QCOMPARE(style->inlineCodeFont().family(), QString("serif"));
        QVERIFY(QMetaObject::invokeMethod(panel, "applyPreset", Q_ARG(QVariant, true)));
        QCOMPARE(style->bodyFont().pixelSize(), 20);
        QCOMPARE(style->h1Font().pixelSize(), 40);
        QCOMPARE(style->codeBlockFont().pixelSize(), 22);
        QGuiApplication::setFont(decorated);
        QVERIFY(QMetaObject::invokeMethod(panel, "resetStyle"));
        QCOMPARE(style->bodyFont().pointSizeF(), 11.25);
        QCOMPARE(style->inlineCodeFont().resolveMask(), inherited.resolveMask());
        QVERIFY(QMetaObject::invokeMethod(panel, "applyPreset", Q_ARG(QVariant, false)));
        QCOMPARE(style->bodyFont().pointSizeF(), 11.25);
    }
    void viewerCopy()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine, QUrl::fromLocalFile(QStringLiteral(QMARKDOWN_VIEWER_SOURCE)));
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *window = qobject_cast<QQuickWindow *>(object.get()); QVERIFY(window);
        if (QGuiApplication::platformName() == "wayland") {
            window->requestActivate();
            QVERIFY(QTest::qWaitForWindowActive(window));
        }
        auto *editor = window->findChild<QObject *>("editor"); QVERIFY(editor);
        auto *preview = window->findChild<QQuickItem *>("preview"); QVERIFY(preview);
        editor->setProperty("text", "```\n  literal 😀\n\n```");
        QTRY_COMPARE(named(preview, "copyButton").size(), 1);
        auto *button = named(preview, "copyButton")[0];
        QSignalSpy requests(preview, SIGNAL(codeBlockCopyRequested(QString)));
        button->forceActiveFocus(); QTRY_VERIFY(button->hasActiveFocus());
        QTest::keyClick(window, Qt::Key_Space);
        QTRY_COMPARE(requests.size(), 1);
        QCOMPARE(QGuiApplication::clipboard()->text(), QString::fromUtf8("  literal 😀\n\n"));
        QVERIFY(!QGuiApplication::clipboard()->mimeData()->hasHtml());
        QCOMPARE(button->property("text").toString(), QString("Copied"));
        QTRY_COMPARE(button->property("text").toString(), QString("Copy"));
        const QPoint point = button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint();
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, point);
        QTRY_COMPARE(requests.size(), 2);
        auto *device = QTest::createTouchDevice();
        QTest::touchEvent(window, device).press(0, point, window);
        QTest::touchEvent(window, device).release(0, point, window);
        QTRY_COMPARE(requests.size(), 3);
        const QString evidence = qEnvironmentVariable("QMARKDOWN_COPY_EVIDENCE");
        if (!evidence.isEmpty()) QVERIFY(window->grabWindow().save(evidence));
        auto *editorItem = qobject_cast<QQuickItem *>(editor); QVERIFY(editorItem);
        editorItem->forceActiveFocus(); QTRY_VERIFY(editorItem->hasActiveFocus());
        QTest::keyClick(window, Qt::Key_A, Qt::ControlModifier);
        QTest::keyClick(window, Qt::Key_V, Qt::ControlModifier);
        QTRY_COMPARE(editor->property("text").toString(), QString::fromUtf8("  literal 😀\n\n"));
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
        QTRY_COMPARE(painted(preview).size(), 2);
        QTRY_COMPARE(painted(preview)[1]->property("text").toString(), QString("Live preview"));
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
        // The native handle width varies with the Qt version and control style.
        const auto sourceEdge = sourceScroll->mapToScene(QPointF(sourceScroll->width(), sourceScroll->height()/2));
        const auto previewEdge = previewScroll->mapToScene(QPointF(0, previewScroll->height()/2));
        const QPoint divider = ((sourceEdge + previewEdge) / 2).toPoint();
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
            "MarkdownView { width: 300; markdown: \"a <img src='https://example.invalid/x'> ![x](https://example.invalid/x) [link](https://example.invalid) &amp; *literal*\" }\n", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QTRY_COMPARE(painted(view).size(), 1);
        QVERIFY(painted(view)[0]->text().contains("<img"));
        QVERIFY(painted(view)[0]->text().contains("x link"));
        QVERIFY(painted(view)[0]->text().contains("link"));
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
        for (const char *type : {"ViewState", "BlockModel", "ModuleAnchor", "FormattedText", "CodeSurface", "CodeActionHost", "TableBlock"}) {
            QQmlComponent hidden(&engine);
            hidden.setData(QByteArray("import QMarkdown\n") + type + " {}", {});
            QVERIFY(hidden.isError());
        }
    }
};
QTEST_MAIN(ViewTest)
#include "tst_view.moc"
