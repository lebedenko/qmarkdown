#include "resourcecontroller.h"
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QImageReader>
#include <QPromise>
#include <QSet>
#include <QtEndian>
#include <memory>
#include <QNetworkCookieJar>
#include <QNetworkCookie>

namespace {
constexpr qint64 encodedLimit = 8 * 1024 * 1024;
constexpr qint64 pixelLimit = 16 * 1000 * 1000;
constexpr qint64 retainedLimit = 64 * 1024 * 1024;
constexpr int deadlineMs = 15000;
bool origin(const QUrl &url) {
    return url.isValid() && url.scheme() == "https" && !url.host().isEmpty()
        && url.userInfo().isEmpty() && !url.authority().contains(u'@')
        && (url.path().isEmpty() || url.path() == "/") && !url.hasQuery() && !url.hasFragment();
}
bool localAuthority(const QUrl &url) {
    return url.authority().isEmpty() || url.authority().compare("localhost", Qt::CaseInsensitive) == 0;
}
QString localPath(const QUrl &url) {
    return url.authority().isEmpty() ? url.toLocalFile() : url.path();
}
bool pixels(const QSize &size) { return size.width() > 0 && size.height() > 0 && qint64(size.width()) * size.height() <= pixelLimit; }
class EmptyCookies : public QNetworkCookieJar {
public:
    using QNetworkCookieJar::QNetworkCookieJar;
    QList<QNetworkCookie> cookiesForUrl(const QUrl &) const override { return {}; }
    bool setCookiesFromUrl(const QList<QNetworkCookie> &, const QUrl &) override { return false; }
};
}
namespace QMarkdownPrivate {
QUrl authorizedResource(const QUrl &url, const MarkdownResourcePolicy &policy)
{
    if (!url.isValid() || url.isRelative() || !url.userInfo().isEmpty() || url.authority().contains(u'@')) return {};
    if (url.scheme() == "qrc") {
        if (policy.allowQrc() && url.authority().isEmpty() && url.path().startsWith(u'/') && !url.hasQuery())
            return url.adjusted(QUrl::NormalizePathSegments | QUrl::RemoveFragment);
    } else if (url.scheme() == "file") {
        if (policy.allowedFileRoots().isEmpty()) return {};
        if (!localAuthority(url) || url.hasQuery() || !QDir::isAbsolutePath(localPath(url))) return {};
        const QFileInfo image(localPath(url));
        const auto path = image.canonicalFilePath();
        if (path.isEmpty() || !image.isFile()) return {};
        for (const auto &root : policy.allowedFileRoots()) {
            if (!root.isValid() || root.scheme() != "file" || !localAuthority(root)
                || root.hasQuery() || root.hasFragment() || !QDir::isAbsolutePath(localPath(root))) continue;
            const QFileInfo info(localPath(root));
            const auto directory = info.canonicalFilePath();
            if (info.isDir() && !directory.isEmpty() && path.startsWith(directory.endsWith(u'/') ? directory : directory + u'/'))
                return QUrl::fromLocalFile(path);
        }
    } else if (url.scheme() == "https" && !url.host().isEmpty()) {
        for (const auto &entry : policy.allowedHttpsOrigins())
            if (origin(entry) && entry.host().compare(url.host(), Qt::CaseInsensitive) == 0 && entry.port(443) == url.port(443))
                return url.adjusted(QUrl::RemoveFragment);
    }
    return {};
}
QImage decodeImage(QByteArray bytes)
{
    if (bytes.isEmpty() || bytes.size() > encodedLimit) return {};
    QByteArray format;
    if (bytes.startsWith(QByteArray::fromHex("89504e470d0a1a0a"))) {
        format = "png";
        // Some PNG plugins display only the first APNG frame and do not report
        // animation support. Reject the animation control chunk independently.
        qsizetype offset = 8;
        while (offset + 12 <= bytes.size()) {
            const auto length = qFromBigEndian<quint32>(bytes.constData() + offset);
            if (qint64(length) > bytes.size() - offset - 12) return {};
            const auto type = bytes.mid(offset + 4, 4);
            if (type == "acTL") return {};
            if (type == "IEND") break;
            offset += qint64(length) + 12;
        }
    }
    else if (bytes.startsWith(QByteArray::fromHex("ffd8ff"))) format = "jpeg";
    else return {};
    QBuffer buffer(&bytes); buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer, format);
    reader.setDecideFormatFromContent(false);
    reader.setAutoTransform(true);
    const auto size = reader.size();
    if (!pixels(size) || (reader.supportsAnimation() || reader.imageCount() > 1)) return {};
    const auto expected = reader.transformation() & QImageIOHandler::TransformationRotate90 ? size.transposed() : size;
    auto image = reader.read();
    if (image.isNull() || !pixels(image.size()) || image.size() != expected) return {};
    image.setDevicePixelRatio(1);
    return image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}
namespace {
Block textSlice(const Block &block, int start, int end) {
    Block result = block;
    result.text = block.text.mid(start, end - start);
    result.images.clear(); result.ranges.clear(); result.links.clear();
    for (const auto &range : block.ranges) {
        const int a = qMax(start, range.start), b = qMin(end, range.start + range.length);
        if (a < b) result.ranges.append({a - start, b - a, range.flags});
    }
    for (const auto &link : block.links) {
        const int a = qMax(start, link.start), b = qMin(end, link.start + link.length);
        if (a < b) result.links.append({a - start, b - a, link.destination});
    }
    return result;
}
}
QVector<Block> projectImages(const QVector<Block> &blocks, const QHash<QUrl, QImage> &images, const QUrl &base)
{
    QVector<Block> result;
    for (auto block : blocks) {
        block.children = projectImages(block.children, images, base);
        QVector<Block> segments;
        int start = 0;
        for (const auto &span : block.images) {
            const auto key = base.resolved(QUrl(span.destination)).adjusted(QUrl::RemoveFragment);
            if (!images.contains(key)) continue;
            if (span.start > start) segments.append(textSlice(block, start, span.start));
            Block image{BlockKind::Image, {}};
            image.image = images.value(key); image.imageLink = span.enclosingLink; image.imageLinked = span.linked;
            segments.append(std::move(image));
            start = span.start + span.length;
        }
        if (!segments.isEmpty()) {
            if (start < block.text.size()) segments.append(textSlice(block, start, block.text.size()));
            block.kind = BlockKind::Segments; block.children = std::move(segments);
            block.images.clear(); block.text.clear(); block.ranges.clear(); block.links.clear();
        }
        result.append(std::move(block));
    }
    return result;
}
}

ResourceController::ResourceController(QObject *parent) : QObject(parent), m_network(new QNetworkAccessManager(this)), m_policy(this)
{
    m_decoder.setMaxThreadCount(1);
    m_network->setCookieJar(new EmptyCookies(m_network));
    m_deadline.setInterval(25);
    connect(&m_deadline, &QTimer::timeout, this, [this] {
        for (qsizetype i = m_next; i < qMin(m_next + 4, m_loads.size()); ++i) {
            auto &load = m_loads[i];
            if (load.started && !load.done && load.elapsed.elapsed() >= deadlineMs) {
                if (load.reply) load.reply->abort();
                finish(i);
            }
        }
    });
}
ResourceController::~ResourceController() { cancel(); m_decoder.waitForDone(); }
void ResourceController::setNetworkManager(QNetworkAccessManager *manager) {
    cancel(); delete m_network; m_network = manager; manager->setParent(this);
    manager->setCookieJar(new EmptyCookies(manager));
}
void ResourceController::cancel()
{
    ++m_generation;
    m_decoder.clear();
    if (m_watcher) m_watcher->cancel();
    delete m_watcher; m_watcher = nullptr;
    m_deadline.stop();
    for (auto &load : m_loads) if (load.reply) { disconnect(load.reply, nullptr, this, nullptr); load.reply->abort(); load.reply->deleteLater(); }
    m_loads.clear(); m_images.clear(); m_next = 0; m_retained = 0;
    // An old decoder can still finish; the single-worker pool serializes generations.
    m_decoding = false;
}
void ResourceController::restart(const QVector<QMarkdownPrivate::Block> &blocks, const QUrl &base, MarkdownResourcePolicy *policy)
{
    cancel();
    m_policy.setAllowedFileRoots(policy->allowedFileRoots()); m_policy.setAllowedHttpsOrigins(policy->allowedHttpsOrigins()); m_policy.setAllowQrc(policy->allowQrc());
    QSet<QUrl> seen;
    std::function<void(const QVector<QMarkdownPrivate::Block> &)> collect = [&](const auto &items) {
        for (const auto &block : items) {
            for (const auto &span : block.images) {
                const auto key = base.resolved(QUrl(span.destination)).adjusted(QUrl::RemoveFragment);
                if (seen.contains(key)) continue;
                seen.insert(key);
                const auto url = QMarkdownPrivate::authorizedResource(key, m_policy);
                if (!url.isEmpty()) { Load load; load.key = key; load.url = url; load.local = url.scheme() != "https"; m_loads.append(load); }
            }
            collect(block.children);
        }
    };
    collect(blocks); m_deadline.start(); pump();
}
void ResourceController::finish(qsizetype index)
{
    auto &load = m_loads[index];
    if (load.done) return;
    if (load.reply) { disconnect(load.reply, nullptr, this, nullptr); load.reply->deleteLater(); load.reply = nullptr; }
    load.done = true;
    pump();
}
void ResourceController::request(qsizetype index)
{
    auto &load = m_loads[index];
    const auto url = QMarkdownPrivate::authorizedResource(load.url, m_policy);
    if (url.isEmpty() || load.elapsed.elapsed() >= deadlineMs) { load.bytes.clear(); finish(index); return; }
    load.url = url;
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setAttribute(QNetworkRequest::CookieLoadControlAttribute, QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::AuthenticationReuseAttribute, QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    request.setAttribute(QNetworkRequest::CacheSaveControlAttribute, false);
    request.setTransferTimeout(deadlineMs);
    auto *reply = m_network->get(request); load.reply = reply;
    reply->setReadBufferSize(64 * 1024);
    const auto generation = m_generation;
    connect(reply, &QNetworkReply::readyRead, this, [this, index, generation, reply] {
        if (generation != m_generation) return;
        auto &load = m_loads[index];
        load.bytes += reply->read(encodedLimit + 1 - load.bytes.size());
        if (load.bytes.size() > encodedLimit || reply->header(QNetworkRequest::ContentLengthHeader).toLongLong() > encodedLimit) {
            load.bytes.clear(); reply->abort(); finish(index);
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, index, generation, reply] {
        if (generation != m_generation || m_loads[index].done) return;
        auto &load = m_loads[index];
        const auto redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError || load.elapsed.elapsed() >= deadlineMs) { load.bytes.clear(); finish(index); return; }
        if (!redirect.isEmpty() && status >= 300 && status < 400) {
            disconnect(reply, nullptr, this, nullptr); reply->deleteLater(); load.reply = nullptr; load.bytes.clear();
            if (++load.redirects > 5) { finish(index); return; }
            load.url = load.url.resolved(redirect);
            // Redirects may never switch to a local resource, even if separately allowed.
            if (load.url.scheme() != "https") { finish(index); return; }
            this->request(index); return;
        }
        load.bytes += reply->read(encodedLimit + 1 - load.bytes.size());
        if (status < 200 || status >= 300 || load.bytes.size() > encodedLimit) load.bytes.clear();
        finish(index);
    });
}
void ResourceController::pump()
{
    if (m_decoding) return;
    while (m_next < m_loads.size() && m_loads[m_next].done && !m_loads[m_next].local && m_loads[m_next].bytes.isEmpty()) ++m_next;
    if (m_next == m_loads.size()) { m_deadline.stop(); return; }
    for (qsizetype i = m_next; i < qMin(m_next + 4, m_loads.size()); ++i) {
        auto &load = m_loads[i];
        if (load.started) continue;
        load.started = true; load.elapsed.start();
        if (!load.local) request(i);
        else { load.done = true; } // Local read and decode share the worker below.
    }
    auto &load = m_loads[m_next];
    if (!load.done) return;
    // A failed network load has no bytes; local files are read only by the worker.
    if (!load.local && load.bytes.isEmpty()) { ++m_next; pump(); return; }
    m_decoding = true;
    const auto generation = m_generation;
    const auto index = m_next;
    const auto url = load.url;
    auto bytes = std::move(load.bytes);
    auto promise = std::make_shared<QPromise<QImage>>(); promise->start();
    auto *watcher = new QFutureWatcher<QImage>(this);
    m_watcher = watcher;
    connect(watcher, &QFutureWatcher<QImage>::finished, this, [this, watcher, generation, index] {
        const auto image = watcher->result(); watcher->deleteLater();
        if (generation != m_generation) return;
        m_watcher = nullptr;
        m_decoding = false;
        if (!image.isNull() && m_loads[index].elapsed.elapsed() < deadlineMs && m_retained + image.sizeInBytes() <= retainedLimit) {
            m_images.insert(m_loads[index].key, image); m_retained += image.sizeInBytes(); emit changed();
        }
        if (generation != m_generation) return;
        ++m_next; pump();
    });
    watcher->setFuture(promise->future());
    m_decoder.start([promise, url, decode = m_decode, bytes = std::move(bytes)]() mutable {
        if (promise->isCanceled()) { promise->finish(); return; }
        if (url.scheme() != "https") {
            QFile file(url.isLocalFile() ? url.toLocalFile() : ":" + url.path());
            if (file.open(QIODevice::ReadOnly) && file.size() <= encodedLimit) bytes = file.read(encodedLimit + 1);
        }
        if (!promise->isCanceled()) promise->addResult(decode(std::move(bytes)));
        promise->finish();
    });
}
