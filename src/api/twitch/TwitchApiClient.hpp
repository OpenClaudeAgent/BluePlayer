#pragma once

#include "core/network/ApiClientBase.hpp"
#include <QObject>
#include <QVariantList>

QT_BEGIN_NAMESPACE
class QNetworkReply;
class QUrl;
QT_END_NAMESPACE

namespace blueplayer::api::twitch {

/**
 * @brief Client pour l'API Twitch Helix
 * 
 * Gère les requêtes HTTP vers l'API Twitch et le parsing des réponses JSON.
 * Utilise ApiClientBase pour la gestion réseau centralisée.
 */
class TwitchApiClient : public blueplayer::core::network::ApiClientBase {
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

  /**
   * @brief Récupère les streams recommandés (streams populaires)
   * @param limit Nombre maximum de streams à récupérer (défaut: 20)
   */
  Q_INVOKABLE void getRecommendedStreams(int limit = 20);

  /**
   * @brief Récupère les catégories/jeux populaires sur Twitch
   * @param limit Nombre maximum de catégories à récupérer (défaut: 20)
   */
  Q_INVOKABLE void getTopCategories(int limit = 20);

signals:
  void streamsReady(const QVariantList& streams);
  void recommendedStreamsReady(const QVariantList& streams);
  void categoriesReady(const QVariantList& categories);
  void userInfoReady(const QString& userId);
  void errorOccurred(const QString& message);  // Gardé pour compatibilité QML

private slots:
  void handleReply();
  void handleRecommendedStreamsReply();
  void handleCategoriesReply();
  void handleUserInfoReply();
  void handleFollowedStreamsReply();

private:
  QString expandThumbnail(const QString& templateUrl) const;
  QVariantList parseStreamsArray(const QJsonArray& entries);
  
  QString m_clientId;
};

}  // namespace blueplayer::api::twitch
