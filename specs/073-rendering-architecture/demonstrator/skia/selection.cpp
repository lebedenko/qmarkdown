// Exercise the independent SkParagraph mapping and hit testing without a GPU.
#define main comparison_entry
#include "main.cpp"
#undef main

int main(int argc,char **argv) {
    if(argc!=2)return 2;
    json manifest;std::ifstream(argv[1])>>manifest;
    bool passed=true;
    for(auto fixture:manifest["fixtures"]) {
        Document document(manifest,fixture);document.layout(752);
        for(size_t i=0;i<document.blocks.size();++i) {
            auto &block=document.blocks[i];
            bool mapping=std::is_sorted(block.mapping.begin(),block.mapping.end())
                         && block.mapping.back()==int(block.source.size());
            passed &= mapping;
            record({{"stage","mapping-test"},{"fixture",fixture["id"]},{"passed",mapping}});
            for(auto object:fixture["blocks"][i]["objects"]) {
                const int begin=object["start"],end=begin+object["length"].get<int>();
                auto index=std::find(block.mapping.begin(),block.mapping.end(),begin)-block.mapping.begin();
                auto boxes=block.paragraph->getRectsForRange(index,index+1,RectHeightStyle::kTight,RectWidthStyle::kTight);
                bool valid=boxes.size()==1;int left=-1,right=-1;
                if(valid) {
                    auto r=boxes[0].rect;
                    auto a=block.paragraph->getGlyphPositionAtCoordinate(r.left()+.01,r.centerY()).position;
                    auto z=block.paragraph->getGlyphPositionAtCoordinate(r.right()-.01,r.centerY()).position;
                    left=block.mapping[std::min(size_t(a),block.mapping.size()-1)];
                    right=block.mapping[std::min(size_t(z),block.mapping.size()-1)];
                    valid=std::abs(r.width()-object["width"].get<float>())<.05
                        && std::min(left,right)==begin && std::max(left,right)==end;
                }
                passed &= valid;
                record({{"stage","object-selection"},{"fixture",fixture["id"]},
                        {"leftSource",left},{"rightSource",right},{"expectedStart",begin},{"expectedEnd",end},
                        {"copiedSource",left>=0 && right>=0?utf.to_bytes(block.source.substr(std::min(left,right),std::abs(right-left))):""},
                        {"passed",valid}});
            }
            for(auto span:fixture["blocks"][i]["spans"]) {
                int begin=span["start"],end=begin+span["length"].get<int>();
                auto a=std::lower_bound(block.mapping.begin(),block.mapping.end(),begin)-block.mapping.begin();
                auto z=std::lower_bound(block.mapping.begin(),block.mapping.end(),end)-block.mapping.begin();
                auto boxes=block.paragraph->getRectsForRange(a,z,RectHeightStyle::kTight,RectWidthStyle::kTight);
                int low=int(block.source.size()),high=0;
                for(auto box:boxes) {
                    auto l=block.paragraph->getGlyphPositionAtCoordinate(box.rect.left()+.01,box.rect.centerY()).position;
                    auto r=block.paragraph->getGlyphPositionAtCoordinate(box.rect.right()-.01,box.rect.centerY()).position;
                    int left=block.mapping[std::min(size_t(l),block.mapping.size()-1)];
                    int right=block.mapping[std::min(size_t(r),block.mapping.size()-1)];
                    low=std::min(low,std::min(left,right));high=std::max(high,std::max(left,right));
                }
                bool valid=!boxes.empty() && low==begin && high==end;
                passed &= valid;
                record({{"stage","formatted-selection"},{"fixture",fixture["id"]},{"kind",span["kind"]},
                        {"passed",valid},{"expectedStart",begin},{"expectedEnd",end},{"hitStart",low},{"hitEnd",high}});
            }
            if(!block.codes.empty()) {
                bool multiline=false;
                for(auto range:block.codes) {
                    auto boxes=block.paragraph->getRectsForRange(range.first,range.second,RectHeightStyle::kMax,RectWidthStyle::kTight);
                    for(size_t j=1;j<boxes.size();++j)if(std::abs(boxes[j].rect.top()-boxes[0].rect.top())>.1)multiline=true;
                }
                record({{"stage","composition-support"},{"fixture",fixture["id"]},
                        {"status",multiline?"unsupported-per-line-padding":"unqualified-reservation-semantics"}});
            }
        }
    }
    return passed?0:1;
}
