#pragma once
#include <QObject>
#include <QPointer>
#include <QVariantList>
#include <atomic>
#include <functional>
#include <memory>
#include "../syntaxtheme.h"

class SyntaxHighlightingProvider;
Q_MOC_INCLUDE("syntaxhighlightingprovider.h")

namespace QMarkdownPrivate {
enum class TokenRole { Keyword, Type, Function, Variable, BuiltIn, String, Number,
                       Comment, Preprocessor, Operator, Constant, Error };
struct HighlightSpan { int start, length; TokenRole role; };
using HighlightSpans = QVector<HighlightSpan>;
using HighlightCancelled = std::function<bool()>;
class HighlightBackend {
public:
    virtual ~HighlightBackend() = default;
    virtual HighlightSpans highlight(const QString &source, const QString &language,
                                    const HighlightCancelled &cancelled) = 0;
};
struct HighlightLimits {
    int sourceUnits = 256 * 1024;
    int lineUnits = 16 * 1024;
    int spans = 65536;
    int milliseconds = 100;
};
std::unique_ptr<HighlightBackend> createHighlightBackend(HighlightLimits limits = {});
#ifdef QMARKDOWN_SYNTAX_TESTING
void resetHighlightScheduler(std::function<std::unique_ptr<HighlightBackend>()> factory = {});
QPair<qsizetype, qsizetype> highlightCacheSize();
#endif
bool syntaxHighlightingAvailable();

// One subscription per displayed source. Palette changes only recolor completed spans.
class SyntaxRequest : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY inputChanged FINAL)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY inputChanged FINAL)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY inputChanged FINAL)
    Q_PROPERTY(SyntaxTheme *theme READ theme WRITE setTheme NOTIFY themeChanged FINAL)
    Q_PROPERTY(SyntaxHighlightingProvider *provider READ provider WRITE setProvider NOTIFY inputChanged FINAL)
    Q_PROPERTY(QVariantList ranges READ ranges NOTIFY rangesChanged FINAL)
public:
    explicit SyntaxRequest(QObject *parent = nullptr);
    ~SyntaxRequest() override;
    QString source() const { return m_source; }
    QString language() const { return m_language; }
    bool enabled() const { return m_enabled; }
    SyntaxTheme *theme() const { return m_theme; }
    SyntaxHighlightingProvider *provider() const;
    void setProvider(SyntaxHighlightingProvider *value);
    QVariantList ranges() const;
    void setSource(const QString &value);
    void setLanguage(const QString &value);
    void setEnabled(bool value);
    void setTheme(SyntaxTheme *value);
signals:
    void inputChanged();
    void themeChanged();
    void rangesChanged();
private:
    void refresh();
    void cancel();
    QString m_source, m_language;
    bool m_enabled = false;
    SyntaxTheme *m_theme = nullptr;
    QMetaObject::Connection m_themeChanged, m_themeDestroyed;
    QPointer<SyntaxHighlightingProvider> m_provider;
    QMetaObject::Connection m_providerInvalidated, m_providerDestroyed;
    quint64 m_subscription = 0;
    quint64 m_generation = 0;
    HighlightSpans m_spans;
    std::shared_ptr<std::atomic_bool> m_cancelled;
};
}
