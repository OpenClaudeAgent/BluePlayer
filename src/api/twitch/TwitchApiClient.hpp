#pragma once

#include <QObject>
#include <QVariantList>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
class QUrl;
QT_END_NAMESPACE

namespace blueplayer::api::twitch {

/**
 * @brief Client pour l'API Twitch Helix
 * 
 * Gère les requêtes HTTP vers l'API Twitch et le parsing des réponses JSON.
 * Utilise QNetworkAccessManager avec cache pour optimiser les performances.
 */
class TwitchApiClient : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Constructeur
   * @param clientId L'ID client Twitch
   * @param parent Le parent QObject
   */
  explicit TwitchApiClient(const QString& clientId, QObject* parent = nullptr);

  /**
   * @brief Configure le token d'accès OAuth
   * @param token Le token d'accès
   */
  void setAccessToken(const QString& token);

  /**
   * @brief Liste les streams populaires
   * @param limit Nombre maximum de streams à récupérer (défaut: 12)
   */
  Q_INVOKABLE void listStreams(int limit = 12);  // Utilise constants::twitch::kDefaultStreamListLimit

  /**
   * @brief Récupère les informations de l'utilisateur authentifié
   */
  Q_INVOKABLE void getUserInfo();

  /**
   * @brief Liste les streams suivis par un utilisateur
   * @param userId L'ID de l'utilisateur
   * @param limit Nombre maximum de streams (défaut: 100)
   */
  Q_INVOKABLE void listFollowedStreams(const QString& userId, int limit = 100);  // Utilise constants::twitch::kDefaultStreamLimit

signals:
  void streamsReady(const QVariantList& streams);
  void userInfoReady(const QString& userId);
  void errorOccurred(const QString& message);

private slots:
  void handleReply();
  void handleUserInfoReply();
  void handleFollowedStreamsReply();

private:
  QString expandThumbnail(const QString& templateUrl) const;
  QNetworkRequest buildRequest(const QUrl& url) const;
  
  // Méthodes génériques pour réduire la duplication
  bool handleNetworkError(QNetworkReply* reply, const QString& errorContext);
  QJsonDocument parseJsonResponse(QNetworkReply* reply, const QString& errorContext);
  QVariantList parseStreamsArray(const QJsonArray& entries);
  
  QString m_clientId;
  QString m_accessToken;
  QNetworkAccessManager* m_networkManager = nullptr;
};

}  // namespace blueplayer::api::twitch

