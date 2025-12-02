#include "api/twitch/TwitchService.hpp"

#include "api/twitch/TwitchApiClient.hpp"
#include "api/twitch/TwitchAuthManager.hpp"
#include "core/Logger.hpp"
#include "core/InputValidator.hpp"

using blueplayer::core::Logger;
using blueplayer::core::LogCategory;
using blueplayer::core::InputValidator;

#include <QVariantMap>
#include <QTimer>

namespace blueplayer::api::twitch {

TwitchService::TwitchService(QObject* parent)
    : QObject(parent),
      m_authManager(new TwitchAuthManager(this)),
      m_apiClient(new TwitchApiClient(QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID")), this)) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("Constructor called"));
  QString clientId = QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID"));
  Logger::debug(LogCategory::Twitch, QStringLiteral("Client ID: %1").arg(clientId.isEmpty() ? QStringLiteral("EMPTY") : clientId.left(10) + "..."));
  
  if (clientId.isEmpty()) {
    Logger::warning(LogCategory::Twitch, QStringLiteral("TWITCH_CLIENT_ID environment variable is not set! API calls may fail."));
  }
  
  connect(m_authManager, &TwitchAuthManager::authenticatedChanged, this, &TwitchService::onAuthStateChanged, Qt::UniqueConnection);
  connect(m_authManager, &TwitchAuthManager::accessTokenChanged, this, &TwitchService::onAccessTokenChanged, Qt::UniqueConnection);
  connect(m_authManager, &TwitchAuthManager::errorOccurred, this, &TwitchService::errorOccurred, Qt::UniqueConnection);

  connect(m_apiClient, &TwitchApiClient::streamsReady, this, &TwitchService::onStreamsReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::recommendedStreamsReady, this, &TwitchService::onRecommendedStreamsReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::categoriesReady, this, &TwitchService::onCategoriesReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::popularClipsReady, this, &TwitchService::onPopularClipsReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::followedClipsReady, this, &TwitchService::onFollowedClipsReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::videosReady, this, &TwitchService::onVideosReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::followedChannelsReady, this, &TwitchService::onFollowedChannelsReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::trendingStreamsReady, this, &TwitchService::onTrendingStreamsReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::newStreamersReady, this, &TwitchService::onNewStreamersReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::categoryStreamsReady, this, &TwitchService::onCategoryStreamsReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::userInfoReady, this, &TwitchService::onUserInfoReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::errorOccurred, this, &TwitchService::errorOccurred, Qt::UniqueConnection);
  
  Logger::debug(LogCategory::Twitch, QStringLiteral("Initial authenticated state: %1").arg(m_authManager->isAuthenticated()));
  
  // Si l'utilisateur est déjà authentifié au démarrage (tokens chargés depuis SecureStorage),
  // s'assurer que le token est défini dans TwitchApiClient
  if (m_authManager->isAuthenticated()) {
    QString token = m_authManager->accessToken();
    if (!token.isEmpty()) {
      Logger::debug(LogCategory::Twitch, QStringLiteral("User already authenticated, setting token in API client, length: %1").arg(token.length()));
      m_apiClient->setAccessToken(token);
    } else {
      Logger::warning(LogCategory::Twitch, QStringLiteral("User marked as authenticated but token is empty"));
    }
  }
}

void TwitchService::onStreamsReady(const QVariantList& streams) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] onStreamsReady() called with %1 streams").arg(streams.size()));
  m_streams = streams;
  emit streamsChanged();
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Emitted streamsChanged()"));
  if (!streams.isEmpty()) {
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Selecting first stream"));
    selectUrl(0);
  } else {
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] No streams to select"));
  }
  // refreshFollowedClips() sera appelé après que les chaînes suivies soient chargées (dans onFollowedChannelsReady)
}

void TwitchService::onRecommendedStreamsReady(const QVariantList& streams) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("onRecommendedStreamsReady() called with %1 streams").arg(streams.size()));
  m_recommendedStreams = streams;
  emit recommendedStreamsChanged();
  Logger::debug(LogCategory::Twitch, QStringLiteral("Emitted recommendedStreamsChanged()"));
}

void TwitchService::onCategoriesReady(const QVariantList& categories) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("onCategoriesReady() called with %1 categories").arg(categories.size()));
  m_categories = categories;
  emit categoriesChanged();
  Logger::debug(LogCategory::Twitch, QStringLiteral("Emitted categoriesChanged()"));
}

void TwitchService::onPopularClipsReady(const QVariantList& clips) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] onPopularClipsReady() called with %1 clips").arg(clips.size()));
  m_popularClips = clips;
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Emitting popularClipsChanged(), m_popularClips.size() = %1").arg(m_popularClips.size()));
  emit popularClipsChanged();
}

void TwitchService::onFollowedClipsReady(const QVariantList& clips) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] onFollowedClipsReady() called with %1 clips").arg(clips.size()));
  m_followedClips = clips;
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Emitting followedClipsChanged(), m_followedClips.size() = %1").arg(m_followedClips.size()));
  emit followedClipsChanged();
}

void TwitchService::onVideosReady(const QVariantList& videos) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] onVideosReady() called with %1 videos").arg(videos.size()));
  m_videos = videos;
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Emitting videosChanged(), m_videos.size() = %1").arg(m_videos.size()));
  emit videosChanged();
}

void TwitchService::onFollowedChannelsReady(const QVariantList& channels) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] onFollowedChannelsReady() called with %1 channels").arg(channels.size()));
  m_followedChannels = channels;
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Emitting followedChannelsChanged(), m_followedChannels.size() = %1").arg(m_followedChannels.size()));
  emit followedChannelsChanged();
  
  // Maintenant que les chaînes suivies sont chargées, on peut charger les clips suivis
  if (!channels.isEmpty()) {
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Followed channels loaded (%1 channels), refreshing followed clips").arg(channels.size()));
    refreshFollowedClips();
  } else {
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] No followed channels - cannot fetch followed clips"));
  }
}

void TwitchService::onTrendingStreamsReady(const QVariantList& streams) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("onTrendingStreamsReady() called with %1 streams").arg(streams.size()));
  m_trendingStreams = streams;
  emit trendingStreamsChanged();
}

void TwitchService::onNewStreamersReady(const QVariantList& streamers) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("onNewStreamersReady() called with %1 streamers").arg(streamers.size()));
  m_newStreamers = streamers;
  emit newStreamersChanged();
}

void TwitchService::onCategoryStreamsReady(const QVariantList& streams) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] onCategoryStreamsReady() called with %1 streams").arg(streams.size()));
  m_categoryStreams = streams;
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Emitting categoryStreamsChanged(), m_categoryStreams.size() = %1").arg(m_categoryStreams.size()));
  
  if (streams.isEmpty()) {
    Logger::warning(LogCategory::Twitch, QStringLiteral("[DEBUG] No category streams received"));
  } else {
    const QVariantMap firstStream = streams.first().toMap();
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] First stream: name=%1, viewers=%2").arg(firstStream.value(QStringLiteral("name")).toString()).arg(firstStream.value(QStringLiteral("viewers")).toString()));
  }
  
  emit categoryStreamsChanged();
}

void TwitchService::onUserInfoReady(const QString& userId) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] onUserInfoReady() called with userId: %1").arg(userId));
  if (m_userId != userId) {
    m_userId = userId;
    emit userIdChanged();
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] User ID changed, emitted userIdChanged()"));
  }
  // Maintenant qu'on a l'ID utilisateur, on peut récupérer les streams suivis et autres données
  if (m_apiClient) {
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Requesting followed streams for userId: %1").arg(userId));
    m_apiClient->listFollowedStreams(userId);
    // Charger les données qui nécessitent userId
    // refreshFollowedClips() sera appelé après que les streams suivis soient chargés (dans onStreamsReady)
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Loading user-specific data after userId ready"));
    refreshVideos();
    refreshFollowedChannels();
    refreshNewStreamers();
  } else {
    Logger::error(LogCategory::Twitch, QStringLiteral("API client is null"));
  }
}

void TwitchService::onAuthStateChanged(bool authenticated) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] onAuthStateChanged() called, authenticated: %1").arg(authenticated));
  emit authenticatedChanged(authenticated);
  if (authenticated && m_apiClient) {
    QString token = m_authManager->accessToken();
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Setting access token, length: %1").arg(token.length()));
    m_apiClient->setAccessToken(token);
    // Si on vient de s'authentifier, charger les streams automatiquement
    if (!m_userId.isEmpty()) {
      Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] User ID already known (%1), refreshing streams").arg(m_userId));
      refreshStreams();
      refreshRecommendedStreams();
      refreshCategories();
    } else {
      Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] User ID not known yet, will be loaded via getUserInfo"));
      // getUserInfo sera appelé par refreshStreams
      refreshStreams();
      refreshRecommendedStreams();
      refreshCategories();
    }
  }
}

void TwitchService::refreshRecommendedStreams() {
  Logger::debug(LogCategory::Twitch, QStringLiteral("refreshRecommendedStreams() called"));
  
  if (!m_apiClient) {
    Logger::error(LogCategory::Twitch, QStringLiteral("API client is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  // Les streams recommandés peuvent être chargés même sans authentification
  // mais avec authentification, on peut personnaliser les recommandations
  QString token = m_authManager ? m_authManager->accessToken() : QString();
  if (!token.isEmpty()) {
    Logger::debug(LogCategory::Twitch, QStringLiteral("Setting access token for recommended streams, length: %1").arg(token.length()));
    m_apiClient->setAccessToken(token);
  } else {
    // Même sans token, on peut récupérer les streams populaires (sans authentification)
    Logger::debug(LogCategory::Twitch, QStringLiteral("No token available, requesting public streams"));
  }

  Logger::debug(LogCategory::Twitch, QStringLiteral("Requesting recommended streams"));
  m_apiClient->getRecommendedStreams(20);  // Récupérer 20 streams recommandés
}

void TwitchService::refreshCategories() {
  Logger::debug(LogCategory::Twitch, QStringLiteral("refreshCategories() called"));
  
  if (!m_apiClient) {
    Logger::error(LogCategory::Twitch, QStringLiteral("API client is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  // Les catégories peuvent être chargées même sans authentification
  // Un token peut être utile pour personnaliser, mais n'est pas requis
  QString token = m_authManager ? m_authManager->accessToken() : QString();
  if (!token.isEmpty()) {
    Logger::debug(LogCategory::Twitch, QStringLiteral("Setting access token for categories, length: %1").arg(token.length()));
    m_apiClient->setAccessToken(token);
  }

  Logger::debug(LogCategory::Twitch, QStringLiteral("Requesting top categories"));
  m_apiClient->getTopCategories(20);  // Récupérer 20 catégories populaires
}

void TwitchService::refreshPopularClips() {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] refreshPopularClips() called"));
  
  if (!m_apiClient) {
    Logger::error(LogCategory::Twitch, QStringLiteral("[DEBUG] API client is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  QString token = m_authManager ? m_authManager->accessToken() : QString();
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Token available: %1, length: %2").arg(token.isEmpty() ? "NO" : "YES").arg(token.length()));
  if (!token.isEmpty()) {
    m_apiClient->setAccessToken(token);
  }

  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Requesting popular clips"));
  m_apiClient->getPopularClips(20);
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] getPopularClips() call completed"));
}

void TwitchService::refreshFollowedClips() {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] refreshFollowedClips() called"));
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] isAuthenticated: %1, userId: %2").arg(isAuthenticated()).arg(m_userId.isEmpty() ? "EMPTY" : m_userId));
  
  if (!m_apiClient || !m_authManager) {
    Logger::error(LogCategory::Twitch, QStringLiteral("[DEBUG] API client or auth manager is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  if (!isAuthenticated() || m_userId.isEmpty()) {
    Logger::warning(LogCategory::Twitch, QStringLiteral("[DEBUG] Not authenticated or userId unknown, clearing followed clips"));
    m_followedClips.clear();
    emit followedClipsChanged();
    return;
  }

  // Récupérer les broadcaster_ids des chaînes suivies
  // On utilise m_followedChannels car elle contient tous les streamers suivis,
  // pas seulement ceux qui sont en direct (contrairement à m_streams)
  QStringList broadcasterIds;
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Extracting broadcaster IDs from %1 followed channels").arg(m_followedChannels.size()));
  
  // Si les chaînes suivies ne sont pas encore chargées, essayer avec les streams suivis en direct
  if (m_followedChannels.isEmpty() && !m_streams.isEmpty()) {
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] No followed channels yet, using %1 live streams instead").arg(m_streams.size()));
    for (int i = 0; i < m_streams.size(); ++i) {
      const QVariant& streamVar = m_streams.at(i);
      const QVariantMap stream = streamVar.toMap();
      QString broadcasterId = stream.value(QStringLiteral("user_id")).toString();
      if (broadcasterId.isEmpty()) {
        broadcasterId = stream.value(QStringLiteral("id")).toString();
      }
      if (!broadcasterId.isEmpty()) {
        broadcasterIds.append(broadcasterId);
        Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Added broadcaster_id from stream: %1").arg(broadcasterId));
      }
    }
  } else {
    // Utiliser les chaînes suivies (contient tous les streamers suivis)
    for (int i = 0; i < m_followedChannels.size(); ++i) {
      const QVariant& channelVar = m_followedChannels.at(i);
      const QVariantMap channel = channelVar.toMap();
      
      // Les chaînes suivies utilisent "broadcaster_id" pour le broadcaster_id
      QString broadcasterId = channel.value(QStringLiteral("broadcaster_id")).toString();
      
      Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Channel %1: broadcaster_id=%2, broadcaster_name=%3").arg(i).arg(broadcasterId).arg(channel.value(QStringLiteral("broadcaster_name")).toString()));
      
      if (!broadcasterId.isEmpty()) {
        broadcasterIds.append(broadcasterId);
        Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Added broadcaster_id: %1").arg(broadcasterId));
      } else {
        Logger::warning(LogCategory::Twitch, QStringLiteral("[DEBUG] Channel %1 has no broadcaster_id").arg(i));
      }
    }
  }
  
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Found %1 broadcaster IDs from %2 channels/%3 streams").arg(broadcasterIds.size()).arg(m_followedChannels.size()).arg(m_streams.size()));

  if (broadcasterIds.isEmpty()) {
    Logger::warning(LogCategory::Twitch, QStringLiteral("[DEBUG] No broadcaster IDs found - cannot fetch followed clips"));
    m_followedClips.clear();
    emit followedClipsChanged();
    return;
  }

  QString token = m_authManager->accessToken();
  if (!token.isEmpty()) {
    m_apiClient->setAccessToken(token);
  }

  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Requesting followed clips for %1 broadcasters: %2").arg(broadcasterIds.size()).arg(broadcasterIds.join(", ")));
  m_apiClient->getFollowedClips(broadcasterIds, 5);
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] getFollowedClips() call completed"));
}

void TwitchService::refreshVideos() {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] refreshVideos() called"));
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] isAuthenticated: %1, userId: %2").arg(isAuthenticated()).arg(m_userId.isEmpty() ? "EMPTY" : m_userId));
  
  if (!m_apiClient || !m_authManager) {
    Logger::error(LogCategory::Twitch, QStringLiteral("[DEBUG] API client or auth manager is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  if (!isAuthenticated() || m_userId.isEmpty()) {
    Logger::warning(LogCategory::Twitch, QStringLiteral("[DEBUG] Not authenticated or userId unknown, clearing videos"));
    m_videos.clear();
    emit videosChanged();
    return;
  }

  QString token = m_authManager->accessToken();
  if (!token.isEmpty()) {
    m_apiClient->setAccessToken(token);
  }

  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Requesting videos for userId: %1").arg(m_userId));
  m_apiClient->getVideos(m_userId, 20);
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] getVideos() call completed"));
}

void TwitchService::refreshFollowedChannels() {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] refreshFollowedChannels() called"));
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] isAuthenticated: %1, userId: %2").arg(isAuthenticated()).arg(m_userId.isEmpty() ? "EMPTY" : m_userId));
  
  if (!m_apiClient || !m_authManager) {
    Logger::error(LogCategory::Twitch, QStringLiteral("[DEBUG] API client or auth manager is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  if (!isAuthenticated() || m_userId.isEmpty()) {
    Logger::warning(LogCategory::Twitch, QStringLiteral("[DEBUG] Not authenticated or userId unknown, clearing followed channels"));
    m_followedChannels.clear();
    emit followedChannelsChanged();
    return;
  }

  QString token = m_authManager->accessToken();
  if (!token.isEmpty()) {
    m_apiClient->setAccessToken(token);
  }

  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Requesting followed channels for userId: %1").arg(m_userId));
  m_apiClient->getFollowedChannels(m_userId);
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] getFollowedChannels() call completed"));
}

void TwitchService::refreshTrendingStreams() {
  Logger::debug(LogCategory::Twitch, QStringLiteral("refreshTrendingStreams() called"));
  
  if (!m_apiClient) {
    Logger::error(LogCategory::Twitch, QStringLiteral("API client is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  QString token = m_authManager ? m_authManager->accessToken() : QString();
  if (!token.isEmpty()) {
    m_apiClient->setAccessToken(token);
  }

  Logger::debug(LogCategory::Twitch, QStringLiteral("Requesting trending streams"));
  m_apiClient->getTrendingStreams(20);
}

void TwitchService::refreshNewStreamers() {
  Logger::debug(LogCategory::Twitch, QStringLiteral("refreshNewStreamers() called"));
  
  if (!m_apiClient || !m_authManager) {
    Logger::error(LogCategory::Twitch, QStringLiteral("API client or auth manager is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  if (!isAuthenticated() || m_userId.isEmpty()) {
    Logger::warning(LogCategory::Twitch, QStringLiteral("Not authenticated or userId unknown"));
    m_newStreamers.clear();
    emit newStreamersChanged();
    return;
  }

  QString token = m_authManager->accessToken();
  if (!token.isEmpty()) {
    m_apiClient->setAccessToken(token);
  }

  Logger::debug(LogCategory::Twitch, QStringLiteral("Requesting new streamers for userId: %1").arg(m_userId));
  m_apiClient->getNewFollowedStreamers(m_userId, 20);
}

void TwitchService::refreshCategoryStreams(const QString& gameId) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] refreshCategoryStreams() called for gameId: %1").arg(gameId));
  
  if (!m_apiClient) {
    Logger::error(LogCategory::Twitch, QStringLiteral("[DEBUG] API client is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  if (gameId.isEmpty()) {
    Logger::warning(LogCategory::Twitch, QStringLiteral("[DEBUG] Game ID is empty, clearing category streams"));
    m_categoryStreams.clear();
    emit categoryStreamsChanged();
    return;
  }

  QString token = m_authManager ? m_authManager->accessToken() : QString();
  if (!token.isEmpty()) {
    m_apiClient->setAccessToken(token);
    Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Access token set, length: %1").arg(token.length()));
  } else {
    Logger::warning(LogCategory::Twitch, QStringLiteral("[DEBUG] No access token available"));
  }

  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] Requesting streams for category: %1").arg(gameId));
  m_apiClient->getStreamsByCategory(gameId, 20);
  Logger::debug(LogCategory::Twitch, QStringLiteral("[DEBUG] getStreamsByCategory() call completed"));
}

void TwitchService::onAccessTokenChanged(const QString& token) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("onAccessTokenChanged() called, token length: %1").arg(token.length()));
  if (m_apiClient) {
    m_apiClient->setAccessToken(token);
  } else {
    Logger::error(LogCategory::Twitch, QStringLiteral("API client is null"));
  }
}

bool TwitchService::isAuthenticated() const {
  return m_authManager && m_authManager->isAuthenticated();
}

QVariantList TwitchService::streams() const {
  return m_streams;
}

QVariantList TwitchService::recommendedStreams() const {
  return m_recommendedStreams;
}

QVariantList TwitchService::categories() const {
  return m_categories;
}

QVariantList TwitchService::popularClips() const {
  return m_popularClips;
}

QVariantList TwitchService::followedClips() const {
  return m_followedClips;
}

QVariantList TwitchService::videos() const {
  return m_videos;
}

QVariantList TwitchService::followedChannels() const {
  return m_followedChannels;
}

QVariantList TwitchService::trendingStreams() const {
  return m_trendingStreams;
}

QVariantList TwitchService::newStreamers() const {
  return m_newStreamers;
}

QVariantList TwitchService::categoryStreams() const {
  return m_categoryStreams;
}

QString TwitchService::selectedStreamUrl() const {
  return m_selectedStreamUrl;
}

QString TwitchService::userId() const {
  return m_userId;
}

void TwitchService::login() {
  if (m_authManager) {
    m_authManager->login();
  }
}

void TwitchService::logout() {
  if (m_authManager) {
    m_authManager->logout();
  }
  m_streams.clear();
  emit streamsChanged();
  m_recommendedStreams.clear();
  emit recommendedStreamsChanged();
  m_categories.clear();
  emit categoriesChanged();
  m_popularClips.clear();
  emit popularClipsChanged();
  m_followedClips.clear();
  emit followedClipsChanged();
  m_videos.clear();
  emit videosChanged();
  m_followedChannels.clear();
  emit followedChannelsChanged();
  m_trendingStreams.clear();
  emit trendingStreamsChanged();
  m_newStreamers.clear();
  emit newStreamersChanged();
  m_categoryStreams.clear();
  emit categoryStreamsChanged();
  m_selectedStreamUrl.clear();
  emit selectedStreamChanged();
  m_userId.clear();
  emit userIdChanged();
}

void TwitchService::refreshStreams() {
  Logger::debug(LogCategory::Twitch, QStringLiteral("refreshStreams() called"));
  Logger::debug(LogCategory::Twitch, QStringLiteral("Authenticated: %1").arg(isAuthenticated()));
  Logger::debug(LogCategory::Twitch, QStringLiteral("Current userId: %1").arg(m_userId.isEmpty() ? QStringLiteral("EMPTY") : m_userId));
  
  if (!isAuthenticated()) {
    Logger::error(LogCategory::Twitch, QStringLiteral("Not authenticated"));
    emit errorOccurred(QStringLiteral("Authentifiez-vous d'abord."));
    return;
  }

  if (!m_apiClient) {
    Logger::error(LogCategory::Twitch, QStringLiteral("API client is null"));
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  // S'assurer que le token est défini dans TwitchApiClient avant de faire des appels
  QString token = m_authManager->accessToken();
  if (token.isEmpty()) {
    Logger::error(LogCategory::Twitch, QStringLiteral("Access token is empty"));
    emit errorOccurred(QStringLiteral("Token d'accès manquant."));
    return;
  }
  Logger::debug(LogCategory::Twitch, QStringLiteral("Ensuring access token is set in API client, length: %1").arg(token.length()));
  m_apiClient->setAccessToken(token);

  // Si on a déjà l'ID utilisateur, on peut directement récupérer les streams suivis
  if (!m_userId.isEmpty()) {
    Logger::debug(LogCategory::Twitch, QStringLiteral("User ID already known, requesting followed streams"));
    m_apiClient->listFollowedStreams(m_userId);
  } else {
    // Sinon, on récupère d'abord l'ID utilisateur
    Logger::debug(LogCategory::Twitch, QStringLiteral("User ID unknown, requesting user info first"));
    m_apiClient->getUserInfo();
  }
}

void TwitchService::playStream(int index) {
  selectUrl(index);
}

void TwitchService::selectUrl(int index) {
  // Validation de l'index
  if (index < 0 || index >= m_streams.size()) {
    Logger::error(LogCategory::Twitch, QStringLiteral("Invalid stream index: %1 (max: %2)").arg(index).arg(m_streams.size() - 1));
    emit errorOccurred(QStringLiteral("Index de stream invalide."));
    return;
  }

  const QVariantMap entry = m_streams.at(index).toMap();
  const QString url = entry.value(QStringLiteral("stream_url")).toString();
  
  // Validation de l'URL
  if (url.isEmpty() || !InputValidator::isValidUrl(url)) {
    Logger::error(LogCategory::Twitch, QStringLiteral("Invalid stream URL: %1").arg(url));
    emit errorOccurred(QStringLiteral("URL de stream invalide."));
    return;
  }

  m_selectedStreamUrl = url;
  emit selectedStreamChanged();
}

}  // namespace blueplayer::api::twitch

