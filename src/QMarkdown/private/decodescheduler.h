#pragma once
#include <QDeadlineTimer>
#include <QImage>
#include <QFuture>
#include <QMutex>
#include <QObject>
#include <QPromise>
#include <QThreadPool>
#include <deque>
#include <functional>
#include <memory>

namespace QMarkdownPrivate {
struct DecodeJob {
    QPromise<QImage> promise;
    QDeadlineTimer deadline;
    std::function<QImage(const DecodeJob &)> decode;
};
class DecodeScheduler : public QObject {
public:
    explicit DecodeScheduler(QObject *parent = nullptr);
    ~DecodeScheduler() override;
    static DecodeScheduler *instance();
    std::shared_ptr<DecodeJob> submit(std::function<QImage(const DecodeJob &)> decode, QDeadlineTimer deadline);
    void cancel(const std::shared_ptr<DecodeJob> &job);
private:
    void dispatch(); // Caller holds m_mutex; codecs execute outside it.
    QMutex m_mutex;
    QThreadPool m_pool;
    std::deque<std::shared_ptr<DecodeJob>> m_pending;
    int m_active = 0;
    bool m_stopping = false;
};
}
