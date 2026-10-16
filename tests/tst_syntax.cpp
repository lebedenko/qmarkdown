#include "private/syntaxhighlighter.h"
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
            request.setSource(QString(256 * 1024 - 10, u' ') + QString::number(i));
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
QTEST_MAIN(SyntaxTest)
#include "tst_syntax.moc"
