// Exercise the independent SkParagraph mapping and hit testing without a GPU.
#define main comparison_entry
#include "main.cpp"
#undef main

int main(int argc,char **argv) {
    if(argc<2 || argc>3)return 2;
    bool trial=argc==3 && std::string(argv[2])=="--fragment-trial";
    json manifest;std::ifstream(argv[1])>>manifest;
    bool passed=true;
    for(auto fixture:manifest["fixtures"]) {
        Document document(manifest,fixture);document.layout(752,trial);
        for(size_t i=0;i<document.blocks.size();++i) {
            auto &block=document.blocks[i];
            if(trial && block.fragmentStatus!="stable")passed=false;
            bool mapping=std::is_sorted(block.mapping.begin(),block.mapping.end())
                         && block.mapping.back()==int(block.source.size());
            passed &= mapping;
            record({{"stage","mapping-test"},{"fixture",fixture["id"]},{"passed",mapping}});
            for(auto object:fixture["blocks"][i]["objects"]) {
                const int begin=object["start"],end=begin+object["length"].get<int>();
                auto index=std::find(block.mapping.begin(),block.mapping.end(),begin)-block.mapping.begin();
                auto boxes=block.paragraph->getRectsForRange(index,index+1,RectHeightStyle::kTight,RectWidthStyle::kTight);
                bool valid=boxes.size()==1;int left=-1,right=-1;
                auto placeholders=block.paragraph->getRectsForPlaceholders();
                SkRect placed=SkRect::MakeEmpty();
                for(size_t k=0;k<block.placeholderIndices.size();++k)
                    if(block.objects[k] && block.mapping[block.placeholderIndices[k]]==begin)placed=placeholders[k].rect;
                if(valid) {
                    auto r=boxes[0].rect;
                    auto a=block.paragraph->getGlyphPositionAtCoordinate(r.left()+.01,r.centerY()).position;
                    auto z=block.paragraph->getGlyphPositionAtCoordinate(r.right()-.01,r.centerY()).position;
                    left=block.mapping[std::min(size_t(a),block.mapping.size()-1)];
                    right=block.mapping[std::min(size_t(z),block.mapping.size()-1)];
                    valid=std::abs(r.width()-object["width"].get<float>())<.05
                        && std::abs(placed.height()-object["ascent"].get<float>()-object["descent"].get<float>())<.05
                        && placed.top()>=-.05 && placed.bottom()<=block.paragraph->getHeight()+.05
                        && std::min(left,right)==begin && std::max(left,right)==end;
                }
                passed &= valid;
                record({{"stage","object-selection"},{"fixture",fixture["id"]},
                        {"objectTop",placed.top()},{"objectBottom",placed.bottom()},{"paragraphHeight",block.paragraph->getHeight()},
                        {"leftSource",left},{"rightSource",right},{"expectedStart",begin},{"expectedEnd",end},
                        {"copiedSource",left>=0 && right>=0?utf.to_bytes(block.source.substr(std::min(left,right),std::abs(right-left))):""},
                        {"passed",valid}});
            }
            for(auto span:fixture["blocks"][i]["spans"]) {
                int begin=span["start"],end=begin+span["length"].get<int>();
                auto a=std::lower_bound(block.mapping.begin(),block.mapping.end(),begin)-block.mapping.begin();
                auto z=std::lower_bound(block.mapping.begin(),block.mapping.end(),end)-block.mapping.begin();
                auto boxes=block.paragraph->getRectsForRange(a,z,RectHeightStyle::kTight,RectWidthStyle::kTight);
                int low=int(block.source.size()),high=0,layoutLow=int(block.mapping.size())-1,layoutHigh=0;
                for(auto box:boxes) {
                    auto l=block.paragraph->getGlyphPositionAtCoordinate(box.rect.left()+.01,box.rect.centerY()).position;
                    auto r=block.paragraph->getGlyphPositionAtCoordinate(box.rect.right()-.01,box.rect.centerY()).position;
                    layoutLow=std::min(layoutLow,int(std::min(l,r)));layoutHigh=std::max(layoutHigh,int(std::max(l,r)));
                    int left=block.mapping[std::min(size_t(l),block.mapping.size()-1)];
                    int right=block.mapping[std::min(size_t(r),block.mapping.size()-1)];
                    low=std::min(low,std::min(left,right));high=std::max(high,std::max(left,right));
                }
                bool valid=!boxes.empty() && low==begin && high==end;
                passed &= valid;
                for(bool reverse:{false,true}) {
                    document.selected=int(i);document.anchor=reverse?layoutHigh:layoutLow;document.selection=reverse?layoutLow:layoutHigh;
                    auto copied=document.selectedSource(),expected=utf.to_bytes(block.source.substr(begin,end-begin));
                    bool exact=copied==expected;passed &= exact;
                    record({{"stage","direction-copy"},{"fixture",fixture["id"]},{"reverse",reverse},
                            {"source",copied},{"expected",expected},{"passed",exact}});
                }
                record({{"stage","formatted-selection"},{"fixture",fixture["id"]},{"kind",span["kind"]},
                        {"passed",valid},{"expectedStart",begin},{"expectedEnd",end},{"hitStart",low},{"hitEnd",high}});
            }
            for(auto span:fixture["blocks"][i]["spans"]) if(span["kind"]=="code") {
                int begin=span["start"],end=begin+span["length"].get<int>();
                struct Fragment { SkRect ink; int a,z; float lineTop; };
                std::vector<Fragment> cells,fragments;
                for(int pos=begin;pos<end;++pos) {
                    int index=block.contentIndices[pos];
                    if(index<0 || (block.source[pos]>=0xdc00 && block.source[pos]<=0xdfff))continue;
                    int endIndex=index+((block.source[pos]>=0xd800 && block.source[pos]<=0xdbff)?2:1);
                    const auto lineBoxes=block.paragraph->getRectsForRange(index,endIndex,RectHeightStyle::kMax,RectWidthStyle::kTight);
                    for(auto box:block.paragraph->getRectsForRange(index,endIndex,RectHeightStyle::kTight,RectWidthStyle::kTight)) {
                        for(auto lineBox:lineBoxes) if(lineBox.rect.top()<=box.rect.centerY() && lineBox.rect.bottom()>=box.rect.centerY()) {
                            cells.push_back({box.rect,index,endIndex,lineBox.rect.top()});break;
                        }
                    }
                }
                // Fallback fonts have different ink centers on the same line.
                // Group by authoritative line bounds and visual adjacency; bidi
                // fragments separated by other content must remain distinct.
                std::sort(cells.begin(),cells.end(),[](auto &a,auto &b) {
                    return a.lineTop==b.lineTop?a.ink.left()<b.ink.left():a.lineTop<b.lineTop;
                });
                for(auto cell:cells) {
                    if(fragments.empty() || std::abs(fragments.back().lineTop-cell.lineTop)>.1
                        || cell.ink.left()>fragments.back().ink.right()+.1)fragments.push_back(cell);
                    else {
                        auto &f=fragments.back();f.ink.join(cell.ink);
                        f.a=std::min(f.a,cell.a);f.z=std::max(f.z,cell.z);
                    }
                }
                for(auto f:fragments) {
                    SkRect reserved=f.ink;
                    int a=block.paddingIndices.count(f.a-1)?f.a-1:f.a;
                    int z=block.paddingIndices.count(f.z)?f.z+1:f.z;
                    for(auto box:block.paragraph->getRectsForRange(a,z,RectHeightStyle::kMax,RectWidthStyle::kTight))
                        if(box.rect.top()<=f.ink.centerY() && box.rect.bottom()>=f.ink.centerY())reserved.join(box.rect);
                    float pad=manifest["padding"][0],vertical=manifest["padding"][1];
                    bool valid=f.ink.left()-reserved.left()>=pad-.05 && reserved.right()-f.ink.right()>=pad-.05
                        && f.ink.top()-reserved.top()>=vertical-.05 && reserved.bottom()-f.ink.bottom()>=vertical-.05;
                    passed &= valid;
                    record({{"stage","fragment-padding"},{"fixture",fixture["id"]},{"trial",trial},
                        {"left",f.ink.left()-reserved.left()},{"right",reserved.right()-f.ink.right()},
                        {"top",f.ink.top()-reserved.top()},{"bottom",reserved.bottom()-f.ink.bottom()},{"passed",valid}});
                }
            }
        }
    }
    return passed?0:1;
}
