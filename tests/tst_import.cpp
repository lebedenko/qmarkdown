#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QMetaProperty>
#include <QTypeRevision>
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
        for (const QByteArray &version : {QByteArray("1.0"), QByteArray("1.1"), QByteArray("1.2"), QByteArray("1.3"), QByteArray("1.4"), QByteArray("1.5"), QByteArray("1.6"), QByteArray("1.7"), QByteArray("1.8"), QByteArray("1.9"), QByteArray("1.10"), QByteArray("1.11"), QByteArray("1.12"), QByteArray("1.13"), QByteArray("1.14"), QByteArray("1.15"), QByteArray("1.16"), QByteArray("1.17"), QByteArray("1.18"), QByteArray("1.19"), QByteArray(""), QByteArray("0.7")}) {
            for (const QByteArray &type : {QByteArray("MarkdownView"), QByteArray("MarkdownStyle"), QByteArray("MarkdownResourcePolicy")}) {
                QTest::newRow((version + "-" + type).constData()) << version << type << (version != "0.7" && version != "1.19");
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
            QVERIFY2(component.errorString().contains("version " + version + " is not installed"), qPrintable(component.errorString()));
        }
    }
    void tablePresentationRevision()
    {
        for (auto version : {QByteArray("1.13"), QByteArray("1.14")}) {
            QQmlEngine engine; QQmlComponent component(&engine);
            component.setData("import QMarkdown " + version + "\nMarkdownStyle { table.borderMode: MarkdownStyle.TableBorderRows; table.alternateRowBackground: 'transparent' }", {});
            if (version == "1.14") {
                QVERIFY2(component.isReady(), qPrintable(component.errorString()));
                std::unique_ptr<QObject> object(component.create()); QVERIFY(object);
                auto *table = object->property("table").value<QObject *>(); QVERIFY(table);
                for (auto name : {"borderMode", "alternateRowBackground"}) {
                    const auto property = table->metaObject()->property(table->metaObject()->indexOfProperty(name));
                    QCOMPARE(QTypeRevision::fromEncodedVersion(property.revision()), QTypeRevision::fromVersion(1, 14));
                    QVERIFY(property.isResettable()); QVERIFY(property.hasNotifySignal());
                }
            } else {
                std::unique_ptr<QObject> object(component.create());
                QEXPECT_FAIL("", "Qt does not enforce revisions on grouped-property members (documented compatibility limitation)", Continue);
                QVERIFY2(!object, qPrintable(component.errorString()));
            }
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
