#pragma once
#include "private/syntaxhighlighter.h"
#include <QElapsedTimer>
#include <QJSValue>
#include <QPointer>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

class QJSEngine;

// Private implementation types: only their QML contract is public.
class SyntaxHighlightingRequest : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(1, 17)
    Q_PROPERTY(QString source READ source CONSTANT FINAL)
    Q_PROPERTY(QString language READ language CONSTANT FINAL)
    Q_PROPERTY(bool cancelled READ cancelled NOTIFY cancelledChanged FINAL)
public:
    using Completion = std::function<void(bool, const QMarkdownPrivate::HighlightSpans &)>;
    SyntaxHighlightingRequest(QString source, QString language, QObject *parent,
                              QJSEngine *engine, Completion completion, QMarkdownPrivate::HighlightCancelled abandoned);
    QString source() const { return m_source; }
    QString language() const { return m_language; }
    bool cancelled() const { return m_cancelled || (!m_terminal && m_abandoned()); }
    Q_INVOKABLE void complete(const QJSValue &spans);
    Q_INVOKABLE void decline();
    void cancel();
signals:
    void cancelledChanged();
private:
    void settle(bool success, const QMarkdownPrivate::HighlightSpans &spans, bool cancelled);
    const QString m_source, m_language;
    QPointer<QJSEngine> m_engine;
    QElapsedTimer m_elapsed;
    QTimer m_timer;
    Completion m_completion;
    QMarkdownPrivate::HighlightCancelled m_abandoned;
    bool m_terminal = false, m_cancelled = false;
};

class SyntaxHighlightingProvider : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ADDED_IN_VERSION(1, 17)
public:
    enum Role { Keyword, Type, Function, Variable, BuiltIn, String, Number,
                Comment, Preprocessor, Operator, Constant, Error };
    Q_ENUM(Role)
    explicit SyntaxHighlightingProvider(QObject *parent = nullptr);
    ~SyntaxHighlightingProvider() override;
    Q_INVOKABLE void invalidate();
    // Subscription IDs and token spans are private native implementation details.
    quint64 subscribe(const QString &source, const QString &language,
                      std::shared_ptr<std::atomic_bool> cancelled,
                      SyntaxHighlightingRequest::Completion completion, QJSEngine *engine = nullptr);
    void unsubscribe(quint64 subscription);
#ifdef QMARKDOWN_SYNTAX_TESTING
    QPair<qsizetype, qsizetype> cacheSize() const;
#endif
signals:
    void highlightRequested(SyntaxHighlightingRequest *request);
    void invalidated();
private:
    struct Private;
    std::unique_ptr<Private> d;
    bool subscribed(quint64 job) const;
    void finish(quint64 job, bool success, const QMarkdownPrivate::HighlightSpans &spans);
};
