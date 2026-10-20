#include "private/syntaxhighlighter.h"
#include "syntaxhighlightingprovider.h"
#include <QJSEngine>
#include <QtTest/QTest>
#include <QSignalSpy>
#include <QElapsedTimer>
#include <QSemaphore>
#include <QTimer>
using namespace QMarkdownPrivate;

struct FakeControl {
    QSemaphore entered, release;
    std::atomic_int calls{0};
    bool delay = false;
};
class FakeBackend : public HighlightBackend {
    std::shared_ptr<FakeControl> m_control;
public:
    explicit FakeBackend(std::shared_ptr<FakeControl> control) : m_control(std::move(control)) {}
    HighlightSpans highlight(const QString &source, const QString &, const HighlightCancelled &) override {
        ++m_control->calls;
        m_control->entered.release();
        if (m_control->delay) m_control->release.tryAcquire(1, 5000);
        return {{0, int(source.size()), TokenRole::Keyword}};
    }
};
class SyntaxTest : public QObject {
    Q_OBJECT
private slots:
    void providerValidation_data();
    void providerValidation();
    void providerCacheAndSubscriptions();
    void providerIntegration();
    void providerTimeout();
    void providerReentrancy();
    void providerFallback();
    void providerDeadlineAndReentrantDelivery();
    void cleanup() { resetHighlightScheduler(); }
    void palette() {
        SyntaxTheme theme;
        QCOMPARE(theme.keywordColor(), QColor("#7c3aed"));
        QVERIFY(!theme.operatorColor().isValid());
        theme.setKeywordColor(Qt::magenta);
        theme.setPreset(SyntaxTheme::Dark);
        QCOMPARE(theme.keywordColor(), QColor(Qt::magenta));
        QCOMPARE(theme.stringColor(), QColor("#86efac"));
        theme.resetKeywordColor();
        QCOMPARE(theme.keywordColor(), QColor("#c4b5fd"));
        theme.setOperatorColor(Qt::cyan); theme.resetOperatorColor();
        QVERIFY(!theme.operatorColor().isValid());
    }
    void languages_data() {
        QTest::addColumn<QString>("language"); QTest::addColumn<QString>("source");
        QTest::newRow("cpp") << "CPP" << "/* comment\ncontinued */\nconst char *s = \"😀é\";\tint n = 42;\n";
        QTest::newRow("python") << "py" << "s = \"\"\"hello\nworld\"\"\"\nif True:\n\tprint(42)\n";
        QTest::newRow("qml") << "QML" << "import QtQuick\nItem { property string s: \"hello\" }\n";
        QTest::newRow("bash") << "shell" << "#!/bin/bash\necho \"$HOME\" # comment\n";
        QTest::newRow("json") << "json" << "{\"key\": 42, \"other\": true}";
        QTest::newRow("js") << "js" << "/* multiline\ncomment */\nconst x = \"hi\"; function f() { return 1; }";
        QTest::newRow("ts") << "typescript" << "const x: number = 42;";
        QTest::newRow("name") << "pYtHoN" << "if True: print(42)";
        QTest::newRow("extension") << "rs" << "fn main() { let x = 42; }";
        QTest::newRow("embedded") << "HTML" << "<script>const x = 42;</script>";
        QTest::newRow("incomplete") << "cpp" << "const char *x = \"unfinished";
    }
    void languages() {
        QFETCH(QString, language); QFETCH(QString, source);
        auto backend = createHighlightBackend();
        if (!syntaxHighlightingAvailable()) { QVERIFY(!backend); return; }
        auto spans = backend->highlight(source, language, [] { return false; });
        QVERIFY(!spans.isEmpty());
        int end = 0;
        for (auto span : spans) {
            QVERIFY(span.start >= end); QVERIFY(span.length > 0);
            end = span.start + span.length; QVERIFY(end <= source.size());
            QVERIFY(!source[span.start].isLowSurrogate());
            QVERIFY(end == source.size() || !source[end].isLowSurrogate());
        }
        if (language == "CPP") {
            QVERIFY(std::any_of(spans.begin(), spans.end(), [](auto s) { return s.start == 11 && s.role == TokenRole::Comment; }));
            QVERIFY(std::any_of(spans.begin(), spans.end(), [&](auto s) { return s.start == source.indexOf("42") && s.role == TokenRole::Number; }));
        }
    }
    void fallbackAndLimits() {
        auto backend = createHighlightBackend(); if (!backend) return;
        auto never = [] { return false; };
        QVERIFY(backend->highlight("const int x = 1;", "", never).isEmpty());
        QVERIFY(backend->highlight("const int x = 1;", "not-a-real-language", never).isEmpty());
        QVERIFY(backend->highlight(QString(256 * 1024 + 1, u' '), "cpp", never).isEmpty());
        QVERIFY(backend->highlight("int x;\n" + QString(16 * 1024 + 1, u' '), "cpp", never).isEmpty());
        QVERIFY(backend->highlight("int x;", "cpp", [] { return true; }).isEmpty());
        int checks = 0;
        QVERIFY(backend->highlight("int x;\nint y;", "cpp", [&] { return ++checks > 3; }).isEmpty());
    }
    void limitsAreWholeBlock() {
        if (!syntaxHighlightingAvailable()) return;
        auto never = [] { return false; };
        auto spans = createHighlightBackend({256 * 1024, 16 * 1024, 2, 1000});
        QVERIFY(spans->highlight("int a = 1; int b = 2;", "cpp", never).isEmpty());
        auto time = createHighlightBackend({256 * 1024, 16 * 1024, 65536, 0});
        QVERIFY(time->highlight("int a = 1;", "cpp", never).isEmpty());
        auto boundary = createHighlightBackend({12, 12, 65536, 1000});
        QVERIFY(!boundary->highlight("int a = 123;", "cpp", never).isEmpty());
        QVERIFY(boundary->highlight("int a = 1234;", "cpp", never).isEmpty());
    }
    void staleResultsAndDeduplication() {
        if (!syntaxHighlightingAvailable()) return;
        auto control = std::make_shared<FakeControl>(); control->delay = true;
        resetHighlightScheduler([control] { return std::make_unique<FakeBackend>(control); });
        SyntaxTheme theme;
        SyntaxRequest first, duplicate;
        for (auto *request : {&first, &duplicate}) {
            request->setTheme(&theme); request->setLanguage("cpp"); request->setEnabled(true); request->setSource("old");
        }
        QTRY_VERIFY(control->entered.available() > 0);
        QCOMPARE(control->calls.load(), 1);
        first.setSource("new source");
        QCoreApplication::processEvents();
        // A queued GUI action executes while the worker is deliberately blocked.
        bool responsive = false; QTimer::singleShot(0, [&] { responsive = true; });
        QTRY_VERIFY(responsive); QVERIFY(first.ranges().isEmpty());
        control->release.release();
        QTRY_VERIFY(!duplicate.ranges().isEmpty());
        QCOMPARE(duplicate.ranges().first().toMap().value("length").toInt(), 3);
        QTRY_COMPARE(control->calls.load(), 2);
        QVERIFY(first.ranges().isEmpty()); control->release.release();
        QTRY_VERIFY(!first.ranges().isEmpty());
        QCOMPARE(first.ranges().first().toMap().value("length").toInt(), 10);
        theme.setPreset(SyntaxTheme::Dark); QCOMPARE(control->calls.load(), 2);
        auto *transient = new SyntaxRequest;
        transient->setTheme(&theme); transient->setLanguage("cpp"); transient->setEnabled(true); transient->setSource("destroyed");
        QTRY_COMPARE(control->calls.load(), 3);
        delete transient; // Must return while the fake backend remains blocked.
        first.setSource("disable"); first.setEnabled(false);
        control->release.release();
        QCoreApplication::processEvents(); QVERIFY(first.ranges().isEmpty());
    }
    void reentrantDelivery() {
        if (!syntaxHighlightingAvailable()) return;
        auto control = std::make_shared<FakeControl>();
        resetHighlightScheduler([control] { return std::make_unique<FakeBackend>(control); });
        SyntaxTheme theme; SyntaxRequest first, second;
        for (auto *request : {&first, &second}) {
            request->setTheme(&theme); request->setLanguage("cpp"); request->setEnabled(true);
        }
        connect(&first, &SyntaxRequest::rangesChanged, &second, [&] {
            if (!first.ranges().isEmpty()) second.setSource("second source");
        });
        first.setSource("first source");
        QTRY_VERIFY(!second.ranges().isEmpty());
        QCOMPARE(control->calls.load(), 2);
    }
    void cacheEviction() {
        if (!syntaxHighlightingAvailable()) return;
        auto control = std::make_shared<FakeControl>();
        resetHighlightScheduler([control] { return std::make_unique<FakeBackend>(control); });
        SyntaxTheme theme; SyntaxRequest request;
        request.setTheme(&theme); request.setLanguage("cpp"); request.setEnabled(true);
        for (int i = 0; i < 130; ++i) {
            request.setSource(QString("source %1").arg(i));
            QTRY_VERIFY(!request.ranges().isEmpty());
        }
        QCOMPARE(highlightCacheSize().first, 128);
        QCOMPARE(control->calls.load(), 130);
        request.setSource("source 129"); request.setSource("source 128");
        QTRY_VERIFY(!request.ranges().isEmpty()); QCOMPARE(control->calls.load(), 130);
        request.setSource("source 0"); QTRY_VERIFY(!request.ranges().isEmpty());
        QCOMPARE(control->calls.load(), 131);
        for (int i = 0; i < 20; ++i) {
            request.setSource((QString(1023, u' ') + "\n").repeated(255) + QString::number(i));
            QTRY_VERIFY(!request.ranges().isEmpty());
        }
        QVERIFY(highlightCacheSize().second <= 8 * 1024 * 1024);
        QVERIFY(highlightCacheSize().first < 128);
    }
    void asynchronousReplacementAndTheme() {
        SyntaxTheme theme;
        SyntaxRequest request; request.setTheme(&theme); request.setLanguage("cpp"); request.setEnabled(true);
        request.setSource("const int oldValue = 42;\n");
        request.setSource("// replacement\n");
        if (!syntaxHighlightingAvailable()) { QTest::qWait(1); QVERIFY(request.ranges().isEmpty()); return; }
        QTRY_VERIFY(!request.ranges().isEmpty());
        QCOMPARE(request.ranges().first().toMap().value("color").value<QColor>(), theme.commentColor());
        theme.setCommentColor(Qt::magenta);
        QCOMPARE(request.ranges().first().toMap().value("color").value<QColor>(), QColor(Qt::magenta));
        request.setEnabled(false); QVERIFY(request.ranges().isEmpty());
        request.setEnabled(true); request.setSource("plain text"); request.setLanguage("unknown");
        QCoreApplication::processEvents(); QVERIFY(request.ranges().isEmpty());
        auto *external = new SyntaxTheme;
        request.setTheme(external); delete external; QVERIFY(request.theme() == nullptr);
        for (int i = 0; i < 40; ++i) {
            auto *transient = new SyntaxRequest;
            transient->setEnabled(true); transient->setSource("int n = 1;"); transient->setLanguage("cpp");
            QCoreApplication::processEvents(); delete transient;
        }
    }
};
void SyntaxTest::providerValidation_data() {
    QTest::addColumn<QString>("expression"); QTest::addColumn<bool>("valid");
    const QList<QPair<QString, bool>> cases = {
        {"[]", true}, {"[{start:4,length:1,role:11},{start:0,length:2,role:0}]", true},
        {"[{start:2,length:2,role:5}]", true},
        {"null", false}, {"({})", false}, {"[null]", false}, {"[[]]", false},
        {"[{start:'0',length:1,role:0}]", false}, {"[{start:NaN,length:1,role:0}]", false},
        {"[{start:Infinity,length:1,role:0}]", false}, {"[{start:.5,length:1,role:0}]", false},
        {"[{start:0,length:0,role:0}]", false}, {"[{start:-1,length:1,role:0}]", false},
        {"[{start:0,length:6,role:0}]", false}, {"[{start:0,length:1,role:12}]", false},
        {"[{start:0,length:1,role:-1}]", false}, {"[{start:0,length:1,role:'0'}]", false},
        {"[{start:0,length:1,role:0.1}]", false}, {"[{start:0,length:1}]", false},
        {"[{start:0,length:2,role:0},{start:1,length:1,role:1}]", false},
        {"[{start:0,length:1,role:0},{start:0,length:1,role:0}]", false},
        {"[{start:3,length:1,role:0}]", false}, {"[{start:2,length:1,role:0}]", false},
        {"new Array(65537)", false}, {"[{start:1e30,length:1,role:0}]", false},
        {"[{start:0,length:Infinity,role:0}]", false}, {"[{start:0,length:1,role:NaN}]", false}
    };
    int i = 0;
    for (const auto &entry : cases) QTest::newRow(qPrintable(QString::number(i++))) << entry.first << entry.second;
}
void SyntaxTest::providerValidation() {
    QFETCH(QString, expression); QFETCH(bool, valid);
    QJSEngine engine; SyntaxHighlightingProvider provider;
    QPointer<SyntaxHighlightingRequest> host;
    connect(&provider, &SyntaxHighlightingProvider::highlightRequested, &provider, [&](auto *request) {
        host = request;
        QCOMPARE(request->source(), QString::fromUtf8("ab😀c"));
        QCOMPARE(request->language(), QString("Exact-Lang"));
        request->complete(engine.evaluate(expression));
        request->decline(); request->complete(engine.evaluate("[]"));
    });
    int calls = 0; HighlightSpans result; bool success = !valid;
    provider.subscribe(QString::fromUtf8("ab😀c"), "Exact-Lang", std::make_shared<std::atomic_bool>(false),
        [&](bool accepted, const auto &spans) { ++calls; success = accepted; result = spans; });
    QCOMPARE(calls, 1); QCOMPARE(success, valid);
    if (valid && !result.isEmpty()) {
        int end = 0; for (auto span : result) { QVERIFY(span.start >= end); end = span.start + span.length; }
    }
    QVERIFY(host && !host->cancelled());
}
void SyntaxTest::providerCacheAndSubscriptions() {
    QJSEngine engine; SyntaxHighlightingProvider provider, other;
    QList<QPointer<SyntaxHighlightingRequest>> pending;
    auto capture = [&](auto *request) { pending.append(request); };
    connect(&provider, &SyntaxHighlightingProvider::highlightRequested, &provider, capture);
    connect(&other, &SyntaxHighlightingProvider::highlightRequested, &other, capture);
    int calls = 0;
    auto done = [&](bool success, const auto &) { QVERIFY(success); ++calls; };
    auto flag = [] { return std::make_shared<std::atomic_bool>(false); };
    auto a = flag(), b = flag();
    const auto first = provider.subscribe("abc", "Case", a, done);
    provider.subscribe("abc", "Case", b, done);
    QCOMPARE(pending.size(), 1);
    a->store(true); provider.unsubscribe(first); QVERIFY(!pending[0]->cancelled());
    pending[0]->complete(engine.evaluate("[]")); QCOMPARE(calls, 1);
    provider.subscribe("abc", "Case", flag(), done); QCOMPARE(calls, 2); QCOMPARE(pending.size(), 1);
    provider.subscribe("abc", "case", flag(), done); QCOMPARE(pending.size(), 2);
    other.subscribe("abc", "Case", flag(), done); QCOMPARE(pending.size(), 3);
    provider.invalidate(); QVERIFY(pending[1]->cancelled());
    pending[1]->complete(engine.evaluate("[]")); QCOMPARE(calls, 2);
    provider.subscribe("abc", "Case", flag(), done); QCOMPARE(pending.size(), 4);
    const auto last = provider.subscribe("cancel", "Case", flag(), done);
    provider.unsubscribe(last); QVERIFY(pending.last()->cancelled());
    pending[2]->complete(engine.evaluate("[]")); QCOMPARE(calls, 3);
}
void SyntaxTest::providerIntegration() {
    QJSEngine engine; SyntaxHighlightingProvider provider;
    SyntaxTheme theme; SyntaxRequest first, second;
    int calls = 0;
    QList<QPointer<SyntaxHighlightingRequest>> pending;
    connect(&provider, &SyntaxHighlightingProvider::highlightRequested, &provider, [&](auto *request) {
        ++calls; pending.append(request);
    });
    for (auto *request : {&first, &second}) {
        request->setProvider(&provider); request->setTheme(&theme); request->setEnabled(true);
        request->setLanguage("HostOnly"); request->setSource("hello");
    }
    QTRY_COMPARE(calls, 1);
    first.setEnabled(false); QVERIFY(!pending[0]->cancelled());
    pending[0]->complete(engine.evaluate("[{start:0,length:5,role:0}]"));
    QTRY_COMPARE(second.ranges().size(), 1); QVERIFY(first.ranges().isEmpty());
    theme.setKeywordColor(Qt::cyan); QCOMPARE(calls, 1);
    QCOMPARE(second.ranges()[0].toMap()["color"].value<QColor>(), QColor(Qt::cyan));
    first.setEnabled(true); QTRY_COMPARE(first.ranges().size(), 1); QCOMPARE(calls, 1);
    provider.invalidate(); QTRY_COMPARE(calls, 2);
    first.setSource("changed"); QTRY_COMPARE(calls, 3);
    second.setEnabled(false); QVERIFY(pending[1]->cancelled());
    pending[1]->complete(engine.evaluate("[{start:0,length:5,role:0}]")); QVERIFY(second.ranges().isEmpty());
    pending[2]->decline();
    first.setSource(QString(16 * 1024 + 1, u'x')); QCoreApplication::processEvents(); QCOMPARE(calls, 3);
    first.setSource(QString(256 * 1024 + 1, u'x')); QCoreApplication::processEvents(); QCOMPARE(calls, 3);
    first.setSource(""); QCoreApplication::processEvents(); QCOMPARE(calls, 3);
    first.setSource("a"); first.setLanguage(""); QCoreApplication::processEvents(); QCOMPARE(calls, 3);
    first.setLanguage("HostOnly"); QTRY_COMPARE(calls, 4);
    first.setProvider(nullptr); QVERIFY(pending.last()->cancelled());
    auto *temporary = new SyntaxHighlightingProvider;
    connect(temporary, &SyntaxHighlightingProvider::highlightRequested, temporary, [&](auto *r) { pending.append(r); });
    first.setProvider(temporary); QTRY_VERIFY(pending.last() && pending.last() != pending[pending.size() - 2]);
    const auto retained = pending.last(); delete temporary;
    QVERIFY(retained && retained->cancelled()); QVERIFY(!first.provider());
}
void SyntaxTest::providerTimeout() {
    SyntaxHighlightingProvider provider;
    QPointer<SyntaxHighlightingRequest> host; int calls = 0;
    connect(&provider, &SyntaxHighlightingProvider::highlightRequested, &provider, [&](auto *r) { host = r; });
    provider.subscribe("x", "host", std::make_shared<std::atomic_bool>(false), [&](bool success, const auto &) {
        QVERIFY(!success); QVERIFY(host->cancelled()); ++calls;
    });
    QElapsedTimer elapsed; elapsed.start();
    QTRY_COMPARE_WITH_TIMEOUT(calls, 1, 6500);
    QVERIFY(elapsed.elapsed() >= 4900);
    QCOMPARE(provider.cacheSize().first, 0);
}
void SyntaxTest::providerReentrancy() {
    QJSEngine engine; SyntaxHighlightingProvider provider;
    SyntaxTheme theme; SyntaxRequest request;
    int calls = 0;
    connect(&provider, &SyntaxHighlightingProvider::highlightRequested, &provider, [&](auto *host) {
        if (++calls == 1) request.setSource("replacement");
        else host->complete(engine.evaluate("[{start:0,length:1,role:1}]"));
    });
    request.setProvider(&provider); request.setTheme(&theme); request.setEnabled(true);
    request.setLanguage("host"); request.setSource("old");
    QTRY_COMPARE(calls, 2); QTRY_COMPARE(request.ranges().size(), 1);
    QCOMPARE(provider.cacheSize().first, 1);
    // Successful-empty results cache; invalid and declined decisions do not.
    SyntaxHighlightingProvider cache;
    int cachedCalls = 0;
    connect(&cache, &SyntaxHighlightingProvider::highlightRequested, &cache, [&](auto *host) {
        ++cachedCalls; host->complete(engine.evaluate("[]"));
    });
    for (int i = 0; i < 130; ++i) cache.subscribe(QString::number(i), "host", std::make_shared<std::atomic_bool>(false), [](bool ok, const auto &) { QVERIFY(ok); });
    QCOMPARE(cache.cacheSize().first, 128); QCOMPARE(cachedCalls, 130);
    cache.subscribe("129", "host", std::make_shared<std::atomic_bool>(false), [](bool, const auto &) {});
    QCOMPARE(cachedCalls, 130);
    cache.subscribe("0", "host", std::make_shared<std::atomic_bool>(false), [](bool, const auto &) {});
    QCOMPARE(cachedCalls, 131);
    cache.invalidate(); QCOMPARE(cache.cacheSize().first, 0);
}

void SyntaxTest::providerFallback() {
    QJSEngine engine; SyntaxHighlightingProvider provider;
    auto control = std::make_shared<FakeControl>();
    resetHighlightScheduler([control] { return std::make_unique<FakeBackend>(control); });
    SyntaxTheme theme; SyntaxRequest request;
    int calls = 0; QPointer<SyntaxHighlightingRequest> host;
    connect(&provider, &SyntaxHighlightingProvider::highlightRequested, &provider, [&](auto *r) { ++calls; host = r; });
    request.setProvider(&provider); request.setTheme(&theme); request.setEnabled(true);
    request.setLanguage("cpp"); request.setSource("int x;");
    QTRY_COMPARE(calls, 1); host->complete(engine.evaluate("[]"));
    QTest::qWait(20); QVERIFY(request.ranges().isEmpty()); QCOMPARE(control->calls.load(), 0);
    provider.invalidate(); QTRY_COMPARE(calls, 2); host->decline();
    if (syntaxHighlightingAvailable()) { QTRY_COMPARE(request.ranges().size(), 1); QCOMPARE(control->calls.load(), 1); }
    else { QTest::qWait(20); QVERIFY(request.ranges().isEmpty()); }
    provider.invalidate(); QTRY_COMPARE(calls, 3);
    host->complete(engine.evaluate("[{start:-1,length:1,role:0}]"));
    if (syntaxHighlightingAvailable()) QTRY_COMPARE(request.ranges().size(), 1);
    else { QTest::qWait(20); QVERIFY(request.ranges().isEmpty()); }
    provider.invalidate(); QTRY_COMPARE(calls, 4); // Invalid results are not cached.
    host->complete(engine.evaluate("[{start:0,length:6,role:5}]"));
    QTRY_COMPARE(request.ranges().size(), 1);
    QCOMPARE(request.ranges()[0].toMap()["color"].value<QColor>(), theme.stringColor());
    request.setProvider(nullptr);
    if (syntaxHighlightingAvailable()) {
        QTRY_COMPARE(request.ranges().size(), 1);
        QCOMPARE(request.ranges()[0].toMap()["color"].value<QColor>(), theme.keywordColor());
    }
    else { QTest::qWait(20); QVERIFY(request.ranges().isEmpty()); }
}

void SyntaxTest::providerDeadlineAndReentrantDelivery() {
    QJSEngine engine;
    SyntaxHighlightingProvider provider;
    QPointer<SyntaxHighlightingRequest> host;
    connect(&provider, &SyntaxHighlightingProvider::highlightRequested, &provider, [&](auto *r) { host = r; });
    int calls = 0;
    auto flag = [] { return std::make_shared<std::atomic_bool>(false); };
    provider.subscribe("late", "host", flag(), [&](bool ok, const auto &) { QVERIFY(!ok); ++calls; });
    QTest::qSleep(5010); // A blocked QML thread cannot extend the completion deadline.
    host->complete(engine.evaluate("[]")); QCOMPARE(calls, 1); QVERIFY(host->cancelled());
    provider.subscribe("shared", "host", flag(), [&](bool ok, const auto &) { QVERIFY(ok); ++calls; provider.invalidate(); });
    provider.subscribe("shared", "host", flag(), [&](bool, const auto &) { QFAIL("Invalidated generation delivered a stale result"); });
    host->complete(engine.evaluate("[]")); QCOMPARE(calls, 2);
    auto *temporary = new SyntaxHighlightingProvider;
    connect(temporary, &SyntaxHighlightingProvider::highlightRequested, temporary, [&](auto *r) { host = r; });
    temporary->subscribe("destroy", "host", flag(), [&](bool ok, const auto &) { QVERIFY(ok); ++calls; delete temporary; });
    temporary->subscribe("destroy", "host", flag(), [&](bool, const auto &) { QFAIL("Destroyed provider delivered again"); });
    host->complete(engine.evaluate("[]")); QCOMPARE(calls, 3);
    SyntaxHighlightingProvider cancelled;
    SyntaxRequest subscription;
    subscription.setProvider(&cancelled); subscription.setSource("old"); subscription.setLanguage("host"); subscription.setEnabled(true);
    connect(&cancelled, &SyntaxHighlightingProvider::highlightRequested, &cancelled, [&](auto *r) {
        if (r->source() != "old") { r->complete(engine.evaluate("[]")); return; }
        subscription.setSource("new");
        QVERIFY(r->cancelled()); r->complete(engine.evaluate("[]"));
    });
    QTRY_COMPARE(cancelled.cacheSize().first, 1);
    // Every role is accepted and the cache counts its retained source, even for empty results.
    SyntaxHighlightingProvider bounded;
    connect(&bounded, &SyntaxHighlightingProvider::highlightRequested, &bounded, [&](auto *r) {
        r->complete(engine.evaluate("Array.from({length:12}, (_,role) => ({start:role,length:1,role}))"));
    });
    for (int i = 0; i < 20; ++i)
        bounded.subscribe((QString(1023, u' ') + "\n").repeated(255) + QString::number(i), "host", flag(), [](bool ok, const auto &spans) { QVERIFY(ok); QCOMPARE(spans.size(), 12); });
    QVERIFY(bounded.cacheSize().first < 20); QVERIFY(bounded.cacheSize().second <= 8 * 1024 * 1024);
}

QTEST_MAIN(SyntaxTest)
#include "tst_syntax.moc"
