#include "syntaxhighlighter.h"
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QThread>
#include <algorithm>
#ifdef QMARKDOWN_WITH_KSYNTAXHIGHLIGHTING
#include <KSyntaxHighlighting/AbstractHighlighter>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Format>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/State>
#include <KSyntaxHighlighting/Theme>
#endif

namespace QMarkdownPrivate {
bool syntaxHighlightingAvailable()
{
#ifdef QMARKDOWN_WITH_KSYNTAXHIGHLIGHTING
    return true;
#else
    return false;
#endif
}
#ifdef QMARKDOWN_WITH_KSYNTAXHIGHLIGHTING
namespace {
using KSyntaxHighlighting::Theme;
int roleFor(Theme::TextStyle style)
{
    switch (style) {
    case Theme::Keyword: case Theme::ControlFlow: return int(TokenRole::Keyword);
    case Theme::DataType: case Theme::Attribute: return int(TokenRole::Type);
    case Theme::Function: return int(TokenRole::Function);
    case Theme::Variable: return int(TokenRole::Variable);
    case Theme::BuiltIn: case Theme::Extension: return int(TokenRole::BuiltIn);
    case Theme::Char: case Theme::SpecialChar: case Theme::String:
    case Theme::VerbatimString: case Theme::SpecialString: return int(TokenRole::String);
    case Theme::DecVal: case Theme::BaseN: case Theme::Float: return int(TokenRole::Number);
    case Theme::Comment: case Theme::Documentation: case Theme::Annotation:
    case Theme::CommentVar: case Theme::RegionMarker: return int(TokenRole::Comment);
    case Theme::Preprocessor: case Theme::Import: return int(TokenRole::Preprocessor);
    case Theme::Operator: return int(TokenRole::Operator);
    case Theme::Constant: return int(TokenRole::Constant);
    case Theme::Information: case Theme::Warning: case Theme::Alert: case Theme::Error:
        return int(TokenRole::Error);
    default: return -1;
    }
}
class Backend : public HighlightBackend, private KSyntaxHighlighting::AbstractHighlighter {
    // Prepared on the application thread, then owned exclusively by the worker.
    KSyntaxHighlighting::Repository m_repository;
    HighlightLimits m_limits;
    HighlightSpans m_spans;
    HighlightCancelled m_cancelled;
    QElapsedTimer m_timer;
    int m_offset = 0, m_lineLength = 0;
    bool m_failed = false;
    bool stopped() { return m_failed || m_cancelled() || m_timer.elapsed() >= m_limits.milliseconds; }
    void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override {
        if (stopped()) { m_failed = true; return; }
        const int role = roleFor(format.textStyle());
        if (role < 0 || length <= 0) return;
        if (offset < 0 || offset > m_lineLength || length > m_lineLength - offset
            || m_spans.size() >= m_limits.spans) { m_failed = true; return; }
        m_spans.append({m_offset + offset, length, TokenRole(role)});
    }
    KSyntaxHighlighting::Definition resolve(QString language) {
        language = language.toLower();
        static const QHash<QString, QString> aliases = {
            {"c", "C"}, {"h", "C"}, {"cpp", "C++"}, {"c++", "C++"}, {"cc", "C++"},
            {"cxx", "C++"}, {"hpp", "C++"}, {"hxx", "C++"},
            {"js", "JavaScript"}, {"javascript", "JavaScript"}, {"jsx", "JavaScript React (JSX)"},
            {"ts", "TypeScript"}, {"typescript", "TypeScript"}, {"tsx", "TypeScript React (TSX)"},
            {"py", "Python"}, {"python", "Python"}, {"qml", "QML"},
            {"sh", "Bash"}, {"bash", "Bash"}, {"shell", "Bash"}, {"json", "JSON"}};
        if (aliases.contains(language)) return m_repository.definitionForName(aliases.value(language));
        for (const auto &definition : m_repository.definitions())
            if (definition.name().compare(language, Qt::CaseInsensitive) == 0) return definition;
        // Only extension identifiers, never a filename or source-content heuristic.
        for (QChar ch : language)
            if (!ch.isLetterOrNumber() && ch != u'+' && ch != u'_' && ch != u'-') return {};
        return m_repository.definitionForFileName(QStringLiteral("source.") + language);
    }
public:
    explicit Backend(HighlightLimits limits, QThread *worker) : m_limits(limits) {
        // Repository installs an application event filter in its constructor.
        // Token roles do not need translation notifications. Detach before moving.
        qApp->removeEventFilter(&m_repository);
        if (worker) m_repository.moveToThread(worker);
    }
    HighlightSpans highlight(const QString &source, const QString &language,
                            const HighlightCancelled &cancelled) override {
        m_spans.clear(); m_cancelled = cancelled; m_failed = false; m_offset = 0;
        if (source.size() > m_limits.sourceUnits || language.isEmpty() || cancelled()) return {};
        m_timer.start();
        // Validate all line limits before invoking the backend.
        int lineStart = 0;
        for (int i = 0; i <= source.size(); ++i) {
            if (i == source.size() || source[i] == u'\n') {
                if (stopped() || i - lineStart > m_limits.lineUnits) return {};
                lineStart = i + 1;
            }
        }
        auto definition = resolve(language);
        if (!definition.isValid() || stopped()) return {};
        setDefinition(definition);
        KSyntaxHighlighting::State state;
        while (m_offset < source.size()) {
            if (stopped()) return {};
            int end = source.indexOf(u'\n', m_offset);
            if (end < 0) end = source.size();
            m_lineLength = end - m_offset;
            state = highlightLine(QStringView(source).mid(m_offset, m_lineLength), state);
            m_offset = end + 1;
        }
        if (stopped()) return {};
        std::sort(m_spans.begin(), m_spans.end(), [](const auto &a, const auto &b) { return a.start < b.start; });
        int end = 0;
        for (const auto &span : m_spans) {
            if (span.start < end) return {};
            end = span.start + span.length;
            // Never split UTF-16 surrogate pairs.
            if ((span.start > 0 && source[span.start].isLowSurrogate() && source[span.start - 1].isHighSurrogate())
                || (end < source.size() && source[end].isLowSurrogate() && source[end - 1].isHighSurrogate())) return {};
        }
        if (stopped()) return {};
        return std::move(m_spans);
    }
};
}
#endif
std::unique_ptr<HighlightBackend> createHighlightBackend(HighlightLimits limits, QThread *worker)
{
#ifdef QMARKDOWN_WITH_KSYNTAXHIGHLIGHTING
    Q_ASSERT(qApp && QThread::currentThread() == qApp->thread());
    return std::make_unique<Backend>(limits, worker);
#else
    Q_UNUSED(limits)
    Q_UNUSED(worker)
    return {};
#endif
}
}
