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
using json=nlohmann::json;
using namespace skia::textlayout;
static auto start=std::chrono::steady_clock::now();
static void record(json data) {
    data["monotonicMs"]=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
    std::cout<<data.dump()<<std::endl;
}
static void require(bool ok,const char *message) { if(!ok)throw std::runtime_error(message); }
static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> utf;
struct DemoBlock {
    std::unique_ptr<Paragraph> paragraph;
    std::u16string source;
    std::vector<int> mapping;
    std::vector<bool> objects;
    std::vector<std::pair<int,int>> codes;
    float y=0;
    bool rule=false,table=false,reservations=false;
};
class Document {
public:
    std::vector<DemoBlock> blocks;
    sk_sp<FontCollection> fonts=sk_make_sp<FontCollection>();
    json manifest,fixture;
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
    void layout(float width) {
        auto began=std::chrono::steady_clock::now();
        blocks.clear();float y=0;
        auto codeFaces=fonts->findTypefaces({SkString("mono")},SkFontStyle());
        require(!codeFaces.empty(),"code font unresolved");
        SkFont codeFont(codeFaces.front(),manifest["fontSize"].get<float>());
        SkFontMetrics metrics;codeFont.getMetrics(&metrics);
        const float codeAscent=-metrics.fAscent+manifest["padding"][1].get<float>();
        const float codeDescent=metrics.fDescent+manifest["padding"][1].get<float>();
        for(auto b:fixture["blocks"]) {
            DemoBlock block;block.y=y;block.rule=b.value("kind","")=="rule";block.table=b.value("kind","")=="table";
            block.source=utf.from_bytes(b["text"].get<std::string>());
            TextStyle base;base.setFontFamilies({SkString("regular"),SkString("cjk"),SkString("emoji")});base.setFontSize(manifest["fontSize"].get<float>());base.setColor(SK_ColorBLACK);
            ParagraphStyle style;style.setTextStyle(base);
            auto builder=ParagraphBuilder::make(style,fonts,SkUnicodes::ICU::Make());
            require(!fonts->findTypefaces(base.getFontFamilies(),base.getFontStyle()).empty(),"explicit fixture fonts unresolved");
            int inserted=0;
            auto placeholder=[&](int index,float w,float ascent,float descent,bool object) {
                builder->addPlaceholder({w,ascent+descent,PlaceholderAlignment::kBaseline,TextBaseline::kAlphabetic,ascent});
                block.mapping.push_back(index);++inserted;block.objects.push_back(object);block.reservations=true;
            };
            const int length=int(block.source.size());
            for(int i=0;i<length;) {
                bool object=false;
                for(auto o:b["objects"]) if(o["start"]==i) {
                    placeholder(i,o["width"],o["ascent"],o["descent"],true);
                    i+=o["length"].get<int>();object=true;break;
                }
                if(object)continue;
                TextStyle current=base;
                for(auto span:b["spans"]) {
                    int a=span["start"],n=span["length"];std::string kind=span["kind"];
                    if(i>=a && i<a+n) {
                        if(kind=="code")current.setFontFamilies({SkString("mono"),SkString("cjk"),SkString("emoji")});
                        if(kind=="bold")current.setFontFamilies({SkString("bold"),SkString("cjk"),SkString("emoji")});
                        if(kind=="italic")current.setFontFamilies({SkString("italic"),SkString("cjk"),SkString("emoji")});
                        if(kind=="strike")current.setDecoration(TextDecoration::kLineThrough);
                    }
                    if(kind=="code" && i==a) {
                        placeholder(i,manifest["padding"][0],codeAscent,codeDescent,false);
                        block.codes.push_back({inserted-1,0});
                    }
                }
                builder->pushStyle(current);
                int count=(block.source[i]>=0xd800 && block.source[i]<=0xdbff)?2:1;
                builder->addText(block.source.substr(i,count));builder->pop();
                for(int k=0;k<count;++k){block.mapping.push_back(i+k);++inserted;}
                i+=count;
                for(auto span:b["spans"]) if(span["kind"]=="code" && i==span["start"].get<int>()+span["length"].get<int>()) {
                    placeholder(i,manifest["padding"][0],codeAscent,codeDescent,false);
                    block.codes.back().second=inserted;
                }
            }
            block.mapping.push_back(length);
            record({{"stage","paragraph-input"},{"sourceUnits",block.source.size()},{"utf8Bytes",builder->getText().size()},{"mapping",block.mapping}});
            block.paragraph=builder->Build();
            block.paragraph->layout(std::min(width,fixture["width"].get<float>()));
            record({{"stage","paragraph-geometry"},{"height",block.paragraph->getHeight()},{"width",block.paragraph->getMaxIntrinsicWidth()},{"unresolvedGlyphs",block.paragraph->unresolvedGlyphs()}});
            y+=block.rule?30:block.paragraph->getHeight()+18;
            blocks.push_back(std::move(block));
        }
        record({{"stage","prepared"},{"blocks",blocks.size()},
                {"preparationMs",std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count()},
                {"reservationSupport","experimental-unqualified"},{"fallbackFonts","explicit-only; missing glyphs unsupported"}});
    }
    void paint(SkCanvas *canvas) {
        SkPaint paint;paint.setAntiAlias(true);
        for(size_t i=0;i<blocks.size();++i) {
            auto &b=blocks[i];canvas->save();canvas->translate(0,b.y);
            if(b.rule){paint.setColor(0xff85919b);canvas->drawRect(SkRect::MakeWH(fixture["width"],1),paint);canvas->restore();continue;}
            for(auto range:b.codes) {
                paint.setColor(0xffeeeeee);
                for(auto box:b.paragraph->getRectsForRange(range.first,range.second,RectHeightStyle::kMax,RectWidthStyle::kTight))
                    canvas->drawRoundRect(box.rect,4,4,paint);
            }
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
    void copy() {
        if(selected<0)return;
        auto &b=blocks[selected];
        int a=b.mapping[std::clamp(std::min(anchor,selection),0,int(b.mapping.size())-1)];
        int z=b.mapping[std::clamp(std::max(anchor,selection),0,int(b.mapping.size())-1)];
        auto text=utf.to_bytes(b.source.substr(a,z-a));
        SDL_SetClipboardText(text.c_str());record({{"stage","copied"},{"source",text}});
    }
};
int main(int argc,char **argv) {
    try {
        record({{"stage","startup"}});
        require(argc>=5,"usage: skia-compare manifest fixture backend output [--interactive]");
        json manifest;std::ifstream(argv[1])>>manifest;json fixture;
        for(auto f:manifest["fixtures"])if(f["id"]==argv[2])fixture=f;
        require(!fixture.is_null(),"unknown fixture");
        if(argc>5 && std::string(argv[5])=="--layout-only") {
            Document document(manifest,fixture);document.layout(752);
            record({{"stage","exit"},{"code",0}});return 0;
        }
        const bool vk=std::string(argv[3])=="vulkan",interactive=argc>5 && std::string(argv[5])=="--interactive";
        require(SDL_Init(SDL_INIT_VIDEO)==0,SDL_GetError());
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
        auto *window=SDL_CreateWindow("073 Skia | reservations UNQUALIFIED | Space S F R 0 +/- drag Ctrl+C",
            SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600,
            SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI|(vk?SDL_WINDOW_VULKAN:SDL_WINDOW_OPENGL));
        require(window,SDL_GetError());
        SDL_GLContext gl=nullptr;sk_sp<GrDirectContext> context;
        std::unique_ptr<skwindow::internal::VulkanWindowContext> vulkan;
        std::string device;
        if(vk) {
            auto proc=reinterpret_cast<PFN_vkGetInstanceProcAddr>(SDL_Vulkan_GetVkGetInstanceProcAddr());
            require(proc,"Vulkan loader missing");
            auto create=[&](VkInstance instance){VkSurfaceKHR surface;require(SDL_Vulkan_CreateSurface(window,instance,&surface),SDL_GetError());return surface;};
            auto present=[&](VkInstance instance,VkPhysicalDevice physical,uint32_t queue) {
                auto surface=create(instance);
                auto support=reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(proc(instance,"vkGetPhysicalDeviceSurfaceSupportKHR"));
                auto destroy=reinterpret_cast<PFN_vkDestroySurfaceKHR>(proc(instance,"vkDestroySurfaceKHR"));
                auto properties=reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(proc(instance,"vkGetPhysicalDeviceProperties"));
                VkBool32 ok=false;support(physical,queue,surface,&ok);destroy(instance,surface,nullptr);
                VkPhysicalDeviceProperties p;properties(physical,&p);device=p.deviceName;return bool(ok);
            };
            vulkan=std::make_unique<skwindow::internal::VulkanWindowContext>(std::make_unique<skwindow::DisplayParams>(),create,present,proc);
            require(vulkan->isValid(),"Skia Vulkan initialization failed");
        } else {
            gl=SDL_GL_CreateContext(window);require(gl,SDL_GetError());SDL_GL_SetSwapInterval(1);
            context=GrDirectContexts::MakeGL(GrGLMakeAssembledInterface(nullptr,[](void *,const char *name){return reinterpret_cast<GrGLFuncPtr>(SDL_GL_GetProcAddress(name));}));require(bool(context),"Skia GL initialization failed");
            device=reinterpret_cast<const char *>(glGetString(GL_RENDERER));
        }
        Document document(manifest,fixture);
        int width=0,height=0,lastW=0,lastH=0,frame=0;float zoom=1,offset=0,speed=.25;bool moving=false,done=false,exposed=false;
        std::filesystem::create_directories(argv[4]);std::vector<uint8_t> pose,stationary;
        sk_sp<SkSurface> glSurface;int wraps=0;
        while(!done) {
            if(!interactive) {
                if(frame==5)offset=-40.25;
                if(frame==7)offset=0;
                if(frame==9)SDL_SetWindowSize(window,620,480);
                if(frame==11){zoom=1.25;lastW=0;}
                if(frame==13){zoom=1;SDL_SetWindowSize(window,800,600);lastW=0;}
            }
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                if(event.type==SDL_QUIT)done=true;
                if(event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_EXPOSED)exposed=true;
                if(event.type==SDL_KEYDOWN) {
                    auto key=event.key.keysym.sym;
                    if(key==SDLK_SPACE)moving=!moving;
                    if(key==SDLK_s){moving=true;speed=.25;}
                    if(key==SDLK_f){moving=true;speed=12;}
                    if(key==SDLK_r)speed=-speed;
                    if(key==SDLK_0){offset=0;zoom=1;moving=false;lastW=0;}
                    if(key==SDLK_EQUALS || key==SDLK_PLUS){zoom=std::min(3.f,zoom+.125f);lastW=0;}
                    if(key==SDLK_MINUS){zoom=std::max(.5f,zoom-.125f);lastW=0;}
                    if(key==SDLK_c && (event.key.keysym.mod&KMOD_CTRL))document.copy();
                }
                if(event.type==SDL_MOUSEBUTTONDOWN || (event.type==SDL_MOUSEMOTION && (event.motion.state&SDL_BUTTON_LMASK))) {
                    float x=event.type==SDL_MOUSEBUTTONDOWN?event.button.x:event.motion.x;
                    float y=event.type==SDL_MOUSEBUTTONDOWN?event.button.y:event.motion.y;
                    document.select((x-24)/zoom,(y-24-offset)/zoom,event.type==SDL_MOUSEBUTTONDOWN);
                }
            }
            if(!exposed){SDL_Delay(1);continue;}
            int logicalW,logicalH;SDL_GetWindowSize(window,&logicalW,&logicalH);
            if(vk)SDL_Vulkan_GetDrawableSize(window,&width,&height);else SDL_GL_GetDrawableSize(window,&width,&height);
            if(width!=lastW || height!=lastH) {
                if(vk)vulkan->resize(width,height);
                document.layout((logicalW-48)/zoom);glSurface.reset();lastW=width;lastH=height;
            }
            sk_sp<SkSurface> surface;
            if(vk)surface=vulkan->getBackbufferSurface();
            else {
                if(!glSurface) {
                    GrGLFramebufferInfo info;info.fFBOID=0;info.fFormat=GL_RGBA8;
                    auto target=GrBackendRenderTargets::MakeGL(width,height,0,8,info);
                    glSurface=SkSurfaces::WrapBackendRenderTarget(context.get(),target,kBottomLeft_GrSurfaceOrigin,kRGBA_8888_SkColorType,nullptr,nullptr);
                    ++wraps;
                }
                surface=glSurface;
            }
            require(bool(surface),"missing GPU surface");
            if(moving){offset+=speed;if(std::abs(offset)>180)speed=-speed;}
            if(!interactive && frame>=2 && frame<=4)offset=17.375;
            auto *canvas=surface->getCanvas();canvas->clear(SK_ColorWHITE);canvas->save();
            canvas->scale(float(width)/logicalW,float(height)/logicalH);canvas->translate(24,24+offset);canvas->scale(zoom,zoom);
            document.paint(canvas);canvas->restore();
            if(frame==0) {
                SDL_DisplayMode mode;SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(window),&mode);
                record({{"stage","ready"},{"api",vk?"vulkan":"opengl"},{"device",device},
                        {"platform",SDL_GetCurrentVideoDriver()},{"dpr",double(width)/logicalW},
                        {"framebufferWidth",width},{"framebufferHeight",height},{"refreshHz",mode.refresh_rate}});
            }
            if(!interactive && (frame==1 || frame==3 || frame==4 || frame==6 || frame==8 || frame==10 || frame==12 || frame==14)) {
                const std::string name=frame==1?"stationary":frame==3?"pose":frame==4?"replay":frame==6?"clip-entry":frame==8?"settled":frame==10?"resize":frame==12?"zoom":"reset";
                record({{"stage","capture"},{"name",name}});
                auto image=surface->makeImageSnapshot();
                auto png=SkPngEncoder::Encode(vk?vulkan->directContext():context.get(),image.get(),{});
                require(bool(png),"PNG encoding failed");
                SkFILEWStream out((std::string(argv[4])+"/"+name+".png").c_str());
                require(out.isValid() && out.write(png->data(),png->size()),"PNG write failed");
                std::vector<uint8_t> bytes(static_cast<const uint8_t *>(png->data()),static_cast<const uint8_t *>(png->data())+png->size());
                if(frame==1)stationary=bytes;
                if(frame==3)pose=bytes;
                if(frame==4){record({{"stage","replay"},{"exact",pose==bytes}});require(pose==bytes,"replay mismatch");}
                if(frame==14){record({{"stage","replay"},{"exact",stationary==bytes},{"operation","resize-zoom-reset"}});require(stationary==bytes,"reset mismatch");done=true;}
            }
            if(vk)vulkan->swapBuffers();else {context->flushAndSubmit();SDL_GL_SwapWindow(window);}
            ++frame;
        }
        record({{"stage","shutdown"},{"glSurfaceWraps",wraps},{"paragraphResources",document.blocks.size()}});
        glSurface.reset();
        vulkan.reset();context.reset();
        if(gl)SDL_GL_DeleteContext(gl);
        SDL_DestroyWindow(window);SDL_Quit();
        record({{"stage","exit"},{"code",0}});return 0;
    } catch(const std::exception &error) {
        record({{"stage","failure"},{"message",error.what()}});return 1;
    }
}
