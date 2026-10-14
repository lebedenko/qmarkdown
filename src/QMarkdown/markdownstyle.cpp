#include "markdownstyle.h"
#include <QGuiApplication>
#include <QFontDatabase>
#include <cmath>
#include <type_traits>

HeadingStyle::HeadingStyle(MarkdownStyle *style, int index)
    : QObject(style), m_style(style), m_index(index) {}
QFont HeadingStyle::font() const { return m_style->m_values.headings[m_index].font; }
void HeadingStyle::setFont(const QFont &value) { m_style->update(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1Font), m_style->m_values.headings[m_index].font, value); }
void HeadingStyle::resetFont() { m_style->reset(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1Font)); }
QColor HeadingStyle::color() const { return m_style->m_values.headings[m_index].color; }
void HeadingStyle::setColor(const QColor &value) { m_style->update(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1Color), m_style->m_values.headings[m_index].color, value); }
void HeadingStyle::resetColor() { m_style->reset(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1Color)); }
bool HeadingStyle::separatorVisible() const { return m_style->m_values.headings[m_index].separatorVisible; }
void HeadingStyle::setSeparatorVisible(bool value) { m_style->update(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1SeparatorVisible), m_style->m_values.headings[m_index].separatorVisible, value); }
void HeadingStyle::resetSeparatorVisible() { m_style->reset(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1SeparatorVisible)); }
QColor HeadingStyle::separatorColor() const { return m_style->m_values.headings[m_index].separatorColor; }
void HeadingStyle::setSeparatorColor(const QColor &value) { m_style->update(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1SeparatorColor), m_style->m_values.headings[m_index].separatorColor, value); }
void HeadingStyle::resetSeparatorColor() { m_style->reset(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1SeparatorColor)); }
qreal HeadingStyle::separatorThickness() const { return m_style->m_values.headings[m_index].separatorThickness; }
void HeadingStyle::setSeparatorThickness(qreal value) { m_style->update(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1SeparatorThickness), m_style->m_values.headings[m_index].separatorThickness, value); }
void HeadingStyle::resetSeparatorThickness() { m_style->reset(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1SeparatorThickness)); }
qreal HeadingStyle::separatorSpacing() const { return m_style->m_values.headings[m_index].separatorSpacing; }
void HeadingStyle::setSeparatorSpacing(qreal value) { m_style->update(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1SeparatorSpacing), m_style->m_values.headings[m_index].separatorSpacing, value); }
void HeadingStyle::resetSeparatorSpacing() { m_style->reset(MarkdownStyle::headingProperty(m_index, MarkdownStyle::Property::H1SeparatorSpacing)); }

InlineCodeStyle::InlineCodeStyle(MarkdownStyle *style)
    : QObject(style), m_style(style) {}
qreal InlineCodeStyle::horizontalPadding() const { return m_style->m_values.inlineCode.horizontalPadding; }
void InlineCodeStyle::setHorizontalPadding(qreal value) { m_style->update(MarkdownStyle::Property::InlineCodeHorizontalPadding, m_style->m_values.inlineCode.horizontalPadding, value); }
void InlineCodeStyle::resetHorizontalPadding() { m_style->reset(MarkdownStyle::Property::InlineCodeHorizontalPadding); }
qreal InlineCodeStyle::verticalPadding() const { return m_style->m_values.inlineCode.verticalPadding; }
void InlineCodeStyle::setVerticalPadding(qreal value) { m_style->update(MarkdownStyle::Property::InlineCodeVerticalPadding, m_style->m_values.inlineCode.verticalPadding, value); }
void InlineCodeStyle::resetVerticalPadding() { m_style->reset(MarkdownStyle::Property::InlineCodeVerticalPadding); }
QFont InlineCodeStyle::font() const { return m_style->m_values.inlineCode.font; }
void InlineCodeStyle::setFont(const QFont &value) { m_style->update(MarkdownStyle::Property::InlineCodeFont, m_style->m_values.inlineCode.font, value); }
void InlineCodeStyle::resetFont() { m_style->reset(MarkdownStyle::Property::InlineCodeFont); }
QColor InlineCodeStyle::foreground() const { return m_style->m_values.inlineCode.foreground; }
void InlineCodeStyle::setForeground(const QColor &value) { m_style->update(MarkdownStyle::Property::InlineCodeForeground, m_style->m_values.inlineCode.foreground, value); }
void InlineCodeStyle::resetForeground() { m_style->reset(MarkdownStyle::Property::InlineCodeForeground); }
QColor InlineCodeStyle::backgroundColor() const { return m_style->m_values.inlineCode.backgroundColor; }
void InlineCodeStyle::setBackgroundColor(const QColor &value) { m_style->update(MarkdownStyle::Property::InlineCodeBackgroundColor, m_style->m_values.inlineCode.backgroundColor, value); }
void InlineCodeStyle::resetBackgroundColor() { m_style->reset(MarkdownStyle::Property::InlineCodeBackgroundColor); }
qreal InlineCodeStyle::cornerRadius() const { return m_style->m_values.inlineCode.cornerRadius; }
void InlineCodeStyle::setCornerRadius(qreal value) { m_style->update(MarkdownStyle::Property::InlineCodeCornerRadius, m_style->m_values.inlineCode.cornerRadius, value); }
void InlineCodeStyle::resetCornerRadius() { m_style->reset(MarkdownStyle::Property::InlineCodeCornerRadius); }

CodeBlockStyle::CodeBlockStyle(MarkdownStyle *style)
    : QObject(style), m_style(style) {}
QFont CodeBlockStyle::font() const { return m_style->m_values.codeBlock.font; }
void CodeBlockStyle::setFont(const QFont &value) { m_style->update(MarkdownStyle::Property::CodeBlockFont, m_style->m_values.codeBlock.font, value); }
void CodeBlockStyle::resetFont() { m_style->reset(MarkdownStyle::Property::CodeBlockFont); }
QColor CodeBlockStyle::color() const { return m_style->m_values.codeBlock.color; }
void CodeBlockStyle::setColor(const QColor &value) { m_style->update(MarkdownStyle::Property::CodeBlockColor, m_style->m_values.codeBlock.color, value); }
void CodeBlockStyle::resetColor() { m_style->reset(MarkdownStyle::Property::CodeBlockColor); }
QColor CodeBlockStyle::backgroundColor() const { return m_style->m_values.codeBlock.backgroundColor; }
void CodeBlockStyle::setBackgroundColor(const QColor &value) { m_style->update(MarkdownStyle::Property::CodeBlockBackgroundColor, m_style->m_values.codeBlock.backgroundColor, value); }
void CodeBlockStyle::resetBackgroundColor() { m_style->reset(MarkdownStyle::Property::CodeBlockBackgroundColor); }
QColor CodeBlockStyle::borderColor() const { return m_style->m_values.codeBlock.borderColor; }
void CodeBlockStyle::setBorderColor(const QColor &value) { m_style->update(MarkdownStyle::Property::CodeBlockBorderColor, m_style->m_values.codeBlock.borderColor, value); }
void CodeBlockStyle::resetBorderColor() { m_style->reset(MarkdownStyle::Property::CodeBlockBorderColor); }
qreal CodeBlockStyle::borderWidth() const { return m_style->m_values.codeBlock.borderWidth; }
void CodeBlockStyle::setBorderWidth(qreal value) { m_style->update(MarkdownStyle::Property::CodeBlockBorderWidth, m_style->m_values.codeBlock.borderWidth, value); }
void CodeBlockStyle::resetBorderWidth() { m_style->reset(MarkdownStyle::Property::CodeBlockBorderWidth); }
qreal CodeBlockStyle::cornerRadius() const { return m_style->m_values.codeBlock.cornerRadius; }
void CodeBlockStyle::setCornerRadius(qreal value) { m_style->update(MarkdownStyle::Property::CodeBlockCornerRadius, m_style->m_values.codeBlock.cornerRadius, value); }
void CodeBlockStyle::resetCornerRadius() { m_style->reset(MarkdownStyle::Property::CodeBlockCornerRadius); }
qreal CodeBlockStyle::horizontalPadding() const { return m_style->m_values.codeBlock.horizontalPadding; }
void CodeBlockStyle::setHorizontalPadding(qreal value) { m_style->update(MarkdownStyle::Property::CodeBlockHorizontalPadding, m_style->m_values.codeBlock.horizontalPadding, value); }
void CodeBlockStyle::resetHorizontalPadding() { m_style->reset(MarkdownStyle::Property::CodeBlockHorizontalPadding); }
qreal CodeBlockStyle::verticalPadding() const { return m_style->m_values.codeBlock.verticalPadding; }
void CodeBlockStyle::setVerticalPadding(qreal value) { m_style->update(MarkdownStyle::Property::CodeBlockVerticalPadding, m_style->m_values.codeBlock.verticalPadding, value); }
void CodeBlockStyle::resetVerticalPadding() { m_style->reset(MarkdownStyle::Property::CodeBlockVerticalPadding); }

TableStyle::TableStyle(MarkdownStyle *style) : QObject(style), m_style(style) {}
QColor TableStyle::headerForeground() const {
    const auto color = m_style->m_values.table.headerForeground;
    return color.isValid() ? color : m_style->bodyColor();
}
void TableStyle::setHeaderForeground(const QColor &value) { m_style->update(MarkdownStyle::Property::TableHeaderForeground, m_style->m_values.table.headerForeground, value); }
void TableStyle::resetHeaderForeground() { m_style->reset(MarkdownStyle::Property::TableHeaderForeground); }
QColor TableStyle::headerBackground() const { return m_style->m_values.table.headerBackground; }
void TableStyle::setHeaderBackground(const QColor &value) { m_style->update(MarkdownStyle::Property::TableHeaderBackground, m_style->m_values.table.headerBackground, value); }
void TableStyle::resetHeaderBackground() { m_style->reset(MarkdownStyle::Property::TableHeaderBackground); }
QColor TableStyle::bodyBackground() const { return m_style->m_values.table.bodyBackground; }
void TableStyle::setBodyBackground(const QColor &value) { m_style->update(MarkdownStyle::Property::TableBodyBackground, m_style->m_values.table.bodyBackground, value); }
void TableStyle::resetBodyBackground() { m_style->reset(MarkdownStyle::Property::TableBodyBackground); }
QColor TableStyle::borderColor() const { return m_style->m_values.table.borderColor; }
void TableStyle::setBorderColor(const QColor &value) { m_style->update(MarkdownStyle::Property::TableBorderColor, m_style->m_values.table.borderColor, value); }
void TableStyle::resetBorderColor() { m_style->reset(MarkdownStyle::Property::TableBorderColor); }
qreal TableStyle::borderWidth() const { return m_style->m_values.table.borderWidth; }
void TableStyle::setBorderWidth(qreal value) { m_style->update(MarkdownStyle::Property::TableBorderWidth, m_style->m_values.table.borderWidth, value); }
void TableStyle::resetBorderWidth() { m_style->reset(MarkdownStyle::Property::TableBorderWidth); }
qreal TableStyle::horizontalPadding() const { return m_style->m_values.table.horizontalPadding; }
void TableStyle::setHorizontalPadding(qreal value) { m_style->update(MarkdownStyle::Property::TableHorizontalPadding, m_style->m_values.table.horizontalPadding, value); }
void TableStyle::resetHorizontalPadding() { m_style->reset(MarkdownStyle::Property::TableHorizontalPadding); }
qreal TableStyle::verticalPadding() const { return m_style->m_values.table.verticalPadding; }
void TableStyle::setVerticalPadding(qreal value) { m_style->update(MarkdownStyle::Property::TableVerticalPadding, m_style->m_values.table.verticalPadding, value); }
void TableStyle::resetVerticalPadding() { m_style->reset(MarkdownStyle::Property::TableVerticalPadding); }

HeadingStyles::HeadingStyles(MarkdownStyle *style) : QObject(style)
{
    m_h1 = new HeadingStyle(style, 0);
    m_h2 = new HeadingStyle(style, 1);
    m_h3 = new HeadingStyle(style, 2);
    m_h4 = new HeadingStyle(style, 3);
    m_h5 = new HeadingStyle(style, 4);
    m_h6 = new HeadingStyle(style, 5);
}

void MarkdownStyle::setSelectionBackgroundColor(const QColor &value) { update(Property::SelectionBackgroundColor, m_values.selectionBackgroundColor, value); }
void MarkdownStyle::resetSelectionBackgroundColor() { reset(Property::SelectionBackgroundColor); }
void MarkdownStyle::setLinkColor(const QColor &value) { update(Property::LinkColor, m_values.linkColor, value); }
void MarkdownStyle::setLinkUnderline(bool value) { update(Property::LinkUnderline, m_values.linkUnderline, value); }
void MarkdownStyle::setThematicBreakColor(const QColor &value) { update(Property::ThematicBreakColor, m_values.thematicBreakColor, value); }
void MarkdownStyle::setThematicBreakThickness(qreal value) { update(Property::ThematicBreakThickness, m_values.thematicBreakThickness, value); }
void MarkdownStyle::setCodeBlockFont(const QFont &value) { update(Property::CodeBlockFont, m_values.codeBlock.font, value); }
void MarkdownStyle::setCodeBlockColor(const QColor &value) { update(Property::CodeBlockColor, m_values.codeBlock.color, value); }
void MarkdownStyle::setInlineCodeFont(const QFont &value) { update(Property::InlineCodeFont, m_values.inlineCode.font, value); }
void MarkdownStyle::setBodyFont(const QFont &value) { update(Property::BodyFont, m_values.bodyFont, value); }
void MarkdownStyle::setBodyColor(const QColor &value) { update(Property::BodyColor, m_values.bodyColor, value); }
void MarkdownStyle::setH1Font(const QFont &value) { update(Property::H1Font, m_values.headings[0].font, value); }
void MarkdownStyle::setH1Color(const QColor &value) { update(Property::H1Color, m_values.headings[0].color, value); }
void MarkdownStyle::setH2Font(const QFont &value) { update(Property::H2Font, m_values.headings[1].font, value); }
void MarkdownStyle::setH2Color(const QColor &value) { update(Property::H2Color, m_values.headings[1].color, value); }
void MarkdownStyle::setH3Font(const QFont &value) { update(Property::H3Font, m_values.headings[2].font, value); }
void MarkdownStyle::setH3Color(const QColor &value) { update(Property::H3Color, m_values.headings[2].color, value); }
void MarkdownStyle::setH4Font(const QFont &value) { update(Property::H4Font, m_values.headings[3].font, value); }
void MarkdownStyle::setH4Color(const QColor &value) { update(Property::H4Color, m_values.headings[3].color, value); }
void MarkdownStyle::setH5Font(const QFont &value) { update(Property::H5Font, m_values.headings[4].font, value); }
void MarkdownStyle::setH5Color(const QColor &value) { update(Property::H5Color, m_values.headings[4].color, value); }
void MarkdownStyle::setH6Font(const QFont &value) { update(Property::H6Font, m_values.headings[5].font, value); }
void MarkdownStyle::setH6Color(const QColor &value) { update(Property::H6Color, m_values.headings[5].color, value); }
void MarkdownStyle::setBlockSpacing(qreal value) { update(Property::BlockSpacing, m_values.blockSpacing, value); }
void MarkdownStyle::setListIndent(qreal value) { update(Property::ListIndent, m_values.listIndent, value); }
void MarkdownStyle::setQuoteIndent(qreal value) { update(Property::QuoteIndent, m_values.quoteIndent, value); }
void MarkdownStyle::setQuoteRuleColor(const QColor &value) { update(Property::QuoteRuleColor, m_values.quoteRuleColor, value); }
void MarkdownStyle::setQuoteRuleThickness(qreal value) { update(Property::QuoteRuleThickness, m_values.quoteRuleThickness, value); }

MarkdownStyle::MarkdownStyle(QObject *parent) : QObject(parent)
{
    m_table = new TableStyle(this);
    m_headings = new HeadingStyles(this);
    m_inlineCode = new InlineCodeStyle(this);
    m_codeBlock = new CodeBlockStyle(this);
    QFont body = QGuiApplication::font();
    body.setWeight(QFont::Normal);
    QFont code = body;
    code.setFamily(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
    QFont inlineFont;
    inlineFont.setFamily(code.family());
    m_defaults.bodyFont = body;
    m_defaults.bodyColor = QColor("#202020");
    m_defaults.selectionBackgroundColor = QColor("#603b82f6");
    m_defaults.linkColor = QColor("#0066cc");
    m_defaults.linkUnderline = true;
    m_defaults.thematicBreakColor = QColor("#202020");
    m_defaults.thematicBreakThickness = 1;
    m_defaults.blockSpacing = 8;
    m_defaults.listIndent = 24;
    m_defaults.quoteIndent = 16;
    m_defaults.quoteRuleColor = QColor("#808080");
    m_defaults.quoteRuleThickness = 2;
    m_defaults.codeBlock.font = code;
    m_defaults.codeBlock.color = QColor("#202020");
    m_defaults.inlineCode.font = inlineFont;
    const qreal proportions[] = {2, 1.75, 1.5, 1.25, 1.125, 1};
    for (int i = 0; i < 6; ++i) {
        auto &heading = m_defaults.headings[i];
        heading.font = body;
        if (body.pixelSize() > 0) heading.font.setPixelSize(qMax(1, qRound(body.pixelSize() * proportions[i])));
        else heading.font.setPointSizeF(body.pointSizeF() * proportions[i]);
        heading.font.setWeight(QFont::Bold);
        heading.color = QColor("#202020");
        heading.separatorColor = Qt::transparent;
    }
    m_defaults.inlineCode.backgroundColor = Qt::transparent;
    m_defaults.codeBlock.backgroundColor = Qt::transparent;
    m_defaults.codeBlock.borderColor = Qt::transparent;
    m_defaults.inlineCode.horizontalPadding = 4;
    m_defaults.inlineCode.verticalPadding = 2;
    m_defaults.table.headerBackground = Qt::transparent;
    m_defaults.table.bodyBackground = Qt::transparent;
    m_defaults.table.borderColor = Qt::transparent;
    m_defaults.table.horizontalPadding = 4;
    m_defaults.table.verticalPadding = 2;
    restoreDefaults();
}

template<typename T>
void MarkdownStyle::update(Property property, T &current, const T &value, bool explicitOverride)
{
    if (explicitOverride) m_overrides.set(static_cast<size_t>(property));
    bool equal = current == value;
    if constexpr (std::is_same_v<T, QFont>)
        equal = equal && current.resolveMask() == value.resolveMask();
    if constexpr (std::is_same_v<T, qreal>)
        equal = equal || (std::isnan(current) && std::isnan(value));
    if (equal) return;
    current = value;
    notify(property);
}

void MarkdownStyle::notify(Property property)
{
    switch (property) {
    case Property::SelectionBackgroundColor:
        emit selectionBackgroundColorChanged();
        break;
    case Property::LinkColor:
        emit linkColorChanged();
        break;
    case Property::LinkUnderline:
        emit linkUnderlineChanged();
        break;
    case Property::ThematicBreakColor:
        emit thematicBreakColorChanged();
        break;
    case Property::ThematicBreakThickness:
        emit thematicBreakThicknessChanged();
        break;
    case Property::BodyFont:
        emit bodyFontChanged();
        break;
    case Property::BodyColor:
        emit bodyColorChanged();
        if (!m_values.table.headerForeground.isValid()) emit m_table->headerForegroundChanged();
        break;
    case Property::BlockSpacing:
        emit blockSpacingChanged();
        break;
    case Property::ListIndent:
        emit listIndentChanged();
        break;
    case Property::QuoteIndent:
        emit quoteIndentChanged();
        break;
    case Property::QuoteRuleColor:
        emit quoteRuleColorChanged();
        break;
    case Property::QuoteRuleThickness:
        emit quoteRuleThicknessChanged();
        break;
    case Property::H1Font:
        emit m_headings->h1()->fontChanged();
        emit h1FontChanged();
        break;
    case Property::H1Color:
        emit m_headings->h1()->colorChanged();
        emit h1ColorChanged();
        break;
    case Property::H1SeparatorVisible:
        emit m_headings->h1()->separatorVisibleChanged();
        break;
    case Property::H1SeparatorColor:
        emit m_headings->h1()->separatorColorChanged();
        break;
    case Property::H1SeparatorThickness:
        emit m_headings->h1()->separatorThicknessChanged();
        break;
    case Property::H1SeparatorSpacing:
        emit m_headings->h1()->separatorSpacingChanged();
        break;
    case Property::H2Font:
        emit m_headings->h2()->fontChanged();
        emit h2FontChanged();
        break;
    case Property::H2Color:
        emit m_headings->h2()->colorChanged();
        emit h2ColorChanged();
        break;
    case Property::H2SeparatorVisible:
        emit m_headings->h2()->separatorVisibleChanged();
        break;
    case Property::H2SeparatorColor:
        emit m_headings->h2()->separatorColorChanged();
        break;
    case Property::H2SeparatorThickness:
        emit m_headings->h2()->separatorThicknessChanged();
        break;
    case Property::H2SeparatorSpacing:
        emit m_headings->h2()->separatorSpacingChanged();
        break;
    case Property::H3Font:
        emit m_headings->h3()->fontChanged();
        emit h3FontChanged();
        break;
    case Property::H3Color:
        emit m_headings->h3()->colorChanged();
        emit h3ColorChanged();
        break;
    case Property::H3SeparatorVisible:
        emit m_headings->h3()->separatorVisibleChanged();
        break;
    case Property::H3SeparatorColor:
        emit m_headings->h3()->separatorColorChanged();
        break;
    case Property::H3SeparatorThickness:
        emit m_headings->h3()->separatorThicknessChanged();
        break;
    case Property::H3SeparatorSpacing:
        emit m_headings->h3()->separatorSpacingChanged();
        break;
    case Property::H4Font:
        emit m_headings->h4()->fontChanged();
        emit h4FontChanged();
        break;
    case Property::H4Color:
        emit m_headings->h4()->colorChanged();
        emit h4ColorChanged();
        break;
    case Property::H4SeparatorVisible:
        emit m_headings->h4()->separatorVisibleChanged();
        break;
    case Property::H4SeparatorColor:
        emit m_headings->h4()->separatorColorChanged();
        break;
    case Property::H4SeparatorThickness:
        emit m_headings->h4()->separatorThicknessChanged();
        break;
    case Property::H4SeparatorSpacing:
        emit m_headings->h4()->separatorSpacingChanged();
        break;
    case Property::H5Font:
        emit m_headings->h5()->fontChanged();
        emit h5FontChanged();
        break;
    case Property::H5Color:
        emit m_headings->h5()->colorChanged();
        emit h5ColorChanged();
        break;
    case Property::H5SeparatorVisible:
        emit m_headings->h5()->separatorVisibleChanged();
        break;
    case Property::H5SeparatorColor:
        emit m_headings->h5()->separatorColorChanged();
        break;
    case Property::H5SeparatorThickness:
        emit m_headings->h5()->separatorThicknessChanged();
        break;
    case Property::H5SeparatorSpacing:
        emit m_headings->h5()->separatorSpacingChanged();
        break;
    case Property::H6Font:
        emit m_headings->h6()->fontChanged();
        emit h6FontChanged();
        break;
    case Property::H6Color:
        emit m_headings->h6()->colorChanged();
        emit h6ColorChanged();
        break;
    case Property::H6SeparatorVisible:
        emit m_headings->h6()->separatorVisibleChanged();
        break;
    case Property::H6SeparatorColor:
        emit m_headings->h6()->separatorColorChanged();
        break;
    case Property::H6SeparatorThickness:
        emit m_headings->h6()->separatorThicknessChanged();
        break;
    case Property::H6SeparatorSpacing:
        emit m_headings->h6()->separatorSpacingChanged();
        break;
    case Property::InlineCodeFont:
        emit m_inlineCode->fontChanged();
        emit inlineCodeFontChanged();
        break;
    case Property::InlineCodeForeground:
        emit m_inlineCode->foregroundChanged();
        break;
    case Property::InlineCodeBackgroundColor:
        emit m_inlineCode->backgroundColorChanged();
        break;
    case Property::InlineCodeCornerRadius:
        emit m_inlineCode->cornerRadiusChanged();
        break;
    case Property::InlineCodeHorizontalPadding:
        emit m_inlineCode->horizontalPaddingChanged();
        break;
    case Property::InlineCodeVerticalPadding:
        emit m_inlineCode->verticalPaddingChanged();
        break;
    case Property::CodeBlockFont:
        emit m_codeBlock->fontChanged();
        emit codeBlockFontChanged();
        break;
    case Property::CodeBlockColor:
        emit m_codeBlock->colorChanged();
        emit codeBlockColorChanged();
        break;
    case Property::CodeBlockBackgroundColor:
        emit m_codeBlock->backgroundColorChanged();
        break;
    case Property::CodeBlockBorderColor:
        emit m_codeBlock->borderColorChanged();
        break;
    case Property::CodeBlockBorderWidth:
        emit m_codeBlock->borderWidthChanged();
        break;
    case Property::CodeBlockCornerRadius:
        emit m_codeBlock->cornerRadiusChanged();
        break;
    case Property::CodeBlockHorizontalPadding:
        emit m_codeBlock->horizontalPaddingChanged();
        break;
    case Property::CodeBlockVerticalPadding:
        emit m_codeBlock->verticalPaddingChanged();
        break;
    case Property::TableHeaderForeground:
        emit m_table->headerForegroundChanged();
        break;
    case Property::TableHeaderBackground:
        emit m_table->headerBackgroundChanged();
        break;
    case Property::TableBodyBackground:
        emit m_table->bodyBackgroundChanged();
        break;
    case Property::TableBorderColor:
        emit m_table->borderColorChanged();
        break;
    case Property::TableBorderWidth:
        emit m_table->borderWidthChanged();
        break;
    case Property::TableHorizontalPadding:
        emit m_table->horizontalPaddingChanged();
        break;
    case Property::TableVerticalPadding:
        emit m_table->verticalPaddingChanged();
        break;
    case Property::Count: Q_UNREACHABLE();
    }
}

void MarkdownStyle::setPreset(Preset value)
{
    if (value < Legacy || value > MinimalDark || m_preset == value) return;
    m_preset = value;
    applyPreset();
    emit presetChanged();
}

void MarkdownStyle::reset(Property property)
{
    m_overrides.reset(static_cast<size_t>(property));
    applyPreset();
}

void MarkdownStyle::resetOverrides()
{
    m_overrides.reset();
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
        values.selectionBackgroundColor = QColor(dark ? "#7078a9ef" : "#603b82f6");
        values.bodyColor = text;
        values.codeBlock.color = text;
        values.linkColor = QColor(dark ? "#8db9f2" : "#0066cc");
        values.thematicBreakColor = rule;
        values.codeBlock.borderColor = rule;
        values.quoteRuleColor = QColor(dark ? "#a0a0a0" : "#808080");
        for (int i = 1; i <= 6; ++i) {
            values.headings[i - 1].color = text;
            values.headings[i - 1].separatorColor = rule;
        }
        if (polished) {
            values.blockSpacing = 12.0;
            for (int i = 1; i <= 2; ++i) {
                values.headings[i - 1].separatorVisible = true;
                values.headings[i - 1].separatorColor = rule;
                values.headings[i - 1].separatorThickness = 1.0;
                values.headings[i - 1].separatorSpacing = 8.0;
            }
            const QColor background(dark ? "#30343b" : "#eff1f3");
            values.inlineCode.backgroundColor = background;
            values.inlineCode.cornerRadius = 4.0;
            values.codeBlock.backgroundColor = background;
            values.codeBlock.borderColor = rule;
            values.codeBlock.borderWidth = 1.0;
            values.codeBlock.cornerRadius = 6.0;
            values.codeBlock.horizontalPadding = 12.0;
            values.codeBlock.verticalPadding = 12.0;
        }
    }
    if (!m_overrides.test(static_cast<size_t>(Property::SelectionBackgroundColor)))
        update(Property::SelectionBackgroundColor, m_values.selectionBackgroundColor, values.selectionBackgroundColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::LinkColor)))
        update(Property::LinkColor, m_values.linkColor, values.linkColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::LinkUnderline)))
        update(Property::LinkUnderline, m_values.linkUnderline, values.linkUnderline, false);
    if (!m_overrides.test(static_cast<size_t>(Property::ThematicBreakColor)))
        update(Property::ThematicBreakColor, m_values.thematicBreakColor, values.thematicBreakColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::ThematicBreakThickness)))
        update(Property::ThematicBreakThickness, m_values.thematicBreakThickness, values.thematicBreakThickness, false);
    if (!m_overrides.test(static_cast<size_t>(Property::BodyFont)))
        update(Property::BodyFont, m_values.bodyFont, values.bodyFont, false);
    if (!m_overrides.test(static_cast<size_t>(Property::BodyColor)))
        update(Property::BodyColor, m_values.bodyColor, values.bodyColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::BlockSpacing)))
        update(Property::BlockSpacing, m_values.blockSpacing, values.blockSpacing, false);
    if (!m_overrides.test(static_cast<size_t>(Property::ListIndent)))
        update(Property::ListIndent, m_values.listIndent, values.listIndent, false);
    if (!m_overrides.test(static_cast<size_t>(Property::QuoteIndent)))
        update(Property::QuoteIndent, m_values.quoteIndent, values.quoteIndent, false);
    if (!m_overrides.test(static_cast<size_t>(Property::QuoteRuleColor)))
        update(Property::QuoteRuleColor, m_values.quoteRuleColor, values.quoteRuleColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::QuoteRuleThickness)))
        update(Property::QuoteRuleThickness, m_values.quoteRuleThickness, values.quoteRuleThickness, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H1Font)))
        update(Property::H1Font, m_values.headings[0].font, values.headings[0].font, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H1Color)))
        update(Property::H1Color, m_values.headings[0].color, values.headings[0].color, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H1SeparatorVisible)))
        update(Property::H1SeparatorVisible, m_values.headings[0].separatorVisible, values.headings[0].separatorVisible, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H1SeparatorColor)))
        update(Property::H1SeparatorColor, m_values.headings[0].separatorColor, values.headings[0].separatorColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H1SeparatorThickness)))
        update(Property::H1SeparatorThickness, m_values.headings[0].separatorThickness, values.headings[0].separatorThickness, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H1SeparatorSpacing)))
        update(Property::H1SeparatorSpacing, m_values.headings[0].separatorSpacing, values.headings[0].separatorSpacing, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H2Font)))
        update(Property::H2Font, m_values.headings[1].font, values.headings[1].font, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H2Color)))
        update(Property::H2Color, m_values.headings[1].color, values.headings[1].color, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H2SeparatorVisible)))
        update(Property::H2SeparatorVisible, m_values.headings[1].separatorVisible, values.headings[1].separatorVisible, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H2SeparatorColor)))
        update(Property::H2SeparatorColor, m_values.headings[1].separatorColor, values.headings[1].separatorColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H2SeparatorThickness)))
        update(Property::H2SeparatorThickness, m_values.headings[1].separatorThickness, values.headings[1].separatorThickness, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H2SeparatorSpacing)))
        update(Property::H2SeparatorSpacing, m_values.headings[1].separatorSpacing, values.headings[1].separatorSpacing, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H3Font)))
        update(Property::H3Font, m_values.headings[2].font, values.headings[2].font, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H3Color)))
        update(Property::H3Color, m_values.headings[2].color, values.headings[2].color, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H3SeparatorVisible)))
        update(Property::H3SeparatorVisible, m_values.headings[2].separatorVisible, values.headings[2].separatorVisible, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H3SeparatorColor)))
        update(Property::H3SeparatorColor, m_values.headings[2].separatorColor, values.headings[2].separatorColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H3SeparatorThickness)))
        update(Property::H3SeparatorThickness, m_values.headings[2].separatorThickness, values.headings[2].separatorThickness, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H3SeparatorSpacing)))
        update(Property::H3SeparatorSpacing, m_values.headings[2].separatorSpacing, values.headings[2].separatorSpacing, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H4Font)))
        update(Property::H4Font, m_values.headings[3].font, values.headings[3].font, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H4Color)))
        update(Property::H4Color, m_values.headings[3].color, values.headings[3].color, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H4SeparatorVisible)))
        update(Property::H4SeparatorVisible, m_values.headings[3].separatorVisible, values.headings[3].separatorVisible, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H4SeparatorColor)))
        update(Property::H4SeparatorColor, m_values.headings[3].separatorColor, values.headings[3].separatorColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H4SeparatorThickness)))
        update(Property::H4SeparatorThickness, m_values.headings[3].separatorThickness, values.headings[3].separatorThickness, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H4SeparatorSpacing)))
        update(Property::H4SeparatorSpacing, m_values.headings[3].separatorSpacing, values.headings[3].separatorSpacing, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H5Font)))
        update(Property::H5Font, m_values.headings[4].font, values.headings[4].font, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H5Color)))
        update(Property::H5Color, m_values.headings[4].color, values.headings[4].color, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H5SeparatorVisible)))
        update(Property::H5SeparatorVisible, m_values.headings[4].separatorVisible, values.headings[4].separatorVisible, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H5SeparatorColor)))
        update(Property::H5SeparatorColor, m_values.headings[4].separatorColor, values.headings[4].separatorColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H5SeparatorThickness)))
        update(Property::H5SeparatorThickness, m_values.headings[4].separatorThickness, values.headings[4].separatorThickness, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H5SeparatorSpacing)))
        update(Property::H5SeparatorSpacing, m_values.headings[4].separatorSpacing, values.headings[4].separatorSpacing, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H6Font)))
        update(Property::H6Font, m_values.headings[5].font, values.headings[5].font, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H6Color)))
        update(Property::H6Color, m_values.headings[5].color, values.headings[5].color, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H6SeparatorVisible)))
        update(Property::H6SeparatorVisible, m_values.headings[5].separatorVisible, values.headings[5].separatorVisible, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H6SeparatorColor)))
        update(Property::H6SeparatorColor, m_values.headings[5].separatorColor, values.headings[5].separatorColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H6SeparatorThickness)))
        update(Property::H6SeparatorThickness, m_values.headings[5].separatorThickness, values.headings[5].separatorThickness, false);
    if (!m_overrides.test(static_cast<size_t>(Property::H6SeparatorSpacing)))
        update(Property::H6SeparatorSpacing, m_values.headings[5].separatorSpacing, values.headings[5].separatorSpacing, false);
    if (!m_overrides.test(static_cast<size_t>(Property::InlineCodeFont)))
        update(Property::InlineCodeFont, m_values.inlineCode.font, values.inlineCode.font, false);
    if (!m_overrides.test(static_cast<size_t>(Property::InlineCodeForeground)))
        update(Property::InlineCodeForeground, m_values.inlineCode.foreground, values.inlineCode.foreground, false);
    if (!m_overrides.test(static_cast<size_t>(Property::InlineCodeBackgroundColor)))
        update(Property::InlineCodeBackgroundColor, m_values.inlineCode.backgroundColor, values.inlineCode.backgroundColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::InlineCodeCornerRadius)))
        update(Property::InlineCodeCornerRadius, m_values.inlineCode.cornerRadius, values.inlineCode.cornerRadius, false);
    if (!m_overrides.test(static_cast<size_t>(Property::InlineCodeHorizontalPadding)))
        update(Property::InlineCodeHorizontalPadding, m_values.inlineCode.horizontalPadding, values.inlineCode.horizontalPadding, false);
    if (!m_overrides.test(static_cast<size_t>(Property::InlineCodeVerticalPadding)))
        update(Property::InlineCodeVerticalPadding, m_values.inlineCode.verticalPadding, values.inlineCode.verticalPadding, false);
    if (!m_overrides.test(static_cast<size_t>(Property::CodeBlockFont)))
        update(Property::CodeBlockFont, m_values.codeBlock.font, values.codeBlock.font, false);
    if (!m_overrides.test(static_cast<size_t>(Property::CodeBlockColor)))
        update(Property::CodeBlockColor, m_values.codeBlock.color, values.codeBlock.color, false);
    if (!m_overrides.test(static_cast<size_t>(Property::CodeBlockBackgroundColor)))
        update(Property::CodeBlockBackgroundColor, m_values.codeBlock.backgroundColor, values.codeBlock.backgroundColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::CodeBlockBorderColor)))
        update(Property::CodeBlockBorderColor, m_values.codeBlock.borderColor, values.codeBlock.borderColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::CodeBlockBorderWidth)))
        update(Property::CodeBlockBorderWidth, m_values.codeBlock.borderWidth, values.codeBlock.borderWidth, false);
    if (!m_overrides.test(static_cast<size_t>(Property::CodeBlockCornerRadius)))
        update(Property::CodeBlockCornerRadius, m_values.codeBlock.cornerRadius, values.codeBlock.cornerRadius, false);
    if (!m_overrides.test(static_cast<size_t>(Property::CodeBlockHorizontalPadding)))
        update(Property::CodeBlockHorizontalPadding, m_values.codeBlock.horizontalPadding, values.codeBlock.horizontalPadding, false);
    if (!m_overrides.test(static_cast<size_t>(Property::CodeBlockVerticalPadding)))
        update(Property::CodeBlockVerticalPadding, m_values.codeBlock.verticalPadding, values.codeBlock.verticalPadding, false);
    if (!m_overrides.test(static_cast<size_t>(Property::TableHeaderForeground)))
        update(Property::TableHeaderForeground, m_values.table.headerForeground, values.table.headerForeground, false);
    if (!m_overrides.test(static_cast<size_t>(Property::TableHeaderBackground)))
        update(Property::TableHeaderBackground, m_values.table.headerBackground, values.table.headerBackground, false);
    if (!m_overrides.test(static_cast<size_t>(Property::TableBodyBackground)))
        update(Property::TableBodyBackground, m_values.table.bodyBackground, values.table.bodyBackground, false);
    if (!m_overrides.test(static_cast<size_t>(Property::TableBorderColor)))
        update(Property::TableBorderColor, m_values.table.borderColor, values.table.borderColor, false);
    if (!m_overrides.test(static_cast<size_t>(Property::TableBorderWidth)))
        update(Property::TableBorderWidth, m_values.table.borderWidth, values.table.borderWidth, false);
    if (!m_overrides.test(static_cast<size_t>(Property::TableHorizontalPadding)))
        update(Property::TableHorizontalPadding, m_values.table.horizontalPadding, values.table.horizontalPadding, false);
    if (!m_overrides.test(static_cast<size_t>(Property::TableVerticalPadding)))
        update(Property::TableVerticalPadding, m_values.table.verticalPadding, values.table.verticalPadding, false);
}
