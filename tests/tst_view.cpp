#include "private/formattedtext.h"
#include "markdownstyle.h"
#include "markdownresourcepolicy.h"
#include <QTemporaryDir>
#include <QImage>
#include <QScreen>
#include "private/inline.h"
#include <QGuiApplication>
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
    void imageRowsAndPolicyLifecycle() {
        QTemporaryDir temp; QVERIFY(temp.isValid());
        QImage image(120, 60, QImage::Format_ARGB32); image.fill(Qt::green);
        QVERIFY(image.save(temp.filePath("image.png")));
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 0.7\nMarkdownView { width: 240 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create()); auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300, 500); view->setParentItem(window.contentItem()); window.show();
        QSignalSpy links(view, SIGNAL(linkActivated(QString)));
        view->setProperty("markdown", "before **![*alt*](image.png)** after");
        QTRY_VERIFY(content(view) > 0); QVERIFY(named(view, "markdownImage").isEmpty());
        QCOMPARE(painted(view).size(), 1); QCOMPARE(painted(view)[0]->text(), "before alt after");
        view->setProperty("baseUrl", QUrl::fromLocalFile(temp.filePath("document.md")));
        auto policy = std::make_unique<MarkdownResourcePolicy>();
        view->setProperty("resourcePolicy", QVariant::fromValue(policy.get()));
        QVERIFY(named(view, "markdownImage").isEmpty());
        policy->setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        QTRY_COMPARE(named(view, "markdownImage").size(), 1);
        auto *row = named(view, "markdownImage")[0]; QTRY_COMPARE(row->width(), 120); QTRY_COMPARE(row->height(), 60);
        QCOMPARE(texts(view).size(), 2); QCOMPARE(texts(view)[0]->property("text").toString(), "before ");
        QCOMPARE(texts(view)[1]->property("text").toString(), " after");
        const auto initialHeight = content(view);
        view->setWidth(40); QTRY_COMPARE(named(view, "markdownImage")[0]->width(), 40);
        QTRY_COMPARE(named(view, "markdownImage")[0]->height(), 20);
        QVERIFY(content(view) > 20); view->setWidth(0); QTRY_COMPARE(content(view), 0); QVERIFY(named(view, "markdownImage").isEmpty());
        view->setWidth(240); QTRY_COMPARE(content(view), initialHeight);
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
        component.setData("import QtQuick\nimport QMarkdown 0.7\nFlickable { width: 180; height: 100; contentHeight: view.contentHeight; clip: true; MarkdownView { id: view; objectName: 'view'; width: 180; markdown: '[![alt](image.png)](image:link)' } }", {});
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
        component.setData("import QtQuick\nimport QMarkdown 0.7\nItem { width: 300; height: 300; MarkdownResourcePolicy { id: p; objectName: \"policy\" } MarkdownView { objectName: \"a\"; width: 150; resourcePolicy: p; markdown: \"![alt](image.png)\" } MarkdownView { objectName: \"b\"; width: 30; resourcePolicy: p; markdown: \"![alt](image.png)\" } }", {});
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
        QTest::newRow("image-in-link") << "[![link](https://example.invalid/img)](outer)" << "outer";
    }
    void linkInteraction()
    {
        QFETCH(QString, source); QFETCH(QString, destination);
        QQmlEngine engine;
        RequestFactory factory; engine.setNetworkAccessManagerFactory(&factory);
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 0.7\nMarkdownView { width: 250 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(300, 300); view->setParentItem(window.contentItem()); window.show();
        view->setProperty("markdown", source);
        QTRY_COMPARE(painted(view).size(), 1);
        auto *item = painted(view)[0]; QTRY_VERIFY(item->logicalHeight() > 0);
        QSignalSpy activation(view, SIGNAL(linkActivated(QString))); QVERIFY(activation.isValid());
        const QPoint point = item->mapToScene(QPointF(5 - item->x(), item->logicalHeight()/2 - item->y())).toPoint();
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
        component.setData("import QtQuick\nimport QMarkdown 0.7\n"
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
        component.setData("import QtQuick\nimport QMarkdown 0.7\nMarkdownView { width: 240 }", {});
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
        component.setData("import QtQuick\nimport QMarkdown 0.7\n"
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
        component.setData("import QtQuick\nimport QMarkdown 0.7\nMarkdownView { width: 110 }", {});
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.show();
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
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
        component.setData("import QtQuick\nimport QMarkdown 0.5\nMarkdownView { width: 280 }", {});
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
        styleComponent.setData("import QMarkdown 0.5\nMarkdownStyle { quoteIndent: 48; quoteRuleThickness: 5 }", {});
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
            "import QtQuick\nimport QMarkdown 0.5\n"
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
            "import QtQuick\nimport QMarkdown 0.5\n"
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
            "            import QMarkdown 0.5\n"
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
        for (const char *type : {"ViewState", "BlockModel", "ModuleAnchor", "FormattedText"}) {
            QQmlComponent hidden(&engine);
            hidden.setData(QByteArray("import QMarkdown\n") + type + " {}", {});
            QVERIFY(hidden.isError());
        }
    }
};
QTEST_MAIN(ViewTest)
#include "tst_view.moc"
