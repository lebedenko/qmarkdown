#pragma once
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>

namespace Diagnostic {
inline int action = 0;
inline void record(const QString &stage, const QString &text, QJsonObject extra = {}) {
  const auto bytes = text.toUtf8();
  extra.insert("action", action);
  extra.insert("pid", QCoreApplication::applicationPid());
  extra.insert("stage", stage);
  extra.insert("text", text);
  extra.insert("byte_length", bytes.size());
  extra.insert("hex", QString::fromLatin1(bytes.toHex()));
  extra.insert("sha256", QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex()));
  QFile out(qEnvironmentVariable("MDV_TRACE"));
  if (!out.open(QIODevice::WriteOnly | QIODevice::Append)) qFatal("Cannot open MDV_TRACE");
  out.write(QJsonDocument(extra).toJson(QJsonDocument::Compact));
  out.write("\n");
  out.flush();
}
}
