#include "syntaxhighlighter.h"
#include <QCoreApplication>
#include <QPointer>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <algorithm>
#include <deque>
#include <list>

namespace QMarkdownPrivate {
namespace {
#ifdef QMARKDOWN_SYNTAX_TESTING
std::function<std::unique_ptr<HighlightBackend>()> testFactory;
#endif
struct Subscriber {
    std::shared_ptr<std::atomic_bool> cancelled;
    std::function<void(const HighlightSpans &)> deliver;
};
struct Job {
    QString source, language;
    QVector<Subscriber> subscribers;
    mutable QMutex mutex;
    void add(Subscriber subscriber) {
        QMutexLocker lock(&mutex); subscribers.append(std::move(subscriber));
    }
    bool cancelled() const {
        QMutexLocker lock(&mutex);
        return std::all_of(subscribers.cbegin(), subscribers.cend(), [](const auto &s) { return s.cancelled->load(); });
    }
};
// All queues, subscriptions and LRU bookkeeping belong to the GUI thread.
class Scheduler : public QObject {
    QThread m_thread;
    QObject *m_worker = new QObject;
    std::unique_ptr<HighlightBackend> m_backend;
    std::deque<std::shared_ptr<Job>> m_pending;
    std::shared_ptr<Job> m_active;
    struct Entry { QString source, language; HighlightSpans spans; qsizetype bytes; };
    std::list<Entry> m_cache;
    qsizetype m_bytes = 0;
    void dispatch() {
        if (m_active) return;
        while (!m_pending.empty() && m_pending.front()->cancelled()) m_pending.pop_front();
        if (m_pending.empty()) return;
        m_active = m_pending.front(); m_pending.pop_front();
        auto job = m_active;
        QMetaObject::invokeMethod(m_worker, [this, job] {
            if (!m_backend) {
#ifdef QMARKDOWN_SYNTAX_TESTING
                m_backend = testFactory ? testFactory() : createHighlightBackend();
#else
                m_backend = createHighlightBackend();
#endif
            }
            const auto cancelled = [job] { return job->cancelled(); };
            auto spans = m_backend && !cancelled() ? m_backend->highlight(job->source, job->language, cancelled) : HighlightSpans{};
            const bool aborted = cancelled();
            QMetaObject::invokeMethod(this, [this, job, spans = std::move(spans), aborted] {
                // A duplicate arriving after all original subscribers cancelled must retry.
                if (aborted && !job->cancelled()) {
                    m_active.reset(); m_pending.push_front(job); dispatch(); return;
                }
                m_active.reset();
                if (!aborted && !job->cancelled()) {
                    const qsizetype bytes = (qMax(job->source.size(), job->source.capacity()) + qMax(job->language.size(), job->language.capacity())) * qsizetype(sizeof(QChar))
                        + spans.capacity() * qsizetype(sizeof(HighlightSpan)) + sizeof(Entry) + 2 * sizeof(void *);
                    m_cache.push_front({job->source, job->language, spans, bytes}); m_bytes += bytes;
                    while (m_cache.size() > 128 || m_bytes > 8 * 1024 * 1024) {
                        m_bytes -= m_cache.back().bytes; m_cache.pop_back();
                    }
                    // Delivery can synchronously replace delegates and submit more work.
                    auto subscribers = std::move(job->subscribers);
                    for (const auto &subscriber : subscribers)
                        if (!subscriber.cancelled->load()) subscriber.deliver(spans);
                }
                dispatch();
            });
        });
    }
public:
    QPair<qsizetype, qsizetype> cacheSize() const { return {qsizetype(m_cache.size()), m_bytes}; }
    Scheduler() : QObject(qApp) {
        m_worker->moveToThread(&m_thread);
        connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
        m_thread.start();
    }
    ~Scheduler() override {
        for (const auto &job : m_pending)
            for (const auto &s : job->subscribers) s.cancelled->store(true);
        if (m_active) for (const auto &s : m_active->subscribers) s.cancelled->store(true);
        QMetaObject::invokeMethod(m_worker, [this] { m_backend.reset(); }, Qt::BlockingQueuedConnection);
        m_thread.quit(); m_thread.wait(); // Application shutdown only, never view destruction.
    }
    void submit(const QString &source, const QString &language, Subscriber subscriber) {
        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            if (it->source != source || it->language != language) continue;
            const auto spans = it->spans;
            m_cache.splice(m_cache.begin(), m_cache, it);
            if (!subscriber.cancelled->load()) subscriber.deliver(spans);
            return;
        }
        auto same = [&](const auto &job) { return job->source == source && job->language == language; };
        if (m_active && same(m_active)) { m_active->add(std::move(subscriber)); return; }
        for (auto it = m_pending.begin(); it != m_pending.end();) {
            if ((*it)->cancelled()) it = m_pending.erase(it);
            else if (same(*it)) { (*it)->add(std::move(subscriber)); return; }
            else ++it;
        }
        auto job = std::make_shared<Job>(); job->source = source; job->language = language;
        job->add(std::move(subscriber)); m_pending.push_back(job); dispatch();
    }
};
QPointer<Scheduler> instance;
Scheduler *scheduler() {
    if (!instance) instance = new Scheduler;
    return instance;
}
}
#ifdef QMARKDOWN_SYNTAX_TESTING
void resetHighlightScheduler(std::function<std::unique_ptr<HighlightBackend>()> factory) {
    delete instance.data();
    testFactory = std::move(factory);
}
QPair<qsizetype, qsizetype> highlightCacheSize() { return scheduler()->cacheSize(); }
#endif
SyntaxRequest::SyntaxRequest(QObject *parent) : QObject(parent)
{
    // Coalesce QML initialization and rapid replacements before subscribing.
    connect(this, &SyntaxRequest::inputChanged, this, &SyntaxRequest::refresh);
}
SyntaxRequest::~SyntaxRequest() { if (m_cancelled) m_cancelled->store(true); }
void SyntaxRequest::setSource(const QString &value) {
    if (m_source == value) return;
    m_source = value; emit inputChanged();
}
void SyntaxRequest::setLanguage(const QString &value) {
    if (m_language == value) return;
    m_language = value; emit inputChanged();
}
void SyntaxRequest::setEnabled(bool value) {
    if (m_enabled == value) return;
    m_enabled = value; emit inputChanged();
}
void SyntaxRequest::setTheme(SyntaxTheme *value) {
    if (m_theme == value) return;
    disconnect(m_themeChanged); disconnect(m_themeDestroyed);
    m_theme = value;
    if (value) {
        m_themeChanged = connect(value, &SyntaxTheme::changed, this, &SyntaxRequest::rangesChanged);
        m_themeDestroyed = connect(value, &QObject::destroyed, this, [this] { m_theme = nullptr; emit themeChanged(); emit rangesChanged(); });
    }
    emit themeChanged(); emit rangesChanged();
}
void SyntaxRequest::refresh() {
    if (m_cancelled) m_cancelled->store(true);
    const auto generation = ++m_generation;
    m_spans.clear(); emit rangesChanged();
    if (!m_enabled || !syntaxHighlightingAvailable() || m_source.isEmpty() || m_source.size() > 256 * 1024 || m_language.isEmpty()) return;
    QTimer::singleShot(0, this, [this, generation] {
        if (generation != m_generation) return;
        m_cancelled = std::make_shared<std::atomic_bool>(false);
        QPointer<SyntaxRequest> guard(this);
        scheduler()->submit(m_source, m_language.toLower(), {m_cancelled, [guard, generation](const HighlightSpans &spans) {
            if (!guard || guard->m_generation != generation) return;
            guard->m_spans = spans; emit guard->rangesChanged();
        }});
    });
}
QVariantList SyntaxRequest::ranges() const {
    QVariantList result;
    if (!m_theme) return result;
    for (const auto &span : m_spans) {
        const auto color = m_theme->color(int(span.role));
        if (color.isValid()) result.append(QVariantMap{{"start", span.start}, {"length", span.length}, {"color", color}});
    }
    return result;
}
}
