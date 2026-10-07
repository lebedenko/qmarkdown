#pragma once
#include <QString>
#include <QVector>

namespace QMarkdownPrivate {
enum InlineFlag { Emphasis = 1, Strong = 2, Code = 4 };
struct InlineRange {
    int start = 0;
    int length = 0;
    int flags = 0;
};
struct InlineContent {
    QString text;
    QVector<InlineRange> ranges;
};
InlineContent parseInline(const QString &source);
// Shared validation boundary: accepts only complete, single-line UTF-8 spans.
bool sourceSpan(const QByteArray &source, int startLine, int startColumn,
                int endLine, int endColumn, int minimum, QByteArray *result);
}
