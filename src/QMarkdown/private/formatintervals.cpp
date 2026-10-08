#include "formatintervals.h"
#include "inline.h"
#include <algorithm>
#include <array>

namespace QMarkdownPrivate {
QVector<FormatInterval> prepareFormatIntervals(
    int textLength, const QVariantList &ranges, const QVariantList &links)
{
    struct Event { int offset; int flags; bool linked; int delta; };
    QVector<Event> events;
    const auto decode = [&](const QVariantList &spans, bool linked) {
        for (const auto &value : spans) {
            const auto span = value.toMap();
            bool startOk = false, lengthOk = false;
            const qint64 decodedStart = span.value("start").toLongLong(&startOk);
            const qint64 decodedLength = span.value("length").toLongLong(&lengthOk);
            if (!startOk || !lengthOk || decodedStart < 0 || decodedLength <= 0
                || decodedStart > textLength || decodedLength > textLength - decodedStart) continue;
            const int start = int(decodedStart), length = int(decodedLength);
            const int flags = linked ? 0 : span.value("flags").toInt() & (Emphasis | Strong | Code);
            events.append({start, flags, linked, 1});
            events.append({start + length, flags, linked, -1});
        }
    };
    decode(ranges, false);
    decode(links, true);
    std::sort(events.begin(), events.end(), [](const Event &a, const Event &b) {
        return a.offset < b.offset;
    });
    QVector<FormatInterval> intervals;
    std::array<qsizetype, 4> active{};
    constexpr std::array<int, 3> flags{Emphasis, Strong, Code};
    for (qsizetype i = 0; i < events.size();) {
        const int start = events[i].offset;
        do {
            const auto &event = events[i++];
            for (int flag = 0; flag < 3; ++flag)
                if (event.flags & flags[flag]) active[flag] += event.delta;
            if (event.linked) active[3] += event.delta;
        } while (i < events.size() && events[i].offset == start);
        int effective = 0;
        for (int flag = 0; flag < 3; ++flag)
            if (active[flag] > 0) effective |= flags[flag];
        if (i < events.size() && (effective || active[3] > 0))
            intervals.append({start, events[i].offset - start, effective, active[3] > 0});
    }
    return intervals;
}
}
