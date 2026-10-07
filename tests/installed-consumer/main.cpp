#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QEventLoop>
#include <QTimer>
#include <QFont>
#include <QColor>
#include <QtQml/QQmlExtensionPlugin>
#include <QDebug>
#include <memory>
#include <functional>

#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData(
            "import QtQuick\n"
            "import QMarkdown 0.3\n"
            "MarkdownView {\n"
            "    width: 180\n"
            "    function editCode() { style.inlineCodeFont.pixelSize = 80; style.codeBlockFont.pixelSize = 32; style.codeBlockColor = \"#345678\" }\n"
            "    markdown: \"# *Title*\\n\\n**Body** `code`\\n``` info\\n  literal *code*\\nlonglonglonglonglonglonglonglonglonglonglonglong\\n```\\n~~~\\n\\tspaces\\n~~~\"\n"
            "    style: MarkdownStyle { bodyColor: \"#123456\" }\n"
            "}", {});
    if (!component.isReady()) {
        qCritical().noquote() << component.errorString();
        return 1;
    }
    std::unique_ptr<QObject> object(component.create());
    auto *view = qobject_cast<QQuickItem *>(object.get());
    if (!view) return 1;
    QQuickWindow window;
    window.resize(300, 300);
    view->setParentItem(window.contentItem());
    window.show();
    auto settle = [&](const std::function<bool()> &ready) {
        QEventLoop loop;
        QTimer timeout;
        timeout.setSingleShot(true);
        QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
        auto connection = QObject::connect(&window, &QQuickWindow::afterAnimating, &loop, [&] {
            if (ready()) loop.quit();
            else window.update();
        });
        timeout.start(5000);
        window.update();
        loop.exec();
        QObject::disconnect(connection);
        return timeout.isActive() && ready();
    };
    auto height = [&] { return view->property("contentHeight").toDouble(); };
    if (!settle([&] { return height() > 0; })) return 2;
    const auto initial = height();
    auto *style = view->property("style").value<QObject *>();
    if (!style || style->property("bodyColor").value<QColor>() != QColor("#123456")) return 3;
    auto font = style->property("bodyFont").value<QFont>();
    font.setPixelSize(48);
    style->setProperty("bodyFont", font);
    style->setProperty("bodyColor", QColor("#654321"));
    if (!settle([&] { return height() > initial; })) return 4;
    std::function<QQuickItem *(QQuickItem *)> findBody = [&](QQuickItem *item) -> QQuickItem * {
        if (item->property("text").toString() == "Body code") return item;
        for (auto *child : item->childItems()) if (auto *found = findBody(child)) return found;
        return nullptr;
    };
    auto *body = findBody(view);
    if (!body || body->property("font").value<QFont>().pixelSize() != 48
        || body->property("color").value<QColor>() != QColor("#654321")) return 5;
    const auto beforeCode = height();
    if (!QMetaObject::invokeMethod(view, "editCode")) return 8;
    if (!settle([&] { return height() > beforeCode; })) return 9;
    if (style->property("inlineCodeFont").value<QFont>().pixelSize() != 80) return 10;
    std::function<QQuickItem *(QQuickItem *)> findCode = [&](QQuickItem *item) -> QQuickItem * {
        if (item->property("text").toString().startsWith("  literal *code*")) return item;
        for (auto *child : item->childItems()) if (auto *found = findCode(child)) return found;
        return nullptr;
    };
    auto *block = findCode(view);
    if (!block || block->property("font").value<QFont>().pixelSize() != 32
        || block->property("color").value<QColor>() != QColor("#345678")
        || block->property("textFormat").toInt() != 0) return 11;
    view->setProperty("markdown", "replacement");
    if (!settle([&] { return height() > 0 && !findBody(view); })) return 6;
    view->setProperty("markdown", "");
    if (!settle([&] { return height() == 0; })) return 7;
    return 0;
}
