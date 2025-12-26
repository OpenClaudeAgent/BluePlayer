#include "core/network/ApiClientBase.hpp"

#include "core/Logger.hpp"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QUrl>

namespace blueplayer::core::network {

ApiClientBase::ApiClientBase(QObject* parent)
    : QObject(parent),
      m_httpClient(new HttpClient(this)) {
  connect(m_httpClient, &HttpClient::networkError, this, &ApiClientBase::onHttpClientError);
}

void ApiClientBase::setBearerToken(const QString& token) {
  m_httpClient->setBearerToken(token);
}

QString ApiClientBase::bearerToken() const {
  return m_httpClient->bearerToken();
}

void ApiClientBase::setDefaultHeader(const QString& name, const QString& value) {
  m_httpClient->setDefaultHeader(name, value);
}

QNetworkReply* ApiClientBase::getJson(const QUrl& url, const QHash<QString, QString>& headers) {
  QHash<QString, QString> jsonHeaders = headers;
  if (!jsonHeaders.contains(QStringLiteral("Content-Type"))) {
    jsonHeaders.insert(QStringLiteral("Accept"), QStringLiteral("application/json"));
  }
  return m_httpClient->get(url, jsonHeaders);
}

QNetworkReply* ApiClientBase::postJson(const QUrl& url, const QJsonDocument& jsonData, const QHash<QString, QString>& headers) {
  QHash<QString, QString> jsonHeaders = headers;
  if (!jsonHeaders.contains(QStringLiteral("Content-Type"))) {
    jsonHeaders.insert(QStringLiteral("Content-Type"), QStringLiteral("application/json"));
  }
  const QByteArray data = jsonData.toJson(QJsonDocument::Compact);
  return m_httpClient->post(url, data, jsonHeaders);
}

QNetworkReply* ApiClientBase::postForm(const QUrl& url, const QHash<QString, QString>& formData, const QHash<QString, QString>& headers) {
  QHash<QString, QString> formHeaders = headers;
  if (!formHeaders.contains(QStringLiteral("Content-Type"))) {
    formHeaders.insert(QStringLiteral("Content-Type"), QStringLiteral("application/x-www-form-urlencoded"));
  }

  QUrlQuery query;
  for (auto it = formData.constBegin(); it != formData.constEnd(); ++it) {
    query.addQueryItem(it.key(), it.value());
  }

  const QByteArray data = query.query(QUrl::FullyEncoded).toUtf8();
  return m_httpClient->post(url, data, formHeaders);
}

QJsonDocument ApiClientBase::parseJsonResponse(QNetworkReply* reply, const QString& errorContext) {
  if (!reply) {
    const Error error(ErrorCode::InvalidResponse, QStringLiteral("Réponse nulle"), errorContext);
    emit errorOccurred(error);
    return QJsonDocument();
  }

  const QByteArray payload = reply->readAll();
  Logger::debug(LogCategory::Network, QStringLiteral("%1 response status: %2").arg(errorContext).arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()));
  Logger::debug(LogCategory::Network, QStringLiteral("%1 response size: %2 bytes").arg(errorContext).arg(payload.size()));

  if (payload.isEmpty()) {
    const Error error(ErrorCode::InvalidResponse, QStringLiteral("Réponse vide"), errorContext);
    Logger::error(LogCategory::Network, QStringLiteral("Empty response in %1").arg(errorContext));
    emit errorOccurred(error);
    return QJsonDocument();
  }

  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);

  if (parseError.error != QJsonParseError::NoError) {
    const Error error(ErrorCode::InvalidResponse, 
                     QStringLiteral("Erreur de parsing JSON: %1").arg(parseError.errorString()),
                     errorContext);
    Logger::error(LogCategory::Network, QStringLiteral("JSON parse error in %1: %2").arg(errorContext, parseError.errorString()));
    Logger::debug(LogCategory::Network, QStringLiteral("Response payload: %1").arg(QString::fromUtf8(payload)));
    emit errorOccurred(error);
    return QJsonDocument();
  }

  if (!document.isObject() && !document.isArray()) {
    const Error error(ErrorCode::InvalidResponse, QStringLiteral("Réponse JSON invalide (ni objet ni tableau)"), errorContext);
    Logger::error(LogCategory::Network, QStringLiteral("Invalid JSON structure in %1").arg(errorContext));
    Logger::debug(LogCategory::Network, QStringLiteral("Response payload: %1").arg(QString::fromUtf8(payload)));
    emit errorOccurred(error);
    return QJsonDocument();
  }

  return document;
}

bool ApiClientBase::handleNetworkError(QNetworkReply* reply, const QString& errorContext) {
  const Error error = HttpClient::checkNetworkError(reply, errorContext);
  if (error.hasError()) {
    emit errorOccurred(error);
    return true;
  }
  return false;
}

void ApiClientBase::onHttpClientError(const Error& error) {
  emit errorOccurred(error);
}

}  // namespace blueplayer::core::network




