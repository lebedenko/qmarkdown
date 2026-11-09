// Measurement copy of boundary native Document; full traversal preserved.
#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_opengl.h>
#include <nlohmann/json.hpp>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkStream.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/ports/SkFontMgr_empty.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLAssembleInterface.h"
#include "include/encode/SkPngEncoder.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TypefaceFontProvider.h"
#include "modules/skunicode/include/SkUnicode_icu.h"
#include "tools/window/VulkanWindowContext.h"
#include <chrono>
#include <codecvt>
#include <locale>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
using json=nlohmann::json;
using namespace skia::textlayout;
static auto start=std::chrono::steady_clock::now();
static void record(json data) {
    data["monotonicMs"]=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    /* Font/layout diagnostic logging disabled in timed adapter. */
}
#include "../boundary-experiment/layout.h"
struct DemoBlock {
    std::unique_ptr<Paragraph> paragraph;
    std::u16string source;
    std::vector<int> mapping, contentIndices, placeholderIndices;
    std::string fragmentStatus="not-trialled";
    std::vector<bool> objects;
    std::set<int> paddingIndices;
    std::vector<std::pair<int,int>> codes;
    float y=0;
    bool rule=false,table=false,reservations=false;
};
class Document {
public:
    std::vector<DemoBlock> blocks;
    sk_sp<FontCollection> fonts=sk_make_sp<FontCollection>();
    json manifest,fixture;
    bool enabled=true;
    int selected=-1,anchor=0,selection=0;
    Document(json m,json f):manifest(m),fixture(f) {
        auto provider=sk_make_sp<TypefaceFontProvider>();
        auto loader=SkFontMgr_New_Custom_Empty();
        for(auto it=m["fonts"].begin();it!=m["fonts"].end();++it) {
            auto face=loader->makeFromFile(it.value().get<std::string>().c_str());
            require(bool(face),"font load failed");
            record({{"stage","font"},{"role",it.key()},{"glyphs",face->countGlyphs()},{"A",face->unicharToGlyph(65)}});
            provider->registerTypeface(face,SkString(it.key()));
        }
        fonts->setAssetFontManager(provider);
        fonts->setDefaultFontManager(provider,"regular");
        fonts->disableFontFallback();
    }
    void layout(float width, bool = false) {
        auto began=std::chrono::steady_clock::now();
        if(blocks.empty())for(auto input:fixture["blocks"]) {
            auto prepared=prepareBoundaryParagraph(manifest,input,fonts,enabled);
            DemoBlock block;
            block.rule=input.value("kind","")=="rule";block.table=input.value("kind","")=="table";
            block.source=prepared.source.text;
            block.mapping=prepared.source.reverse;
            block.contentIndices=prepared.source.units;
            for(auto range:prepared.code)block.codes.push_back({prepared.source.units[range.first],prepared.source.units[range.second]});
            block.objects.assign(input["objects"].size(),true);
            block.paragraph=std::move(prepared.paragraph);
            blocks.push_back(std::move(block));
        }
        float y=0;
        for(auto& block:blocks) {
            block.y=y;
            block.paragraph->layout(std::min(width,fixture["width"].get<float>()));
            y+=block.rule?30:block.paragraph->getHeight()+18;
        }
        record({{"stage","prepared"},{"blocks",blocks.size()},
            {"preparationMs",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count()},
            {"reservationSupport","boundary-experiment; automated gates separate from visual acceptance"}});
    }
    void paint(SkCanvas *canvas) {
        SkPaint paint;paint.setAntiAlias(true);
        for(size_t i=0;i<blocks.size();++i) {
            auto &b=blocks[i];canvas->save();canvas->translate(0,b.y);
            if(b.rule){paint.setColor(0xff85919b);canvas->drawRect(SkRect::MakeWH(fixture["width"],1),paint);canvas->restore();continue;}
#ifdef FRAGMENT_PATCHED
            paintBoundaryBackgrounds(b.paragraph.get(),canvas);
#else
            // Preserve the original viewer's unpadded code-background painter.
            for(auto range:b.codes) {
                paint.setColor(0xffeeeeee);
                for(auto box:b.paragraph->getRectsForRange(range.first,range.second,RectHeightStyle::kMax,RectWidthStyle::kTight))
                    canvas->drawRoundRect(box.rect,4,4,paint);
            }
#endif
            if(int(i)==selected) {
                paint.setColor(0xff99bbee);
                for(auto box:b.paragraph->getRectsForRange(std::min(anchor,selection),std::max(anchor,selection),RectHeightStyle::kMax,RectWidthStyle::kTight))
                    canvas->drawRect(box.rect,paint);
            }
            auto boxes=b.paragraph->getRectsForPlaceholders();
            for(size_t j=0;j<boxes.size() && j<b.objects.size();++j) if(b.objects[j]) {
                paint.setColor(0xffb9eadb);canvas->drawRoundRect(boxes[j].rect,4,4,paint);
            }
            b.paragraph->paint(canvas,0,0);
            if(b.table) {paint.setColor(0xffcbd5df);canvas->drawRect(SkRect::MakeXYWH(0,b.paragraph->getHeight()-1,fixture["width"],1),paint);}
            canvas->restore();
        }
    }
    void select(float x,float y,bool begin) {
        for(size_t i=0;i<blocks.size();++i) {
            auto &b=blocks[i];
            if(y>=b.y && y<b.y+b.paragraph->getHeight()) {
                int pos=b.paragraph->getGlyphPositionAtCoordinate(x,y-b.y).position;
                if(begin){selected=int(i);anchor=pos;}
                if(selected==int(i))selection=pos;
                return;
            }
        }
    }
    std::string selectedSource() const {
        if(selected<0)return {};
        auto &b=blocks[selected];
        int a=b.mapping[std::clamp(std::min(anchor,selection),0,int(b.mapping.size())-1)];
        int z=b.mapping[std::clamp(std::max(anchor,selection),0,int(b.mapping.size())-1)];
        return utf.to_bytes(b.source.substr(a,z-a));
    }
    void copy() {
        auto text=selectedSource();
        SDL_SetClipboardText(text.c_str());record({{"stage","copied"},{"source",text}});
    }
};
