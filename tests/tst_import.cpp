#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QtQml/QQmlExtensionPlugin>
#include <QtTest/QTest>
#include <memory>

#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

class ImportTest : public QObject
{
    Q_OBJECT

private slots:
    void importsModule()
    {
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown\nItem { width: 42; height: 24 }",
                          QUrl());
        QVERIFY2(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *item = qobject_cast<QQuickItem *>(object.get());
        QVERIFY(item);
        QCOMPARE(item->width(), 42.0);
        QCOMPARE(item->height(), 24.0);
    }
};

QTEST_MAIN(ImportTest)
#include "tst_import.moc"
