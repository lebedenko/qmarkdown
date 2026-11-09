// Read-only diagnostics of the unchanged historical reservation adapter.
#define main historical_comparison_entry
#include "../skia/main.cpp"
#undef main
#include "modules/skparagraph/src/ParagraphImpl.h"
int main(int argc,char** argv) {
    if(argc!=2)return 2;
    json manifest;std::ifstream(argv[1])>>manifest;
    for(auto fixture:manifest["fixtures"])if(fixture["id"]=="objects") {
        fixture["width"]=679;
        Document document(manifest,fixture);document.layout(752);
        for(auto& block:document.blocks) {
            auto impl=static_cast<ParagraphImpl*>(block.paragraph.get());
            json lines=json::array(),objects=json::array();
            for(auto& line:impl->lines())lines.push_back({
                {"baseline",line.baseline()},{"roundBaseline",std::floor(line.baseline()+.5)},
                {"rawHeight",line.sizes().ideographicBaseline()},{"height",line.height()},
                {"delta",line.sizes().delta()}});
            auto boxes=block.paragraph->getRectsForPlaceholders();
            for(size_t i=0;i<boxes.size();++i)if(block.objects[i]) {
                auto r=boxes[i].rect;
                // Match original painting (non-antialiased round rectangle), with
                // a margin to observe pixels outside the paragraph's allocation.
                auto surface=SkSurfaces::Raster(SkImageInfo::MakeN32Premul(800,100));
                auto canvas=surface->getCanvas();canvas->clear(SK_ColorTRANSPARENT);
                canvas->translate(0,16);SkPaint paint;paint.setColor(0xffb9eadb);
                canvas->drawRoundRect(r,4,4,paint);
                SkPixmap pixels;surface->peekPixels(&pixels);int first=100,last=-1;
                for(int y=0;y<100;++y)for(int x=0;x<800;++x)if(SkColorGetA(pixels.getColor(x,y))) {
                    first=std::min(first,y);last=std::max(last,y);
                }
                objects.push_back({{"pathBounds",{r.left(),r.top(),r.right(),r.bottom()}},
                    {"paintedPixelRowsAtDpr1",{first-16,last-16}},
                    {"containedAtTolerance005",r.top()>=-.05 && r.bottom()<=block.paragraph->getHeight()+.05}});
            }
            record({{"stage","historical-object-rounding"},{"width",679},{"lines",lines},
                {"objects",objects},{"paragraphHeight",block.paragraph->getHeight()}});
        }
    }
}
