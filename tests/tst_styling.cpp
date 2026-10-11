#include "markdownstyle.h"
#include "private/formattedtext.h"
#include "private/inline.h"
#include <QGuiApplication>
#include <QQuickWindow>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlProperty>
#include <QSignalSpy>
#include <QPainter>
#include <QDir>
#include <QTemporaryDir>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#include <limits>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

class StylingTest : public QObject
{
    Q_OBJECT
    static QList<QQuickItem *> named(QQuickItem *item, const QString &name)
    {
        QList<QQuickItem *> result;
        if (item->objectName() == name) result.append(item);
        for (auto *child : item->childItems()) result.append(named(child, name));
        return result;
    }
    static QQuickItem *textItem(QQuickItem *item, const QString &text)
    {
        if (item->property("text").toString() == text) return item;
        for (auto *child : item->childItems()) if (auto *found = textItem(child, text)) return found;
        return nullptr;
    }
private slots:
    void defaultsAliasesAndNotifications()
    {
        MarkdownStyle style;
        QCOMPARE(style.preset(), MarkdownStyle::Legacy);
        auto *heading = style.headings()->h1();
        QCOMPARE(heading->font(), style.h1Font());
        QCOMPARE(heading->color(), style.h1Color());
        QVERIFY(!heading->separatorVisible());
        QCOMPARE(heading->separatorThickness(), 0.0);
        QCOMPARE(style.codeBlock()->backgroundColor(), QColor(Qt::transparent));
        QSignalSpy flat(&style, &MarkdownStyle::h1ColorChanged), grouped(heading, &HeadingStyle::colorChanged);
        heading->setColor(Qt::red);
        QCOMPARE(style.h1Color(), QColor(Qt::red));
        QCOMPARE(flat.count(), 1); QCOMPARE(grouped.count(), 1);
        style.setH1Color(Qt::blue);
        QCOMPARE(heading->color(), QColor(Qt::blue));
        QCOMPARE(flat.count(), 2); QCOMPARE(grouped.count(), 2);
        style.setH1Color(Qt::blue);
        QCOMPARE(flat.count(), 2); QCOMPARE(grouped.count(), 2);
        QFont font = style.inlineCodeFont(); font.setPixelSize(23);
        style.inlineCode()->setFont(font);
        QCOMPARE(style.inlineCodeFont(), font);
        QCOMPARE(style.inlineCodeFont().resolveMask(), font.resolveMask());
        style.setCodeBlockColor(Qt::cyan);
        QCOMPARE(style.codeBlock()->color(), QColor(Qt::cyan));
        style.codeBlock()->setColor(Qt::magenta);
        QCOMPARE(style.codeBlockColor(), QColor(Qt::magenta));
    }
    void allGroupedFontAliases()
    {
        MarkdownStyle style;
        for (int i = 1; i <= 6; ++i) {
            auto *group = style.headings()->property(qPrintable(QString("h%1").arg(i))).value<QObject *>();
            QVERIFY(group);
            const QByteArray flatName = QString("h%1Font").arg(i).toUtf8();
            QSignalSpy flat(&style, ("2h" + QByteArray::number(i) + "FontChanged()").constData());
            QSignalSpy grouped(group, SIGNAL(fontChanged()));
            QFont font = group->property("font").value<QFont>(); font.setPixelSize(30 + i);
            QVERIFY(group->setProperty("font", font));
            QCOMPARE(style.property(flatName).value<QFont>(), font);
            QCOMPARE(flat.count(), 1); QCOMPARE(grouped.count(), 1);
            font.setItalic(true); QVERIFY(style.setProperty(flatName, font));
            QCOMPARE(group->property("font").value<QFont>().resolveMask(), font.resolveMask());
            QCOMPARE(flat.count(), 2); QCOMPARE(grouped.count(), 2);
        }
        QSignalSpy flat(&style, &MarkdownStyle::inlineCodeFontChanged);
        QSignalSpy grouped(style.inlineCode(), &InlineCodeStyle::fontChanged);
        QFont overlay; overlay.setFamily("QMarkdown alias test");
        style.setInlineCodeFont(overlay);
        QCOMPARE(style.inlineCode()->font().resolveMask(), overlay.resolveMask());
        QCOMPARE(flat.count(), 1); QCOMPARE(grouped.count(), 1);
    }
    void sharedStyleDestruction()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nItem { MarkdownStyle { id: shared; objectName: 'shared'; preset: MarkdownStyle.PolishedDark }"
            "MarkdownView { objectName: 'a'; width: 180; style: shared; markdown: '```\\ncode\\n```' }"
            "MarkdownView { objectName: 'b'; width: 180; style: shared; markdown: '```\\ncode\\n```' } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *host = qobject_cast<QQuickItem *>(object.get()); QVERIFY(host);
        auto *style = host->findChild<MarkdownStyle *>("shared"); QVERIFY(style);
        auto *a = host->findChild<QQuickItem *>("a"), *b = host->findChild<QQuickItem *>("b"); QVERIFY(a && b);
        QQuickWindow window; host->setParentItem(window.contentItem()); window.resize(500, 500); window.show();
        QTRY_COMPARE(named(a, "codeSurface").size(), 1);
        QTRY_COMPARE(named(b, "codeSurface").size(), 1);
        style->codeBlock()->setCornerRadius(10);
        QTRY_COMPARE(named(a, "codeSurface").first()->property("radius").toDouble(), 10.0);
        QTRY_COMPARE(named(b, "codeSurface").first()->property("radius").toDouble(), 10.0);
        QPointer<CodeBlockStyle> group(style->codeBlock());
        delete style; QVERIFY(group.isNull());
        QTRY_COMPARE(named(a, "codeSurface").first()->property("radius").toDouble(), 0.0);
        QTRY_COMPARE(named(b, "codeSurface").first()->property("radius").toDouble(), 0.0);
    }
    void presetsAndExplicitDefaults()
    {
        MarkdownStyle style;
        const auto font = style.bodyFont();
        style.setBodyColor(style.bodyColor());
        style.headings()->h1()->setSeparatorVisible(false);
        style.codeBlock()->setBorderWidth(0);
        style.inlineCode()->setBackgroundColor(Qt::transparent);
        style.setPreset(MarkdownStyle::PolishedDark);
        QCOMPARE(style.bodyColor(), QColor("#202020"));
        QVERIFY(!style.headings()->h1()->separatorVisible());
        QCOMPARE(style.codeBlock()->borderWidth(), 0.0);
        QCOMPARE(style.inlineCode()->backgroundColor(), QColor(Qt::transparent));
        QCOMPARE(style.h2Color(), QColor("#e6e6e6"));
        QCOMPARE(style.linkColor(), QColor("#8db9f2"));
        QCOMPARE(style.bodyFont(), font);
        style.resetOverrides();
        QCOMPARE(style.bodyColor(), QColor("#e6e6e6"));
        QVERIFY(style.headings()->h1()->separatorVisible());
        QCOMPARE(style.headings()->h2()->separatorSpacing(), 8.0);
        QCOMPARE(style.codeBlock()->borderWidth(), 1.0);
        QCOMPARE(style.inlineCode()->backgroundColor(), QColor("#30343b"));
        style.headings()->h1()->setSeparatorSpacing(22);
        style.headings()->h1()->resetSeparatorSpacing();
        QCOMPARE(style.headings()->h1()->separatorSpacing(), 8.0);
        style.setPreset(MarkdownStyle::MinimalLight);
        QCOMPARE(style.blockSpacing(), 8.0);
        QCOMPARE(style.thematicBreakColor(), QColor("#d0d7de"));
        QCOMPARE(style.bodyFont(), font);
        QVERIFY(!style.headings()->h1()->separatorVisible());
        QCOMPARE(style.codeBlock()->borderWidth(), 0.0);
        style.restoreDefaults();
        QCOMPARE(style.preset(), MarkdownStyle::Legacy);
        QCOMPARE(style.thematicBreakColor(), QColor("#202020"));
    }
    void declarationOrderAndFontEdits_data()
    {
        QTest::addColumn<QByteArray>("properties");
        QTest::newRow("preset-first") << QByteArray("preset: MarkdownStyle.PolishedDark; bodyColor: '#202020'; headings.h1.separatorVisible: false; codeBlock.borderWidth: 0; inlineCode.backgroundColor: 'transparent'");
        QTest::newRow("preset-last") << QByteArray("bodyColor: '#202020'; headings.h1.separatorVisible: false; codeBlock.borderWidth: 0; inlineCode.backgroundColor: 'transparent'; preset: MarkdownStyle.PolishedDark");
    }
    void declarationOrderAndFontEdits()
    {
        QFETCH(QByteArray, properties);
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nMarkdownStyle { " + properties +
            "; headings.h1.font.pixelSize: 29; inlineCode.font.italic: true; function edit() { codeBlock.font.pixelSize = 31 } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *style = qobject_cast<MarkdownStyle *>(object.get()); QVERIFY(style);
        QCOMPARE(style->bodyColor(), QColor("#202020"));
        QVERIFY(!style->headings()->h1()->separatorVisible());
        QCOMPARE(style->codeBlock()->borderWidth(), 0.0);
        QCOMPARE(style->inlineCode()->backgroundColor(), QColor(Qt::transparent));
        QCOMPARE(style->h1Font().pixelSize(), 29);
        QVERIFY(style->inlineCodeFont().italic());
        QVERIFY(QMetaObject::invokeMethod(style, "edit"));
        QCOMPARE(style->codeBlockFont().pixelSize(), 31);
        style->setPreset(MarkdownStyle::MinimalLight);
        QCOMPARE(style->h1Font().pixelSize(), 29);
        QCOMPARE(style->codeBlockFont().pixelSize(), 31);
        QVERIFY(QQmlProperty(style, "codeBlock.font", &engine).reset());
        QVERIFY(style->codeBlockFont().pixelSize() != 31);
    }
    void resetPreservesBindingsAndOwnedGroups()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nItem { id: host; property color ink: 'red'; property real pad: 33; MarkdownStyle { id: s; objectName: 'style'; headings.h1.color: host.ink; codeBlock.horizontalPadding: host.pad; preset: MarkdownStyle.PolishedDark } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *style = object->findChild<MarkdownStyle *>("style"); QVERIFY(style);
        QPointer<HeadingStyle> group(style->headings()->h1());
        QCOMPARE(style->h1Color(), QColor(Qt::red));
        style->resetOverrides();
        QCOMPARE(style->h1Color(), QColor("#e6e6e6"));
        QCOMPARE(style->codeBlock()->horizontalPadding(), 12.0);
        object->setProperty("ink", QColor(Qt::blue)); object->setProperty("pad", 7);
        QCOMPARE(style->h1Color(), QColor(Qt::blue));
        QCOMPARE(style->codeBlock()->horizontalPadding(), 7.0);
        style->setPreset(MarkdownStyle::MinimalLight);
        QCOMPARE(style->h1Color(), QColor(Qt::blue));
        QVERIFY(group == style->headings()->h1());
        object.reset(); QVERIFY(group.isNull());
    }
    void revisionedApi()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QMarkdown 1.0\nMarkdownStyle { preset: 1 }", {});
        QVERIFY(component.isError());
        QQmlComponent legacyComponent(&engine);
        legacyComponent.setData("import QMarkdown 1.0\nMarkdownStyle { bodyColor: 'red' }", {});
        QVERIFY2(legacyComponent.isReady(), qPrintable(legacyComponent.errorString()));
        std::unique_ptr<QObject> legacy(legacyComponent.create()); QVERIFY(legacy);
        QQmlComponent styledComponent(&engine);
        styledComponent.setData("import QMarkdown 1.1\nMarkdownStyle { preset: MarkdownStyle.PolishedLight }", {});
        QVERIFY2(styledComponent.isReady(), qPrintable(styledComponent.errorString()));
        std::unique_ptr<QObject> styled(styledComponent.create()); QVERIFY(styled);
    }
    void surfacesAndInvalidMetrics()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nMarkdownView { width: 120; style: MarkdownStyle { preset: MarkdownStyle.PolishedLight } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(400, 800); window.show();
        view->setProperty("markdown", "# one\n\n## *two*\n\n### three\n\n#\n\n> ## quoted\n\n- # listed\n\n```\n  literal *x*  \n```\n\n```\n```\n\n    longlonglonglonglonglonglonglong");
        QTRY_VERIFY(textItem(view, "one"));
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        QTRY_COMPARE(named(view, "codeSurface").size(), 3);
        int visibleRules = 0;
        for (auto *rule : named(view, "headingSeparator")) if (rule->height() > 0) {
            ++visibleRules;
            QCOMPARE(rule->height(), 1.0);
            QVERIFY(rule->width() > 0);
            QCOMPARE(rule->property("color").value<QColor>(), QColor("#d0d7de"));
        }
        QCOMPARE(visibleRules, 5);
        auto *literal = textItem(view, "  literal *x*  "); QVERIFY(literal);
        QCOMPARE(literal->x(), 12.0); QCOMPARE(literal->y(), 12.0);
        QCOMPARE(literal->width(), 96.0);
        QCOMPARE(literal->parentItem()->height(), literal->height() + 24);
        for (auto *surface : named(view, "codeSurface")) {
            QCOMPARE(surface->property("color").value<QColor>(), QColor("#eff1f3"));
            QCOMPARE(surface->property("radius").toDouble(), 6.0);
        }
        style->codeBlock()->setHorizontalPadding(std::numeric_limits<qreal>::infinity());
        style->codeBlock()->setVerticalPadding(-10);
        style->codeBlock()->setCornerRadius(std::numeric_limits<qreal>::quiet_NaN());
        style->headings()->h1()->setSeparatorThickness(std::numeric_limits<qreal>::infinity());
        style->headings()->h1()->setSeparatorSpacing(-4);
        QTRY_COMPARE(literal->x(), 0.0); QTRY_COMPARE(literal->y(), 0.0);
        QCOMPARE(literal->parentItem()->property("radius").toDouble(), 0.0);
        view->setWidth(0.5); style->codeBlock()->setHorizontalPadding(100);
        QTRY_VERIFY(literal->width() >= 1.0);
        QVERIFY(std::isfinite(view->property("contentHeight").toDouble()));
    }
    void headingImageTransitions()
    {
        QTemporaryDir directory; QVERIFY(directory.isValid());
        QImage image(40, 20, QImage::Format_RGB32); image.fill(Qt::red);
        QVERIFY(image.save(directory.filePath("image.png")));
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nMarkdownView { width: 180; style: MarkdownStyle { preset: MarkdownStyle.PolishedLight } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(400, 500); window.show();
        view->setProperty("baseUrl", QUrl::fromLocalFile(directory.filePath("doc.md")));
        view->setProperty("markdown", "# before ![image](image.png) *after*\n\n## ![only](image.png)");
        const auto visibleRules = [&] {
            int count = 0;
            for (auto *rule : named(view, "headingSeparator")) if (rule->height() > 0) ++count;
            return count;
        };
        QTRY_COMPARE(visibleRules(), 2);
        auto *policy = view->property("resourcePolicy").value<QObject *>(); QVERIFY(policy);
        policy->setProperty("allowedFileRoots", QVariant::fromValue(QList<QUrl>{QUrl::fromLocalFile(directory.path())}));
        QTRY_COMPARE(named(view, "markdownImage").size(), 2);
        QCOMPARE(visibleRules(), 2);
        for (auto *rule : named(view, "headingSeparator")) if (rule->height() > 0)
            QVERIFY(rule->mapToItem(view, QPointF()).y() >= 20);
        policy->setProperty("allowedFileRoots", QVariant::fromValue(QList<QUrl>{}));
        QTRY_COMPARE(named(view, "markdownImage").size(), 0);
        QCOMPARE(visibleRules(), 2);
    }
    void inlineGeometryAndPaint_data()
    {
        QTest::addColumn<QString>("text");
        QTest::newRow("wrap") << QString("code words wrap words words words");
        QTest::newRow("bidi") << QString::fromUtf8("abc אבג xyz مرحبا");
        QTest::newRow("graphemes") << QString::fromUtf8("á é 👩‍💻 🙂 code words");
        QTest::newRow("newlines") << QString("first\nsecond\nthird");
    }
    void inlineGeometryAndPaint()
    {
        QFETCH(QString, text);
        QQuickWindow window; window.resize(400, 300); window.show();
        FormattedText item(window.contentItem());
        QFont font = QGuiApplication::font(); font.setPixelSize(20);
        item.setFont(font); item.setCodeFont(font); item.setLayoutWidth(130); item.setText(text);
        item.setFormatRanges({QVariantMap{{"start", 0}, {"length", text.size()},
            {"flags", QMarkdownPrivate::Code | QMarkdownPrivate::Strong | QMarkdownPrivate::Emphasis}}});
        item.setLinkSpans({QVariantMap{{"start", 0}, {"length", text.size() / 2}, {"destination", "target"}}});
        QTRY_VERIFY(item.layout() && item.logicalHeight() > 0);
        const QRectF geometry(item.x(), item.y(), item.width(), item.height());
        const qreal height = item.logicalHeight();
        QList<int> hits;
        for (int y = 0; y < item.height(); y += 3)
            for (int x = 0; x < item.width(); x += 3) hits.append(item.linkAt(x, y));
        item.setCodeBackground(QColor("#eff1f3")); item.setCodeCornerRadius(4);
        item.setCodeForeground(Qt::red);
        QTRY_VERIFY(item.layout() && !item.layout()->formats().isEmpty());
        QTRY_COMPARE(item.layout()->formats().last().format.foreground().color(), QColor(Qt::red));
        QCOMPARE(QRectF(item.x(), item.y(), item.width(), item.height()), geometry);
        QCOMPARE(item.logicalHeight(), height);
        int index = 0;
        for (int y = 0; y < item.height(); y += 3)
            for (int x = 0; x < item.width(); x += 3) QCOMPARE(item.linkAt(x, y), hits[index++]);
        for (const auto &range : item.layout()->formats())
            QCOMPARE(range.format.foreground().color(), range.start < text.size() / 2 ? item.linkColor() : QColor(Qt::red));
        QImage image(qCeil(item.width()), qCeil(item.height()), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        { QPainter painter(&image); item.paint(&painter); }
        int backgroundPixels = 0;
        for (int y = 0; y < image.height(); ++y) for (int x = 0; x < image.width(); ++x)
            if (image.pixelColor(x, y) == QColor("#eff1f3")) ++backgroundPixels;
        QVERIFY(backgroundPixels > 20);
        item.setCodeCornerRadius(std::numeric_limits<qreal>::infinity());
        QCOMPARE(item.codeCornerRadius(), 0.0);
    }
    void presentationFixtures()
    {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown 1.1\nMarkdownView { width: 420 }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); QVERIFY(view);
        QQuickWindow window; window.resize(440, 640); view->setParentItem(window.contentItem());
        view->setX(10); view->setY(10); window.show();
        view->setProperty("markdown", "# Heading `code`\n\n## Second heading\n\nBody with **strong**, *emphasis*, `inline code` and [linked `code`](target).\n\n> A quote with `code`.\n\n- A list item\n\n```\n  literal <tag>\n  long words that wrap onto another line\n```\n\n---");
        auto *style = view->property("style").value<MarkdownStyle *>(); QVERIFY(style);
        const QStringList names = {"legacy", "polished-light", "polished-dark", "minimal-light", "minimal-dark"};
        const QString directory = QString(QMARKDOWN_PRESENTATION_DIR) + "/" + qEnvironmentVariable("QT_SCALE_FACTOR", "1");
        QVERIFY(QDir().mkpath(directory));
        for (int i = 0; i < names.size(); ++i) {
            const auto preset = static_cast<MarkdownStyle::Preset>(i);
            style->setPreset(preset);
            window.setColor(i == 2 || i == 4 ? QColor("#181a1f") : QColor(Qt::white));
            QTRY_VERIFY(view->property("contentHeight").toDouble() > 0);
            QTRY_COMPARE(named(view, "codeSurface").size(), 1);
            auto *surface = named(view, "codeSurface").first();
            QTRY_COMPARE(surface->property("radius").toDouble(), i == 1 || i == 2 ? 6.0 : 0.0);
            const bool polished = i == 1 || i == 2;
            QCOMPARE(QQmlProperty(surface, "border.width", &engine).read().toDouble(), polished ? 1.0 : 0.0);
            QCOMPARE(QQmlProperty(surface, "border.color", &engine).read().value<QColor>(),
                     i > 0 ? QColor(i == 2 || i == 4 ? "#454b54" : "#d0d7de") : QColor(Qt::transparent));
            const auto image = window.grabWindow(); QVERIFY(!image.isNull());
            const qreal scale = qreal(image.width()) / window.width();
            const auto point = surface->mapToScene(QPointF(surface->width() / 2, surface->height() - 6));
            if (polished)
                QCOMPARE(image.pixelColor(qRound(point.x() * scale), qRound(point.y() * scale)),
                         QColor(i == 2 ? "#30343b" : "#eff1f3"));
            QCOMPARE(image.pixelColor(0, 0), window.color());
            QVERIFY(image.save(directory + "/" + names[i] + ".png"));
        }
    }
};
QTEST_MAIN(StylingTest)
#include "tst_styling.moc"
