#include "mathitem.h"
#include <QCache>
#include <QPainter>
#include <QCoreApplication>
#include <QDataStream>
#include <QPointer>
#include <QThreadPool>
#include <cmath>
#ifdef QMARKDOWN_WITH_MATH
#include "latex.h"
#include "core/formula.h"
#include "platform/qt/graphic_qt.h"
#include "qmarkdown_budget.h"
#endif

namespace QMarkdownPrivate {
bool mathAvailable() {
#ifdef QMARKDOWN_WITH_MATH
    return true;
#else
    return false;
#endif
}
namespace {
// A deliberately small rendering dialect. Reject mutation, expansion, resource
// commands and numeric dimensions before the backend can interpret them.
bool permitted(const QString &source) {
    if (source.toUtf8().size() > 16384 || source.contains(QChar(0))) return false;
    static const QStringList commands = QStringLiteral(
        "frac dfrac tfrac sqrt alpha beta gamma delta epsilon varepsilon zeta eta theta vartheta iota kappa lambda mu nu xi pi varpi rho varrho sigma varsigma tau upsilon phi varphi chi psi omega Gamma Delta Theta Lambda Xi Pi Sigma Upsilon Phi Psi Omega "
        "sum prod int iint iiint oint infty partial nabla pm mp times div cdot le leq ge geq ne neq approx equiv sim simeq to rightarrow leftarrow leftrightarrow Rightarrow Leftarrow Leftrightarrow in notin subset supset subseteq supseteq cup cap emptyset forall exists neg land lor "
        "left right middle big Big bigg Bigg langle rangle lvert rvert lVert rVert vert Vert lbrace rbrace ldots cdots vdots ddots "
        "sin cos tan cot sec csc arcsin arccos arctan sinh cosh tanh log ln exp lim min max det gcd "
        "mathrm mathit mathbf mathsf mathtt mathcal mathbb mathfrak text operatorname overline underline hat widehat bar vec dot ddot tilde widetilde overbrace underbrace limits nolimits begin end quad qquad").split(u' ');
    static const QStringList environments = {QStringLiteral("matrix"), QStringLiteral("pmatrix"), QStringLiteral("bmatrix"), QStringLiteral("Bmatrix"), QStringLiteral("vmatrix"), QStringLiteral("Vmatrix"), QStringLiteral("aligned")};
    QStringList stack;
    int braces = 0;
    for (qsizetype i = 0; i < source.size(); ++i) {
        const auto c = source.at(i);
        if (c == u'{') { if (++braces > 64) return false; }
        else if (c == u'}') { if (--braces < 0) return false; }
        else if (c == u'%' || c == u'$' || c.unicode() > 127) return false;
        else if (c == u'\\') {
            if (++i == source.size()) return false;
            const auto start = i;
            while (i < source.size() && source.at(i).isLetter()) ++i;
            const auto command = source.mid(start, i - start);
            if (command.isEmpty()) {
                if (!QStringLiteral("\\{}|,;:! ").contains(source.at(i))) return false;
            } else {
                if (!commands.contains(command)) return false;
                if (command == QStringLiteral("begin") || command == QStringLiteral("end")) {
                    auto j = i;
                    while (j < source.size() && source.at(j).isSpace()) ++j;
                    if (j == source.size() || source.at(j) != u'{') return false;
                    const auto close = source.indexOf(u'}', j);
                    if (close < 0) return false;
                    const auto env = source.mid(j + 1, close - j - 1);
                    if (!environments.contains(env)) return false;
                    if (command == QStringLiteral("begin")) { if (stack.size() >= 32) return false; stack.append(env); }
                    else if (stack.isEmpty() || stack.takeLast() != env) return false;
                }
                --i;
            }
        }
    }
    return braces == 0 && stack.isEmpty();
}
class MathService : public QObject {
public:
    QThreadPool pool;
    int pending = 0; // GUI thread only
    // Worker thread only: bounded by byte cost and entry count.
    QCache<QByteArray, MathRecording> cache{16 * 1024 * 1024};
    bool fonts = false;
    explicit MathService(QObject *parent) : QObject(parent) {
        pool.setMaxThreadCount(1);
#ifdef QMARKDOWN_WITH_MATH
        fonts = tex::Font_qt::registerFonts();
#endif
    }
    ~MathService() override { pool.clear(); pool.waitForDone(); }
};
MathService *service() {
    static QPointer<MathService> instance;
    if (!instance && QCoreApplication::instance()) instance = new MathService(QCoreApplication::instance());
    return instance;
}
std::shared_ptr<const MathRecording> render(MathService *scheduler, const QString &source, qreal size, QColor color) {
#ifdef QMARKDOWN_WITH_MATH
    QByteArray key;
    QDataStream stream(&key, QIODevice::WriteOnly);
    stream << source << double(size) << color;
    if (auto cached = scheduler->cache.object(key)) return std::make_shared<MathRecording>(*cached);
    try {
        // Initialization is worker-owned and input-independent.
        static const bool initialized = [] { tex::LaTeX::init(); return true; }();
        Q_UNUSED(initialized)
        struct Budget {
            Budget() { tex::qmarkdownOperations = 100000; tex::qmarkdownAllocations = 4096; }
            ~Budget() { tex::qmarkdownOperations = tex::qmarkdownAllocations = -1; }
        } budget;
        // Formula caches hold mutable atom trees; never share them between inputs.
        struct FormulaState {
            ~FormulaState() { tex::Formula::_predefinedTeXFormulas.clear(); }
        } formulaState;
        tex::Formula::_predefinedTeXFormulas.clear();
        tex::Formula formula(source.toStdWString());
        tex::TeXRenderBuilder builder;
        std::unique_ptr<tex::TeXRender> result(builder.setStyle(tex::TexStyle::display).setTextSize(float(size)).setForeground(color.rgba()).build(formula));
        const QSizeF bounds(result->getWidth(), result->getHeight());
        if (!std::isfinite(bounds.width()) || !std::isfinite(bounds.height()) || bounds.width() <= 0 || bounds.height() <= 0 || bounds.width() > 16384 || bounds.height() > 16384) return {};
        auto recording = std::make_shared<MathRecording>();
        recording->size = bounds;
        QPainter painter(&recording->picture);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        tex::Graphics2D_qt graphics(&painter);
        result->draw(graphics, 0, 0);
        painter.end();
        const auto bytes = recording->picture.size();
        if (bytes > 1024 * 1024) return {};
        if (scheduler->cache.size() >= 64) scheduler->cache.clear();
        scheduler->cache.insert(key, new MathRecording(*recording), int(bytes + key.size()));
        return recording;
    } catch (const std::exception &) { return {}; }
#else
    Q_UNUSED(scheduler) Q_UNUSED(source) Q_UNUSED(size) Q_UNUSED(color)
    return {};
#endif
}
}
MathItem::MathItem(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true);
    setSmooth(false);
    connect(this, &MathItem::inputChanged, this, &MathItem::refresh);
}
MathItem::~MathItem() { if (m_cancelled) *m_cancelled = true; }
void MathItem::setSource(const QString &v) { if (m_source != v) { m_source = v; emit inputChanged(); } }
void MathItem::setColor(const QColor &v) { if (m_color != v) { m_color = v; emit inputChanged(); } }
void MathItem::setTextSize(qreal v) { if (m_size != v) { m_size = v; emit inputChanged(); } }
void MathItem::setMathEnabled(bool v) { if (m_enabled != v) { m_enabled = v; emit inputChanged(); } }
void MathItem::refresh() {
    if (m_cancelled) *m_cancelled = true;
    const auto generation = ++m_generation;
    m_pending = false;
    m_recording.reset(); setImplicitSize(0, 0); update(); emit resultChanged();
    if (!m_enabled || !mathAvailable() || !permitted(m_source) || !std::isfinite(m_size) || m_size <= 0 || m_size > 512) return;
    auto scheduler = service();
    if (!scheduler || !scheduler->fonts || scheduler->pending >= 64) return;
    ++scheduler->pending; m_pending = true; emit resultChanged();
    auto cancelled = m_cancelled = std::make_shared<std::atomic_bool>(false);
    const auto source = m_source; const auto size = m_size; const auto color = m_color;
    const QPointer<MathItem> target(this);
    scheduler->pool.start([scheduler, target, source, size, color, cancelled, generation] {
        auto recording = cancelled->load() ? nullptr : render(scheduler, source, size, color);
        QMetaObject::invokeMethod(scheduler, [scheduler, target, cancelled, generation, recording] {
            --scheduler->pending;
            if (cancelled->load() || !target || target->m_generation != generation) return;
            target->m_pending = false;
            target->m_recording = recording;
            if (recording) target->setImplicitSize(recording->size.width(), recording->size.height());
            target->update(); emit target->resultChanged();
        }, Qt::QueuedConnection);
    });
}
void MathItem::paint(QPainter *painter) {
    if (m_recording) painter->drawPicture(QPointF(0, 0), m_recording->picture);
}
}
