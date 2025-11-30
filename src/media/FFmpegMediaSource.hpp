#pragma once

#include <atomic>
#include <memory>
#include <QObject>
#include <QString>
#include <QVideoSink>
#include <thread>

QT_BEGIN_NAMESPACE
class QVideoSink;
class QVideoFrame;
QT_END_NAMESPACE

namespace blueplayer::media {

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

signals:
  void videoSinkChanged();
  void playingChanged(bool playing);

private:
  void decodeLoop(QString path);
  void deliverFrame(const QVideoFrame& frame);

  QVideoSink* m_videoSink = nullptr;
  std::atomic<bool> m_running{false};
  std::atomic<bool> m_stopRequested{false};
  QString m_currentFile;
  std::unique_ptr<std::thread> m_decodeThread;
};

}  // namespace blueplayer::media

