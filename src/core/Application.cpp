#include "Application.hpp"

#include <QDebug>

#include "media/FFmpegBridge.hpp"

namespace blueplayer::core {

Application::Application(QObject* parent)
    : QObject(parent) {}

void Application::initialize() {
  blueplayer::media::FFmpegBridge::ensureInitialized();
  qInfo() << "Initialisation BluePlayer (squelette).";
  qInfo() << "FFmpeg:" << blueplayer::media::FFmpegBridge::versionSummary();
}

}  // namespace blueplayer::core

