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
  // IMPORTANT: Utiliser setRawHeader pour préserver la casse exacte des noms de headers
  // Qt convertit automatiquement les headers en minuscules si on utilise setHeader()
  // Note: rawHeaderList() normalise les headers en minuscules pour l'affichage,
  // mais setRawHeader() devrait préserver la casse exacte lors de l'envoi
  QStringList defaultHeaderNames;
  for (auto it = m_defaultHeaders.constBegin(); it != m_defaultHeaders.constEnd(); ++it) {
    // Utiliser les bytes bruts pour préserver la casse exacte (important pour Client-ID)
    // IMPORTANT: setRawHeader() préserve la casse exacte lors de l'envoi HTTP
    // rawHeaderList() normalise en minuscules pour l'affichage, mais les headers réels
    // sont envoyés avec la casse exacte spécifiée
    QByteArray headerName = it.key().toUtf8();
    QByteArray headerValue = it.value().toUtf8();
    
    // Pour Client-ID, forcer la casse exacte (C majuscule, I majuscule, D majuscule)
    // car l'API GraphQL Twitch est stricte sur la casse
    if (it.key().compare("Client-ID", Qt::CaseInsensitive) == 0) {
      headerName = QByteArray("Client-ID");  // Forcer la casse exacte
      Logger::debug(LogCategory::Network, QStringLiteral("Forcing Client-ID header with exact case: Client-ID"));
    }
    
    request.setRawHeader(headerName, headerValue);
    defaultHeaderNames << it.key();
    // Masquer les valeurs sensibles dans les logs
    QString logValue = (it.key().compare("Authorization", Qt::CaseInsensitive) == 0) 
                       ? QStringLiteral("Bearer ***") 
                       : it.value().left(30) + (it.value().length() > 30 ? "..." : "");
    Logger::debug(LogCategory::Network, QStringLiteral("Setting default header: %1 = %2").arg(it.key(), logValue));
  }
  if (!defaultHeaderNames.isEmpty()) {
    Logger::debug(LogCategory::Network, QStringLiteral("Default headers applied: %1").arg(defaultHeaderNames.join(", ")));
  }

  // Appliquer les headers personnalisés (écrasent les headers par défaut si même clé)
  // IMPORTANT: Utiliser setRawHeader pour préserver la casse exacte des noms de headers
  QStringList customHeaderNames;
  for (auto it = customHeaders.constBegin(); it != customHeaders.constEnd(); ++it) {
    // Utiliser les bytes bruts pour préserver la casse exacte (important pour Client-ID)
    QByteArray headerName = it.key().toUtf8();
    QByteArray headerValue = it.value().toUtf8();
    
    // Pour Client-ID, s'assurer que la casse est exacte (C majuscule, I majuscule, D majuscule)
    if (it.key().compare("Client-ID", Qt::CaseInsensitive) == 0) {
      headerName = QByteArray("Client-ID");
    }
    
    request.setRawHeader(headerName, headerValue);
    customHeaderNames << it.key();
    Logger::debug(LogCategory::Network, QStringLiteral("Setting custom header: %1 = %2").arg(it.key(), it.value().left(30) + "..."));
  }
  if (!customHeaderNames.isEmpty()) {
    Logger::debug(LogCategory::Network, QStringLiteral("Custom headers applied: %1").arg(customHeaderNames.join(", ")));
  }
  
  // Log final des headers envoyés (pour debug)
  // NOTE: rawHeaderList() normalise les noms en minuscules pour l'affichage,
  // mais cela ne reflète pas nécessairement ce qui est envoyé sur le réseau
  QList<QByteArray> allHeaders = request.rawHeaderList();
  QStringList headerList;
  for (const QByteArray& header : allHeaders) {
    QByteArray value = request.rawHeader(header);
    QString logValue = (header.compare("Authorization", Qt::CaseInsensitive) == 0) 
                       ? QStringLiteral("Bearer ***") 
                       : QString::fromUtf8(value.left(30)) + (value.length() > 30 ? "..." : "");
    headerList << QStringLiteral("%1: %2").arg(QString::fromUtf8(header), logValue);
  }
  Logger::debug(LogCategory::Network, QStringLiteral("Final request headers (%1): %2").arg(allHeaders.size()).arg(headerList.join("; ")));
  
  // Log détaillé des bytes bruts des headers pour investigation
  // Cela montre exactement ce qui est passé à setRawHeader()
  Logger::debug(LogCategory::Network, QStringLiteral("=== RAW HEADER BYTES INVESTIGATION ==="));
  Logger::debug(LogCategory::Network, QStringLiteral("URL: %1").arg(url.toString()));
  for (auto it = m_defaultHeaders.constBegin(); it != m_defaultHeaders.constEnd(); ++it) {
    QByteArray headerName = it.key().toUtf8();
    if (it.key().compare("Client-ID", Qt::CaseInsensitive) == 0) {
      headerName = QByteArray("Client-ID");
    }
    QByteArray headerValue = it.value().toUtf8();
    QString valueLog = (it.key().compare("Authorization", Qt::CaseInsensitive) == 0) 
                       ? QStringLiteral("Bearer ***") 
                       : QString::fromUtf8(headerValue);
    Logger::debug(LogCategory::Network, QStringLiteral("Default header bytes - Name: '%1' (%2 bytes), Value: '%3' (%4 bytes)")
                  .arg(QString::fromUtf8(headerName))
                  .arg(headerName.size())
                  .arg(valueLog)
                  .arg(headerValue.size()));
  }
  for (auto it = customHeaders.constBegin(); it != customHeaders.constEnd(); ++it) {
    QByteArray headerName = it.key().toUtf8();
    if (it.key().compare("Client-ID", Qt::CaseInsensitive) == 0) {
      headerName = QByteArray("Client-ID");
    }
    QByteArray headerValue = it.value().toUtf8();
    Logger::debug(LogCategory::Network, QStringLiteral("Custom header bytes - Name: '%1' (%2 bytes), Value: '%3' (%4 bytes)")
                  .arg(QString::fromUtf8(headerName))
                  .arg(headerName.size())
                  .arg(QString::fromUtf8(headerValue))
                  .arg(headerValue.size()));
  }
  Logger::debug(LogCategory::Network, QStringLiteral("=== END RAW HEADER INVESTIGATION ==="));

  return request;
}

}  // namespace blueplayer::core::network




