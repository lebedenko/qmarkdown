#include "private/viewstate.h"
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QThread>
#include <atomic>
#include <cstdio>
#include <memory>

static void require(bool condition, const char *message)
{
    if (!condition) qFatal("%s", message);
}
static QSet<QQuickItem *> delegates(QQuickItem *root)
{
    QSet<QQuickItem *> result;
    for (auto *item : root->childItems()) {
        if (root->objectName() == "blockSequence" && !QByteArray(item->metaObject()->className()).contains("Repeater")) result.insert(item);
        result.unite(delegates(item));
    }
    return result;
}
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QTemporaryDir directory; require(directory.isValid(), "temporary directory");
    QImage image(16, 16, QImage::Format_ARGB32); image.fill(Qt::blue);
    require(image.save(directory.filePath("image.png")), "fixture image");
    QJsonArray results;
    for (const int size : {1024, 10 * 1024, 100 * 1024}) {
        QString source = "![retained](image.png)\n\n";
        const QString paragraph = "Retained paragraph with **emphasis**, [link](destination) and `code`.\n\n";
        while (source.size() < size) source += paragraph;
        source.truncate(size);
        for (const bool streaming : {false, true}) {
            QQmlEngine engine; QQmlComponent component(&engine);
            component.setData("import QtQuick\nimport QMarkdown 1.18\nMarkdownView { width: 640 }", {});
            std::unique_ptr<QQuickItem> view(qobject_cast<QQuickItem *>(component.create()));
            require(bool(view), qPrintable(component.errorString()));
            QQuickWindow window; view->setParentItem(window.contentItem()); window.resize(660, 600); window.show();
            auto *state = view->findChild<ViewState *>(); require(state, "view state");
            state->setBaseUrl(QUrl::fromLocalFile(directory.filePath("source.md")));
            state->resourcePolicy()->setAllowedFileRoots({QUrl::fromLocalFile(directory.path())});
            auto *resources = state->findChild<ResourceController *>(); require(resources, "resources");
            std::atomic<int> decodes{0};
            resources->setDecoder([&](QByteArray bytes) { ++decodes; return QMarkdownPrivate::decodeImage(std::move(bytes)); });
            int publications = 0, resets = 0, created = 0, removed = 0;
            QObject::connect(state, &ViewState::searchChanged, [&] { ++publications; });
            QObject::connect(state->blocks(), &QAbstractItemModel::modelReset, [&] { ++resets; });
            QSet<QQuickItem *> observed;
            QObject observer;
            auto settle = [&] {
                QElapsedTimer deadline; deadline.start();
                do {
                    QCoreApplication::processEvents();
                    if (resources->idle()) break;
                    QThread::msleep(1);
                } while (deadline.elapsed() < 5000);
                require(resources->idle(), "resource completion deadline");
                QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
                QCoreApplication::processEvents();
                auto current = delegates(view.get());
                for (auto *item : current) {
                    if (observed.contains(item)) continue;
                    observed.insert(item); ++created;
                    QObject::connect(item, &QObject::destroyed, &observer, [&, item] { observed.remove(item); ++removed; });
                }
            };
            state->setStreamingEnabled(streaming);
            QElapsedTimer timer; timer.start();
            int chunks = 0;
            for (int offset = 0; offset < source.size(); offset += 1024) {
                ++chunks;
                if (streaming) state->appendMarkdown(source.mid(offset, 1024));
                else state->setMarkdown(source.left(offset + 1024));
                if (!streaming || chunks % 4 == 0) { state->flushMarkdown(); settle(); }
            }
            state->flushMarkdown(); settle();
            const double milliseconds = timer.nsecsElapsed() / 1e6;
            require(resets == (streaming ? 0 : chunks), "reset count");
            require(publications == (streaming ? (chunks + 3) / 4 : chunks), "publication count");
            require(decodes == (streaming ? 1 : chunks), "retained resource decode count");
            results.append(QJsonObject{{"utf16_units", size}, {"streaming", streaming}, {"chunks", chunks},
                {"publications", publications}, {"model_resets", resets}, {"delegate_creations", created},
                {"delegate_removals", removed}, {"resource_decodes", int(decodes)}, {"resource_restarts", int(decodes) - 1},
                {"retained_resources", resources->images().size()}, {"elapsed_ms", milliseconds}});
        }
    }
    const auto json = QJsonDocument(QJsonObject{{"qt", qVersion()}, {"build", "Release"},
        {"chunk_units", 1024}, {"chunks_per_publication", 4}, {"results", results}}).toJson();
    std::fwrite(json.constData(), 1, json.size(), stdout);
}
