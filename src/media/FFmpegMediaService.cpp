#include "media/FFmpegMediaService.hpp"

#include "media/FFmpegMediaSource.hpp"

#include <QUrl>
#include <QVideoSink>
#include <QDebug>
#include <QString>

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
  if (!source.isLocalFile()) {
    emit errorOccurred(tr("Les sources distantes ne sont pas (encore) prises en charge."));
    return;
  }

  const QString localPath = source.toLocalFile();
  if (localPath.isEmpty()) {
    emit errorOccurred(tr("Chemin invalide."));
    return;
  }

  if (!m_source->open(localPath)) {
    emit errorOccurred(tr("Impossible d'ouvrir le fichier."));
    return;
  }

  if (m_videoSink == nullptr) {
    emit errorOccurred(tr("Aucune cible vidéo n'est configurée."));
    return;
  }

  m_source->play();
}

void FFmpegMediaService::stop() {
  if (m_source) {
    m_source->stop();
  }
}

}  // namespace blueplayer::media

