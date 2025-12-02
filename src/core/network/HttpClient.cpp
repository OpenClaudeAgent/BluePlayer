#include "core/network/HttpClient.hpp"

#include "core/Logger.hpp"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

namespace blueplayer::core::network {

HttpClient::HttpClient(QObject* parent)
    : QObject(parent),
      m_networkManager(new QNetworkAccessManager(this)),
      m_cache(new NetworkCache(this)) {
  m_networkManager->setCache(m_cache);
  Logger::debug(LogCategory::Network, QStringLiteral("HttpClient initialized with cache"));
}

QNetworkReply* HttpClient::get(const QUrl& url, const QHash<QString, QString>& headers) {
  QNetworkRequest request = buildRequest(url, headers);
  QNetworkReply* reply = m_networkManager->get(request);
  
  // Vérifier les erreurs immédiates
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    const Error error = checkNetworkError(reply, QStringLiteral("GET request"));
    if (error.isValid()) {
      emit networkError(error);
    }
  });
  
  return reply;
}

QNetworkReply* HttpClient::post(const QUrl& url, const QByteArray& data, const QHash<QString, QString>& headers) {
  QNetworkRequest request = buildRequest(url, headers);
  QNetworkReply* reply = m_networkManager->post(request, data);
  
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    const Error error = checkNetworkError(reply, QStringLiteral("POST request"));
    if (error.isValid()) {
      emit networkError(error);
    }
  });
  
  return reply;
}

QNetworkReply* HttpClient::put(const QUrl& url, const QByteArray& data, const QHash<QString, QString>& headers) {
  QNetworkRequest request = buildRequest(url, headers);
  QNetworkReply* reply = m_networkManager->put(request, data);
  
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    const Error error = checkNetworkError(reply, QStringLiteral("PUT request"));
    if (error.isValid()) {
      emit networkError(error);
    }
  });
  
  return reply;
}

QNetworkReply* HttpClient::deleteResource(const QUrl& url, const QHash<QString, QString>& headers) {
  QNetworkRequest request = buildRequest(url, headers);
  QNetworkReply* reply = m_networkManager->deleteResource(request);
  
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    const Error error = checkNetworkError(reply, QStringLiteral("DELETE request"));
    if (error.isValid()) {
      emit networkError(error);
    }
  });
  
  return reply;
}

void HttpClient::setBearerToken(const QString& token) {
  m_bearerToken = token;
  if (!token.isEmpty()) {
    setDefaultHeader(QStringLiteral("Authorization"), QStringLiteral("Bearer %1").arg(token));
  } else {
    removeDefaultHeader(QStringLiteral("Authorization"));
  }
}

void HttpClient::setDefaultHeader(const QString& name, const QString& value) {
  m_defaultHeaders.insert(name, value);
}

void HttpClient::removeDefaultHeader(const QString& name) {
  m_defaultHeaders.remove(name);
}

void HttpClient::clearDefaultHeaders() {
  m_defaultHeaders.clear();
}

Error HttpClient::checkNetworkError(QNetworkReply* reply, const QString& context) {
  if (!reply) {
    return Error(ErrorCode::NetworkError, QStringLiteral("Réponse réseau nulle"), context);
  }

  if (reply->error() != QNetworkReply::NoError) {
    const QString errorString = reply->errorString();
    const QByteArray payload = reply->readAll();
    
    Logger::error(LogCategory::Network, QStringLiteral("Network error in %1: %2").arg(context, errorString));
    if (!payload.isEmpty()) {
      Logger::debug(LogCategory::Network, QStringLiteral("Response payload: %1").arg(QString::fromUtf8(payload)));
    }

    // Mapper les erreurs réseau vers ErrorCode approprié
    ErrorCode code = ErrorCode::NetworkError;
    if (reply->error() == QNetworkReply::TimeoutError) {
      code = ErrorCode::NetworkTimeout;
    } else if (reply->error() == QNetworkReply::ConnectionRefusedError) {
      code = ErrorCode::NetworkConnectionRefused;
    }

    return Error(code, QStringLiteral("Erreur lors de %1: %2").arg(context, errorString), QString::fromUtf8(payload));
  }

  return Error();  // Pas d'erreur
}

QNetworkRequest HttpClient::buildRequest(const QUrl& url, const QHash<QString, QString>& customHeaders) const {
  QNetworkRequest request(url);

  // Appliquer les headers par défaut
  for (auto it = m_defaultHeaders.constBegin(); it != m_defaultHeaders.constEnd(); ++it) {
    request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
  }

  // Appliquer les headers personnalisés (écrasent les headers par défaut si même clé)
  for (auto it = customHeaders.constBegin(); it != customHeaders.constEnd(); ++it) {
    request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
  }

  return request;
}

}  // namespace blueplayer::core::network



