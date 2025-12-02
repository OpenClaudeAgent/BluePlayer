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

  /**
   * @brief Récupère les clips populaires globaux
   * @param limit Nombre maximum de clips à récupérer (défaut: 20)
   */
  Q_INVOKABLE void getPopularClips(int limit = 20);

  /**
   * @brief Récupère les clips des streamers suivis
   * @param broadcasterIds Liste des IDs des streamers suivis
   * @param limit Nombre maximum de clips par streamer (défaut: 5)
   */
  Q_INVOKABLE void getFollowedClips(const QStringList& broadcasterIds, int limit = 5);

  /**
   * @brief Récupère les VODs (vidéos archivées) d'un utilisateur
   * @param userId L'ID de l'utilisateur
   * @param limit Nombre maximum de VODs à récupérer (défaut: 20)
   */
  Q_INVOKABLE void getVideos(const QString& userId, int limit = 20);

  /**
   * @brief Récupère les chaînes suivies par un utilisateur
   * @param userId L'ID de l'utilisateur
   */
  Q_INVOKABLE void getFollowedChannels(const QString& userId);

  /**
   * @brief Récupère les streams tendances (triés par viewers décroissant)
   * @param limit Nombre maximum de streams à récupérer (défaut: 20)
   */
  Q_INVOKABLE void getTrendingStreams(int limit = 20);

  /**
   * @brief Récupère les nouveaux streamers suivis récemment
   * @param userId L'ID de l'utilisateur
   * @param limit Nombre maximum de streamers à récupérer (défaut: 20)
   */
  Q_INVOKABLE void getNewFollowedStreamers(const QString& userId, int limit = 20);

  /**
   * @brief Récupère les streams d'une catégorie spécifique
   * @param gameId L'ID du jeu/catégorie
   * @param limit Nombre maximum de streams à récupérer (défaut: 20)
   */
  Q_INVOKABLE void getStreamsByCategory(const QString& gameId, int limit = 20);

signals:
  void streamsReady(const QVariantList& streams);
  void recommendedStreamsReady(const QVariantList& streams);
  void categoriesReady(const QVariantList& categories);
  void popularClipsReady(const QVariantList& clips);
  void followedClipsReady(const QVariantList& clips);
  void videosReady(const QVariantList& videos);
  void followedChannelsReady(const QVariantList& channels);
  void trendingStreamsReady(const QVariantList& streams);
  void newStreamersReady(const QVariantList& streamers);
  void categoryStreamsReady(const QVariantList& streams);
  void userInfoReady(const QString& userId);
  void errorOccurred(const QString& message);  // Gardé pour compatibilité QML

private slots:
  void handleReply();
  void handleRecommendedStreamsReply();
  void handleCategoriesReply();
  void handlePopularClipsReply();
  void handleFollowedClipsReply();
  void handleVideosReply();
  void handleFollowedChannelsReply();
  void handleTrendingStreamsReply();
  void handleNewStreamersReply();
  void handleCategoryStreamsReply();
  void handleUserInfoReply();
  void handleFollowedStreamsReply();
  void handleUsersInfoReply();

private:
  QString expandThumbnail(const QString& templateUrl) const;
  QVariantList parseStreamsArray(const QJsonArray& entries);
  QVariantList parseClipsArray(const QJsonArray& entries);
  QVariantList parseVideosArray(const QJsonArray& entries);
  QVariantList parseChannelsArray(const QJsonArray& entries);
  void getUsersInfo(const QStringList& userIds);
  
  QString m_clientId;
  QVariantList m_pendingChannels;  // Stocke temporairement les chaînes en attendant les avatars
};

}  // namespace blueplayer::api::twitch
