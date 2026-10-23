#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QMetaProperty>
#include <QtTest/QTest>
#include <QtQml/QQmlExtensionPlugin>
#include <memory>
#ifdef QMARKDOWN_STATIC
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

static QList<QQuickItem *> tables(QQuickItem *parent)
{
    QList<QQuickItem *> result;
    for (auto *child : parent->childItems()) {
        if (child->objectName() == "markdownTable") result.append(child);
        result.append(tables(child));
    }
    return result;
}
int main(int argc, char **argv)
{
    QGuiApplication app(argc,argv); QQmlEngine engine; QQmlComponent component(&engine);
    component.setData("import QtQuick\nimport QMarkdown 1.17\nMarkdownView { width: 180; markdown: '| **header** | `code` |\\n| :- | -: |\\n| ~~www.a.b~~ | [link](host:target) |' }",{});
    if (!component.isReady()) { qWarning() << component.errorString(); return 1; }
    std::unique_ptr<QObject> object(component.create()); auto *view=qobject_cast<QQuickItem *>(object.get());
    if (!view) return 2;
    for (const char *property : {"tablesEnabled","tagFilterEnabled","gfmEnabled","taskListsEnabled","strikethroughEnabled","extendedAutolinksEnabled"})
        if (view->property(property).toBool()) return 3;
    QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(300,400); window.show();
    QCoreApplication::processEvents(); if (!tables(view).isEmpty()) return 4;
    const auto source=view->property("markdown"); QSignalSpy changes(view,SIGNAL(gfmEnabledChanged())), markdownChanges(view,SIGNAL(markdownChanged()));
    if (!view->setProperty("gfmEnabled",true)) return 5;
    if (!QTest::qWaitFor([&]{ return tables(view).size()==1 && view->property("contentHeight").toDouble()>0; })) return 6;
    for (const char *property : {"tablesEnabled","tagFilterEnabled","taskListsEnabled","strikethroughEnabled","extendedAutolinksEnabled"})
        if (view->property(property).toBool()) return 7;
    view->setProperty("gfmEnabled",true); if(changes.size()!=1 || markdownChanges.size()!=0 || view->property("markdown")!=source) return 8;
    view->setProperty("tablesEnabled",true); view->setProperty("gfmEnabled",false);
    if (!QTest::qWaitFor([&]{return tables(view).size()==1;})) return 9;
    view->setProperty("tablesEnabled",false); if(!QTest::qWaitFor([&]{return tables(view).isEmpty();}))return 10;
    auto *style=view->property("style").value<QObject *>(); if(!style)return 11;
    auto *table=style->property("table").value<QObject *>(); if(!table)return 12;
    if(table->property("horizontalPadding").toDouble()!=4 || table->property("verticalPadding").toDouble()!=2)return 13;
    if(!table->setProperty("borderWidth",1.0) || !table->setProperty("headerBackground",QColor(Qt::yellow)))return 14;
    auto *code=style->property("inlineCode").value<QObject *>(); if(!code)return 15;
    if(code->property("horizontalPadding").toDouble()!=4 || code->property("verticalPadding").toDouble()!=2)return 16;
    QSignalSpy padding(code,SIGNAL(horizontalPaddingChanged()));
    code->setProperty("horizontalPadding",0.0); code->setProperty("horizontalPadding",0.0);
    if(padding.size()!=1)return 17;
    const int property=code->metaObject()->indexOfProperty("horizontalPadding");
    if(property<0 || !code->metaObject()->property(property).reset(code) || code->property("horizontalPadding").toDouble()!=4)return 18;
    if (!table->setProperty("borderMode", 2) || table->property("borderMode").toInt() != 2) return 19;
    table->setProperty("bodyBackground", QColor(Qt::blue));
    if (table->property("alternateRowBackground").value<QColor>() != QColor(Qt::blue)) return 20;
    table->setProperty("alternateRowBackground", QColor(Qt::transparent));
    if (table->property("alternateRowBackground").value<QColor>() != QColor(Qt::transparent)) return 21;
    const int alternate = table->metaObject()->indexOfProperty("alternateRowBackground");
    if (alternate < 0 || !table->metaObject()->property(alternate).reset(table)
        || table->property("alternateRowBackground").value<QColor>() != QColor(Qt::blue)) return 22;
    return 0;
}
