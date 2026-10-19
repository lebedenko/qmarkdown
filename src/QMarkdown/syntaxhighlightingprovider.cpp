#include "syntaxhighlightingprovider.h"
#include <QJSEngine>
#include <QCoreApplication>
#include <QQmlEngine>
#include <algorithm>
#include <cmath>
#include <list>

using namespace QMarkdownPrivate;
namespace {
bool boundary(const QString &source, int offset) {
    return offset == 0 || offset == source.size()
        || !source[offset - 1].isHighSurrogate() || !source[offset].isLowSurrogate();
}
bool integer(const QJSValue &value, double &number) {
    if (!value.isNumber()) return false;
    number = value.toNumber();
    return std::isfinite(number) && std::floor(number) == number;
}
bool validate(const QJSValue &value, const QString &source, HighlightSpans &spans) {
    if (!value.isArray()) return false;
    double count;
    if (!integer(value.property(QStringLiteral("length")), count)
        || count < 0 || count > HighlightLimits{}.spans) return false;
    spans.reserve(int(count));
    for (quint32 i = 0; i < quint32(count); ++i) {
        const auto item = value.property(i);
        if (!item.isObject() || item.isArray()) return false;
        double start, length, role;
        if (!integer(item.property(QStringLiteral("start")), start)
            || !integer(item.property(QStringLiteral("length")), length)
            || !integer(item.property(QStringLiteral("role")), role)
            || start < 0 || length <= 0 || start > source.size()
            || length > source.size() - start || role < 0 || role > SyntaxHighlightingProvider::Error)
            return false;
        if (!boundary(source, int(start)) || !boundary(source, int(start + length))) return false;
        spans.append({int(start), int(length), TokenRole(int(role))});
    }
    std::sort(spans.begin(), spans.end(), [](auto a, auto b) { return a.start < b.start; });
    int end = 0;
    for (auto span : spans) {
        if (span.start < end) return false;
        end = span.start + span.length;
    }
    return true;
}
}
SyntaxHighlightingRequest::SyntaxHighlightingRequest(QString source, QString language, QObject *parent,
    QJSEngine *engine, Completion completion, HighlightCancelled abandoned)
    : QObject(parent), m_source(std::move(source)), m_language(std::move(language)),
      m_engine(engine), m_completion(std::move(completion)), m_abandoned(std::move(abandoned))
{
    m_elapsed.start();
    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, [this] { settle(false, {}, true); });
    m_timer.start(5000);
}
void SyntaxHighlightingRequest::settle(bool success, const HighlightSpans &spans, bool cancelled) {
    if (m_terminal) return;
    m_terminal = true; m_cancelled = cancelled; m_timer.stop();
    auto completion = std::move(m_completion);
    // Host-retained wrappers survive cancellation; otherwise the engine can collect them.
    if (m_engine) {
        const auto wrapper = m_engine->newQObject(this);
        Q_UNUSED(wrapper);
        setParent(nullptr);
        QQmlEngine::setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);
    } else {
        setParent(nullptr); deleteLater();
    }
    if (cancelled) emit cancelledChanged();
    if (completion) completion(success, spans);
}
void SyntaxHighlightingRequest::complete(const QJSValue &value) {
    if (m_terminal) return;
    if (cancelled()) { cancel(); return; }
    if (m_elapsed.elapsed() >= 5000) { settle(false, {}, true); return; }
    HighlightSpans spans;
    const bool valid = validate(value, m_source, spans);
    // JS property getters may reenter and cancel/settle this request during validation.
    if (!m_terminal) {
        if (cancelled()) { cancel(); return; }
        if (m_elapsed.elapsed() >= 5000) settle(false, {}, true);
        else settle(valid, valid ? spans : HighlightSpans{}, false);
    }
}
void SyntaxHighlightingRequest::decline() {
    if (!m_terminal && cancelled()) { cancel(); return; }
    settle(false, {}, m_elapsed.elapsed() >= 5000);
}
void SyntaxHighlightingRequest::cancel() {
    m_completion = {};
    settle(false, {}, true);
}

struct SyntaxHighlightingProvider::Private {
    struct Subscriber {
        quint64 id;
        std::shared_ptr<std::atomic_bool> cancelled;
        SyntaxHighlightingRequest::Completion complete;
    };
    struct Job {
        quint64 id;
        QString source, language;
        QPointer<SyntaxHighlightingRequest> request;
        QVector<Subscriber> subscribers;
    };
    struct Entry { QString source, language; HighlightSpans spans; qsizetype bytes; };
    QVector<Job> jobs;
    std::list<Entry> cache;
    qsizetype bytes = 0;
    quint64 nextId = 0, generation = 0;
    bool destroying = false;
};
SyntaxHighlightingProvider::SyntaxHighlightingProvider(QObject *parent) : QObject(parent), d(new Private) {}
SyntaxHighlightingProvider::~SyntaxHighlightingProvider() {
    d->destroying = true;
    const auto jobs = std::move(d->jobs);
    for (const auto &job : jobs) if (job.request) job.request->cancel();
}
quint64 SyntaxHighlightingProvider::subscribe(const QString &source, const QString &language,
    std::shared_ptr<std::atomic_bool> cancelled, SyntaxHighlightingRequest::Completion completion, QJSEngine *engine)
{
    if (d->destroying) return 0;
    for (auto it = d->cache.begin(); it != d->cache.end(); ++it) {
        if (it->source != source || it->language != language) continue;
        const auto spans = it->spans;
        d->cache.splice(d->cache.begin(), d->cache, it);
        if (!cancelled->load()) completion(true, spans);
        return 0;
    }
    const auto id = ++d->nextId;
    Private::Subscriber subscriber{id, std::move(cancelled), std::move(completion)};
    for (auto &job : d->jobs) {
        if (job.source == source && job.language == language) {
            job.subscribers.append(std::move(subscriber)); return id;
        }
    }
    QPointer<SyntaxHighlightingProvider> guard(this);
    if (!engine) engine = qjsEngine(this);
    // QML marks a dying object’s children deleted before its C++ destructor runs.
    // Runtime ownership must therefore be outside the provider’s QObject subtree.
    auto *request = new SyntaxHighlightingRequest(source, language, qApp, engine,
        [guard, id](bool success, const HighlightSpans &spans) {
            if (guard) guard->finish(id, success, spans);
        }, [guard, id] { return !guard || !guard->subscribed(id); });
    if (auto *qml = qobject_cast<QQmlEngine *>(engine)) QQmlEngine::setContextForObject(request, qml->rootContext());
    d->jobs.append({id, source, language, request, {std::move(subscriber)}});
    emit highlightRequested(request); // May complete, invalidate or destroy this provider synchronously.
    return id;
}
bool SyntaxHighlightingProvider::subscribed(quint64 id) const {
    for (const auto &job : d->jobs) if (job.id == id)
        return std::any_of(job.subscribers.cbegin(), job.subscribers.cend(), [](const auto &s) { return !s.cancelled->load(); });
    return false;
}
void SyntaxHighlightingProvider::finish(quint64 id, bool success, const HighlightSpans &spans) {
    auto it = std::find_if(d->jobs.begin(), d->jobs.end(), [id](const auto &job) { return job.id == id; });
    if (it == d->jobs.end()) return;
    auto job = std::move(*it); d->jobs.erase(it);
    if (success) {
        const qsizetype bytes = (qMax(job.source.size(), job.source.capacity()) + qMax(job.language.size(), job.language.capacity())) * qsizetype(sizeof(QChar))
            + spans.capacity() * qsizetype(sizeof(HighlightSpan)) + sizeof(Private::Entry) + 2 * sizeof(void *);
        d->cache.push_front({job.source, job.language, spans, bytes}); d->bytes += bytes;
        while (d->cache.size() > 128 || d->bytes > 8 * 1024 * 1024) {
            d->bytes -= d->cache.back().bytes; d->cache.pop_back();
        }
    }
    const auto generation = d->generation;
    QPointer<SyntaxHighlightingProvider> guard(this);
    for (const auto &subscriber : job.subscribers) {
        if (!guard || guard->d->generation != generation) break;
        if (!subscriber.cancelled->load()) subscriber.complete(success, spans);
    }
}
void SyntaxHighlightingProvider::unsubscribe(quint64 id) {
    for (auto it = d->jobs.begin(); it != d->jobs.end(); ++it) {
        auto &subscribers = it->subscribers;
        subscribers.erase(std::remove_if(subscribers.begin(), subscribers.end(), [id](const auto &s) { return s.id == id; }), subscribers.end());
        if (!subscribers.isEmpty()) continue;
        const auto request = it->request; d->jobs.erase(it);
        if (request) request->cancel();
        return;
    }
}
void SyntaxHighlightingProvider::invalidate() {
    if (d->destroying) return;
    ++d->generation;
    d->cache.clear(); d->bytes = 0;
    const auto jobs = std::move(d->jobs); d->jobs.clear();
    QPointer<SyntaxHighlightingProvider> guard(this);
    for (const auto &job : jobs) if (job.request) job.request->cancel();
    if (guard) emit invalidated();
}
#ifdef QMARKDOWN_SYNTAX_TESTING
QPair<qsizetype, qsizetype> SyntaxHighlightingProvider::cacheSize() const {
    return {qsizetype(d->cache.size()), d->bytes};
}
#endif
