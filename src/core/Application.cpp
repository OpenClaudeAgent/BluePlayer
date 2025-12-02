#include "Application.hpp"

#include <QTimer>

#include "api/twitch/TwitchService.hpp"
#include "core/Config.hpp"
#include "core/Constants.hpp"
#include "core/Logger.hpp"
#include "media/FFmpegBridge.hpp"
#include "media/FFmpegMediaService.hpp"

namespace blueplayer::core {

Application::Application(QObject* parent)
    : QObject(parent),
      m_mediaService(std::make_unique<media::FFmpegMediaService>(this)),
      m_twitchService(std::make_unique<api::twitch::TwitchService>(this)) {}

void Application::initialize() {
  Config::instance().load();
  Logger::initialize();
  
  blueplayer::media::FFmpegBridge::ensureInitialized();
  Logger::info(LogCategory::Core, QStringLiteral("Initialisation BluePlayer (squelette)."));
  Logger::info(LogCategory::Core, QStringLiteral("FFmpeg: %1").arg(blueplayer::media::FFmpegBridge::versionSummary()));
  
  Logger::debug(LogCategory::Core, QStringLiteral("initialize() called"));
  Logger::debug(LogCategory::Core, QStringLiteral("TwitchService exists: %1").arg(m_twitchService != nullptr));
  
  // Si l'utilisateur est déjà authentifié, charger les streams suivis automatiquement
  if (m_twitchService && m_twitchService->isAuthenticated()) {
    Logger::debug(LogCategory::Core, QStringLiteral("User is authenticated, scheduling refreshStreams()"));
    // Utiliser QTimer::singleShot pour s'assurer que les signaux sont bien connectés dans QML
    QTimer::singleShot(constants::media::kRefreshStreamsDelayMs, [this]() {
      Logger::debug(LogCategory::Core, QStringLiteral("Executing scheduled refreshStreams()"));
      if (m_twitchService) {
        m_twitchService->refreshStreams();
      } else {
        Logger::error(LogCategory::Core, QStringLiteral("TwitchService is null in timer callback"));
      }
    });
  } else {
    Logger::debug(LogCategory::Core, QStringLiteral("User is NOT authenticated"));
    if (m_twitchService) {
      Logger::debug(LogCategory::Core, QStringLiteral("Connecting to authenticatedChanged signal"));
      // Connecter le signal pour charger automatiquement les streams quand l'utilisateur se connecte
      connect(m_twitchService.get(), &api::twitch::TwitchService::authenticatedChanged,
              this, [this](bool authenticated) {
                Logger::debug(LogCategory::Core, QStringLiteral("authenticatedChanged signal received, authenticated: %1").arg(authenticated));
                if (authenticated && m_twitchService) {
                  Logger::debug(LogCategory::Core, QStringLiteral("Calling refreshStreams() after authentication"));
                  m_twitchService->refreshStreams();
                }
              });
    } else {
      Logger::error(LogCategory::Core, QStringLiteral("TwitchService is null"));
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

