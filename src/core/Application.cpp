#include "Application.hpp"

#include <QDebug>
#include <QTimer>

#include "api/twitch/TwitchService.hpp"
#include "media/FFmpegBridge.hpp"
#include "media/FFmpegMediaService.hpp"

namespace blueplayer::core {

Application::Application(QObject* parent)
    : QObject(parent),
      m_mediaService(std::make_unique<media::FFmpegMediaService>(this)),
      m_twitchService(std::make_unique<api::twitch::TwitchService>(this)) {}

void Application::initialize() {
  blueplayer::media::FFmpegBridge::ensureInitialized();
  qInfo() << "Initialisation BluePlayer (squelette).";
  qInfo() << "FFmpeg:" << blueplayer::media::FFmpegBridge::versionSummary();
  
  qDebug() << "[Application] initialize() called";
  qDebug() << "[Application] TwitchService exists:" << (m_twitchService != nullptr);
  
  // Si l'utilisateur est déjà authentifié, charger les streams suivis automatiquement
  if (m_twitchService && m_twitchService->isAuthenticated()) {
    qDebug() << "[Application] User is authenticated, scheduling refreshStreams()";
    // Utiliser QTimer::singleShot pour s'assurer que les signaux sont bien connectés dans QML
    QTimer::singleShot(500, [this]() {
      qDebug() << "[Application] Executing scheduled refreshStreams()";
      if (m_twitchService) {
        m_twitchService->refreshStreams();
      } else {
        qDebug() << "[Application] ERROR: TwitchService is null in timer callback";
      }
    });
  } else {
    qDebug() << "[Application] User is NOT authenticated";
    if (m_twitchService) {
      qDebug() << "[Application] Connecting to authenticatedChanged signal";
      // Connecter le signal pour charger automatiquement les streams quand l'utilisateur se connecte
      connect(m_twitchService.get(), &api::twitch::TwitchService::authenticatedChanged,
              this, [this](bool authenticated) {
                qDebug() << "[Application] authenticatedChanged signal received, authenticated:" << authenticated;
                if (authenticated && m_twitchService) {
                  qDebug() << "[Application] Calling refreshStreams() after authentication";
                  m_twitchService->refreshStreams();
                }
              });
    } else {
      qDebug() << "[Application] ERROR: TwitchService is null";
    }
  }
}

blueplayer::media::FFmpegMediaService* Application::mediaService() const {
  return m_mediaService.get();
}

blueplayer::api::twitch::TwitchService* Application::twitchService() const {
  return m_twitchService.get();
}

}  // namespace blueplayer::core

