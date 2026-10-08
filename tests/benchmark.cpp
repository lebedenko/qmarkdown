#include "private/document.h"
#include "private/viewstate.h"
#include <QGuiApplication>
#include <QFontInfo>
#include <QThread>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QQuickItem>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSemaphore>
#include <QSysInfo>
#include <QTemporaryDir>
#include <QTextStream>
#include <algorithm>
#include <atomic>
#include <memory>
#include <stdexcept>
#ifdef QMARKDOWN_STATIC
#include <QtQml/QQmlExtensionPlugin>
Q_IMPORT_QML_PLUGIN(QMarkdownPlugin)
#endif
using namespace QMarkdownPrivate;

namespace {
constexpr int warmups = 2, runs = 10;
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
int items(QQuickItem *item) {
    int count = 1;
    for (auto *child : item->childItems()) count += items(child);
    return count;
}
void polish(QQuickItem *item) {
    item->ensurePolished();
    for (auto *child : item->childItems()) polish(child);
}
// Settle native layout, not GPU drawing. Process deferred QML delegates and polish
// until both geometry and the scene's item count are stable for three rounds.
void settle(QQuickItem *view) {
    QElapsedTimer deadline; deadline.start();
    double previous = -1; int previousItems = -1, stable = 0;
    while (stable < 3) {
        require(deadline.elapsed() < 30000, "Layout did not settle in 30 seconds");
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents(); polish(view);
        const double height = view->property("contentHeight").toDouble();
        const int count = items(view);
        stable = height == previous && count == previousItems ? stable + 1 : 0;
        previous = height; previousItems = count;
    }
    require(previous > 0, "Empty rendered layout");
}
template<class Predicate> void until(Predicate predicate) {
    QElapsedTimer deadline; deadline.start();
    while (!predicate()) {
        require(deadline.elapsed() < 5000, "Image lifecycle did not complete in 5 seconds");
        QCoreApplication::processEvents();
        QThread::yieldCurrentThread();
    }
}
QString workload(int bytes, bool dense) {
    // ASCII makes source bytes and UTF-16 character counts reproducible. Mixed
    // documents contain all four categories without enormous formatted paragraphs.
    const QString unit = dense ? "**a** *b* `c` " :
        "Plain text with several words to wrap at the fixed viewport width.\n\n"
        "A **strong** and *emphasized* paragraph with `code`.\n\n"
        "``` cpp\nint value = 42;\n```\n\n"
        "> - Outer\n>   - Inner **value**\n\n";
    QString result;
    while (result.size() + unit.size() <= bytes - 2) result += unit;
    result += QString(bytes - 2 - result.size(), u'x');
    result += "\n\n";
    require(result.toUtf8().size() == bytes, "Workload size mismatch");
    return result;
}
struct Result { double milliseconds; int rendered = 0, resets = 0; };
template<class Operation> QJsonObject measure(const QString &name, int bytes, int imageCount, Operation operation) {
    QVector<double> samples; QJsonArray rendered, resets;
    for (int i = -warmups; i < runs; ++i) {
        const auto result = operation();
        if (i >= 0) { samples.append(result.milliseconds); rendered.append(result.rendered); resets.append(result.resets); }
    }
    auto sorted = samples; std::sort(sorted.begin(), sorted.end());
    QJsonArray durations; for (auto value : samples) durations.append(value);
    return {{"operation", name}, {"sourceBytes", bytes}, {"images", imageCount},
            {"medianMs", (sorted[4] + sorted[5]) / 2}, {"maximumMs", sorted.last()},
            {"durationsMs", durations}, {"renderedItemCounts", rendered}, {"modelResetCounts", resets}};
}
struct Gate {
    QSemaphore release;
    std::atomic<int> entered{0};
    std::atomic<bool> timedOut{false};
};
struct Release {
    std::shared_ptr<Gate> gate;
    ~Release() { gate->release.release(64); }
};
}
int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    QFont font("Noto Sans"); font.setPixelSize(16); QGuiApplication::setFont(font);
    const bool smoke = app.arguments().contains("--smoke");
    try {
        require(QFontInfo(font).family() == font.family(), "Install Noto Sans for fixed benchmark typography");
        QQmlEngine engine; QQmlComponent component(&engine);
        component.setData("import QtQuick\nimport QMarkdown\nMarkdownView { width: 480 }", {});
        require(component.isReady(), qPrintable(component.errorString()));
        std::unique_ptr<QObject> object(component.create());
        auto *view = qobject_cast<QQuickItem *>(object.get()); require(view, "Missing MarkdownView");
        auto *state = view->findChild<ViewState *>(); require(state, "Missing private state");
        state->style()->setBodyFont(font);
        QFont mono("Noto Sans Mono"); mono.setPixelSize(16);
        require(QFontInfo(mono).family() == mono.family(), "Install Noto Sans Mono for fixed benchmark typography");
        state->style()->setCodeBlockFont(mono); state->style()->setInlineCodeFont(mono);
        QQuickWindow window; window.resize(480, 640); view->setParentItem(window.contentItem());
        int resets = 0;
        QObject::connect(state->blocks(), &QAbstractItemModel::modelReset, &app, [&] { ++resets; });
        QJsonArray measurements;
        const QList<int> sizes = smoke ? QList<int>{1024} : QList<int>{1024, 10*1024, 100*1024, 1024*1024};
        for (int size : sizes) {
            const auto source = workload(size, false);
            const auto parsed = parse(source);
            measurements.append(measure("parse-mixed", size, 0, [&] {
                QElapsedTimer timer; timer.start(); auto blocks = parse(source);
                require(!blocks.isEmpty(), "Empty parse"); return Result{timer.nsecsElapsed()/1e6};
            }));
            BlockModel model; int modelResets = 0;
            QObject::connect(&model, &QAbstractItemModel::modelReset, &app, [&] { ++modelResets; });
            measurements.append(measure("replace-mixed", size, 0, [&] {
                modelResets = 0; QElapsedTimer timer; timer.start(); model.replace(parsed);
                return Result{timer.nsecsElapsed()/1e6, 0, modelResets};
            }));
            measurements.append(measure("layout-mixed", size, 0, [&] {
                state->setMarkdown(""); QCoreApplication::processEvents(); resets = 0;
                QElapsedTimer timer; timer.start(); state->setMarkdown(source); settle(view);
                return Result{timer.nsecsElapsed()/1e6, items(view), resets};
            }));
        }
        const auto dense = workload(smoke ? 1024 : 10*1024, true);
        measurements.append(measure("parse-dense", dense.size(), 0, [&] {
            QElapsedTimer timer; timer.start(); auto blocks = parse(dense);
            require(!blocks.isEmpty(), "Empty dense parse"); return Result{timer.nsecsElapsed()/1e6};
        }));
        const auto denseParsed = parse(dense);
        BlockModel denseModel; int denseResets = 0;
        QObject::connect(&denseModel, &QAbstractItemModel::modelReset, &app, [&] { ++denseResets; });
        measurements.append(measure("replace-dense", dense.size(), 0, [&] {
            denseResets = 0; QElapsedTimer timer; timer.start(); denseModel.replace(denseParsed);
            return Result{timer.nsecsElapsed()/1e6, 0, denseResets};
        }));
        measurements.append(measure("layout-dense", dense.size(), 0, [&] {
            state->setMarkdown(""); QCoreApplication::processEvents(); resets = 0;
            QElapsedTimer timer; timer.start(); state->setMarkdown(dense); settle(view);
            return Result{timer.nsecsElapsed()/1e6, items(view), resets};
        }));
        QTemporaryDir directory; require(directory.isValid(), "Missing image directory");
        QImage image(20, 10, QImage::Format_ARGB32); image.fill(Qt::red);
        for (int i = 0; i < 32; ++i) require(image.save(directory.filePath(QString::number(i)+".png")), "Image fixture write failed");
        MarkdownResourcePolicy policy;
        policy.setAllowedFileRoots({QUrl::fromLocalFile(directory.path())});
        for (int count : smoke ? QList<int>{1} : QList<int>{1, 8, 32}) {
            QString source;
            for (int i = 0; i < count; ++i) source += "![](" + QUrl::fromLocalFile(directory.filePath(QString::number(i)+".png")).toString() + ")\n\n";
            const auto parsed = parse(source);
            measurements.append(measure("sequential-local-images", source.toUtf8().size(), count, [&] {
                state->setMarkdown(""); QCoreApplication::processEvents();
                ResourceController controller; auto gate = std::make_shared<Gate>(); Release cleanup{gate};
                controller.setDecoder([gate](QByteArray bytes) {
                    ++gate->entered;
                    if (!gate->release.tryAcquire(1, 5000)) gate->timedOut = true;
                    return decodeImage(std::move(bytes));
                });
                QObject::connect(&controller, &ResourceController::changed, &app, [&] {
                    static_cast<BlockModel *>(state->blocks())->replace(projectImages(parsed, controller.images(), {}));
                });
                resets = 0; QElapsedTimer timer; timer.start();
                state->setMarkdown(source); settle(view);
                controller.restart(parsed, {}, &policy);
                for (int i = 1; i <= count; ++i) {
                    until([&] { return gate->entered.load() == i; });
                    gate->release.release();
                    until([&] { return controller.images().size() == i; }); settle(view);
                }
                require(!gate->timedOut && controller.idle() && resets == count + 1, "Invalid sequential image completion");
                return Result{timer.nsecsElapsed()/1e6, items(view), resets};
            }));
        }
        QJsonObject report{{"schema", 1}, {"advisory", true}, {"smoke", smoke},
            {"warmups", warmups}, {"runs", runs}, {"viewportWidth", 480},
            {"font", font.family()}, {"resolvedFont", QFontInfo(font).family()}, {"fontPixelSize", 16},
            {"codeFont", mono.family()}, {"resolvedCodeFont", QFontInfo(mono).family()}, {"qt", qVersion()}, {"cmark", "0.31.2"},
            {"platform", QGuiApplication::platformName()}, {"os", QSysInfo::prettyProductName()},
            {"architecture", QSysInfo::currentCpuArchitecture()}, {"compiler", QMARKDOWN_COMPILER},
            {"buildType", QMARKDOWN_BUILD_TYPE}, {"measurements", measurements}};
        QTextStream(stdout) << QJsonDocument(report).toJson();
    } catch (const std::exception &error) {
        QTextStream(stderr) << error.what() << '\n'; return 1;
    }
}
