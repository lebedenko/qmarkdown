#include "private/resourcecontroller.h"
#include <QBuffer>
#include <QSemaphore>
#include <atomic>
#include <memory>
#include <thread>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>
#include <QSignalSpy>
#include <QNetworkCookie>
#include <QtTest/QTest>
using namespace QMarkdownPrivate;

namespace {
QByteArray png(int width = 20, int height = 10) {
    QImage image(width, height, QImage::Format_ARGB32); image.fill(Qt::red);
    QByteArray bytes; QBuffer buffer(&bytes); buffer.open(QIODevice::WriteOnly); image.save(&buffer, "PNG"); return bytes;
}
// Gates own no controller state; the worker keeps them alive after cancellation.
struct DecodeGate {
    QSemaphore entered, release, completed;
    std::atomic<bool> timedOut{false};
    QImage decode(QByteArray bytes) {
        entered.release();
        if (!release.tryAcquire(1, 5000)) timedOut = true;
        auto result = decodeImage(std::move(bytes));
        completed.release();
        return result;
    }
};
struct ReleaseGate {
    std::shared_ptr<DecodeGate> gate;
    ~ReleaseGate() { gate->release.release(64); }
};
struct Response {
    QByteArray bytes;
    QUrl redirect;
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    bool held = false;
    int status = 200;
};
class FakeReply : public QNetworkReply {
public:
    FakeReply(const QNetworkRequest &request, Response response, QObject *parent)
        : QNetworkReply(parent), m_response(std::move(response)) {
        setRequest(request); setUrl(request.url()); setOperation(QNetworkAccessManager::GetOperation);
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, m_response.status);
        if (!m_response.redirect.isEmpty()) {
            setAttribute(QNetworkRequest::RedirectionTargetAttribute, m_response.redirect);
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 302);
        }
        open(QIODevice::ReadOnly);
        if (!m_response.held) QTimer::singleShot(0, this, [this] { complete(); });
    }
    void complete() {
        if (isFinished()) return;
        if (m_response.error != NoError) setError(m_response.error, "Fake failure");
        emit readyRead();
        if (isFinished()) return;
        setFinished(true); emit finished();
    }
    void abort() override { if (isFinished()) return; aborted = true; setError(OperationCanceledError, "Canceled"); setFinished(true); emit finished(); }
    qint64 bytesAvailable() const override { return m_response.bytes.size() - m_offset + QNetworkReply::bytesAvailable(); }
    bool aborted = false;
protected:
    qint64 readData(char *data, qint64 maximum) override {
        const auto count = qMin(maximum, qint64(m_response.bytes.size() - m_offset));
        if (!count) return -1;
        memcpy(data, m_response.bytes.constData() + m_offset, count); m_offset += count; return count;
    }
private:
    Response m_response;
    qint64 m_offset = 0;
};
class FakeManager : public QNetworkAccessManager {
public:
    QHash<QUrl, Response> responses;
    QList<QNetworkRequest> requests;
    QList<QPointer<FakeReply>> replies;
protected:
    QNetworkReply *createRequest(Operation, const QNetworkRequest &request, QIODevice *) override {
        requests.append(request);
        auto *reply = new FakeReply(request, responses.value(request.url()), this); replies.append(reply); return reply;
    }
};
void write(const QString &path, const QByteArray &bytes) { QFile file(path); if (file.open(QIODevice::WriteOnly)) file.write(bytes); }
}
class ResourceTest : public QObject {
    Q_OBJECT
private slots:
    void permissions() {
        QTemporaryDir temp; QVERIFY(temp.isValid());
        QDir dir(temp.path()); QVERIFY(dir.mkdir("root")); QVERIFY(dir.mkdir("root-other"));
        const auto file = temp.filePath("root/image.png"), outside = temp.filePath("root-other/image.png");
        write(file, png()); write(outside, png());
        MarkdownResourcePolicy policy;
        QVERIFY(authorizedResource(QUrl::fromLocalFile(file), policy).isEmpty());
        QVERIFY(authorizedResource(QUrl("qrc:/image.png"), policy).isEmpty());
        QVERIFY(authorizedResource(QUrl("https://example.invalid/a"), policy).isEmpty());
        policy.setAllowedFileRoots({QUrl::fromLocalFile(temp.filePath("root"))});
        QCOMPARE(authorizedResource(QUrl::fromLocalFile(file), policy), QUrl::fromLocalFile(file));
        QVERIFY(authorizedResource(QUrl::fromLocalFile(outside), policy).isEmpty());
        QVERIFY(authorizedResource(QUrl::fromLocalFile(temp.filePath("root/../root-other/image.png")), policy).isEmpty());
        QCOMPARE(authorizedResource(QUrl("file://localhost" + file), policy), QUrl::fromLocalFile(file));
        QVERIFY(authorizedResource(QUrl("file://remote" + file), policy).isEmpty());
        QVERIFY(QFile::link(outside, temp.filePath("root/escape.png")));
        QVERIFY(authorizedResource(QUrl::fromLocalFile(temp.filePath("root/escape.png")), policy).isEmpty());
        QVERIFY(QFile::link(file, temp.filePath("root/inside.png")));
        QCOMPARE(authorizedResource(QUrl::fromLocalFile(temp.filePath("root/inside.png")), policy), QUrl::fromLocalFile(file));
        for (const QUrl &invalid : {QUrl("root"), QUrl::fromLocalFile(file), QUrl("file://remote" + temp.path()), QUrl("file:relative")}) {
            policy.setAllowedFileRoots({invalid}); QVERIFY(authorizedResource(QUrl::fromLocalFile(file), policy).isEmpty());
        }
        policy.setAllowQrc(true); QVERIFY(!authorizedResource(QUrl("qrc:/image.png"), policy).isEmpty());
        QVERIFY(authorizedResource(QUrl("qrc://remote/image.png"), policy).isEmpty());
        policy.setAllowedHttpsOrigins({QUrl("https://EXAMPLE.invalid:443")});
        QVERIFY(!authorizedResource(QUrl("https://example.invalid/a?q=1#fragment"), policy).isEmpty());
        QVERIFY(authorizedResource(QUrl("https://example.invalid:444/a"), policy).isEmpty());
        QVERIFY(authorizedResource(QUrl("https://example.invalid.evil/a"), policy).isEmpty());
        QVERIFY(authorizedResource(QUrl("https://user@example.invalid/a"), policy).isEmpty());
        QVERIFY(authorizedResource(QUrl("https://@example.invalid/a"), policy).isEmpty());
        for (const auto &invalid : {"https://example.invalid/path", "https://example.invalid/?q=1", "https://example.invalid/#x", "https://user@example.invalid", "http://example.invalid"}) {
            policy.setAllowedHttpsOrigins({QUrl(invalid)}); QVERIFY(authorizedResource(QUrl("https://example.invalid/a"), policy).isEmpty());
        }
        for (const auto &invalid : {"http://example.invalid/a", "data:image/png,a", "ftp://example.invalid/a", "relative.png", "image://provider/a"})
            QVERIFY(authorizedResource(QUrl(invalid), policy).isEmpty());
    }
    void decodeLimits() {
        const auto image = decodeImage(png()); QCOMPARE(image.size(), QSize(20, 10)); QCOMPARE(image.devicePixelRatio(), 1.0);
        QVERIFY(decodeImage("<svg/>").isNull()); QVERIFY(decodeImage("GIF89a").isNull());
        QVERIFY(decodeImage(QByteArray::fromHex("89504e470d0a1a0a") + "bad").isNull());
        QVERIFY(decodeImage(png().left(24)).isNull());
        QVERIFY(decodeImage(QByteArray(8 * 1024 * 1024 + 1, 'x')).isNull());
        auto padded = png(); padded.append(QByteArray(8 * 1024 * 1024 - padded.size(), '\0'));
        QVERIFY(!decodeImage(padded).isNull()); padded.append('x'); QVERIFY(decodeImage(padded).isNull());
        auto animated = png();
        // APNG acTL: one frame, infinite plays, with a valid CRC.
        animated.insert(33, QByteArray::fromHex("000000086163544c0000000100000000b42de9a0"));
        QVERIFY(decodeImage(animated).isNull());
        QVERIFY(decodeImage(png(4001, 4000)).isNull());
        QVERIFY(!decodeImage(png(4000, 4000)).isNull());
        QByteArray jpeg; QBuffer buffer(&jpeg); buffer.open(QIODevice::WriteOnly); image.save(&buffer, "JPEG");
        QCOMPARE(decodeImage(jpeg).size(), image.size());
        // EXIF orientation 6 rotates the JPEG 90 degrees clockwise.
        jpeg.insert(2, QByteArray::fromHex("ffe1002245786966000049492a0008000000010012010300010000000600000000000000"));
        QCOMPARE(decodeImage(jpeg).size(), QSize(10, 20));
    }
    void localResolutionAndProjection() {
        QTemporaryDir temp; const auto file = temp.filePath("image.png"); write(file, png());
        MarkdownResourcePolicy policy; policy.setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        ResourceController controller; const auto blocks = parse("😀 **before ![*alt*](image.png) after** ![](image.png)");
        controller.restart(blocks, {}, &policy); QVERIFY(controller.images().isEmpty());
        const auto base = QUrl::fromLocalFile(temp.filePath("document.md"));
        QSignalSpy ready(&controller, &ResourceController::changed); controller.restart(blocks, base, &policy);
        QTRY_COMPARE(ready.size(), 1); QCOMPARE(controller.images().size(), 1);
        const auto projected = projectImages(blocks, controller.images(), base);
        QCOMPARE(projected[0].kind, BlockKind::Segments); QCOMPARE(projected[0].children.size(), 4);
        QCOMPARE(projected[0].children[0].text, QString::fromUtf8("😀 before "));
        QCOMPARE(projected[0].children[1].kind, BlockKind::Image);
        QCOMPARE(projected[0].children[2].text, " after ");
        QCOMPARE(projected[0].children[2].ranges[0].start, 0); QCOMPARE(projected[0].children[2].ranges[0].length, 6);
        controller.restart({}, base, &policy); QVERIFY(controller.images().isEmpty());
    }
    void networkPolicyDedupAndRedirects() {
        ResourceController controller; auto *manager = new FakeManager; controller.setNetworkManager(manager);
        MarkdownResourcePolicy policy;
        const auto blocks = parse("![a](https://a.invalid/image) ![b](https://a.invalid/image)");
        controller.restart(blocks, {}, &policy); QCOMPARE(manager->requests.size(), 0);
        policy.setAllowedHttpsOrigins({QUrl("https://a.invalid")});
        controller.restart(parse("<img src='https://a.invalid/image'>\n\ntext <img src='https://a.invalid/image'>"), {}, &policy);
        QCOMPARE(manager->requests.size(), 0);
        manager->responses[QUrl("https://a.invalid/image")] = {png(), QUrl("https://b.invalid/image")};
        controller.restart(blocks, {}, &policy); QTRY_COMPARE(manager->requests.size(), 1);
        QTRY_VERIFY(manager->replies[0].isNull()); QVERIFY(controller.images().isEmpty());
        policy.setAllowedHttpsOrigins({QUrl("https://a.invalid"), QUrl("https://b.invalid")});
        manager->responses[QUrl("https://b.invalid/image")] = {png()};
        QSignalSpy ready(&controller, &ResourceController::changed); controller.restart(blocks, {}, &policy);
        QTRY_COMPARE(ready.size(), 1); QCOMPARE(manager->requests.size(), 3); QCOMPARE(controller.images().size(), 1);
        for (const auto &request : manager->requests) {
            QCOMPARE(request.attribute(QNetworkRequest::RedirectPolicyAttribute).toInt(), int(QNetworkRequest::ManualRedirectPolicy));
            QCOMPARE(request.attribute(QNetworkRequest::CookieLoadControlAttribute).toInt(), int(QNetworkRequest::Manual));
            QCOMPARE(request.attribute(QNetworkRequest::CookieSaveControlAttribute).toInt(), int(QNetworkRequest::Manual));
            QCOMPARE(request.attribute(QNetworkRequest::AuthenticationReuseAttribute).toInt(), int(QNetworkRequest::Manual));
            QCOMPARE(request.attribute(QNetworkRequest::CacheSaveControlAttribute).toBool(), false);
        }
        // The original image's key survives redirect resolution.
        QVERIFY(controller.images().contains(QUrl("https://a.invalid/image")));
        QTemporaryDir temp; write(temp.filePath("image.png"), png());
        policy.setAllowedFileRoots({QUrl::fromLocalFile(temp.path())});
        manager->responses[QUrl("https://a.invalid/image")] = {png(), QUrl::fromLocalFile(temp.filePath("image.png"))};
        controller.restart(blocks, {}, &policy); QTRY_VERIFY(manager->replies.last().isNull()); QVERIFY(controller.images().isEmpty());
        const auto before = manager->requests.size();
        manager->responses[QUrl("https://a.invalid/image")] = {{}, QUrl("/image")};
        controller.restart(blocks, {}, &policy); QTRY_COMPARE(manager->requests.size(), before + 6);
        QTRY_VERIFY(manager->replies.last().isNull()); QVERIFY(controller.images().isEmpty());
    }
    void errors_data() {
        QTest::addColumn<int>("error"); QTest::addColumn<int>("status"); QTest::addColumn<QByteArray>("bytes");
        QTest::newRow("tls") << int(QNetworkReply::SslHandshakeFailedError) << 200 << png();
        QTest::newRow("connection") << int(QNetworkReply::ConnectionRefusedError) << 200 << png();
        QTest::newRow("404") << 0 << 404 << png();
        QTest::newRow("malformed") << 0 << 200 << QByteArray("bad");
        QTest::newRow("encoded-limit") << 0 << 200 << QByteArray(8 * 1024 * 1024 + 1, 'x');
    }
    void errors() {
        QFETCH(int, error); QFETCH(int, status); QFETCH(QByteArray, bytes);
        ResourceController controller; auto *manager = new FakeManager; controller.setNetworkManager(manager);
        MarkdownResourcePolicy policy; policy.setAllowedHttpsOrigins({QUrl("https://a.invalid")});
        manager->responses[QUrl("https://a.invalid/image")] = {bytes, {}, QNetworkReply::NetworkError(error), false, status};
        controller.restart(parse("![alt](https://a.invalid/image)"), {}, &policy);
        QTRY_VERIFY(controller.idle());
        QVERIFY(controller.images().isEmpty()); QCOMPARE(manager->requests.size(), 1);
    }
    void cancellationAndConcurrency() {
        ResourceController controller; auto *manager = new FakeManager; controller.setNetworkManager(manager);
        MarkdownResourcePolicy policy; policy.setAllowedHttpsOrigins({QUrl("https://a.invalid")});
        QString source;
        for (int i = 0; i < 8; ++i) {
            const auto url = QUrl("https://a.invalid/" + QString::number(i));
            manager->responses[url] = {png(), {}, QNetworkReply::NoError, true}; source += "![alt](" + url.toString() + ") ";
        }
        controller.restart(parse(source), {}, &policy); QCOMPARE(manager->requests.size(), 4);
        auto first = manager->replies[0]; auto second = manager->replies[1];
        second->complete(); QVERIFY(controller.images().isEmpty()); // admission waits for document order
        controller.restart({}, {}, &policy); QVERIFY(first->aborted); QVERIFY(controller.images().isEmpty());
        // Late completion from an aborted old generation cannot admit an image.
        first->complete(); QCoreApplication::processEvents(); QVERIFY(controller.images().isEmpty());
        manager->responses[QUrl("https://a.invalid/new")] = {png()};
        controller.restart(parse("![](https://a.invalid/new)"), {}, &policy);
        QTRY_COMPARE(controller.images().size(), 1); QVERIFY(controller.images().contains(QUrl("https://a.invalid/new")));
        controller.restart(parse("![](https://a.invalid/0)"), {}, &policy); QVERIFY(controller.images().isEmpty());
        controller.restart({}, {}, &policy);
    }
    void staleDecoderCompletion_data() {
        QTest::addColumn<QString>("change");
        for (const auto &change : {"replacement", "policy", "base", "repeated"})
            QTest::newRow(change) << QString(change);
    }
    void staleDecoderCompletion() {
        QFETCH(QString, change);
        ResourceController controller;
        auto gate = std::make_shared<DecodeGate>();
        ReleaseGate cleanup{gate}; // Released before controller's waiting destructor, even on assertion failure.
        controller.setDecoder([gate](QByteArray bytes) { return gate->decode(std::move(bytes)); });
        auto *manager = new FakeManager; controller.setNetworkManager(manager);
        MarkdownResourcePolicy policy; policy.setAllowedHttpsOrigins({QUrl("https://a.invalid")});
        const QUrl old("https://a.invalid/old/image"), next("https://a.invalid/new/image");
        manager->responses[old] = {png(), {}, QNetworkReply::NoError, true};
        manager->responses[next] = {png(), {}, QNetworkReply::NoError, true};
        QSignalSpy published(&controller, &ResourceController::changed);
        const int repetitions = change == "repeated" ? 5 : 1;
        for (int iteration = 0; iteration < repetitions; ++iteration) {
            published.clear();
            controller.restart(parse("![](image)"), QUrl("https://a.invalid/old/"), &policy);
            manager->replies.last()->complete();
            QVERIFY(gate->entered.tryAcquire(1, 2000)); // Cancellation now occurs inside the real decoder wrapper.
            if (change == "policy") {
                policy.setAllowedHttpsOrigins({});
                controller.restart(parse("![](image)"), QUrl("https://a.invalid/old/"), &policy);
            } else if (change == "base") {
                controller.restart(parse("![](image)"), QUrl("https://a.invalid/new/"), &policy);
            } else {
                if (change == "repeated")
                    for (int i = 0; i < 20; ++i) controller.restart({}, {}, &policy);
                controller.restart(parse("![](https://a.invalid/new/image)"), {}, &policy);
            }
            gate->release.release();
            QVERIFY(gate->completed.tryAcquire(1, 2000));
            QCoreApplication::processEvents();
            QCOMPARE(published.size(), 0); QVERIFY(controller.images().isEmpty());
            if (change == "policy") {
                policy.setAllowedHttpsOrigins({QUrl("https://a.invalid")});
                controller.restart(parse("![](https://a.invalid/new/image)"), {}, &policy);
            }
            manager->replies.last()->complete();
            QVERIFY(gate->entered.tryAcquire(1, 2000));
            QVERIFY(controller.images().isEmpty());
            gate->release.release();
            QTRY_VERIFY_WITH_TIMEOUT(controller.idle(), 2000);
            QCOMPARE(published.size(), 1); QCOMPARE(controller.images().size(), 1);
            QVERIFY(controller.images().contains(next)); QVERIFY(!controller.images().contains(old));
            QVERIFY(!gate->timedOut);
        }
    }
    void pendingNetworkDestruction() {
        auto controller = std::make_unique<ResourceController>();
        auto *manager = new FakeManager; controller->setNetworkManager(manager);
        MarkdownResourcePolicy policy; policy.setAllowedHttpsOrigins({QUrl("https://a.invalid")});
        manager->responses[QUrl("https://a.invalid/image")] = {png(), {}, QNetworkReply::NoError, true};
        controller->restart(parse("![](https://a.invalid/image)"), {}, &policy);
        auto reply = manager->replies.last();
        bool aborted = false; int publications = 0;
        connect(reply, &QNetworkReply::finished, this, [&] { aborted = reply->aborted; });
        connect(controller.get(), &ResourceController::changed, this, [&] { ++publications; });
        controller.reset();
        QVERIFY(aborted); QVERIFY(reply.isNull()); QCOMPARE(publications, 0);
        QCoreApplication::processEvents(); QCOMPARE(publications, 0);
    }
    void activeDecodeDestruction() {
        auto controller = std::make_unique<ResourceController>();
        auto gate = std::make_shared<DecodeGate>(); ReleaseGate cleanup{gate};
        controller->setDecoder([gate](QByteArray bytes) { return gate->decode(std::move(bytes)); });
        auto *manager = new FakeManager; controller->setNetworkManager(manager);
        MarkdownResourcePolicy policy; policy.setAllowedHttpsOrigins({QUrl("https://a.invalid")});
        manager->responses[QUrl("https://a.invalid/image")] = {png(), {}, QNetworkReply::NoError, true};
        int publications = 0;
        connect(controller.get(), &ResourceController::changed, this, [&] { ++publications; });
        controller->restart(parse("![](https://a.invalid/image)"), {}, &policy);
        manager->replies.last()->complete(); QVERIFY(gate->entered.tryAcquire(1, 2000));
        // Release from another thread: destruction on this thread waits for the active worker.
        auto *watcher = controller->findChild<QFutureWatcherBase *>(); QVERIFY(watcher);
        QSemaphore cancelled;
        connect(watcher, &QObject::destroyed, this, [&] { cancelled.release(); });
        std::atomic<bool> observedCancellation{false};
        std::thread release([gate, &cancelled, &observedCancellation] {
            observedCancellation = cancelled.tryAcquire(1, 2000);
            gate->release.release(); // Also releases on a failed cancellation observation.
        });
        controller.reset(); release.join();
        QVERIFY(observedCancellation);
        QVERIFY(gate->completed.tryAcquire(1, 2000)); QVERIFY(!gate->timedOut);
        QCoreApplication::processEvents(); QCOMPARE(publications, 0);
    }
    void deadline() {
        ResourceController controller; auto *manager = new FakeManager; controller.setNetworkManager(manager);
        MarkdownResourcePolicy policy; policy.setAllowedHttpsOrigins({QUrl("https://a.invalid")});
        manager->responses[QUrl("https://a.invalid/image")] = {png(), {}, QNetworkReply::NoError, true};
        controller.restart(parse("![](https://a.invalid/image)"), {}, &policy);
        QTRY_VERIFY_WITH_TIMEOUT(manager->replies[0].isNull(), 16000); QVERIFY(controller.images().isEmpty());
        QCOMPARE(manager->requests.size(), 1);
    }
    void retainedBudgetAndOrder() {
        ResourceController controller; auto *manager = new FakeManager; controller.setNetworkManager(manager);
        MarkdownResourcePolicy policy; policy.setAllowedHttpsOrigins({QUrl("https://a.invalid")});
        QString source;
        const auto bytes = png(2000, 2000); // 16,000,000 decoded bytes each: four fit, fifth does not.
        for (int i = 0; i < 5; ++i) {
            const QUrl url("https://a.invalid/" + QString::number(i)); manager->responses[url] = {bytes}; source += "![](" + url.toString() + ") ";
        }
        QSignalSpy ready(&controller, &ResourceController::changed); controller.restart(parse(source), {}, &policy);
        QTRY_COMPARE(ready.size(), 4); QTRY_COMPARE(manager->requests.size(), 5);
        QTRY_VERIFY(controller.idle());
        QCOMPARE(controller.images().size(), 4);
        for (int i = 0; i < 4; ++i) QVERIFY(controller.images().contains(QUrl("https://a.invalid/" + QString::number(i))));
        QVERIFY(!controller.images().contains(QUrl("https://a.invalid/4")));
    }
};
QTEST_GUILESS_MAIN(ResourceTest)
#include "tst_resources.moc"
