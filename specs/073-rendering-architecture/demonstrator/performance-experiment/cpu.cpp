#include "../boundary-experiment/layout.h"
#include <chrono>
#include <sys/resource.h>
#ifdef PERF_CONTROL_COUNTERS
extern "C" size_t perfControlShapeCount;
#endif
using Clock=std::chrono::steady_clock;
static double elapsed(Clock::time_point t) {return std::chrono::duration<double,std::milli>(Clock::now()-t).count();}
static sk_sp<FontCollection> loadFonts(const json& manifest) {
    auto provider=sk_make_sp<TypefaceFontProvider>();auto loader=SkFontMgr_New_Custom_Empty();
    for(auto it=manifest.at("fonts").begin();it!=manifest.at("fonts").end();++it) {
        auto face=loader->makeFromFile(it.value().get<std::string>().c_str());require(bool(face),"font unavailable");
        provider->registerTypeface(face,SkString(it.key()));
    }
    auto fonts=sk_make_sp<FontCollection>();fonts->setAssetFontManager(provider);
    fonts->setDefaultFontManager(provider,"regular");fonts->disableFontFallback();return fonts;
}
int main(int argc,char** argv) {
    try {
        require(argc==4,"usage: cpu manifest variant cold|warm|diagnostic");
        json m;std::ifstream(argv[1])>>m;
        std::string variant=argv[2],mode=argv[3];bool enabled=variant!="upstream";
        if(variant=="zero")m["padding"]={0,0};
#ifndef FRAGMENT_PATCHED
        require(!enabled,"unpatched binary cannot enable boundaries");
#endif
        auto fonts=loadFonts(m);auto f=m.at("fixtures")[0];float width=f.at("width");
        int samples=mode=="warm"?35:1;
        for(int sample=0;sample<samples;++sample) {
            // Empty paragraph cache gives construction/initial layout the same starting state.
            // Font/typeface caches stay warm within this process.
            fonts->getParagraphCache()->reset();
            std::vector<PreparedBoundaryParagraph> paragraphs;
            auto t=Clock::now();
            for(const auto& b:f.at("blocks"))paragraphs.push_back(prepareBoundaryParagraph(m,b,fonts,enabled));
            double construction=elapsed(t);
#ifdef PERF_CONTROL_COUNTERS
            perfControlShapeCount=0;
#endif
            t=Clock::now();for(auto& p:paragraphs)p.paragraph->layout(width);double initial=elapsed(t);
            json geometry=json::array();
            if(mode=="diagnostic")for(auto& p:paragraphs) {
                auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());json lines=json::array(),runs=json::array();
                for(auto& line:impl->lines())lines.push_back({line.textWithNewlines().start,line.textWithNewlines().end,line.width(),line.height()});
                for(auto& run:impl->runs()) {json glyphs=json::array();for(auto g:run.glyphs())glyphs.push_back(g);runs.push_back(glyphs);}
                geometry.push_back({{"lines",lines},{"glyphs",runs},{"sourceToBytes",p.source.bytes},{"sourceToUnits",p.source.units},{"layoutToSource",p.source.reverse}});
            }
            size_t generation=0;
#ifdef PERF_CONTROL_COUNTERS
            generation=perfControlShapeCount;
#endif
#ifdef FRAGMENT_PATCHED
            for(auto& p:paragraphs)generation+=static_cast<ParagraphImpl*>(p.paragraph.get())->experimentalShapeGeneration();
#endif
            t=Clock::now();for(auto& p:paragraphs)p.paragraph->layout(width+17);double reflow=elapsed(t);
            t=Clock::now();for(auto& p:paragraphs)p.paragraph->layout(width+17);double repeat=elapsed(t);
            json invalidation=nullptr,counters=nullptr,reuse=nullptr;
#ifdef FRAGMENT_PATCHED
            size_t after=0;for(auto& p:paragraphs)after+=static_cast<ParagraphImpl*>(p.paragraph.get())->experimentalShapeGeneration();
            reuse=after==generation;require(after==generation,"width/repeat reshaped");
            if(enabled) {
                bool any=false;t=Clock::now();
                for(auto& p:paragraphs) {
                    auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());auto spans=impl->experimentalSpans();
                    if(spans.empty())continue;
                    any=true;auto before=impl->experimentalShapeGeneration();spans[0].identity+=spans.size();
                    impl->experimentalSetSpans(spans);p.paragraph->layout(width+17);
                    require(impl->experimentalShapeGeneration()==before+1,"metadata failed to invalidate");
                }
                if(any)invalidation=elapsed(t);
            }
#ifdef PERF_COUNTERS
            counters={{"candidates",0},{"fragmentCalculations",0},{"shapeGenerations",0}};
            for(auto& p:paragraphs) {auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());
                counters["candidates"]=counters["candidates"].get<size_t>()+impl->perfCandidates;
                counters["fragmentCalculations"]=counters["fragmentCalculations"].get<size_t>()+impl->perfFragments;
                counters["shapeGenerations"]=counters["shapeGenerations"].get<size_t>()+impl->experimentalShapeGeneration();}
#endif
#endif
#ifdef PERF_CONTROL_COUNTERS
            require(perfControlShapeCount==generation,"control width/repeat reshaped");reuse=true;
            counters={{"shapeGenerations",perfControlShapeCount},{"candidates",nullptr},{"fragmentCalculations",nullptr}};
#endif
            rusage usage{};getrusage(RUSAGE_SELF,&usage);
            json row={{"sample",sample},{"warmup",mode=="warm" && sample<5},{"constructionMs",construction},
                {"initialLayoutMs",initial},{"widthReflowMs",reflow},{"repeatLayoutMs",repeat},
                {"metadataInvalidationMs",invalidation},{"peakRssBytes",usage.ru_maxrss*1024},
                {"shapingReused",reuse},{"counters",counters}};
            if(mode=="diagnostic")row["geometry"]=geometry;
            std::cout<<row.dump()<<std::endl;
        }
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 2;}
}
