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
using Clock=std::chrono::steady_clock;
static int64_t ns(Clock::duration d) {return std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();}
static json boundary() {
    auto start=Clock::now();
    int cpu=sched_getcpu();struct rusage u{};int result=getrusage(RUSAGE_SELF,&u);
    auto end=Clock::now();
    auto optional=[](bool valid,json value,const char* reason) {return json{{"value",valid?value:json(nullptr)},{"reason",valid?json(nullptr):json(reason)}};};
    return {{"startMonotonicNs",ns(start.time_since_epoch())},{"durationNs",ns(end-start)},
        {"cpu",optional(cpu>=0,cpu,"sched_getcpu failed")},
        {"processCpuNs",optional(result==0,int64_t(u.ru_utime.tv_sec+u.ru_stime.tv_sec)*1000000000LL+int64_t(u.ru_utime.tv_usec+u.ru_stime.tv_usec)*1000,"getrusage failed")},
        {"voluntary",optional(result==0,u.ru_nvcsw,"getrusage failed")},
        {"involuntary",optional(result==0,u.ru_nivcsw,"getrusage failed")}};
}
int main(int argc,char** argv) {
 try {
    require(argc==8,"usage: prepared-reflow FIXTURE zero correctness|timing --batch-size N --operation-telemetry disabled|enabled");
    bool correctness=std::string(argv[3])=="correctness",enabled=std::string(argv[7])=="enabled";
    require(correctness || std::string(argv[3])=="timing","invalid mode");
    require(std::string(argv[2])=="zero" && std::string(argv[4])=="--batch-size" && std::string(argv[6])=="--operation-telemetry","invalid options");
    require(enabled || std::string(argv[7])=="disabled","invalid telemetry");
    int n=std::stoi(argv[5]);require(n==1 || n==16 || n==64 || n==256,"invalid size");
    json m;std::ifstream(argv[1])>>m;m["padding"]={0,0};auto f=m.at("fixtures")[0];
    require(f.at("blocks").size()==1 && float(f.at("width"))==480,"unexpected fixture");
    int64_t resolution=std::numeric_limits<int64_t>::max();auto previous=Clock::now();
    for(int i=0;i<10000;++i){auto now=Clock::now();auto d=ns(now-previous);if(d>0)resolution=std::min(resolution,d);previous=now;}
    require(resolution!=std::numeric_limits<int64_t>::max(),"clock probe failed");
    auto fonts=loadFonts(m);auto cache=fonts->getParagraphCache();int before=cache->count();
    std::vector<PreparedBoundaryParagraph> paragraphs;paragraphs.reserve(n);
    std::vector<ParagraphImpl*> pointers;std::vector<std::vector<std::string>> events(n);
    for(int i=0;i<n;++i){paragraphs.push_back(prepareBoundaryParagraph(m,f.at("blocks")[0],fonts,true));pointers.push_back(static_cast<ParagraphImpl*>(paragraphs.back().paragraph.get()));}
    require(std::set<ParagraphImpl*>(pointers.begin(),pointers.end()).size()==size_t(n),"aliased instances");
    if(correctness)cache->setChecker([&](ParagraphImpl* p,const char* event,bool){auto it=std::find(pointers.begin(),pointers.end(),p);require(it!=pointers.end(),"unknown cache instance");events[it-pointers.begin()].push_back(event);});
    for(auto p:pointers)p->layout(480);
    cache->setChecker({});int prepared=cache->count();
    std::vector<size_t> generations;std::vector<int> initialStates;for(auto p:pointers){generations.push_back(p->experimentalShapeGeneration());initialStates.push_back(int(p->state()));require(p->getMaxWidth()==480 && p->state()==kFormatted,"initial state");}
    if(correctness){for(int i=0;i<n;++i)require(generations[i]==size_t(i==0),"initial shaping generation");require(before==0 && prepared==1,"shared cache count");for(int i=0;i<n;++i)require(events[i]==(i==0?std::vector<std::string>{"missingParagraph","addedParagraph"}:std::vector<std::string>{"foundParagraph"}),"shared cache events");}
    json pre=nullptr,post=nullptr;if(enabled)pre=boundary();
    auto start=Clock::now();
    for(auto p:pointers)p->layout(497);
    auto end=Clock::now();
    if(enabled)post=boundary();
    int64_t elapsed=ns(end-start);require(elapsed>0,"invalid elapsed");
    for(int i=0;i<n;++i)require(pointers[i]->experimentalShapeGeneration()==generations[i] && pointers[i]->getMaxWidth()==497 && pointers[i]->state()==kFormatted,"reflow state/generation");
    std::vector<size_t> finalGenerations;std::vector<int> finalStates;for(auto p:pointers){finalGenerations.push_back(p->experimentalShapeGeneration());finalStates.push_back(int(p->state()));}
    int reflow=cache->count();require(prepared==reflow,"reflow changed cache");
    json geometries=json::array();bool isolation=false;
    if(correctness){
        for(auto& p:paragraphs)geometries.push_back(geometry(p));
        for(auto& g:geometries)require(g==geometries[0],"instance geometry mismatch");
        pointers[0]->layout(311);require(pointers[0]->getMaxWidth()==311,"mutation failed");
        require(geometry(paragraphs[0])!=geometries[0],"mutation unchanged geometry");
        for(int i=1;i<n;++i)require(geometry(paragraphs[i])==geometries[i] && pointers[i]->getMaxWidth()==497 && pointers[i]->experimentalShapeGeneration()==generations[i],"mutation leaked");
        isolation=true;
    }
    json delta=nullptr;if(enabled){delta=json::object();for(auto key:{"processCpuNs","voluntary","involuntary"})delta[key]=pre[key]["value"].is_null()||post[key]["value"].is_null()?json(nullptr):json(post[key]["value"].get<int64_t>()-pre[key]["value"].get<int64_t>());}
    json out={{"phase","prepared-reflow"},{"mode",argv[3]},{"telemetry",argv[7]},{"elapsedNs",elapsed},{"applicableParagraphs",n},{"elapsedPerParagraphNs",double(elapsed)/n},
        {"clockPeriodNs",1e9*Clock::period::num/Clock::period::den},{"minimumPositiveDeltaNs",resolution},
        {"timerStartMonotonicNs",ns(start.time_since_epoch())},{"timerEndMonotonicNs",ns(end.time_since_epoch())},
        {"initialGenerations",generations},{"finalGenerations",finalGenerations},{"initialStates",initialStates},{"finalStates",finalStates},{"initialWidth",480},
        {"cacheCounts",{before,prepared,reflow}},{"sharedFontCollection",true},{"cachePolicy","default unchanged; fallback disabled"},
        {"distinctInstances",true},{"generationUnchanged",true},{"finalWidth",497},{"boundaryBefore",pre},{"boundaryAfter",post},{"boundaryDeltas",delta}};
    if(correctness){out["geometry"]=geometries;out["cacheEvents"]=events;out["mutationIsolation"]=isolation;}
    std::cout<<out.dump()<<std::endl;return 0;
 } catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 2;}
}
