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

/**
 * @brief Gestionnaire d'authentification OAuth2 pour Twitch
 * 
 * Implémente le flux OAuth2 avec PKCE (Proof Key for Code Exchange).
 * Gère le stockage sécurisé des tokens et le rafraîchissement automatique.
 */
class TwitchAuthManager final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
  Q_PROPERTY(QString accessToken READ accessToken NOTIFY accessTokenChanged)

public:
  /**
   * @brief Constructeur
   * @param parent Le parent QObject
   */
  explicit TwitchAuthManager(QObject* parent = nullptr);
  ~TwitchAuthManager() override;

  /**
   * @brief Vérifie si l'utilisateur est authentifié
   * @return true si authentifié
   */
  [[nodiscard]] bool isAuthenticated() const;

  /**
   * @brief Obtient le token d'accès actuel
   * @return Le token d'accès (vide si non authentifié)
   */
  [[nodiscard]] QString accessToken() const;

  /**
   * @brief Lance le processus d'authentification OAuth2
   * 
   * Ouvre le navigateur pour l'authentification utilisateur.
   */
  Q_INVOKABLE void login();

  /**
   * @brief Déconnecte l'utilisateur et efface les tokens
   */
  Q_INVOKABLE void logout();

  /**
   * @brief Rafraîchit le token d'accès avec le refresh token
   */
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

