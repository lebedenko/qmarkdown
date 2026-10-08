#pragma once
#include <QQuickPaintedItem>
#include <QTextLayout>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>
#include <memory>

class FormattedText : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
    Q_PROPERTY(QVariantList formatRanges READ formatRanges WRITE setFormatRanges NOTIFY formatRangesChanged FINAL)
    Q_PROPERTY(QVariantList linkSpans READ linkSpans WRITE setLinkSpans NOTIFY linkSpansChanged FINAL)
    Q_PROPERTY(QColor linkColor READ linkColor WRITE setLinkColor NOTIFY linkColorChanged FINAL)
    Q_PROPERTY(bool linkUnderline READ linkUnderline WRITE setLinkUnderline NOTIFY linkUnderlineChanged FINAL)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged FINAL)
    Q_PROPERTY(QFont codeFont READ codeFont WRITE setCodeFont NOTIFY codeFontChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)
    Q_PROPERTY(qreal layoutWidth READ layoutWidth WRITE setLayoutWidth NOTIFY layoutWidthChanged FINAL)
    Q_PROPERTY(qreal logicalHeight READ logicalHeight NOTIFY layoutChanged FINAL)
public:
    explicit FormattedText(QQuickItem *parent = nullptr);
    QString text() const { return m_text; }
    QVariantList formatRanges() const { return m_ranges; }
    QFont font() const { return m_font; }
    QFont codeFont() const { return m_codeFont; }
    QColor color() const { return m_color; }
    qreal layoutWidth() const { return m_layoutWidth; }
    qreal logicalHeight() const { return m_logicalHeight; }
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
    void linkSpansChanged();
    void linkColorChanged();
    void linkUnderlineChanged();
    void textChanged();
    void formatRangesChanged();
    void fontChanged();
    void codeFontChanged();
    void colorChanged();
    void layoutWidthChanged();
    void layoutChanged();
protected:
    void updatePolish() override;
    void itemChange(ItemChange change, const ItemChangeData &data) override;
private:
    void observeWindow(QQuickWindow *window);
    void observeScreen(QQuickWindow *window);
    QMetaObject::Connection m_windowScreenConnection, m_screenDpiConnection;
    QString m_text;
    QVariantList m_ranges, m_links;
    QColor m_linkColor = QColor("#0066cc");
    bool m_linkUnderline = true;
    QFont m_font, m_codeFont, m_layoutFont;
    QColor m_color = Qt::black;
    qreal m_layoutWidth = 0, m_logicalHeight = 0;
    QPointF m_paintOffset;
    std::unique_ptr<QTextLayout> m_layout;
};
