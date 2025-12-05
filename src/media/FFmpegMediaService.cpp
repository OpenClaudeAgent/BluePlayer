#include "media/FFmpegMediaService.hpp"

#include "media/FFmpegMediaSource.hpp"
#include "media/MpvMediaSource.hpp"
#include "core/InputValidator.hpp"
#include "core/Logger.hpp"
#include "core/ErrorHandler.hpp"

#include <QUrl>
#include <QVideoSink>
#include <QDebug>
#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <algorithm>

using blueplayer::core::Logger;
using blueplayer::core::LogCategory;
using blueplayer::core::InputValidator;
using blueplayer::core::ErrorHandler;
using blueplayer::core::ErrorCode;

namespace blueplayer::media {

FFmpegMediaService::FFmpegMediaService(QObject* parent)
    : QObject(parent), 
      m_mpvSource(std::make_unique<MpvMediaSource>(this)),
      m_ffmpegSource(std::make_unique<FFmpegMediaSource>(this)),
      m_useMpv(true) {
  
  // Connect MPV signals
  connect(m_mpvSource.get(), &MpvMediaSource::playingChanged, this, &FFmpegMediaService::playingChanged);
  connect(m_mpvSource.get(), &MpvMediaSource::videoSinkChanged, this, &FFmpegMediaService::videoSinkChanged);
  connect(m_mpvSource.get(), &MpvMediaSource::pausedChanged, this, &FFmpegMediaService::pausedChanged);
  connect(m_mpvSource.get(), &MpvMediaSource::volumeChanged, this, &FFmpegMediaService::volumeChanged);
  connect(m_mpvSource.get(), &MpvMediaSource::mutedChanged, this, &FFmpegMediaService::mutedChanged);
  connect(m_mpvSource.get(), &MpvMediaSource::durationChanged, this, &FFmpegMediaService::durationChanged);
  connect(m_mpvSource.get(), &MpvMediaSource::positionChanged, this, &FFmpegMediaService::positionChanged);
  connect(m_mpvSource.get(), &MpvMediaSource::positionChanged, this, [this](double) { updateLiveOffset(); });
  connect(m_mpvSource.get(), &MpvMediaSource::durationChanged, this, [this](double) { updateLiveOffset(); });
  connect(m_mpvSource.get(), &MpvMediaSource::bufferingChanged, this, &FFmpegMediaService::bufferingChanged);
  connect(m_mpvSource.get(), &MpvMediaSource::errorOccurred, this, &FFmpegMediaService::errorOccurred);
  connect(m_mpvSource.get(), &MpvMediaSource::recordingChanged, this, &FFmpegMediaService::recordingChanged);
  connect(m_mpvSource.get(), &MpvMediaSource::recordingPathChanged, this, &FFmpegMediaService::recordingPathChanged);
  
  // Connect FFmpeg fallback signals (if needed)
  connect(m_ffmpegSource.get(), &FFmpegMediaSource::playingChanged, this, [this](bool playing) {
    if (!m_useMpv) emit playingChanged(playing);
  });
  connect(m_ffmpegSource.get(), &FFmpegMediaSource::pausedChanged, this, [this](bool paused) {
    if (!m_useMpv) emit pausedChanged(paused);
  });
  
  Logger::debug(LogCategory::Media, QStringLiteral("FFmpegMediaService initialized with libmpv backend"));
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
  
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->setVideoSink(sink);
  } else if (m_ffmpegSource) {
    m_ffmpegSource->setVideoSink(sink);
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
    Logger::debug(LogCategory::Media, QStringLiteral("Playing remote URL: %1").arg(sourcePath.left(80) + "..."));
  }

  if (m_videoSink == nullptr) {
    const auto error = ErrorHandler::mediaError(ErrorCode::MediaDeviceError, QStringLiteral("play"), QStringLiteral("Aucune cible vidéo n'est configurée"));
    emit errorOccurred(error.toString());
    return;
  }

  // Utiliser MPV pour les streams (HLS, HTTP) - bien meilleur buffering et A/V sync
  bool isStream = sourcePath.contains(".m3u8") || sourcePath.contains("http://") || sourcePath.contains("https://");
  
  if (isStream && m_useMpv && m_mpvSource) {
    Logger::debug(LogCategory::Media, QStringLiteral("[MPV] Playing stream via libmpv"));
    m_mpvSource->setVideoSink(m_videoSink);
    // Démarre l'enregistrement local (TS brut) avant de charger l'URL
    QString recordPath = buildRecordingPath(sourcePath);
    m_mpvSource->startRecording(recordPath);
    m_mpvSource->play(sourcePath);
    setLiveMode(true);
  } else if (m_ffmpegSource) {
    // Fallback FFmpeg pour fichiers locaux
    Logger::debug(LogCategory::Media, QStringLiteral("[FFmpeg] Playing local file"));
    m_ffmpegSource->setVideoSink(m_videoSink);
    if (m_ffmpegSource->open(sourcePath)) {
      m_ffmpegSource->play();
    } else {
      const auto error = ErrorHandler::mediaError(ErrorCode::MediaDecodeError, QStringLiteral("play"), QStringLiteral("Impossible d'ouvrir la source"));
      emit errorOccurred(error.toString());
    }
  }
}

void FFmpegMediaService::playFile(const QString& filePath) {
  play(QUrl::fromLocalFile(filePath));
}

void FFmpegMediaService::stop() {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->stopRecording();
    m_mpvSource->stop();
  }
  if (m_ffmpegSource) {
    m_ffmpegSource->stop();
  }
  m_liveOffset = 0.0;
  m_liveMode = true;
  emit liveOffsetChanged(m_liveOffset);
  emit liveModeChanged(m_liveMode);
}

void FFmpegMediaService::pause() {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->pause();
  } else if (m_ffmpegSource) {
    m_ffmpegSource->pause();
  }
}

void FFmpegMediaService::resume() {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->resume();
  } else if (m_ffmpegSource) {
    m_ffmpegSource->resume();
  }
}

void FFmpegMediaService::togglePause() {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->togglePause();
  } else if (m_ffmpegSource) {
    m_ffmpegSource->togglePause();
  }
}

bool FFmpegMediaService::isPaused() const {
  if (m_useMpv && m_mpvSource) {
    return m_mpvSource->isPaused();
  }
  return m_ffmpegSource ? m_ffmpegSource->isPaused() : false;
}

float FFmpegMediaService::volume() const {
  if (m_useMpv && m_mpvSource) {
    return m_mpvSource->volume();
  }
  return m_ffmpegSource ? m_ffmpegSource->volume() : 1.0f;
}

void FFmpegMediaService::setVolume(float vol) {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->setVolume(vol);
  }
  if (m_ffmpegSource) {
    m_ffmpegSource->setVolume(vol);
  }
}

bool FFmpegMediaService::isMuted() const {
  if (m_useMpv && m_mpvSource) {
    return m_mpvSource->isMuted();
  }
  return m_ffmpegSource ? m_ffmpegSource->isMuted() : false;
}

void FFmpegMediaService::setMuted(bool muted) {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->setMuted(muted);
  }
  if (m_ffmpegSource) {
    m_ffmpegSource->setMuted(muted);
  }
}

void FFmpegMediaService::toggleMute() {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->setMuted(!m_mpvSource->isMuted());
  } else if (m_ffmpegSource) {
    m_ffmpegSource->setMuted(!m_ffmpegSource->isMuted());
  }
}

double FFmpegMediaService::duration() const {
  if (m_useMpv && m_mpvSource) {
    return m_mpvSource->duration();
  }
  return 0.0;
}

double FFmpegMediaService::position() const {
  if (m_useMpv && m_mpvSource) {
    return m_mpvSource->position();
  }
  return 0.0;
}

void FFmpegMediaService::seek(double seconds) {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->seek(seconds);
  }
}

void FFmpegMediaService::startRecording(const QString& outputPath) {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->startRecording(outputPath);
  }
}

void FFmpegMediaService::stopRecording() {
  if (m_useMpv && m_mpvSource) {
    m_mpvSource->stopRecording();
  }
}

void FFmpegMediaService::setLiveMode(bool live) {
  if (m_liveMode == live) return;
  m_liveMode = live;
  emit liveModeChanged(m_liveMode);
}

bool FFmpegMediaService::isRecording() const {
  if (m_useMpv && m_mpvSource) {
    return m_mpvSource->isRecording();
  }
  return false;
}

QString FFmpegMediaService::recordingPath() const {
  if (m_useMpv && m_mpvSource) {
    return m_mpvSource->recordingPath();
  }
  return {};
}

void FFmpegMediaService::updateLiveOffset() {
  if (!m_useMpv || !m_mpvSource) return;
  double dur = m_mpvSource->duration();
  double pos = m_mpvSource->position();
  if (dur <= 0 || pos < 0) {
    m_liveOffset = 0.0;
  } else {
    m_liveOffset = std::max(0.0, dur - pos);
  }
  emit liveOffsetChanged(m_liveOffset);
}

QString FFmpegMediaService::buildRecordingPath(const QString& sourcePath) const {
  Q_UNUSED(sourcePath);
  QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (baseDir.isEmpty()) {
    baseDir = QDir::homePath() + "/.blueplayer";
  }
  QDir dir(baseDir + "/recordings");
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  const QString timestamp = QDateTime::currentDateTimeUtc().toString("yyyyMMdd_hhmmss");
  QString filePath = dir.filePath(QStringLiteral("stream_%1.ts").arg(timestamp));
  return filePath;
}

}  // namespace blueplayer::media
