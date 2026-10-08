#pragma once
#include <QObject>
#include <QFont>
#include <QDoubleValidator>
#include <QLocale>
#include <QtQml/qqmlregistration.h>

// SpinBox owns formatting. Qt 6.8 applies fixup while its text binding is
// evaluating; rewriting the text there can re-enter that binding on unit changes.
class SizeValidator : public QDoubleValidator
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString locale READ localeName WRITE setLocaleName NOTIFY changed)
public:
    using QDoubleValidator::QDoubleValidator;
    QString localeName() const { return locale().name(); }
    void setLocaleName(const QString &name) { setLocale(QLocale(name)); }
    void fixup(QString &) const override {}
};

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
