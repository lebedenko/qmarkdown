#include "../boundary-experiment/layout.h"
#include <chrono>
#include "ScanBound.h"
static sk_sp<FontCollection> loadFonts(const json& manifest) {
    auto provider=sk_make_sp<TypefaceFontProvider>();auto loader=SkFontMgr_New_Custom_Empty();
    for(auto it=manifest.at("fonts").begin();it!=manifest.at("fonts").end();++it) {
        auto face=loader->makeFromFile(it.value().get<std::string>().c_str());require(bool(face),"font unavailable");
        provider->registerTypeface(face,SkString(it.key()));
    }
    auto fonts=sk_make_sp<FontCollection>();fonts->setAssetFontManager(provider);
    fonts->setDefaultFontManager(provider,"regular");fonts->disableFontFallback();return fonts;
}
static json counters(ParagraphImpl* p) {
#ifdef PHASE_COUNTERS
    json out={{"candidates",p->phaseCandidates},{"whitespaceVisits",p->phaseWhitespace},
        {"ownerBuilds",p->ownerBuilds},{"ownerExaminations",p->ownerExaminations},
        {"ownerLookups",p->ownerLookups},{"ownerCapacity",p->ownerCapacity()},
        {"ownerPayloadBytes",p->ownerPayload()}};
    for(int i=0;i<2;++i) {
        const auto& w=p->phaseWork[i];
        out[i==0?"fitting":"formatting"]={{"calls",w.calls},{"runVisits",w.runVisits},
            {"ownerComparisons",w.ownerComparisons},{"reorderCalls",w.reorderCalls},{"slices",w.slices}};
    }
    const auto& s=p->scanWork;
    out["floatingPoint"]={{"rounding",std::fegetround()},{"mxcsr",_mm_getcsr()}};
    out["scan"]={{"preprocessing",s.preprocessing},{"trimming",s.trimming},
        {"suffixComparisons",s.suffixComparisons},{"evaluations",s.evaluations},
        {"skipped",s.skipped},{"shadowChecks",s.shadowChecks},{"scratchSize",s.scratchSize},
        {"scratchCapacity",s.scratchCapacity},{"scratchBytes",s.scratchBytes},{"lines",s.lines},
        {"fallback",{s.fallback[0],s.fallback[1],s.fallback[2],s.fallback[3],s.fallback[4],s.fallback[5],s.fallback[6]}}};
    return out;
#else
    return nullptr;
#endif
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
int main(int argc,char** argv) {
    try {
        require(argc==4,"usage: phase-trace manifest zero|padded oracle|shadow|prototype");
        std::string mode=argv[3];
        require(mode=="oracle" || mode=="shadow" || mode=="prototype","invalid scan mode");
        json m;std::ifstream(argv[1])>>m;
        if(std::string(argv[2])=="zero")m["padding"]={0,0};
        auto fonts=loadFonts(m);auto f=m.at("fixtures")[0];float width=f.at("width");
        std::vector<PreparedBoundaryParagraph> paragraphs;
        for(auto& b:f.at("blocks"))paragraphs.push_back(prepareBoundaryParagraph(m,b,fonts,true));
        for(const char* phase:{"initial","reflow","repeat","metadata"}) {
            json work=json::array(),shapes=json::array(),geometries=json::array();
            double ms=0;
            for(auto& p:paragraphs) {
                auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());
                impl->scanMode=mode=="oracle"?0:mode=="shadow"?1:2;
                auto before=impl->experimentalShapeGeneration();
#ifdef PHASE_COUNTERS
                impl->resetPhaseWork();
                require(!impl->phaseFitting,"fitting scope leaked");
#endif
                bool metadata=std::string(phase)=="metadata";
                auto spans=impl->experimentalSpans();
                auto start=std::chrono::steady_clock::now();
                if(metadata && !spans.empty()) {spans[0].identity+=spans.size();impl->experimentalSetSpans(spans);}
                if(!metadata || !spans.empty())p.paragraph->layout(std::string(phase)=="initial"?width:width+17);
                ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
                auto after=impl->experimentalShapeGeneration();
                if(std::string(phase)=="reflow" || std::string(phase)=="repeat")require(before==after,"width/repeat reshaped");
                if(metadata && !spans.empty())require(after==before+1,"metadata did not invalidate");
                work.push_back(counters(impl)); // Snapshot before any geometry traversal.
                shapes.push_back(after-before);
                auto g=geometry(p);
#ifdef PHASE_COUNTERS
                auto afterQuery=counters(impl);
                impl->resetPhaseWork();
                require(impl->experimentalShapeGeneration()==after,"reset changed shaping state");
                require(geometry(p)==g,"reset changed geometry");
                require(afterQuery["candidates"]==work.back()["candidates"],"query performed fitting");
#endif
                geometries.push_back(g);
            }
            std::cout<<json({{"phase",phase},{"diagnosticMs",ms},{"counters",work},
                {"shapeDeltas",shapes},{"geometry",geometries}}).dump()<<std::endl;
        }
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 2;}
}
