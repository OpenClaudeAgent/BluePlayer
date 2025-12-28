#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <functional>

namespace blueplayer::core {
class ISecureStorage;
}

namespace blueplayer::media {
class HlsAdFilter;
}

namespace blueplayer::api::twitch {

class TwitchAuthManager;
class TwitchApiClient;

/**
 * @brief Service principal pour l'intégration Twitch
 * 
 * Gère l'authentification et la récupération des streams Twitch.
 * Expose une API simple pour QML avec les propriétés et méthodes nécessaires.
 */
class TwitchService : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticatedChanged)
  Q_PROPERTY(QVariantList streams READ streams NOTIFY streamsChanged)
  Q_PROPERTY(QVariantList recommendedStreams READ recommendedStreams NOTIFY recommendedStreamsChanged)
  Q_PROPERTY(QVariantList categories READ categories NOTIFY categoriesChanged)
  Q_PROPERTY(QVariantList popularClips READ popularClips NOTIFY popularClipsChanged)
  Q_PROPERTY(QVariantList followedClips READ followedClips NOTIFY followedClipsChanged)
  Q_PROPERTY(QVariantList videos READ videos NOTIFY videosChanged)
  Q_PROPERTY(QVariantList followedChannels READ followedChannels NOTIFY followedChannelsChanged)
  Q_PROPERTY(QVariantList newStreamers READ newStreamers NOTIFY newStreamersChanged)
  Q_PROPERTY(QVariantList categoryStreams READ categoryStreams NOTIFY categoryStreamsChanged)
  Q_PROPERTY(QVariantList searchChannelResults READ searchChannelResults NOTIFY searchChannelResultsChanged)
  Q_PROPERTY(QVariantList searchCategoryResults READ searchCategoryResults NOTIFY searchCategoryResultsChanged)
  Q_PROPERTY(QString selectedStreamUrl READ selectedStreamUrl NOTIFY selectedStreamChanged)
  Q_PROPERTY(QString userId READ userId NOTIFY userIdChanged)
  Q_PROPERTY(QString userName READ userName NOTIFY userNameChanged)
  Q_PROPERTY(QString accessToken READ accessToken NOTIFY accessTokenChanged)
  Q_PROPERTY(QVariantList availableQualities READ availableQualities NOTIFY availableQualitiesChanged)
  Q_PROPERTY(QString currentQuality READ currentQuality NOTIFY currentQualityChanged)
  Q_PROPERTY(QString defaultQuality READ defaultQuality WRITE setDefaultQuality NOTIFY defaultQualityChanged)

public:
  /**
   * @brief Constructeur avec injection de dépendances
   * @param secureStorage Stockage sécurisé à utiliser (nullptr = Keychain macOS)
   * @param parent Le parent QObject
   */
  explicit TwitchService(blueplayer::core::ISecureStorage* secureStorage = nullptr, 
                         QObject* parent = nullptr);

  /**
   * @brief Vérifie si l'utilisateur est authentifié
   * @return true si authentifié
   */
  [[nodiscard]] bool isAuthenticated() const;

  /**
   * @brief Obtient la liste des streams suivis
   * @return La liste des streams (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList streams() const;

  /**
   * @brief Obtient la liste des streams recommandés
   * @return La liste des streams recommandés (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList recommendedStreams() const;

  /**
   * @brief Obtient la liste des catégories populaires
   * @return La liste des catégories (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList categories() const;

  /**
   * @brief Obtient la liste des clips populaires
   * @return La liste des clips populaires (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList popularClips() const;

  /**
   * @brief Obtient la liste des clips des streamers suivis
   * @return La liste des clips suivis (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList followedClips() const;

  /**
   * @brief Obtient la liste des VODs
   * @return La liste des VODs (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList videos() const;

  /**
   * @brief Obtient la liste des chaînes suivies
   * @return La liste des chaînes suivies (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList followedChannels() const;

  /**
   * @brief Obtient la liste des streams tendances
   * @return La liste des streams tendances (QVariantList pour QML)
   */

  /**
   * @brief Obtient la liste des nouveaux streamers suivis
   * @return La liste des nouveaux streamers (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList newStreamers() const;

  /**
   * @brief Obtient la liste des streams par catégorie
   * @return La liste des streams par catégorie (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList categoryStreams() const;

  /**
   * @brief Obtient les résultats de recherche de chaînes
   * @return La liste des chaînes trouvées (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList searchChannelResults() const;

  /**
   * @brief Obtient les résultats de recherche de catégories
   * @return La liste des catégories trouvées (QVariantList pour QML)
   */
  [[nodiscard]] QVariantList searchCategoryResults() const;

  /**
   * @brief Obtient l'URL du stream sélectionné
   * @return L'URL du stream
   */
  [[nodiscard]] QString selectedStreamUrl() const;

  /**
   * @brief Obtient l'ID utilisateur Twitch
   * @return L'ID utilisateur
   */
  [[nodiscard]] QString userId() const;

  /**
   * @brief Obtient le nom d'utilisateur Twitch
   * @return Le nom d'utilisateur
   */
  [[nodiscard]] QString userName() const;

  /**
   * @brief Obtient le token d'accès OAuth
   * @return Le token d'accès
   */
  [[nodiscard]] QString accessToken() const;

  /**
   * @brief Obtient la liste des qualités disponibles pour le stream actuel
   * @return La liste des qualités ({name, url, bandwidth, resolution})
   */
  [[nodiscard]] QVariantList availableQualities() const;

  /**
   * @brief Obtient le nom de la qualité actuellement sélectionnée
   * @return Le nom de la qualité (ex: "1080p60", "Auto")
   */
  [[nodiscard]] QString currentQuality() const;

  /**
   * @brief Obtient la qualité par défaut configurée par l'utilisateur
   * @return Le nom de la qualité par défaut (ex: "Auto", "1080p60", "720p")
   */
  [[nodiscard]] QString defaultQuality() const;

  /**
   * @brief Définit la qualité par défaut (persistée dans les préférences)
   * @param quality Le nom de la qualité à utiliser par défaut
   */
  Q_INVOKABLE void setDefaultQuality(const QString& quality);

  /**
   * @brief Lance le processus d'authentification OAuth
   */
  Q_INVOKABLE void login();

  /**
   * @brief Déconnecte l'utilisateur et efface les tokens
   */
  Q_INVOKABLE void logout();

  /**
   * @brief Rafraîchit la liste des streams suivis
   */
  Q_INVOKABLE void refreshStreams();

  /**
   * @brief Rafraîchit la liste des streams recommandés
   */
  Q_INVOKABLE void refreshRecommendedStreams();

  /**
   * @brief Rafraîchit la liste des catégories populaires
   */
  Q_INVOKABLE void refreshCategories();

  /**
   * @brief Rafraîchit la liste des clips populaires
   */
  Q_INVOKABLE void refreshPopularClips();

  /**
   * @brief Rafraîchit la liste des clips des streamers suivis
   */
  Q_INVOKABLE void refreshFollowedClips();

  /**
   * @brief Rafraîchit la liste des VODs de l'utilisateur
   */
  Q_INVOKABLE void refreshVideos();

  /**
   * @brief Rafraîchit la liste des chaînes suivies
   */
  Q_INVOKABLE void refreshFollowedChannels();

  /**
   * @brief Rafraîchit la liste des streams tendances
   */

  /**
   * @brief Rafraîchit la liste des nouveaux streamers suivis
   */
  Q_INVOKABLE void refreshNewStreamers();

  /**
   * @brief Rafraîchit la liste des streams d'une catégorie spécifique
   * @param gameId L'ID du jeu/catégorie
   */
  Q_INVOKABLE void refreshCategoryStreams(const QString& gameId);

  /**
   * @brief Effectue une recherche Twitch (chaînes et catégories)
   * @param query Le terme de recherche
   */
  Q_INVOKABLE void search(const QString& query);

  /**
   * @brief Efface les résultats de recherche
   */
  Q_INVOKABLE void clearSearchResults();

  /**
   * @brief Sélectionne et prépare un stream pour la lecture
   * @param index L'index du stream dans la liste
   */
  Q_INVOKABLE void playStream(int index);

  /**
   * @brief Obtient l'URL HLS pour un stream Twitch
   * @param streamerLogin Le login du streamer
   */
  Q_INVOKABLE void getStreamHlsUrl(const QString& streamerLogin);
  
  /**
   * @brief Obtient l'URL HLS actuelle (après appel à getStreamHlsUrl)
   * @return L'URL HLS ou QString vide si non disponible
   */
  Q_INVOKABLE QString currentHlsUrl() const;

  /**
   * @brief Change la qualité du stream actuel
   * @param qualityName Le nom de la qualité (ex: "1080p60", "720p", "Auto")
   */
  Q_INVOKABLE void setStreamQuality(const QString& qualityName);

signals:
  void authenticatedChanged(bool authenticated);
  void streamsChanged();
  void recommendedStreamsChanged();
  void categoriesChanged();
  void popularClipsChanged();
  void followedClipsChanged();
  void videosChanged();
  void followedChannelsChanged();
  void newStreamersChanged();
  void categoryStreamsChanged();
  void searchChannelResultsChanged();
  void searchCategoryResultsChanged();
  void selectedStreamChanged();
  void userIdChanged();
  void userNameChanged();
  void accessTokenChanged();
  void hlsUrlReady(const QString& url);
  void errorOccurred(const QString& message);
  void adsDetected(int segmentCount);
  void adsFinished();
  void adFilterLog(const QString& message);
  void availableQualitiesChanged();
  void currentQualityChanged();
  void defaultQualityChanged();
  void qualityChanged(const QString& url);

private:
  void selectUrl(int index);
  void fetchAndSelectBestQuality(const QString& masterPlaylistUrl);
  QString selectBestQualityFromPlaylist(const QString& playlistContent);
  
  /**
   * @brief Helper pour exécuter une action avec le token d'authentification
   * 
   * Ce helper encapsule le pattern répétitif de vérification et configuration
   * du token d'accès avant d'exécuter une action sur l'API.
   * 
   * @param action La fonction à exécuter si le token est valide
   * @return true si l'action a été exécutée, false si le token est vide
   */
  bool ensureTokenAndExecute(std::function<void()> action);

private slots:
  void onAuthStateChanged(bool authenticated);
  void onAccessTokenChanged(const QString& token);
  void onStreamsReady(const QVariantList& streams);
  void onRecommendedStreamsReady(const QVariantList& streams);
  void onCategoriesReady(const QVariantList& categories);
  void onPopularClipsReady(const QVariantList& clips);
  void onFollowedClipsReady(const QVariantList& clips);
  void onVideosReady(const QVariantList& videos);
  void onFollowedChannelsReady(const QVariantList& channels);
  void onNewStreamersReady(const QVariantList& streamers);
  void onCategoryStreamsReady(const QVariantList& streams);
  void onSearchChannelsReady(const QVariantList& channels);
  void onSearchCategoriesReady(const QVariantList& categories);
  void onUserInfoReady(const QString& userId);
  void onUserInfoReadyWithName(const QString& userId, const QString& userName);
  void onPlaybackAccessTokenReady(const QString& token, const QString& sig);
  void onTokenInvalidated();
  void onAdFilterCleanStream(const QString& url);
  void onAdFilterAdsDetected(int count);
  void onAdFilterAdsFinished();
  void onAdFilterDebugLog(const QString& message);
  void onAdFilterRequestNewToken();
  void onAdFilterMaxRetries(const QString& url);

private:
  TwitchAuthManager* m_authManager = nullptr;
  TwitchApiClient* m_apiClient = nullptr;
  blueplayer::media::HlsAdFilter* m_adFilter = nullptr;
  QVariantList m_streams;
  QVariantList m_recommendedStreams;
  QVariantList m_categories;
  QVariantList m_popularClips;
  QVariantList m_followedClips;
  QVariantList m_videos;
  QVariantList m_followedChannels;
  QVariantList m_newStreamers;
  QVariantList m_categoryStreams;
  QVariantList m_searchChannelResults;
  QVariantList m_searchCategoryResults;
  QString m_selectedStreamUrl;
  QString m_userId;
  QString m_userName;
  QString m_currentHlsUrl;
  QString m_pendingStreamerLogin;
  QVariantList m_availableQualities;
  QString m_currentQuality;
};

}  // namespace blueplayer::api::twitch

