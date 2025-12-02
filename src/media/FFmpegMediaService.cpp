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

  if (!source.isLocalFile()) {
    const auto error = ErrorHandler::mediaError(ErrorCode::MediaFormatNotSupported, QStringLiteral("play"), QStringLiteral("Les sources distantes ne sont pas (encore) prises en charge"));
    emit errorOccurred(error.toString());
    return;
  }

  const QString localPath = source.toLocalFile();
  
  // Validation robuste du chemin de fichier
  if (localPath.isEmpty() || !InputValidator::isValidFilePath(localPath)) {
    Logger::error(LogCategory::Media, QStringLiteral("Invalid file path: %1").arg(localPath));
    const auto error = ErrorHandler::mediaError(ErrorCode::MediaFileNotFound, QStringLiteral("play"), QStringLiteral("Chemin de fichier invalide ou fichier inexistant"));
    emit errorOccurred(error.toString());
    return;
  }

  // Sanitiser le chemin pour éviter les injections
  QString sanitizedPath = InputValidator::sanitizeString(localPath);
  if (sanitizedPath != localPath) {
    Logger::warning(LogCategory::Media, QStringLiteral("Path sanitized: %1 -> %2").arg(localPath).arg(sanitizedPath));
  }

  if (!m_source->open(sanitizedPath)) {
    Logger::error(LogCategory::Media, QStringLiteral("Failed to open file: %1").arg(sanitizedPath));
    const auto error = ErrorHandler::mediaError(ErrorCode::MediaDecodeError, QStringLiteral("play"), QStringLiteral("Impossible d'ouvrir le fichier"));
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

