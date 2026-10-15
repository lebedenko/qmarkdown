#include "private/formattedtext.h"
#include "private/viewstate.h"
#include "markdownresourcepolicy.h"
#include <QTemporaryDir>
#include <QPainter>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QJSValue>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtQml/QQmlExtensionPlugin>
#include <QtTest/QTest>
#include <memory>
#include <limits>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

class ReadingTest : public QObject
{
    Q_OBJECT
    static std::unique_ptr<QQuickItem> create(QQmlEngine &engine) {
        QQmlComponent component(&engine);
        component.setData(R"(
pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown
MarkdownView {
    width: 300
    function nextResult(): bool { return searchNext() }
    function previousResult(): bool { return searchPrevious() }
    function heading(index: int): bool { return navigateToHeading(index) }
    function fragment(value: string): bool { return navigateToFragment(value) }
    function outline(): string { return JSON.stringify(headings) }
    fencedBlockRenderers: ({preview: custom})
    Component {
        id: custom
        Item {
            objectName: "customReaderFence"
            required property string codeText
            required property string infoString
            required property string language
            required property var markdownStyle
            implicitHeight: 40
        }
    }
}
)", {});
        std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
        if (!view) qWarning() << component.errorString();
        return view;
    }
    static bool action(QObject *view, const char *name) {
        bool result = false;
        QMetaObject::invokeMethod(view, name, Q_RETURN_ARG(bool, result));
        return result;
    }
    static bool fragment(QObject *view, const QString &value) {
        bool result = false;
        QMetaObject::invokeMethod(view, "fragment", Q_RETURN_ARG(bool, result), Q_ARG(QString, value));
        return result;
    }
    static QList<QQuickItem *> items(QQuickItem *root) {
        QList<QQuickItem *> result;
        for (auto *item : root->childItems()) { result.append(item); result.append(items(item)); }
        return result;
    }
private slots:
    void plainTextSearchModeTransitions() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        view->setProperty("markdown", "plain needle\n\n<div>needle</div>\n");
        view->setProperty("searchQuery", "needle");
        auto highlights = [&] {
            int count = 0;
            for (auto *item : items(view.get()))
                if (auto *text = qobject_cast<FormattedText *>(item); text && !text->searchRanges().isEmpty()) ++count;
            return count;
        };
        QCOMPARE(highlights(), 0);
        view->setProperty("searchActive", true);
        QTRY_COMPARE(highlights(), 2);
        view->setProperty("searchActive", false);
        QTRY_COMPARE(highlights(), 0);
        view->setProperty("selectionEnabled", true);
        view->setProperty("searchActive", true);
        QTRY_COMPARE(highlights(), 2);
        view->setProperty("selectionEnabled", false);
        QTRY_COMPARE(highlights(), 2);
    }
    void boundedSearchRanges() {
        ViewState state;
        state.setMarkdown(QString::fromUtf8("😀needle needle\n\nlast"));
        state.setSearchQuery("needle"); state.setSearchActive(true);
        auto range = [](int start, int length, bool current) {
            return QVariant(QVariantMap{{"start", start}, {"length", length}, {"current", current}});
        };
        QCOMPARE(state.searchRangesFor(0, 2), QVariantList{}); // UTF-16 emoji ends at match start.
        QCOMPARE(state.searchRangesFor(8, 1), QVariantList{}); // Match end excluded.
        QCOMPARE(state.searchRangesFor(3, 3), QVariantList{range(0, 3, true)});
        QCOMPARE(state.searchRangesFor(7, 3), (QVariantList{range(0, 1, true), range(2, 1, false)}));
        QVERIFY(state.searchRangesFor(0, 0).isEmpty());
        QVERIFY(state.searchRangesFor(0, -1).isEmpty());
        QVERIFY(state.searchRangesFor(std::numeric_limits<int>::max(), 10).isEmpty());
        const auto revision = state.searchRevision();
        QVERIFY(state.searchNext());
        QCOMPARE(state.searchResultCount(), 2);
        QVERIFY(state.searchRevision() != revision);
        QCOMPARE(state.searchRangesFor(9, 6), QVariantList{range(0, 6, true)});
        QCOMPARE(state.searchRangesFor(2, 6), QVariantList{range(0, 6, false)});
        state.setSearchQuery("needle\n\nlast");
        QCOMPARE(state.searchRangesFor(9, 6), QVariantList{range(0, 6, true)});
        QCOMPARE(state.searchRangesFor(17, 4), QVariantList{range(0, 4, true)});
        state.setMarkdown("![needle](missing.png)");
        state.setSearchQuery("eed");
        QCOMPARE(state.searchRangesFor(0, 6), QVariantList{range(1, 3, true)});
        state.setSearchActive(false);
        QVERIFY(state.searchRangesFor(0, 6).isEmpty());
    }
    void literalSearchAndSelection() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        view->setProperty("markdown", QString::fromUtf8("# CAFÉ\n\nCafé **café** `café`\n\n```\ncafé\n```\n\n![café](missing.png)\n\n| café |\n| --- |\n| café |"));
        view->setProperty("tablesEnabled", true);
        view->setProperty("searchQuery", QString::fromUtf8("café"));
        QCOMPARE(view->property("searchResultCount").toInt(), 0);
        QVERIFY(!action(view.get(), "nextResult"));
        view->setProperty("selectionEnabled", true);
        QMetaObject::invokeMethod(view.get(), "selectAll");
        const auto selected = view->property("selectedText").toString();
        view->setProperty("searchActive", true);
        QCOMPARE(view->property("searchResultCount").toInt(), 8);
        QCOMPARE(view->property("currentSearchResult").toInt(), 0);
        QVERIFY(action(view.get(), "previousResult"));
        QCOMPARE(view->property("currentSearchResult").toInt(), 7);
        QVERIFY(action(view.get(), "nextResult"));
        QCOMPARE(view->property("currentSearchResult").toInt(), 0);
        view->setProperty("searchCaseSensitive", true);
        QCOMPARE(view->property("searchResultCount").toInt(), 6);
        QCOMPARE(view->property("selectedText").toString(), selected);
        QSignalSpy copy(view.get(), SIGNAL(copyRequested(QString)));
        QMetaObject::invokeMethod(view.get(), "requestCopy");
        QCOMPARE(copy.size(), 1); QCOMPARE(copy[0][0].toString(), selected);
        view->setProperty("searchQuery", "missing query");
        QCOMPARE(view->property("currentSearchResult").toInt(), -1);
        view->setProperty("searchQuery", "");
        QCOMPARE(view->property("searchResultCount").toInt(), 0);
        view->setProperty("markdown", QString::fromUtf8("aaaa 😀😀"));
        view->setProperty("searchQuery", "aa");
        QCOMPARE(view->property("searchResultCount").toInt(), 2);
        view->setProperty("searchQuery", QString::fromUtf8("😀"));
        QCOMPARE(view->property("searchResultCount").toInt(), 2);
        view->setProperty("searchActive", false);
        QCOMPARE(view->property("searchResultCount").toInt(), 0);
        QCOMPARE(view->property("searchQuery").toString(), QString::fromUtf8("😀"));
    }
    void outlineAndFragments() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        view->setProperty("markdown", QString::fromUtf8("# Foo\n\n> ## Foo\n\n- ### Foo 1\n\n#### !!!\n\n##### !!!\n\n###### Élan &amp; `Code` 😀\n\n# 100%25\n\n# +\n\n# Foo-1"));
        QString json;
        QVERIFY(QMetaObject::invokeMethod(view.get(), "outline", Q_RETURN_ARG(QString, json)));
        const auto entries = QJsonDocument::fromJson(json.toUtf8()).array();
        QCOMPARE(entries.size(), 9);
        const QStringList ids = {"foo", "foo-1", "foo-1-1", "section", "section-1", QString::fromUtf8("élan-code-😀"), "10025", "+", "foo1"};
        for (int i = 0; i < ids.size(); ++i) {
            QCOMPARE(entries[i].toObject()["fragmentId"].toString(), ids[i]);
            QVERIFY(fragment(view.get(), "#" + ids[i]));
        }
        QCOMPARE(entries[5].toObject()["level"].toInt(), 6);
        QCOMPARE(entries[5].toObject()["title"].toString(), QString::fromUtf8("Élan & Code 😀"));
        QVERIFY(fragment(view.get(), "#%C3%A9lan-code-%F0%9F%98%80"));
        QVERIFY(fragment(view.get(), "#%2B"));
        QVERIFY(!fragment(view.get(), "#%252B"));
        QVERIFY(!fragment(view.get(), "#Foo"));
        QVERIFY(!fragment(view.get(), "foo"));
        QVERIFY(!fragment(view.get(), "#"));
        QVERIFY(!fragment(view.get(), "https://example.org/#foo"));
        bool result = true;
        QVERIFY(QMetaObject::invokeMethod(view.get(), "heading", Q_RETURN_ARG(bool, result), Q_ARG(int, -1)));
        QVERIFY(!result);
    }
    void geometryAndPainting() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; window.resize(320, 600); view->setParentItem(window.contentItem()); window.show();
        view->setProperty("markdown", "# Start\n\nFirst paragraph.\n\n## Target heading\n\nA **needle** and `needle` then needle.\n\n~~~preview\nneedle\n~~~");
        QSignalSpy navigation(view.get(), SIGNAL(navigationRequested(QRectF)));
        QVERIFY(fragment(view.get(), "#target-heading"));
        QTRY_VERIFY(!navigation.isEmpty());
        auto target = navigation.last()[0].toRectF(); QVERIFY(target.y() > 0); QVERIFY(target.width() > 0);
        const qreal viewY = view->y();
        view->setProperty("searchQuery", "needle"); view->setProperty("searchActive", true);
        QTRY_COMPARE(view->property("searchResultCount").toInt(), 4);
        auto leaves = [&] {
            QList<FormattedText *> found;
            for (auto *item : items(view.get())) if (auto *text = qobject_cast<FormattedText *>(item)) found.append(text);
            return found;
        };
        QTRY_VERIFY(!leaves().isEmpty());
        QTRY_VERIFY(navigation.last()[0].toRectF().y() > target.y());
        QCOMPARE(view->y(), viewY);
        bool painted = false;
        for (auto *text : leaves()) if (text->text().contains("needle")) {
            QTRY_VERIFY(text->layout());
            QVERIFY(!text->searchRanges().isEmpty());
            const auto before = text->logicalHeight();
            const auto range = text->rangeRectangles(2, 8); QVERIFY(!range.isEmpty());
            text->setSearchHighlightColor(Qt::red);
            QCOMPARE(text->logicalHeight(), before); painted = true;
        }
        QVERIFY(painted);
        const auto count = navigation.size();
        view->setWidth(100);
        QTRY_VERIFY(navigation.size() > count);
        navigation.clear();
        QVERIFY(!fragment(view.get(), "#absent"));
        view->setWidth(200); QCoreApplication::processEvents();
        QCOMPARE(navigation.size(), 0);
        view->setProperty("searchActive", false);
        QTRY_VERIFY([&] { for (auto *item : items(view.get())) if (item->objectName() == "customReaderFence" && item->isVisible()) return true; return false; }());
        view->setProperty("selectionEnabled", true);
        view->setProperty("searchActive", true); view->setProperty("searchActive", false);
        for (auto *item : items(view.get())) QVERIFY(item->objectName() != "customReaderFence" || !item->isVisible());
    }
    void imageGeometryAndDeferredTargets() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        QQuickWindow window; window.resize(320, 600); view->setParentItem(window.contentItem()); window.show();
        QTemporaryDir dir; QVERIFY(dir.isValid());
        QImage image(80, 120, QImage::Format_ARGB32); image.fill(Qt::blue);
        QVERIFY(image.save(dir.filePath("shape.png")));
        view->setProperty("baseUrl", QUrl::fromLocalFile(dir.path() + "/"));
        view->setProperty("markdown", "![needle](shape.png)\n\n## After image");
        view->setProperty("searchQuery", "needle"); view->setProperty("searchActive", true);
        QSignalSpy navigation(view.get(), SIGNAL(navigationRequested(QRectF)));
        QVERIFY(fragment(view.get(), "#after-image"));
        QTRY_VERIFY(!navigation.isEmpty());
        const qreal fallbackY = navigation.last()[0].toRectF().y();
        auto *policy = view->property("resourcePolicy").value<MarkdownResourcePolicy *>(); QVERIFY(policy);
        policy->setAllowedFileRoots({QUrl::fromLocalFile(dir.path() + "/")});
        QTRY_VERIFY(navigation.last()[0].toRectF().y() > fallbackY + 50);
        QCOMPARE(view->property("currentSearchResult").toInt(), 0);
        QVERIFY(action(view.get(), "nextResult"));
        QTRY_VERIFY(navigation.last()[0].toRectF().height() >= 120);
        QQuickItem *imageItem = nullptr;
        for (auto *item : items(view.get())) if (item->objectName() == "markdownImage") imageItem = item;
        QVERIFY(imageItem);
        QVERIFY(imageItem->childItems().size() >= 2);
        QVERIFY(imageItem->childItems()[1]->isVisible());
        view->setWidth(0); navigation.clear();
        QVERIFY(fragment(view.get(), "#after-image"));
        QCoreApplication::processEvents(); QCOMPARE(navigation.size(), 0);
        view->setWidth(300); QTRY_VERIFY(!navigation.isEmpty());
        view->setWidth(0); navigation.clear();
        QVERIFY(fragment(view.get(), "#after-image"));
        view->setProperty("markdown", "# Replacement");
        view->setWidth(300); QCoreApplication::processEvents(); QCOMPARE(navigation.size(), 0);
    }
    void highlightPixelsAndSelectionPriority() {
        FormattedText text;
        QQuickWindow window; text.setParentItem(window.contentItem());
        window.resize(250, 80); window.show();
        QFont font; font.setPixelSize(20); text.setFont(font);
        text.setText("one two"); text.setLayoutWidth(200);
        text.setSearchHighlightColor(Qt::yellow); text.setCurrentSearchHighlightColor(Qt::red);
        text.setSearchRanges({QVariantMap{{"start", 0}, {"length", 3}, {"current", false}},
            QVariantMap{{"start", 4}, {"length", 3}, {"current", true}}});
        QTRY_VERIFY(text.layout());
        const auto height = text.logicalHeight();
        auto pixel = [&](int position) {
            QImage image(250, 80, QImage::Format_ARGB32); image.fill(Qt::white);
            QPainter painter(&image); text.paint(&painter); painter.end();
            const auto rect = text.rangeRectangles(position, position + 1)[0].translated(text.paintOffset());
            return image.pixelColor(qFloor(rect.center().x()), qFloor(rect.bottom() - 1));
        };
        QCOMPARE(pixel(0), QColor(Qt::yellow)); QCOMPARE(pixel(4), QColor(Qt::red));
        text.setSelectionColor(Qt::green); text.setSelectionStart(0); text.setSelectionEnd(7);
        QCOMPARE(pixel(0), QColor(Qt::green)); QCOMPARE(pixel(4), QColor(Qt::green));
        QCOMPARE(text.logicalHeight(), height);
    }
    void effectiveOptionsAndReset() {
        QQmlEngine engine; auto view = create(engine); QVERIFY(view);
        view->setProperty("markdown", "# one one\n\n~~one~~");
        view->setProperty("gfmEnabled", true);
        view->setProperty("searchQuery", "one"); view->setProperty("searchActive", true);
        QVERIFY(action(view.get(), "nextResult"));
        QCOMPARE(view->property("currentSearchResult").toInt(), 1);
        view->setProperty("strikethroughEnabled", true); // Effective options unchanged.
        QCOMPARE(view->property("currentSearchResult").toInt(), 1);
        view->setProperty("gfmEnabled", false); // Other effective extensions change.
        QCOMPARE(view->property("currentSearchResult").toInt(), 0);
        QVERIFY(action(view.get(), "nextResult"));
        view->setProperty("markdown", "# one one one");
        QCOMPARE(view->property("currentSearchResult").toInt(), 0);
    }
};
QTEST_MAIN(ReadingTest)
#include "tst_reading.moc"
