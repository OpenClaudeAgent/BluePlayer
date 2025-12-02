#pragma once

#include <QtGlobal>
#include <QObject>
#include <QSslConfiguration>
#include <QString>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
class QTcpServer;
class QUrl;
QT_END_NAMESPACE

namespace blueplayer::api::twitch {

class TwitchAuthManager final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
  Q_PROPERTY(QString accessToken READ accessToken NOTIFY accessTokenChanged)

public:
  explicit TwitchAuthManager(QObject* parent = nullptr);
  ~TwitchAuthManager() override;

  [[nodiscard]] bool isAuthenticated() const;
  [[nodiscard]] QString accessToken() const;

  Q_INVOKABLE void login();
  Q_INVOKABLE void logout();
  Q_INVOKABLE void refresh();

signals:
  void authenticatedChanged(bool authenticated);
  void accessTokenChanged(const QString& token);
  void errorOccurred(const QString& message);

private slots:
  void handleLocalCallback(const QUrl& location);
  void handleTokenReply();

private:
  void startListener();
  void stopListener();
  void consumeAuthorizationCode(const QString& code, const QString& state);
  void requestAccessToken(const QString& code);
  void persistCredentials();
  void loadCredentials();
  void emitAuthenticated();
  void emitTokenChanged();
  QString generateCodeVerifier();
  QString generateState() const;
  QString codeChallenge(const QString& verifier) const;

  void handleNetworkError(QNetworkReply* reply, const QString& fallback);
  QSslConfiguration buildSslConfiguration() const;

  QString m_clientId;
  QString m_redirectUri;
  QString m_clientSecret;
  QString m_scope;
  quint16 m_listenPort = 8443;  // Utilise constants::twitch::kDefaultRedirectPort dans le constructeur
  QString m_codeVerifier;
  QString m_state;
  QString m_accessToken;
  QString m_refreshToken;
  QString m_tlsCertPath;
  QString m_tlsKeyPath;
  QSslConfiguration m_sslConfig;
  QNetworkAccessManager* m_networkManager = nullptr;
  QTcpServer* m_server = nullptr;
  bool m_isAuthenticated = false;
};

}  // namespace blueplayer::api::twitch

