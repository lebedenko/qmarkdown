#pragma once
#include <QQuickPaintedItem>
#include <QTextLayout>
#include <QGlyphRun>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>
#include <memory>

class FormattedText : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QVariantList searchRanges READ searchRanges WRITE setSearchRanges NOTIFY searchPaintingChanged FINAL)
    Q_PROPERTY(QColor searchHighlightColor READ searchHighlightColor WRITE setSearchHighlightColor NOTIFY searchPaintingChanged FINAL)
    Q_PROPERTY(QColor currentSearchHighlightColor READ currentSearchHighlightColor WRITE setCurrentSearchHighlightColor NOTIFY searchPaintingChanged FINAL)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionChanged FINAL)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionChanged FINAL)
    Q_PROPERTY(QColor selectionColor READ selectionColor WRITE setSelectionColor NOTIFY selectionChanged FINAL)
    Q_PROPERTY(int horizontalAlignment READ horizontalAlignment WRITE setHorizontalAlignment NOTIFY horizontalAlignmentChanged FINAL)
    Q_PROPERTY(QVariantList codeSpans READ codeSpans WRITE setCodeSpans NOTIFY codeSpansChanged FINAL)
    Q_PROPERTY(qreal codeHorizontalPadding READ codeHorizontalPadding WRITE setCodeHorizontalPadding NOTIFY codeHorizontalPaddingChanged FINAL)
    Q_PROPERTY(qreal codeVerticalPadding READ codeVerticalPadding WRITE setCodeVerticalPadding NOTIFY codeVerticalPaddingChanged FINAL)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
    Q_PROPERTY(QVariantList formatRanges READ formatRanges WRITE setFormatRanges NOTIFY formatRangesChanged FINAL)
    Q_PROPERTY(QVariantList linkSpans READ linkSpans WRITE setLinkSpans NOTIFY linkSpansChanged FINAL)
    Q_PROPERTY(QColor linkColor READ linkColor WRITE setLinkColor NOTIFY linkColorChanged FINAL)
    Q_PROPERTY(bool linkUnderline READ linkUnderline WRITE setLinkUnderline NOTIFY linkUnderlineChanged FINAL)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged FINAL)
    Q_PROPERTY(QFont codeFont READ codeFont WRITE setCodeFont NOTIFY codeFontChanged FINAL)
    Q_PROPERTY(QColor codeForeground READ codeForeground WRITE setCodeForeground NOTIFY codeForegroundChanged FINAL)
    Q_PROPERTY(QColor codeBackground READ codeBackground WRITE setCodeBackground NOTIFY codeBackgroundChanged FINAL)
    Q_PROPERTY(qreal codeCornerRadius READ codeCornerRadius WRITE setCodeCornerRadius NOTIFY codeCornerRadiusChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(qreal layoutWidth READ layoutWidth WRITE setLayoutWidth NOTIFY layoutWidthChanged FINAL)
    Q_PROPERTY(qreal logicalHeight READ logicalHeight NOTIFY layoutChanged FINAL)
public:
    explicit FormattedText(QQuickItem *parent = nullptr);
    QVariantList searchRanges() const { return m_searchRanges; }
    void setSearchRanges(const QVariantList &value);
    QColor searchHighlightColor() const { return m_searchColor; }
    QColor currentSearchHighlightColor() const { return m_currentSearchColor; }
    void setSearchHighlightColor(const QColor &value);
    void setCurrentSearchHighlightColor(const QColor &value);
    QVector<QRectF> rangeRectangles(int start, int end) const;
    QPointF paintOffset() const { return m_paintOffset; }
    int selectionStart() const { return m_selectionStart; }
    int selectionEnd() const { return m_selectionEnd; }
    QColor selectionColor() const { return m_selectionColor; }
    void setSelectionStart(int value);
    void setSelectionEnd(int value);
    void setSelectionColor(const QColor &value);
    Q_INVOKABLE int cursorAt(qreal x, qreal y) const;
    int characterAt(qreal x, qreal y) const;
    QVector<QRectF> selectionRectangles() const;
    int horizontalAlignment() const { return m_alignment; }
    void setHorizontalAlignment(int value);
    QVariantList codeSpans() const { return m_codes; }
    void setCodeSpans(const QVariantList &value);
    qreal codeHorizontalPadding() const { return m_codeHorizontalPadding; }
    qreal codeVerticalPadding() const { return m_codeVerticalPadding; }
    void setCodeHorizontalPadding(qreal value);
    void setCodeVerticalPadding(qreal value);
    QString text() const { return m_text; }
    QVariantList formatRanges() const { return m_ranges; }
    QFont font() const { return m_font; }
    QFont codeFont() const { return m_codeFont; }
    QColor codeForeground() const { return m_codeForeground; }
    QColor codeBackground() const { return m_codeBackground; }
    qreal codeCornerRadius() const { return m_codeCornerRadius; }
    void setCodeForeground(const QColor &value);
    void setCodeBackground(const QColor &value);
    void setCodeCornerRadius(qreal value);
    QColor color() const { return m_color; }
    qreal layoutWidth() const { return m_layoutWidth; }
    qreal logicalHeight() const { return m_logicalHeight; }
    const QVector<QRectF> &codeRectangles() const { return m_codeRects; }
    const QTextLayout *layout() const { return m_layout.get(); }
    void setText(const QString &value);
    void setFormatRanges(const QVariantList &value);
    void setFont(const QFont &value);
    void setCodeFont(const QFont &value);
    void setColor(const QColor &value);
    void setLayoutWidth(qreal value);
    QVariantList linkSpans() const { return m_links; }
    QColor linkColor() const { return m_linkColor; }
    bool linkUnderline() const { return m_linkUnderline; }
    void setLinkSpans(const QVariantList &value);
    void setLinkColor(const QColor &value);
    void setLinkUnderline(bool value);
    Q_INVOKABLE int linkAt(qreal x, qreal y) const;
    Q_INVOKABLE QString linkDestination(int identity) const;
    void paint(QPainter *painter) override;
signals:
    void searchPaintingChanged();
    void selectionChanged();
    void codeSpansChanged();
    void codeHorizontalPaddingChanged();
    void codeVerticalPaddingChanged();
    void linkSpansChanged();
    void linkColorChanged();
    void linkUnderlineChanged();
    void horizontalAlignmentChanged();
    void textChanged();
    void formatRangesChanged();
    void fontChanged();
    void codeFontChanged();
    void colorChanged();
    void codeForegroundChanged();
    void codeBackgroundChanged();
    void codeCornerRadiusChanged();
    void layoutWidthChanged();
    void layoutChanged();
protected:
    void updatePolish() override;
    void itemChange(ItemChange change, const ItemChangeData &data) override;
private:
    QVariantList m_searchRanges;
    QColor m_searchColor = QColor("#70ffff00"), m_currentSearchColor = QColor("#b0ff9900");
    struct CursorCell { int start, end; QRectF rect; bool reversed; };
    QVector<CursorCell> m_cursorCells;
    struct CursorLine { int start; qreal top, bottom; };
    QVector<CursorLine> m_cursorLines;
    int m_selectionStart = 0, m_selectionEnd = 0;
    QColor m_selectionColor;
    void observeWindow(QQuickWindow *window);
    void observeScreen(QQuickWindow *window);
    QMetaObject::Connection m_windowScreenConnection, m_screenDpiConnection;
    QString m_text;
    QVariantList m_ranges, m_links, m_codes;
    qreal m_codeHorizontalPadding = 4, m_codeVerticalPadding = 2;
    struct PaintedRun { QGlyphRun glyphs; QColor color; };
    struct HitRegion { QRectF rect; int link; };
    QVector<PaintedRun> m_paddedRuns;
    QVector<HitRegion> m_hitRegions;
    bool m_padded = false;
    QColor m_linkColor = QColor("#0066cc");
    bool m_linkUnderline = true;
    QFont m_font, m_codeFont, m_layoutFont;
    QColor m_codeForeground;
    QColor m_codeBackground = Qt::transparent;
    qreal m_codeCornerRadius = 0;
    QVector<QRectF> m_codeRects;
    QColor m_color = Qt::black;
    int m_alignment = Qt::AlignLeft;
    qreal m_layoutWidth = 0, m_logicalHeight = 0;
    QPointF m_paintOffset;
    std::unique_ptr<QTextLayout> m_layout;
};
