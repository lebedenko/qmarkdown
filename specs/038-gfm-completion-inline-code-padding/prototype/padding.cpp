// Bounded investigation only; not linked into QMarkdown.
#include <QGuiApplication>
#include <QTextLayout>
#include <QElapsedTimer>
#include <QPainter>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

struct Span { int start, length, identity; };
struct Cell { int source, code; QRectF rect; qreal shift = 0; };
struct Fragment { int code; QRectF rect; };
struct Result { QVector<Cell> cells; QVector<Fragment> fragments; QList<QGlyphRun> runs; qreal height = 0; int lines = 0, passes = 0; };
static void require(bool value, const char *message) {
    if (!value) { fprintf(stderr, "%s\n", message); std::exit(1); }
}
static QVector<Cell> cellsFor(const QTextLayout &layout, const QTextLine &line, const QVector<int> &codes) {
    QVector<Cell> cells;
    for (int pos = line.textStart(); pos < line.textStart() + line.textLength(); ++pos) {
        if (!layout.isValidCursorPosition(pos) || layout.text()[pos] == QChar::LineSeparator) continue;
        const auto a = line.cursorToX(pos, QTextLine::Leading), b = line.cursorToX(pos, QTextLine::Trailing);
        if (a != b) cells.append({pos, codes[pos], QRectF(qMin(a,b), line.y(), std::abs(a-b), line.height())});
    }
    std::sort(cells.begin(), cells.end(), [](const Cell &a, const Cell &b) { return a.rect.left() < b.rect.left(); });
    return cells;
}
static int fragmentsIn(const QVector<Cell> &cells) {
    int result = 0, previous = -1;
    qreal right = -1;
    for (const auto &cell : cells) {
        if (cell.code >= 0 && (cell.code != previous || cell.rect.left() > right + .01)) ++result;
        previous = cell.code; right = cell.rect.right();
    }
    return result;
}
static Result prepare(const QString &text, const QVector<Span> &spans, const QFont &font, qreal width, qreal px, qreal py) {
    QTextLayout layout(text, font);
    layout.setCacheEnabled(true);
    QTextOption option; option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); layout.setTextOption(option);
    QList<QTextLayout::FormatRange> formats;
    for (const auto &span : spans) {
        QTextCharFormat format; format.setFontFixedPitch(true);
        // Force shaping boundaries without changing source positions.
        format.setForeground(QColor::fromRgb(0x123456 + span.identity));
        formats.append({span.start, span.length, format});
    }
    layout.setFormats(formats);
    QVector<int> codes(text.size(), -1);
    for (const auto &span : spans) for (int pos=span.start; pos<span.start+span.length; ++pos) codes[pos]=span.identity;
    Result result;
    layout.beginLayout();
    while (true) {
        auto line = layout.createLine(); if (!line.isValid()) break;
        line.setLineWidth(width);
        int budget = 0;
        QVector<Cell> cells;
        while (true) {
            ++result.passes;
            cells = cellsFor(layout, line, codes);
            const int count = fragmentsIn(cells);
            if (px == 0 || count <= budget) break;
            budget = count;
            line.setLineWidth(qMax(qreal(0), width - 2 * px * budget));
            require(result.passes < text.size() * (spans.size() + 2) + 10, "unbounded layout work");
        }
        const bool code = fragmentsIn(cells) > 0;
        const qreal vertical = code ? py : 0;
        line.setPosition({0, result.height + vertical});
        cells = cellsFor(layout, line, codes);
        qreal shift = 0;
        for (int i = 0; i < cells.size(); ++i) {
            auto &cell = cells[i];
            const bool first = cell.code >= 0 && (i == 0 || cells[i-1].code != cell.code);
            const bool last = cell.code >= 0 && (i+1 == cells.size() || cells[i+1].code != cell.code);
            if (first) shift += px;
            cell.shift = shift; cell.rect.translate(shift, 0);
            if (first) result.fragments.append({cell.code, cell.rect.adjusted(-px, -vertical, px, vertical)});
            else if (cell.code >= 0) result.fragments.last().rect = result.fragments.last().rect.united(cell.rect.adjusted(-px,-vertical,px,vertical));
            if (last) shift += px;
        }
        QVector<qreal> shifts(line.textLength(), 0);
        for (const auto &cell : cells) shifts[cell.source-line.textStart()]=cell.shift;
        auto runs = line.glyphRuns(-1,-1,QTextLayout::RetrieveGlyphIndexes | QTextLayout::RetrieveGlyphPositions | QTextLayout::RetrieveStringIndexes);
        for (auto &run : runs) {
            auto positions = run.positions(); const auto indexes = run.stringIndexes();
            require(positions.size() == indexes.size(), "missing source mapping");
            for (int i = 0; i < positions.size(); ++i) {
                // A cluster can cover several UTF-16 positions; use its valid cursor start.
                int source = int(indexes[i]);
                while (source > line.textStart() && !layout.isValidCursorPosition(source)) --source;
                positions[i].rx() += shifts[source-line.textStart()];
            }
            run.setPositions(positions); result.runs.append(run);
        }
        result.cells += cells;
        result.height += line.height() + 2 * vertical; ++result.lines;
    }
    layout.endLayout();
    require(layout.text() == text, "semantic text changed");
    return result;
}
static int hit(const Result &result, QPointF point) {
    for (const auto &fragment : result.fragments) if (fragment.rect.contains(point)) return fragment.code;
    for (const auto &cell : result.cells) if (cell.rect.contains(point)) return cell.code;
    return -1;
}
static void check(const Result &result, qreal width, qreal px, qreal py) {
    for (int i = 1; i < result.cells.size(); ++i) {
        const auto &a=result.cells[i-1], &b=result.cells[i];
        if (a.rect.top() == b.rect.top()) if (a.code != b.code && a.rect.right() > b.rect.left() + .01) { fprintf(stderr,"cells %d [%g,%g] %d [%g,%g]\n",a.source,double(a.rect.left()),double(a.rect.right()),b.source,double(b.rect.left()),double(b.rect.right())); require(false,"neighbor overlap"); }
    }
    for (const auto &fragment : result.fragments) {
        QRectF glyphs;
        for (const auto &cell : result.cells) if (cell.code == fragment.code && fragment.rect.contains(cell.rect.center())) glyphs = glyphs.isNull() ? cell.rect : glyphs.united(cell.rect);
        require(std::abs(glyphs.left() - fragment.rect.left() - px) < .01, "left padding");
        require(std::abs(fragment.rect.right() - glyphs.right() - px) < .01, "right padding");
        require(std::abs(glyphs.top() - fragment.rect.top() - py) < .01, "vertical padding");
        require(hit(result,{fragment.rect.left()+.01,fragment.rect.center().y()}) == fragment.code, "left padding link hit");
        require(hit(result,{fragment.rect.right()-.01,fragment.rect.center().y()}) == fragment.code, "right padding link hit");
        require(fragment.rect.bottom() <= result.height + .01, "last line overlap");
        // At narrow widths only an indivisible glyph plus its padding may overflow.
        if (fragment.rect.right() > width + .01) {
            int count=0; for (const auto &cell : result.cells) if (fragment.rect.contains(cell.rect.center())) ++count;
            require(count == 1, "avoidable width overflow");
        }
        for (const auto &cell : result.cells) if (cell.code != fragment.code) require(!fragment.rect.intersects(cell.rect), "padding overlaps neighboring text");
    }
}
int main(int argc,char **argv) {
    QGuiApplication app(argc,argv); QFont font("DejaVu Sans"); font.setPixelSize(16);
    struct Case { QString text; QVector<Span> spans; };
    const QList<Case> cases = {
        {QStringLiteral("before code after"),{{7,4,0}}},
        {QStringLiteral("before abcdefghijklmnopqrstuvwxyz after"),{{7,26,0}}},
        {QStringLiteral("left firstsecond right"),{{5,5,0},{10,6,1}}},
        {QString::fromUtf8("left אבג abc אבג right"),{{5,11,0}}},
        {QString::fromUtf8("אבג code abc אבג"),{{4,8,0}}},
        {QString::fromUtf8("left e\u0301🙂👩‍💻 right"),{{5,9,0}}}
    };
    QJsonArray timings; int checked=0;
    for (int size : {12,16,28}) for (qreal width : {1.,32.,80.,220.}) for (qreal px : {0.,4.}) {
        font.setPixelSize(size);
        for (const auto &test : cases) {
            const qreal py=px ? 2 : 0;
            auto result=prepare(test.text,test.spans,font,width,px,py); check(result,width,px,py); ++checked;
            if (px == 0) {
                QTextLayout baseline(test.text,font); QTextOption option; option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere); baseline.setTextOption(option);
                baseline.beginLayout(); int lines=0; qreal height=0;
                while (true) { auto line=baseline.createLine(); if (!line.isValid()) break; line.setLineWidth(width); ++lines; height+=line.height(); }
                baseline.endLayout(); require(lines == result.lines && height == result.height,"zero padding compatibility");
            }
        }
    }
    font.setPixelSize(16);
    for (int copies : {10,100,1000}) {
        QString text; QVector<Span> spans;
        for(int i=0;i<copies;++i) { int pos=text.size(); text+=QStringLiteral("word code more "); spans.append({pos+5,4,i}); }
        for (qreal padding : {0.,4.}) {
            QElapsedTimer timer; timer.start(); Result result;
            for(int n=0;n<5;++n) result=prepare(text,spans,font,240,padding,padding?2:0);
            timings.append(QJsonObject{{"characters",text.size()},{"padding",padding},{"mean_ms",timer.nsecsElapsed()/5.e6},{"lines",result.lines},{"passes",result.passes}});
        }
    }
    const auto imageResult=prepare(cases[3].text,cases[3].spans,font,220,4,2);
    QImage image(240,120,QImage::Format_ARGB32_Premultiplied); image.fill(Qt::white); QPainter painter(&image);
    painter.setPen(Qt::NoPen); painter.setBrush(QColor("#dddddd")); for(const auto &fragment:imageResult.fragments) painter.drawRect(fragment.rect.translated(10,10));
    painter.setPen(Qt::black); for(const auto &run:imageResult.runs) painter.drawGlyphRun({10,10},run); painter.end();
    if (argc>1) require(image.save(QString::fromLocal8Bit(argv[1])), "image save failed");
    const auto output=QJsonDocument(QJsonObject{{"qt",qVersion()},{"checked_cases",checked},{"timings",timings}}).toJson(); fwrite(output.data(),1,size_t(output.size()),stdout);
}
