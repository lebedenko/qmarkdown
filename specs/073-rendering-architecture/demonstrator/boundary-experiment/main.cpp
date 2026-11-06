#include "layout.h"

static json inspect(const json& manifest,const json& fixture,const json& block,
                    sk_sp<FontCollection> fonts,bool enabled,const std::filesystem::path& images,int blockIndex) {
    auto prepared=prepareBoundaryParagraph(manifest,block,fonts,enabled);
    auto& source=prepared.source;
    auto& code=prepared.code;
    auto& paragraph=prepared.paragraph;
    auto& layoutText=prepared.layoutText;
    auto byte=layoutText.size();
    auto impl=static_cast<ParagraphImpl*>(paragraph.get());
#ifdef FRAGMENT_PATCHED
    auto spans=impl->experimentalSpans();
#endif
    auto cacheCount=fonts->getParagraphCache()->count();
    paragraph->layout(fixture.at("width"));
#ifdef FRAGMENT_PATCHED
    if(!spans.empty())require(cacheCount==fonts->getParagraphCache()->count(),"boundary metadata entered shaping cache");
#endif
    json result={{"fixture",fixture.at("id")},{"block",blockIndex},{"source",block.at("text")},
        {"layoutText",layoutText},{"sourceToBytes",source.bytes},{"sourceToUnits",source.units},{"layoutToSource",source.reverse},
        {"height",paragraph->getHeight()},{"unresolvedGlyphs",paragraph->unresolvedGlyphs()},
        {"runs",json::array()},{"clusters",json::array()},{"lines",json::array()},
        {"ranges",json::array()},{"objects",json::array()},{"edges",json::array()}};
    result["breakFlags"]=json::array();
    for(size_t i=0;i<=size_t(byte);++i)
        if(impl->codeUnitHasProperty(i,SkUnicode::CodeUnitFlags::kSoftLineBreakBefore) ||
           impl->codeUnitHasProperty(i,SkUnicode::CodeUnitFlags::kHardLineBreakBefore))
            result["breakFlags"].push_back(i);
    auto nextBoundary=[&](int i) {
        auto next=source.boundaries.upper_bound(i);
        while(next!=source.boundaries.end() && source.units[*next]<0)++next;
        return next==source.boundaries.end()?int(source.units.size()):*next;
    };
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
        {"delta",line.sizes().delta()},{"rawAscent",line.sizes().rawAscent()},
        {"rawDescent",line.sizes().rawDescent()},{"ascent",line.sizes().ascent()},
        {"descent",line.sizes().descent()},{"leading",line.sizes().leading()},{"height",line.height()},{"offset",{line.offset().x(),line.offset().y()}}});
#ifdef FRAGMENT_PATCHED
    if(enabled) {
        result["fragments"]=json::array();
        int lineIndex=0;
        for(auto& line:impl->lines()) {
            auto slices=impl->experimentalVisualSlices(line.trimmedText());
            for(auto slice:slices)result["fragments"].push_back({{"line",lineIndex},
                {"identity",slice.identity},{"run",slice.runIndex},{"before",slice.before},{"after",slice.after}});
            result["lines"][lineIndex]["visualRuns"]=json::array();
            line.iterateThroughVisualRuns(false,[&](const Run* run,SkScalar offset,TextRange range,SkScalar* width) {
                auto context=line.measureTextInsideOneRun(range,run,offset,0,false,TextLine::GlyphCluster);
                *width=context.clip.width();
                result["lines"][lineIndex]["visualRuns"].push_back({{"run",run->index()},
                    {"range",{range.start,range.end}},{"rect",rect(context.clip)}});
                return true;
            });
            result["lines"][lineIndex]["width"]=line.width();
            result["lines"][lineIndex]["trimmedRange"]={line.trimmedText().start,line.trimmedText().end};
            result["lines"][lineIndex]["padding"]=impl->experimentalPadding(line.trimmedText());
            result["lines"][lineIndex]["overflow"]=std::max(0.0f,line.width()-fixture.at("width").get<float>());
            ++lineIndex;
        }
    }
#endif
    for(int i:source.boundaries) {
        if(source.units[i]<0)continue;
        int next=nextBoundary(i);
        if(next>=int(source.units.size()))continue;
        json boxes=json::array();
        for(auto box:paragraph->getRectsForRange(source.units[i],source.units[next],RectHeightStyle::kTight,RectWidthStyle::kTight)) {
            auto r=box.rect;
            auto nearLeft=paragraph->getGlyphPositionAtCoordinate(r.left()+.01,r.centerY()).position;
            auto nearRight=paragraph->getGlyphPositionAtCoordinate(r.right()-.01,r.centerY()).position;
            auto margin=std::min(.1f,r.width()/4);
            auto left=paragraph->getGlyphPositionAtCoordinate(r.left()+margin,r.centerY()).position;
            auto right=paragraph->getGlyphPositionAtCoordinate(r.right()-margin,r.centerY()).position;
            boxes.push_back({{"rect",rect(r)},{"hits",{left,right}},
                {"sourceHits",{source.reverse.at(left),source.reverse.at(right)}},
                {"rtl",box.direction==TextDirection::kRtl},{"probeMargin",margin},
                {"nearEdgeSourceHits",{source.reverse.at(nearLeft),source.reverse.at(nearRight)}}});
        }
        result["ranges"].push_back({{"sourceStart",i},{"sourceEnd",next},{"boxes",boxes}});
    }
    for(auto box:paragraph->getRectsForPlaceholders()) {
        auto r=box.rect;
        auto mask=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(
            std::max(1,int(std::ceil(r.right()))+32),std::max(1,int(std::ceil(paragraph->getHeight()))+32)));
        require(bool(mask),"object mask allocation failed");
        mask->getCanvas()->clear(SK_ColorTRANSPARENT);
        mask->getCanvas()->translate(16,16);
        SkPaint paint;paint.setColor(0xffb9eadb);paint.setAntiAlias(true);
        mask->getCanvas()->drawRoundRect(r,4,4,paint);
        SkPixmap pixels;require(mask->peekPixels(&pixels),"object pixels unavailable");
        int left=pixels.width(),top=pixels.height(),right=0,bottom=0;
        for(int y=0;y<pixels.height();++y)for(int x=0;x<pixels.width();++x)
            if(SkColorGetA(pixels.getColor(x,y))) {
                left=std::min(left,x);top=std::min(top,y);right=std::max(right,x+1);bottom=std::max(bottom,y+1);
            }
        auto painted=SkRect::MakeLTRB(left-16,top-16,right-16,bottom-16);
        result["objects"].push_back({{"rect",rect(r)},{"paintBounds",rect(r)},
            {"pixelPaintBoundsDpr1",rect(painted)},
            {"paintContained",painted.top()>=-.05 && painted.bottom()<=paragraph->getHeight()+.05},
            {"contained",r.top()>=-.05 && r.bottom()<=paragraph->getHeight()+.05}});
    }
    for(size_t i=0;i<code.size();++i) {
        auto [a,z]=code[i];require(source.bytes[a]>=0 && source.bytes[z]>=0,"unmapped span edge");
        result["edges"].push_back({{"identity",i},{"source",{a,z}},{"bytes",{source.bytes[a],source.bytes[z]}}});
    }
#ifdef FRAGMENT_PATCHED
    if(enabled) {
        result["issues"]=json::array();
        for(auto span:spans)for(auto edge:{span.source.start,span.source.end})
            for(auto& cluster:impl->clusters())
                if(cluster.textRange().start<edge && edge<cluster.textRange().end)
                    result["issues"].push_back({{"identity",span.identity},{"edge",edge}});
        result["status"]=result["issues"].empty()?"boundary-only":"blocked-intracluster-edge";
        auto snapshot=[&]() {
            json state=json::array();
            for(auto& line:impl->lines())state.push_back({line.textWithNewlines().start,
                line.textWithNewlines().end,line.width(),line.height(),line.baseline()});
            for(auto& run:impl->runs()) {
                json glyphs=json::array(),positions=json::array();
                for(auto glyph:run.glyphs())glyphs.push_back(glyph);
                for(auto pos:run.positions())positions.push_back({pos.x(),pos.y()});
                state.push_back({run.textRange().start,run.textRange().end,glyphs,positions});
            }
            for(auto box:paragraph->getRectsForPlaceholders())state.push_back(rect(box.rect));
            for(auto i:source.boundaries)if(source.units[i]>=0 && i<int(source.text.size())) {
                int next=nextBoundary(i);
                for(auto box:paragraph->getRectsForRange(source.units[i],source.units[next],RectHeightStyle::kTight,RectWidthStyle::kTight))
                    state.push_back(rect(box.rect));
            }
            return state;
        };
        const auto original=snapshot();
        auto generation=impl->experimentalShapeGeneration();
        paragraph->layout(fixture.at("width"));
        require(source.text.empty() || generation==impl->experimentalShapeGeneration(),"repeat reshaped");
        paragraph->layout(fixture.at("width").get<float>()+1);
        require(source.text.empty() || generation==impl->experimentalShapeGeneration(),"width reshaped");
        paragraph->layout(fixture.at("width"));
        impl->experimentalSetSpans(spans);
        paragraph->layout(fixture.at("width"));
        require(source.text.empty() || generation==impl->experimentalShapeGeneration(),"equal metadata reshaped");
        if(!spans.empty()) {
            auto changed=spans;changed[0].identity+=1000;
            impl->experimentalSetSpans(changed);paragraph->layout(fixture.at("width"));
            require(impl->experimentalShapeGeneration()==generation+1,"metadata failed to reshape");
            impl->experimentalSetSpans({});paragraph->layout(fixture.at("width"));
            impl->experimentalSetSpans(spans);paragraph->layout(fixture.at("width"));
            require(impl->experimentalShapeGeneration()>=generation+2,"re-added metadata failed to reshape");
        }
        if(original!=snapshot())std::cerr<<json({{"before",original},{"after",snapshot()}}).dump()<<std::endl;
        require(original==snapshot(),"metadata/width round trip changed geometry");
        result["invalidationChecked"]=true;
    }
#else
    require(!enabled,"control cannot enable boundaries");
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
