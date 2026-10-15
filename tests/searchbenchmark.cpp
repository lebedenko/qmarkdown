#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <QTextStream>
#include <algorithm>
#include <memory>
#ifdef QMARKDOWN_STATIC
#include <QtQml/QQmlExtensionPlugin>
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif

// Advisory only: measure QML range bindings as well as C++ search/navigation.
static void polish(QQuickItem *item) {
    item->ensurePolished();
    for (auto *child : item->childItems()) polish(child);
}
static void settle(QQuickItem *item) {
    for (int i = 0; i < 3; ++i) {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
        polish(item);
    }
}
static int searchableItems(QQuickItem *item) {
    int count = item->metaObject()->indexOfProperty("searchRanges") >= 0 ? 1 : 0;
    for (auto *child : item->childItems()) count += searchableItems(child);
    return count;
}
int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    QQmlComponent component(&engine);
    component.setData("import QtQuick\nimport QMarkdown\nMarkdownView { width: 500; function nextResult() { searchNext() } }", {});
    QQuickWindow window; window.resize(500, 800); window.show();
    const bool quick = app.arguments().contains("--quick");
    const int samples = quick ? 3 : 5, advances = quick ? 3 : 10;
    const QList<int> blockCounts = quick ? QList<int>{25, 100} : QList<int>{100, 500};
    const QList<int> matchCounts = quick ? QList<int>{1, 4} : QList<int>{1, 8};
    QJsonArray results;
    for (int blocks : blockCounts) for (int matches : matchCounts) {
        QTextStream(stderr) << "Search benchmark: " << blocks << " blocks, " << matches << " matches/block" << Qt::endl;
        QList<double> activation, next;
        for (int run = -1; run < samples; ++run) {
            std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
            if (!view) { qWarning() << component.errors(); return 1; }
            view->setParentItem(window.contentItem());
            QString source;
            for (int i = 0; i < blocks; ++i) source += QString("needle ").repeated(matches) + "**other words**\n\n";
            view->setProperty("markdown", source);
            settle(view.get());
            view->setProperty("searchQuery", "needle");
            QElapsedTimer timer; timer.start();
            view->setProperty("searchActive", true); settle(view.get());
            const double activeMs = timer.nsecsElapsed() / 1e6;
            if (searchableItems(view.get()) != blocks || view->property("searchResultCount").toInt() != blocks * matches) {
                qWarning() << "Missing searchable delegates/results" << searchableItems(view.get()) << blocks;
                return 1;
            }
            timer.restart();
            for (int i = 0; i < advances; ++i) { QMetaObject::invokeMethod(view.get(), "nextResult"); settle(view.get()); }
            const double nextMs = timer.nsecsElapsed() / (1e6 * advances);
            if (run >= 0) { activation.append(activeMs); next.append(nextMs); }
        }
        std::sort(activation.begin(), activation.end()); std::sort(next.begin(), next.end());
        results.append(QJsonObject{{"blocks", blocks}, {"matches", blocks * matches},
            {"activation_median_ms", activation[samples / 2]}, {"next_result_median_ms", next[samples / 2]}});
    }
    QTextStream(stdout) << QJsonDocument(QJsonObject{{"qt", qVersion()},
        {"platform", QSysInfo::prettyProductName()}, {"build_abi", QSysInfo::buildAbi()}, {"compiler", __VERSION__},
        {"samples", samples}, {"warmups", 1}, {"next_actions_per_sample", advances}, {"results", results}}).toJson();
}
