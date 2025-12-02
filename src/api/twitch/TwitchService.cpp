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
  Logger::debug(LogCategory::Twitch, QStringLiteral("onStreamsReady() called with %1 streams").arg(streams.size()));
  m_streams = streams;
  emit streamsChanged();
  Logger::debug(LogCategory::Twitch, QStringLiteral("Emitted streamsChanged()"));
  if (!streams.isEmpty()) {
    Logger::debug(LogCategory::Twitch, QStringLiteral("Selecting first stream"));
    selectUrl(0);
  } else {
    Logger::debug(LogCategory::Twitch, QStringLiteral("No streams to select"));
  }
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

void TwitchService::onUserInfoReady(const QString& userId) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("onUserInfoReady() called with userId: %1").arg(userId));
  if (m_userId != userId) {
    m_userId = userId;
    emit userIdChanged();
    Logger::debug(LogCategory::Twitch, QStringLiteral("User ID changed, emitted userIdChanged()"));
  }
  // Maintenant qu'on a l'ID utilisateur, on peut récupérer les streams suivis
  if (m_apiClient) {
    Logger::debug(LogCategory::Twitch, QStringLiteral("Requesting followed streams for userId: %1").arg(userId));
    m_apiClient->listFollowedStreams(userId);
  } else {
    Logger::error(LogCategory::Twitch, QStringLiteral("API client is null"));
  }
}

void TwitchService::onAuthStateChanged(bool authenticated) {
  Logger::debug(LogCategory::Twitch, QStringLiteral("onAuthStateChanged() called, authenticated: %1").arg(authenticated));
  emit authenticatedChanged(authenticated);
  if (authenticated && m_apiClient) {
    QString token = m_authManager->accessToken();
    Logger::debug(LogCategory::Twitch, QStringLiteral("Setting access token, length: %1").arg(token.length()));
    m_apiClient->setAccessToken(token);
    // Si on vient de s'authentifier, charger les streams automatiquement
    if (!m_userId.isEmpty()) {
      Logger::debug(LogCategory::Twitch, QStringLiteral("User ID already known, refreshing streams"));
      refreshStreams();
      refreshRecommendedStreams();
      refreshCategories();
    } else {
      Logger::debug(LogCategory::Twitch, QStringLiteral("User ID not known yet, will be loaded via getUserInfo"));
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

