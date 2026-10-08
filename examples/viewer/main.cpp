#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlExtensionPlugin>

#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

void qml_register_types_QMarkdownViewer_Tools();

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qml_register_types_QMarkdownViewer_Tools();
    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("QMarkdownViewer", "Main");
    return app.exec();
}
