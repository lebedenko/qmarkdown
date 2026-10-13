#include "private/document.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtTest/QTest>
using namespace QMarkdownPrivate;

class ParserTest : public QObject
{
    Q_OBJECT
private slots:
    void semanticCodeSpanIdentity()
    {
        const auto content=parseInline(QString::fromUtf8("`f`**`i`** [אבג `🙂`](target)"));
        QCOMPARE(content.text,QString::fromUtf8("fi אבג 🙂"));
        QCOMPARE(content.codes.size(),3);
        QCOMPARE(content.codes[0].start,0); QCOMPARE(content.codes[0].length,1);
        QCOMPARE(content.codes[1].start,1); QCOMPARE(content.codes[1].length,1);
        QCOMPARE(content.codes[2].start,7); QCOMPARE(content.codes[2].length,2);
        QCOMPARE(content.links[0].start,3); QCOMPARE(content.links[0].length,6);
        const auto block=parse("`one` *`two`*")[0]; QCOMPARE(block.codes.size(),2);
    }

    void allExtensionCombinations()
    {
        const QString source="- [x] task\n\n| ~~www.a.b~~ | <script> |\n| :- | -: |\n| `code` | ![image](pic) |\n\ntext <STYLE>";
        for(int mask=0;mask<32;++mask) {
            const auto blocks=parse(source,ParseOptions{bool(mask&1),bool(mask&2),bool(mask&4),bool(mask&8),bool(mask&16)});
            QCOMPARE(blocks[0].children[0].taskState,(mask&1)?TaskState::Checked:TaskState::Ordinary);
            if(mask&8) {
                const auto &cells=blocks[1].children[0].children; QCOMPARE(blocks[1].kind,BlockKind::Table);
                QCOMPARE(!cells[0].ranges.isEmpty(),bool(mask&2)); QCOMPARE(!cells[0].links.isEmpty(),bool(mask&4));
                QCOMPARE(cells[1].inlines[0].literal,"<script>");
                QCOMPARE(cells[1].inlines[0].filteredHtml,(mask&16)?QString("&lt;script>"):QString("<script>"));
                QCOMPARE(blocks[1].children[1].children[1].images[0].destination,"pic");
            } else QVERIFY(blocks[1].kind!=BlockKind::Table);
            const auto &html=blocks.last().inlines.last(); QCOMPARE(html.literal,"<STYLE>");
            QCOMPARE(html.filteredHtml,(mask&16)?QString("&lt;STYLE>"):QString("<STYLE>"));
        }
    }
    void officialTables()
    {
        QFile file(QStringLiteral(QMARKDOWN_TABLE_FIXTURE)); QVERIFY(file.open(QIODevice::ReadOnly));
        const auto examples = QJsonDocument::fromJson(file.readAll()).array(); QCOMPARE(examples.size(), 8);
        for (const auto &value : examples) {
            const auto example = value.toObject(); const auto source = example["markdown"].toString();
            const auto rows = example["rows"].toArray();
            const auto blocks = parse(source, ParseOptions{false, false, false, true});
            QVERIFY2(!blocks.isEmpty(), qPrintable(QString::number(example["example"].toInt())));
            if (rows.isEmpty()) {
                QCOMPARE(blocks[0].kind, BlockKind::Paragraph);
                QCOMPARE(blocks[0].text, "| abc | def | | --- | | bar |");
                continue;
            }
            QCOMPARE(blocks[0].kind, BlockKind::Table); QCOMPARE(blocks[0].children.size(), rows.size());
            const auto alignment = example["alignment"].toArray();
            for (int row = 0; row < rows.size(); ++row) {
                const auto &actual = blocks[0].children[row]; QCOMPARE(actual.kind, BlockKind::TableRow);
                const auto cells = rows[row].toArray(); QCOMPARE(actual.children.size(), cells.size());
                for (int cell = 0; cell < cells.size(); ++cell) {
                    QCOMPARE(actual.children[cell].kind, BlockKind::TableCell);
                    QCOMPARE(actual.children[cell].text, cells[cell].toString());
                    QCOMPARE(actual.children[cell].tableAlignment, alignment[cell].toInt());
                }
            }
            if (example["example"].toInt() == 200) {
                const auto &body = blocks[0].children;
                QCOMPARE(body[1].children[0].ranges[0].flags, int(Code));
                QCOMPARE(body[1].children[0].ranges[0].start, 2);
                QCOMPARE(body[1].children[0].inlines[1].literal, "|");
                QCOMPARE(body[2].children[0].ranges[0].flags, int(Strong));
            }
            const auto following = example["following"].toArray(); QCOMPARE(blocks.size(), following.size() + 1);
            if (!following.isEmpty()) {
                QCOMPARE(blocks[1].kind, following[0].toString() == "Quote" ? BlockKind::Quote : BlockKind::Paragraph);
                QCOMPARE(blocks[1].kind == BlockKind::Quote ? blocks[1].children[0].text : blocks[1].text, "bar");
            }
            QVERIFY(parse(source)[0].kind != BlockKind::Table);
        }
    }
    void tableContainersAndInlineSemantics()
    {
        const auto options = ParseOptions{true, true, true, true, true};
        const auto table = parse("> - | [**link**](host:target) | `a\\|b` |\n>   | :- | -: |\n>   | ~~www.a.b~~ | ![alt](pic.png) |\n", options)[0].children[0].children[0].children[0];
        QCOMPARE(table.kind, BlockKind::Table);
        const auto &header = table.children[0].children;
        QCOMPARE(header[0].links[0].destination, "host:target"); QCOMPARE(header[0].ranges[0].flags, int(Strong));
        QCOMPARE(header[1].text, "a|b"); QCOMPARE(header[1].inlines[0].kind, InlineKind::Code);
        const auto &body = table.children[1].children;
        QCOMPARE(body[0].links[0].destination, "http://www.a.b");
        QCOMPARE(body[0].ranges[0].flags, int(Strikethrough));
        QCOMPARE(body[1].images[0].destination, "pic.png");
        QCOMPARE(body[1].inlines[0].kind, InlineKind::Image);
        const auto preceding = parse("ordinary\n| a |\n| - |\nbody\n", options);
        QCOMPARE(preceding.size(), 2); QCOMPARE(preceding[0].text, "ordinary"); QCOMPARE(preceding[1].kind, BlockKind::Table);
        QCOMPARE(preceding[1].children[1].children[0].text, "body");
        for (const auto &delimiter : {QString("| : |"), QString("| - x |"), QString("| --:x |"), QString("| - | - |")})
            QVERIFY(parse("| a |\n" + delimiter, options)[0].kind != BlockKind::Table);
        QCOMPARE(parse("heading\n---", options)[0].kind, BlockKind::Heading);
        QCOMPARE(parse("| a |\n-", options)[0].kind, BlockKind::Table);
        const auto empty = parse("||\n|-|\n||", options)[0]; QCOMPARE(empty.children[0].children[0].text, "");
        for (const auto &ending : {QString("\n# heading"), QString("\n```\ncode\n```"), QString("\n- item"), QString("\n\ntext")}) {
            const auto blocks = parse("| a |\n| - |" + ending, options);
            QCOMPARE(blocks.size(), 2); QCOMPARE(blocks[0].children.size(), 1);
        }
        const auto escaped = parse("| a | b |\n| - | - |\n| x\\\\|y |", options)[0].children[1].children;
        QCOMPARE(escaped[0].text, "x\\"); QCOMPARE(escaped[1].text, "y");
    }
    void officialTagFilter()
    {
        QFile file(QStringLiteral(QMARKDOWN_TAG_FILTER_FIXTURE)); QVERIFY(file.open(QIODevice::ReadOnly));
        const auto examples = QJsonDocument::fromJson(file.readAll()).array(); QCOMPARE(examples.size(), 1);
        const auto example = examples[0].toObject();
        const auto blocks = parse(example["markdown"].toString(), ParseOptions{false, false, false, false, true});
        QCOMPARE(blocks.size(), 2); int html = 0;
        for (const auto &node : blocks[0].inlines) if (node.kind == InlineKind::Html) {
            QCOMPARE(node.literal, example["raw"].toArray()[html].toString());
            QCOMPARE(node.filteredHtml, example["filtered"].toArray()[html].toString()); ++html;
        }
        QCOMPARE(html, 4); QCOMPARE(blocks[1].text, example["blockRaw"].toString());
        QCOMPARE(blocks[1].filteredHtml, example["blockFiltered"].toString());
        const auto disabled = parse(example["markdown"].toString());
        QCOMPARE(disabled[1].filteredHtml, disabled[1].text);
        QCOMPARE(disabled[0].text, blocks[0].text);
        for (const auto &name : {QString("title"), QString("textarea"), QString("style"), QString("xmp"), QString("iframe"),
                                 QString("noembed"), QString("noframes"), QString("script"), QString("plaintext")}) {
            const auto input = "<" + name.toUpper() + " a='b'></" + name + "><" + name + "Extra>";
            QCOMPARE(filterRawHtml(input), "&lt;" + name.toUpper() + " a='b'>&lt;/" + name + "><" + name + "Extra>");
        }
        QCOMPARE(filterRawHtml("<scripture> <script-x> <script1> < script> <script/foo> <script/>"),
                 "<scripture> <script-x> <script1> < script> <script/foo> &lt;script/>");
        const auto code = parse("`<script>`", ParseOptions{false, false, false, false, true})[0];
        QCOMPARE(code.inlines[0].kind, InlineKind::Code); QCOMPARE(code.inlines[0].literal, "<script>");
    }
    void extendedAutolinks_data()
    {
        QTest::addColumn<QString>("source"); QTest::addColumn<QString>("label"); QTest::addColumn<QString>("destination");
        QTest::newRow("https") << "https://a.b/path" << "https://a.b/path" << "https://a.b/path";
        QTest::newRow("www") << "www.a.b" << "www.a.b" << "http://www.a.b";
        QTest::newRow("www-single-domain") << "www.localhost" << "" << "";
        QTest::newRow("uppercase") << "HTTP://a.b WWW.a.b" << "" << "";
        QTest::newRow("single-domain") << "http://localhost" << "" << "";
        QTest::newRow("empty-segment") << "http://a..b" << "" << "";
        QTest::newRow("underscore-final") << "http://a.b_c" << "" << "";
        QTest::newRow("underscore-penultimate") << "http://a_b.c" << "" << "";
        QTest::newRow("underscore-earlier") << "http://a_b.c.d" << "http://a_b.c.d" << "http://a_b.c.d";
        QTest::newRow("ascii-domain") << QString::fromUtf8("http://é.b") << "" << "";
        QTest::newRow("domain-hyphen") << "http://-a.b-" << "http://-a.b-" << "http://-a.b-";
        QTest::newRow("blocked-boundary") << "xwww.a.b ,http://a.b" << "" << "";
        QTest::newRow("parenthesis") << "(http://a.b/x(y)))" << "http://a.b/x(y)" << "http://a.b/x(y)";
        QTest::newRow("interior-parentheses") << "http://a.b/x))+y" << "http://a.b/x))+y" << "http://a.b/x))+y";
        QTest::newRow("punctuation") << "http://a.b/x?!.,:*_~" << "http://a.b/x" << "http://a.b/x";
        QTest::newRow("entity-tail") << "http://a.b/x&tail;" << "http://a.b/x" << "http://a.b/x";
        QTest::newRow("numeric-tail") << "http://a.b/x&#65;" << "http://a.b/xA" << "http://a.b/xA";
        QTest::newRow("interior-entity") << "http://a.b/x&amp;y" << "http://a.b/x&y" << "http://a.b/x&y";
        QTest::newRow("semicolon") << "http://a.b/x;" << "http://a.b/x;" << "http://a.b/x;";
        QTest::newRow("source-escape") << "http://a.b/x\\*y" << "http://a.b/x\\*y" << "http://a.b/x\\*y";
        QTest::newRow("escaped-prefix") << "\\www.a.b" << "" << "";
        QTest::newRow("entity-prefix") << "&#119;ww.a.b" << "" << "";
        QTest::newRow("decoded-email") << "foo&#64;bar.baz" << "foo@bar.baz" << "mailto:foo@bar.baz";
        QTest::newRow("escaped-email") << "foo\\+bar@a.b" << "foo+bar@a.b" << "mailto:foo+bar@a.b";
        QTest::newRow("email-anywhere") << ":foo@a.b" << "foo@a.b" << "mailto:foo@a.b";
        QTest::newRow("email-underscore") << "foo@a_b.c_d" << "foo@a_b.c_d" << "mailto:foo@a_b.c_d";
        QTest::newRow("email-empty-segment") << "foo@a..b" << "" << "";
        QTest::newRow("email-plus-domain") << "foo@a.b+c" << "" << "";
        QTest::newRow("email-trailing-hyphen") << "foo@a.b-" << "" << "";
        QTest::newRow("email-trailing-underscore") << "foo@a.b_" << "" << "";
        QTest::newRow("email-period") << "foo@a.b." << "foo@a.b" << "mailto:foo@a.b";
        QTest::newRow("mailto") << "mailto:foo@a.b/path" << "mailto:foo@a.b" << "mailto:foo@a.b";
        QTest::newRow("xmpp") << "xmpp:foo@a.b/res@x.y/more" << "xmpp:foo@a.b/res@x.y" << "xmpp:foo@a.b/res@x.y";
        QTest::newRow("xmpp-resource-grammar") << "xmpp:foo@a.b/res-more" << "xmpp:foo@a.b/res" << "xmpp:foo@a.b/res";
        QTest::newRow("unicode-offset") << QString::fromUtf8("😀 é http://a.b") << "http://a.b" << "http://a.b";
        QTest::newRow("unicode-space") << QString::fromUtf8("é\u00a0http://a.b") << "http://a.b" << "http://a.b";
        QTest::newRow("unicode-blocked") << QString::fromUtf8("éhttp://a.b") << "" << "";
        QTest::newRow("soft-break") << "one\nhttp://a.b" << "http://a.b" << "http://a.b";
        QTest::newRow("hard-break") << "one  \nhttp://a.b" << "http://a.b" << "http://a.b";
        QTest::newRow("code") << "`www.a.b foo@a.b`" << "" << "";
        QTest::newRow("html") << "a <span title='www.a.b foo@a.b'>" << "" << "";
    }
    void extendedAutolinks()
    {
        QFETCH(QString, source); QFETCH(QString, label); QFETCH(QString, destination);
        for (const auto &ending : {QString("\n"), QString("\r\n"), QString("\r")}) {
            auto input = source; input.replace("\n", ending);
            const auto block = parse(input, false, false, true)[0];
            QCOMPARE(block.links.size(), label.isEmpty() ? 0 : 1);
            if (!label.isEmpty()) {
                QCOMPARE(block.links[0].destination, destination);
                QCOMPARE(block.links[0].start, block.text.indexOf(label));
                QCOMPARE(block.links[0].length, label.size());
                QCOMPARE(block.text.mid(block.links[0].start, block.links[0].length), label);
            }
            QVERIFY(parse(input)[0].links.isEmpty());
        }
    }
    void extendedAutolinkSemantics()
    {
        for (const auto &source : {QString("[www.a.b](explicit:target)"), QString("[ www.a.b](explicit:target)"),
                                  QString("[**http://a.b** foo@a.b](explicit:target)"), QString("[www.a.b][ref]\n\n[ref]: explicit:target")}) {
            const auto block = parse(source, false, false, true)[0];
            QCOMPARE(block.links.size(), 1); QCOMPARE(block.links[0].destination, "explicit:target");
            const auto &link = block.inlines.last(); QCOMPARE(link.kind, InlineKind::Link);
            for (const auto &child : link.children) QVERIFY(child.kind != InlineKind::Link);
        }
        const auto restored = parse("[http://a.b/x\\*y `code`](explicit:target)", false, false, true)[0];
        QCOMPARE(restored.text, "http://a.b/x*y code"); QCOMPARE(restored.links.size(), 1);
        QCOMPARE(restored.links[0].destination, "explicit:target");
        QCOMPARE(restored.inlines[0].children.last().kind, InlineKind::Code);
        const auto restoredStyle = parse("[http://a.b/*word*](explicit:target)", false, false, true)[0];
        QCOMPARE(restoredStyle.text, "http://a.b/word"); QCOMPARE(restoredStyle.ranges[0].flags, int(Emphasis));
        const auto restoredCode = parse("[http://a.b/`x]y`](explicit:target)", false, false, true)[0];
        QCOMPARE(restoredCode.text, "http://a.b/x]y"); QCOMPARE(restoredCode.links.size(), 1);
        QCOMPARE(restoredCode.links[0].destination, "explicit:target");
        const auto restoredEscape = parse("[http://a.b/x\\]y](explicit:target)", false, false, true)[0];
        QCOMPARE(restoredEscape.text, "http://a.b/x]y"); QCOMPARE(restoredEscape.links.size(), 1);
        QCOMPARE(restoredEscape.links[0].destination, "explicit:target");
        const auto nestedLink = parse("[http://a.b/[inner](inner:target)](outer:target)", false, false, true)[0];
        QCOMPARE(nestedLink.links.size(), 2); QCOMPARE(nestedLink.links[1].destination, "inner:target");
        QVERIFY(nestedLink.text.startsWith("[http://a.b/inner]"));
        const auto restoredImage = parse("[http://a.b/![alt](pic)](explicit:target)", false, false, true)[0];
        QCOMPARE(restoredImage.links.size(), 1); QCOMPARE(restoredImage.links[0].destination, "explicit:target");
        QCOMPARE(restoredImage.images.size(), 1); QCOMPARE(restoredImage.images[0].destination, "pic");
        const auto image = parse("![www.a.b foo@a.b](image.png)", false, false, true)[0];
        QVERIFY(image.links.isEmpty()); QCOMPARE(image.images.size(), 1);
        QCOMPARE(image.inlines[0].kind, InlineKind::Image);
        QCOMPARE(image.inlines[0].children[0].kind, InlineKind::Link);
        QCOMPARE(image.inlines[0].children[0].destination, "http://www.a.b");
        QCOMPARE(image.inlines[0].children.last().destination, "mailto:foo@a.b");
        for (const auto &source : {QString("    www.a.b foo@a.b"), QString("```\nwww.a.b foo@a.b\n```"), QString("<div>\nwww.a.b foo@a.b\n</div>")})
            QVERIFY(parse(source, false, false, true)[0].links.isEmpty());
        for (bool tasks : {false, true}) for (bool strike : {false, true}) for (bool autolinks : {false, true}) {
            const auto item = parse("- [x] ~~**www.a.b**~~", tasks, strike, autolinks)[0].children[0];
            QCOMPARE(item.taskState, tasks ? TaskState::Checked : TaskState::Ordinary);
            const auto &block = item.children[0];
            QCOMPARE(block.text, QString(tasks ? "" : "[x] ") + (strike ? "www.a.b" : "~~www.a.b~~"));
            QCOMPARE(block.links.size(), autolinks ? 1 : 0);
            if (autolinks) { QCOMPARE(block.links[0].destination, "http://www.a.b"); QCOMPARE(block.links[0].length, 7); }
            QCOMPARE(block.ranges[0].flags, int(Strong | (strike ? Strikethrough : 0)));
        }
    }
    void officialExtendedAutolinkExamples()
    {
        QFile file(QStringLiteral(QMARKDOWN_AUTOLINK_FIXTURE)); QVERIFY(file.open(QIODevice::ReadOnly));
        const auto examples = QJsonDocument::fromJson(file.readAll()).array(); QCOMPARE(examples.size(), 14);
        for (int e = 0; e < examples.size(); ++e) {
            const auto example = examples[e].toObject(); QCOMPARE(example["example"].toInt(), 622 + e);
            const auto blocks = parse(example["markdown"].toString(), false, false, true);
            const auto texts = example["texts"].toArray(), links = example["links"].toArray();
            QCOMPARE(blocks.size(), texts.size());
            for (int i = 0; i < blocks.size(); ++i) {
                QCOMPARE(blocks[i].text, texts[i].toString()); const auto expected = links[i].toArray();
                QCOMPARE(blocks[i].links.size(), expected.size());
                int semanticLinks = 0;
                for (const auto &node : blocks[i].inlines) {
                    if (node.kind != InlineKind::Link) continue;
                    const auto link = expected[semanticLinks++].toArray();
                    QCOMPARE(node.destination, link[2].toString());
                    QCOMPARE(node.children.size(), 1); QCOMPARE(node.children[0].kind, InlineKind::Text);
                    QCOMPARE(node.children[0].literal, texts[i].toString().mid(link[0].toInt(), link[1].toInt()));
                }
                QCOMPARE(semanticLinks, expected.size());
                for (int j = 0; j < expected.size(); ++j) {
                    const auto link = expected[j].toArray();
                    QCOMPARE(blocks[i].links[j].start, link[0].toInt()); QCOMPARE(blocks[i].links[j].length, link[1].toInt());
                    QCOMPARE(blocks[i].links[j].destination, link[2].toString());
                }
            }
        }
    }

    void strikethrough_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("text");
        QTest::addColumn<int>("flags");
        QTest::newRow("single") << "~word~" << "word" << int(Strikethrough);
        QTest::newRow("double") << "~~word~~" << "word" << int(Strikethrough);
        QTest::newRow("three") << "a ~~~word~~~" << "a ~~~word~~~" << 0;
        QTest::newRow("four") << "a ~~~~word~~~~" << "a ~~~~word~~~~" << 0;
        QTest::newRow("mismatch") << "~word~~" << "~word~~" << 0;
        QTest::newRow("reverse-mismatch") << "~~word~" << "~~word~" << 0;
        QTest::newRow("unclosed") << "~~word" << "~~word" << 0;
        QTest::newRow("opening-space") << "~~ word~~" << "~~ word~~" << 0;
        QTest::newRow("closing-space") << "~~word ~~" << "~~word ~~" << 0;
        QTest::newRow("tab") << "~\tword~" << "~\tword~" << 0;
        QTest::newRow("unicode-space") << QString::fromUtf8("~\xc2\xa0word~") << QString::fromUtf8("~\xc2\xa0word~") << 0;
        QTest::newRow("intraword") << "a~word~b" << "awordb" << int(Strikethrough);
        QTest::newRow("punctuation") << "(~word~)" << "(word)" << int(Strikethrough);
        QTest::newRow("punctuation-blocked") << "a~!word~" << "a~!word~" << 0;
        QTest::newRow("escaped") << "\\~word\\~" << "~word~" << 0;
        QTest::newRow("entities") << "&#126;word&#x7e;" << "~word~" << 0;
        QTest::newRow("mixed-escape") << "\\~~word~~" << "~~word~~" << 0;
        QTest::newRow("mixed-entity") << "~&#126;word~~" << "~~word~~" << 0;
        QTest::newRow("softbreak") << "~~a\nb~~" << "a b" << int(Strikethrough);
        QTest::newRow("hardbreak") << "~~a  \nb~~" << "a\nb" << int(Strikethrough);
        QTest::newRow("code") << "`~~word~~`" << "~~word~~" << int(Code);
        QTest::newRow("html-attribute") << "a <span title='~~word~~'>" << "a <span title='~~word~~'>" << 0;
        QTest::newRow("strong") << "~~**word**~~" << "word" << int(Strikethrough | Strong);
        QTest::newRow("emphasis-outer") << "*~~word~~*" << "word" << int(Strikethrough | Emphasis);
        QTest::newRow("code-inner") << "~~`word`~~" << "word" << int(Strikethrough | Code);
        QTest::newRow("equal-length-search") << "~a ~~b~ c~~" << "a ~~b c~~" << int(Strikethrough);
    }
    void strikethrough()
    {
        QFETCH(QString, source); QFETCH(QString, text); QFETCH(int, flags);
        for (const auto &ending : {QString("\n"), QString("\r\n"), QString("\r")}) {
            auto input = source; input.replace("\n", ending);
            const auto blocks = parse(input, false, true);
            QCOMPARE(blocks.size(), 1); QCOMPARE(blocks[0].text, text);
            int effective = 0;
            for (const auto &range : blocks[0].ranges) effective |= range.flags;
            QCOMPARE(effective, flags);
        }
    }
    void strikethroughSemantics()
    {
        const auto b = parse(QString::fromUtf8("😀 ~~**é** [~link~](custom:target) ![~~alt~~](pic)~~"), false, true)[0];
        QCOMPARE(b.text, QString::fromUtf8("😀 é link alt"));
        QCOMPARE(b.ranges[0].start, 3); QCOMPARE(b.ranges[0].length, 1);
        QCOMPARE(b.ranges[0].flags, int(Strong | Strikethrough));
        QCOMPARE(b.inlines[1].kind, InlineKind::Strikethrough);
        QCOMPARE(b.inlines[1].children[0].kind, InlineKind::Strong);
        QCOMPARE(b.links[0].start, 5); QCOMPARE(b.links[0].length, 4);
        QCOMPARE(b.links[0].destination, "custom:target");
        QCOMPARE(b.images[0].start, 10); QCOMPARE(b.images[0].length, 3);
        QCOMPARE(b.images[0].destination, "pic");
        QCOMPARE(b.inlines[1].children.last().children[0].kind, InlineKind::Strikethrough);
        const auto precedence = parse("~~[word~~](url)", false, true)[0];
        QCOMPARE(precedence.text, "~~word~~"); QVERIFY(precedence.ranges.isEmpty());
        QCOMPARE(precedence.links[0].destination, "url");
        QCOMPARE(parse("~~[word](url)~~", false, true)[0].ranges[0].flags, int(Strikethrough));
        QCOMPARE(parse("[~~word~~][ref]\n\n[ref]: custom:ref", false, true)[0].links[0].destination, "custom:ref");
        const auto paragraphs = parse("~~one\n\ntwo~~", false, true);
        QCOMPARE(paragraphs.size(), 2); QVERIFY(paragraphs[0].ranges.isEmpty()); QVERIFY(paragraphs[1].ranges.isEmpty());
        for (const auto &source : {QString("    ~~code~~"), QString("```\n~~code~~\n```"), QString("<div>\n~~html~~\n</div>")}) {
            const auto literal = parse(source, false, true)[0];
            QVERIFY(literal.text.contains("~~")); QVERIFY(literal.ranges.isEmpty());
        }
        for (bool tasks : {false, true}) for (bool strike : {false, true}) {
            const auto item = parse("- [x] ~~done~~", tasks, strike)[0].children[0];
            QCOMPARE(item.taskState, tasks ? TaskState::Checked : TaskState::Ordinary);
            QCOMPARE(item.children[0].text, QString(tasks ? "" : "[x] ") + (strike ? "done" : "~~done~~"));
            QCOMPARE(item.children[0].ranges.isEmpty(), !strike);
        }
    }
    void officialStrikethroughExamples()
    {
        QFile file(QStringLiteral(QMARKDOWN_STRIKE_FIXTURE)); QVERIFY(file.open(QIODevice::ReadOnly));
        const auto examples = QJsonDocument::fromJson(file.readAll()).array(); QCOMPARE(examples.size(), 3);
        for (const auto &entry : examples) {
            const auto example = entry.toObject(); const auto blocks = parse(example["markdown"].toString(), false, true);
            const auto texts = example["texts"].toArray(), ranges = example["ranges"].toArray();
            QCOMPARE(blocks.size(), texts.size());
            for (int i = 0; i < blocks.size(); ++i) {
                QCOMPARE(blocks[i].text, texts[i].toString()); const auto expected = ranges[i].toArray();
                QCOMPARE(blocks[i].ranges.size(), expected.size());
                for (int j = 0; j < expected.size(); ++j) {
                    const auto r = expected[j].toArray();
                    QCOMPARE(blocks[i].ranges[j].start, r[0].toInt()); QCOMPARE(blocks[i].ranges[j].length, r[1].toInt());
                    QCOMPARE(blocks[i].ranges[j].flags, r[2].toInt());
                }
            }
        }
    }

    void taskMarkers_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<TaskState>("state");
        QTest::addColumn<QString>("text");
        QTest::newRow("unchecked") << "- [ ] pending" << TaskState::Unchecked << "pending";
        QTest::newRow("checked") << "- [x] finished" << TaskState::Checked << "finished";
        QTest::newRow("uppercase") << "- [X] finished" << TaskState::Checked << "finished";
        QTest::newRow("tab-marker") << "- [\t] pending" << TaskState::Unchecked << "pending";
        QTest::newRow("vertical-tab-marker") << "- [\v] pending" << TaskState::Unchecked << "pending";
        QTest::newRow("form-feed-marker") << "- [\f] pending" << TaskState::Unchecked << "pending";
        QTest::newRow("newline-marker") << "- [\n  ] pending" << TaskState::Unchecked << "pending";
        QTest::newRow("tab-separator") << "- [x]\tfinished" << TaskState::Checked << "finished";
        QTest::newRow("newline-separator") << "- [x]\n  finished" << TaskState::Checked << "finished";
        QTest::newRow("vertical-tab-separator") << "- [x]\vfinished" << TaskState::Checked << "finished";
        QTest::newRow("form-feed-separator") << "- [x]\ffinished" << TaskState::Checked << "finished";
        QTest::newRow("extra-spaces") << "-   [x]   finished" << TaskState::Checked << "finished";
        QTest::newRow("ordered") << "9) [x] finished" << TaskState::Checked << "finished";
        QTest::newRow("empty") << "- [x] " << TaskState::Checked << "";
        QTest::newRow("empty-eof") << "- [ ]" << TaskState::Unchecked << "";
        QTest::newRow("escaped") << "- \\[x] ordinary" << TaskState::Ordinary << "[x] ordinary";
        QTest::newRow("escaped-close") << "- [x\\] ordinary" << TaskState::Ordinary << "[x] ordinary";
        QTest::newRow("entity") << "- &#91;x] ordinary" << TaskState::Ordinary << "[x] ordinary";
        QTest::newRow("entity-state") << "- [&#32;] ordinary" << TaskState::Ordinary << "[ ] ordinary";
        QTest::newRow("entity-separator") << "- [x]&#32;ordinary" << TaskState::Ordinary << "[x] ordinary";
        QTest::newRow("no-separator") << "- [x]ordinary" << TaskState::Ordinary << "[x]ordinary";
        QTest::newRow("invalid-state") << "- [y] ordinary" << TaskState::Ordinary << "[y] ordinary";
        QTest::newRow("long-marker") << "- [xx] ordinary" << TaskState::Ordinary << "[xx] ordinary";
        QTest::newRow("unicode-space-marker") << QString::fromUtf8("- [\u00a0] ordinary") << TaskState::Ordinary << QString::fromUtf8("[\u00a0] ordinary");
        QTest::newRow("unicode-space-separator") << QString::fromUtf8("- [x]\u00a0ordinary") << TaskState::Ordinary << QString::fromUtf8("[x]\u00a0ordinary");
        QTest::newRow("inline-code") << "- `[x]` ordinary" << TaskState::Ordinary << "[x] ordinary";
        QTest::newRow("inline-html") << "- <b>[x]</b> ordinary" << TaskState::Ordinary << "<b>[x]</b> ordinary";
        QTest::newRow("not-prefix") << "- label [x] ordinary" << TaskState::Ordinary << "label [x] ordinary";
    }
    void taskMarkers()
    {
        QFETCH(QString, source); QFETCH(TaskState, state); QFETCH(QString, text);
        for (const QString &ending : {QString("\n"), QString("\r\n"), QString("\r")}) {
            const auto blocks = parse(QString(source).replace("\n", ending), true);
            QCOMPARE(blocks.size(), 1);
            QCOMPARE(blocks[0].kind, BlockKind::List);
            QCOMPARE(blocks[0].children.size(), 1);
            const auto &item = blocks[0].children[0];
            QCOMPARE(item.taskState, state);
            QCOMPARE(item.children.size(), 1);
            QCOMPARE(item.children[0].text, text);
            QCOMPARE(parse(source)[0].children[0].taskState, TaskState::Ordinary);
        }
    }
    void taskContextAndInlineSemantics()
    {
        for (const QString &source : {QString("[x] ordinary"), QString("# [x] ordinary"),
                QString("- # [x] ordinary"), QString("-     [x] ordinary"),
                QString("- ```\n  [x] ordinary\n  ```"), QString("- <div>\n  [x] ordinary\n  </div>")}) {
            const auto blocks = parse(source, true);
            QVERIFY(!blocks.isEmpty());
            QCOMPARE(blocks[0].taskState, TaskState::Ordinary);
            if (blocks[0].kind == BlockKind::List)
                QCOMPARE(blocks[0].children[0].taskState, TaskState::Ordinary);
        }
        const auto loose = parse("- first\n\n  [x] later\n\n- [ ] **bold** [label][ref] ![*alt*](pic)\n\n[ref]: custom:link \"title\"", true);
        QVERIFY(!loose[0].tight);
        QCOMPARE(loose[0].children[0].taskState, TaskState::Ordinary);
        QCOMPARE(loose[0].children[0].children[1].text, "[x] later");
        const auto &task = loose[0].children[1];
        QCOMPARE(task.taskState, TaskState::Unchecked);
        const auto &paragraph = task.children[0];
        QCOMPARE(paragraph.text, "bold label alt");
        QCOMPARE(paragraph.ranges[0].start, 0); QCOMPARE(paragraph.ranges[0].flags, int(Strong));
        QCOMPARE(paragraph.links[0].start, 5); QCOMPARE(paragraph.links[0].destination, "custom:link");
        QCOMPARE(paragraph.images[0].start, 11); QCOMPARE(paragraph.images[0].destination, "pic");
        QCOMPARE(paragraph.inlines[2].title, "title");
        const auto references = parse("- [x] task\n\n[x]: custom:reference", true);
        QCOMPARE(references[0].children[0].taskState, TaskState::Checked);
        QVERIFY(references[0].children[0].children[0].links.isEmpty());
        QCOMPARE(parse("- [x]: custom:reference\n  [x] task", true)[0].children[0].taskState, TaskState::Checked);
    }
    void officialTaskListExamples()
    {
        QFile file(QStringLiteral(QMARKDOWN_GFM_FIXTURE)); QVERIFY(file.open(QIODevice::ReadOnly));
        const auto document = QJsonDocument::fromJson(file.readAll()); QVERIFY(document.isArray());
        for (const auto &value : document.array()) {
            const auto example = value.toObject();
            const auto blocks = parse(example["markdown"].toString(), true);
            QVERIFY2(blocks.size() == 1 && blocks[0].kind == BlockKind::List,
                qPrintable(QString::number(example["example"].toInt())));
            QList<int> states; QStringList labels;
            const auto collect = [&](const auto &self, const QVector<Block> &nodes) -> void {
                for (const auto &block : nodes) {
                    if (block.kind == BlockKind::ListItem) {
                        states.append(static_cast<int>(block.taskState));
                        QVERIFY(!block.children.isEmpty()); labels.append(block.children[0].text);
                    }
                    self(self, block.children);
                }
            };
            collect(collect, blocks);
            QList<int> expectedStates; QStringList expectedLabels;
            for (const auto &state : example["states"].toArray()) expectedStates.append(state.toInt());
            for (const auto &label : example["labels"].toArray()) expectedLabels.append(label.toString());
            QCOMPARE(states, expectedStates); QCOMPARE(labels, expectedLabels);
            QVERIFY(blocks[0].tight);
        }
    }
    void retainedBreaksAndLiteralIdentity() {
        const auto blocks = parse(QString::fromUtf8("😀&#10;é\nnext  \nhard <b>`code`</b><!-- literal -->"));
        const auto &nodes = blocks[0].inlines;
        QCOMPARE(nodes.size(), 9);
        QCOMPARE(nodes[0].kind, InlineKind::Text);
        QCOMPARE(nodes[0].literal, QString::fromUtf8("😀\né"));
        QCOMPARE(nodes[1].kind, InlineKind::SoftBreak);
        QCOMPARE(nodes[3].kind, InlineKind::HardBreak);
        QCOMPARE(nodes[5].kind, InlineKind::Html); QCOMPARE(nodes[5].literal, "<b>");
        QCOMPARE(nodes[6].kind, InlineKind::Code); QCOMPARE(nodes[6].literal, "code");
        QCOMPARE(nodes[7].kind, InlineKind::Html); QCOMPARE(nodes[7].literal, "</b>");
        QCOMPARE(nodes[8].kind, InlineKind::Html); QCOMPARE(nodes[8].literal, "<!-- literal -->");
        QCOMPARE(blocks[0].text, QString::fromUtf8("😀\né next\nhard <b>code</b><!-- literal -->"));
        QCOMPARE(blocks[0].ranges[0].start, 18);
    }
    void retainedNestingTitlesAndEmptyLinks() {
        const auto blocks = parse("# [*_é_*][id][](empty \"caption\") **__strong__**\n\n[id]: a\\*b?x=1&amp;y=2 \"t &amp;\"");
        const auto &nodes = blocks[0].inlines;
        QCOMPARE(blocks[0].kind, BlockKind::Heading);
        QCOMPARE(nodes[0].kind, InlineKind::Link);
        QCOMPARE(nodes[0].destination, "a*b?x=1&y=2"); QCOMPARE(nodes[0].title, "t &");
        QCOMPARE(nodes[0].children[0].kind, InlineKind::Emphasis);
        QCOMPARE(nodes[0].children[0].children[0].kind, InlineKind::Emphasis);
        QCOMPARE(nodes[0].children[0].children[0].children[0].literal, "é");
        QCOMPARE(nodes[1].kind, InlineKind::Link);
        QVERIFY(nodes[1].children.isEmpty()); QCOMPARE(nodes[1].title, "caption");
        QCOMPARE(nodes[3].kind, InlineKind::Strong);
        QCOMPARE(nodes[3].children[0].kind, InlineKind::Strong);
        QCOMPARE(blocks[0].links.size(), 1);
        QCOMPARE(blocks[0].ranges[0].flags, int(Emphasis));
        QCOMPARE(blocks[0].ranges[1].flags, int(Strong));
        const auto trimmed = parse("[x] \nnext\n\n[x]: url")[0].inlines;
        QCOMPARE(trimmed.size(), 3);
        QCOMPARE(trimmed[0].kind, InlineKind::Link);
        QCOMPARE(trimmed[1].kind, InlineKind::SoftBreak);
        QCOMPARE(trimmed[2].literal, "next");
        QVERIFY(parse("#")[0].inlines.isEmpty());
        QVERIFY(parse("    code")[0].inlines.isEmpty());
    }
    void retainedImageDescriptionsAndFixturePrefix() {
        const auto content = parseInline(QString::fromUtf8("![**é** [inner](ignored \"t\") ![*猫*](nested \"n\")](img \"pic\")![](empty)"));
        QCOMPARE(content.nodes.size(), 2);
        const auto &image = content.nodes[0];
        QCOMPARE(image.kind, InlineKind::Image); QCOMPARE(image.title, "pic");
        QCOMPARE(image.children[0].kind, InlineKind::Strong);
        QCOMPARE(image.children[2].kind, InlineKind::Link); QCOMPARE(image.children[2].title, "t");
        QCOMPARE(image.children[4].kind, InlineKind::Image); QCOMPARE(image.children[4].title, "n");
        QCOMPARE(image.children[4].children[0].kind, InlineKind::Emphasis);
        QVERIFY(content.nodes[1].children.isEmpty());
        QVERIFY(content.links.isEmpty()); QCOMPARE(content.images.size(), 2);
        QCOMPARE(content.images[0].start, 0); QCOMPARE(content.images[1].length, 0);
        QCOMPARE(content.text, QString::fromUtf8("é inner 猫"));
        QCOMPARE(parseInline("plain").nodes[0].literal, "plain");
        QVERIFY(parseInline("").nodes.isEmpty());
        QCOMPARE(parseInline("*x*").nodes[0].kind, InlineKind::Emphasis);
    }

    void imageMetadata() {
        const auto content = parseInline(QString::fromUtf8("😀 ![**é** [inner](ignored) ![nested](no)](a\\*b?x=1&amp;y=2 \"title &amp;\")![](empty) [![*linked*](image)]()"));
        QCOMPARE(content.images.size(), 3);
        QCOMPARE(content.images[0].start, 3);
        QCOMPARE(content.images[0].destination, "a*b?x=1&y=2");
        QCOMPARE(content.images[0].title, "title &");
        QCOMPARE(content.text.mid(content.images[0].start, content.images[0].length), "é inner nested");
        QCOMPARE(content.images[1].length, 0);
        QCOMPARE(content.images[1].start, content.images[0].start + content.images[0].length);
        QVERIFY(!content.images[0].linked);
        // Unresolved references still follow cmark's literal projection.
        const auto blocks = parse("> - # [![*linked*][id]](outer)\n\n[id]: image.png \"caption\"");
        const auto &heading = blocks[0].children[0].children[0].children[0];
        QCOMPARE(heading.images.size(), 1); QVERIFY(heading.images[0].linked);
        QCOMPARE(heading.images[0].enclosingLink, "outer"); QCOMPARE(heading.images[0].title, "caption");
        QCOMPARE(heading.text, "linked"); QCOMPARE(heading.ranges[0].flags, int(Emphasis));
    }

    void linkDestinations_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("label");
        QTest::addColumn<QString>("destination");
        QTest::newRow("inline-decoded") << "[x](a\\*b?x=1&amp;y=2)" << "x" << "a*b?x=1&y=2";
        QTest::newRow("reference") << "[label][id]\n\n[id]: ../a#part" << "label" << "../a#part";
        QTest::newRow("collapsed") << "[label][]\n\n[label]: #part" << "label" << "#part";
        QTest::newRow("shortcut") << "[label]\n\n[label]: custom:action" << "label" << "custom:action";
        QTest::newRow("empty") << "[empty]()" << "empty" << "";
        QTest::newRow("uri") << "<https://example.invalid/a>" << "https://example.invalid/a" << "https://example.invalid/a";
        QTest::newRow("email") << "<me@example.invalid>" << "me@example.invalid" << "mailto:me@example.invalid";
        QTest::newRow("formatted") << "[**a** *b* `c`](#x)" << "a b c" << "#x";
    }
    void linkDestinations()
    {
        QFETCH(QString, source); QFETCH(QString, label); QFETCH(QString, destination);
        const auto blocks = parse(source);
        QCOMPARE(blocks.size(), 1); QCOMPARE(blocks[0].text, label);
        QCOMPARE(blocks[0].links.size(), 1);
        QCOMPARE(blocks[0].links[0].start, 0);
        QCOMPARE(blocks[0].links[0].length, label.size());
        QCOMPARE(blocks[0].links[0].destination, destination);
    }
    void linkSpansAndImages()
    {
        const auto content = parseInline(QString::fromUtf8("😀 [é](same)[日本語](same) ![**a** [b](inner)](image) [![*c*](image)](outer)"));
        QCOMPARE(content.text, QString::fromUtf8("😀 é日本語 a b c"));
        QCOMPARE(content.links.size(), 3);
        QCOMPARE(content.links[0].start, 3); QCOMPARE(content.links[0].length, 1);
        QCOMPARE(content.links[1].start, 4); QCOMPARE(content.links[1].length, 3);
        QCOMPARE(content.links[2].destination, "outer");
        QCOMPARE(content.text.mid(content.links[2].start, content.links[2].length), "c");
        QVERIFY(!content.ranges.isEmpty());
        QVERIFY(parseInline("[unresolved][missing] ` [x](code) ` <a href='html'>x</a>").links.isEmpty());
        QVERIFY(parse("    [code](inert)")[0].links.isEmpty());
        const auto quote = parse("> - # [heading](#x)");
        QCOMPARE(quote[0].children[0].children[0].children[0].links[0].destination, "#x");
    }

    void normalization()
    {
        const QString expected = QString::fromUtf8("a  b\tc é 日本語 😀\u00a0");
        for (const QString &ending : {QString("\n"), QString("\r\n"), QString("\r")}) {
            auto blocks = parse("   a  b\tc \t" + ending + QString::fromUtf8(" é 日本語 😀\u00a0 ") + ending + "\t ");
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
        QCOMPARE(blocks[0].text, QString("em code \\x & <b>x</b> link x - list > quote --- ==="));
        QCOMPARE(blocks[0].ranges.size(), 2);
        QCOMPARE(blocks[1].text, QString("title"));
        QCOMPARE(blocks[2].text, QString("body"));
        QCOMPARE(blocks[2].level, 2);
        QCOMPARE(blocks[3].text, QString("end"));
    }
    void setext_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<int>("level");
        QTest::addColumn<QString>("text");
        QTest::newRow("h1-single") << "title\n=" << 1 << "title";
        QTest::newRow("h2-single") << "title\n-" << 2 << "title";
        QTest::newRow("h2-rule-precedence") << "title\n---" << 2 << "title";
        QTest::newRow("indent3-trailing-tabs") << "title\n   === \t" << 1 << "title";
        QTest::newRow("multiline") << "  first \n second\t\n==" << 1 << "first second";
        QTest::newRow("formatted") << "*title*\n**more** `code`\n--" << 2 << "title more code";
        QTest::newRow("indent4") << "title\n    ===" << 0 << "title ===";
        QTest::newRow("tab-indent") << "title\n\t===" << 0 << "title ===";
        QTest::newRow("internal-space") << "title\n= =" << 0 << "title = =";
        QTest::newRow("suffix") << "title\n---x" << 0 << "title ---x";
        QTest::newRow("mixed") << "title\n=-=" << 0 << "title =-=";
        QTest::newRow("no-pending-paragraph") << "===" << 0 << "===";
    }
    void setext()
    {
        QFETCH(QString, source); QFETCH(int, level); QFETCH(QString, text);
        const auto blocks = parse(source);
        QCOMPARE(blocks.size(), 1);
        QCOMPARE(blocks[0].kind, level ? BlockKind::Heading : BlockKind::Paragraph);
        QCOMPARE(blocks[0].level, level);
        QCOMPARE(blocks[0].text, text);
        if (source.startsWith("*title*")) QCOMPARE(blocks[0].ranges.size(), 3);
    }
    void thematicBreaks_data()
    {
        QTest::addColumn<QString>("source");
        for (const auto marker : {u'*', u'-', u'_'}) {
            const QString m(marker);
            QTest::newRow(qPrintable(m + "compact")) << QString(3, marker);
            QTest::newRow(qPrintable(m + "spaced")) << "   " + m + " \t" + m + "  " + m + "\t";
            QTest::newRow(qPrintable(m + "many")) << QString(300, marker);
        }
    }
    void thematicBreaks()
    {
        QFETCH(QString, source);
        for (const QString &prefix : {QString(), QString("paragraph\n")}) {
            const auto blocks = parse(prefix + source);
            const bool setext = !prefix.isEmpty() && source == QString(300, u'-');
            const bool compactSetext = !prefix.isEmpty() && source == "---";
            if (setext || compactSetext) {
                QCOMPARE(blocks.size(), 1);
                QCOMPARE(blocks[0].kind, BlockKind::Heading);
                QCOMPARE(blocks[0].level, 2);
            } else {
                QCOMPARE(blocks.size(), prefix.isEmpty() ? 1 : 2);
                QCOMPARE(blocks.last().kind, BlockKind::ThematicBreak);
                QVERIFY(blocks.last().text.isEmpty());
                QVERIFY(blocks.last().ranges.isEmpty());
            }
        }
        for (const QString &invalid : {QString("**"), QString("--"), QString("__"),
             QString("*-*"), QString("_ _ x _"), QString("***x"), QString("***") + QChar(0xa0)})
            QCOMPARE(parse(invalid)[0].kind, BlockKind::Paragraph);
    }
    void indentedCode_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("text");
        QTest::newRow("four-columns") << "    # hi" << "# hi\n";
        QTest::newRow("tab") << "\t# hi" << "# hi\n";
        QTest::newRow("spaces-tab") << " \tx" << "x\n";
        QTest::newRow("residual-tab") << "\t\tx\t  " << "\tx\t  \n";
        QTest::newRow("residual-spaces") << "      x  \n     y\t" << "  x  \n y\t\n";
        QTest::newRow("internal-blanks") << "    x\n\n \t\n      \n    y" << "x\n\n\n  \ny\n";
        QTest::newRow("exclude-edge-blanks") << "\n     \n    x\n\n      " << "x\n";
        QTest::newRow("eof-no-newline") << "    x" << "x\n";
        QTest::newRow("eof-newline") << "    x\n" << "x\n";
        QTest::newRow("eof-many-blanks") << "    x\n\n\n" << "x\n";
        QTest::newRow("literal") << "    **em** `code` &amp; <img> [x](https://example.invalid) ![x](x)\n    ---\n    ```" << "**em** `code` &amp; <img> [x](https://example.invalid) ![x](x)\n---\n```\n";
        QTest::newRow("unicode-nul") << QString::fromUtf8("    日本語 😀 ") + QChar(0) << QString::fromUtf8("日本語 😀 ") + QChar(0xfffd) + "\n";
    }
    void indentedCode()
    {
        QFETCH(QString, source); QFETCH(QString, text);
        for (const QString &ending : {QString("\n"), QString("\r\n"), QString("\r")}) {
            auto input = source; input.replace("\n", ending);
            const auto blocks = parse(input);
            QCOMPARE(blocks.size(), 1);
            QCOMPARE(blocks[0].kind, BlockKind::CodeBlock);
            QCOMPARE(blocks[0].text, text);
            QVERIFY(blocks[0].ranges.isEmpty());
            QVERIFY(blocks[0].infoString.isEmpty());
        }
    }
    void leafBlockTransitions()
    {
        const auto blocks = parse("title\n===\n---\n    code\n\n   # next\nparagraph\n    continuation\n* * *\n~~~ info\n    literal\n~~~\n\tlast\nend");
        QCOMPARE(blocks.size(), 9);
        QCOMPARE(blocks[0].kind, BlockKind::Heading);
        QCOMPARE(blocks[1].kind, BlockKind::ThematicBreak);
        QCOMPARE(blocks[2].text, QString("code\n"));
        QCOMPARE(blocks[3].level, 1);
        QCOMPARE(blocks[4].kind, BlockKind::Paragraph);
        QCOMPARE(blocks[4].text, QString("paragraph continuation"));
        QCOMPARE(blocks[5].kind, BlockKind::ThematicBreak);
        QCOMPARE(blocks[6].text, QString("    literal\n"));
        QCOMPARE(blocks[6].infoString, QString("info"));
        QCOMPARE(blocks[7].text, QString("last\n"));
        QCOMPARE(blocks[8].text, QString("end"));
        QCOMPARE(parse("   ***")[0].kind, BlockKind::ThematicBreak);
        QCOMPARE(parse("    ***")[0].kind, BlockKind::CodeBlock);
        QCOMPARE(parse("\t___")[0].kind, BlockKind::CodeBlock);
        QCOMPARE(parse("title\n\n---").last().kind, BlockKind::ThematicBreak);
        QCOMPARE(parse("    first\n\ntext\n    continues").last().text, QString("text continues"));
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
    void inertLabels_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("display");
        QTest::addColumn<int>("flags");
        QTest::newRow("link") << "[**label**](https://example.invalid/a?x=1&amp;y=2 'title')" << "label" << int(Strong);
        QTest::newRow("image") << "![*image*](image.png)" << "image" << int(Emphasis);
        QTest::newRow("autolink") << "<https://example.invalid/a>" << "https://example.invalid/a" << 0;
        QTest::newRow("email") << "<user@example.invalid>" << "user@example.invalid" << 0;
        QTest::newRow("html") << "<b data-x='&amp;'>" << "<b data-x='&amp;'>" << 0;
        QTest::newRow("closing-html") << "</b>" << "</b>" << 0;
        QTest::newRow("comment") << "<!-- *comment* -->" << "<!-- *comment* -->" << 0;
        QTest::newRow("space-target") << QString(R"([x](<with space> "title"))") << "x" << 0;
        QTest::newRow("escaped-label") << "[a\\]b](target)" << "a]b" << 0;
    }
    void inertLabels()
    {
        QFETCH(QString, source); QFETCH(QString, display); QFETCH(int, flags);
        const QString prefix = QString::fromUtf8("😀 é 日本語 & * ");
        const auto block = parse(QString::fromUtf8("😀 é 日本語 &amp; ") + "\\* " + source + " *tail*")[0];
        QCOMPARE(block.text, prefix + display + " tail");
        QCOMPARE(block.ranges.size(), flags ? 2 : 1);
        if (flags) {
            QCOMPARE(block.ranges[0].start, prefix.size());
            QCOMPARE(block.ranges[0].length, display.size());
            QCOMPARE(block.ranges[0].flags, flags);
        }
        QCOMPARE(block.ranges.last().start, block.text.size() - 4);
    }
    void containersAndInlineMigration()
    {
        for (const QString &marker : {QString("-"), QString("+"), QString("*")}) {
            const auto b = parse(marker + " a\n" + marker + "\n" + marker + " b");
            QCOMPARE(b[0].kind, BlockKind::List);
            QCOMPARE(b[0].children.size(), 3);
            QVERIFY(b[0].children[1].children.isEmpty());
            QVERIFY(b[0].tight);
        }
        for (const QString &delimiter : {QString("."), QString(")")}) {
            const auto b = parse("9" + delimiter + " a\n10" + delimiter + " b");
            QVERIFY(b[0].ordered);
            QCOMPARE(b[0].start, 9);
            QCOMPARE(b[0].delimiter, delimiter[0]);
        }
        QCOMPARE(parse("- a\n\n- b")[0].tight, false);
        const auto nestedLoose = parse("- outer\n  - a\n\n  - b\n- end")[0];
        QVERIFY(nestedLoose.tight);
        QVERIFY(!nestedLoose.children[0].children[1].tight);
        QCOMPARE(parse("- a\n+ b").size(), 2);
        QCOMPARE(parse("paragraph\n2. stays")[0].text, QString("paragraph 2. stays"));
        QCOMPARE(parse("paragraph\n1. interrupts").size(), 2);
        QCOMPARE(parse("- a\n  ---")[0].children[0].children[0].kind, BlockKind::Heading);
        const auto nested = parse("> - *a*\n>   continuation\n>   - b\n>     > quote\n>\n> # heading\n>\n> ---\n>\n>     code\n>\n> ~~~\n> fence\n> ~~~");
        QCOMPARE(nested[0].kind, BlockKind::Quote);
        const auto children = nested[0].children;
        QCOMPARE(children.size(), 5);
        QCOMPARE(children[0].children[0].children[0].text, QString("a continuation"));
        QCOMPARE(children[0].children[0].children[1].kind, BlockKind::List);
        QCOMPARE(children[1].kind, BlockKind::Heading);
        QCOMPARE(children[2].kind, BlockKind::ThematicBreak);
        QCOMPARE(children[3].text, QString("code\n"));
        QCOMPARE(children[4].text, QString("fence\n"));
        QCOMPARE(parse("- a\n\t- b")[0].children[0].children[1].kind, BlockKind::List);
        QCOMPARE(parse("> lazy\ncontinuation")[0].children[0].text, QString("lazy continuation"));
        QCOMPARE(parse("***")[0].kind, BlockKind::ThematicBreak);
        QCOMPARE(parse(">")[0].children.size(), 0);
        const auto labels = parse("[*label*][id] ![**image**](x) <https://example.invalid>\n\n[id]: /url");
        QCOMPARE(labels.size(), 1);
        QCOMPARE(labels[0].text, QString("label image https://example.invalid"));
        QCOMPARE(labels[0].ranges.size(), 2);
        QCOMPARE(parse("[missing][id]")[0].text, QString("[missing][id]"));
        QCOMPARE(parse("*one*  \ntwo\\\nthree\nfour")[0].text, QString("one\ntwo\nthree four"));
        QCOMPARE(parse("<div>\n*literal* &amp;\n</div>")[0].text, QString("<div>\n*literal* &amp;\n</div>\n"));
        QCOMPARE(parse("a <b>*b*</b>")[0].text, QString("a <b>b</b>"));
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
        QCOMPARE(parse("[x]: /url\n[x]")[0].text, QString("x"));
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
        // A shorter closer remains literal content, even beyond 255 characters.
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
    void longFences_data()
    {
        QTest::addColumn<QString>("source");
        QTest::addColumn<QString>("expected");
        QTest::addColumn<bool>("closed");
        for (const QChar marker : {QChar('`'), QChar('~')}) {
            for (const int length : {254, 255, 256, 300, 4096}) {
                for (const int delta : {-1, 0, 1, 2, 3}) {
                    for (const QString &ending : {QString("\n"), QString("\r\n"), QString("\r")}) {
                        const QString opener(length, marker);
                        const QString closer(length + (delta <= 1 ? delta : 0),
                                             delta == 3 ? (marker == '`' ? '~' : '`') : marker);
                        const bool closed = delta == 0 || delta == 1;
                        QString source = opener + "\nx\n";
                        QString expected = "x\n";
                        if (delta != 2) {
                            source += closer + "\n";
                            if (!closed) expected += closer + "\n";
                        }
                        source += "after\n";
                        if (!closed) expected += "after\n";
                        source.replace("\n", ending);
                        const auto name = QString("%1-%2-%3-%4").arg(marker).arg(length).arg(delta).arg(ending.size() == 2 ? "crlf" : ending == "\r" ? "cr" : "lf").toUtf8();
                        QTest::newRow(name.constData()) << source << expected << closed;
                    }
                }
            }
        }
    }
    void longFences()
    {
        QFETCH(QString, source); QFETCH(QString, expected); QFETCH(bool, closed);
        const auto blocks = parse(source);
        QCOMPARE(blocks.size(), closed ? 2 : 1);
        QCOMPARE(blocks[0].kind, BlockKind::CodeBlock);
        QCOMPARE(blocks[0].text, expected);
        if (closed) QCOMPARE(blocks[1].text, QString("after"));
        for (const QString &prefix : {QString("> "), QString("  ")}) {
            QString nested = source;
            nested.replace("\r\n", "\n"); nested.replace("\r", "\n");
            nested.replace("\n", "\n" + prefix);
            nested.chop(prefix.size());
            nested.prepend(prefix == "> " ? prefix : QString("- "));
            const auto containers = parse(nested);
            QCOMPARE(containers.size(), 1);
            const auto children = prefix == "> " ? containers[0].children : containers[0].children[0].children;
            QCOMPARE(children.size(), closed ? 2 : 1);
            QCOMPARE(children[0].kind, BlockKind::CodeBlock);
            QCOMPARE(children[0].text, expected);
            if (closed) QCOMPARE(children[1].text, QString("after"));
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
        QCOMPARE(blocks[2].kind, BlockKind::CodeBlock);
        blocks = parse("~~~\n# protected\n~~~\n# after\nparagraph");
        QCOMPARE(blocks.size(), 3);
        QCOMPARE(blocks[1].level, 1);
        QCOMPARE(blocks[2].text, QString("paragraph"));
    }
};
QTEST_APPLESS_MAIN(ParserTest)
#include "tst_parser.moc"
