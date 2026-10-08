#pragma once
#include <QObject>
#include <QFont>
#include <QColor>
#include <QtQml/qqmlregistration.h>

class MarkdownStyle : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QColor thematicBreakColor READ thematicBreakColor WRITE setThematicBreakColor NOTIFY thematicBreakColorChanged FINAL)
    Q_PROPERTY(qreal thematicBreakThickness READ thematicBreakThickness WRITE setThematicBreakThickness NOTIFY thematicBreakThicknessChanged FINAL)
    Q_PROPERTY(QFont codeBlockFont READ codeBlockFont WRITE setCodeBlockFont NOTIFY codeBlockFontChanged FINAL)
    Q_PROPERTY(QColor codeBlockColor READ codeBlockColor WRITE setCodeBlockColor NOTIFY codeBlockColorChanged FINAL)
    Q_PROPERTY(QFont inlineCodeFont READ inlineCodeFont WRITE setInlineCodeFont NOTIFY inlineCodeFontChanged FINAL)
    Q_PROPERTY(QFont bodyFont READ bodyFont WRITE setBodyFont NOTIFY bodyFontChanged FINAL)
    Q_PROPERTY(QColor bodyColor READ bodyColor WRITE setBodyColor NOTIFY bodyColorChanged FINAL)
    Q_PROPERTY(QFont h1Font READ h1Font WRITE setH1Font NOTIFY h1FontChanged FINAL)
    Q_PROPERTY(QColor h1Color READ h1Color WRITE setH1Color NOTIFY h1ColorChanged FINAL)
    Q_PROPERTY(QFont h2Font READ h2Font WRITE setH2Font NOTIFY h2FontChanged FINAL)
    Q_PROPERTY(QColor h2Color READ h2Color WRITE setH2Color NOTIFY h2ColorChanged FINAL)
    Q_PROPERTY(QFont h3Font READ h3Font WRITE setH3Font NOTIFY h3FontChanged FINAL)
    Q_PROPERTY(QColor h3Color READ h3Color WRITE setH3Color NOTIFY h3ColorChanged FINAL)
    Q_PROPERTY(QFont h4Font READ h4Font WRITE setH4Font NOTIFY h4FontChanged FINAL)
    Q_PROPERTY(QColor h4Color READ h4Color WRITE setH4Color NOTIFY h4ColorChanged FINAL)
    Q_PROPERTY(QFont h5Font READ h5Font WRITE setH5Font NOTIFY h5FontChanged FINAL)
    Q_PROPERTY(QColor h5Color READ h5Color WRITE setH5Color NOTIFY h5ColorChanged FINAL)
    Q_PROPERTY(QFont h6Font READ h6Font WRITE setH6Font NOTIFY h6FontChanged FINAL)
    Q_PROPERTY(QColor h6Color READ h6Color WRITE setH6Color NOTIFY h6ColorChanged FINAL)
    Q_PROPERTY(qreal blockSpacing READ blockSpacing WRITE setBlockSpacing NOTIFY blockSpacingChanged FINAL)
    Q_PROPERTY(qreal listIndent READ listIndent WRITE setListIndent NOTIFY listIndentChanged FINAL)
    Q_PROPERTY(qreal quoteIndent READ quoteIndent WRITE setQuoteIndent NOTIFY quoteIndentChanged FINAL)
    Q_PROPERTY(QColor quoteRuleColor READ quoteRuleColor WRITE setQuoteRuleColor NOTIFY quoteRuleColorChanged FINAL)
    Q_PROPERTY(qreal quoteRuleThickness READ quoteRuleThickness WRITE setQuoteRuleThickness NOTIFY quoteRuleThicknessChanged FINAL)
public:
    explicit MarkdownStyle(QObject *parent = nullptr);
    void restoreDefaults();
    QColor thematicBreakColor() const { return m_thematicBreakColor; }
    void setThematicBreakColor(const QColor &value);
    qreal thematicBreakThickness() const { return m_thematicBreakThickness; }
    void setThematicBreakThickness(qreal value);
    QFont codeBlockFont() const { return m_codeBlockFont; }
    void setCodeBlockFont(const QFont &value);
    QColor codeBlockColor() const { return m_codeBlockColor; }
    void setCodeBlockColor(const QColor &value);
    QFont inlineCodeFont() const { return m_inlineCodeFont; }
    void setInlineCodeFont(const QFont &value);
    QFont bodyFont() const { return m_bodyFont; }
    void setBodyFont(const QFont &value);
    QColor bodyColor() const { return m_bodyColor; }
    void setBodyColor(const QColor &value);
    QFont h1Font() const { return m_h1Font; }
    void setH1Font(const QFont &value);
    QColor h1Color() const { return m_h1Color; }
    void setH1Color(const QColor &value);
    QFont h2Font() const { return m_h2Font; }
    void setH2Font(const QFont &value);
    QColor h2Color() const { return m_h2Color; }
    void setH2Color(const QColor &value);
    QFont h3Font() const { return m_h3Font; }
    void setH3Font(const QFont &value);
    QColor h3Color() const { return m_h3Color; }
    void setH3Color(const QColor &value);
    QFont h4Font() const { return m_h4Font; }
    void setH4Font(const QFont &value);
    QColor h4Color() const { return m_h4Color; }
    void setH4Color(const QColor &value);
    QFont h5Font() const { return m_h5Font; }
    void setH5Font(const QFont &value);
    QColor h5Color() const { return m_h5Color; }
    void setH5Color(const QColor &value);
    QFont h6Font() const { return m_h6Font; }
    void setH6Font(const QFont &value);
    QColor h6Color() const { return m_h6Color; }
    void setH6Color(const QColor &value);
    qreal blockSpacing() const { return m_blockSpacing; }
    void setBlockSpacing(qreal value);
    qreal listIndent() const { return m_listIndent; }
    void setListIndent(qreal value);
    qreal quoteIndent() const { return m_quoteIndent; }
    void setQuoteIndent(qreal value);
    QColor quoteRuleColor() const { return m_quoteRuleColor; }
    void setQuoteRuleColor(const QColor &value);
    qreal quoteRuleThickness() const { return m_quoteRuleThickness; }
    void setQuoteRuleThickness(qreal value);
signals:
    void thematicBreakColorChanged();
    void thematicBreakThicknessChanged();
    void codeBlockFontChanged();
    void codeBlockColorChanged();
    void inlineCodeFontChanged();
    void bodyFontChanged();
    void bodyColorChanged();
    void h1FontChanged();
    void h1ColorChanged();
    void h2FontChanged();
    void h2ColorChanged();
    void h3FontChanged();
    void h3ColorChanged();
    void h4FontChanged();
    void h4ColorChanged();
    void h5FontChanged();
    void h5ColorChanged();
    void h6FontChanged();
    void h6ColorChanged();
    void blockSpacingChanged();
    void listIndentChanged();
    void quoteIndentChanged();
    void quoteRuleColorChanged();
    void quoteRuleThicknessChanged();
private:
    qreal m_listIndent = 24;
    qreal m_quoteIndent = 16;
    QColor m_quoteRuleColor = QColor("#808080");
    qreal m_quoteRuleThickness = 2;
    QColor m_thematicBreakColor;
    qreal m_thematicBreakThickness = 1;
    QFont m_codeBlockFont, m_defaultCodeBlockFont;
    QColor m_codeBlockColor;
    QFont m_inlineCodeFont, m_defaultInlineCodeFont;
    QFont m_bodyFont, m_defaultbodyFont;
    QColor m_bodyColor;
    QFont m_h1Font, m_defaulth1Font;
    QColor m_h1Color;
    QFont m_h2Font, m_defaulth2Font;
    QColor m_h2Color;
    QFont m_h3Font, m_defaulth3Font;
    QColor m_h3Color;
    QFont m_h4Font, m_defaulth4Font;
    QColor m_h4Color;
    QFont m_h5Font, m_defaulth5Font;
    QColor m_h5Color;
    QFont m_h6Font, m_defaulth6Font;
    QColor m_h6Color;
    qreal m_blockSpacing = 8;
};
