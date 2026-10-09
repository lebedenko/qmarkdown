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
    void publicTypes_data()
    {
        QTest::addColumn<QByteArray>("version");
        QTest::addColumn<QByteArray>("type");
        QTest::addColumn<bool>("accepted");
        for (const QByteArray &version : {QByteArray("1.0"), QByteArray(""), QByteArray("0.7")}) {
            for (const QByteArray &type : {QByteArray("MarkdownView"), QByteArray("MarkdownStyle"), QByteArray("MarkdownResourcePolicy")}) {
                QTest::newRow((version + "-" + type).constData()) << version << type << (version != "0.7");
            }
        }
    }
    void publicTypes()
    {
        QFETCH(QByteArray, version); QFETCH(QByteArray, type); QFETCH(bool, accepted);
        QQmlEngine engine;
        QQmlComponent component(&engine);
        component.setData("import QMarkdown " + version + "\n" + type + " {}", {});
        if (accepted) {
            QVERIFY2(component.isReady(), qPrintable(component.errorString()));
            std::unique_ptr<QObject> object(component.create());
            QVERIFY(object);
        } else {
            QVERIFY(component.isError());
            QVERIFY2(component.errorString().contains("version 0.7 is not installed"), qPrintable(component.errorString()));
        }
    }
    void resourcePolicyConfiguration() {
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QMarkdown 1.0\nMarkdownView { baseUrl: 'qrc:/documents/readme.md'; resourcePolicy: MarkdownResourcePolicy { allowedFileRoots: ['file:///tmp/images/']; allowedHttpsOrigins: ['https://example.invalid']; allowQrc: true } }", {});
        QVERIFY2(component.isReady(), qPrintable(component.errorString())); std::unique_ptr<QObject> object(component.create()); QVERIFY(object);
        QCOMPARE(object->property("baseUrl").toUrl(), QUrl("qrc:/documents/readme.md"));
        auto *policy = object->property("resourcePolicy").value<QObject *>(); QVERIFY(policy);
        QCOMPARE(policy->property("allowedFileRoots").value<QList<QUrl>>(), QList<QUrl>{QUrl("file:///tmp/images/")});
        QCOMPARE(policy->property("allowedHttpsOrigins").value<QList<QUrl>>(), QList<QUrl>{QUrl("https://example.invalid")});
        QVERIFY(policy->property("allowQrc").toBool());
    }
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
