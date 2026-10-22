#include <QGuiApplication>
#include <QQuickWindow>
#include <QQuickItem>
#include <QSGTextNode>
#include <QTextLayout>
#include <QTimer>
#include <QDebug>
#include <cstdio>

// Standalone gate: one shared shaping layout, transparent surrounding text,
// and selection foreground emission. No production renderer changes.
class Probe final : public QQuickItem
{
public:
    QTextLayout layout;
    bool selection = false;
    Probe(QQuickWindow &window) : QQuickItem(window.contentItem())
    {
        setFlag(ItemHasContents);
        setSize(window.size());
        QFont font("DejaVu Sans");
        font.setPixelSize(64);
        font.setItalic(true);
        layout.setFont(font);
        layout.setText("fX");
        QTextCharFormat first, second;
        first.setForeground(Qt::black);
        second.setForeground(Qt::transparent);
        // Distinct source formats model an inline-code boundary.
        first.setProperty(QTextFormat::UserProperty, 1);
        layout.setFormats({{0, 1, first}, {1, 1, second}});
        layout.beginLayout();
        auto line = layout.createLine();
        line.setLineWidth(250);
        line.setPosition({20, 20});
        layout.endLayout();
    }
    void select()
    {
        selection = true;
        auto formats = layout.formats();
        for (auto &format : formats) format.format.setForeground(Qt::transparent);
        layout.setFormats(formats);
        layout.beginLayout();
        auto line = layout.createLine();
        line.setLineWidth(250);
        line.setPosition({20, 20});
        layout.endLayout();
        update();
    }
protected:
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override
    {
        delete old;
        auto *node = window()->createTextNode();
        node->setRenderType(QSGTextNode::QtRendering);
        node->setColor(Qt::transparent);
        node->setSelectionColor(Qt::transparent);
        node->setSelectionTextColor(Qt::black);
        node->addTextLayout({}, &layout, selection ? 0 : -1, selection ? 1 : -1);
        return node;
    }
};
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQuickWindow window;
    window.setColor(Qt::white);
    window.resize(300, 140);
    Probe probe(window);
    window.show();
    QTimer::singleShot(200, &app, [&] {
        const auto reference = window.grabWindow();
        if (reference.isNull() || !reference.save("reference.png")) {
            std::fprintf(stderr, "Reference capture failed\n");
            app.exit(2);
            return;
        }
        probe.select();
        QTimer::singleShot(200, &app, [&, reference] {
            const auto selected = window.grabWindow();
            if (selected.isNull() || selected.size() != reference.size()
                || !selected.save("selected.png")) {
                std::fprintf(stderr, "Selection capture failed\n");
                app.exit(2);
                return;
            }
            int differences = 0, lostInk = 0;
            for (int y = 0; y < reference.height(); ++y)
                for (int x = 0; x < reference.width(); ++x) {
                    if (reference.pixel(x, y) != selected.pixel(x, y)) ++differences;
                    if (qGray(reference.pixel(x, y)) < qGray(selected.pixel(x, y))) ++lostInk;
                }
            std::printf("Qt %s DPR %.2f different pixels %d lost ink pixels %d\n",
                        qVersion(), double(window.devicePixelRatio()), differences, lostInk);
            app.exit(differences ? 1 : 0);
        });
    });
    return app.exec();
}
