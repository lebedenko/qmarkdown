#define main trace_main
#include "trace.cpp"
#undef main

namespace skia::textlayout {
struct OwnershipTestAccess {
    static bool valid(ParagraphImpl* p) { return p->fOwnersValid; }
    static size_t capacity(ParagraphImpl* p) { return p->fRunOwners.capacity(); }
    static const size_t* data(ParagraphImpl* p) { return p->fRunOwners.data(); }
    static void invalidate(ParagraphImpl* p) { p->invalidateRunOwners(); }
    static void build(ParagraphImpl* p) { p->buildRunOwners(); }
    static void stale(ParagraphImpl* p) { for(auto& x:p->fRunOwners)x=0; ++p->fRunRevision; }
    static void corrupt(ParagraphImpl* p) { for(auto& x:p->fRunOwners)x=p->kOwnershipLegacy-1; }
    static void unsplit(ParagraphImpl* p) {
        p->invalidateRunOwners();
        p->fExperimentalSpans={{7,{2,5},3,1},{8,{5,8},4,2}};
        p->buildRunOwners();
    }
};
}
using A=OwnershipTestAccess;
static json slices(ParagraphImpl* p, TextRange r) {
    json out=json::array();
    for(auto s:p->experimentalVisualSlices(r))out.push_back({s.runIndex,s.identity,s.before,s.after});
    return out;
}
static void differential(ParagraphImpl* p,size_t length) {
    std::vector<json> expected;
    for(size_t a=0;a<=length;++a)for(size_t b=a;b<=length;++b)expected.push_back(slices(p,{a,b}));
    A::invalidate(p);
    size_t i=0;
    for(size_t a=0;a<=length;++a)for(size_t b=a;b<=length;++b)
        require(slices(p,{a,b})==expected[i++],"legacy range mismatch");
    A::build(p);
}
int main(int argc,char** argv) {
    try {
        require(argc==2,"ownership-tests font manifest required");
        json m;std::ifstream(argv[1])>>m;auto fonts=loadFonts(m);
        json block={{"text","abcdefghij klmnop"},{"spans",json::array({
            {{"kind","code"},{"start",2},{"length",3}},
            {{"kind","code"},{"start",5},{"length",3}}})},{"objects",json::array()}};
        auto p=prepareBoundaryParagraph(m,block,fonts,true);
        auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());impl->layout(70);
        require(A::valid(impl),"missing table");differential(impl,16);
        auto ptr=A::data(impl);auto cap=A::capacity(impl);auto gen=impl->experimentalShapeGeneration();
        for(auto width:{87,87,0,70}) {impl->layout(width);require(A::data(impl)==ptr && A::capacity(impl)==cap,"reflow allocation");}
        require(gen==impl->experimentalShapeGeneration(),"reflow shaping");
        auto spans=impl->experimentalSpans();impl->experimentalSetSpans(spans);
        require(A::data(impl)==ptr && A::valid(impl),"equal setter invalidation");
        auto expected=slices(impl,{0,16});A::stale(impl);
        require(slices(impl,{0,16})==expected,"stale table accepted");A::build(impl);A::corrupt(impl);
        require(slices(impl,{0,16})==expected,"invalid entry accepted");A::build(impl);
        for(int mutation=0;mutation<3;++mutation) {
            auto changed=impl->experimentalSpans();
            if(mutation==0)changed[0].identity+=10;
            if(mutation==1)changed[0].horizontalPadding+=2;
            if(mutation==2)changed[0].source.start=1;
            impl->experimentalSetSpans(changed);
            require(!A::valid(impl) && A::capacity(impl)==0,"metadata retained table");
            impl->layout(70);require(A::valid(impl),"metadata did not rebuild");differential(impl,16);
        }
        impl->setState(kIndexed);require(!A::valid(impl) && A::capacity(impl)==0,"discard retained table");
        impl->layout(70);require(A::valid(impl),"replacement missing table");
        impl->experimentalSetSpans({});impl->layout(70);
        require(!A::valid(impl) && A::capacity(impl)==0,"removal retained storage");
        impl->experimentalSetSpans(spans);impl->layout(70);require(A::valid(impl),"readd failed");
        impl->experimentalSetSpans({});impl->layout(70); // metadata-free cache hit
        require(!A::valid(impl) && A::capacity(impl)==0,"cache inherited owners");
        block["spans"]=json::array();auto plain=prepareBoundaryParagraph(m,block,fonts,true);
        auto pi=static_cast<ParagraphImpl*>(plain.paragraph.get());pi->layout(500);
        require(A::capacity(pi)==0,"plain allocated");A::unsplit(pi);differential(pi,16);
        block["text"]="";auto empty=prepareBoundaryParagraph(m,block,fonts,true);
        auto ei=static_cast<ParagraphImpl*>(empty.paragraph.get());ei->layout(70);
        require(!A::valid(ei) && A::capacity(ei)==0,"empty/failed shape retained owners");
        // Missing font collection forces a nonempty shape failure.
        auto noFonts=sk_make_sp<FontCollection>();noFonts->disableFontFallback();
        block["text"]="abc";block["spans"]=json::array({{{"kind","code"},{"start",0},{"length",3}}});
        auto failed=prepareBoundaryParagraph(m,block,noFonts,true);
        auto fi=static_cast<ParagraphImpl*>(failed.paragraph.get());fi->layout(70);
        require(!A::valid(fi) && A::capacity(fi)==0,"failed shaping table");
        block["text"]="abcXdef";block["spans"]=json::array({{{"kind","code"},{"start",0},{"length",7}}});
        block["objects"]=json::array({{{"start",3},{"length",1},{"width",12},{"ascent",10},{"descent",3}}});
        auto object=prepareBoundaryParagraph(m,block,fonts,true);auto oi=static_cast<ParagraphImpl*>(object.paragraph.get());
        oi->layout(70);differential(oi,9);
        std::cout<<"ownership ranges, fallback, stale entries, lifecycle, cache, failure and objects passed\n";
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 2;}
}
