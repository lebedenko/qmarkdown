// Minimal toolkit-only scale query; no desktop-scale input or viewport ownership.
#include <iostream>
#include <string>
#include <poll.h>
#include <unistd.h>
#ifdef PROBE_QT
#include <QGuiApplication>
#include <QWindow>
#include <QTimer>
#include <QScreen>
#include <QElapsedTimer>
#include <QBackingStore>
#include <QPainter>
#include <QSocketNotifier>
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv);QWindow window;QElapsedTimer clock;clock.start();
    window.resize(800,600);window.show();QBackingStore store(&window);
    QTimer timer;
    QObject::connect(&timer,&QTimer::timeout,&window,[&]{
        if(window.isExposed()) {
            store.resize(window.size());store.beginPaint(QRect(QPoint(),window.size()));
            QPainter painter(store.paintDevice());painter.fillRect(0,0,800,600,Qt::white);
            painter.fillRect(24,24,22,22,Qt::black);painter.end();store.endPaint();
            store.flush(QRect(QPoint(),window.size()));
        }
        std::cout<<"{\"timeNs\":"<<clock.nsecsElapsed()<<",\"qt\":\""<<qVersion()
          <<"\",\"window\":["<<window.width()<<","<<window.height()<<"],\"density\":"
          <<window.devicePixelRatio()<<",\"output\":\""<<window.screen()->name().toStdString()<<"\"}"<<std::endl;
    });timer.start(100);
    QSocketNotifier input(STDIN_FILENO,QSocketNotifier::Read,&window);
    bool live=app.arguments().contains("--live");input.setEnabled(live);
    QObject::connect(&input,&QSocketNotifier::activated,&window,[&]{
        char data[256];auto n=read(STDIN_FILENO,data,sizeof(data));
        if(n<=0 || std::string(data,n).find("quit")!=std::string::npos)app.quit();
    });
    if(!live)QTimer::singleShot(5000,&app,&QCoreApplication::quit);
    return app.exec();
}
#else
#ifdef PROBE_SDL3
#include <SDL3/SDL.h>
#else
#include <SDL2/SDL.h>
#endif
int main(int argc,char **) {
    bool live=argc>1;
#ifdef PROBE_SDL3
    if(!SDL_Init(SDL_INIT_VIDEO))return 2;
    auto window=SDL_CreateWindow("scale contract SDL3",800,600,SDL_WINDOW_HIGH_PIXEL_DENSITY|SDL_WINDOW_RESIZABLE);
#else
    if(SDL_Init(SDL_INIT_VIDEO)!=0)return 2;
    auto window=SDL_CreateWindow("scale contract SDL2",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600,SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
#endif
    if(!window){std::cerr<<SDL_GetError()<<std::endl;return 3;}
    auto start=SDL_GetTicks();
    while(live || SDL_GetTicks()-start<5000) {
        pollfd input{STDIN_FILENO,POLLIN,0};
        if(live && poll(&input,1,0)>0) {
            char data[256];auto n=read(STDIN_FILENO,data,sizeof(data));
            if(n<=0 || std::string(data,n).find("quit")!=std::string::npos)break;
        }
        auto surface=SDL_GetWindowSurface(window);
        if(!surface){std::cerr<<SDL_GetError()<<std::endl;return 4;}
#ifdef PROBE_SDL3
        SDL_FillSurfaceRect(surface,nullptr,0xffffffff);
#else
        SDL_FillRect(surface,nullptr,0xffffffff);
#endif
        SDL_UpdateWindowSurface(window);
        SDL_Event event;unsigned type=0;
        while(SDL_PollEvent(&event)) {type=event.type;std::cout<<"{\"event\":"<<type<<",\"ticks\":"<<SDL_GetTicks()<<"}"<<std::endl;}
        int w,h,pw,ph;SDL_GetWindowSize(window,&w,&h);
#ifdef PROBE_SDL3
        SDL_GetWindowSizeInPixels(window,&pw,&ph);
        std::cout<<"{\"density\":"<<SDL_GetWindowPixelDensity(window)<<",\"displayScale\":"<<SDL_GetWindowDisplayScale(window)<<",";
#else
        SDL_GetWindowSizeInPixels(window,&pw,&ph);
        std::cout<<"{";
#endif
        std::cout<<"\"ticks\":"<<SDL_GetTicks()<<",\"window\":["<<w<<","<<h<<"],\"buffer\":["<<pw<<","<<ph<<"]}"<<std::endl;
        SDL_Delay(100);
    }
    SDL_DestroyWindow(window);SDL_Quit();
}
#endif
