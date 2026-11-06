#include "document.h"
#include <SDL_syswm.h>
#include <wayland-client.h>
#include "presentation.h"
#include <deque>
#include <thread>
#include <sys/resource.h>
using Clock=std::chrono::steady_clock;
static double milliseconds(Clock::time_point t) {return std::chrono::duration<double,std::milli>(Clock::now()-t).count();}
// Hooks used only in the generated copy of the pinned Vulkan helper.
GrFlushInfo perfFlushInfo;
double perfSubmissionMs=0;
int perfVkPresentMode=-1;
struct Frame {
    json row;
    struct wp_presentation_feedback* feedback=nullptr;
    bool gpuDone=false,submitted=false;
    uint64_t gpuNs=0;
};
struct Presentation {
    wl_display* display=nullptr;wl_surface* surface=nullptr;wl_registry* registry=nullptr;
    wp_presentation* object=nullptr;uint32_t clock=0;bool clockKnown=false;
    static void clockId(void* data,wp_presentation*,uint32_t id) {auto p=static_cast<Presentation*>(data);p->clock=id;p->clockKnown=true;}
    static void global(void* data,wl_registry* registry,uint32_t name,const char* interface,uint32_t version) {
        auto p=static_cast<Presentation*>(data);
        if(std::string(interface)=="wp_presentation") {
            p->object=static_cast<wp_presentation*>(wl_registry_bind(registry,name,&wp_presentation_interface,std::min(version,1u)));
            static const wp_presentation_listener listener={clockId};wp_presentation_add_listener(p->object,&listener,p);
        }
    }
    static void removed(void*,wl_registry*,uint32_t) {}
    void init(SDL_Window* window) {
        SDL_SysWMinfo info;SDL_VERSION(&info.version);
        if(!SDL_GetWindowWMInfo(window,&info) || info.subsystem!=SDL_SYSWM_WAYLAND)return;
        display=info.info.wl.display;surface=info.info.wl.surface;registry=wl_display_get_registry(display);
        static const wl_registry_listener listener={global,removed};wl_registry_add_listener(registry,&listener,this);
        wl_display_roundtrip(display);wl_display_roundtrip(display);
    }
    static void syncOutput(void* data,struct wp_presentation_feedback*,wl_output* output) {
        static_cast<Frame*>(data)->row["syncOutputProxyId"]=wl_proxy_get_id(reinterpret_cast<wl_proxy*>(output));
    }
    static void presented(void* data,struct wp_presentation_feedback* feedback,uint32_t hi,uint32_t lo,uint32_t ns,
                          uint32_t refresh,uint32_t seqHi,uint32_t seqLo,uint32_t flags) {
        auto f=static_cast<Frame*>(data);f->row["presentation"]="presented";
        f->row["presentedNs"]=((uint64_t(hi)<<32)|lo)*1000000000+ns;
        f->row["refreshNs"]=refresh;f->row["sequence"]=(uint64_t(seqHi)<<32)|seqLo;
        f->row["presentationFlags"]=flags;wp_presentation_feedback_destroy(feedback);f->feedback=nullptr;
    }
    static void discarded(void* data,struct wp_presentation_feedback* feedback) {
        auto f=static_cast<Frame*>(data);f->row["presentation"]="discarded";
        wp_presentation_feedback_destroy(feedback);f->feedback=nullptr;
    }
    void request(Frame& f) {
        if(!object)return;
        f.feedback=wp_presentation_feedback(object,surface);
        static const wp_presentation_feedback_listener listener={syncOutput,presented,discarded};
        wp_presentation_feedback_add_listener(f.feedback,&listener,&f);
    }
    ~Presentation() {if(object)wp_presentation_destroy(object);if(registry)wl_registry_destroy(registry);}
};
static void gpuFinished(void* data,const skgpu::GpuStats& stats) {auto f=static_cast<Frame*>(data);f->gpuDone=true;f->gpuNs=stats.elapsedTime;}
static void submitted(void* data,bool success) {static_cast<Frame*>(data)->submitted=success;}
int main(int argc,char** argv) {
    try {
        require(argc>=6,"usage: native manifest variant backend scenario output [capture|diagnostic]");
        json manifest;std::ifstream(argv[1])>>manifest;auto fixture=manifest.at("fixtures")[0];
        std::string variant=argv[2],scenario=argv[4];bool vk=std::string(argv[3])=="vulkan";
        bool capture=argc>6 && std::string(argv[6])=="capture",diagnostic=argc>6 && std::string(argv[6])=="diagnostic";
        if(variant=="zero")manifest["padding"]={0,0};
        std::deque<Frame> frames; // Declared before contexts: callbacks stay valid on exceptions too.
#include "setup.inc"
        Presentation presentation;presentation.init(window);
        Document document(manifest,fixture);document.enabled=variant!="upstream";
        auto dc=vk?vulkan->directContext():context.get();
        bool gpuSupported=bool(dc->supportedGpuStats() & skgpu::GpuStatsFlags::kElapsedTime);
        int lastW=0,lastH=0;sk_sp<SkSurface> glSurface;
        auto began=Clock::now();bool exposed=false,scenarioStarted=false;int frameIndex=0;json scale;
        while(!scenarioStarted || milliseconds(began)<(capture || diagnostic?1000:7000)) {
            SDL_Event event;while(SDL_PollEvent(&event)) {
                require(event.type!=SDL_QUIT,"measurement window closed");
                if(event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_EXPOSED)exposed=true;
            }
            if(!exposed){SDL_Delay(1);continue;}
            auto frameStart=Clock::now();double seconds=milliseconds(began)/1000;
            if(scenario=="resize" && scenarioStarted)SDL_SetWindowSize(window,700+int(90*std::sin(seconds*3)),600);
            int logicalW,logicalH,width,height;SDL_GetWindowSize(window,&logicalW,&logicalH);
            if(vk)SDL_Vulkan_GetDrawableSize(window,&width,&height);else SDL_GL_GetDrawableSize(window,&width,&height);
            double layoutMs=0;int layoutCalls=0;
            if(width!=lastW || height!=lastH) {
                if(vk)vulkan->resize(width,height);
                auto t=Clock::now();document.layout(logicalW-48);layoutMs=milliseconds(t);layoutCalls=1;
                glSurface.reset();lastW=width;lastH=height;
            }
            if(!scenarioStarted) {began=Clock::now();seconds=0;scenarioStarted=true;}
            if(scale.is_null()) {
                SDL_version version;SDL_GetVersion(&version);int display=SDL_GetWindowDisplayIndex(window);
                SDL_DisplayMode mode{};SDL_GetCurrentDisplayMode(display,&mode);
                std::set<std::string> sdlLibraries;std::ifstream maps("/proc/self/maps");std::string mapped;
                while(std::getline(maps,mapped))if(mapped.find("/libSDL")!=std::string::npos)sdlLibraries.insert(mapped.substr(mapped.find('/')));
                timespec resolution{};bool clockResolved=presentation.clockKnown && clock_getres(presentation.clock,&resolution)==0;
                scale={{"variant",variant},{"api",vk?"vulkan":"opengl"},{"fixtureId",fixture.at("id")},{"sdlVersion",std::to_string(version.major)+"."+std::to_string(version.minor)+"."+std::to_string(version.patch)},
                    {"sdlLoadedLibraries",sdlLibraries},{"windowFlags",SDL_GetWindowFlags(window)},{"glSwapInterval",vk?json(nullptr):json(SDL_GL_GetSwapInterval())},{"vulkanPresentMode",vk?json(perfVkPresentMode):json(nullptr)},{"windowCoordinates",{logicalW,logicalH}},
                    {"drawablePixels",{width,height}},{"drawablePerWindowX",double(width)/logicalW},
                    {"drawablePerWindowY",double(height)/logicalH},{"documentZoom",1},
                    {"desktopScale",1.25},{"desktopScaleSource","owner configuration; protocol diagnosis separate"},
                    {"outputName",SDL_GetDisplayName(display)},{"outputIndex",display},{"refreshHz",mode.refresh_rate},
                    {"platform",SDL_GetCurrentVideoDriver()},{"device",device},
                    {"presentationClock",presentation.clockKnown?json(presentation.clock):json(nullptr)},
                    {"timestampUnitNs",1},{"clockResolutionNs",clockResolved?json(resolution.tv_sec*1000000000LL+resolution.tv_nsec):json(nullptr)},{"gpuElapsedSupported",gpuSupported}};
            }
            auto acquireStart=Clock::now();sk_sp<SkSurface> surface;
            if(vk)surface=vulkan->getBackbufferSurface();
            else {
                if(!glSurface) {
                    GrGLFramebufferInfo info;info.fFBOID=0;info.fFormat=GL_RGBA8;
                    auto target=GrBackendRenderTargets::MakeGL(width,height,0,8,info);
                    glSurface=SkSurfaces::WrapBackendRenderTarget(context.get(),target,kBottomLeft_GrSurfaceOrigin,kRGBA_8888_SkColorType,nullptr,nullptr);
                }
                surface=glSurface;
            }
            require(bool(surface),"missing GPU surface");double acquireMs=milliseconds(acquireStart);
            float offset=0;
            if(scenario=="slow")offset=-float(seconds*15);
            if(scenario=="flick-reversal")offset=-float(160*std::sin(seconds*6));
            if(scenario=="settling")offset=-float(180*(1-std::exp(-seconds*1.5)));
            auto paintStart=Clock::now();
            if(scenario=="selection") {document.selected=0;document.anchor=0;document.selection=1+(int(seconds*60)%std::max(1,int(document.blocks[0].mapping.size())-1));}
            auto canvas=surface->getCanvas();canvas->clear(SK_ColorWHITE);canvas->save();
            canvas->scale(float(width)/logicalW,float(height)/logicalH);canvas->translate(24,24+offset);
            document.paint(canvas);canvas->restore();double paintMs=milliseconds(paintStart);
            if(capture && frameIndex==1) {
                auto png=SkPngEncoder::Encode(dc,surface->makeImageSnapshot().get(),{});require(bool(png),"capture failed");
                SkFILEWStream out((std::string(argv[5])+".png").c_str());require(out.write(png->data(),png->size()),"capture write failed");
            }
            frames.emplace_back();auto& f=frames.back();
            f.row={{"frame",frameIndex++},{"scenarioSeconds",seconds},{"warmup",seconds<2},{"layoutMs",layoutMs},{"layoutCalls",layoutCalls},
                {"paintMs",paintMs},{"acquireMs",acquireMs},{"presentation","missing"},{"presentedNs",nullptr},
                {"refreshNs",nullptr},{"gpuElapsedMs",nullptr},{"windowCoordinates",{logicalW,logicalH}},
                {"drawablePixels",{width,height}},{"documentZoom",1}};
            presentation.request(f);
            perfFlushInfo={};if(gpuSupported) {
                perfFlushInfo.fGpuStatsFlags=skgpu::GpuStatsFlags::kElapsedTime;
                perfFlushInfo.fFinishedWithStatsProc=gpuFinished;perfFlushInfo.fFinishedContext=&f;
                perfFlushInfo.fSubmittedProc=submitted;perfFlushInfo.fSubmittedContext=&f;
            }
            auto submitStart=Clock::now();double swapMs;
            if(vk) {vulkan->swapBuffers();swapMs=milliseconds(submitStart)-perfSubmissionMs;}
            else {
                dc->flush(surface.get(),perfFlushInfo,nullptr);dc->submit();perfSubmissionMs=milliseconds(submitStart);
                auto t=Clock::now();SDL_GL_SwapWindow(window);swapMs=milliseconds(t);
            }
            f.row["submissionMs"]=perfSubmissionMs;f.row["swapBlockingMs"]=swapMs;
            dc->checkAsyncWorkCompletion();int resources=0;size_t bytes=0;dc->getResourceCacheUsage(&resources,&bytes);
            f.row["gpuCacheBytes"]=bytes;f.row["gpuCacheResources"]=resources;
            rusage usage{};getrusage(RUSAGE_SELF,&usage);f.row["peakRssBytes"]=usage.ru_maxrss*1024;
            f.row["frameThreadWallMs"]=milliseconds(frameStart);
        }
        double scenarioElapsedSeconds=milliseconds(began)/1000;
        // Drain outside measurement. Missing feedback/stats remain explicitly unavailable.
        auto drain=Clock::now();while(milliseconds(drain)<500) {SDL_PumpEvents();if(presentation.display)wl_display_dispatch_pending(presentation.display);dc->checkAsyncWorkCompletion();SDL_Delay(1);}
        glSurface.reset();vulkan.reset();context.reset();
        std::ofstream output(argv[5]);output<<json({{"type","scale"},{"data",scale}}).dump()<<'\n';
        for(auto& f:frames) {
            if(f.gpuDone && f.submitted)f.row["gpuElapsedMs"]=double(f.gpuNs)/1000000;
            if(f.feedback)wp_presentation_feedback_destroy(f.feedback);
            output<<f.row.dump()<<'\n';
        }
        output<<json({{"type","run"},{"scenario",scenario},{"warmupSeconds",capture || diagnostic?json(nullptr):json(2)},{"measurementSeconds",capture || diagnostic?json(nullptr):json(5)},
            {"bufferedFrames",frames.size()},{"observedScenarioSeconds",scenarioElapsedSeconds},{"captureRun",capture},{"protocolDiagnostic",diagnostic}}).dump()<<'\n';
        output.flush();require(bool(output),"measurement output write failed");
        // Presentation must release protocol proxies before SDL destroys its display.
        if(presentation.object){wp_presentation_destroy(presentation.object);presentation.object=nullptr;}
        if(presentation.registry){wl_registry_destroy(presentation.registry);presentation.registry=nullptr;}
        if(gl)SDL_GL_DeleteContext(gl);SDL_DestroyWindow(window);SDL_Quit();return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<std::endl;return 2;}
}
