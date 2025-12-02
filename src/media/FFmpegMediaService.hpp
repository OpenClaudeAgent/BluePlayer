#pragma once

#include <memory>
#include <QObject>
#include <QString>

#include <QVideoSink>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

namespace blueplayer::media {

class FFmpegMediaSource;

class FFmpegMediaService : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

public:
  explicit FFmpegMediaService(QObject* parent = nullptr);
  ~FFmpegMediaService() override;

  QVideoSink* videoSink() const;
  void setVideoSink(QVideoSink* sink);

  Q_INVOKABLE void play(const QUrl& source);
  Q_INVOKABLE void playFile(const QString& filePath);
  Q_INVOKABLE void stop();

signals:
  void videoSinkChanged();
  void playingChanged(bool playing);
  void errorOccurred(QString message);

private:
  std::unique_ptr<FFmpegMediaSource> m_source;
  QVideoSink* m_videoSink = nullptr;
};

}  // namespace blueplayer::media

