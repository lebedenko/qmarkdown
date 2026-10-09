#include "decodescheduler.h"
#include <QCoreApplication>
#include <QMutexLocker>
#include <QPointer>
#include <algorithm>

namespace QMarkdownPrivate {
DecodeScheduler::DecodeScheduler(QObject *parent) : QObject(parent) { m_pool.setMaxThreadCount(2); }
DecodeScheduler::~DecodeScheduler()
{
    {
        QMutexLocker lock(&m_mutex);
        m_stopping = true;
        for (const auto &job : m_pending) {
            job->promise.future().cancel();
            job->decode = {};
            job->promise.finish();
        }
        m_pending.clear();
    }
    m_pool.waitForDone();
}
DecodeScheduler *DecodeScheduler::instance()
{
    static QPointer<DecodeScheduler> scheduler;
    if (!scheduler) scheduler = new DecodeScheduler(QCoreApplication::instance());
    return scheduler;
}
std::shared_ptr<DecodeJob> DecodeScheduler::submit(std::function<QImage(const DecodeJob &)> decode, QDeadlineTimer deadline)
{
    auto job = std::make_shared<DecodeJob>();
    job->decode = std::move(decode); job->deadline = deadline; job->promise.start();
    QMutexLocker lock(&m_mutex);
    if (m_stopping) {
        job->promise.future().cancel(); job->decode = {}; job->promise.finish();
    } else { m_pending.push_back(job); dispatch(); }
    return job;
}
void DecodeScheduler::cancel(const std::shared_ptr<DecodeJob> &job)
{
    if (!job) return;
    QMutexLocker lock(&m_mutex);
    job->promise.future().cancel();
    const auto found = std::find(m_pending.begin(), m_pending.end(), job);
    if (found != m_pending.end()) {
        m_pending.erase(found); job->decode = {}; job->promise.finish();
    }
}
void DecodeScheduler::dispatch()
{
    while (!m_stopping && m_active < 2 && !m_pending.empty()) {
        auto job = m_pending.front(); m_pending.pop_front();
        if (job->promise.isCanceled() || job->deadline.hasExpired()) {
            job->decode = {}; job->promise.finish(); continue;
        }
        ++m_active;
        m_pool.start([this, job] {
            QImage image;
            if (!job->promise.isCanceled() && !job->deadline.hasExpired()) image = job->decode(*job);
            QMutexLocker lock(&m_mutex);
            job->decode = {};
            if (!m_stopping && !job->promise.isCanceled() && !job->deadline.hasExpired())
                job->promise.addResult(std::move(image));
            image = {};
            job->promise.finish();
            --m_active; dispatch();
        });
    }
}
}
