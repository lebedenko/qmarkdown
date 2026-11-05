// Exercise the actual viewer's prepared geometry without a GPU window.
#define main comparison_entry
#include "qt.cpp"
#undef main

int main(int argc,char **argv) {
    elapsed.start();
    QGuiApplication app(argc,argv);
    if(argc!=2)return 2;
    QFile file(argv[1]);if(!file.open(QIODevice::ReadOnly))return 2;
    const auto manifest=QJsonDocument::fromJson(file.readAll()).object();
    bool passed=true;
    for(const auto &value:manifest["fixtures"].toArray()) {
        auto fixture=value.toObject();
        Window window(manifest,fixture,false,"combined");
        auto blocks=fixture["blocks"].toArray();
        for(int i=0;i<blocks.size();++i) {
            auto &text=window.paragraphs[size_t(i)]->text;
            const auto block=blocks[i].toObject();
            for(const auto &value:block["objects"].toArray()) {
                const auto object=value.toObject();
                const int begin=object["start"].toInt(),end=begin+object["length"].toInt();
                const auto rects=text.rangeRectangles(begin,end);
                bool valid=rects.size()==1;
                int left=-1,right=-1;
                if(valid) {
                    const auto rect=rects.first();
                    left=text.cursorAt(rect.left()+.01,rect.center().y());
                    right=text.cursorAt(rect.right()-.01,rect.center().y());
                    valid=qAbs(rect.width()-object["width"].toDouble())<.05
                        && qMin(left,right)==begin && qMax(left,right)==end;
                }
                passed &= valid;
                record({{"stage","object-selection"},{"fixture",fixture["id"]},
                        {"leftSource",left},{"rightSource",right},{"expectedStart",begin},{"expectedEnd",end},
                        {"copiedSource",left>=0 && right>=0?text.text().mid(qMin(left,right),qAbs(right-left)):QString()},
                        {"passed",valid}});
            }
            for(const auto &value:block["spans"].toArray()) {
                const auto span=value.toObject();
                int begin=span["start"].toInt(),end=begin+span["length"].toInt();
                int low=text.text().size(),high=0;
                auto rects=text.rangeRectangles(begin,end);
                for(auto rect:rects) {
                    int a=text.cursorAt(rect.left()+.01,rect.center().y());
                    int z=text.cursorAt(rect.right()-.01,rect.center().y());
                    low=qMin(low,qMin(a,z));high=qMax(high,qMax(a,z));
                }
                bool valid=!rects.isEmpty() && low==begin && high==end;
                passed &= valid;
                record({{"stage","formatted-selection"},{"fixture",fixture["id"]},
                        {"kind",span["kind"]},{"passed",valid},{"expectedStart",begin},{"expectedEnd",end},
                        {"hitStart",low},{"hitEnd",high},{"copiedSource",text.text().mid(low,high-low)}});
            }
            // All nonempty paragraphs must expose selection geometry spanning
            // the source, including shaped ligatures and bidirectional runs.
            if(!text.text().isEmpty()) {
                const auto rectangles=text.rangeRectangles(0,text.text().size());
                bool valid=!rectangles.isEmpty();
                for(auto rect:rectangles)valid &= rect.isValid();
                passed &= valid;
                record({{"stage","full-source-selection"},{"fixture",fixture["id"]},
                        {"rectangles",rectangles.size()},{"passed",valid}});
            }
        }
    }
    return passed?0:1;
}
