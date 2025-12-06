#include "media/MpvFboItem.hpp"

#include <QDebug>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>

namespace blueplayer::media {

static void onMpvEvents(void *ctx) {
  auto *self = static_cast<MpvFboItem *>(ctx);
  if (self) {
    QMetaObject::invokeMethod(self, "handleMpvEvents", Qt::QueuedConnection);
  }
}

static void onMpvUpdate(void *ctx) {
  auto *self = static_cast<MpvFboItem *>(ctx);
  if (!self)
    return;
  // Trigger a render on the render thread
  self->update();
}

MpvFboItem::MpvFboItem(QQuickItem *parent) : QQuickFramebufferObject(parent) {
  setFlag(QQuickItem::ItemHasContents, true);
  initMpv();
}

MpvFboItem::~MpvFboItem() { destroyMpv(); }

QQuickFramebufferObject::Renderer *MpvFboItem::createRenderer() const {
  return new MpvFboRenderer(const_cast<MpvFboItem *>(this));
}

void MpvFboItem::initMpv() {
  m_mpv = mpv_create();
  if (!m_mpv) {
    qCritical() << "[MpvFboItem] Failed to create mpv context";
    return;
  }

  mpv_set_option_string(m_mpv, "vo", "gpu");
  mpv_set_option_string(m_mpv, "profile", "gpu-hq");
  mpv_set_option_string(m_mpv, "gpu-api", "opengl");
  mpv_set_option_string(m_mpv, "tone-mapping", "auto");
  mpv_set_option_string(m_mpv, "vd-lavc-dr", "yes");
  mpv_set_option_string(m_mpv, "opengl-pbo", "yes");
  mpv_set_option_string(m_mpv, "keep-open", "yes");
  mpv_set_option_string(m_mpv, "hwdec", m_hwdec.toUtf8().constData());

  mpv_request_log_messages(m_mpv, "info");

  mpv_observe_property(m_mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
  mpv_observe_property(m_mpv, 0, "duration", MPV_FORMAT_DOUBLE);
  mpv_observe_property(m_mpv, 0, "pause", MPV_FORMAT_FLAG);

  if (mpv_initialize(m_mpv) < 0) {
    qCritical() << "[MpvFboItem] Failed to initialize mpv";
    mpv_destroy(m_mpv);
    m_mpv = nullptr;
    return;
  }

  mpv_opengl_init_params gl_init_params{nullptr, nullptr, nullptr};
  mpv_render_param params[] = {
      {MPV_RENDER_PARAM_API_TYPE,
       const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
      {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
      {MPV_RENDER_PARAM_INVALID, nullptr},
  };

  int res = mpv_render_context_create(&m_renderCtx, m_mpv, params);
  if (res < 0) {
    qCritical() << "[MpvFboItem] Failed to create render context:"
                << mpv_error_string(res);
    return;
  }

  mpv_render_context_set_update_callback(m_renderCtx, onMpvUpdate, this);
  mpv_set_wakeup_callback(m_mpv, onMpvEvents, this);
}

void MpvFboItem::destroyMpv() {
  if (m_renderCtx) {
    mpv_render_context_free(m_renderCtx);
    m_renderCtx = nullptr;
  }
  if (m_mpv) {
    mpv_terminate_destroy(m_mpv);
    m_mpv = nullptr;
  }
}

void MpvFboItem::handleMpvEvents() {
  if (!m_mpv)
    return;

  while (true) {
    mpv_event *event = mpv_wait_event(m_mpv, 0);
    if (event->event_id == MPV_EVENT_NONE)
      break;

    switch (event->event_id) {
    case MPV_EVENT_PROPERTY_CHANGE: {
      auto *prop = static_cast<mpv_event_property *>(event->data);
      processPropertyChange(prop->name, prop->data, prop->format);
      break;
    }
    case MPV_EVENT_END_FILE: {
      m_playing = false;
      emit playingChanged(false);
      break;
    }
    case MPV_EVENT_LOG_MESSAGE: {
      auto *msg = static_cast<mpv_event_log_message *>(event->data);
      if (msg->event_id == MPV_EVENT_LOG_MESSAGE &&
          msg->log_level <= MPV_LOG_LEVEL_ERROR) {
        QString text = QString::fromUtf8(msg->text).trimmed();
        emit errorOccurred(text);
      }
      break;
    }
    default:
      break;
    }
  }
}

void MpvFboItem::processPropertyChange(const char *name, void *data,
                                       int format) {
  if (strcmp(name, "time-pos") == 0 && format == MPV_FORMAT_DOUBLE) {
    m_position = *static_cast<double *>(data);
    emit positionChanged(m_position);
  } else if (strcmp(name, "duration") == 0 && format == MPV_FORMAT_DOUBLE) {
    m_duration = *static_cast<double *>(data);
    emit durationChanged(m_duration);
  } else if (strcmp(name, "pause") == 0 && format == MPV_FORMAT_FLAG) {
    bool paused = *static_cast<int *>(data) != 0;
    m_paused = paused;
    emit pausedChanged(paused);
  }
}

void MpvFboItem::play() {
  if (!m_source.isEmpty())
    play(m_source);
}

void MpvFboItem::play(const QString &url) {
  if (!m_mpv)
    return;

  const char *stopCmd[] = {"stop", nullptr};
  mpv_command(m_mpv, stopCmd);

  if (m_source != url) {
    m_source = url;
    emit sourceChanged(url);
  }

  QByteArray urlBytes = url.toUtf8();
  const char *cmd[] = {"loadfile", urlBytes.constData(), nullptr};
  mpv_command(m_mpv, cmd);

  m_playing = true;
  m_paused = false;
  emit playingChanged(true);
  emit pausedChanged(false);
}

void MpvFboItem::stop() {
  if (!m_mpv)
    return;
  const char *cmd[] = {"stop", nullptr};
  mpv_command(m_mpv, cmd);
  m_playing = false;
  emit playingChanged(false);
}

void MpvFboItem::pause() {
  if (!m_mpv)
    return;
  int flag = 1;
  mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
}

void MpvFboItem::resume() {
  if (!m_mpv)
    return;
  int flag = 0;
  mpv_set_property(m_mpv, "pause", MPV_FORMAT_FLAG, &flag);
}

void MpvFboItem::togglePause() {
  if (m_paused)
    resume();
  else
    pause();
}

void MpvFboItem::seek(double seconds) {
  if (!m_mpv)
    return;
  char seekVal[32];
  snprintf(seekVal, sizeof(seekVal), "%.3f", seconds);
  const char *cmd[] = {"seek", seekVal, "absolute", nullptr};
  mpv_command(m_mpv, cmd);
}

void MpvFboItem::setSource(const QString &url) {
  if (m_source == url)
    return;
  m_source = url;
  emit sourceChanged(url);
}

void MpvFboItem::setVolume(float vol) {
  if (!m_mpv)
    return;
  vol = qBound(0.0f, vol, 1.0f);
  m_volume = vol;
  double mpvVol = vol * 100.0;
  mpv_set_property(m_mpv, "volume", MPV_FORMAT_DOUBLE, &mpvVol);
  emit volumeChanged(vol);
}

void MpvFboItem::setMuted(bool muted) {
  if (!m_mpv)
    return;
  m_muted = muted;
  int flag = muted ? 1 : 0;
  mpv_set_property(m_mpv, "mute", MPV_FORMAT_FLAG, &flag);
  emit mutedChanged(muted);
}

void MpvFboItem::setPaused(bool paused) {
  if (paused)
    pause();
  else
    resume();
}

void MpvFboItem::setHwdec(const QString &mode) {
  if (!m_mpv)
    return;
  if (m_hwdec == mode)
    return;
  m_hwdec = mode;
  ensureHwdecApplied();
  emit hwdecChanged(m_hwdec);
}

void MpvFboItem::ensureHwdecApplied() {
  if (!m_mpv)
    return;
  mpv_set_property_string(m_mpv, "hwdec", m_hwdec.toUtf8().constData());
}

// -----------------------------------------------------------------------------
// Renderer
// -----------------------------------------------------------------------------

MpvFboRenderer::MpvFboRenderer(MpvFboItem *item) : m_item(item) {}

MpvFboRenderer::~MpvFboRenderer() = default;

QOpenGLFramebufferObject *
MpvFboRenderer::createFramebufferObject(const QSize &size) {
  QOpenGLFramebufferObjectFormat fmt;
  fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
  fmt.setSamples(4);
  return new QOpenGLFramebufferObject(size, fmt);
}

void MpvFboRenderer::render() {
  if (!m_item->m_renderCtx)
    return;

  auto f = QOpenGLContext::currentContext()->functions();
  QOpenGLFramebufferObject *fbo = framebufferObject();
  if (!fbo)
    return;

  GLuint fbo_handle = fbo->handle();
  int vp[] = {0, 0, fbo->width(), fbo->height()};

  mpv_opengl_fbo mpvfbo{static_cast<int>(fbo_handle), fbo->width(),
                        fbo->height(), 0};
  int flip_y = 1;

  mpv_render_param params[] = {
      {MPV_RENDER_PARAM_OPENGL_FBO, &mpvfbo},
      {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
      {MPV_RENDER_PARAM_INVALID, nullptr},
  };

  mpv_render_context_render(m_item->m_renderCtx, params);

  f->glFlush();
  update();
}

} // namespace blueplayer::media

