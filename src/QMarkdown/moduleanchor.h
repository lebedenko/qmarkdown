#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

// Keeps generated module registration anchored without exposing a QML type.
class ModuleAnchor : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    using QObject::QObject;
};
