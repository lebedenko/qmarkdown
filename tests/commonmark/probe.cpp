#include "private/document.h"
#include "../../third_party/cmark/symbols.h"
#include <cmark.h>
#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>
#include <cstdio>
#include <memory>

using namespace QMarkdownPrivate;

namespace {
QJsonArray serialize(const QVector<Block> &blocks)
{
    QJsonArray result;
    for (const auto &block : blocks) {
        QJsonObject value;
        switch (block.kind) {
        case BlockKind::Paragraph:
        case BlockKind::Heading: {
            value["kind"] = block.kind == BlockKind::Heading ? "Heading" : "Paragraph";
            value["text"] = block.text;
            if (block.kind == BlockKind::Heading) value["level"] = block.level;
            QJsonArray ranges, links, images;
            for (const auto &range : block.ranges)
                ranges.append(QJsonObject{{"start", range.start}, {"length", range.length}, {"flags", range.flags}});
            for (const auto &link : block.links)
                links.append(QJsonObject{{"start", link.start}, {"length", link.length}, {"destination", link.destination}});
            for (const auto &image : block.images)
                images.append(QJsonObject{{"start", image.start}, {"length", image.length},
                    {"destination", image.destination}, {"title", image.title},
                    {"enclosingLink", image.enclosingLink}, {"linked", image.linked}});
            value["ranges"] = ranges;
            value["links"] = links;
            value["images"] = images;
            break;
        }
        case BlockKind::CodeBlock:
            value = {{"kind", "CodeBlock"}, {"text", block.text}, {"infoString", block.infoString}};
            break;
        case BlockKind::HtmlBlock:
            value = {{"kind", "HtmlBlock"}, {"text", block.text}};
            break;
        case BlockKind::ThematicBreak:
            value["kind"] = "ThematicBreak";
            break;
        case BlockKind::List:
            value = {{"kind", "List"}, {"ordered", block.ordered}, {"start", block.start},
                {"delimiter", QString(block.delimiter)}, {"tight", block.tight}, {"children", serialize(block.children)}};
            break;
        case BlockKind::ListItem:
        case BlockKind::Quote:
            value = {{"kind", block.kind == BlockKind::Quote ? "Quote" : "ListItem"},
                {"children", serialize(block.children)}};
            break;
        default:
            qFatal("Unexpected resource/rendering block in parser output");
        }
        result.append(value);
    }
    return result;
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QFile input, output;
    if (!input.open(stdin, QIODevice::ReadOnly) || !output.open(stdout, QIODevice::WriteOnly)) return 2;
    QJsonParseError error;
    const auto request = QJsonDocument::fromJson(input.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !request.isObject()
        || request.object().keys() != QStringList{"examples"} || !request["examples"].isArray()) {
        std::fprintf(stderr, "Invalid probe request\n");
        return 2;
    }
    QJsonArray results;
    QSet<int> seen;
    for (const auto entry : request["examples"].toArray()) {
        const auto example = entry.toObject();
        const auto id = example["example"];
        if (!entry.isObject() || example.keys() != QStringList{"example", "markdown"}
            || !id.isDouble() || id.toDouble() != id.toInt() || id.toInt() <= 0
            || seen.contains(id.toInt()) || !example["markdown"].isString()) {
            std::fprintf(stderr, "Invalid or duplicate probe example\n");
            return 2;
        }
        seen.insert(id.toInt());
        const auto source = example["markdown"].toString();
        const auto bytes = source.toUtf8();
        std::unique_ptr<cmark_node, decltype(&cmark_node_free)> tree(
            cmark_parse_document(bytes.constData(), bytes.size(), CMARK_OPT_DEFAULT), cmark_node_free);
        if (!tree) return 2;
        // Only this test serializer includes raw HTML. Production never renders HTML.
        char *html = cmark_render_html(tree.get(), CMARK_OPT_UNSAFE);
        if (!html) return 2;
        const QString rendered = QString::fromUtf8(html);
        cmark_get_default_mem_allocator()->free(html);
        results.append(QJsonObject{{"example", id}, {"html", rendered}, {"model", serialize(parse(source))}});
    }
    const QJsonDocument response(QJsonObject{{"schema", 1}, {"qt", qVersion()},
        {"cmark", cmark_version_string()}, {"parseOptions", "CMARK_OPT_DEFAULT"},
        {"htmlOptions", "CMARK_OPT_UNSAFE (test serializer only)"}, {"examples", results}});
    const auto bytes = response.toJson(QJsonDocument::Compact);
    return output.write(bytes) == bytes.size() ? 0 : 2;
}
