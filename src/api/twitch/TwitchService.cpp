#include "api/twitch/TwitchService.hpp"

#include "api/twitch/TwitchApiClient.hpp"
#include "api/twitch/TwitchAuthManager.hpp"

#include <QDebug>
#include <QVariantMap>

namespace blueplayer::api::twitch {

TwitchService::TwitchService(QObject* parent)
    : QObject(parent),
      m_authManager(new TwitchAuthManager(this)),
      m_apiClient(new TwitchApiClient(QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID")), this)) {
  qDebug() << "[TwitchService] Constructor called";
  QString clientId = QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID"));
  qDebug() << "[TwitchService] Client ID:" << (clientId.isEmpty() ? "EMPTY" : clientId.left(10) + "...");
  
  connect(m_authManager, &TwitchAuthManager::authenticatedChanged, this, &TwitchService::onAuthStateChanged, Qt::UniqueConnection);
  connect(m_authManager, &TwitchAuthManager::accessTokenChanged, this, &TwitchService::onAccessTokenChanged, Qt::UniqueConnection);
  connect(m_authManager, &TwitchAuthManager::errorOccurred, this, &TwitchService::errorOccurred, Qt::UniqueConnection);

  connect(m_apiClient, &TwitchApiClient::streamsReady, this, &TwitchService::onStreamsReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::userInfoReady, this, &TwitchService::onUserInfoReady, Qt::UniqueConnection);
  connect(m_apiClient, &TwitchApiClient::errorOccurred, this, &TwitchService::errorOccurred, Qt::UniqueConnection);
  
  qDebug() << "[TwitchService] Initial authenticated state:" << m_authManager->isAuthenticated();
}

void TwitchService::onStreamsReady(const QVariantList& streams) {
  qDebug() << "[TwitchService] onStreamsReady() called with" << streams.size() << "streams";
  m_streams = streams;
  emit streamsChanged();
  qDebug() << "[TwitchService] Emitted streamsChanged()";
  if (!streams.isEmpty()) {
    qDebug() << "[TwitchService] Selecting first stream";
    selectUrl(0);
  } else {
    qDebug() << "[TwitchService] No streams to select";
  }
}

void TwitchService::onUserInfoReady(const QString& userId) {
  qDebug() << "[TwitchService] onUserInfoReady() called with userId:" << userId;
  if (m_userId != userId) {
    m_userId = userId;
    emit userIdChanged();
    qDebug() << "[TwitchService] User ID changed, emitted userIdChanged()";
  }
  // Maintenant qu'on a l'ID utilisateur, on peut récupérer les streams suivis
  if (m_apiClient) {
    qDebug() << "[TwitchService] Requesting followed streams for userId:" << userId;
    m_apiClient->listFollowedStreams(userId);
  } else {
    qDebug() << "[TwitchService] ERROR: API client is null";
  }
}

void TwitchService::onAuthStateChanged(bool authenticated) {
  qDebug() << "[TwitchService] onAuthStateChanged() called, authenticated:" << authenticated;
  emit authenticatedChanged(authenticated);
  if (authenticated && m_apiClient) {
    QString token = m_authManager->accessToken();
    qDebug() << "[TwitchService] Setting access token, length:" << token.length();
    m_apiClient->setAccessToken(token);
  }
}

void TwitchService::onAccessTokenChanged(const QString& token) {
  qDebug() << "[TwitchService] onAccessTokenChanged() called, token length:" << token.length();
  if (m_apiClient) {
    m_apiClient->setAccessToken(token);
  } else {
    qDebug() << "[TwitchService] ERROR: API client is null";
  }
}

bool TwitchService::isAuthenticated() const {
  return m_authManager && m_authManager->isAuthenticated();
}

QVariantList TwitchService::streams() const {
  return m_streams;
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
  m_selectedStreamUrl.clear();
  emit selectedStreamChanged();
  m_userId.clear();
  emit userIdChanged();
}

void TwitchService::refreshStreams() {
  qDebug() << "[TwitchService] refreshStreams() called";
  qDebug() << "[TwitchService] Authenticated:" << isAuthenticated();
  qDebug() << "[TwitchService] Current userId:" << (m_userId.isEmpty() ? "EMPTY" : m_userId);
  
  if (!isAuthenticated()) {
    qDebug() << "[TwitchService] ERROR: Not authenticated";
    emit errorOccurred(QStringLiteral("Authentifiez-vous d'abord."));
    return;
  }

  if (!m_apiClient) {
    qDebug() << "[TwitchService] ERROR: API client is null";
    emit errorOccurred(QStringLiteral("Client API non initialisé."));
    return;
  }

  // Si on a déjà l'ID utilisateur, on peut directement récupérer les streams suivis
  if (!m_userId.isEmpty()) {
    qDebug() << "[TwitchService] User ID already known, requesting followed streams";
    m_apiClient->listFollowedStreams(m_userId);
  } else {
    // Sinon, on récupère d'abord l'ID utilisateur
    qDebug() << "[TwitchService] User ID unknown, requesting user info first";
    m_apiClient->getUserInfo();
  }
}

void TwitchService::playStream(int index) {
  selectUrl(index);
}

void TwitchService::selectUrl(int index) {
  if (index < 0 || index >= m_streams.size()) {
    emit errorOccurred(QStringLiteral("Index de stream invalide."));
    return;
  }

  const QVariantMap entry = m_streams.at(index).toMap();
  const QString url = entry.value(QStringLiteral("stream_url")).toString();
  if (url.isEmpty()) {
    emit errorOccurred(QStringLiteral("URL de stream manquante."));
    return;
  }

  m_selectedStreamUrl = url;
  emit selectedStreamChanged();
}

}  // namespace blueplayer::api::twitch

