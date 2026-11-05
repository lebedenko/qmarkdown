// CPU feasibility preflight. No padded-layout success is claimed by this program.
#include "modules/skparagraph/src/ParagraphImpl.h"
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
static json inspect(const json& manifest,const json& fixture,const json& block,
                    sk_sp<FontCollection> fonts,bool enabled,const std::filesystem::path& images,int blockIndex) {
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
    auto paragraph=builder->Build();paragraph->layout(fixture.at("width"));
    auto impl=static_cast<ParagraphImpl*>(paragraph.get());
    json result={{"fixture",fixture.at("id")},{"block",blockIndex},{"source",block.at("text")},
        {"sourceToBytes",source.bytes},{"sourceToUnits",source.units},{"layoutToSource",source.reverse},
        {"height",paragraph->getHeight()},{"unresolvedGlyphs",paragraph->unresolvedGlyphs()},
        {"runs",json::array()},{"clusters",json::array()},{"lines",json::array()},
        {"ranges",json::array()},{"objects",json::array()},{"edges",json::array()}};
    for(auto& run:impl->runs()) {
        SkString family;if(run.font().getTypeface())run.font().getTypeface()->getFamilyName(&family);
        json glyphs=json::array(),positions=json::array();
        for(auto glyph:run.glyphs())glyphs.push_back(glyph);
        for(auto p:run.positions())positions.push_back({p.x(),p.y()});
        result["runs"].push_back({{"range",{run.textRange().start,run.textRange().end}},
            {"fontFamily",family.c_str()},{"fontSize",run.font().getSize()},
            {"glyphs",glyphs},{"positions",positions},{"rtl",!run.leftToRight()},{"object",run.isPlaceholder()}});
    }
    for(auto& cluster:impl->clusters())result["clusters"].push_back({
        {"range",{cluster.textRange().start,cluster.textRange().end}},
        {"glyphCount",cluster.size()},{"width",cluster.width()}});
    for(auto& line:impl->lines())result["lines"].push_back({
        {"range",{line.textWithNewlines().start,line.textWithNewlines().end}},
        {"baseline",line.baseline()},{"roundBaseline",std::floor(line.baseline()+.5)},
        {"delta",line.sizes().delta()},{"height",line.height()},{"offset",{line.offset().x(),line.offset().y()}}});
    for(int i:source.boundaries) {
        if(source.units[i]<0)continue;
        int next=i+1;while(next<int(source.units.size()) && source.units[next]<0)++next;
        if(next>=int(source.units.size()))continue;
        json boxes=json::array();
        for(auto box:paragraph->getRectsForRange(source.units[i],source.units[next],RectHeightStyle::kTight,RectWidthStyle::kTight)) {
            auto r=box.rect;
            auto left=paragraph->getGlyphPositionAtCoordinate(r.left()+.01,r.centerY()).position;
            auto right=paragraph->getGlyphPositionAtCoordinate(r.right()-.01,r.centerY()).position;
            boxes.push_back({{"rect",rect(r)},{"hits",{left,right}},
                {"sourceHits",{source.reverse.at(left),source.reverse.at(right)}}});
        }
        result["ranges"].push_back({{"sourceStart",i},{"boxes",boxes}});
    }
    for(auto box:paragraph->getRectsForPlaceholders())result["objects"].push_back({
        {"rect",rect(box.rect)},{"paintBounds",rect(box.rect)},
        {"contained",box.rect.top()>=-.05 && box.rect.bottom()<=paragraph->getHeight()+.05}});
    for(size_t i=0;i<code.size();++i) {
        auto [a,z]=code[i];require(source.bytes[a]>=0 && source.bytes[z]>=0,"unmapped span edge");
        result["edges"].push_back({{"identity",i},{"source",{a,z}},{"bytes",{source.bytes[a],source.bytes[z]}}});
    }
#ifdef FRAGMENT_PATCHED
    if(enabled) {
        std::vector<ParagraphImpl::ExperimentalFragmentSpan> spans;
        for(size_t i=0;i<code.size();++i)spans.push_back({i,{size_t(source.bytes[code[i].first]),size_t(source.bytes[code[i].second])},manifest.at("padding")[0]});
        result["issues"]=json::array();
        for(auto issue:impl->experimentalCheckFragmentEdges(spans))result["issues"].push_back({
            {"identity",issue.identity},{"edgeByte",issue.edge},
            {"clusterBytes",{issue.cluster.start,issue.cluster.end}},{"glyphCount",issue.glyphCount}});
        result["status"]=result["issues"].empty()?"preflight-only":"blocked-intracluster-edge";
    }
#else
    require(!enabled,"control cannot enable patched preflight");
#endif
    if(!images.empty()) {
        auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(800,180));require(bool(surface),"raster surface failed");
        auto canvas=surface->getCanvas();canvas->clear(SK_ColorWHITE);canvas->translate(24,24);
        SkPaint paint;paint.setColor(0xffb9eadb);
        for(auto box:paragraph->getRectsForPlaceholders())canvas->drawRoundRect(box.rect,4,4,paint);
        paragraph->paint(canvas,0,0);
        auto data=SkPngEncoder::Encode(nullptr,surface->makeImageSnapshot().get(),{});require(bool(data),"PNG encode failed");
        auto path=images/(fixture.at("id").get<std::string>()+"-"+std::to_string(blockIndex)+".png");
        SkFILEWStream stream(path.c_str());require(stream.write(data->data(),data->size()),"PNG write failed");
    }
    return result;
}
int main(int argc,char** argv) {
    try {
        require(argc>=2 && argc<=4,"usage: fragment-preflight manifest [--enabled|--disabled] [image-directory]");
        bool enabled=argc>=3 && std::string(argv[2])=="--enabled";
        if(argc>=3)require(enabled || std::string(argv[2])=="--disabled","unknown mode");
        json manifest;std::ifstream(argv[1])>>manifest;
        auto provider=sk_make_sp<TypefaceFontProvider>();auto loader=SkFontMgr_New_Custom_Empty();
        for(auto it=manifest.at("fonts").begin();it!=manifest.at("fonts").end();++it) {
            auto face=loader->makeFromFile(it.value().get<std::string>().c_str());require(bool(face),"font unavailable");
            provider->registerTypeface(face,SkString(it.key()));
        }
        auto fonts=sk_make_sp<FontCollection>();fonts->setAssetFontManager(provider);fonts->disableFontFallback();
        fonts->setDefaultFontManager(provider,"regular");
        std::filesystem::path images=argc==4?argv[3]:"";if(!images.empty())std::filesystem::create_directories(images);
        bool blocked=false;
        for(auto fixture:manifest.at("fixtures")) {
            int index=0;
            for(auto block:fixture.at("blocks")) {
                auto result=inspect(manifest,fixture,block,fonts,enabled,images,index++);
                blocked |= result.value("status","")=="blocked-intracluster-edge";
                std::cout<<result.dump()<<std::endl;
            }
        }
        return blocked?1:0;
    } catch(const std::exception& e) {std::cerr<<e.what()<<std::endl;return 2;}
}
