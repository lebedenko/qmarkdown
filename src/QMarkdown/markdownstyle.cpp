#include "markdownstyle.h"
#include <QGuiApplication>
#include <QFontDatabase>
#include <cmath>

HeadingStyle::HeadingStyle(MarkdownStyle *style, const QString &prefix)
    : QObject(style), m_style(style), m_prefix(prefix) {}
QFont HeadingStyle::font() const { return m_style->value(m_prefix + "font").value<QFont>(); }
void HeadingStyle::setFont(const QFont &value) { m_style->setValue(m_prefix + "font", QVariant::fromValue(value)); }
void HeadingStyle::resetFont() { m_style->resetValue(m_prefix + "font"); }
QColor HeadingStyle::color() const { return m_style->value(m_prefix + "color").value<QColor>(); }
void HeadingStyle::setColor(const QColor &value) { m_style->setValue(m_prefix + "color", QVariant::fromValue(value)); }
void HeadingStyle::resetColor() { m_style->resetValue(m_prefix + "color"); }
bool HeadingStyle::separatorVisible() const { return m_style->value(m_prefix + "separatorVisible").value<bool>(); }
void HeadingStyle::setSeparatorVisible(bool value) { m_style->setValue(m_prefix + "separatorVisible", QVariant::fromValue(value)); }
void HeadingStyle::resetSeparatorVisible() { m_style->resetValue(m_prefix + "separatorVisible"); }
QColor HeadingStyle::separatorColor() const { return m_style->value(m_prefix + "separatorColor").value<QColor>(); }
void HeadingStyle::setSeparatorColor(const QColor &value) { m_style->setValue(m_prefix + "separatorColor", QVariant::fromValue(value)); }
void HeadingStyle::resetSeparatorColor() { m_style->resetValue(m_prefix + "separatorColor"); }
qreal HeadingStyle::separatorThickness() const { return m_style->value(m_prefix + "separatorThickness").value<qreal>(); }
void HeadingStyle::setSeparatorThickness(qreal value) { m_style->setValue(m_prefix + "separatorThickness", QVariant::fromValue(value)); }
void HeadingStyle::resetSeparatorThickness() { m_style->resetValue(m_prefix + "separatorThickness"); }
qreal HeadingStyle::separatorSpacing() const { return m_style->value(m_prefix + "separatorSpacing").value<qreal>(); }
void HeadingStyle::setSeparatorSpacing(qreal value) { m_style->setValue(m_prefix + "separatorSpacing", QVariant::fromValue(value)); }
void HeadingStyle::resetSeparatorSpacing() { m_style->resetValue(m_prefix + "separatorSpacing"); }

InlineCodeStyle::InlineCodeStyle(MarkdownStyle *style, const QString &prefix)
    : QObject(style), m_style(style), m_prefix(prefix) {}
QFont InlineCodeStyle::font() const { return m_style->value(m_prefix + "font").value<QFont>(); }
void InlineCodeStyle::setFont(const QFont &value) { m_style->setValue(m_prefix + "font", QVariant::fromValue(value)); }
void InlineCodeStyle::resetFont() { m_style->resetValue(m_prefix + "font"); }
QColor InlineCodeStyle::foreground() const { return m_style->value(m_prefix + "foreground").value<QColor>(); }
void InlineCodeStyle::setForeground(const QColor &value) { m_style->setValue(m_prefix + "foreground", QVariant::fromValue(value)); }
void InlineCodeStyle::resetForeground() { m_style->resetValue(m_prefix + "foreground"); }
QColor InlineCodeStyle::backgroundColor() const { return m_style->value(m_prefix + "backgroundColor").value<QColor>(); }
void InlineCodeStyle::setBackgroundColor(const QColor &value) { m_style->setValue(m_prefix + "backgroundColor", QVariant::fromValue(value)); }
void InlineCodeStyle::resetBackgroundColor() { m_style->resetValue(m_prefix + "backgroundColor"); }
qreal InlineCodeStyle::cornerRadius() const { return m_style->value(m_prefix + "cornerRadius").value<qreal>(); }
void InlineCodeStyle::setCornerRadius(qreal value) { m_style->setValue(m_prefix + "cornerRadius", QVariant::fromValue(value)); }
void InlineCodeStyle::resetCornerRadius() { m_style->resetValue(m_prefix + "cornerRadius"); }

CodeBlockStyle::CodeBlockStyle(MarkdownStyle *style, const QString &prefix)
    : QObject(style), m_style(style), m_prefix(prefix) {}
QFont CodeBlockStyle::font() const { return m_style->value(m_prefix + "font").value<QFont>(); }
void CodeBlockStyle::setFont(const QFont &value) { m_style->setValue(m_prefix + "font", QVariant::fromValue(value)); }
void CodeBlockStyle::resetFont() { m_style->resetValue(m_prefix + "font"); }
QColor CodeBlockStyle::color() const { return m_style->value(m_prefix + "color").value<QColor>(); }
void CodeBlockStyle::setColor(const QColor &value) { m_style->setValue(m_prefix + "color", QVariant::fromValue(value)); }
void CodeBlockStyle::resetColor() { m_style->resetValue(m_prefix + "color"); }
QColor CodeBlockStyle::backgroundColor() const { return m_style->value(m_prefix + "backgroundColor").value<QColor>(); }
void CodeBlockStyle::setBackgroundColor(const QColor &value) { m_style->setValue(m_prefix + "backgroundColor", QVariant::fromValue(value)); }
void CodeBlockStyle::resetBackgroundColor() { m_style->resetValue(m_prefix + "backgroundColor"); }
QColor CodeBlockStyle::borderColor() const { return m_style->value(m_prefix + "borderColor").value<QColor>(); }
void CodeBlockStyle::setBorderColor(const QColor &value) { m_style->setValue(m_prefix + "borderColor", QVariant::fromValue(value)); }
void CodeBlockStyle::resetBorderColor() { m_style->resetValue(m_prefix + "borderColor"); }
qreal CodeBlockStyle::borderWidth() const { return m_style->value(m_prefix + "borderWidth").value<qreal>(); }
void CodeBlockStyle::setBorderWidth(qreal value) { m_style->setValue(m_prefix + "borderWidth", QVariant::fromValue(value)); }
void CodeBlockStyle::resetBorderWidth() { m_style->resetValue(m_prefix + "borderWidth"); }
qreal CodeBlockStyle::cornerRadius() const { return m_style->value(m_prefix + "cornerRadius").value<qreal>(); }
void CodeBlockStyle::setCornerRadius(qreal value) { m_style->setValue(m_prefix + "cornerRadius", QVariant::fromValue(value)); }
void CodeBlockStyle::resetCornerRadius() { m_style->resetValue(m_prefix + "cornerRadius"); }
qreal CodeBlockStyle::horizontalPadding() const { return m_style->value(m_prefix + "horizontalPadding").value<qreal>(); }
void CodeBlockStyle::setHorizontalPadding(qreal value) { m_style->setValue(m_prefix + "horizontalPadding", QVariant::fromValue(value)); }
void CodeBlockStyle::resetHorizontalPadding() { m_style->resetValue(m_prefix + "horizontalPadding"); }
qreal CodeBlockStyle::verticalPadding() const { return m_style->value(m_prefix + "verticalPadding").value<qreal>(); }
void CodeBlockStyle::setVerticalPadding(qreal value) { m_style->setValue(m_prefix + "verticalPadding", QVariant::fromValue(value)); }
void CodeBlockStyle::resetVerticalPadding() { m_style->resetValue(m_prefix + "verticalPadding"); }

HeadingStyles::HeadingStyles(MarkdownStyle *style) : QObject(style)
{
    m_h1 = new HeadingStyle(style, "h1.");
    m_h2 = new HeadingStyle(style, "h2.");
    m_h3 = new HeadingStyle(style, "h3.");
    m_h4 = new HeadingStyle(style, "h4.");
    m_h5 = new HeadingStyle(style, "h5.");
    m_h6 = new HeadingStyle(style, "h6.");
}

void MarkdownStyle::setLinkColor(const QColor &value) { setValue("linkColor", QVariant::fromValue(value)); }
void MarkdownStyle::setLinkUnderline(bool value) { setValue("linkUnderline", QVariant::fromValue(value)); }
void MarkdownStyle::setThematicBreakColor(const QColor &value) { setValue("thematicBreakColor", QVariant::fromValue(value)); }
void MarkdownStyle::setThematicBreakThickness(qreal value) { setValue("thematicBreakThickness", QVariant::fromValue(value)); }
void MarkdownStyle::setCodeBlockFont(const QFont &value) { setValue("codeBlock.font", QVariant::fromValue(value)); }
void MarkdownStyle::setCodeBlockColor(const QColor &value) { setValue("codeBlock.color", QVariant::fromValue(value)); }
void MarkdownStyle::setInlineCodeFont(const QFont &value) { setValue("inlineCode.font", QVariant::fromValue(value)); }
void MarkdownStyle::setBodyFont(const QFont &value) { setValue("bodyFont", QVariant::fromValue(value)); }
void MarkdownStyle::setBodyColor(const QColor &value) { setValue("bodyColor", QVariant::fromValue(value)); }
void MarkdownStyle::setH1Font(const QFont &value) { setValue("h1.font", QVariant::fromValue(value)); }
void MarkdownStyle::setH1Color(const QColor &value) { setValue("h1.color", QVariant::fromValue(value)); }
void MarkdownStyle::setH2Font(const QFont &value) { setValue("h2.font", QVariant::fromValue(value)); }
void MarkdownStyle::setH2Color(const QColor &value) { setValue("h2.color", QVariant::fromValue(value)); }
void MarkdownStyle::setH3Font(const QFont &value) { setValue("h3.font", QVariant::fromValue(value)); }
void MarkdownStyle::setH3Color(const QColor &value) { setValue("h3.color", QVariant::fromValue(value)); }
void MarkdownStyle::setH4Font(const QFont &value) { setValue("h4.font", QVariant::fromValue(value)); }
void MarkdownStyle::setH4Color(const QColor &value) { setValue("h4.color", QVariant::fromValue(value)); }
void MarkdownStyle::setH5Font(const QFont &value) { setValue("h5.font", QVariant::fromValue(value)); }
void MarkdownStyle::setH5Color(const QColor &value) { setValue("h5.color", QVariant::fromValue(value)); }
void MarkdownStyle::setH6Font(const QFont &value) { setValue("h6.font", QVariant::fromValue(value)); }
void MarkdownStyle::setH6Color(const QColor &value) { setValue("h6.color", QVariant::fromValue(value)); }
void MarkdownStyle::setBlockSpacing(qreal value) { setValue("blockSpacing", QVariant::fromValue(value)); }
void MarkdownStyle::setListIndent(qreal value) { setValue("listIndent", QVariant::fromValue(value)); }
void MarkdownStyle::setQuoteIndent(qreal value) { setValue("quoteIndent", QVariant::fromValue(value)); }
void MarkdownStyle::setQuoteRuleColor(const QColor &value) { setValue("quoteRuleColor", QVariant::fromValue(value)); }
void MarkdownStyle::setQuoteRuleThickness(qreal value) { setValue("quoteRuleThickness", QVariant::fromValue(value)); }

MarkdownStyle::MarkdownStyle(QObject *parent) : QObject(parent)
{
    m_headings = new HeadingStyles(this);
    m_inlineCode = new InlineCodeStyle(this, "inlineCode.");
    m_codeBlock = new CodeBlockStyle(this, "codeBlock.");
    QFont body = QGuiApplication::font();
    body.setWeight(QFont::Normal);
    QFont code = body;
    code.setFamily(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
    QFont inlineFont;
    inlineFont.setFamily(code.family());
    m_defaults = {{"bodyFont", body}, {"bodyColor", QColor("#202020")},
                  {"linkColor", QColor("#0066cc")}, {"linkUnderline", true},
                  {"thematicBreakColor", QColor("#202020")}, {"thematicBreakThickness", 1.0},
                  {"blockSpacing", 8.0}, {"listIndent", 24.0}, {"quoteIndent", 16.0},
                  {"quoteRuleColor", QColor("#808080")}, {"quoteRuleThickness", 2.0},
                  {"codeBlock.font", code}, {"codeBlock.color", QColor("#202020")},
                  {"inlineCode.font", inlineFont}, {"inlineCode.foreground", QColor()}};
    const qreal proportions[] = {2, 1.75, 1.5, 1.25, 1.125, 1};
    for (int i = 1; i <= 6; ++i) {
        const QString prefix = QString("h%1.").arg(i);
        QFont font = body;
        if (font.pixelSize() > 0) font.setPixelSize(qMax(1, qRound(font.pixelSize() * proportions[i - 1])));
        else font.setPointSizeF(font.pointSizeF() * proportions[i - 1]);
        font.setWeight(QFont::Bold);
        m_defaults.insert(prefix + "font", font);
        m_defaults.insert(prefix + "color", QColor("#202020"));
        m_defaults.insert(prefix + "separatorVisible", false);
        m_defaults.insert(prefix + "separatorColor", QColor(Qt::transparent));
        m_defaults.insert(prefix + "separatorThickness", 0.0);
        m_defaults.insert(prefix + "separatorSpacing", 0.0);
    }
    for (const QString &prefix : {QString("inlineCode."), QString("codeBlock.")}) {
        m_defaults.insert(prefix + "backgroundColor", QColor(Qt::transparent));
        m_defaults.insert(prefix + "cornerRadius", 0.0);
    }
    m_defaults.insert("codeBlock.borderColor", QColor(Qt::transparent));
    for (const QString &field : {QString("borderWidth"), QString("horizontalPadding"), QString("verticalPadding")})
        m_defaults.insert("codeBlock." + field, 0.0);
    restoreDefaults();
}

void MarkdownStyle::setValue(const QString &key, const QVariant &value, bool explicitOverride)
{
    if (explicitOverride) m_overrides.insert(key);
    const auto current = m_values.value(key);
    bool equal = current == value;
    if (value.metaType() == QMetaType::fromType<QFont>())
        equal = equal && current.value<QFont>().resolveMask() == value.value<QFont>().resolveMask();
    if (value.metaType() == QMetaType::fromType<qreal>())
        equal = equal || (std::isnan(current.toDouble()) && std::isnan(value.toDouble()));
    if (equal) return;
    m_values.insert(key, value);
    if (key == "linkColor") emit linkColorChanged();
    if (key == "linkUnderline") emit linkUnderlineChanged();
    if (key == "thematicBreakColor") emit thematicBreakColorChanged();
    if (key == "thematicBreakThickness") emit thematicBreakThicknessChanged();
    if (key == "codeBlock.font") emit codeBlockFontChanged();
    if (key == "codeBlock.color") emit codeBlockColorChanged();
    if (key == "inlineCode.font") emit inlineCodeFontChanged();
    if (key == "bodyFont") emit bodyFontChanged();
    if (key == "bodyColor") emit bodyColorChanged();
    if (key == "h1.font") emit h1FontChanged();
    if (key == "h1.color") emit h1ColorChanged();
    if (key == "h2.font") emit h2FontChanged();
    if (key == "h2.color") emit h2ColorChanged();
    if (key == "h3.font") emit h3FontChanged();
    if (key == "h3.color") emit h3ColorChanged();
    if (key == "h4.font") emit h4FontChanged();
    if (key == "h4.color") emit h4ColorChanged();
    if (key == "h5.font") emit h5FontChanged();
    if (key == "h5.color") emit h5ColorChanged();
    if (key == "h6.font") emit h6FontChanged();
    if (key == "h6.color") emit h6ColorChanged();
    if (key == "blockSpacing") emit blockSpacingChanged();
    if (key == "listIndent") emit listIndentChanged();
    if (key == "quoteIndent") emit quoteIndentChanged();
    if (key == "quoteRuleColor") emit quoteRuleColorChanged();
    if (key == "quoteRuleThickness") emit quoteRuleThicknessChanged();
    const auto dot = key.indexOf('.');
    if (dot < 0) return;
    QObject *group = nullptr;
    const auto prefix = key.left(dot);
    if (prefix == "inlineCode") group = m_inlineCode;
    else if (prefix == "codeBlock") group = m_codeBlock;
    else if (prefix.startsWith('h')) group = m_headings->property(prefix.toUtf8()).value<QObject *>();
    if (group) QMetaObject::invokeMethod(group, (key.mid(dot + 1) + "Changed").toUtf8().constData(), Qt::DirectConnection);
}

void MarkdownStyle::setPreset(Preset value)
{
    if (value < Legacy || value > MinimalDark || m_preset == value) return;
    m_preset = value;
    applyPreset();
    emit presetChanged();
}

void MarkdownStyle::resetValue(const QString &key)
{
    m_overrides.remove(key);
    applyPreset();
}

void MarkdownStyle::resetOverrides()
{
    m_overrides.clear();
    applyPreset();
}

void MarkdownStyle::restoreDefaults()
{
    const bool changed = m_preset != Legacy;
    m_preset = Legacy;
    resetOverrides();
    if (changed) emit presetChanged();
}

void MarkdownStyle::applyPreset()
{
    auto values = m_defaults;
    const bool polished = m_preset == PolishedLight || m_preset == PolishedDark;
    const bool dark = m_preset == PolishedDark || m_preset == MinimalDark;
    if (m_preset != Legacy) {
        const QColor text(dark ? "#e6e6e6" : "#202020");
        const QColor rule(dark ? "#454b54" : "#d0d7de");
        values["bodyColor"] = text;
        values["codeBlock.color"] = text;
        values["linkColor"] = QColor(dark ? "#8db9f2" : "#0066cc");
        values["thematicBreakColor"] = rule;
        values["codeBlock.borderColor"] = rule;
        values["quoteRuleColor"] = QColor(dark ? "#a0a0a0" : "#808080");
        for (int i = 1; i <= 6; ++i) {
            values[QString("h%1.color").arg(i)] = text;
            values[QString("h%1.separatorColor").arg(i)] = rule;
        }
        if (polished) {
            values["blockSpacing"] = 12.0;
            for (int i = 1; i <= 2; ++i) {
                const QString prefix = QString("h%1.").arg(i);
                values[prefix + "separatorVisible"] = true;
                values[prefix + "separatorColor"] = rule;
                values[prefix + "separatorThickness"] = 1.0;
                values[prefix + "separatorSpacing"] = 8.0;
            }
            const QColor background(dark ? "#30343b" : "#eff1f3");
            values["inlineCode.backgroundColor"] = background;
            values["inlineCode.cornerRadius"] = 4.0;
            values["codeBlock.backgroundColor"] = background;
            values["codeBlock.borderColor"] = rule;
            values["codeBlock.borderWidth"] = 1.0;
            values["codeBlock.cornerRadius"] = 6.0;
            values["codeBlock.horizontalPadding"] = 12.0;
            values["codeBlock.verticalPadding"] = 12.0;
        }
    }
    for (auto it = values.cbegin(); it != values.cend(); ++it)
        if (!m_overrides.contains(it.key())) setValue(it.key(), it.value(), false);
}
