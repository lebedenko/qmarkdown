#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSignalSpy>
#include <QMetaProperty>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv); QQmlEngine engine; QQmlComponent component(&engine);
    component.setData("import QtQuick\nimport QMarkdown 1.7\nMarkdownView { width: 200; markdown: '**one**\\n\\ntwo'; style: MarkdownStyle { selectionBackgroundColor: '#603b82f6' } }", {});
    std::unique_ptr<QObject> view(component.create());
    if (!view) { qWarning() << component.errorString(); return 1; }
    if (view->property("selectionEnabled").toBool() || !view->property("selectedText").toString().isEmpty()) return 2;
    QSignalSpy copy(view.get(), SIGNAL(copyRequested(QString)));
    QMetaObject::invokeMethod(view.get(), "selectAll"); QMetaObject::invokeMethod(view.get(), "requestCopy");
    if (copy.size()) return 3;
    view->setProperty("selectionEnabled", true); QMetaObject::invokeMethod(view.get(), "selectAll");
    if (view->property("selectedText").toString() != "one\n\ntwo" || view->property("selectionStart").toInt() != 0
        || view->property("selectionEnd").toInt() != 8) return 4;
    QMetaObject::invokeMethod(view.get(), "requestCopy");
    if (copy.size() != 1 || copy[0][0].toString() != "one\n\ntwo") return 5;
    QMetaObject::invokeMethod(view.get(), "clearSelection");
    if (!view->property("selectedText").toString().isEmpty()) return 6;
    auto *style = view->property("style").value<QObject *>(); if (!style) return 7;
    const int color = style->metaObject()->indexOfProperty("selectionBackgroundColor");
    if (color < 0 || !style->metaObject()->property(color).reset(style)) return 8;
    return 0;
}
