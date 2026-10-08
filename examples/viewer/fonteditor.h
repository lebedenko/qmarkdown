#pragma once
#include <QObject>
#include <QFont>
#include <QtQml/qqmlregistration.h>

// QML font getters synthesize the other unit, and setters can reject a switch.
// Keep native size inspection and conversion local to the playground.
class FontEditor : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    using QObject::QObject;
    Q_INVOKABLE bool points(const QFont &font) const { return font.pixelSize() < 0; }
    Q_INVOKABLE bool hasSize(const QFont &font) const
    { return font.resolveMask() & QFont::SizeResolved; }
    Q_INVOKABLE qreal size(const QFont &font) const
    { return points(font) ? font.pointSizeF() : font.pixelSize(); }
    Q_INVOKABLE QFont resized(QFont font, bool points, qreal size) const
    {
        if (points) font.setPointSizeF(size);
        else font.setPixelSize(qMax(1, qRound(size)));
        return font;
    }
};
