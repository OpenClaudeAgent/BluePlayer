#pragma once

#include "media/PlaybackSpeedLogic.hpp"

#include <QElapsedTimer>
#include <QImage>
#include <QMutex>
#include <QQuickPaintedItem>
#include <mpv/client.h>
#include <mpv/render.h>

namespace blueplayer::media {

class MpvQuickItem : public QQuickPaintedItem {
  Q_OBJECT

  // Media properties
  Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
  Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
  Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
  Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
  Q_PROPERTY(double position READ position NOTIFY positionChanged)
  Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
  Q_PROPERTY(bool isLiveMode READ isLiveMode NOTIFY isLiveModeChanged)
  Q_PROPERTY(double liveOffset READ liveOffset NOTIFY liveOffsetChanged)
  Q_PROPERTY(bool buffering READ buffering NOTIFY bufferingChanged)
  Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
  Q_PROPERTY(
      QString recordingPath READ recordingPath NOTIFY recordingPathChanged)
  Q_PROPERTY(double playbackRate READ playbackRate WRITE setPlaybackRate NOTIFY
                 playbackRateChanged)
  Q_PROPERTY(bool hardwareDecoding READ hardwareDecoding WRITE
                 setHardwareDecoding NOTIFY hardwareDecodingChanged)
  Q_PROPERTY(bool cropVideo READ cropVideo WRITE setCropVideo NOTIFY
                 cropVideoChanged)

public:
  explicit MpvQuickItem(QQuickItem *parent = nullptr);
  ~MpvQuickItem() override;

  void paint(QPainter *painter) override;

  // Property getters
  QString source() const { return m_source; }
  bool playing() const { return m_playing; }
  bool paused() const { return m_paused; }
  double duration() const { return m_duration.load(); }
  double position() const { return m_position.load(); }
  float volume() const { return m_volume; }
  bool muted() const { return m_muted; }
  bool isLiveMode() const { return m_isLiveMode.load(); }
  double liveOffset() const;
  bool buffering() const { return m_buffering; }
  bool isRecording() const { return m_isRecording; }
  QString recordingPath() const { return m_recordingPath; }
  double playbackRate() const { return m_playbackRate; }
  bool hardwareDecoding() const { return m_hwDecoding; }
  bool cropVideo() const { return m_cropVideo; }
  
  // Live edge detection (timing-based, not UI mode)
  // Delegates to PlaybackSpeedLogic for testability
  Q_INVOKABLE bool isNearLiveEdge() const;
  Q_INVOKABLE bool isApproachingLiveEdge() const;

  // MPV access
  mpv_handle *mpvHandle() const { return m_mpv; }
  mpv_render_context *renderContext() const { return m_renderCtx; }
  void setRenderContext(mpv_render_context *ctx) { m_renderCtx = ctx; }

  // Static callbacks
  static void onMpvEvents(void *ctx);
  static void onMpvUpdate(void *ctx);

public slots:
  void play();
  void play(const QString &url);
  void stop();
  void pause();
  void resume();
  void togglePause();
  void seek(double seconds);
  void seekToLive();
  void setSource(const QString &url);
  void setVolume(float vol);
  void setMuted(bool muted = true);
  void setPaused(bool paused);
  void startRecording(const QString &outputPath);
  void stopRecording();
  void setPlaybackRate(double rate);
  void setHardwareDecoding(bool enabled);
  void setCropVideo(bool crop);

signals:
  void sourceChanged(const QString &source);
  void playingChanged(bool playing);
  void pausedChanged(bool paused);
  void durationChanged(double duration);
  void positionChanged(double position);
  void volumeChanged(float volume);
  void mutedChanged(bool muted);
  void errorOccurred(const QString &error);
  void isLiveModeChanged(bool isLive);
  void liveOffsetChanged(double offset);
  void bufferingChanged(bool buffering);
  void recordingChanged(bool recording);
  void recordingPathChanged(const QString &path);
  void playbackRateChanged(double rate);
  void hardwareDecodingChanged(bool enabled);
  void cropVideoChanged(bool crop);
  void speedAutoReset(const QString &reason);
  void leftLiveEdge();

private slots:
  void handleMpvEvents();
  void doUpdate();

private:
  void initMpv();
  void initRenderContext();
  void destroyMpv();
  void processPropertyChange(const char *name, void *data, int format);

  mpv_handle *m_mpv = nullptr;
  mpv_render_context *m_renderCtx = nullptr;

  QString m_source;
  bool m_playing = false;
  bool m_paused = false;
  float m_volume = 1.0f;
  bool m_muted = false;
  bool m_buffering = false;

  // Recording state
  bool m_isRecording = false;
  QString m_recordingPath;

  // Use atomics for properties read from multiple threads
  std::atomic<double> m_duration{0.0};
  std::atomic<double> m_position{0.0};
  std::atomic<bool> m_isLiveMode{false};
  std::atomic<double> m_streamStart{-1.0}; // -1.0 means not observed or not a live stream
  std::atomic<bool> m_userInitiatedSeek{false}; // Track if user initiated a seek (to allow leaving live mode)
  std::atomic<bool> m_wasAtLiveEdgeBeforePause{false}; // Track if we were at live edge before pause

  // Playback tweaks
  double m_playbackRate = 1.0;
  bool m_hwDecoding = true;
  bool m_cropVideo = false;

  // Software rendering buffer
  QImage m_buffer;
  QMutex m_bufferMutex;

  // Throttle time-pos notifications to avoid UI overload
  QElapsedTimer m_posThrottle;
  double m_lastEmittedPos = -1.0;
};

} // namespace blueplayer::media
