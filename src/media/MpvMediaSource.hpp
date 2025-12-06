#pragma once

#include <QObject>
#include <QString>
#include <QVideoSink>
#include <QVideoFrame>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <atomic>
#include <memory>
#include <thread>

// Forward declaration
struct mpv_handle;
struct mpv_render_context;

namespace blueplayer::media {

/**
 * @brief Source média basée sur libmpv
 * 
 * Utilise libmpv pour la lecture de streams HLS avec:
 * - Buffering automatique
 * - Synchronisation A/V parfaite
 * - Support du recording (stream-record)
 * - Hardware acceleration (VideoToolbox sur Mac)
 */
class MpvMediaSource : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool playing READ isPlaying NOTIFY playingChanged)
  Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged)
  Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
  Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
  Q_PROPERTY(double position READ position NOTIFY positionChanged)
  Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)

public:
  explicit MpvMediaSource(QObject* parent = nullptr);
  ~MpvMediaSource() override;

  // Video sink
  QVideoSink* videoSink() const { return m_videoSink; }
  void setVideoSink(QVideoSink* sink);

  // Playback control
  Q_INVOKABLE void play(const QString& url);
  Q_INVOKABLE void stop();
  Q_INVOKABLE void pause();
  Q_INVOKABLE void resume();
  Q_INVOKABLE void togglePause();
  Q_INVOKABLE void seek(double seconds);
  
  // Recording (for future local copy feature)
  Q_INVOKABLE void startRecording(const QString& outputPath);
  Q_INVOKABLE void stopRecording();
  bool isRecording() const { return m_isRecording.load(); }
  QString recordingPath() const { return m_recordingPath; }
  
  // State
  bool isPlaying() const { return m_playing.load(); }
  bool isPaused() const { return m_paused.load(); }
  double duration() const { return m_duration.load(); }
  double position() const { return m_position.load(); }
  
  // Volume
  float volume() const { return m_volume.load(); }
  void setVolume(float vol);
  bool isMuted() const { return m_muted.load(); }
  void setMuted(bool muted);

signals:
  void videoSinkChanged();
  void playingChanged(bool playing);
  void pausedChanged(bool paused);
  void volumeChanged(float volume);
  void mutedChanged(bool muted);
  void durationChanged(double duration);
  void positionChanged(double position);
  void bufferingChanged(bool buffering);
  void recordingChanged(bool recording);
  void recordingPathChanged(const QString& path);
  void errorOccurred(const QString& message);

private:
  void initMpv();
  void destroyMpv();
  void handleMpvEvents();
  void renderFrame();
  static void onMpvUpdate(void* ctx);
  static void onMpvRender(void* ctx);
  bool ensureGlContext(int width, int height);

  mpv_handle* m_mpv = nullptr;
  mpv_render_context* m_renderCtx = nullptr;
  QVideoSink* m_videoSink = nullptr;
  std::unique_ptr<QOpenGLContext> m_glContext;
  std::unique_ptr<QOffscreenSurface> m_glSurface;
  std::unique_ptr<QOpenGLFramebufferObject> m_fbo;
  std::unique_ptr<QOpenGLFunctions> m_gl;
  int m_fboWidth = 0;
  int m_fboHeight = 0;
  
  std::atomic<bool> m_playing{false};
  std::atomic<bool> m_paused{false};
  std::atomic<bool> m_muted{false};
  std::atomic<float> m_volume{1.0f};
  std::atomic<double> m_duration{0.0};
  std::atomic<double> m_position{0.0};
  std::atomic<bool> m_isRecording{false};
  QString m_recordingPath;
  
  std::unique_ptr<std::thread> m_eventThread;
  std::atomic<bool> m_stopRequested{false};
  
  int m_videoWidth = 1920;
  int m_videoHeight = 1080;
};

}  // namespace blueplayer::media

