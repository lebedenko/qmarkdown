#include "markdownstyle.h"
#include <QGuiApplication>
#include <QFontDatabase>
#include <cmath>

MarkdownStyle::MarkdownStyle(QObject *parent) : QObject(parent)
{
    m_defaultCodeBlockFont = QGuiApplication::font();
    m_defaultCodeBlockFont.setFamily(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
    m_defaultCodeBlockFont.setPixelSize(16);
    m_defaultCodeBlockFont.setWeight(QFont::Normal);
    m_defaultInlineCodeFont.setFamily(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
    m_defaultbodyFont = QGuiApplication::font();
    m_defaultbodyFont.setPixelSize(16);
    m_defaultbodyFont.setWeight(QFont::Normal);
    m_defaulth1Font = QGuiApplication::font();
    m_defaulth1Font.setPixelSize(32);
    m_defaulth1Font.setWeight(QFont::Bold);
    m_defaulth2Font = QGuiApplication::font();
    m_defaulth2Font.setPixelSize(28);
    m_defaulth2Font.setWeight(QFont::Bold);
    m_defaulth3Font = QGuiApplication::font();
    m_defaulth3Font.setPixelSize(24);
    m_defaulth3Font.setWeight(QFont::Bold);
    m_defaulth4Font = QGuiApplication::font();
    m_defaulth4Font.setPixelSize(20);
    m_defaulth4Font.setWeight(QFont::Bold);
    m_defaulth5Font = QGuiApplication::font();
    m_defaulth5Font.setPixelSize(18);
    m_defaulth5Font.setWeight(QFont::Bold);
    m_defaulth6Font = QGuiApplication::font();
    m_defaulth6Font.setPixelSize(16);
    m_defaulth6Font.setWeight(QFont::Bold);
    restoreDefaults();
}

void MarkdownStyle::restoreDefaults()
{
    setThematicBreakColor(QColor("#202020"));
    setThematicBreakThickness(1);
    setCodeBlockFont(m_defaultCodeBlockFont);
    setCodeBlockColor(QColor("#202020"));
    setInlineCodeFont(m_defaultInlineCodeFont);
    setBodyFont(m_defaultbodyFont);
    setBodyColor(QColor("#202020"));
    setH1Font(m_defaulth1Font);
    setH1Color(QColor("#202020"));
    setH2Font(m_defaulth2Font);
    setH2Color(QColor("#202020"));
    setH3Font(m_defaulth3Font);
    setH3Color(QColor("#202020"));
    setH4Font(m_defaulth4Font);
    setH4Color(QColor("#202020"));
    setH5Font(m_defaulth5Font);
    setH5Color(QColor("#202020"));
    setH6Font(m_defaulth6Font);
    setH6Color(QColor("#202020"));
    setListIndent(24);
    setQuoteIndent(16);
    setQuoteRuleColor(QColor("#808080"));
    setQuoteRuleThickness(2);
    setBlockSpacing(8);
}

void MarkdownStyle::setBodyFont(const QFont &value)
{
    if (m_bodyFont == value && m_bodyFont.resolveMask() == value.resolveMask()) return;
    m_bodyFont = value;
    emit bodyFontChanged();
}

void MarkdownStyle::setBodyColor(const QColor &value)
{
    if (m_bodyColor == value) return;
    m_bodyColor = value;
    emit bodyColorChanged();
}

void MarkdownStyle::setH1Font(const QFont &value)
{
    if (m_h1Font == value && m_h1Font.resolveMask() == value.resolveMask()) return;
    m_h1Font = value;
    emit h1FontChanged();
}

void MarkdownStyle::setH1Color(const QColor &value)
{
    if (m_h1Color == value) return;
    m_h1Color = value;
    emit h1ColorChanged();
}

void MarkdownStyle::setH2Font(const QFont &value)
{
    if (m_h2Font == value && m_h2Font.resolveMask() == value.resolveMask()) return;
    m_h2Font = value;
    emit h2FontChanged();
}

void MarkdownStyle::setH2Color(const QColor &value)
{
    if (m_h2Color == value) return;
    m_h2Color = value;
    emit h2ColorChanged();
}

void MarkdownStyle::setH3Font(const QFont &value)
{
    if (m_h3Font == value && m_h3Font.resolveMask() == value.resolveMask()) return;
    m_h3Font = value;
    emit h3FontChanged();
}

void MarkdownStyle::setH3Color(const QColor &value)
{
    if (m_h3Color == value) return;
    m_h3Color = value;
    emit h3ColorChanged();
}

void MarkdownStyle::setH4Font(const QFont &value)
{
    if (m_h4Font == value && m_h4Font.resolveMask() == value.resolveMask()) return;
    m_h4Font = value;
    emit h4FontChanged();
}

void MarkdownStyle::setH4Color(const QColor &value)
{
    if (m_h4Color == value) return;
    m_h4Color = value;
    emit h4ColorChanged();
}

void MarkdownStyle::setH5Font(const QFont &value)
{
    if (m_h5Font == value && m_h5Font.resolveMask() == value.resolveMask()) return;
    m_h5Font = value;
    emit h5FontChanged();
}

void MarkdownStyle::setH5Color(const QColor &value)
{
    if (m_h5Color == value) return;
    m_h5Color = value;
    emit h5ColorChanged();
}

void MarkdownStyle::setH6Font(const QFont &value)
{
    if (m_h6Font == value && m_h6Font.resolveMask() == value.resolveMask()) return;
    m_h6Font = value;
    emit h6FontChanged();
}

void MarkdownStyle::setH6Color(const QColor &value)
{
    if (m_h6Color == value) return;
    m_h6Color = value;
    emit h6ColorChanged();
}

void MarkdownStyle::setBlockSpacing(qreal value)
{
    if (m_blockSpacing == value || (std::isnan(m_blockSpacing) && std::isnan(value))) return;
    m_blockSpacing = value;
    emit blockSpacingChanged();
}

void MarkdownStyle::setInlineCodeFont(const QFont &value)
{
    if (m_inlineCodeFont == value && m_inlineCodeFont.resolveMask() == value.resolveMask()) return;
    m_inlineCodeFont = value;
    emit inlineCodeFontChanged();
}

void MarkdownStyle::setCodeBlockFont(const QFont &value)
{
    if (m_codeBlockFont == value && m_codeBlockFont.resolveMask() == value.resolveMask()) return;
    m_codeBlockFont = value;
    emit codeBlockFontChanged();
}

void MarkdownStyle::setCodeBlockColor(const QColor &value)
{
    if (m_codeBlockColor == value) return;
    m_codeBlockColor = value;
    emit codeBlockColorChanged();
}

void MarkdownStyle::setThematicBreakColor(const QColor &value)
{
    if (m_thematicBreakColor == value) return;
    m_thematicBreakColor = value;
    emit thematicBreakColorChanged();
}

void MarkdownStyle::setThematicBreakThickness(qreal value)
{
    if (m_thematicBreakThickness == value
        || (std::isnan(m_thematicBreakThickness) && std::isnan(value))) return;
    m_thematicBreakThickness = value;
    emit thematicBreakThicknessChanged();
}

void MarkdownStyle::setListIndent(qreal value)
{
    if (m_listIndent == value || (std::isnan(m_listIndent) && std::isnan(value))) return;
    m_listIndent = value;
    emit listIndentChanged();
}

void MarkdownStyle::setQuoteIndent(qreal value)
{
    if (m_quoteIndent == value || (std::isnan(m_quoteIndent) && std::isnan(value))) return;
    m_quoteIndent = value;
    emit quoteIndentChanged();
}

void MarkdownStyle::setQuoteRuleColor(const QColor &value)
{
    if (m_quoteRuleColor == value) return;
    m_quoteRuleColor = value;
    emit quoteRuleColorChanged();
}

void MarkdownStyle::setQuoteRuleThickness(qreal value)
{
    if (m_quoteRuleThickness == value || (std::isnan(m_quoteRuleThickness) && std::isnan(value))) return;
    m_quoteRuleThickness = value;
    emit quoteRuleThicknessChanged();
}
