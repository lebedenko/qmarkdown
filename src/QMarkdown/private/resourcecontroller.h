#pragma once
#include "../markdownresourcepolicy.h"
#include "document.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "decodescheduler.h"
#include <QTimer>
#include <QElapsedTimer>
#include <QHash>
#include <QPointer>
#include <QFutureWatcher>
#include <functional>

namespace QMarkdownPrivate {
// Private helpers also used by deterministic fixtures.
QUrl authorizedResource(const QUrl &url, const MarkdownResourcePolicy &policy);
QImage decodeImage(QByteArray bytes);
QVector<Block> projectImages(const QVector<Block> &blocks, const QHash<QUrl, QImage> &images, const QUrl &base);
}

class ResourceController : public QObject
{
    Q_OBJECT
public:
    explicit ResourceController(QObject *parent = nullptr);
    ~ResourceController() override;
    void restart(const QVector<QMarkdownPrivate::Block> &blocks, const QUrl &base, MarkdownResourcePolicy *policy);
    bool idle() const { return m_next == m_loads.size() && !m_decoding; }
    const QHash<QUrl, QImage> &images() const { return m_images; }
    // Private fixture seam; each worker copies its decoder independently of controller lifetime.
    using Decoder = std::function<QImage(QByteArray)>;
    void setDecoder(Decoder decoder) { m_decode = std::move(decoder); }
    void setAdmissionTimeout(int milliseconds) { m_admissionTimeout = milliseconds; }
    // Tests inject an isolated manager; production always owns its own manager.
    void setNetworkManager(QNetworkAccessManager *manager);
signals:
    void changed();
private:
    struct Load {
        QUrl key, url;
        QByteArray bytes;
        QPointer<QNetworkReply> reply;
        QElapsedTimer elapsed;
        int redirects = 0;
        bool started = false, done = false, local = false;
    };
    void cancel();
    void pump();
    void request(qsizetype index);
    void finish(qsizetype index);
    QNetworkAccessManager *m_network;
    QPointer<QMarkdownPrivate::DecodeScheduler> m_scheduler;
    std::shared_ptr<QMarkdownPrivate::DecodeJob> m_job;
    Decoder m_decode = QMarkdownPrivate::decodeImage;
    QTimer m_deadline;
    MarkdownResourcePolicy m_policy;
    QVector<Load> m_loads;
    QHash<QUrl, QImage> m_images;
    quint64 m_generation = 0;
    qsizetype m_next = 0;
    qint64 m_retained = 0;
    QPointer<QFutureWatcher<QImage>> m_watcher;
    int m_admissionTimeout = 15000;
    bool m_decoding = false;
};
