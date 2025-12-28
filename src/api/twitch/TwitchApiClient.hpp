#pragma once

#include "core/network/ApiClientBase.hpp"
#include "core/network/CurlHttpClient.hpp"
#include <QObject>
#include <QVariantList>
#include <memory>

QT_BEGIN_NAMESPACE
class QNetworkReply;
class QUrl;
QT_END_NAMESPACE

// Forward declaration for test access
class TestTwitchApiClient;

namespace blueplayer::api::twitch {

/**
 * @brief Client pour l'API Twitch Helix
 * 
 * Gère les requêtes HTTP vers l'API Twitch et le parsing des réponses JSON.
 * Utilise ApiClientBase pour la gestion réseau centralisée.
 */
class TwitchApiClient : public blueplayer::core::network::ApiClientBase {
  Q_OBJECT

  // Friend class for testing private methods
  friend class ::TestTwitchApiClient;

public:
  /**
   * @brief Constructeur
   * @param clientId L'ID client Twitch
   * @param parent Le parent QObject
   */
  explicit TwitchApiClient(const QString& clientId, QObject* parent = nullptr);
  
  /**
   * @brief Constructeur de test avec injection de dépendances
   * @param clientId L'ID client Twitch
   * @param httpClient Client HTTP injecté pour les tests
   * @param parent Le parent QObject
   */
  explicit TwitchApiClient(const QString& clientId, 
                           blueplayer::core::network::IHttpClient* httpClient,
                           QObject* parent = nullptr);

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
  Q_INVOKABLE void getFollowedChannels(const QString& userId, int limit = 100);

  /**
   * @brief Récupère les streams tendances (triés par viewers décroissant)
   * @param limit Nombre maximum de streams à récupérer (défaut: 20)
   */

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

  /**
   * @brief Obtient le PlaybackAccessToken pour un stream via GraphQL
   * @param streamerLogin Le login du streamer
   */
  void getPlaybackAccessToken(const QString& streamerLogin);

  /**
   * @brief Recherche des chaînes par nom
   * @param query Le terme de recherche
   * @param limit Nombre maximum de résultats (défaut: 10)
   */
  Q_INVOKABLE void searchChannels(const QString& query, int limit = 10);

  /**
   * @brief Recherche des catégories/jeux par nom
   * @param query Le terme de recherche
   * @param limit Nombre maximum de résultats (défaut: 10)
   */
  Q_INVOKABLE void searchCategories(const QString& query, int limit = 10);

signals:
  void streamsReady(const QVariantList& streams);
  void recommendedStreamsReady(const QVariantList& streams);
  void categoriesReady(const QVariantList& categories);
  void popularClipsReady(const QVariantList& clips);
  void followedClipsReady(const QVariantList& clips);
  void videosReady(const QVariantList& videos);
  void followedChannelsReady(const QVariantList& channels);
  void newStreamersReady(const QVariantList& streamers);
  void categoryStreamsReady(const QVariantList& streams);
  void userInfoReady(const QString& userId);
  void userInfoReadyWithName(const QString& userId, const QString& userName);
  void playbackAccessTokenReady(const QString& token, const QString& sig);
  void searchChannelsReady(const QVariantList& channels);
  void searchCategoriesReady(const QVariantList& categories);
  void errorOccurred(const QString& message);  // Gardé pour compatibilité QML
  void tokenInvalidated();  // Émis quand le token est invalide (Client-ID mismatch)

private slots:
  void handleReply();
  void handleRecommendedStreamsReply();
  void handleCategoriesReply();
  void handlePopularClipsReply();
  void handleFollowedClipsReply();
  void handleVideosReply();
  void handleFollowedChannelsReply();
  void handleCategoryStreamsReply();
  void handleUserInfoReply();
  void handleFollowedStreamsReply();
  void handleUsersInfoReply();
  void handlePlaybackAccessTokenReply();
  void handlePlaybackAccessTokenResponse(const QJsonDocument& document, const QString& streamerLogin);
  void handleSearchChannelsReply();
  void handleSearchCategoriesReply();

private:
  /**
   * @brief Construit une URL Helix en utilisant la base URL configurée
   * @param endpoint Le chemin de l'endpoint (ex: "/helix/streams")
   * @return L'URL complète
   */
  QUrl buildHelixUrl(const QString& endpoint) const;
  
  QString expandThumbnail(const QString& templateUrl) const;
  QVariantList parseStreamsArray(const QJsonArray& entries);
  QVariantList parseClipsArray(const QJsonArray& entries);
  QVariantList parseVideosArray(const QJsonArray& entries);
  QVariantList parseChannelsArray(const QJsonArray& entries);
  QVariantList parseSearchChannelsArray(const QJsonArray& entries);
  QVariantList parseSearchCategoriesArray(const QJsonArray& entries);
  void getUsersInfo(const QStringList& userIds);
  
  QString m_clientId;
  
  // CurlHttpClient for GraphQL requests (preserves header case)
  std::unique_ptr<blueplayer::core::network::CurlHttpClient> m_curlClient;
  QVariantList m_pendingChannels;  // Stocke temporairement les chaînes en attendant les avatars
  QVariantList m_pendingChannelsForNewStreamers;  // Stocke les entries JSON originales converties en QVariantList pour récupérer followed_at
  void emitNewStreamersFromChannels(const QVariantList& channels, const QVariantList& entries);
};

}  // namespace blueplayer::api::twitch
