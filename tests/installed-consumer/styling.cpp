#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QColor>
#include <QTimer>
#include <QEventLoop>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick\nimport QMarkdown 1.1\n"
        "MarkdownView { width: 260; markdown: '# Heading\\n\\nBody `inline code`.\\n\\n```\\n  literal\\n```';"
        "style: MarkdownStyle { preset: MarkdownStyle.PolishedDark; headings.h1.color: '#123456';"
        "inlineCode.cornerRadius: 5; codeBlock.horizontalPadding: 14 } }", {});
    if (!component.isReady()) { qWarning() << component.errorString(); return 1; }
    std::unique_ptr<QObject> object(component.create());
    auto *view = qobject_cast<QQuickItem *>(object.get());
    if (!view) return 2;
    QQuickWindow window; window.resize(300, 500); view->setParentItem(window.contentItem()); window.show();
    QEventLoop loop;
    QTimer ready;
    QObject::connect(&ready, &QTimer::timeout, &loop, [&] {
        if (view->property("contentHeight").toDouble() > 0) loop.quit();
    });
    ready.start(10);
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    if (view->property("contentHeight").toDouble() <= 0) return 3;
    auto *style = view->property("style").value<QObject *>();
    if (!style || style->property("h1Color").value<QColor>() != QColor("#123456")
        || style->property("bodyColor").value<QColor>() != QColor("#e6e6e6")) return 4;
    auto *code = style->property("codeBlock").value<QObject *>();
    auto *inlineCode = style->property("inlineCode").value<QObject *>();
    if (!code || !inlineCode || code->property("horizontalPadding").toDouble() != 14
        || inlineCode->property("cornerRadius").toDouble() != 5) return 5;
    if (!QMetaObject::invokeMethod(style, "resetOverrides")) return 6;
    if (code->property("horizontalPadding").toDouble() != 12
        || style->property("h1Color").value<QColor>() != QColor("#e6e6e6")) return 7;
    style->setProperty("preset", 3); // MinimalLight
    if (style->property("bodyColor").value<QColor>() != QColor("#202020")
        || code->property("backgroundColor").value<QColor>().alpha() != 0) return 8;
    return 0;
}
