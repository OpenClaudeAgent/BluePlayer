#include "api/twitch/TwitchAuthManager.hpp"

#include <QCryptographicHash>
#include <QDesktopServices>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAbstractSocket>
#include <QNetworkRequest>
#include <QList>
#include <QSsl>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>
#include <QSettings>
#include <QRandomGenerator>
#include <QTextStream>
#include <QTimer>
#include <QHostAddress>
#include <Qt>
#include <QIODevice>

namespace {

constexpr quint16 kDefaultPort = 8443;
constexpr auto kAuthorizeEndpoint = "https://id.twitch.tv/oauth2/authorize";
constexpr auto kTokenEndpoint = "https://id.twitch.tv/oauth2/token";

class CallbackServer : public QTcpServer {
  Q_OBJECT

public:
  explicit CallbackServer(const QSslConfiguration& config,
                          quint16 port,
                          QObject* parent = nullptr)
      : QTcpServer(parent), m_sslConfig(config), m_port(port) {}
  ~CallbackServer() override = default;

signals:
  void callbackReceived(const QUrl& location);

protected:
  void incomingConnection(qintptr descriptor) override {
    QTcpSocket* socket = nullptr;
    if (m_sslConfig.isNull()) {
      socket = new QTcpSocket(this);
      if (!socket->setSocketDescriptor(descriptor)) {
        socket->deleteLater();
        return;
      }
      setupRead(socket);
    } else {
      QSslSocket* sslSocket = new QSslSocket(this);
      if (!sslSocket->setSocketDescriptor(descriptor)) {
        sslSocket->deleteLater();
        return;
      }
      sslSocket->setSslConfiguration(m_sslConfig);
      connect(sslSocket, &QSslSocket::encrypted, this, [this, sslSocket]() {
        setupRead(sslSocket);
      });
      sslSocket->startServerEncryption();
      socket = sslSocket;
    }

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
  }

private:
  void setupRead(QIODevice* ioDevice) {
    connect(ioDevice,
            &QIODevice::readyRead,
            this,
            [this, ioDevice]() { handleRequest(ioDevice); },
            Qt::UniqueConnection);
  }

  void handleRequest(QIODevice* socket) {
    const QByteArray request = socket->readAll();
    const QList<QByteArray> lines = request.split('\n');
    if (!lines.isEmpty()) {
      const QList<QByteArray> parts = lines.first().split(' ');
      if (parts.size() >= 2) {
        const QByteArray path = parts.at(1);
        const QString scheme = m_sslConfig.isNull() ? QStringLiteral("http")
                                                    : QStringLiteral("https");
        const QUrl url =
            QUrl(QStringLiteral("%1://127.0.0.1:%2%3")
                     .arg(scheme)
                     .arg(m_port)
                     .arg(QString::fromUtf8(path)));
        emit callbackReceived(url);
      }
    }
    const QByteArray response =
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<html><body><h1>BluePlayer</h1><p>Vous pouvez fermer cette fenêtre.</p></body></html>";
    socket->write(response);
    if (auto tcpSocket = qobject_cast<QAbstractSocket*>(socket)) {
      tcpSocket->disconnectFromHost();
    }
  }

  QSslConfiguration m_sslConfig;
  quint16 m_port = 0;
};

QString base64UrlEncode(const QByteArray& bytes) {
  return QString::fromUtf8(
      bytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

QString buildRandomString(int length) {
  const char charset[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
  QString result;
  result.reserve(length);
  for (int i = 0; i < length; ++i) {
    const int index =
        QRandomGenerator::global()->bounded(static_cast<int>(sizeof(charset)) - 1);
    result.append(charset[index]);
  }
  return result;
}

}  // namespace

namespace blueplayer::api::twitch {

TwitchAuthManager::TwitchAuthManager(QObject* parent)
    : QObject(parent),
      m_scope(QStringLiteral("user:read:email user:read:follows")),
      m_listenPort(kDefaultPort),
      m_networkManager(new QNetworkAccessManager(this)) {
  m_clientId = QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID"));
  m_clientSecret = QString::fromUtf8(qgetenv("TWITCH_CLIENT_SECRET"));
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
    m_redirectUri =
        QStringLiteral("https://127.0.0.1:%1/callback").arg(QString::number(m_listenPort));
  }

  const QByteArray customPort = qgetenv("TWITCH_REDIRECT_PORT");
  if (!customPort.isEmpty()) {
    bool ok = false;
    const int port = QString::fromUtf8(customPort).toInt(&ok);
    if (ok) {
      m_listenPort = static_cast<quint16>(port);
      m_redirectUri =
          QStringLiteral("https://127.0.0.1:%1/callback").arg(QString::number(m_listenPort));
    }
  }

  m_sslConfig = buildSslConfiguration();
  loadCredentials();
}

TwitchAuthManager::~TwitchAuthManager() {
  stopListener();
}

bool TwitchAuthManager::isAuthenticated() const {
  return m_isAuthenticated;
}

QString TwitchAuthManager::accessToken() const {
  return m_accessToken;
}

void TwitchAuthManager::login() {
  if (m_clientId.isEmpty()) {
    emit errorOccurred(QStringLiteral("TWITCH_CLIENT_ID n'est pas défini."));
    return;
  }

  if (m_isAuthenticated) {
    emit authenticatedChanged(true);
    return;
  }

  startListener();
  m_codeVerifier = generateCodeVerifier();
  m_state = generateState();

  QUrl url(QString::fromUtf8(kAuthorizeEndpoint));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("response_type"), QStringLiteral("code"));
  query.addQueryItem(QStringLiteral("client_id"), m_clientId);
  query.addQueryItem(QStringLiteral("redirect_uri"), m_redirectUri);
  query.addQueryItem(QStringLiteral("scope"), m_scope);
  query.addQueryItem(QStringLiteral("state"), m_state);
  query.addQueryItem(QStringLiteral("code_challenge"), codeChallenge(m_codeVerifier));
  query.addQueryItem(QStringLiteral("code_challenge_method"), QStringLiteral("S256"));
  url.setQuery(query);

  QDesktopServices::openUrl(url);
}

void TwitchAuthManager::logout() {
  m_accessToken.clear();
  m_refreshToken.clear();
  m_isAuthenticated = false;
  persistCredentials();
  emitAuthenticated();
  emitTokenChanged();
}

void TwitchAuthManager::refresh() {
  if (m_refreshToken.isEmpty()) {
    emit errorOccurred(QStringLiteral("Jeton de rafraîchissement manquant."));
    return;
  }

  QUrl tokenUrl(QString::fromUtf8(kTokenEndpoint));
  QNetworkRequest request(tokenUrl);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("application/x-www-form-urlencoded"));
  QUrlQuery body;
  body.addQueryItem(QStringLiteral("client_id"), m_clientId);
  body.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));
  body.addQueryItem(QStringLiteral("refresh_token"), m_refreshToken);
  if (!m_clientSecret.isEmpty()) {
    body.addQueryItem(QStringLiteral("client_secret"), m_clientSecret);
  }

  QNetworkReply* reply =
      m_networkManager->post(request, body.query(QUrl::FullyEncoded).toUtf8());
  connect(reply, &QNetworkReply::finished, this, &TwitchAuthManager::handleTokenReply);
}

void TwitchAuthManager::handleLocalCallback(const QUrl& location) {
  stopListener();
  const QUrlQuery query(location.query());
  const QString state = query.queryItemValue(QStringLiteral("state"));
  const QString code = query.queryItemValue(QStringLiteral("code"));

  if (state != m_state) {
    emit errorOccurred(QStringLiteral("État OAuth incohérent."));
    return;
  }

  if (code.isEmpty()) {
    emit errorOccurred(QStringLiteral("Code d'autorisation manquant."));
    return;
  }

  consumeAuthorizationCode(code, state);
}

void TwitchAuthManager::handleTokenReply() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) {
    return;
  }

  const auto data = reply->readAll();
  if (reply->error() != QNetworkReply::NoError) {
    handleNetworkError(reply, QString::fromUtf8(data));
    reply->deleteLater();
    return;
  }

  const QJsonDocument document = QJsonDocument::fromJson(data);
  if (!document.isObject()) {
    handleNetworkError(reply, QStringLiteral("Réponse OAuth invalide."));
    reply->deleteLater();
    return;
  }

  const QJsonObject object = document.object();
  m_accessToken = object.value(QStringLiteral("access_token")).toString();
  m_refreshToken = object.value(QStringLiteral("refresh_token")).toString();
  persistCredentials();
  emitTokenChanged();
  emitAuthenticated();
  reply->deleteLater();
}

void TwitchAuthManager::startListener() {
  if (m_server) {
    return;
  }

  m_sslConfig = buildSslConfiguration();
  m_server = new CallbackServer(m_sslConfig, m_listenPort, this);
  if (auto callbackServer = qobject_cast<CallbackServer*>(m_server)) {
    connect(callbackServer,
            &CallbackServer::callbackReceived,
            this,
            &TwitchAuthManager::handleLocalCallback);
  }

  if (!m_server->listen(QHostAddress::LocalHost, m_listenPort)) {
    emit errorOccurred(QStringLiteral("Impossible d'écouter le port local pour OAuth."));
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

void TwitchAuthManager::consumeAuthorizationCode(const QString& code, const QString&) {
  requestAccessToken(code);
}

void TwitchAuthManager::requestAccessToken(const QString& code) {
  QUrl tokenUrl(QString::fromUtf8(kTokenEndpoint));
  QNetworkRequest request(tokenUrl);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("application/x-www-form-urlencoded"));
  QUrlQuery body;
  body.addQueryItem(QStringLiteral("client_id"), m_clientId);
  body.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("authorization_code"));
  body.addQueryItem(QStringLiteral("code"), code);
  body.addQueryItem(QStringLiteral("redirect_uri"), m_redirectUri);
  body.addQueryItem(QStringLiteral("code_verifier"), m_codeVerifier);
  if (!m_clientSecret.isEmpty()) {
    body.addQueryItem(QStringLiteral("client_secret"), m_clientSecret);
  }

  QNetworkReply* reply =
      m_networkManager->post(request, body.query(QUrl::FullyEncoded).toUtf8());
  connect(reply, &QNetworkReply::finished, this, &TwitchAuthManager::handleTokenReply);
}

void TwitchAuthManager::persistCredentials() {
  QSettings settings(QStringLiteral("BluePlayer"), QStringLiteral("Twitch"));
  settings.setValue(QStringLiteral("access_token"), m_accessToken);
  settings.setValue(QStringLiteral("refresh_token"), m_refreshToken);
}

void TwitchAuthManager::loadCredentials() {
  QSettings settings(QStringLiteral("BluePlayer"), QStringLiteral("Twitch"));
  m_accessToken = settings.value(QStringLiteral("access_token")).toString();
  m_refreshToken = settings.value(QStringLiteral("refresh_token")).toString();
  m_isAuthenticated = !m_accessToken.isEmpty();
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

QString TwitchAuthManager::generateCodeVerifier() {
  return buildRandomString(64);
}

QString TwitchAuthManager::generateState() const {
  return buildRandomString(24);
}

QString TwitchAuthManager::codeChallenge(const QString& verifier) const {
  const QByteArray hash =
      QCryptographicHash::hash(verifier.toUtf8(), QCryptographicHash::Sha256);
  return base64UrlEncode(hash);
}

void TwitchAuthManager::handleNetworkError(QNetworkReply* reply, const QString& fallback) {
  emit errorOccurred(
      reply->errorString().isEmpty() ? fallback : reply->errorString());
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

}  // namespace blueplayer::api::twitch

#include "api/twitch/TwitchAuthManager.moc"

