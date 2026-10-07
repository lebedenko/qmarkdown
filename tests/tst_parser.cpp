#include "private/document.h"
#include "private/inlineadapter.h"
#include "../third_party/cmark/symbols.h"
#include <cmark.h>
#include <memory>
#include <QtTest/QTest>
using namespace QMarkdownPrivate;

class ParserTest : public QObject
{
    Q_OBJECT
private slots:
    void normalization()
    {
        const QString expected = QString::fromUtf8("a  b\tc é 日本語 😀\u00a0");
        for (const QString &ending : {QString("\n"), QString("\r\n"), QString("\r")}) {
            auto blocks = parse(" \ta  b\tc \t" + ending + QString::fromUtf8(" é 日本語 😀\u00a0 ") + ending + "\t ");
            QCOMPARE(blocks.size(), 1);
            QCOMPARE(blocks[0].text, expected);
        }
        QCOMPARE(parse(QString(QChar(0)))[0].text, QString(QChar(0xfffd)));
        QCOMPARE(parse(QString(QChar(0xa0)))[0].text, QString(QChar(0xa0)));
        QVERIFY(parse(" \t\r\n\r\t ").isEmpty());
    }
    void headings_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<int>("level");
        QTest::addColumn<QString>("text");
        for (int i = 1; i <= 6; ++i)
            QTest::newRow(qPrintable(QString::number(i))) << QString(i, '#') + " title" << i << "title";
        QTest::newRow("indent3") << "   # hi\t### \t" << 1 << "hi";
        QTest::newRow("indent4") << "    # hi" << 0 << "# hi";
        QTest::newRow("tab-indent") << "\t# hi" << 0 << "# hi";
        QTest::newRow("no-separator") << "#title" << 0 << "#title";
        QTest::newRow("seven") << "####### title" << 0 << "####### title";
        QTest::newRow("empty") << "##" << 2 << "";
        QTest::newRow("empty-closing") << "## ###" << 2 << "";
        QTest::newRow("embedded") << "# hi###" << 1 << "hi###";
        QTest::newRow("suffix") << "# hi ### x" << 1 << "hi ### x";
        QTest::newRow("backslash") << "# hi \\###" << 1 << "hi ###";
        QTest::newRow("tab-separator") << "#\thi" << 1 << "hi";
        QTest::newRow("nbsp") << QString("#") + QChar(0xa0) + "hi" << 0 << QString("#") + QChar(0xa0) + "hi";
    }
    void headings()
    {
        QFETCH(QString, source); QFETCH(int, level); QFETCH(QString, text);
        const auto blocks = parse(source);
        QCOMPARE(blocks.size(), 1);
        QCOMPARE(blocks[0].level, level);
        QCOMPARE(blocks[0].text, text);
        QCOMPARE(blocks[0].kind, level ? BlockKind::Heading : BlockKind::Paragraph);
    }
    void literalAndOrder()
    {
        const QString literal = "*em* `code` \\x &amp; <b>x</b> [link](https://example.com) ![x](x) - list > quote --- ===";
        auto blocks = parse(literal + "\n# title\nbody\n---\n\nend");
        QCOMPARE(blocks.size(), 4);
        QCOMPARE(blocks[0].text, QString("em code \\x & <b>x</b> [link](https://example.com) ![x](x) - list > quote --- ==="));
        QCOMPARE(blocks[0].ranges.size(), 2);
        QCOMPARE(blocks[1].text, QString("title"));
        QCOMPARE(blocks[2].text, QString("body ---"));
        QCOMPARE(blocks[3].text, QString("end"));
    }
    void inlineSyntax_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("display");
        QTest::addColumn<int>("flags");
        QTest::newRow("em-star") << "*word*" << "word" << int(Emphasis);
        QTest::newRow("em-underscore") << "_word_" << "word" << int(Emphasis);
        QTest::newRow("strong") << "__word__" << "word" << int(Strong);
        QTest::newRow("nested") << "***word***" << "word" << int(Emphasis | Strong);
        QTest::newRow("nested-code") << "***`word`***" << "word" << int(Emphasis | Strong | Code);
        QTest::newRow("underscore-boundary") << "some_word_here" << "some_word_here" << 0;
        QTest::newRow("unmatched") << "**word" << "**word" << 0;
        QTest::newRow("different-runs") << "``word`" << "``word`" << 0;
        QTest::newRow("matching-runs") << "`` `word` ``" << "`word`" << int(Code);
        QTest::newRow("code-trim") << "`  word  `" << " word " << int(Code);
        QTest::newRow("code-spaces") << "`   `" << "   " << int(Code);
        QTest::newRow("code-tab") << "`a\tb`" << "a\tb" << int(Code);
        QTest::newRow("code-no-decode") << "`&amp; \\*`" << "&amp; \\*" << int(Code);
        QTest::newRow("escaped-delimiters") << "\\*word\\*" << "*word*" << 0;
        QTest::newRow("non-escape") << "\\x" << "\\x" << 0;
        QTest::newRow("entities") << "&amp; &#65; &#x1F600; &NotEqualTilde;" << QString::fromUtf8("& A 😀 ≂̸") << 0;
        QTest::newRow("invalid-entities") << "&bogus; &amp &#xZZ; &#999999999;" << "&bogus; &amp &#xZZ; &#999999999;" << 0;
        QTest::newRow("null-entity") << "&#0;" << QString(QChar(0xfffd)) << 0;
        QTest::newRow("block-looking") << "# word" << "# word" << 0;
        QTest::newRow("list-looking") << "- *word*" << "- word" << int(Emphasis);
        QTest::newRow("setext-looking") << "---" << "---" << 0;
        QTest::newRow("definition-looking") << "[id]: /url" << "[id]: /url" << 0;
        QTest::newRow("reference") << "[label][missing]" << "[label][missing]" << 0;
        QTest::newRow("reference-emphasis") << "[*label*][missing]" << "[label][missing]" << int(Emphasis);
    }
    void inlineSyntax()
    {
        QFETCH(QString, source); QFETCH(QString, display); QFETCH(int, flags);
        const auto content = parseInline(source);
        QCOMPARE(content.text, display);
        QCOMPARE(content.ranges.isEmpty(), flags == 0);
        if (flags) QCOMPARE(content.ranges[0].flags, flags);
        int previousEnd = 0;
        for (const auto &range : content.ranges) {
            QVERIFY(range.start >= previousEnd);
            QVERIFY(range.length > 0);
            QVERIFY(range.start + range.length <= content.text.size());
            previousEnd = range.start + range.length;
        }
    }
    void mixedRanges()
    {
        const auto content = parseInline("*a **b** c* **d** `e`");
        QCOMPARE(content.text, QString("a b c d e"));
        QCOMPARE(content.ranges.size(), 5);
        const int starts[] = {0, 2, 3, 6, 8};
        const int lengths[] = {2, 1, 2, 1, 1};
        const int flags[] = {Emphasis, Emphasis | Strong, Emphasis, Strong, Code};
        for (int i = 0; i < 5; ++i) {
            QCOMPARE(content.ranges[i].start, starts[i]);
            QCOMPARE(content.ranges[i].length, lengths[i]);
            QCOMPARE(content.ranges[i].flags, flags[i]);
        }
        const auto adjacent = parseInline("*a*_b_");
        QCOMPARE(adjacent.text, QString("ab"));
        QCOMPARE(adjacent.ranges.size(), 1);
        QCOMPARE(adjacent.ranges[0].length, 2);
    }
    void preservedSource_data()
    {
        QTest::addColumn<QString>("construct");
        for (const QString &source : {QString("[**label**](https://example.invalid/a?x=1&amp;y=2 'title')"),
             QString("![*image*](image.png)"), QString("<https://example.invalid/a>"),
             QString("<user@example.invalid>"), QString("<b data-x='&amp;'>"),
             QString("</b>"), QString("<!-- *comment* -->"),
             QString(R"([x](<with space> "title"))"), QString("[a\\]b](target)")})
            QTest::newRow(qPrintable(source)) << source;
    }
    void preservedSource()
    {
        QFETCH(QString, construct);
        const QString prefix = QString::fromUtf8("😀 é 日本語 ") + "&amp; \\* ";
        const auto content = parseInline(prefix + construct + " *tail*");
        QCOMPARE(content.text, QString::fromUtf8("😀 é 日本語 & * ") + construct + " tail");
        QCOMPARE(content.ranges.size(), 1);
        QCOMPARE(content.ranges[0].start, int(content.text.size() - 4));
        const auto wrapped = parseInline("**" + construct + "**");
        QCOMPARE(wrapped.text, construct);
        QCOMPARE(wrapped.ranges.size(), 1);
        QCOMPARE(wrapped.ranges[0].flags, int(Strong));
    }
    void conservativeFallback()
    {
        // The adapter's contract is one normalized line; additional blocks or
        // unexpected softbreak nodes reject the entire result, including formats.
        for (const QString &source : {QString("*one*\n\n# two"), QString("*one*\ntwo")}) {
            const auto content = parseInline(source);
            QCOMPARE(content.text, source);
            QVERIFY(content.ranges.isEmpty());
        }
        QByteArray span;
        const QByteArray source = QString::fromUtf8("abc 😀 xyz").toUtf8();
        QVERIFY(sourceSpan(source, 1, 5, 1, 8, 0, &span));
        QCOMPARE(QString::fromUtf8(span), QString::fromUtf8("😀"));
        QVERIFY(!sourceSpan(source, 1, 6, 1, 8, 0, &span));
        QVERIFY(!sourceSpan(source, 1, 5, 1, 7, 0, &span));
        QVERIFY(!sourceSpan(source, 2, 5, 2, 8, 0, &span));
        QVERIFY(!sourceSpan(source, 1, 5, 1, 99, 0, &span));
        QVERIFY(!sourceSpan(source, 1, 0, 1, 8, 0, &span));
        QVERIFY(!sourceSpan(source, 1, 5, 1, 4, 0, &span));
        QVERIFY(!sourceSpan(source, 1, 5, 1, 8, 9, &span));
        QVERIFY(!sourceSpan(QByteArray("\xff"), 1, 1, 1, 1, 0, &span));
        QVERIFY(!sourceSpan(QByteArray("\xc2"), 1, 1, 1, 1, 0, &span));
    }
    void invalidAdapterTree()
    {
        const QString original = "*good* [label](target)";
        const QByteArray input = "QMarkdownInline " + original.toUtf8();
        using Owner = std::unique_ptr<cmark_node, decltype(&cmark_node_free)>;
        Owner document(cmark_parse_document(input.constData(), input.size(), CMARK_OPT_DEFAULT), &cmark_node_free);
        auto *paragraph = cmark_node_first_child(document.get());
        QVERIFY(paragraph);
        // A public-created node has no valid source span. It follows valid
        // formatted content, so the whole-block fallback must discard that too.
        auto *invalid = cmark_node_new(CMARK_NODE_HTML_INLINE);
        QVERIFY(cmark_node_set_literal(invalid, "<b>"));
        QVERIFY(cmark_node_append_child(paragraph, invalid));
        auto result = adaptInlineDocument(original, document.get());
        QCOMPARE(result.text, original);
        QVERIFY(result.ranges.isEmpty());
        cmark_node_free(invalid);
        auto *unexpected = cmark_node_new(CMARK_NODE_SOFTBREAK);
        QVERIFY(cmark_node_append_child(paragraph, unexpected));
        result = adaptInlineDocument(original, document.get());
        QCOMPARE(result.text, original);
        QVERIFY(result.ranges.isEmpty());
        cmark_node_free(unexpected);
        auto *extra = cmark_node_new(CMARK_NODE_PARAGRAPH);
        QVERIFY(cmark_node_append_child(document.get(), extra));
        result = adaptInlineDocument(original, document.get());
        QCOMPARE(result.text, original);
        QVERIFY(result.ranges.isEmpty());
        cmark_node_free(extra);
        QVERIFY(cmark_node_set_literal(cmark_node_first_child(paragraph), "wrong sentinel"));
        result = adaptInlineDocument(original, document.get());
        QCOMPARE(result.text, original);
        QVERIFY(result.ranges.isEmpty());
    }
    void headingAndFenceFormatting()
    {
        const auto blocks = parse("# *title* `code`\n\n*body*\n```\n*literal* &amp;\n```\n# next");
        QCOMPARE(blocks.size(), 4);
        QCOMPARE(blocks[0].text, QString("title code"));
        QCOMPARE(blocks[0].level, 1);
        QCOMPARE(blocks[0].ranges.size(), 2);
        QCOMPARE(blocks[1].text, QString("body"));
        QCOMPARE(blocks[2].text, QString("*literal* &amp;\n"));
        QVERIFY(blocks[2].ranges.isEmpty());
        QCOMPARE(blocks[3].text, QString("next"));
        QCOMPARE(parse("[x]: /url\n[x]")[0].text, QString("[x]: /url [x]"));
    }
    void fences_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("expected");
        QTest::newRow("backticks") << "``` info\n# protected\n\n```\n" << "# protected\n\n";
        QTest::newRow("matching") << "  ~~~~ info `\n # protected\n~~~\n````\n~~~~ suffix\n ~~~~~\n" << "# protected\n~~~\n````\n~~~~ suffix\n";
        QTest::newRow("unclosed") << "```\n# unclosed\n\n" << "# unclosed\n\n";
        QTest::newRow("empty") << "~~~\n~~~" << "";
        QTest::newRow("opener-only") << "```" << "";
        QTest::newRow("opener-newline") << "```\n" << "";
        QTest::newRow("blank") << "```\n\n```" << "\n";
        QTest::newRow("tabs") << "```\n\ttext\n\t```\n```\n" << "\ttext\n\t```\n";
        QTest::newRow("partial-tab") << "   ```\n\tx\n \tx\n  x\n    x\n```" << " x\n x\nx\n x\n";
        QTest::newRow("residual-tabs") << "  ~~~\n\t\tx  \n~~~" << "  \tx  \n";
        QTest::newRow("final-no-newline") << "```\nx" << "x\n";
        QTest::newRow("final-newline") << "```\nx\n" << "x\n";
        QTest::newRow("literal") << "```\n*em* `code` &amp; <b> 日本語 😀\n```" << "*em* `code` &amp; <b> 日本語 😀\n";
        QTest::newRow("long") << QString(300, '`') + "\nx\n" + QString(299, '`') + "\n" + QString(301, '`') << "x\n" + QString(299, '`') + "\n";
        QTest::newRow("suffix-tab") << "~~~\nx\n   ~~~~ \t" << "x\n";
        QTest::newRow("closer-indent4") << "~~~\n    ~~~\n~~~" << "    ~~~\n";
    }
    void fences()
    {
        QFETCH(QString, source); QFETCH(QString, expected);
        for (const QString &ending : {QString("\n"), QString("\r\n"), QString("\r")}) {
            auto normalized = source;
            normalized.replace("\n", ending);
            const auto blocks = parse("before\n" + normalized);
            QCOMPARE(blocks.size(), 2);
            QCOMPARE(blocks[1].kind, BlockKind::CodeBlock);
            QCOMPARE(blocks[1].text, expected);
            QVERIFY(blocks[1].ranges.isEmpty());
        }
    }
    void fenceInfo()
    {
        QCOMPARE(parse("```  lang extra &amp; \\* &#65;  \n``` ")[0].infoString, QString("lang extra & * A"));
        QCOMPARE(parse("~~~ `lang` &bogus;\n~~~")[0].infoString, QString("`lang` &bogus;"));
        QCOMPARE(parse("~~~ ~~~\n~~~")[0].infoString, QString("~~~"));
        QCOMPARE(parse("```\n" + QString(QChar(0)))[0].text, QString(QChar(0xfffd)) + "\n");
    }
    void invalidFencesAndResume()
    {
        auto blocks = parse("``` bad`info\n# heading\n    ~~~\n# next");
        QCOMPARE(blocks.size(), 4);
        QCOMPARE(blocks[0].kind, BlockKind::Paragraph);
        QCOMPARE(blocks[2].kind, BlockKind::Paragraph);
        blocks = parse("~~~\n# protected\n~~~\n# after\nparagraph");
        QCOMPARE(blocks.size(), 3);
        QCOMPARE(blocks[1].level, 1);
        QCOMPARE(blocks[2].text, QString("paragraph"));
    }
};
QTEST_APPLESS_MAIN(ParserTest)
#include "tst_parser.moc"
