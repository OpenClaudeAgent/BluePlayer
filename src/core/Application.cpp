#include "Application.hpp"

#include <QDebug>

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
}

blueplayer::media::FFmpegMediaService* Application::mediaService() const {
  return m_mediaService.get();
}

blueplayer::api::twitch::TwitchService* Application::twitchService() const {
  return m_twitchService.get();
}

}  // namespace blueplayer::core

