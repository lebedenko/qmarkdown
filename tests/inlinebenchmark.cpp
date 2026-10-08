#include "private/formatintervals.h"
#include "private/inline.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <QTextStream>
#include <algorithm>
#include <stdexcept>
using namespace QMarkdownPrivate;

namespace {
void require(bool condition, const char *message) {
    if (!condition) throw std::runtime_error(message);
}
// Previous renderer algorithm retained only for valid benchmark inputs.
QVector<FormatInterval> previous(int size, const QVariantList &ranges, const QVariantList &links) {
    QList<int> boundaries{0, size};
    const auto add = [&](const QVariantList &spans) {
        for (const auto &value : spans) {
            const auto span = value.toMap();
            const int start = span.value("start").toInt(), length = span.value("length").toInt();
            if (start < 0 || length <= 0 || start > size || length > size - start) continue;
            boundaries.append(start); boundaries.append(start + length);
        }
    };
    add(ranges); add(links);
    std::sort(boundaries.begin(), boundaries.end());
    boundaries.erase(std::unique(boundaries.begin(), boundaries.end()), boundaries.end());
    QVector<FormatInterval> result;
    for (qsizetype i = 0; i + 1 < boundaries.size(); ++i) {
        const int start = boundaries[i], length = boundaries[i + 1] - start;
        int flags = 0; bool linked = false;
        for (const auto &value : ranges) {
            const auto range = value.toMap();
            if (start >= range.value("start").toInt()
                && start < range.value("start").toInt() + range.value("length").toInt())
                flags |= range.value("flags").toInt();
        }
        for (const auto &value : links) {
            const auto link = value.toMap();
            if (start >= link.value("start").toInt()
                && start < link.value("start").toInt() + link.value("length").toInt()) linked = true;
        }
        if (flags || linked) result.append({start, length, flags, linked});
    }
    return result;
}
template<class Operation> QJsonObject measure(const QString &algorithm, int count, int linkCount,
                                              int size, const QVector<FormatInterval> &expected, Operation operation) {
    QVector<double> samples;
    for (int run = -2; run < 10; ++run) {
        QElapsedTimer timer; timer.start();
        const auto result = operation();
        const double duration = timer.nsecsElapsed() / 1e6;
        require(result == expected, "Interval output mismatch");
        if (run >= 0) samples.append(duration);
    }
    auto sorted = samples; std::sort(sorted.begin(), sorted.end());
    QJsonArray durations; for (double duration : samples) durations.append(duration);
    return {{"algorithm", algorithm}, {"workload", linkCount ? "overlapping-links" : "format-only"},
        {"formatSpans", count}, {"linkSpans", linkCount}, {"textLengthUtf16", size},
        {"outputIntervals", int(expected.size())}, {"equivalent", true}, {"durationsMs", durations},
        {"medianMs", (sorted[4] + sorted[5]) / 2}, {"maximumMs", sorted.last()}};
}
}
int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    try {
        QJsonArray measurements;
        for (int count : {1024, 2048, 4096, 8192}) {
            for (bool linked : {false, true}) {
                QVariantList ranges, links;
                const int size = count * 4;
                // Generated outside all timed sections. Reversed formatting order,
                // overlapping links, adjacent formats and coincident boundaries.
                for (int i = count - 1; i >= 0; --i)
                    ranges.append(QVariantMap{{"start", i * 4}, {"length", 4}, {"flags", 1 << (i % 3)}});
                if (linked) for (int i = 0; i < count; ++i)
                    links.append(QVariantMap{{"start", i * 4}, {"length", qMin(12, size - i * 4)}});
                const auto expected = previous(size, ranges, links);
                require(prepareFormatIntervals(size, ranges, links) == expected, "Initial equivalence failed");
                measurements.append(measure("previous", count, int(links.size()), size, expected,
                    [&] { return previous(size, ranges, links); }));
                measurements.append(measure("sweep", count, int(links.size()), size, expected,
                    [&] { return prepareFormatIntervals(size, ranges, links); }));
            }
        }
        const QJsonObject report{{"schema", 1}, {"advisory", true}, {"warmups", 2}, {"runs", 10},
            {"qt", qVersion()}, {"os", QSysInfo::prettyProductName()},
            {"architecture", QSysInfo::currentCpuArchitecture()}, {"compiler", QMARKDOWN_COMPILER},
            {"buildType", QMARKDOWN_BUILD_TYPE}, {"measurements", measurements}};
        QTextStream(stdout) << QJsonDocument(report).toJson();
    } catch (const std::exception &error) {
        QTextStream(stderr) << error.what() << '\n'; return 1;
    }
}
