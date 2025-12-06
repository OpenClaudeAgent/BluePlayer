#include "media/MpvMediaSource.hpp"

#include <QDebug>
#include <QDir>
#include <QImage>
#include <QStandardPaths>
#include <QVideoFrame>
#include <QCoreApplication>
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <chrono>
#include <cstdio>
#include <cstring>

#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

namespace blueplayer::media {

MpvMediaSource::MpvMediaSource(QObject* parent) : QObject(parent) {
  initMpv();
}

MpvMediaSource::~MpvMediaSource() {
  stop();
  destroyMpv();
}

void MpvMediaSource::initMpv() {
  m_mpv = mpv_create();
  if (!m_mpv) {
    qCritical() << "[MPV] Failed to create mpv context";
    return;
  }
  
  // Configuration de base
  mpv_set_option_string(m_mpv, "vo", "libmpv");
  mpv_set_option_string(m_mpv, "keep-open", "yes");
  // Logs pour diag perf / hwdec
  mpv_set_option_string(m_mpv, "msg-level", "vd=debug,vf=debug");  // decode info
  
  // Hardware decoding - forcer VideoToolbox sur Mac
  mpv_set_option_string(m_mpv, "hwdec", "videotoolbox");
  mpv_set_option_string(m_mpv, "hwdec-codecs", "all");
  
  // Threading optimisé
  mpv_set_option_string(m_mpv, "vd-lavc-threads", "4");
  mpv_set_option_string(m_mpv, "ad-lavc-threads", "2");
  
  // Buffering pour streams live - DVR étendu avec cache disque
  mpv_set_option_string(m_mpv, "cache", "yes");
  mpv_set_option_string(m_mpv, "cache-secs", "3600");  // 1 heure de DVR possible
  mpv_set_option_string(m_mpv, "demuxer-readahead-secs", "30");  // lecture anticipée réduite
  mpv_set_option_string(m_mpv, "demuxer-seekable-cache", "yes");
  
  // Cache hybride : mémoire limitée + disque pour le reste
  mpv_set_option_string(m_mpv, "demuxer-max-bytes", "150MiB");   // RAM limitée
  mpv_set_option_string(m_mpv, "demuxer-max-back-bytes", "100MiB");  // historique RAM
  mpv_set_option_string(m_mpv, "cache-on-disk", "yes");  // Utiliser le disque
  
  // Répertoire cache disque (dans App Support)
  QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/cache";
  QDir().mkpath(cacheDir);
  mpv_set_option_string(m_mpv, "cache-dir", cacheDir.toUtf8().constData());
  qDebug() << "[MPV] Disk cache directory:" << cacheDir;
  
  mpv_set_option_string(m_mpv, "force-seekable", "yes");
  mpv_set_option_string(m_mpv, "cache-pause", "yes");
  
  // Seek sur keyframes pour éviter les artefacts verts
  mpv_set_option_string(m_mpv, "hr-seek", "no");  // seek aux keyframes, pas exact
  mpv_set_option_string(m_mpv, "hr-seek-framedrop", "yes");  // drop frames si nécessaire
  
  // Optimisations performance CPU
  mpv_set_option_string(m_mpv, "video-sync", "audio");  // sync sur audio (moins CPU)
  mpv_set_option_string(m_mpv, "framedrop", "decoder+vo");  // drop frames si CPU surchargé
  
  // HLS spécifique
  mpv_set_option_string(m_mpv, "hls-bitrate", "max");  // Meilleure qualité
  
  // Audio
  mpv_set_option_string(m_mpv, "audio-channels", "stereo");
  mpv_set_option_string(m_mpv, "volume-max", "100");
  
  // Désactiver l'OSD
  mpv_set_option_string(m_mpv, "osd-level", "0");
  
  // Observer les propriétés
  mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
  mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
  mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);
  mpv_observe_property(m_mpv, 0, "width", MPV_FORMAT_INT64);
  mpv_observe_property(m_mpv, 0, "height", MPV_FORMAT_INT64);
  mpv_observe_property(m_mpv, 0, "paused-for-cache", MPV_FORMAT_FLAG);
  
  if (mpv_initialize(m_mpv) < 0) {
    qCritical() << "[MPV] Failed to initialize mpv";
    mpv_destroy(m_mpv);
    m_mpv = nullptr;
    return;
  }
  
  // Le render context mpv OpenGL sera créé paresseusement quand le contexte GL Qt sera prêt (voir renderFrame/ensureGlContext).
  
  qDebug() << "[MPV] Initialized successfully";
}

void MpvMediaSource::destroyMpv() {
  if (m_renderCtx) {
    mpv_render_context_free(m_renderCtx);
    m_renderCtx = nullptr;
  }
  
  if (m_mpv) {
    mpv_terminate_destroy(m_mpv);
    m_mpv = nullptr;
  }
}

void MpvMediaSource::setVideoSink(QVideoSink* sink) {
  if (m_videoSink == sink) return;
  m_videoSink = sink;
  emit videoSinkChanged();
}

void MpvMediaSource::play(const QString& url) {
  if (!m_mpv) {
    emit errorOccurred("MPV not initialized");
    return;
  }
  
  qDebug() << "[MPV] Playing:" << url.left(80) << "...";
  
  // Stop any existing playback
  const char* stopCmd[] = {"stop", nullptr};
  mpv_command(m_mpv, stopCmd);
  
  // Load the URL
  const char* cmd[] = {"loadfile", url.toUtf8().constData(), nullptr};
  int result = mpv_command(m_mpv, cmd);
  
  if (result < 0) {
    QString error = QString::fromUtf8(mpv_error_string(result));
    qCritical() << "[MPV] Failed to load:" << error;
    emit errorOccurred(error);
    return;
  }
  
  m_playing = true;
  m_paused = false;
  m_stopRequested = false;
  
  emit playingChanged(true);
  emit pausedChanged(false);
  
  // Start event processing thread
  if (!m_eventThread || !m_eventThread->joinable()) {
    m_eventThread = std::make_unique<std::thread>([this]() {
      handleMpvEvents();
    });
  }
}

void MpvMediaSource::stop() {
  if (!m_mpv) return;
  
  m_stopRequested = true;
  
  const char* cmd[] = {"stop", nullptr};
  mpv_command(m_mpv, cmd);
  
  // Wait for event thread
  if (m_eventThread && m_eventThread->joinable()) {
    mpv_wakeup(m_mpv);  // Wake up the event loop
    m_eventThread->join();
  }
  m_eventThread.reset();
  
  m_playing = false;
  m_paused = false;
  emit playingChanged(false);
}

void MpvMediaSource::pause() {
  if (!m_mpv || !m_playing) return;
  
  int flag = 1;
  mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
}

void MpvMediaSource::resume() {
  if (!m_mpv || !m_playing) return;
  
  int flag = 0;
  mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
}

void MpvMediaSource::togglePause() {
  if (m_paused) {
    resume();
  } else {
    pause();
  }
}

void MpvMediaSource::seek(double seconds) {
  if (!m_mpv) return;
  
  double currentPos = m_position.load();
  qDebug() << "[MPV] Seek requested: target=" << seconds << "current=" << currentPos;
  
  // Utiliser la commande seek avec mode keyframe pour éviter les artefacts verts
  // "absolute+keyframes" cherche le keyframe le plus proche de la position demandée
  char seekVal[32];
  snprintf(seekVal, sizeof(seekVal), "%.3f", seconds);
  const char* cmd[] = {"seek", seekVal, "absolute+keyframes", nullptr};
  int result = mpv_command(m_mpv, cmd);
  
  if (result < 0) {
    qWarning() << "[MPV] Keyframe seek failed:" << mpv_error_string(result);
    
    // Fallback: seek absolu standard
    const char* fallbackCmd[] = {"seek", seekVal, "absolute", nullptr};
    result = mpv_command(m_mpv, fallbackCmd);
    if (result < 0) {
      qWarning() << "[MPV] Absolute seek failed:" << mpv_error_string(result);
      
      // Dernier recours: propriété time-pos
      result = mpv_set_property(m_mpv, "time-pos", MPV_FORMAT_DOUBLE, &seconds);
      if (result < 0) {
        qWarning() << "[MPV] time-pos seek failed:" << mpv_error_string(result);
      } else {
        qDebug() << "[MPV] time-pos seek succeeded";
      }
    } else {
      qDebug() << "[MPV] Absolute seek succeeded";
    }
  } else {
    qDebug() << "[MPV] Keyframe seek succeeded";
  }
}

void MpvMediaSource::setVolume(float vol) {
  if (!m_mpv) return;
  
  vol = qBound(0.0f, vol, 1.0f);
  m_volume = vol;
  
  double mpvVol = vol * 100.0;
  mpv_set_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &mpvVol);
  
  emit volumeChanged(vol);
}

void MpvMediaSource::setMuted(bool muted) {
  if (!m_mpv) return;
  
  m_muted = muted;
  int flag = muted ? 1 : 0;
  mpv_set_property(m_mpv, "mute", MPV_FORMAT_FLAG, &flag);
  
  emit mutedChanged(muted);
}

void MpvMediaSource::startRecording(const QString& outputPath) {
  if (!m_mpv) return;

  // Si on change de chemin alors que l'enregistrement est actif, on arrête d'abord.
  if (m_isRecording && outputPath == m_recordingPath) {
    return;
  }
  if (m_isRecording && outputPath != m_recordingPath) {
    stopRecording();
  }
  
  qDebug() << "[MPV] Starting recording to:" << outputPath;
  mpv_set_option_string(m_mpv, "stream-record", outputPath.toUtf8().constData());
  m_recordingPath = outputPath;
  m_isRecording = true;
  emit recordingPathChanged(m_recordingPath);
  emit recordingChanged(true);
}

void MpvMediaSource::stopRecording() {
  if (!m_mpv || !m_isRecording) return;
  
  qDebug() << "[MPV] Stopping recording";
  mpv_set_option_string(m_mpv, "stream-record", "");
  m_isRecording = false;
  m_recordingPath.clear();
  emit recordingPathChanged(QString());
  emit recordingChanged(false);
}

void MpvMediaSource::handleMpvEvents() {
  // Timer pour limiter le rendu à ~30fps (suffisant pour UI, économie CPU)
  auto lastRender = std::chrono::steady_clock::now();
  const auto frameInterval = std::chrono::milliseconds(33);  // ~30fps pour économiser CPU
  
  while (!m_stopRequested && m_mpv) {
    // Timeout plus long (16ms) pour réduire la charge CPU
    mpv_event* event = mpv_wait_event(m_mpv, 0.016);
    
    if (event->event_id == MPV_EVENT_NONE) {
      // Render seulement si assez de temps s'est écoulé
      auto now = std::chrono::steady_clock::now();
      if (m_renderCtx && m_videoSink && (now - lastRender) >= frameInterval) {
        renderFrame();
        lastRender = now;
      }
      continue;
    }
    
    switch (event->event_id) {
      case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property* prop = static_cast<mpv_event_property*>(event->data);
        
        if (strcmp(prop->name, "time-pos") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
          m_position = *static_cast<double*>(prop->data);
          QMetaObject::invokeMethod(this, [this]() {
            emit positionChanged(m_position);
          }, Qt::QueuedConnection);
        }
        else if (strcmp(prop->name, "duration") == 0 && prop->format == MPV_FORMAT_DOUBLE) {
          m_duration = *static_cast<double*>(prop->data);
          QMetaObject::invokeMethod(this, [this]() {
            emit durationChanged(m_duration);
          }, Qt::QueuedConnection);
        }
        else if (strcmp(prop->name, "pause") == 0 && prop->format == MPV_FORMAT_FLAG) {
          m_paused = *static_cast<int*>(prop->data) != 0;
          QMetaObject::invokeMethod(this, [this]() {
            emit pausedChanged(m_paused);
          }, Qt::QueuedConnection);
        }
        else if (strcmp(prop->name, "width") == 0 && prop->format == MPV_FORMAT_INT64) {
          m_videoWidth = static_cast<int>(*static_cast<int64_t*>(prop->data));
          qDebug() << "[MPV] Video width:" << m_videoWidth;
        }
        else if (strcmp(prop->name, "height") == 0 && prop->format == MPV_FORMAT_INT64) {
          m_videoHeight = static_cast<int>(*static_cast<int64_t*>(prop->data));
          qDebug() << "[MPV] Video height:" << m_videoHeight;
        }
        else if (strcmp(prop->name, "paused-for-cache") == 0 && prop->format == MPV_FORMAT_FLAG) {
          bool buffering = *static_cast<int*>(prop->data) != 0;
          QMetaObject::invokeMethod(this, [this, buffering]() {
            emit bufferingChanged(buffering);
          }, Qt::QueuedConnection);
        }
        break;
      }
      
      case MPV_EVENT_END_FILE: {
        mpv_event_end_file* endFile = static_cast<mpv_event_end_file*>(event->data);
        if (endFile->reason == MPV_END_FILE_REASON_ERROR) {
          QString error = QString::fromUtf8(mpv_error_string(endFile->error));
          qCritical() << "[MPV] Playback error:" << error;
          QMetaObject::invokeMethod(this, [this, error]() {
            emit errorOccurred(error);
          }, Qt::QueuedConnection);
        }
        m_playing = false;
        QMetaObject::invokeMethod(this, [this]() {
          emit playingChanged(false);
        }, Qt::QueuedConnection);
        break;
      }
      
      case MPV_EVENT_SHUTDOWN:
        m_stopRequested = true;
        break;
        
      default:
        break;
    }
  }
}

void MpvMediaSource::renderFrame() {
  if (!m_renderCtx || !m_videoSink || m_videoWidth <= 0 || m_videoHeight <= 0) {
    return;
  }
  
  // Lazy init mpv GL render context once GL context is ready
  if (!m_renderCtx) {
    if (!ensureGlContext(m_videoWidth, m_videoHeight)) {
      return;
    }
    m_glContext->makeCurrent(m_glSurface.get());
    mpv_opengl_init_params glInitParams = {
      [](void* fn_ctx, const char* name) -> void* {
        QOpenGLContext* ctx = reinterpret_cast<QOpenGLContext*>(fn_ctx);
        return reinterpret_cast<void*>(ctx->getProcAddress(QByteArray(name)));
      },
      m_glContext.get()
    };
    mpv_render_param params[] = {
      {MPV_RENDER_PARAM_API_TYPE, const_cast<char*>(MPV_RENDER_API_TYPE_OPENGL)},
      {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &glInitParams},
      {MPV_RENDER_PARAM_INVALID, nullptr}
    };
    if (mpv_render_context_create(&m_renderCtx, m_mpv, params) < 0) {
      qCritical() << "[MPV] Failed to create GL render context (lazy init)";
      m_glContext->doneCurrent();
      return;
    }
    mpv_render_context_set_update_callback(m_renderCtx, onMpvRender, this);
    m_glContext->doneCurrent();
  }

  // Check if a new frame is available
  uint64_t flags = mpv_render_context_update(m_renderCtx);
  if (!(flags & MPV_RENDER_UPDATE_FRAME)) {
    return;
  }
  
  // Assurer un contexte GL offscreen
  if (!ensureGlContext(m_videoWidth, m_videoHeight)) {
    return;
  }

  m_glContext->makeCurrent(m_glSurface.get());

  if (!m_fbo || m_fboWidth != m_videoWidth || m_fboHeight != m_videoHeight) {
    QOpenGLFramebufferObjectFormat fmt;
    fmt.setAttachment(QOpenGLFramebufferObject::NoAttachment);
    fmt.setInternalTextureFormat(GL_RGBA);
    m_fbo = std::make_unique<QOpenGLFramebufferObject>(m_videoWidth, m_videoHeight, fmt);
    m_fboWidth = m_videoWidth;
    m_fboHeight = m_videoHeight;
  }

  m_fbo->bind();

  mpv_opengl_fbo fbo = {
    .fbo = static_cast<int>(m_fbo->handle()),
    .w = m_videoWidth,
    .h = m_videoHeight,
    .internal_format = 0
  };
  int flip_y = 1;
  mpv_render_param params[] = {
    {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
    {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
    {MPV_RENDER_PARAM_INVALID, nullptr}
  };

  if (mpv_render_context_render(m_renderCtx, params) < 0) {
    m_fbo->release();
    m_glContext->doneCurrent();
    return;
  }

  // Lire les pixels dans un QImage (copie CPU) pour alimenter QVideoSink.
  // Note: pour une intégration pure GPU il faudrait un item Qt Quick spécifique.
  QImage image(m_videoWidth, m_videoHeight, QImage::Format_RGBA8888);
  m_gl->glReadPixels(0, 0, m_videoWidth, m_videoHeight, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
  m_fbo->release();
  m_glContext->doneCurrent();

  QVideoFrame frame(image.copy());
  if (m_videoSink) {
    QMetaObject::invokeMethod(m_videoSink, [sink = m_videoSink, frame]() {
      if (sink) {
        sink->setVideoFrame(frame);
      }
    }, Qt::QueuedConnection);
  }
}

void MpvMediaSource::onMpvUpdate(void* ctx) {
  // Called when mpv wants to update (unused for now)
  Q_UNUSED(ctx);
}

void MpvMediaSource::onMpvRender(void* ctx) {
  // Called when a new frame is ready
  MpvMediaSource* self = static_cast<MpvMediaSource*>(ctx);
  if (self && self->m_videoSink) {
    // Will be handled in event loop
  }
}

bool MpvMediaSource::ensureGlContext(int width, int height) {
  Q_UNUSED(width);
  Q_UNUSED(height);
  if (m_glContext && m_glSurface && m_glContext->isValid()) {
    return true;
  }
  m_glSurface = std::make_unique<QOffscreenSurface>();
  m_glSurface->setFormat(QSurfaceFormat::defaultFormat());
  m_glSurface->create();

  m_glContext = std::make_unique<QOpenGLContext>();
  m_glContext->setFormat(m_glSurface->format());
  m_glContext->setShareContext(QOpenGLContext::globalShareContext());
  if (!m_glContext->create()) {
    qCritical() << "[MPV] Failed to create GL context";
    m_glContext.reset();
    m_glSurface.reset();
    return false;
  }
  if (!m_glContext->makeCurrent(m_glSurface.get())) {
    qCritical() << "[MPV] Failed to make GL context current";
    m_glContext.reset();
    m_glSurface.reset();
    return false;
  }
  m_gl = std::make_unique<QOpenGLFunctions>();
  m_gl->initializeOpenGLFunctions();
  m_glContext->doneCurrent();
  return true;
}

}  // namespace blueplayer::media

