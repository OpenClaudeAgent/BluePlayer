#include "TestHelpers.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QRandomGenerator>

namespace blueplayer::test {

TestHelpers::TestHelpers(QObject* parent) : QObject(parent) {}

QString TestHelpers::createTwitchApiResponse(const QString& data) {
  QJsonObject root;
  root["data"] = QJsonArray::fromStringList(QStringList() << data);
  QJsonDocument doc(root);
  return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString TestHelpers::createTwitchErrorResponse(const QString& error, int statusCode) {
  QJsonObject root;
  root["error"] = error;
  root["status"] = statusCode;
  root["message"] = error;
  QJsonDocument doc(root);
  return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

bool TestHelpers::urlContainsParams(const QUrl& url, const QMap<QString, QString>& expectedParams) {
  QUrlQuery query(url.query());
  for (auto it = expectedParams.constBegin(); it != expectedParams.constEnd(); ++it) {
    if (query.queryItemValue(it.key()) != it.value()) {
      return false;
    }
  }
  return true;
}

QString TestHelpers::createMockAccessToken() {
  // Génère un token mocké de 30 caractères (format simplifié)
  QString token;
  const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  for (int i = 0; i < 30; ++i) {
    token.append(chars.at(QRandomGenerator::global()->bounded(chars.length())));
  }
  return token;
}

QString TestHelpers::createMockRefreshToken() {
  // Génère un refresh token mocké de 40 caractères
  QString token;
  const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  for (int i = 0; i < 40; ++i) {
    token.append(chars.at(QRandomGenerator::global()->bounded(chars.length())));
  }
  return token;
}

}  // namespace blueplayer::test





