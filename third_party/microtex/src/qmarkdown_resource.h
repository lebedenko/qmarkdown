#pragma once
#include <QFile>
#include <tinyxml2.h>
#include <string>
inline int qmarkdownLoadXml(tinyxml2::XMLDocument &doc, const std::string &name) {
    const auto path = QString::fromStdString(name);
    if (!path.startsWith(":/qmarkdown-math/") || path.contains("..")) return tinyxml2::XML_ERROR_FILE_NOT_FOUND;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return tinyxml2::XML_ERROR_FILE_NOT_FOUND;
    const auto data = file.readAll();
    return doc.Parse(data.constData(), size_t(data.size()));
}
