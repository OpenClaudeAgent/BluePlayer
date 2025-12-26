#include "api/twitch/TwitchAuthManager.hpp"

#include "core/Constants.hpp"
#include "core/ErrorHandler.hpp"
#include "core/Logger.hpp"
#include "core/SecureStorage.hpp"

#include <QSettings> // Pour migration depuis ancien stockage

using blueplayer::core::ErrorHandler;
using blueplayer::core::LogCategory;
using blueplayer::core::Logger;
using blueplayer::core::SecureStorage;

#include <QAbstractSocket>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QHostAddress>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QSettings>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <Qt>

namespace {

class CallbackServer : public QTcpServer {
  Q_OBJECT

public:
  explicit CallbackServer(const QSslConfiguration &config, quint16 port,
                          QObject *parent = nullptr)
      : QTcpServer(parent), m_sslConfig(config), m_port(port) {}
  ~CallbackServer() override = default;

signals:
  void callbackReceived(const QUrl &location);

protected:
  void incomingConnection(qintptr descriptor) override {
    QTcpSocket *socket = nullptr;
    if (m_sslConfig.isNull()) {
      socket = new QTcpSocket(this);
      if (!socket->setSocketDescriptor(descriptor)) {
        socket->deleteLater();
        return;
      }
      setupRead(socket);
    } else {
      QSslSocket *sslSocket = new QSslSocket(this);
      if (!sslSocket->setSocketDescriptor(descriptor)) {
        sslSocket->deleteLater();
        return;
      }
      sslSocket->setSslConfiguration(m_sslConfig);
      connect(sslSocket, &QSslSocket::encrypted, this,
              &CallbackServer::onSslEncrypted);
      sslSocket->startServerEncryption();
      socket = sslSocket;
    }

    connect(socket, &QTcpSocket::disconnected, socket,
            &QTcpSocket::deleteLater);
  }

private:
  void setupRead(QIODevice *ioDevice) {
    connect(ioDevice, &QIODevice::readyRead, this,
            [this, ioDevice]() { handleRequest(ioDevice); });
  }

  void handleRequest(QIODevice *socket) {
    const QByteArray request = socket->readAll();
    const QList<QByteArray> lines = request.split('\n');
    if (!lines.isEmpty()) {
      const QList<QByteArray> parts = lines.first().split(' ');
      if (parts.size() >= 2) {
        const QByteArray path = parts.at(1);
        const QString scheme = m_sslConfig.isNull() ? QStringLiteral("http")
                                                    : QStringLiteral("https");
        const QUrl url = QUrl(QStringLiteral("%1://127.0.0.1:%2%3")
                                  .arg(scheme)
                                  .arg(m_port)
                                  .arg(QString::fromUtf8(path)));
        emit callbackReceived(url);
      }
    }
    const QByteArray response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html><head><meta charset=\"utf-8\"><title>BluePlayer</title>"
        "<style>body{font-family:system-ui,sans-serif;background:#050d17;color:"
        "#fff;"
        "display:flex;align-items:center;justify-content:center;height:100vh;"
        "margin:0;}"
        ".card{padding:24px;border-radius:12px;background:rgba(2,18,44,.95);"
        "box-shadow:0 15px 35px rgba(0,0,0,.35);text-align:center;}"
        ".card h1{margin:0 0 8px;font-size:26px;}"
        ".card p{margin:0;font-size:16px;color:#8aa0c1;}</style>"
        "<script>"
        "setTimeout(()=>{window.location.href='about:blank';},4000);"
        "window.addEventListener('unload', ()=>window.close());"
        "</script></head>"
        "<body><div class=\"card\"><h1>Authentification réussie</h1>"
        "<p>BluePlayer a bien reçu le callback OAuth. Cette page va se "
        "fermer.</p>"
        "</div></body></html>";
    socket->write(response);
    if (auto tcpSocket = qobject_cast<QAbstractSocket *>(socket)) {
      tcpSocket->disconnectFromHost();
    }
  }

  QSslConfiguration m_sslConfig;
  quint16 m_port = 0;

private slots:
  void onSslEncrypted();
};

void CallbackServer::onSslEncrypted() {
  if (auto sslSocket = qobject_cast<QSslSocket *>(sender())) {
    setupRead(sslSocket);
  }
}

QString base64UrlEncode(const QByteArray &bytes) {
  return QString::fromUtf8(bytes.toBase64(QByteArray::Base64UrlEncoding |
                                          QByteArray::OmitTrailingEquals));
}

QString buildRandomString(int length) {
  const char charset[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
  QString result;
  result.reserve(length);
  for (int i = 0; i < length; ++i) {
    const int index = QRandomGenerator::global()->bounded(
        static_cast<int>(sizeof(charset)) - 1);
    result.append(charset[index]);
  }
  return result;
}

} // namespace

namespace blueplayer::api::twitch {

TwitchAuthManager::TwitchAuthManager(QObject *parent)
    : QObject(parent), m_scope(QString::fromUtf8(
                           blueplayer::core::constants::twitch::kDefaultScope)),
      m_listenPort(blueplayer::core::constants::twitch::kDefaultRedirectPort),
      m_httpClient(new blueplayer::core::network::HttpClient(this)) {
  // Connecter les erreurs réseau de HttpClient
  connect(m_httpClient, &blueplayer::core::network::HttpClient::networkError,
          this, [this](const blueplayer::core::Error &error) {
            emit errorOccurred(error.toString());
          });
  m_clientId = QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID"));
  m_clientSecret = QString::fromUtf8(qgetenv("TWITCH_CLIENT_SECRET"));

  // Log Client-ID used during OAuth token generation for investigation
  Logger::debug(
      LogCategory::Twitch,
      QStringLiteral("[DEBUG] OAuth Client-ID: %1")
          .arg(m_clientId.isEmpty() ? QStringLiteral("EMPTY") : m_clientId));
  const QByteArray certPath = qgetenv("TWITCH_TLS_CERT_PATH");
  if (!certPath.isEmpty()) {
    m_tlsCertPath = QString::fromUtf8(certPath);
  }
  const QByteArray keyPath = qgetenv("TWITCH_TLS_KEY_PATH");
  if (!keyPath.isEmpty()) {
    m_tlsKeyPath = QString::fromUtf8(keyPath);
  }
  const QByteArray redirect = qgetenv("TWITCH_REDIRECT_URI");
  if (!redirect.isEmpty()) {
    m_redirectUri = QString::fromUtf8(redirect);
  } else {
    m_redirectUri = QStringLiteral("https://127.0.0.1:%1/callback")
                        .arg(QString::number(m_listenPort));
  }

  const QByteArray customPort = qgetenv("TWITCH_REDIRECT_PORT");
  if (!customPort.isEmpty()) {
    bool ok = false;
    const int port = QString::fromUtf8(customPort).toInt(&ok);
    if (ok) {
      m_listenPort = static_cast<quint16>(port);
      m_redirectUri = QStringLiteral("https://127.0.0.1:%1/callback")
                          .arg(QString::number(m_listenPort));
    }
  }

  m_sslConfig = buildSslConfiguration();
  loadCredentials();
}

TwitchAuthManager::~TwitchAuthManager() { stopListener(); }

bool TwitchAuthManager::isAuthenticated() const { return m_isAuthenticated; }

QString TwitchAuthManager::accessToken() const {
  // #region agent log
  QFile logFile(QStringLiteral("/Users/user/Projects/BluePlayer/.cursor/debug.log"));
  if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    QJsonObject logEntry;
    logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
    logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
    logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("A");
    logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:220");
    logEntry[QStringLiteral("message")] = QStringLiteral("accessToken() called");
    QJsonObject data;
    data[QStringLiteral("isRefreshing")] = m_isRefreshing;
    data[QStringLiteral("tokenLength")] = m_accessToken.length();
    data[QStringLiteral("tokenPreview")] = m_accessToken.isEmpty() ? QStringLiteral("EMPTY") : m_accessToken.left(8) + QStringLiteral("...");
    data[QStringLiteral("isExpired")] = isTokenExpiredOrExpiringSoon();
    logEntry[QStringLiteral("data")] = data;
    logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
    QTextStream stream(&logFile);
    stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
    logFile.close();
  }
  // #endregion
  
  // Si un refresh est en cours, ne pas retourner l'ancien token expiré
  // Attendre que le refresh soit terminé
  if (m_isRefreshing) {
    // #region agent log
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
      QJsonObject logEntry;
      logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
      logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
      logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("A");
      logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:225");
      logEntry[QStringLiteral("message")] = QStringLiteral("accessToken() - refresh in progress, returning empty token");
      logEntry[QStringLiteral("data")] = QJsonObject();
      logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
      QTextStream stream(&logFile);
      stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
      logFile.close();
    }
    // #endregion
    Logger::debug(LogCategory::Twitch,
                  QStringLiteral("Token refresh in progress, returning empty token"));
    return QString(); // Retourner un token vide pendant le refresh
  }
  
  // Vérifier et rafraîchir le token si nécessaire avant de le retourner
  const_cast<TwitchAuthManager *>(this)->ensureValidToken();
  
  // Si le token est expiré et qu'un refresh vient d'être lancé, retourner vide
  if (m_isRefreshing || (isTokenExpiredOrExpiringSoon() && !m_refreshToken.isEmpty())) {
    // #region agent log
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
      QJsonObject logEntry;
      logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
      logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
      logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("A");
      logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:240");
      logEntry[QStringLiteral("message")] = QStringLiteral("accessToken() - token expired, refresh needed, returning empty");
      logEntry[QStringLiteral("data")] = QJsonObject();
      logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
      QTextStream stream(&logFile);
      stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
      logFile.close();
    }
    // #endregion
    Logger::debug(LogCategory::Twitch,
                  QStringLiteral("Token expired and refresh needed, returning empty token"));
    return QString(); // Retourner un token vide si expiré et refresh nécessaire
  }
  
  // #region agent log
  if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    QJsonObject logEntry;
    logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
    logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
    logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("A");
    logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:250");
    logEntry[QStringLiteral("message")] = QStringLiteral("accessToken() returning valid token");
    QJsonObject data;
    data[QStringLiteral("isRefreshing")] = m_isRefreshing;
    data[QStringLiteral("tokenLength")] = m_accessToken.length();
    data[QStringLiteral("tokenPreview")] = m_accessToken.isEmpty() ? QStringLiteral("EMPTY") : m_accessToken.left(8) + QStringLiteral("...");
    logEntry[QStringLiteral("data")] = data;
    logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
    QTextStream stream(&logFile);
    stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
    logFile.close();
  }
  // #endregion
  return m_accessToken;
}

void TwitchAuthManager::login() {
  if (m_clientId.isEmpty()) {
    const auto error = ErrorHandler::twitchAuthError(
        QStringLiteral("login"),
        QStringLiteral("TWITCH_CLIENT_ID n'est pas défini"));
    emit errorOccurred(error.toString());
    return;
  }

  if (m_isAuthenticated) {
    emit authenticatedChanged(true);
    return;
  }

  startListener();
  m_codeVerifier = generateCodeVerifier();
  m_state = generateState();

  QUrl url(QString::fromUtf8(
      blueplayer::core::constants::twitch::kAuthorizeEndpoint));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("response_type"), QStringLiteral("code"));
  query.addQueryItem(QStringLiteral("client_id"), m_clientId);
  query.addQueryItem(QStringLiteral("redirect_uri"), m_redirectUri);
  query.addQueryItem(QStringLiteral("scope"), m_scope);
  query.addQueryItem(QStringLiteral("state"), m_state);
  query.addQueryItem(QStringLiteral("code_challenge"),
                     codeChallenge(m_codeVerifier));
  query.addQueryItem(QStringLiteral("code_challenge_method"),
                     QStringLiteral("S256"));
  url.setQuery(query);

  QDesktopServices::openUrl(url);
}

void TwitchAuthManager::logout() {
  const bool wasAuthenticated = m_isAuthenticated;
  m_accessToken.clear();
  m_refreshToken.clear();
  m_isAuthenticated = false;

  // Supprimer tous les tokens et le Client-ID stocké
  SecureStorage secureStorage(this);
  secureStorage.remove(QStringLiteral("access_token"));
  secureStorage.remove(QStringLiteral("refresh_token"));
  secureStorage.remove(QStringLiteral("token_expiration"));
  secureStorage.remove(QStringLiteral("token_client_id"));

  // Émettre explicitement le signal de déconnexion
  if (wasAuthenticated) {
    emit authenticatedChanged(false);
  }
  emit accessTokenChanged(m_accessToken);
}

void TwitchAuthManager::refresh() {
  if (m_refreshToken.isEmpty()) {
    const auto error = ErrorHandler::twitchAuthError(
        QStringLiteral("refresh"),
        QStringLiteral("Jeton de rafraîchissement manquant"));
    emit errorOccurred(error.toString());
    Logger::warning(
        LogCategory::Twitch,
        QStringLiteral("Cannot refresh token: refresh token is empty"));
    m_isRefreshing = false;
    return;
  }

  if (m_isRefreshing) {
    Logger::debug(
        LogCategory::Twitch,
        QStringLiteral("Token refresh already in progress, skipping"));
    return;
  }

  m_isRefreshing = true;
  Logger::debug(LogCategory::Twitch,
                QStringLiteral("Refreshing access token..."));

  QUrl tokenUrl(
      QString::fromUtf8(blueplayer::core::constants::twitch::kTokenEndpoint));
  QUrlQuery body;
  body.addQueryItem(QStringLiteral("client_id"), m_clientId);
  body.addQueryItem(QStringLiteral("grant_type"),
                    QStringLiteral("refresh_token"));
  body.addQueryItem(QStringLiteral("refresh_token"), m_refreshToken);
  if (!m_clientSecret.isEmpty()) {
    body.addQueryItem(QStringLiteral("client_secret"), m_clientSecret);
  }

  QHash<QString, QString> headers;
  headers.insert(QStringLiteral("Content-Type"),
                 QStringLiteral("application/x-www-form-urlencoded"));
  QNetworkReply *reply = m_httpClient->post(
      tokenUrl, body.query(QUrl::FullyEncoded).toUtf8(), headers);
  connect(reply, &QNetworkReply::finished, this,
          &TwitchAuthManager::handleTokenReply);
}

void TwitchAuthManager::handleLocalCallback(const QUrl &location) {
  stopListener();
  const QUrlQuery query(location.query());
  const QString state = query.queryItemValue(QStringLiteral("state"));
  const QString code = query.queryItemValue(QStringLiteral("code"));

  if (state != m_state) {
    const auto error =
        ErrorHandler::twitchAuthError(QStringLiteral("handleLocalCallback"),
                                      QStringLiteral("État OAuth incohérent"));
    emit errorOccurred(error.toString());
    return;
  }

  if (code.isEmpty()) {
    const auto error = ErrorHandler::twitchAuthError(
        QStringLiteral("handleLocalCallback"),
        QStringLiteral("Code d'autorisation manquant"));
    emit errorOccurred(error.toString());
    return;
  }

  consumeAuthorizationCode(code, state);
}

void TwitchAuthManager::handleTokenReply() {
  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
  if (!reply) {
    return;
  }

  const blueplayer::core::Error networkError =
      blueplayer::core::network::HttpClient::checkNetworkError(
          reply, QStringLiteral("requête OAuth"));
  if (networkError.hasError()) {
    // #region agent log
    QFile logFile(QStringLiteral("/Users/user/Projects/BluePlayer/.cursor/debug.log"));
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
      QJsonObject logEntry;
      logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
      logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
      logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("B");
      logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:359");
      logEntry[QStringLiteral("message")] = QStringLiteral("handleTokenReply() - network error");
      QJsonObject data;
      data[QStringLiteral("errorMessage")] = networkError.toString();
      int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      data[QStringLiteral("statusCode")] = statusCode;
      logEntry[QStringLiteral("data")] = data;
      logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
      QTextStream stream(&logFile);
      stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
      logFile.close();
    }
    // #endregion
    m_isRefreshing = false;
    emit errorOccurred(networkError.toString());

    // Si on obtient une erreur 400 (Bad Request) ou 401 (Unauthorized) lors
    // d'un refresh, cela signifie généralement que le refresh token est
    // invalide ou révoqué. Il faut déconnecter l'utilisateur pour nettoyer
    // l'état.
    int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 400 || statusCode == 401) {
      Logger::warning(
          LogCategory::Twitch,
          QStringLiteral("Refresh token invalid (Status %1) - logging out")
              .arg(statusCode));
      logout();
    }

    reply->deleteLater();
    return;
  }

  const QByteArray data = reply->readAll();
  const QJsonDocument document = QJsonDocument::fromJson(data);
  if (!document.isObject()) {
    m_isRefreshing = false;
    const auto error =
        ErrorHandler::twitchAuthError(QStringLiteral("handleTokenReply"),
                                      QStringLiteral("Réponse OAuth invalide"));
    emit errorOccurred(error.toString());
    reply->deleteLater();
    return;
  }

  const QJsonObject object = document.object();
  m_accessToken = object.value(QStringLiteral("access_token")).toString();

  // #region agent log
  QFile logFile(QStringLiteral("/Users/user/Projects/BluePlayer/.cursor/debug.log"));
  if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    QJsonObject logEntry;
    logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
    logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
    logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("D");
    logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:394");
    logEntry[QStringLiteral("message")] = QStringLiteral("handleTokenReply() - new token received");
    QJsonObject logData;
    logData[QStringLiteral("newTokenLength")] = m_accessToken.length();
    logData[QStringLiteral("newTokenPreview")] = m_accessToken.isEmpty() ? QStringLiteral("EMPTY") : m_accessToken.left(8) + QStringLiteral("...");
    logEntry[QStringLiteral("data")] = logData;
    logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
    QTextStream stream(&logFile);
    stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
    logFile.close();
  }
  // #endregion

  // Récupérer le refresh_token s'il est présent (peut être absent lors d'un
  // refresh)
  const QString newRefreshToken =
      object.value(QStringLiteral("refresh_token")).toString();
  if (!newRefreshToken.isEmpty()) {
    m_refreshToken = newRefreshToken;
  }

  // Récupérer expires_in et calculer la date d'expiration
  // expires_in est en secondes, par défaut 4 heures (14400 secondes) pour
  // Twitch
  const int expiresIn = object.value(QStringLiteral("expires_in")).toInt(14400);
  m_tokenExpirationTime = QDateTime::currentDateTimeUtc().addSecs(expiresIn);

  Logger::debug(LogCategory::Twitch,
                QStringLiteral("Token expires at: %1 (in %2 seconds)")
                    .arg(m_tokenExpirationTime.toString(Qt::ISODate))
                    .arg(expiresIn));

  m_isRefreshing = false;
  persistCredentials();
  // #region agent log
  if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    QJsonObject logEntry;
    logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
    logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
    logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("C");
    logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:417");
    logEntry[QStringLiteral("message")] = QStringLiteral("handleTokenReply() - about to emit signals");
    QJsonObject logData;
    logData[QStringLiteral("tokenLength")] = m_accessToken.length();
    logEntry[QStringLiteral("data")] = logData;
    logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
    QTextStream stream(&logFile);
    stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
    logFile.close();
  }
  // #endregion
  emitTokenChanged();
  emitAuthenticated();
  // #region agent log
  if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    QJsonObject logEntry;
    logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
    logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
    logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("C");
    logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:418");
    logEntry[QStringLiteral("message")] = QStringLiteral("handleTokenReply() - signals emitted");
    logEntry[QStringLiteral("data")] = QJsonObject();
    logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
    QTextStream stream(&logFile);
    stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
    logFile.close();
  }
  // #endregion
  reply->deleteLater();
}

void TwitchAuthManager::startListener() {
  if (m_server) {
    return;
  }

  m_sslConfig = buildSslConfiguration();
  m_server = new CallbackServer(m_sslConfig, m_listenPort, this);
  if (auto callbackServer = qobject_cast<CallbackServer *>(m_server)) {
    connect(callbackServer, &CallbackServer::callbackReceived, this,
            &TwitchAuthManager::handleLocalCallback);
  }

  if (!m_server->listen(QHostAddress::LocalHost, m_listenPort)) {
    const auto error = ErrorHandler::networkError(
        QStringLiteral("startListener"),
        QStringLiteral("Impossible d'écouter le port local pour OAuth"));
    emit errorOccurred(error.toString());
    stopListener();
  }
}

void TwitchAuthManager::stopListener() {
  if (m_server && m_server->isListening()) {
    m_server->close();
  }
  if (m_server) {
    m_server->deleteLater();
    m_server = nullptr;
  }
}

void TwitchAuthManager::consumeAuthorizationCode(const QString &code,
                                                 const QString &) {
  requestAccessToken(code);
}

void TwitchAuthManager::requestAccessToken(const QString &code) {
  QUrl tokenUrl(
      QString::fromUtf8(blueplayer::core::constants::twitch::kTokenEndpoint));
  QUrlQuery body;
  body.addQueryItem(QStringLiteral("client_id"), m_clientId);
  body.addQueryItem(QStringLiteral("grant_type"),
                    QStringLiteral("authorization_code"));
  body.addQueryItem(QStringLiteral("code"), code);
  body.addQueryItem(QStringLiteral("redirect_uri"), m_redirectUri);
  body.addQueryItem(QStringLiteral("code_verifier"), m_codeVerifier);
  if (!m_clientSecret.isEmpty()) {
    body.addQueryItem(QStringLiteral("client_secret"), m_clientSecret);
  }

  QHash<QString, QString> headers;
  headers.insert(QStringLiteral("Content-Type"),
                 QStringLiteral("application/x-www-form-urlencoded"));
  QNetworkReply *reply = m_httpClient->post(
      tokenUrl, body.query(QUrl::FullyEncoded).toUtf8(), headers);
  connect(reply, &QNetworkReply::finished, this,
          &TwitchAuthManager::handleTokenReply);
}

void TwitchAuthManager::persistCredentials() {
  // Utiliser SecureStorage pour chiffrer les tokens
  SecureStorage secureStorage(this);

  // Ne logger que les premiers caractères du token pour la sécurité
  QString tokenPreview = m_accessToken.isEmpty()
                             ? QStringLiteral("EMPTY")
                             : m_accessToken.left(8) + "...";
  Logger::debug(
      LogCategory::Twitch,
      QStringLiteral("Persisting access token: %1").arg(tokenPreview));

  secureStorage.store(QStringLiteral("access_token"), m_accessToken);
  secureStorage.store(QStringLiteral("refresh_token"), m_refreshToken);

  // Stocker la date d'expiration (en format ISO string)
  if (m_tokenExpirationTime.isValid()) {
    secureStorage.store(QStringLiteral("token_expiration"),
                        m_tokenExpirationTime.toString(Qt::ISODate));
  }

  // IMPORTANT: Stocker le Client-ID utilisé pour générer le token
  // Cela permet de vérifier que le token correspond au Client-ID actuel
  secureStorage.store(QStringLiteral("token_client_id"), m_clientId);
  Logger::debug(LogCategory::Twitch,
                QStringLiteral("Persisting Client-ID used for token: %1")
                    .arg(m_clientId.isEmpty() ? QStringLiteral("EMPTY")
                                              : m_clientId.left(10) + "..."));
}

void TwitchAuthManager::loadCredentials() {
  Logger::debug(LogCategory::Twitch,
                QStringLiteral("loadCredentials() called"));

  // Utiliser SecureStorage pour déchiffrer les tokens
  SecureStorage secureStorage(this);

  m_accessToken = secureStorage.retrieve(QStringLiteral("access_token"));
  m_refreshToken = secureStorage.retrieve(QStringLiteral("refresh_token"));

  // Charger la date d'expiration du token
  const QString expirationStr =
      secureStorage.retrieve(QStringLiteral("token_expiration"));
  if (!expirationStr.isEmpty()) {
    m_tokenExpirationTime = QDateTime::fromString(expirationStr, Qt::ISODate);
    if (!m_tokenExpirationTime.isValid()) {
      Logger::warning(LogCategory::Twitch,
                      QStringLiteral("Invalid token expiration date: %1")
                          .arg(expirationStr));
      m_tokenExpirationTime = QDateTime(); // Invalider
    }
  }

  // IMPORTANT: Vérifier que le Client-ID utilisé pour générer le token
  // correspond au Client-ID actuel Si ce n'est pas le cas, le token est
  // invalide et doit être régénéré NOTE: Pour les anciens tokens sans Client-ID
  // stocké, on les considère comme valides et on stocke le Client-ID actuel. Si
  // le token ne fonctionne pas, l'erreur 400 sera détectée lors de la première
  // requête API et le token sera invalidé à ce moment-là.
  const QString storedClientId =
      secureStorage.retrieve(QStringLiteral("token_client_id"));
  if (!m_accessToken.isEmpty() && !storedClientId.isEmpty()) {
    if (storedClientId != m_clientId) {
      Logger::warning(
          LogCategory::Twitch,
          QStringLiteral("Client-ID mismatch detected! Token was generated "
                         "with Client-ID '%1' but current Client-ID is '%2'")
              .arg(storedClientId.left(10) + "...",
                   m_clientId.isEmpty() ? QStringLiteral("EMPTY")
                                        : m_clientId.left(10) + "..."));
      Logger::warning(LogCategory::Twitch,
                      QStringLiteral("Invalidating tokens - user must "
                                     "re-authenticate with current Client-ID"));
      // Invalider les tokens car ils ne correspondent pas au Client-ID actuel
      m_accessToken.clear();
      m_refreshToken.clear();
      m_tokenExpirationTime = QDateTime();
      secureStorage.remove(QStringLiteral("access_token"));
      secureStorage.remove(QStringLiteral("refresh_token"));
      secureStorage.remove(QStringLiteral("token_expiration"));
      secureStorage.remove(QStringLiteral("token_client_id"));
    } else {
      Logger::debug(LogCategory::Twitch,
                    QStringLiteral("Client-ID verification passed: token "
                                   "matches current Client-ID"));
    }
  } else if (!m_accessToken.isEmpty() && storedClientId.isEmpty()) {
    // Token existe mais pas de Client-ID stocké (ancien token avant cette
    // implémentation) On considère le token comme valide pour l'instant et on
    // stocke le Client-ID actuel Si le token ne fonctionne pas avec ce
    // Client-ID, l'erreur 400 sera détectée lors de la première requête API et
    // le token sera invalidé à ce moment-là
    Logger::debug(
        LogCategory::Twitch,
        QStringLiteral("Token found but no stored Client-ID - storing current "
                       "Client-ID for future verification"));
    Logger::debug(
        LogCategory::Twitch,
        QStringLiteral(
            "Token will be validated on first API call - if Client-ID "
            "mismatch, error 400 will trigger token invalidation"));
    secureStorage.store(QStringLiteral("token_client_id"), m_clientId);
  }

  // Migration depuis l'ancien QSettings si SecureStorage est vide
  if (m_accessToken.isEmpty()) {
    Logger::debug(LogCategory::Twitch,
                  QStringLiteral(
                      "No tokens in SecureStorage, checking legacy QSettings"));
    QSettings legacySettings(QStringLiteral("BluePlayer"),
                             QStringLiteral("Twitch"));
    QString legacyAccessToken =
        legacySettings.value(QStringLiteral("access_token")).toString();
    QString legacyRefreshToken =
        legacySettings.value(QStringLiteral("refresh_token")).toString();

    if (!legacyAccessToken.isEmpty()) {
      Logger::debug(
          LogCategory::Twitch,
          QStringLiteral("Found legacy tokens, migrating to SecureStorage"));
      m_accessToken = legacyAccessToken;
      m_refreshToken = legacyRefreshToken;
      // Migrer vers SecureStorage
      secureStorage.store(QStringLiteral("access_token"), m_accessToken);
      secureStorage.store(QStringLiteral("refresh_token"), m_refreshToken);
      // Stocker le Client-ID actuel avec le token migré
      secureStorage.store(QStringLiteral("token_client_id"), m_clientId);
      // Supprimer les anciens tokens
      legacySettings.remove(QStringLiteral("access_token"));
      legacySettings.remove(QStringLiteral("refresh_token"));
      legacySettings.sync();
      Logger::debug(LogCategory::Twitch, QStringLiteral("Migration completed"));
    }
  }

  // Ne logger que les premiers caractères du token pour la sécurité
  QString tokenPreview = m_accessToken.isEmpty()
                             ? QStringLiteral("EMPTY")
                             : m_accessToken.left(8) + "...";
  Logger::debug(LogCategory::Twitch,
                QStringLiteral("Loaded access token: %1 (length: %2)")
                    .arg(tokenPreview)
                    .arg(m_accessToken.length()));
  Logger::debug(LogCategory::Twitch,
                QStringLiteral("Loaded refresh token, length: %1")
                    .arg(m_refreshToken.length()));

  const bool wasAuthenticated = m_isAuthenticated;
  
  // Vérifier si le token est expiré avant de définir m_isAuthenticated
  // Si le token est expiré et qu'un refresh est nécessaire, ne pas marquer
  // comme authentifié tant que le refresh n'est pas terminé
  bool needsRefresh = !m_accessToken.isEmpty() && 
                      isTokenExpiredOrExpiringSoon() && 
                      !m_refreshToken.isEmpty();
  
  // Ne marquer comme authentifié que si on a un token valide (non expiré)
  // OU si on n'a pas besoin de refresh (token valide)
  m_isAuthenticated = !m_accessToken.isEmpty() && !needsRefresh;

  Logger::debug(LogCategory::Twitch,
                QStringLiteral("Was authenticated: %1").arg(wasAuthenticated));
  Logger::debug(LogCategory::Twitch,
                QStringLiteral("Is authenticated: %1").arg(m_isAuthenticated));
  Logger::debug(LogCategory::Twitch,
                QStringLiteral("Needs refresh: %1").arg(needsRefresh));

  // Si on a un token, émettre les signaux pour déclencher l'auto-login
  if (!m_accessToken.isEmpty()) {
    Logger::debug(LogCategory::Twitch,
                  QStringLiteral("Token found, checking expiration"));
    
    // Si le token est expiré ou va expirer bientôt, le rafraîchir
    if (needsRefresh) {
      Logger::debug(
          LogCategory::Twitch,
          QStringLiteral("Token expired or expiring soon, refreshing..."));
      // #region agent log
      QFile logFile(QStringLiteral("/Users/user/Projects/BluePlayer/.cursor/debug.log"));
      if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QJsonObject logEntry;
        logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
        logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
        logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("A");
        logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:760");
        logEntry[QStringLiteral("message")] = QStringLiteral("loadCredentials() - launching refresh, NOT setting authenticated=true");
        QJsonObject data;
        data[QStringLiteral("tokenLength")] = m_accessToken.length();
        logEntry[QStringLiteral("data")] = data;
        logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
        QTextStream stream(&logFile);
        stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
        logFile.close();
      }
      // #endregion
      refresh();
      // Ne pas émettre les signaux maintenant - ils seront émis après le refresh
      // dans handleTokenReply() via emitTokenChanged() et emitAuthenticated()
      Logger::debug(LogCategory::Twitch,
                    QStringLiteral("Refresh started during load - will emit "
                                   "signals after refresh completes"));
      return;
    }

    // Token valide, émettre les signaux
    Logger::debug(LogCategory::Twitch,
                  QStringLiteral("Token is valid, emitting authentication signals"));
    if (!wasAuthenticated) {
      Logger::debug(LogCategory::Twitch,
                    QStringLiteral("Emitting authenticatedChanged(true)"));
      emit authenticatedChanged(true);
    }
    Logger::debug(LogCategory::Twitch,
                  QStringLiteral("Emitting accessTokenChanged()"));
    emit accessTokenChanged(m_accessToken);

  } else {
    Logger::debug(
        LogCategory::Twitch,
        QStringLiteral("No credentials found, user not authenticated"));
  }
}

void TwitchAuthManager::emitAuthenticated() {
  const bool authenticated = !m_accessToken.isEmpty();
  if (m_isAuthenticated != authenticated) {
    m_isAuthenticated = authenticated;
    emit authenticatedChanged(authenticated);
  }
}

void TwitchAuthManager::emitTokenChanged() {
  emit accessTokenChanged(m_accessToken);
  emitAuthenticated();
}

bool TwitchAuthManager::isTokenExpiredOrExpiringSoon() const {
  if (!m_tokenExpirationTime.isValid()) {
    // Si pas de date d'expiration stockée, considérer comme expiré pour forcer
    // un refresh (utile pour les tokens existants avant cette implémentation)
    return true;
  }

  const QDateTime now = QDateTime::currentDateTimeUtc();
  const qint64 secondsUntilExpiration = now.secsTo(m_tokenExpirationTime);

  // Rafraîchir si le token est expiré ou va expirer dans les 5 prochaines
  // minutes
  constexpr qint64 refreshThresholdSeconds = 300; // 5 minutes

  return secondsUntilExpiration <= refreshThresholdSeconds;
}

void TwitchAuthManager::ensureValidToken() {
  // #region agent log
  QFile logFile(QStringLiteral("/Users/user/Projects/BluePlayer/.cursor/debug.log"));
  if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    QJsonObject logEntry;
    logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
    logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
    logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("A");
    logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:708");
    logEntry[QStringLiteral("message")] = QStringLiteral("ensureValidToken() called");
    QJsonObject data;
    data[QStringLiteral("hasToken")] = !m_accessToken.isEmpty();
    data[QStringLiteral("isRefreshing")] = m_isRefreshing;
    data[QStringLiteral("isExpired")] = isTokenExpiredOrExpiringSoon();
    data[QStringLiteral("hasRefreshToken")] = !m_refreshToken.isEmpty();
    logEntry[QStringLiteral("data")] = data;
    logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
    QTextStream stream(&logFile);
    stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
    logFile.close();
  }
  // #endregion
  // Ne rien faire si on n'a pas de token
  if (m_accessToken.isEmpty()) {
    return;
  }

  // Ne rien faire si un rafraîchissement est déjà en cours
  if (m_isRefreshing) {
    // #region agent log
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
      QJsonObject logEntry;
      logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
      logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
      logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("A");
      logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:715");
      logEntry[QStringLiteral("message")] = QStringLiteral("ensureValidToken() - refresh already in progress, returning");
      logEntry[QStringLiteral("data")] = QJsonObject();
      logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
      QTextStream stream(&logFile);
      stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
      logFile.close();
    }
    // #endregion
    return;
  }

  // Vérifier si le token est expiré ou va expirer bientôt
  if (isTokenExpiredOrExpiringSoon()) {
    if (!m_refreshToken.isEmpty()) {
      Logger::debug(
          LogCategory::Twitch,
          QStringLiteral(
              "Token expired or expiring soon, refreshing automatically..."));
      // #region agent log
      if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QJsonObject logEntry;
        logEntry[QStringLiteral("sessionId")] = QStringLiteral("debug-session");
        logEntry[QStringLiteral("runId")] = QStringLiteral("run1");
        logEntry[QStringLiteral("hypothesisId")] = QStringLiteral("A");
        logEntry[QStringLiteral("location")] = QStringLiteral("TwitchAuthManager.cpp:726");
        logEntry[QStringLiteral("message")] = QStringLiteral("ensureValidToken() - calling refresh()");
        logEntry[QStringLiteral("data")] = QJsonObject();
        logEntry[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();
        QTextStream stream(&logFile);
        stream << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
        logFile.close();
      }
      // #endregion
      refresh();
    } else {
      Logger::warning(
          LogCategory::Twitch,
          QStringLiteral("Token expired but no refresh token available"));
      // Le token est expiré et on ne peut pas le rafraîchir, déconnecter
      // l'utilisateur
      logout();
    }
  }
}

QString TwitchAuthManager::generateCodeVerifier() {
  return buildRandomString(64);
}

QString TwitchAuthManager::generateState() const {
  return buildRandomString(24);
}

QString TwitchAuthManager::codeChallenge(const QString &verifier) const {
  const QByteArray hash =
      QCryptographicHash::hash(verifier.toUtf8(), QCryptographicHash::Sha256);
  return base64UrlEncode(hash);
}

QSslConfiguration TwitchAuthManager::buildSslConfiguration() const {
  if (m_tlsCertPath.isEmpty() || m_tlsKeyPath.isEmpty()) {
    return QSslConfiguration();
  }

  QFile certFile(m_tlsCertPath);
  if (!certFile.open(QIODevice::ReadOnly)) {
    return QSslConfiguration();
  }

  const QList<QSslCertificate> certificates =
      QSslCertificate::fromDevice(&certFile, QSsl::Pem);
  certFile.close();
  if (certificates.isEmpty()) {
    return QSslConfiguration();
  }

  QFile keyFile(m_tlsKeyPath);
  if (!keyFile.open(QIODevice::ReadOnly)) {
    return QSslConfiguration();
  }

  const QSslKey key(&keyFile, QSsl::Rsa, QSsl::Pem);
  keyFile.close();
  if (key.isNull()) {
    return QSslConfiguration();
  }

  QSslConfiguration config;
  config.setLocalCertificate(certificates.first());
  config.setPrivateKey(key);
  config.setPeerVerifyMode(QSslSocket::VerifyNone);
  config.setProtocol(QSsl::TlsV1_2OrLater);
  return config;
}

} // namespace blueplayer::api::twitch

#include "api/twitch/TwitchAuthManager.moc"
