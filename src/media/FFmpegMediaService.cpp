#include "media/FFmpegMediaService.hpp"

#include "media/FFmpegMediaSource.hpp"
#include "core/InputValidator.hpp"
#include "core/Logger.hpp"
#include "core/ErrorHandler.hpp"

#include <QUrl>
#include <QVideoSink>
#include <QDebug>
#include <QString>

using blueplayer::core::Logger;
using blueplayer::core::LogCategory;
using blueplayer::core::InputValidator;
using blueplayer::core::ErrorHandler;
using blueplayer::core::ErrorCode;

namespace blueplayer::media {

FFmpegMediaService::FFmpegMediaService(QObject* parent)
    : QObject(parent), m_source(std::make_unique<FFmpegMediaSource>(this)) {
  connect(m_source.get(), &FFmpegMediaSource::playingChanged, this, &FFmpegMediaService::playingChanged);
  connect(m_source.get(), &FFmpegMediaSource::videoSinkChanged, this, &FFmpegMediaService::videoSinkChanged);
}

FFmpegMediaService::~FFmpegMediaService() = default;

QVideoSink* FFmpegMediaService::videoSink() const {
  return m_videoSink;
}

void FFmpegMediaService::setVideoSink(QVideoSink* sink) {
  if (m_videoSink == sink) {
    return;
  }

  m_videoSink = sink;
  if (m_source) {
    m_source->setVideoSink(sink);
  }

  emit videoSinkChanged();
}

void FFmpegMediaService::play(const QUrl& source) {
  // Validation de l'URL
  if (!source.isValid()) {
    Logger::error(LogCategory::Media, QStringLiteral("Invalid URL: %1").arg(source.toString()));
    const auto error = ErrorHandler::mediaError(ErrorCode::MediaFileNotFound, QStringLiteral("play"), QStringLiteral("URL invalide"));
    emit errorOccurred(error.toString());
    return;
  }

  QString sourcePath;
  
  if (source.isLocalFile()) {
    // Fichier local
    sourcePath = source.toLocalFile();
    
    // Validation robuste du chemin de fichier
    if (sourcePath.isEmpty() || !InputValidator::isValidFilePath(sourcePath)) {
      Logger::error(LogCategory::Media, QStringLiteral("Invalid file path: %1").arg(sourcePath));
      const auto error = ErrorHandler::mediaError(ErrorCode::MediaFileNotFound, QStringLiteral("play"), QStringLiteral("Chemin de fichier invalide ou fichier inexistant"));
      emit errorOccurred(error.toString());
      return;
    }

    // Sanitiser le chemin pour éviter les injections
    QString sanitizedPath = InputValidator::sanitizeString(sourcePath);
    if (sanitizedPath != sourcePath) {
      Logger::warning(LogCategory::Media, QStringLiteral("Path sanitized: %1 -> %2").arg(sourcePath).arg(sanitizedPath));
    }
    sourcePath = sanitizedPath;
  } else {
    // URL distante (HLS, HTTP, etc.)
    sourcePath = source.toString();
    Logger::debug(LogCategory::Media, QStringLiteral("Playing remote URL: %1").arg(sourcePath));
  }

  if (!m_source->open(sourcePath)) {
    Logger::error(LogCategory::Media, QStringLiteral("Failed to open source: %1").arg(sourcePath));
    const auto error = ErrorHandler::mediaError(ErrorCode::MediaDecodeError, QStringLiteral("play"), QStringLiteral("Impossible d'ouvrir la source"));
    emit errorOccurred(error.toString());
    return;
  }

  if (m_videoSink == nullptr) {
    const auto error = ErrorHandler::mediaError(ErrorCode::MediaDeviceError, QStringLiteral("play"), QStringLiteral("Aucune cible vidéo n'est configurée"));
    emit errorOccurred(error.toString());
    return;
  }

  m_source->play();
}

void FFmpegMediaService::playFile(const QString& filePath) {
  play(QUrl::fromLocalFile(filePath));
}

void FFmpegMediaService::stop() {
  if (m_source) {
    m_source->stop();
  }
}

}  // namespace blueplayer::media

