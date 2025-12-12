#pragma once

#include <QQuickFramebufferObject>
#include <mpv/client.h>
#include <mpv/render_gl.h>

namespace blueplayer::media {

class MpvFboRenderer;

class MpvFboItem : public QQuickFramebufferObject {
  Q_OBJECT
  Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
  Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
  Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
  Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
  Q_PROPERTY(double position READ position NOTIFY positionChanged)
  Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
  Q_PROPERTY(QString hwdec READ hwdec WRITE setHwdec NOTIFY hwdecChanged)

public:
  explicit MpvFboItem(QQuickItem *parent = nullptr);
  ~MpvFboItem() override;

  Renderer *createRenderer() const override;

  QString source() const { return m_source; }
  bool playing() const { return m_playing; }
  bool paused() const { return m_paused; }
  double duration() const { return m_duration; }
  double position() const { return m_position; }
  float volume() const { return m_volume; }
  bool muted() const { return m_muted; }
  QString hwdec() const { return m_hwdec; }

public slots:
  void play();
  void play(const QString &url);
  void stop();
  void pause();
  void resume();
  void togglePause();
  void seek(double seconds);
  void setSource(const QString &url);
  void setVolume(float vol);
  void setMuted(bool muted = true);
  void setPaused(bool paused);
  void setHwdec(const QString &mode);

signals:
  void sourceChanged(const QString &source);
  void playingChanged(bool playing);
  void pausedChanged(bool paused);
  void durationChanged(double duration);
  void positionChanged(double position);
  void volumeChanged(float volume);
  void mutedChanged(bool muted);
  void hwdecChanged(const QString &mode);
  void errorOccurred(const QString &error);

private:
  friend class MpvFboRenderer;
  void handleMpvEvents();
  void processPropertyChange(const char *name, void *data, int format);
  void initMpv();
  void destroyMpv();
  void initRenderContextIfNeeded();
  void ensureHwdecApplied();

  mpv_handle *m_mpv = nullptr;
  mpv_render_context *m_renderCtx = nullptr;

  QString m_source;
  bool m_playing = false;
  bool m_paused = false;
  float m_volume = 1.0f;
  bool m_muted = false;
  double m_duration = 0.0;
  double m_position = 0.0;
  QString m_hwdec = QStringLiteral("auto");
};

class MpvFboRenderer : public QQuickFramebufferObject::Renderer {
public:
  explicit MpvFboRenderer(MpvFboItem *item);
  ~MpvFboRenderer() override;

  void render() override;
  QOpenGLFramebufferObject *createFramebufferObject(
      const QSize &size) override;

private:
  MpvFboItem *m_item;
};

} // namespace blueplayer::media

