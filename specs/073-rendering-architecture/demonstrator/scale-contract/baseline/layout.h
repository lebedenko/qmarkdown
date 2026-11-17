// Shared validated paragraph construction for CPU checks and native viewer.
#pragma once
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "include/ports/SkFontMgr_empty.h"
#include "modules/skunicode/include/SkUnicode_icu.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include <unicode/ubrk.h>
#include <unicode/ustring.h>
#include <nlohmann/json.hpp>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <set>
using namespace skia::textlayout;
using json=nlohmann::json;
static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> utf;
static void require(bool ok,const char* message) { if(!ok)throw std::runtime_error(message); }
static json rect(SkRect r) { return {r.left(),r.top(),r.right(),r.bottom()}; }
struct Source {
    std::u16string text;
    std::set<int> boundaries;
    std::vector<int> bytes, units, reverse;
    explicit Source(const std::string& input) {
        UErrorCode error=U_ZERO_ERROR;int32_t length=0;
        u_strFromUTF8(nullptr,0,&length,input.data(),input.size(),&error);
        require(error==U_BUFFER_OVERFLOW_ERROR || U_SUCCESS(error),"invalid UTF-8");
        error=U_ZERO_ERROR;text.resize(length);
        u_strFromUTF8(reinterpret_cast<UChar*>(text.data()),length,nullptr,input.data(),input.size(),&error);
        require(U_SUCCESS(error),"invalid UTF-8");
        error=U_ZERO_ERROR;
        auto iterator=ubrk_open(UBRK_CHARACTER,"root",reinterpret_cast<const UChar*>(text.data()),length,&error);
        require(U_SUCCESS(error) && iterator,"grapheme iterator failed");
        for(int i=ubrk_first(iterator);i!=UBRK_DONE;i=ubrk_next(iterator))boundaries.insert(i);
        ubrk_close(iterator);
        bytes.assign(length+1,-1);units.assign(length+1,-1);
    }
    std::pair<int,int> interval(const json& span) const {
        require(span.at("start").is_number_integer() && span.at("length").is_number_integer(),"noninteger interval");
        int64_t a=span.at("start"),n=span.at("length");
        require(a>=0 && n>0 && a<=int64_t(text.size()) && n<=int64_t(text.size())-a,"invalid interval");
        int z=int(a+n);
        require(boundaries.count(int(a)) && boundaries.count(z),"grapheme-splitting interval");
        return {int(a),z};
    }
};
struct PreparedBoundaryParagraph {
    Source source;
    std::vector<std::pair<int,int>> code;
    std::string layoutText;
    std::unique_ptr<Paragraph> paragraph;
};
static PreparedBoundaryParagraph prepareBoundaryParagraph(const json& manifest,const json& block,
                                                          sk_sp<FontCollection> fonts,bool enabled) {
    Source source(block.at("text"));
    std::vector<std::pair<int,int>> code,objects;
    for(auto span:block.at("spans")) {
        auto range=source.interval(span);
        if(span.at("kind")=="code")code.push_back(range);
    }
    for(auto object:block.at("objects"))objects.push_back(source.interval(object));
    auto nonoverlap=[](const auto& ranges) {
        int end=0;for(auto [a,z]:ranges){require(a>=end,"unordered/overlapping intervals");end=z;}
    };
    nonoverlap(code);nonoverlap(objects);
    for(auto [a,z]:code)for(auto [oa,oz]:objects)
        require(z<=oa || a>=oz || (a<=oa && z>=oz),"partial object intersection");
    TextStyle base;base.setFontFamilies({SkString("regular"),SkString("cjk"),SkString("emoji")});
    base.setFontSize(manifest.at("fontSize"));base.setColor(SK_ColorBLACK);
    ParagraphStyle style;style.setTextStyle(base);
    auto builder=ParagraphBuilder::make(style,fonts,SkUnicodes::ICU::Make());
    int byte=0,unit=0;
    for(int pos=0;pos<int(source.text.size());) {
        source.bytes[pos]=byte;source.units[pos]=unit;
        bool objectFound=false;
        for(auto object:block.at("objects"))if(object.at("start")==pos) {
            float ascent=object.at("ascent"),descent=object.at("descent");
            builder->addPlaceholder({object.at("width"),ascent+descent,PlaceholderAlignment::kBaseline,TextBaseline::kAlphabetic,ascent});
            source.reverse.push_back(pos);pos+=object.at("length").get<int>();byte+=3;++unit;objectFound=true;break;
        }
        if(objectFound)continue;
        TextStyle current=base;
        for(auto span:block.at("spans")) {
            auto [a,z]=source.interval(span);
            if(pos<a || pos>=z)continue;
            std::string kind=span.at("kind");
            if(kind=="code")current.setFontFamilies({SkString("mono"),SkString("cjk"),SkString("emoji")});
            if(kind=="bold")current.setFontFamilies({SkString("bold"),SkString("cjk"),SkString("emoji")});
            if(kind=="italic")current.setFontFamilies({SkString("italic"),SkString("cjk"),SkString("emoji")});
            if(kind=="strike")current.setDecoration(TextDecoration::kLineThrough);
        }
        int count=source.text[pos]>=0xd800 && source.text[pos]<=0xdbff?2:1;
        auto text=source.text.substr(pos,count);
        builder->pushStyle(current);builder->addText(text);builder->pop();
        for(int j=0;j<count;++j)source.reverse.push_back(pos+j);
        byte+=utf.to_bytes(text).size();unit+=count;pos+=count;
    }
    source.bytes.back()=byte;source.units.back()=unit;source.reverse.push_back(source.text.size());
    require(builder->getText().size()==size_t(byte),"UTF-8 mapping mismatch");
    auto layoutText=std::string(builder->getText().data(),builder->getText().size());
    auto paragraph=builder->Build();
    auto impl=static_cast<ParagraphImpl*>(paragraph.get());
#ifdef FRAGMENT_PATCHED
    std::vector<ParagraphImpl::ExperimentalFragmentSpan> spans;
    if(enabled) {
        for(size_t i=0;i<code.size();++i) {
            require(source.bytes[code[i].first]>=0 && source.bytes[code[i].second]>=0,"unmapped span edge");
            spans.push_back({i,{size_t(source.bytes[code[i].first]),size_t(source.bytes[code[i].second])},manifest.at("padding")[0],manifest.at("padding")[1]});
        }
        impl->experimentalSetSpans(spans);
    }
#endif
    return {std::move(source),std::move(code),std::move(layoutText),std::move(paragraph)};
}

#ifdef FRAGMENT_PATCHED
static void paintBoundaryBackgrounds(Paragraph* paragraph,SkCanvas* canvas) {
    auto impl=static_cast<ParagraphImpl*>(paragraph);
    SkPaint paint;paint.setAntiAlias(true);paint.setColor(0xffeeeeee);
    for(auto& line:impl->lines()) {
        auto slices=impl->experimentalVisualSlices(line.trimmedText());
        size_t identity=EMPTY_INDEX;
        SkRect bounds=SkRect::MakeEmpty();
        auto flush=[&]() {
            if(identity==EMPTY_INDEX || bounds.isEmpty())return;
            for(auto span:impl->experimentalSpans())if(span.identity==identity) {
                auto padded=bounds.makeOutset(span.horizontalPadding,span.verticalPadding);
                padded.offset(line.offset());
                canvas->drawRoundRect(padded,4,4,paint);
            }
        };
        line.iterateThroughVisualRuns(false,[&](const Run* run,SkScalar offset,TextRange range,SkScalar* width) {
            auto context=line.measureTextInsideOneRun(range,run,offset,0,false,TextLine::GlyphCluster);
            *width=context.clip.width();
            auto slice=std::find_if(slices.begin(),slices.end(),[&](auto s){return s.runIndex==run->index();});
            auto next=slice==slices.end()?EMPTY_INDEX:slice->identity;
            if(next!=identity) {flush();identity=next;bounds=SkRect::MakeEmpty();}
            if(identity!=EMPTY_INDEX)bounds.join(context.clip);
            return true;
        });
        flush();
    }
}
#endif
