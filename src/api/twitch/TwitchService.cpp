#include "api/twitch/TwitchService.hpp"

#include "api/twitch/TwitchApiClient.hpp"
#include "api/twitch/TwitchAuthManager.hpp"

#include <QVariantMap>

namespace blueplayer::api::twitch {

TwitchService::TwitchService(QObject* parent)
    : QObject(parent),
      m_authManager(new TwitchAuthManager(this)),
      m_apiClient(new TwitchApiClient(QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID")), this)) {
  connect(m_authManager, &TwitchAuthManager::authenticatedChanged, this, [this](bool authenticated) {
    emit authenticatedChanged(authenticated);
    if (authenticated) {
      m_apiClient->setAccessToken(m_authManager->accessToken());
    }
  });

  connect(m_authManager, &TwitchAuthManager::accessTokenChanged, this, [this](const QString& token) {
    m_apiClient->setAccessToken(token);
  });
  connect(m_authManager, &TwitchAuthManager::errorOccurred, this, &TwitchService::errorOccurred);

  connect(m_apiClient, &TwitchApiClient::streamsReady, this, [this](const QVariantList& streams) {
    m_streams = streams;
    emit streamsChanged();
    selectUrl(0);
  });

  connect(m_apiClient, &TwitchApiClient::errorOccurred, this, &TwitchService::errorOccurred);

  if (m_authManager->isAuthenticated()) {
    m_apiClient->setAccessToken(m_authManager->accessToken());
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
}

void TwitchService::refreshStreams() {
  if (!isAuthenticated()) {
    emit errorOccurred(QStringLiteral("Authentifiez-vous d'abord."));
    return;
  }

  if (m_apiClient) {
    m_apiClient->listStreams();
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

