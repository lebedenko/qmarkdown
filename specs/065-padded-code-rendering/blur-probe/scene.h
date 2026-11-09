#pragma once
#include "private/formattedtext.h"
#include "private/inline.h"
struct Fixture { QString id, text; qreal width = 740; int kind = 0, alignment = Qt::AlignLeft; bool clip = false; };
static QList<Fixture> fixtures() {
    QList<Fixture> result;
    for (QString id : {"plain", "bold", "italic", "link", "code", "combined"})
        result.append({id, "Try strong, emphasis, inline code, host links, and denied image fallback. <b>HTML stays literal</b>."});
    for (bool clip : {false,true}) {
        const QString suffix = clip ? "-clipped" : "";
        result.append({"mixed-decorated"+suffix, QString::fromUtf8("office é العربية שלום code text"),400,1,Qt::AlignLeft,clip});
        result.append({"wrapped-adjacent-linked"+suffix,"adjacentcode office linked strikeout wraps",120,1,Qt::AlignHCenter,clip});
        result.append({"tiny-right"+suffix,"fX code",1,1,Qt::AlignRight,clip});
    }
    return result;
}
static void configure(FormattedText &text, const Fixture &f) {
    text.setText(f.text); text.setLayoutWidth(f.width); text.setHorizontalAlignment(f.alignment);
    QFont font("DejaVu Sans"); font.setPixelSize(18); text.setFont(font);
    text.setCodeFont(QFont("DejaVu Sans Mono")); text.setCodeBackground(QColor("#eeeeee"));
    text.setSelectionColor(QColor("#8099bbff")); text.setFocusColor(Qt::red);
    QVariantList ranges;
    auto span = [&](QString word,int flags) { return QVariantMap{{"start",f.text.indexOf(word)},{"length",word.size()},{"flags",flags}}; };
    if (f.kind) {
        text.setCodeCornerRadius(4);
        ranges = {QVariantMap{{"start",0},{"length",6},{"flags",QMarkdownPrivate::Code|QMarkdownPrivate::Strikethrough}},
                  QVariantMap{{"start",6},{"length",4},{"flags",QMarkdownPrivate::Code|QMarkdownPrivate::Strikethrough}}};
    } else {
        if (f.id=="bold" || f.id=="combined") ranges.append(span("strong",QMarkdownPrivate::Strong));
        if (f.id=="italic" || f.id=="combined") ranges.append(span("emphasis",QMarkdownPrivate::Emphasis));
        if (f.id=="code" || f.id=="combined") ranges.append(span("inline code",QMarkdownPrivate::Code));
    }
    text.setFormatRanges(ranges);
    if (f.kind || f.id=="link" || f.id=="combined")
        text.setLinkSpans({QVariantMap{{"start",f.kind?6:f.text.indexOf("host links")},{"length",f.kind?4:10},{"logicalId",0},{"href","#test"}}});
}

// Review initializes every paragraph at the same origin before assigning its row.
static FormattedText *createReviewParagraph(QQuickItem *content, const Fixture &f) {
    auto *clip=new QQuickItem(content);
    clip->setPosition({24.5,40}); clip->setSize({560,100}); clip->setClip(f.clip);
    auto *text=new FormattedText(clip); configure(*text,f); return text;
}
