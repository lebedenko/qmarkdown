#pragma once
#include <QVariantList>
#include <QVector>

namespace QMarkdownPrivate {
struct FormatInterval {
    int start = 0;
    int length = 0;
    int flags = 0;
    bool linked = false;
    bool operator==(const FormatInterval &other) const {
        return start == other.start && length == other.length
            && flags == other.flags && linked == other.linked;
    }
};
Q_DECL_HIDDEN QVector<FormatInterval> prepareFormatIntervals(
    int textLength, const QVariantList &ranges, const QVariantList &links);
}
