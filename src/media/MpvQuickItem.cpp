#include "media/MpvQuickItem.hpp"

#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QByteArray>
#include <QtGlobal>
#include <QStandardPaths>
#include <QTimer>
#include <cstring>

namespace blueplayer::media {

// ============================================================================
// MpvQuickItem implementation
// ============================================================================
MpvQuickItem::MpvQuickItem(QQuickItem *parent) : QQuickPaintedItem(parent) {
  // Software rendering usually doesn't need flipping if we draw QImage directly
  // QQuickPaintedItem draws top-left to bottom-right standard.

  // Optimize for video playback
  setOpaquePainting(true);
  setAntialiasing(false);
  setPerformanceHint(QQuickPaintedItem::FastFBOResizing, true);

  initMpv();
}

MpvQuickItem::~MpvQuickItem() {
  stop();
  destroyMpv();
}

void MpvQuickItem::initMpv() {
  m_mpv = mpv_create();
  if (!m_mpv) {
    qCritical() << "[MpvQuickItem] Failed to create mpv context";
    return;
  }

  // Basic configuration
  mpv_set_option_string(m_mpv, "vo", "libmpv");
  mpv_set_option_string(m_mpv, "keep-open", "yes");
  mpv_set_option_string(m_mpv, "terminal", "yes");

  // Enable log messages
  mpv_request_log_messages(m_mpv, "debug");

  // Hardware decoding - Enable videotoolbox-copy for optimization
  // With QQuickPaintedItem (Software Rendering), 'copy' mode is ideal as it
  // utilizes GPU for decoding and copies frames to RAM, where we need them
  // anyway.
  mpv_set_option_string(m_mpv, "hwdec",
                        m_hwDecoding ? "videotoolbox-copy" : "no");
  mpv_set_option_string(m_mpv, "hwdec-codecs", "all");

  // Threading - still useful for copy/format conversion
  mpv_set_option_string(m_mpv, "vd-lavc-threads", "0");
  mpv_set_option_string(m_mpv, "ad-lavc-threads", "0");

  // Buffering configuration
  mpv_set_option_string(m_mpv, "cache", "yes");
  mpv_set_option_string(m_mpv, "cache-secs", "300");
  mpv_set_option_string(m_mpv, "demuxer-readahead-secs", "30");
  mpv_set_option_string(m_mpv, "demuxer-seekable-cache", "yes");
  mpv_set_option_string(m_mpv, "demuxer-max-bytes", "200MiB");
  mpv_set_option_string(m_mpv, "demuxer-max-back-bytes", "50MiB");
  mpv_set_option_string(m_mpv, "cache-on-disk", "no");

  mpv_set_option_string(m_mpv, "force-seekable", "yes");
  mpv_set_option_string(m_mpv, "cache-pause", "yes");
  mpv_set_option_string(m_mpv, "hr-seek", "no");
  mpv_set_option_string(m_mpv, "hr-seek-framedrop", "yes");

  // Sync alignment
  mpv_set_option_string(m_mpv, "video-sync", "audio");
  mpv_set_option_string(m_mpv, "framedrop", "no");

  // Audio/OSD
  mpv_set_option_string(m_mpv, "audio-channels", "stereo");
  mpv_set_option_string(m_mpv, "volume-max", "100");
  mpv_set_option_string(m_mpv, "osd-level", "0");
  mpv_set_option_string(m_mpv, "panscan", m_cropVideo ? "1" : "0");
  const QByteArray speedVal = QByteArray::number(m_playbackRate);
  mpv_set_option_string(m_mpv, "speed", speedVal.constData());

  // Observe properties
  mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
  mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
  mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
  mpv_observe_property(m_mpv, 0, "paused-for-cache", MPV_FORMAT_FLAG);

  if (mpv_initialize(m_mpv) < 0) {
    qCritical() << "[MpvQuickItem] Failed to initialize mpv";
    mpv_destroy(m_mpv);
    m_mpv = nullptr;
    return;
  }

  // Initialize SW render context
  initRenderContext();

  // Set up event callback
  mpv_set_wakeup_callback(m_mpv, onMpvEvents, this);

  qDebug() << "[MpvQuickItem] Initialized successfully (Software Rendering)";
}

void MpvQuickItem::initRenderContext() {
  mpv_render_param params[] = {
      {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_SW)},
      {MPV_RENDER_PARAM_INVALID, nullptr}};

  int result = mpv_render_context_create(&m_renderCtx, m_mpv, params);
  if (result < 0) {
    qCritical() << "[MpvQuickItem] Failed to create render context:"
                << mpv_error_string(result);
    return;
  }

  mpv_render_context_set_update_callback(m_renderCtx, onMpvUpdate, this);
}

void MpvQuickItem::destroyMpv() {
  if (m_renderCtx) {
    mpv_render_context_free(m_renderCtx);
    m_renderCtx = nullptr;
  }

  if (m_mpv) {
    mpv_terminate_destroy(m_mpv);
    m_mpv = nullptr;
  }
}

void MpvQuickItem::paint(QPainter *painter) {
  QMutexLocker locker(&m_bufferMutex);
  if (!m_buffer.isNull()) {
    // Draw the image scaled to fit the item
    painter->drawImage(boundingRect(), m_buffer);
  } else {
    painter->fillRect(boundingRect(), Qt::black);
  }

  // Report swap to mpv to keep timing correct
  if (m_renderCtx) {
    mpv_render_context_report_swap(m_renderCtx);
  }
}

void MpvQuickItem::onMpvUpdate(void *ctx) {
  MpvQuickItem *self = static_cast<MpvQuickItem *>(ctx);
  // Schedule render on Main Thread
  QMetaObject::invokeMethod(self, "doUpdate", Qt::QueuedConnection);
}

void MpvQuickItem::doUpdate() {
  if (!m_renderCtx)
    return;

  // Check if we need to resize buffer
  // Getting the size of the item
  int w = static_cast<int>(width());
  int h = static_cast<int>(height());

  if (w <= 0 || h <= 0)
    return;

  // Render frame to buffer
  {
    QMutexLocker locker(&m_bufferMutex);

    if (m_buffer.size() != QSize(w, h)) {
      m_buffer = QImage(w, h, QImage::Format_ARGB32);
    }

    // Prepare stride/size info
    int stride = static_cast<int>(m_buffer.bytesPerLine());
    int size[] = {w, h};

    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_SW_SIZE, size},
        {MPV_RENDER_PARAM_SW_FORMAT,
         const_cast<char *>("bgra")}, // QImage::Format_ARGB32 is actually BGRA
                                      // in memory usually
        {MPV_RENDER_PARAM_SW_STRIDE, &stride},
        {MPV_RENDER_PARAM_SW_POINTER, m_buffer.bits()},
        {MPV_RENDER_PARAM_INVALID, nullptr}};

    int result = mpv_render_context_render(m_renderCtx, params);
    if (result < 0) {
      // Don't spam logs on resize/init
      // qWarning() << "[MpvQuickItem] Render failed:" <<
      // mpv_error_string(result);
    }
  }

  // Trigger repaint
  update();
}

void MpvQuickItem::onMpvEvents(void *ctx) {
  MpvQuickItem *self = static_cast<MpvQuickItem *>(ctx);
  if (self) {
    QMetaObject::invokeMethod(self, "handleMpvEvents", Qt::QueuedConnection);
  }
}

void MpvQuickItem::handleMpvEvents() {
  if (!m_mpv)
    return;

  while (true) {
    mpv_event *event = mpv_wait_event(m_mpv, 0);
    if (event->event_id == MPV_EVENT_NONE)
      break;

    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
      mpv_event_property *prop = static_cast<mpv_event_property *>(event->data);
      processPropertyChange(prop->name, prop->data, prop->format);
      break;
    }
    case MPV_EVENT_END_FILE: {
      mpv_event_end_file *endFile =
          static_cast<mpv_event_end_file *>(event->data);
      if (endFile->reason == MPV_END_FILE_REASON_ERROR) {
        QString error = QString::fromUtf8(mpv_error_string(endFile->error));
        qCritical() << "[MpvQuickItem] Playback error:" << error;
        emit errorOccurred(error);
      }
      m_playing = false;
      emit playingChanged(false);
      break;
    }
    case MPV_EVENT_LOG_MESSAGE: {
      mpv_event_log_message *msg =
          static_cast<mpv_event_log_message *>(event->data);
      if (msg->log_level >= MPV_LOG_LEVEL_INFO) { // Only log info/warn/error
        // QString text = QString::fromUtf8(msg->text).trimmed();
        // qDebug() << "[mpv]" << msg->prefix << ":" << text;
      }
      break;
    }
    default:
      break;
    }
  }
}

void MpvQuickItem::processPropertyChange(const char *name, void *data,
                                         int format) {
  if (strcmp(name, "time-pos") == 0 && format == MPV_FORMAT_DOUBLE) {
    double pos = *static_cast<double *>(data);
    m_position = pos;
    emit positionChanged(pos);
    emit liveOffsetChanged(liveOffset());

    double dur = m_duration.load();
    if (dur > 0 && (dur - pos) < 5.0) {
      if (!m_isLiveMode.load()) {
        m_isLiveMode = true;
        emit isLiveModeChanged(true);
      }
    }
  } else if (strcmp(name, "duration") == 0 && format == MPV_FORMAT_DOUBLE) {
    double dur = *static_cast<double *>(data);
    m_duration = dur;
    emit durationChanged(dur);
  } else if (strcmp(name, "pause") == 0 && format == MPV_FORMAT_FLAG) {
    bool paused = *static_cast<int *>(data) != 0;
    m_paused = paused;
    emit pausedChanged(paused);
  } else if (strcmp(name, "paused-for-cache") == 0 &&
             format == MPV_FORMAT_FLAG) {
    bool buffering = *static_cast<int *>(data) != 0;
    m_buffering = buffering;
    emit bufferingChanged(buffering);
  }
}

// Pass-through methods unchanged logic
void MpvQuickItem::setSource(const QString &url) {
  if (m_source == url)
    return;
  m_source = url;
  emit sourceChanged(url);
}

void MpvQuickItem::play() {
  if (!m_source.isEmpty())
    play(m_source);
}

void MpvQuickItem::play(const QString &url) {
  if (!m_mpv)
    return;

  const char *stopCmd[] = {"stop", nullptr};
  mpv_command(m_mpv, stopCmd);

  if (m_source != url) {
    m_source = url;
    emit sourceChanged(url);
  }

  // Store QByteArray to keep it alive during mpv_command
  QByteArray urlBytes = url.toUtf8();
  const char *cmd[] = {"loadfile", urlBytes.constData(), nullptr};
  mpv_command(m_mpv, cmd);

  m_playing = true;
  m_paused = false;
  emit playingChanged(true);
  emit pausedChanged(false);
}

void MpvQuickItem::stop() {
  if (!m_mpv)
    return;
  const char *cmd[] = {"stop", nullptr};
  mpv_command(m_mpv, cmd);
  m_playing = false;
  m_buffering = false;
  emit playingChanged(false);
  emit bufferingChanged(false);
}

void MpvQuickItem::pause() {
  if (!m_mpv)
    return;
  int flag = 1;
  mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
}

void MpvQuickItem::resume() {
  if (!m_mpv)
    return;
  int flag = 0;
  mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
}

void MpvQuickItem::togglePause() {
  if (m_paused)
    resume();
  else
    pause();
}

void MpvQuickItem::seek(double seconds) {
  if (!m_mpv)
    return;

  if (m_isLiveMode.load()) {
    m_isLiveMode = false;
    emit isLiveModeChanged(false);
  }

  char seekVal[32];
  snprintf(seekVal, sizeof(seekVal), "%.3f", seconds);
  const char *cmd[] = {"seek", seekVal, "absolute", nullptr};
  mpv_command(m_mpv, cmd);
}

void MpvQuickItem::seekToLive() {
  if (!m_mpv)
    return;
  const char *cmd[] = {"seek", "100", "absolute-percent", nullptr};
  mpv_command(m_mpv, cmd);
  m_isLiveMode = true;
  emit isLiveModeChanged(true);
}

double MpvQuickItem::liveOffset() const {
  return m_duration.load() - m_position.load();
}

void MpvQuickItem::setVolume(float vol) {
  if (!m_mpv)
    return;
  vol = qBound(0.0f, vol, 1.0f);
  m_volume = vol;
  double mpvVol = vol * 100.0;
  mpv_set_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &mpvVol);
  emit volumeChanged(vol);
}

void MpvQuickItem::setMuted(bool muted) {
  if (!m_mpv)
    return;
  m_muted = muted;
  int flag = muted ? 1 : 0;
  mpv_set_property(m_mpv, "mute", MPV_FORMAT_FLAG, &flag);
  emit mutedChanged(muted);
}

void MpvQuickItem::setPaused(bool paused) {
  if (paused)
    pause();
  else
    resume();
}

void MpvQuickItem::startRecording(const QString &outputPath) {
  if (!m_mpv)
    return;
  mpv_set_option_string(m_mpv, "stream-record",
                        outputPath.toUtf8().constData());
  m_recordingPath = outputPath;
  m_isRecording = true;
  emit recordingPathChanged(m_recordingPath);
  emit recordingChanged(true);
}

void MpvQuickItem::stopRecording() {
  if (!m_mpv)
    return;
  mpv_set_option_string(m_mpv, "stream-record", "");
  m_isRecording = false;
  emit recordingChanged(false);
}

void MpvQuickItem::setPlaybackRate(double rate) {
  if (!m_mpv)
    return;
  double clamped = qBound(0.25, rate, 3.0);
  if (qFuzzyCompare(m_playbackRate, clamped))
    return;

  m_playbackRate = clamped;
  mpv_set_property(m_mpv, "speed", MPV_FORMAT_DOUBLE, &clamped);
  emit playbackRateChanged(clamped);
}

void MpvQuickItem::setHardwareDecoding(bool enabled) {
  if (!m_mpv)
    return;
  if (m_hwDecoding == enabled)
    return;
  m_hwDecoding = enabled;
  mpv_set_property_string(m_mpv, "hwdec",
                          enabled ? "videotoolbox-copy" : "no");
  emit hardwareDecodingChanged(enabled);
}

void MpvQuickItem::setCropVideo(bool crop) {
  if (!m_mpv)
    return;
  if (m_cropVideo == crop)
    return;
  m_cropVideo = crop;

  double panscan = crop ? 1.0 : 0.0;
  mpv_set_property(m_mpv, "panscan", MPV_FORMAT_DOUBLE, &panscan);
  emit cropVideoChanged(crop);
}

} // namespace blueplayer::media
