#pragma once
#include <QClipboard>
#include <QGuiApplication>
#include <QObject>
#include <QtQml/qqmlregistration.h>

// Clipboard ownership belongs to the playground, never the library.
class Clipboard : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    using QObject::QObject;
    Q_INVOKABLE void copyText(const QString &text) const
    { QGuiApplication::clipboard()->setText(text, QClipboard::Clipboard); }
};
