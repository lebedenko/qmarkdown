#define main trace_main
#include "trace.cpp"
#undef main
#include <cfenv>
int main(int argc,char** argv) {
    try {
        require(argc==2,"font manifest required");json m;std::ifstream(argv[1])>>m;auto fonts=loadFonts(m);
        size_t cases=0;
        // UTF-16 lengths: combining sequence, supplementary emoji and mixed bidi.
        for(auto text:{std::string("ab   cd\nefgh"),std::string("a\xCC\x81" "bc xyz"),
                       std::string("abc \xD7\x90\xD7\x91 xyz"),std::string("abcdefghijk"),
                       std::string("a\xE2\x80\x8B" "b c"),std::string("   ")}) {
            auto length=text=="ab   cd\nefgh"?12:text=="abcdefghijk"?11:text=="   "?3:
                text.find("\xD7")!=std::string::npos?10:text.find("\xE2")!=std::string::npos?5:8;
            json block={{"text",text},{"spans",json::array({{{"kind","code"},{"start",0},{"length",length}}})},{"objects",json::array()}};
            for(float width:{0.f,1.f,17.25f,40.f,std::nextafter(40.f,0.f),std::nextafter(40.f,100.f),400.f})
                for(auto pad:{json::array({0,0}),json::array({0,3}),json::array({4,2})}) {
                    m["padding"]=pad;json expected;std::vector<PreparedBoundaryParagraph> keep;
                    for(int mode=0;mode<3;++mode) {
                        auto p=prepareBoundaryParagraph(m,block,fonts,true);auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());
                        impl->scanMode=mode;impl->layout(width);auto g=geometry(p);
                        if(mode==0)expected=g;else require(g==expected,"focused geometry mismatch");
                        if(pad==json::array({0,0}))require(impl->scanWork.preprocessing==0 && impl->scanWork.scratchCapacity==0,"zero path preprocessing");
                        require(impl->scanWork.scratchSize<=impl->clusters().size(),"scratch size bound");
                        // Keep paragraphs alive to avoid cross-mode cache coupling.
                        keep.push_back(std::move(p));
                    }
                    ++cases;
                }
        }
        // Adjacent identities and placeholder coverage.
        json block={{"text","abcdef ghijk"},{"spans",json::array({
            {{"kind","code"},{"start",0},{"length",3}},{{"kind","code"},{"start",3},{"length",3}}})},
            {"objects",json::array({{{"start",7},{"length",1},{"width",24},{"ascent",14},{"descent",4}}})}};
        m["padding"]={4,2};json expected;
        for(int mode=0;mode<3;++mode){auto p=prepareBoundaryParagraph(m,block,fonts,true);auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());impl->scanMode=mode;impl->layout(39);if(!mode)expected=geometry(p);else require(geometry(p)==expected,"adjacent/object mismatch");}
        // Actual scanner fallback equivalence under unsupported rounding/padding.
        for(int reason:{2,4}) {
            json oracle;
            for(int mode:{0,2}) {
                auto p=prepareBoundaryParagraph(m,block,fonts,true);auto impl=static_cast<ParagraphImpl*>(p.paragraph.get());impl->scanMode=mode;
                if(reason==4){auto spans=impl->experimentalSpans();spans[0].horizontalPadding=-1;impl->experimentalSetSpans(spans);}
                int old=std::fegetround();if(reason==2)std::fesetround(FE_DOWNWARD);
                impl->layout(39);auto g=geometry(p);std::fesetround(old);
                if(!mode)oracle=g;else {require(g==oracle,"fallback geometry mismatch");require(impl->scanWork.fallback[reason]>0,"missing fallback reason");}
            }
        }
        std::cout<<"focused cases="<<cases+3<<" passed\n";
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
