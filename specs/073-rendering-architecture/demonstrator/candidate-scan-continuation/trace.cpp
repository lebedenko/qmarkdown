#include "../boundary-experiment/layout.h"
#include <chrono>
#include <cfenv>
#include <xmmintrin.h>
#include <sched.h>
static void require(bool ok,const std::string& message) { if(!ok)throw std::runtime_error(message); }
static sk_sp<FontCollection> loadFonts(const json& manifest) {
    auto provider=sk_make_sp<TypefaceFontProvider>();auto loader=SkFontMgr_New_Custom_Empty();
    for(auto it=manifest.at("fonts").begin();it!=manifest.at("fonts").end();++it) {
        auto face=loader->makeFromFile(it.value().get<std::string>().c_str());require(bool(face),"font unavailable");
        provider->registerTypeface(face,SkString(it.key()));
    }
    auto fonts=sk_make_sp<FontCollection>();fonts->setAssetFontManager(provider);
    fonts->setDefaultFontManager(provider,"regular");fonts->disableFontFallback();return fonts;
}
static json geometry(PreparedBoundaryParagraph& p) {
    auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());
    json lines=json::array(),runs=json::array(),fragments=json::array(),boxes=json::array();
    for(auto& run:impl->runs()) {
        json glyphs=json::array(),positions=json::array();
        for(auto g:run.glyphs())glyphs.push_back(g);
        for(auto xy:run.positions())positions.push_back({xy.x(),xy.y()});
        runs.push_back({{"range",{run.textRange().start,run.textRange().end}},{"glyphs",glyphs},{"positions",positions}});
    }
    for(auto& line:impl->lines()) {
        lines.push_back({line.textWithNewlines().start,line.textWithNewlines().end,line.width(),line.height(),line.offset().x(),line.offset().y(),line.baseline()});
        json slices=json::array();
        for(auto s:impl->experimentalVisualSlices(line.trimmedText()))slices.push_back({s.runIndex,s.identity,s.before,s.after});
        fragments.push_back(slices);
    }
    for(auto b:impl->getRectsForRange(0,p.source.text.size(),RectHeightStyle::kTight,RectWidthStyle::kTight))boxes.push_back(rect(b.rect));
    return {{"lines",lines},{"runs",runs},{"fragments",fragments},{"boxes",boxes},
        {"sourceToBytes",p.source.bytes},{"sourceToUnits",p.source.units},{"layoutToSource",p.source.reverse}};
}
#include <sys/resource.h>
#include <limits>
int main(int argc,char** argv) {
    try {
        require(argc==4,"usage: runtime-trace manifest zero|padded correctness|timing");
        bool correctness=std::string(argv[3])=="correctness";
        require(correctness || std::string(argv[3])=="timing","invalid mode");
        json m;std::ifstream(argv[1])>>m;
        require(std::string(argv[2])=="zero" || std::string(argv[2])=="padded","invalid variant");
        if(std::string(argv[2])=="zero")m["padding"]={0,0};
        using Clock=std::chrono::steady_clock;
        double period=1e9*Clock::period::num/Clock::period::den;
        int64_t resolution=std::numeric_limits<int64_t>::max();
        auto previous=Clock::now();
        for(int i=0;i<10000;++i) {
            auto now=Clock::now();auto delta=std::chrono::duration_cast<std::chrono::nanoseconds>(now-previous).count();
            if(delta>0)resolution=std::min<int64_t>(resolution,delta);
            previous=now;
        }
        require(resolution!=std::numeric_limits<int64_t>::max(),"clock probe failed");
        cpu_set_t affinity;CPU_ZERO(&affinity);require(sched_getaffinity(0,sizeof(affinity),&affinity)==0,"affinity unavailable");
        json cpus=json::array();for(int i=0;i<CPU_SETSIZE;++i)if(CPU_ISSET(i,&affinity))cpus.push_back(i);
        json fp={{"rounding",std::fegetround()},{"mxcsr",_mm_getcsr()}};
        std::vector<std::vector<std::string>> events;
        std::vector<ParagraphImpl*> pointers;
        auto fonts=loadFonts(m);
        if(correctness)fonts->getParagraphCache()->setChecker([&](ParagraphImpl* impl,const char* event,bool) {
            auto it=std::find(pointers.begin(),pointers.end(),impl);
            require(it!=pointers.end(),"cache event for unknown paragraph");
            events[std::distance(pointers.begin(),it)].push_back(event);
        });auto f=m.at("fixtures")[0];float width=f.at("width");
        std::vector<PreparedBoundaryParagraph> paragraphs;
        for(auto& b:f.at("blocks"))paragraphs.push_back(prepareBoundaryParagraph(m,b,fonts,true));
        for(auto& p:paragraphs)pointers.push_back(static_cast<ParagraphImpl*>(p.paragraph.get()));
        for(const char* phase:{"initial","reflow","repeat","metadata"}) {
            events.assign(paragraphs.size(),{});
            bool metadata=std::string(phase)=="metadata";
            using Spans=std::decay_t<decltype(static_cast<ParagraphImpl*>(paragraphs[0].paragraph.get())->experimentalSpans())>;
            std::vector<Spans> copies;
            std::vector<size_t> generations;
            std::vector<ParagraphImpl*> applicable;
            for(auto& p:paragraphs) {
                auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());
                generations.push_back(impl->experimentalShapeGeneration());
                if(metadata) {
                    copies.push_back(impl->experimentalSpans());
                    if(!copies.back().empty()) {copies.back()[0].identity+=copies.back().size();applicable.push_back(impl);}
                } else applicable.push_back(impl);
            }
            auto start=Clock::now();
            if(metadata) {
                size_t j=0;
                for(auto& spans:copies)if(!spans.empty()) {
                    auto impl=applicable[j++];impl->experimentalSetSpans(spans);impl->layout(width+17);
                }
            } else for(auto impl:applicable)impl->layout(std::string(phase)=="initial"?width:width+17);
            auto elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-start).count();
            json shapes=json::array(),geometries=json::array();
            for(size_t i=0;i<paragraphs.size();++i) {
                auto impl=static_cast<ParagraphImpl*>(paragraphs[i].paragraph.get());
                auto delta=impl->experimentalShapeGeneration()-generations[i];
                bool hasMetadata=!impl->experimentalSpans().empty();
                bool initial=std::string(phase)=="initial";
                bool valid=initial ? (hasMetadata ? delta==1 : delta<=1) :
                    delta==size_t(metadata && hasMetadata);
                std::string expected=initial && !hasMetadata ? "0 (cache hit) or 1 (cache miss)" :
                    std::to_string(size_t(initial || (metadata && hasMetadata)));
                require(valid,"paragraph "+std::to_string(i)+" phase "+phase+
                    " expected shaping delta "+expected+" observed "+std::to_string(delta));
                if(correctness) {
                    std::vector<std::string> expectedEvents;
                    if(initial && !hasMetadata)expectedEvents=delta==0 ?
                        std::vector<std::string>{"foundParagraph"} :
                        std::vector<std::string>{"missingParagraph","addedParagraph"};
                    require(events[i]==expectedEvents,"paragraph "+std::to_string(i)+" phase "+phase+
                        " expected cache events "+json(expectedEvents).dump()+" observed "+json(events[i]).dump()+
                        " shaping delta "+std::to_string(delta));
                }
                shapes.push_back(delta);
                if(correctness)geometries.push_back(geometry(paragraphs[i]));
            }
            struct rusage usage{};require(getrusage(RUSAGE_SELF,&usage)==0,"rusage failed");
            json out={{"phase",phase},{"elapsedNs",applicable.empty()?json(nullptr):json(elapsed)},
                {"cpuAffinity",cpus},{"fpEnvironment",fp},{"shapeDeltas",shapes},{"clockPeriodNs",period},{"minimumPositiveDeltaNs",resolution},
                {"highWaterRssBytes",usage.ru_maxrss*1024},{"applicableParagraphs",applicable.size()}};
            if(correctness){out["geometry"]=geometries;out["cacheEvents"]=events;}
            std::cout<<out.dump()<<std::endl;
        }
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 2;}
}
