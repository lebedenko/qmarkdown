#pragma once
#include <QQuickPaintedItem>
#include <QImage>
#include <QPainter>

class ImageItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(qreal naturalWidth READ naturalWidth NOTIFY imageChanged FINAL)
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged FINAL)
public:
    explicit ImageItem(QQuickItem *parent = nullptr) : QQuickPaintedItem(parent) {
        connect(this, &QQuickItem::widthChanged, this, &ImageItem::resizeImage);
    }
    qreal naturalWidth() const { return m_image.width(); }
    QImage image() const { return m_image; }
    void setImage(const QImage &value) { m_image = value; resizeImage(); update(); emit imageChanged(); }
    void paint(QPainter *painter) override {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->drawImage(QRectF(0, 0, width(), height()), m_image);
    }
signals:
    void imageChanged();
private:
    void resizeImage() { setImplicitHeight(m_image.width() > 0 ? m_image.height() * qMin(qMax(0.0, width()) / m_image.width(), 1.0) : 0); }
    QImage m_image;
};
