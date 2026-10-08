#include "private/formatintervals.h"
#include "private/inline.h"
#include <QtTest>
#include <limits>
using namespace QMarkdownPrivate;

namespace {
QVariant span(int start, int length, int flags = 0) {
    return QVariantMap{{"start", start}, {"length", length}, {"flags", flags}};
}
// Independent character coverage, with boundaries tracked separately: adjacent
// equal formats must remain distinct. Use wide arithmetic rather than the sweep.
QVector<FormatInterval> reference(int size, const QVariantList &ranges, const QVariantList &links) {
    QVector<int> flags(size);
    QVector<bool> linked(size), boundaries(size + 1);
    boundaries[0] = boundaries[size] = true;
    const auto cover = [&](const QVariantList &spans, bool link) {
        for (const auto &value : spans) {
            const auto map = value.toMap();
            bool a = false, b = false;
            const qint64 start = map.value("start").toLongLong(&a);
            const qint64 length = map.value("length").toLongLong(&b);
            if (!a || !b || start < 0 || length <= 0 || start > size || length > size) continue;
            const qint64 end = start + length;
            if (end > size) continue;
            boundaries[start] = boundaries[end] = true;
            for (int pos = int(start); pos < end; ++pos) {
                if (link) linked[pos] = true;
                else flags[pos] |= map.value("flags").toInt() & (Emphasis | Strong | Code);
            }
        }
    };
    cover(ranges, false); cover(links, true);
    QVector<FormatInterval> result;
    for (int start = 0; start < size;) {
        int end = start + 1;
        while (!boundaries[end]) ++end;
        if (flags[start] || linked[start]) result.append({start, end - start, flags[start], linked[start]});
        start = end;
    }
    return result;
}
}
class FormatIntervalsTest : public QObject {
    Q_OBJECT
private slots:
    void coverage_data() {
        QTest::addColumn<QString>("text");
        QTest::addColumn<QVariantList>("ranges");
        QTest::addColumn<QVariantList>("links");
        QTest::newRow("unsorted-overlapping-nested-duplicates") << QString("abcdefghijkl")
            << QVariantList{span(5, 5, Code), span(0, 8, Emphasis), span(2, 2, Strong),
                            span(0, 8, Emphasis), span(3, 8, Emphasis), span(6, 2, Strong | Code)}
            << QVariantList{span(4, 6), span(1, 8), span(1, 8)};
        QTest::newRow("adjacent-coincident") << QString("abcdefgh")
            << QVariantList{span(0, 2, Strong), span(2, 2, Strong), span(2, 4, Emphasis), span(4, 2, Code)}
            << QVariantList{span(0, 4), span(4, 2), span(6, 2)};
        QTest::newRow("gaps-and-unsupported-boundaries") << QString("abcdefghij")
            << QVariantList{span(1, 8, Strong), span(3, 2, 0), span(6, 1, 128)} << QVariantList{};
        QTest::newRow("links-only") << QString("abcdefghij") << QVariantList{}
            << QVariantList{span(2, 4), span(4, 5), span(3, 2)};
        QTest::newRow("empty") << QString() << QVariantList{span(0, 1, Code)} << QVariantList{span(0, 1)};
        QTest::newRow("plain") << QString("abc") << QVariantList{} << QVariantList{};
        const QVariantList invalid{span(-1, 2, Strong), span(0, 0, Strong), span(1, -2, Strong),
            span(4, 1, Strong), span(2, 2, Strong), span(1, std::numeric_limits<int>::max(), Strong),
            QVariantMap{{"length", 2}, {"flags", Strong}},
            QVariantMap{{"start", "bad"}, {"length", 2}, {"flags", Strong}},
            QVariantMap{{"start", qint64(1) << 40}, {"length", 1}, {"flags", Strong}},
            QVariantMap{{"start", 0}, {"length", qint64(1) << 40}, {"flags", Strong}}, QVariant("bad")};
        QTest::newRow("invalid") << QString("abc") << invalid << invalid;
        QTest::newRow("invalid-inside-valid") << QString("abc")
            << (QVariantList{span(0, 3, Emphasis)} + invalid) << invalid;
        QTest::newRow("astral-utf16") << QString::fromUtf8("a😀b𐐀c")
            << QVariantList{span(1, 2, Strong), span(4, 2, Emphasis | Code)}
            << QVariantList{span(1, 5)};
    }
    void coverage() {
        QFETCH(QString, text); QFETCH(QVariantList, ranges); QFETCH(QVariantList, links);
        const auto actual = prepareFormatIntervals(int(text.size()), ranges, links);
        const auto expected = reference(int(text.size()), ranges, links);
        QCOMPARE(actual.size(), expected.size());
        for (qsizetype i = 0; i < actual.size(); ++i) {
            QCOMPARE(actual[i].start, expected[i].start); QCOMPARE(actual[i].length, expected[i].length);
            QCOMPARE(actual[i].flags, expected[i].flags); QCOMPARE(actual[i].linked, expected[i].linked);
        }
    }
    void deterministicCombinations() {
        for (int seed = 0; seed < 64; ++seed) {
            QVariantList ranges, links;
            for (int i = 0; i < 24; ++i) {
                const int start = (seed * 7 + i * 11) % 32;
                const int length = 1 + (seed + i * 3) % (32 - start);
                ranges.append(span(start, length, (seed + i) % 16));
                if ((i + seed) % 3 == 0) links.append(span(start, length));
            }
            QVERIFY(prepareFormatIntervals(32, ranges, links) == reference(32, ranges, links));
        }
    }
    void maximumOffset() {
        const int size = std::numeric_limits<int>::max();
        const auto intervals = prepareFormatIntervals(size,
            {span(size - 2, 2, Code), span(size - 1, 2, Emphasis), span(size, 1, Strong)}, {});
        QCOMPARE(intervals.size(), 1);
        QCOMPARE(intervals[0].start, size - 2); QCOMPARE(intervals[0].length, 2);
        QCOMPARE(intervals[0].flags, int(Code));
    }
    void boundariesRemainDistinct() {
        const auto intervals = prepareFormatIntervals(4, {span(0, 2, Strong), span(2, 2, Strong)}, {});
        QCOMPARE(intervals.size(), 2);
        QCOMPARE(intervals[0].start, 0); QCOMPARE(intervals[0].length, 2);
        QCOMPARE(intervals[1].start, 2); QCOMPARE(intervals[1].length, 2);
    }
};
QTEST_GUILESS_MAIN(FormatIntervalsTest)
#include "tst_formatintervals.moc"
