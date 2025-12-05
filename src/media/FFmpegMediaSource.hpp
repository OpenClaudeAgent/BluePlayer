#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <QObject>
#include <QString>
#include <QVideoSink>
#include <QVideoFrame>
#include <QAudioSink>
#include <QAudioFormat>
#include <QIODevice>
#include <thread>

QT_BEGIN_NAMESPACE
class QVideoSink;
class QVideoFrame;
class QAudioSink;
QT_END_NAMESPACE

namespace blueplayer::media {

/**
 * @brief Source média FFmpeg avec buffering optimisé pour streams HLS
 * 
 * Utilise un buffer de frames pour absorber les variations de latence réseau
 * et éviter les saccades lors de la lecture de streams Twitch.
 * Supporte la lecture audio avec contrôle du volume.
 */
class FFmpegMediaSource final : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged)
  Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)

public:
  explicit FFmpegMediaSource(QObject* parent = nullptr);
  ~FFmpegMediaSource() override;

  QVideoSink* videoSink() const;
  void setVideoSink(QVideoSink* sink);
  Q_INVOKABLE bool open(QString filePath);
  Q_INVOKABLE void play();
  Q_INVOKABLE void stop();
  Q_INVOKABLE void pause();
  Q_INVOKABLE void resume();
  Q_INVOKABLE void togglePause();
  
  // État de pause
  bool isPaused() const { return m_paused.load(); }
  
  // Contrôle du volume
  float volume() const { return m_volume.load(); }
  void setVolume(float vol);
  bool isMuted() const { return m_muted.load(); }
  void setMuted(bool muted);
  
  // Configuration du buffer
  void setBufferSize(int frames) { m_bufferSize = frames; }
  int bufferSize() const { return m_bufferSize; }
  int bufferedFrames() const { return static_cast<int>(m_frameBuffer.size()); }

signals:
  void videoSinkChanged();
  void playingChanged(bool playing);
  void pausedChanged(bool paused);
  void bufferingChanged(bool buffering);
  void bufferProgress(int percent);
  void volumeChanged(float volume);
  void mutedChanged(bool muted);

private:
  void decodeLoop(QString path);
  void renderLoop();
  void audioLoop();
  void deliverFrame(const QVideoFrame& frame);
  void enqueueFrame(QVideoFrame&& frame);
  bool dequeueFrame(QVideoFrame& frame);
  
  // Audio buffer management
  void enqueueAudio(QByteArray&& audioData);
  bool dequeueAudio(QByteArray& audioData);
  void initAudioOutput(int sampleRate, int channels);

  QVideoSink* m_videoSink = nullptr;
  std::atomic<bool> m_running{false};
  std::atomic<bool> m_stopRequested{false};
  std::atomic<bool> m_buffering{true};
  std::atomic<bool> m_paused{false};
  QString m_currentFile;
  std::unique_ptr<std::thread> m_decodeThread;
  std::unique_ptr<std::thread> m_renderThread;
  std::unique_ptr<std::thread> m_audioThread;
  
  // Frame buffer pour absorber les variations de latence
  std::queue<QVideoFrame> m_frameBuffer;
  std::mutex m_bufferMutex;
  std::condition_variable m_bufferCondition;
  std::condition_variable m_pauseCondition;
  int m_bufferSize = 30;  // ~1 seconde à 30fps, ~0.5s à 60fps
  static constexpr int MIN_BUFFER_BEFORE_PLAY = 10;  // Frames minimum avant lecture
  
  // Audio output
  std::unique_ptr<QAudioSink> m_audioSink;
  QIODevice* m_audioDevice = nullptr;
  std::queue<QByteArray> m_audioBuffer;
  std::mutex m_audioMutex;
  std::condition_variable m_audioCondition;
  std::atomic<float> m_volume{1.0f};
  std::atomic<bool> m_muted{false};
  std::atomic<bool> m_hasAudio{false};
  int m_audioSampleRate = 0;
  int m_audioChannels = 0;
};

}  // namespace blueplayer::media

