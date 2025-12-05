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
#include <thread>

QT_BEGIN_NAMESPACE
class QVideoSink;
class QVideoFrame;
QT_END_NAMESPACE

namespace blueplayer::media {

/**
 * @brief Source média FFmpeg avec buffering optimisé pour streams HLS
 * 
 * Utilise un buffer de frames pour absorber les variations de latence réseau
 * et éviter les saccades lors de la lecture de streams Twitch.
 */
class FFmpegMediaSource final : public QObject {
  Q_OBJECT

public:
  explicit FFmpegMediaSource(QObject* parent = nullptr);
  ~FFmpegMediaSource() override;

  QVideoSink* videoSink() const;
  void setVideoSink(QVideoSink* sink);
  Q_INVOKABLE bool open(QString filePath);
  Q_INVOKABLE void play();
  Q_INVOKABLE void stop();
  
  // Configuration du buffer
  void setBufferSize(int frames) { m_bufferSize = frames; }
  int bufferSize() const { return m_bufferSize; }
  int bufferedFrames() const { return static_cast<int>(m_frameBuffer.size()); }

signals:
  void videoSinkChanged();
  void playingChanged(bool playing);
  void bufferingChanged(bool buffering);
  void bufferProgress(int percent);

private:
  void decodeLoop(QString path);
  void renderLoop();
  void deliverFrame(const QVideoFrame& frame);
  void enqueueFrame(QVideoFrame&& frame);
  bool dequeueFrame(QVideoFrame& frame);

  QVideoSink* m_videoSink = nullptr;
  std::atomic<bool> m_running{false};
  std::atomic<bool> m_stopRequested{false};
  std::atomic<bool> m_buffering{true};
  QString m_currentFile;
  std::unique_ptr<std::thread> m_decodeThread;
  std::unique_ptr<std::thread> m_renderThread;
  
  // Frame buffer pour absorber les variations de latence
  std::queue<QVideoFrame> m_frameBuffer;
  std::mutex m_bufferMutex;
  std::condition_variable m_bufferCondition;
  int m_bufferSize = 30;  // ~1 seconde à 30fps, ~0.5s à 60fps
  static constexpr int MIN_BUFFER_BEFORE_PLAY = 10;  // Frames minimum avant lecture
};

}  // namespace blueplayer::media

