#pragma once
#include <QObject>
#include <QFont>
#include <QColor>
#include <array>
#include <bitset>
#include <QtQml/qqmlregistration.h>

class MarkdownStyle;

class HeadingStyle : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(1, 1)
    Q_PROPERTY(QFont font READ font WRITE setFont RESET resetFont NOTIFY fontChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor RESET resetColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(bool separatorVisible READ separatorVisible WRITE setSeparatorVisible RESET resetSeparatorVisible NOTIFY separatorVisibleChanged FINAL)
    Q_PROPERTY(QColor separatorColor READ separatorColor WRITE setSeparatorColor RESET resetSeparatorColor NOTIFY separatorColorChanged FINAL)
    Q_PROPERTY(qreal separatorThickness READ separatorThickness WRITE setSeparatorThickness RESET resetSeparatorThickness NOTIFY separatorThicknessChanged FINAL)
    Q_PROPERTY(qreal separatorSpacing READ separatorSpacing WRITE setSeparatorSpacing RESET resetSeparatorSpacing NOTIFY separatorSpacingChanged FINAL)
public:
    HeadingStyle(MarkdownStyle *style, int index);
    QFont font() const;
    void setFont(const QFont &value);
    void resetFont();
    QColor color() const;
    void setColor(const QColor &value);
    void resetColor();
    bool separatorVisible() const;
    void setSeparatorVisible(bool value);
    void resetSeparatorVisible();
    QColor separatorColor() const;
    void setSeparatorColor(const QColor &value);
    void resetSeparatorColor();
    qreal separatorThickness() const;
    void setSeparatorThickness(qreal value);
    void resetSeparatorThickness();
    qreal separatorSpacing() const;
    void setSeparatorSpacing(qreal value);
    void resetSeparatorSpacing();
signals:
    void fontChanged();
    void colorChanged();
    void separatorVisibleChanged();
    void separatorColorChanged();
    void separatorThicknessChanged();
    void separatorSpacingChanged();
private:
    MarkdownStyle *m_style;
    int m_index;
};

class InlineCodeStyle : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(1, 1)
    Q_PROPERTY(QFont font READ font WRITE setFont RESET resetFont NOTIFY fontChanged FINAL)
    Q_PROPERTY(QColor foreground READ foreground WRITE setForeground RESET resetForeground NOTIFY foregroundChanged FINAL)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor RESET resetBackgroundColor NOTIFY backgroundColorChanged FINAL)
    Q_PROPERTY(qreal cornerRadius READ cornerRadius WRITE setCornerRadius RESET resetCornerRadius NOTIFY cornerRadiusChanged FINAL)
    Q_PROPERTY(qreal horizontalPadding READ horizontalPadding WRITE setHorizontalPadding RESET resetHorizontalPadding NOTIFY horizontalPaddingChanged FINAL)
    Q_PROPERTY(qreal verticalPadding READ verticalPadding WRITE setVerticalPadding RESET resetVerticalPadding NOTIFY verticalPaddingChanged FINAL)
public:
    qreal horizontalPadding() const;
    void setHorizontalPadding(qreal value);
    void resetHorizontalPadding();
    qreal verticalPadding() const;
    void setVerticalPadding(qreal value);
    void resetVerticalPadding();
    explicit InlineCodeStyle(MarkdownStyle *style);
    QFont font() const;
    void setFont(const QFont &value);
    void resetFont();
    QColor foreground() const;
    void setForeground(const QColor &value);
    void resetForeground();
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &value);
    void resetBackgroundColor();
    qreal cornerRadius() const;
    void setCornerRadius(qreal value);
    void resetCornerRadius();
signals:
    void horizontalPaddingChanged();
    void verticalPaddingChanged();
    void fontChanged();
    void foregroundChanged();
    void backgroundColorChanged();
    void cornerRadiusChanged();
private:
    MarkdownStyle *m_style;
};

class CodeBlockStyle : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(1, 1)
    Q_PROPERTY(QFont font READ font WRITE setFont RESET resetFont NOTIFY fontChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor RESET resetColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor RESET resetBackgroundColor NOTIFY backgroundColorChanged FINAL)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor RESET resetBorderColor NOTIFY borderColorChanged FINAL)
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth RESET resetBorderWidth NOTIFY borderWidthChanged FINAL)
    Q_PROPERTY(qreal cornerRadius READ cornerRadius WRITE setCornerRadius RESET resetCornerRadius NOTIFY cornerRadiusChanged FINAL)
    Q_PROPERTY(qreal horizontalPadding READ horizontalPadding WRITE setHorizontalPadding RESET resetHorizontalPadding NOTIFY horizontalPaddingChanged FINAL)
    Q_PROPERTY(qreal verticalPadding READ verticalPadding WRITE setVerticalPadding RESET resetVerticalPadding NOTIFY verticalPaddingChanged FINAL)
public:
    explicit CodeBlockStyle(MarkdownStyle *style);
    QFont font() const;
    void setFont(const QFont &value);
    void resetFont();
    QColor color() const;
    void setColor(const QColor &value);
    void resetColor();
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &value);
    void resetBackgroundColor();
    QColor borderColor() const;
    void setBorderColor(const QColor &value);
    void resetBorderColor();
    qreal borderWidth() const;
    void setBorderWidth(qreal value);
    void resetBorderWidth();
    qreal cornerRadius() const;
    void setCornerRadius(qreal value);
    void resetCornerRadius();
    qreal horizontalPadding() const;
    void setHorizontalPadding(qreal value);
    void resetHorizontalPadding();
    qreal verticalPadding() const;
    void setVerticalPadding(qreal value);
    void resetVerticalPadding();
signals:
    void fontChanged();
    void colorChanged();
    void backgroundColorChanged();
    void borderColorChanged();
    void borderWidthChanged();
    void cornerRadiusChanged();
    void horizontalPaddingChanged();
    void verticalPaddingChanged();
private:
    MarkdownStyle *m_style;
};

class TableStyle : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(1, 6)
    Q_PROPERTY(QColor headerForeground READ headerForeground WRITE setHeaderForeground RESET resetHeaderForeground NOTIFY headerForegroundChanged FINAL)
    Q_PROPERTY(QColor headerBackground READ headerBackground WRITE setHeaderBackground RESET resetHeaderBackground NOTIFY headerBackgroundChanged FINAL)
    Q_PROPERTY(QColor bodyBackground READ bodyBackground WRITE setBodyBackground RESET resetBodyBackground NOTIFY bodyBackgroundChanged FINAL)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor RESET resetBorderColor NOTIFY borderColorChanged FINAL)
    Q_PROPERTY(qreal borderWidth READ borderWidth WRITE setBorderWidth RESET resetBorderWidth NOTIFY borderWidthChanged FINAL)
    Q_PROPERTY(qreal horizontalPadding READ horizontalPadding WRITE setHorizontalPadding RESET resetHorizontalPadding NOTIFY horizontalPaddingChanged FINAL)
    Q_PROPERTY(qreal verticalPadding READ verticalPadding WRITE setVerticalPadding RESET resetVerticalPadding NOTIFY verticalPaddingChanged FINAL)
public:
    explicit TableStyle(MarkdownStyle *style);
    QColor headerForeground() const;
    void setHeaderForeground(const QColor &value);
    void resetHeaderForeground();
    QColor headerBackground() const;
    void setHeaderBackground(const QColor &value);
    void resetHeaderBackground();
    QColor bodyBackground() const;
    void setBodyBackground(const QColor &value);
    void resetBodyBackground();
    QColor borderColor() const;
    void setBorderColor(const QColor &value);
    void resetBorderColor();
    qreal borderWidth() const;
    void setBorderWidth(qreal value);
    void resetBorderWidth();
    qreal horizontalPadding() const;
    void setHorizontalPadding(qreal value);
    void resetHorizontalPadding();
    qreal verticalPadding() const;
    void setVerticalPadding(qreal value);
    void resetVerticalPadding();
signals:
    void headerForegroundChanged();
    void headerBackgroundChanged();
    void bodyBackgroundChanged();
    void borderColorChanged();
    void borderWidthChanged();
    void horizontalPaddingChanged();
    void verticalPaddingChanged();
private:
    MarkdownStyle *m_style;
};

class HeadingStyles : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    QML_ADDED_IN_VERSION(1, 1)
    Q_PROPERTY(HeadingStyle *h1 READ h1 CONSTANT FINAL)
    Q_PROPERTY(HeadingStyle *h2 READ h2 CONSTANT FINAL)
    Q_PROPERTY(HeadingStyle *h3 READ h3 CONSTANT FINAL)
    Q_PROPERTY(HeadingStyle *h4 READ h4 CONSTANT FINAL)
    Q_PROPERTY(HeadingStyle *h5 READ h5 CONSTANT FINAL)
    Q_PROPERTY(HeadingStyle *h6 READ h6 CONSTANT FINAL)
public:
    explicit HeadingStyles(MarkdownStyle *style);
    HeadingStyle *h1() const { return m_h1; }
    HeadingStyle *h2() const { return m_h2; }
    HeadingStyle *h3() const { return m_h3; }
    HeadingStyle *h4() const { return m_h4; }
    HeadingStyle *h5() const { return m_h5; }
    HeadingStyle *h6() const { return m_h6; }
private:
    HeadingStyle *m_h1;
    HeadingStyle *m_h2;
    HeadingStyle *m_h3;
    HeadingStyle *m_h4;
    HeadingStyle *m_h5;
    HeadingStyle *m_h6;
};

class MarkdownStyle : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QColor selectionBackgroundColor READ selectionBackgroundColor WRITE setSelectionBackgroundColor RESET resetSelectionBackgroundColor NOTIFY selectionBackgroundColorChanged REVISION(1, 7) FINAL)
    Q_PROPERTY(QColor linkColor READ linkColor WRITE setLinkColor NOTIFY linkColorChanged FINAL)
    Q_PROPERTY(bool linkUnderline READ linkUnderline WRITE setLinkUnderline NOTIFY linkUnderlineChanged FINAL)
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
    Q_PROPERTY(Preset preset READ preset WRITE setPreset NOTIFY presetChanged REVISION(1, 1) FINAL)
    Q_PROPERTY(HeadingStyles *headings READ headings CONSTANT REVISION(1, 1) FINAL)
    Q_PROPERTY(InlineCodeStyle *inlineCode READ inlineCode CONSTANT REVISION(1, 1) FINAL)
    Q_PROPERTY(CodeBlockStyle *codeBlock READ codeBlock CONSTANT REVISION(1, 1) FINAL)
    Q_PROPERTY(TableStyle *table READ table CONSTANT REVISION(1, 6) FINAL)
public:
    enum Preset { Legacy, PolishedLight, PolishedDark, MinimalLight, MinimalDark };
    Q_ENUM(Preset)
    explicit MarkdownStyle(QObject *parent = nullptr);
    void restoreDefaults();
    Q_INVOKABLE Q_REVISION(1, 1) void resetOverrides();
    Preset preset() const { return m_preset; }
    void setPreset(Preset value);
    HeadingStyles *headings() const { return m_headings; }
    InlineCodeStyle *inlineCode() const { return m_inlineCode; }
    TableStyle *table() const { return m_table; }
    CodeBlockStyle *codeBlock() const { return m_codeBlock; }
    QColor selectionBackgroundColor() const { return m_values.selectionBackgroundColor; }
    void setSelectionBackgroundColor(const QColor &value);
    void resetSelectionBackgroundColor();
    QColor linkColor() const { return m_values.linkColor; }
    void setLinkColor(const QColor &value);
    bool linkUnderline() const { return m_values.linkUnderline; }
    void setLinkUnderline(bool value);
    QColor thematicBreakColor() const { return m_values.thematicBreakColor; }
    void setThematicBreakColor(const QColor &value);
    qreal thematicBreakThickness() const { return m_values.thematicBreakThickness; }
    void setThematicBreakThickness(qreal value);
    QFont codeBlockFont() const { return m_values.codeBlock.font; }
    void setCodeBlockFont(const QFont &value);
    QColor codeBlockColor() const { return m_values.codeBlock.color; }
    void setCodeBlockColor(const QColor &value);
    QFont inlineCodeFont() const { return m_values.inlineCode.font; }
    void setInlineCodeFont(const QFont &value);
    QFont bodyFont() const { return m_values.bodyFont; }
    void setBodyFont(const QFont &value);
    QColor bodyColor() const { return m_values.bodyColor; }
    void setBodyColor(const QColor &value);
    QFont h1Font() const { return m_values.headings[0].font; }
    void setH1Font(const QFont &value);
    QColor h1Color() const { return m_values.headings[0].color; }
    void setH1Color(const QColor &value);
    QFont h2Font() const { return m_values.headings[1].font; }
    void setH2Font(const QFont &value);
    QColor h2Color() const { return m_values.headings[1].color; }
    void setH2Color(const QColor &value);
    QFont h3Font() const { return m_values.headings[2].font; }
    void setH3Font(const QFont &value);
    QColor h3Color() const { return m_values.headings[2].color; }
    void setH3Color(const QColor &value);
    QFont h4Font() const { return m_values.headings[3].font; }
    void setH4Font(const QFont &value);
    QColor h4Color() const { return m_values.headings[3].color; }
    void setH4Color(const QColor &value);
    QFont h5Font() const { return m_values.headings[4].font; }
    void setH5Font(const QFont &value);
    QColor h5Color() const { return m_values.headings[4].color; }
    void setH5Color(const QColor &value);
    QFont h6Font() const { return m_values.headings[5].font; }
    void setH6Font(const QFont &value);
    QColor h6Color() const { return m_values.headings[5].color; }
    void setH6Color(const QColor &value);
    qreal blockSpacing() const { return m_values.blockSpacing; }
    void setBlockSpacing(qreal value);
    qreal listIndent() const { return m_values.listIndent; }
    void setListIndent(qreal value);
    qreal quoteIndent() const { return m_values.quoteIndent; }
    void setQuoteIndent(qreal value);
    QColor quoteRuleColor() const { return m_values.quoteRuleColor; }
    void setQuoteRuleColor(const QColor &value);
    qreal quoteRuleThickness() const { return m_values.quoteRuleThickness; }
    void setQuoteRuleThickness(qreal value);
signals:
    void presetChanged();
    void selectionBackgroundColorChanged();
    void linkColorChanged();
    void linkUnderlineChanged();
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
    friend class TableStyle;
    friend class HeadingStyle;
    friend class InlineCodeStyle;
    friend class CodeBlockStyle;
    struct HeadingValues {
        QFont font{};
        QColor color{};
        bool separatorVisible{};
        QColor separatorColor{};
        qreal separatorThickness{};
        qreal separatorSpacing{};
    };
    struct InlineCodeValues {
        QFont font{};
        QColor foreground{};
        QColor backgroundColor{};
        qreal cornerRadius{};
        qreal horizontalPadding{};
        qreal verticalPadding{};
    };
    struct CodeBlockValues {
        QFont font{};
        QColor color{};
        QColor backgroundColor{};
        QColor borderColor{};
        qreal borderWidth{};
        qreal cornerRadius{};
        qreal horizontalPadding{};
        qreal verticalPadding{};
    };
    struct TableValues {
        QColor headerForeground{};
        QColor headerBackground{};
        QColor bodyBackground{};
        QColor borderColor{};
        qreal borderWidth{};
        qreal horizontalPadding{};
        qreal verticalPadding{};
    };
    struct Values {
        QColor selectionBackgroundColor{};
        QColor linkColor{};
        bool linkUnderline{};
        QColor thematicBreakColor{};
        qreal thematicBreakThickness{};
        QFont bodyFont{};
        QColor bodyColor{};
        qreal blockSpacing{};
        qreal listIndent{};
        qreal quoteIndent{};
        QColor quoteRuleColor{};
        qreal quoteRuleThickness{};
        std::array<HeadingValues, 6> headings;
        InlineCodeValues inlineCode;
        CodeBlockValues codeBlock;
        TableValues table;
    };
    enum class Property {
        SelectionBackgroundColor,
        LinkColor,
        LinkUnderline,
        ThematicBreakColor,
        ThematicBreakThickness,
        BodyFont,
        BodyColor,
        BlockSpacing,
        ListIndent,
        QuoteIndent,
        QuoteRuleColor,
        QuoteRuleThickness,
        H1Font,
        H1Color,
        H1SeparatorVisible,
        H1SeparatorColor,
        H1SeparatorThickness,
        H1SeparatorSpacing,
        H2Font,
        H2Color,
        H2SeparatorVisible,
        H2SeparatorColor,
        H2SeparatorThickness,
        H2SeparatorSpacing,
        H3Font,
        H3Color,
        H3SeparatorVisible,
        H3SeparatorColor,
        H3SeparatorThickness,
        H3SeparatorSpacing,
        H4Font,
        H4Color,
        H4SeparatorVisible,
        H4SeparatorColor,
        H4SeparatorThickness,
        H4SeparatorSpacing,
        H5Font,
        H5Color,
        H5SeparatorVisible,
        H5SeparatorColor,
        H5SeparatorThickness,
        H5SeparatorSpacing,
        H6Font,
        H6Color,
        H6SeparatorVisible,
        H6SeparatorColor,
        H6SeparatorThickness,
        H6SeparatorSpacing,
        InlineCodeFont,
        InlineCodeForeground,
        InlineCodeBackgroundColor,
        InlineCodeCornerRadius,
        InlineCodeHorizontalPadding,
        InlineCodeVerticalPadding,
        CodeBlockFont,
        CodeBlockColor,
        CodeBlockBackgroundColor,
        CodeBlockBorderColor,
        CodeBlockBorderWidth,
        CodeBlockCornerRadius,
        CodeBlockHorizontalPadding,
        CodeBlockVerticalPadding,
        TableHeaderForeground,
        TableHeaderBackground,
        TableBodyBackground,
        TableBorderColor,
        TableBorderWidth,
        TableHorizontalPadding,
        TableVerticalPadding,
        Count
    };
    static Property headingProperty(int index, Property first) {
        return static_cast<Property>(static_cast<int>(first) + index * (static_cast<int>(Property::H2Font) - static_cast<int>(Property::H1Font)));
    }
    template<typename T> void update(Property property, T &current, const T &value, bool explicitOverride = true);
    void notify(Property property);
    void reset(Property property);
    void applyPreset();
    Values m_values, m_defaults;
    std::bitset<static_cast<size_t>(Property::Count)> m_overrides;
    Preset m_preset = Legacy;
    HeadingStyles *m_headings;
    InlineCodeStyle *m_inlineCode;
    CodeBlockStyle *m_codeBlock;
    TableStyle *m_table;
};
