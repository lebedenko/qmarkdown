#!/usr/bin/env python3
"""Generate isolated viewer overlays; exact replacements fail closed on drift."""
from pathlib import Path

HERE = Path(__file__).resolve().parent


def replace(text, old, new):
    if text.count(old) != 1:
        raise ValueError('overlay anchor changed: ' + old[:80])
    return text.replace(old, new)


def generate(destination):
    destination.mkdir(parents=True, exist_ok=False)
    qt = (HERE.parent/'qt.cpp').read_text()
    qt = replace(qt, '#include <QGuiApplication>', '#include <QGuiApplication>\n#include <QSocketNotifier>\n#include <unistd.h>')
    # Mapping uses the same item transform and cursorAt path as pointer handlers.
    qt = replace(qt, 'protected:\n    void keyPressEvent', '''public:
    void studyCheck(QString checkpoint) {
        bool inverse=true,contained=true;
        QJsonArray geometry;
        for(auto &p:paragraphs) {
            if(p->rule)continue;
            auto g=DemoAccess::geometry(p->text);
            for(auto value:g["cells"].toArray()) {
                auto c=value.toObject();
                QPointF local(c["x"].toDouble()+c["width"].toDouble()/2,
                              c["y"].toDouble()+c["height"].toDouble()/2);
                auto restored=p->mapFromScene(p->mapToScene(local));
                inverse &= QLineF(local,restored).length()<=.0001;
                inverse &= p->text.cursorAt(local.x(),local.y())==
                           p->text.cursorAt(restored.x(),restored.y());
            }
            auto &text=p->text;
            for(const auto &value:text.property("demoObjects").toList()) {
                auto object=value.toMap();
                int begin=object["start"].toInt(),end=begin+object["length"].toInt();
                auto rects=text.rangeRectangles(begin,end);
                bool valid=rects.size()==1;
                if(valid) {
                    auto rect=rects.first();
                    int left=text.cursorAt(rect.left()+.01,rect.center().y());
                    int right=text.cursorAt(rect.right()-.01,rect.center().y());
                    valid=qAbs(rect.width()-object["width"].toDouble())<.05
                        && rect.height()+.05>=object["ascent"].toDouble()+object["descent"].toDouble()
                        && rect.top()>=0 && rect.bottom()<=text.logicalHeight()+.05
                        && qMin(left,right)==begin && qMax(left,right)==end;
                }
                contained &= valid;
            }
            geometry.append(g);
        }
        QString forward,reverse;
        for(auto &p:paragraphs) if(!p->text.text().isEmpty()) {
            selectRange(p->text,0,p->text.text().size());forward=selectedSource();
            selectRange(p->text,p->text.text().size(),0);reverse=selectedSource();
            selectRange(p->text,0,0);break;
        }
        record({{"stage","study-check"},{"checkpoint",checkpoint},{"inverse",inverse},{"contained",contained},
                {"forward",forward},{"reverse",reverse},{"geometry",geometry}});
    }
protected:
    bool event(QEvent *event) override {
        if(event->type()==QEvent::DevicePixelRatioChange)
            record({{"stage","scale-notification"},{"dpr",devicePixelRatio()}});
        return QQuickWindow::event(event);
    }
    void keyPressEvent''')
    qt = replace(qt, 'const bool interactive=args.contains("--interactive"),', 'const bool live=args.contains("--scale-live");\n    const bool interactive=args.contains("--interactive") || live,')
    qt = replace(qt, '    // The receiver is the window:', '''    const auto mapping=[&](QString reason) {
        record({{"stage","mapping"},{"reason",reason},{"output",window.screen()->name()},
                {"window",QJsonArray{window.width(),window.height()}},
                {"dpr",window.devicePixelRatio()},{"zoom",window.zoom},
                {"nominalFontSize",manifest["fontSize"]},
                {"windowId",QString::number(window.winId())}});
    };
    QObject::connect(&window,&QWindow::screenChanged,&window,[&]{mapping("screen-change");});

    QSocketNotifier input(STDIN_FILENO,QSocketNotifier::Read,&window);
    input.setEnabled(live);
    QByteArray commands;
    QObject::connect(&input,&QSocketNotifier::activated,&window,[&]{
        char data[1024];auto n=::read(STDIN_FILENO,data,sizeof(data));
        if(n<=0){input.setEnabled(false);app.quit();return;}
        commands.append(data,n);
        while(commands.contains('\\n')) {
            auto at=commands.indexOf('\\n');auto command=commands.left(at);commands.remove(0,at+1);
            if(command=="quit"){window.counters("shutdown");app.quit();return;}
            if(command.startsWith("checkpoint ")) {
                auto label=QString::fromUtf8(command.mid(11));
                window.motion.stop();window.offset=0;window.zoom=1;window.place();
                mapping(label);window.studyCheck(label);
                QTimer::singleShot(100,&window,[&,label]{
                    auto image=capture("checkpoint-"+label);
                    record({{"stage","checkpoint"},{"id",label},
                        {"buffer",QJsonArray{image.width(),image.height()}}});
                });
            }
        }
    });
    // The receiver is the window:''')
    qt = replace(qt, '            phase=1;\n            if(interactive)return;', '            mapping("ready");window.studyCheck("ready");\n            phase=1;\n            if(interactive)return;')
    qt = replace(qt, '        return image;', '''        record({{"stage","capture-mapping"},{"name",name},
                {"output",window.screen()->name()},{"window",QJsonArray{window.width(),window.height()}},
                {"buffer",QJsonArray{image.width(),image.height()}},
                {"dpr",window.devicePixelRatio()},{"zoom",window.zoom}});
        return image;''')
    (destination/'qt.cpp').write_text(qt)
    native = (HERE.parent/'boundary-experiment/native.cpp').read_text()
    native = replace(native, '#include <SDL.h>', '#include <SDL.h>\n#include <poll.h>\n#include <unistd.h>')
    native = replace(native, '    void copy() {', '''    void studyCheck(const std::string& checkpoint,float zoom,float offset) {
        bool inverse=true,contained=true;json geometry=json::array();
        int oldSelected=selected,oldAnchor=anchor,oldSelection=selection;
        for(auto &b:blocks) {
            json boxes=json::array();
            for(auto box:b.paragraph->getRectsForPlaceholders()) {
                contained &= box.rect.top()>=-.05 && box.rect.bottom()<=b.paragraph->getHeight()+.05;
                boxes.push_back(rect(box.rect));
            }
            for(size_t pos=0;pos+1<b.mapping.size();++pos) {
                for(auto box:b.paragraph->getRectsForRange(pos,pos+1,RectHeightStyle::kTight,RectWidthStyle::kTight)) {
                    float x=box.rect.centerX(),y=box.rect.centerY()+b.y;
                    float wx=24+x*zoom,wy=24+offset+y*zoom;
                    auto direct=b.paragraph->getGlyphPositionAtCoordinate(x,y-b.y).position;
                    select((wx-24)/zoom,(wy-24-offset)/zoom,true);
                    inverse &= selected>=0 && anchor==int(direct);
                }
            }
            geometry.push_back({{"height",b.paragraph->getHeight()},{"objects",boxes}});
        }
        std::string forward,reverse;
        if(!blocks.empty()) {
            selected=0;anchor=0;selection=int(blocks[0].mapping.size())-1;forward=selectedSource();
            std::swap(anchor,selection);reverse=selectedSource();
        }
        selected=oldSelected;anchor=oldAnchor;selection=oldSelection;
        record({{"stage","study-check"},{"checkpoint",checkpoint},{"inverse",inverse},
                {"contained",contained},{"forward",forward},{"reverse",reverse},{"geometry",geometry}});
    }
    void copy() {''')
    native = replace(native, '        const bool vk=std::string(argv[3])=="vulkan",interactive=argc>5 && std::string(argv[5])=="--interactive";', '''        const bool live=argc>5 && std::string(argv[5])=="--scale-live";
        const bool vk=std::string(argv[3])=="vulkan",interactive=live || (argc>5 && std::string(argv[5])=="--interactive");''')
    native = replace(native, '        while(!done) {', '''        std::string commands,checkpoint;
        int settle=0;int previousDisplay=-2,previousLW=-1,previousLH=-1;
        while(!done) {
            if(live) {
                pollfd input{STDIN_FILENO,POLLIN,0};
                if(poll(&input,1,0)>0) {
                    char data[1024];auto n=read(STDIN_FILENO,data,sizeof(data));
                    if(n<=0){done=true;continue;}commands.append(data,n);
                    auto at=commands.find('\\n');
                    if(at!=std::string::npos) {
                        auto command=commands.substr(0,at);commands.erase(0,at+1);
                        if(command=="quit"){done=true;continue;}
                        if(command.starts_with("checkpoint ")) {
                            checkpoint=command.substr(11);settle=3;offset=0;zoom=1;moving=false;
                        }
                    }
                }
            }''')
    native = replace(native, '            sk_sp<SkSurface> surface;', '''            int display=SDL_GetWindowDisplayIndex(window);
            if(logicalW!=previousLW || logicalH!=previousLH || display!=previousDisplay) {
                record({{"stage","mapping-change"},{"window",{logicalW,logicalH}},
                    {"buffer",{width,height}},{"display",display}});
                previousLW=logicalW;previousLH=logicalH;previousDisplay=display;
            }
            sk_sp<SkSurface> surface;''')
    native = replace(native, '            if(frame==0) {', '''            if(frame==0 || (!checkpoint.empty() && --settle==0)) {
                record({{"stage","mapping"},{"reason",checkpoint.empty()?"ready":checkpoint},
                    {"display",SDL_GetWindowDisplayIndex(window)},
                    {"displayName",SDL_GetDisplayName(SDL_GetWindowDisplayIndex(window))},
                    {"window",{logicalW,logicalH}},{"buffer",{width,height}},
                    {"ratio",{double(width)/logicalW,double(height)/logicalH}},
                    {"zoom",zoom},{"nominalFontSize",manifest["fontSize"]},
                    {"windowId",SDL_GetWindowID(window)}});
                document.studyCheck(checkpoint.empty()?"ready":checkpoint,zoom,offset);
                if(!checkpoint.empty()) {
                    auto image=surface->makeImageSnapshot();
                    auto png=SkPngEncoder::Encode(vk?vulkan->directContext():context.get(),image.get(),{});
                    require(bool(png),"checkpoint PNG encoding");
                    SkFILEWStream out((std::string(argv[4])+"/checkpoint-"+checkpoint+".png").c_str());
                    require(out.isValid() && out.write(png->data(),png->size()),"checkpoint PNG write");
                    record({{"stage","checkpoint"},{"id",checkpoint},{"buffer",{width,height}}});
                    checkpoint.clear();
                }
            }
            if(frame==0) {''')
    native = replace(native, '                record({{"stage","capture"},{"name",name}});', '''                record({{"stage","capture"},{"name",name}});
                record({{"stage","capture-mapping"},{"name",name},{"window",{logicalW,logicalH}},
                    {"buffer",{width,height}},{"ratio",{double(width)/logicalW,double(height)/logicalH}},{"zoom",zoom}});''')
    (destination/'native.cpp').write_text(native)
    (destination/'layout.h').write_bytes((HERE.parent/'boundary-experiment/layout.h').read_bytes())


if __name__ == '__main__':
    import sys
    generate(Path(sys.argv[1]))
