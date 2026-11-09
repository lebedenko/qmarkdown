#!/usr/bin/env python3
"""Generate an isolated SDL3 candidate from hash-pinned historical sources."""
import hashlib
import json
from pathlib import Path
import shutil
import sys
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[3]

def replace(text, old, new):
    if text.count(old)!=1: raise ValueError('anchor drift: '+old[:100])
    return text.replace(old,new)

def generate(destination):
    pins=json.loads((HERE/'frozen-inputs.json').read_text())
    for name,digest in pins.items():
        if hashlib.sha256((ROOT/name).read_bytes()).hexdigest()!=digest:
            raise ValueError('frozen source changed: '+name)
    source=HERE/'baseline'
    destination.mkdir(parents=True,exist_ok=False)
    for p in source.iterdir():
        if p.is_file():shutil.copyfile(p,destination/p.name)
    shutil.copyfile(HERE/'mapping.h',destination/'mapping.h')
    qt=(source/'qt.cpp').read_text()
    qt=replace(qt,'    const auto capture=[&](QString name) {','''    qint64 mappingGeneration=0;
    QJsonObject previousMapping;
    const auto contractMapping=[&](int bufferWidth,int bufferHeight) {
        double sx=double(bufferWidth)/window.width(),sy=double(bufferHeight)/window.height();
        QJsonObject value{{"window",QJsonArray{window.width(),window.height()}},
            {"surface",QJsonArray{window.width(),window.height()}},
            {"logical",QJsonArray{window.width(),window.height()}},
            {"buffer",QJsonArray{bufferWidth,bufferHeight}},
            {"density",window.devicePixelRatio()},{"displayScale",window.devicePixelRatio()},
            {"contentScale",1},{"zoom",window.zoom},{"scroll",window.offset},
            {"forward",QJsonArray{sx*window.zoom,sy*window.zoom,24*sx,(24+window.offset)*sy}},
            {"inverse",QJsonArray{1/(sx*window.zoom),1/(sy*window.zoom),
                -24/window.zoom,-(24+window.offset)/window.zoom}}};
        if(value!=previousMapping) {
            previousMapping=value;++mappingGeneration;
            value["stage"]="scale-contract";value["generation"]=mappingGeneration;record(value);
        }
    };
    const auto capture=[&](QString name) {''')
    qt=replace(qt,'        record({{"stage","capture-mapping"},{"name",name},',
        '        contractMapping(image.width(),image.height());\n        record({{"stage","capture-mapping"},{"generation",mappingGeneration},{"name",name},')
    qt=replace(qt,'record({{"stage","checkpoint"},{"id",label},',
        'record({{"stage","checkpoint"},{"generation",mappingGeneration},{"id",label},')
    (destination/'qt.cpp').write_text(qt)
    native=(source/'native.cpp').read_text()
    for old,new in [('#include <SDL.h>','#include <SDL3/SDL.h>\n#include "mapping.h"'),
        ('#include <SDL_vulkan.h>','#include <SDL3/SDL_vulkan.h>'),
        ('#include <SDL_opengl.h>','#include <SDL3/SDL_opengl.h>'),
        ('SDL_Init(SDL_INIT_VIDEO)==0','SDL_Init(SDL_INIT_VIDEO)'),
        ('SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600,','800,600,'),
        ('SDL_WINDOW_ALLOW_HIGHDPI','SDL_WINDOW_HIGH_PIXEL_DENSITY'),
        ('SDL_Vulkan_CreateSurface(window,instance,&surface)','SDL_Vulkan_CreateSurface(window,instance,nullptr,&surface)'),
        ('SDL_GL_DeleteContext','SDL_GL_DestroyContext'),
        ('SDL_GetWindowDisplayIndex','SDL_GetDisplayForWindow'),
        ('SDL_QUIT','SDL_EVENT_QUIT'),('SDL_KEYDOWN','SDL_EVENT_KEY_DOWN'),
        ('SDL_MOUSEBUTTONDOWN','SDL_EVENT_MOUSE_BUTTON_DOWN'),('SDL_MOUSEMOTION','SDL_EVENT_MOUSE_MOTION'),
        ('event.key.keysym.sym','event.key.key'),('event.key.keysym.mod','event.key.mod'),
        ('KMOD_CTRL','SDL_KMOD_CTRL')]:
        if old not in native:raise ValueError(old)
        native=native.replace(old,new)
    for key in 'sfrc':native=native.replace('SDLK_'+key,'SDLK_'+key.upper())
    native=replace(native,'event.type==SDL_WINDOWEVENT && event.window.event==SDL_WINDOWEVENT_EXPOSED','event.type==SDL_EVENT_WINDOW_EXPOSED')
    native=replace(native,'if(vk)SDL_Vulkan_GetDrawableSize(window,&width,&height);else SDL_GL_GetDrawableSize(window,&width,&height);','SDL_GetWindowSizeInPixels(window,&width,&height);')
    native=replace(native,'SDL_DisplayMode mode;SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window),&mode);','const SDL_DisplayMode *mode=SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window));')
    native=native.replace('mode.refresh_rate','mode?mode->refresh_rate:0')
    native=replace(native,'        while(!done) {','''        scale_contract::State mappingState;
        float layoutWidth=-1;
        const auto queryMapping=[&] {
            scale_contract::Mapping m;
            require(SDL_GetWindowSize(window,&m.windowWidth,&m.windowHeight),SDL_GetError());
            require(SDL_GetWindowSizeInPixels(window,&m.bufferWidth,&m.bufferHeight),SDL_GetError());
            m.density=SDL_GetWindowPixelDensity(window);m.displayScale=SDL_GetWindowDisplayScale(window);
            m.zoom=zoom;m.scroll=offset;m.validate();return m;
        };
        while(!done) {''')
    native=replace(native,'            while(SDL_PollEvent(&event)) {','''            while(SDL_PollEvent(&event)) {
                auto eventMapping=queryMapping();
                record({{"stage","toolkit-event"},{"timestampNs",event.common.timestamp},
                    {"event",event.type},{"density",eventMapping.density},
                    {"displayScale",eventMapping.displayScale}});''')
    native=replace(native,'document.select((x-24)/zoom,(y-24-offset)/zoom,event.type==SDL_EVENT_MOUSE_BUTTON_DOWN);','auto p=queryMapping().windowToDocument({x,y});\n                    document.select(p.x,p.y,event.type==SDL_EVENT_MOUSE_BUTTON_DOWN);')
    native=replace(native,'            if(width!=lastW || height!=lastH) {','''            auto mapping=queryMapping();
            if(mappingState.update(mapping)) {
                auto origin=mapping.documentToBuffer({0,0});
                auto unit=mapping.documentToBuffer({1,1});
                record({{"stage","scale-contract"},{"generation",mappingState.generation},
                    {"timestampNs",SDL_GetTicksNS()},{"window",{logicalW,logicalH}},
                    {"buffer",{width,height}},{"surface",{logicalW,logicalH}},
                    {"logical",{mapping.logicalWidth(),mapping.logicalHeight()}},
                    {"density",mapping.density},{"displayScale",mapping.displayScale},
                    {"contentScale",mapping.contentScale()},{"zoom",zoom},{"scroll",offset},
                    {"forward",{unit.x-origin.x,unit.y-origin.y,origin.x,origin.y}},
                    {"inverse",{1/(unit.x-origin.x),1/(unit.y-origin.y),
                        -origin.x/(unit.x-origin.x),-origin.y/(unit.y-origin.y)}}});
            }
            if(layoutWidth!=float(mapping.layoutWidth())) {
                layoutWidth=mapping.layoutWidth();document.layout(layoutWidth);
            }
            if(width!=lastW || height!=lastH) {''')
    start=native.index('            if(mappingState.update(mapping)) {')
    end=native.index('            if(layoutWidth!=',start)
    mapping_record=native[start:end]
    native=native[:start]+native[end:]
    native=replace(native,'            auto *canvas=surface->getCanvas();',
        '            mapping=queryMapping();\n'+mapping_record+'            auto *canvas=surface->getCanvas();')
    native=replace(native,'document.layout((logicalW-48)/zoom);glSurface.reset();','glSurface.reset();')
    native=replace(native,'canvas->scale(float(width)/logicalW,float(height)/logicalH);canvas->translate(24,24+offset);canvas->scale(zoom,zoom);','''auto origin=mapping.documentToBuffer({0,0});
            auto unit=mapping.documentToBuffer({1,1});
            canvas->translate(origin.x,origin.y);canvas->scale(unit.x-origin.x,unit.y-origin.y);''')
    # Both synthetic selection and pointer handling use the same inverse transform.
    native=replace(native,'float wx=24+x*zoom,wy=24+offset+y*zoom;','scale_contract::Mapping mapping;mapping.zoom=zoom;mapping.scroll=offset;\n                    auto window=mapping.documentToWindow({x,y});auto restored=mapping.windowToDocument(window);')
    native=replace(native,'select((wx-24)/zoom,(wy-24-offset)/zoom,true);','select(restored.x,restored.y,true);')
    native=replace(native,'void studyCheck(const std::string& checkpoint,float zoom,float offset)',
        'void studyCheck(const std::string& checkpoint,float zoom,float offset,const scale_contract::Mapping* actual=nullptr)')
    native=replace(native,'scale_contract::Mapping mapping;mapping.zoom=zoom;mapping.scroll=offset;',
        'scale_contract::Mapping mapping=actual?*actual:scale_contract::Mapping{};mapping.zoom=zoom;mapping.scroll=offset;')
    native=replace(native,'document.studyCheck(checkpoint.empty()?"ready":checkpoint,zoom,offset);',
        'document.studyCheck(checkpoint.empty()?"ready":checkpoint,zoom,offset,&mapping);')
    native=native.replace(';lastW=0;', ';')
    native=native.replace('{"stage","capture-mapping"},{"name",name}','{"stage","capture-mapping"},{"generation",mappingState.generation},{"name",name}')
    native=native.replace('{"stage","checkpoint"},{"id",checkpoint}','{"stage","checkpoint"},{"generation",mappingState.generation},{"id",checkpoint}')
    (destination/'native.cpp').write_text(native)
    cmake=(source/'CMakeLists.txt').read_text().replace('IMPORTED_TARGET sdl2 ','IMPORTED_TARGET sdl3=3.4.18 ')
    (destination/'CMakeLists.txt').write_text(cmake)

if __name__=='__main__':generate(Path(sys.argv[1]).resolve())
