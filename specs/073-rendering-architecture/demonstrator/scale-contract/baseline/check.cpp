#define main viewer_entry
#ifdef STUDY_QT
#include "qt.cpp"
#else
#include "native.cpp"
#endif
#undef main

int main(int argc,char **argv) {
#ifdef STUDY_QT
    QGuiApplication app(argc,argv);
    QFile file(argv[1]);if(!file.open(QIODevice::ReadOnly))return 2;
    auto manifest=QJsonDocument::fromJson(file.readAll()).object();
    for(auto f:manifest["fixtures"].toArray()) {
        Window window(manifest,f.toObject(),false,"combined");
        QJsonArray before,after;
        for(auto &p:window.paragraphs)before.append(DemoAccess::geometry(p->text));
        window.studyCheck(f.toObject()["id"].toString());
        for(auto &p:window.paragraphs)after.append(DemoAccess::geometry(p->text));
        if(before!=after)return 3;
    }
#else
    json manifest;std::ifstream(argv[1])>>manifest;
    for(auto f:manifest["fixtures"]) {
        Document document(manifest,f);document.layout(752);
        auto snapshot=[&] {
            json data=json::array();
            for(auto &b:document.blocks) {
                json runs=json::array();
                for(auto &r:static_cast<ParagraphImpl*>(b.paragraph.get())->runs()) {
                    json positions=json::array(),glyphs=json::array();
                    for(auto p:r.positions())positions.push_back({p.x(),p.y()});
                    for(auto g:r.glyphs())glyphs.push_back(g);
                    runs.push_back({{"positions",positions},{"glyphs",glyphs}});
                }
                data.push_back({{"height",b.paragraph->getHeight()},{"runs",runs}});
            }
            return data;
        };
        auto before=snapshot();document.studyCheck(f["id"],1,0);
        if(before!=snapshot())return 3;
    }
#endif
    return 0;
}
