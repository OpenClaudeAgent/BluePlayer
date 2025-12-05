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
class MpvMediaSource;

/**
 * @brief Service de lecture média utilisant libmpv (avec fallback FFmpeg)
 * 
 * Fournit une interface Qt pour lire des streams HLS et fichiers vidéo.
 * Utilise libmpv pour une lecture robuste avec buffering et sync A/V automatiques.
 * Supporte la pause/reprise, le contrôle du volume et le recording.
 */
class FFmpegMediaService : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
  Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged)
  Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
  Q_PROPERTY(double duration READ duration NOTIFY durationChanged)
  Q_PROPERTY(double position READ position NOTIFY positionChanged)
  Q_PROPERTY(double liveOffset READ liveOffset NOTIFY liveOffsetChanged)
  Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
  Q_PROPERTY(QString recordingPath READ recordingPath NOTIFY recordingPathChanged)

public:
  /**
   * @brief Constructeur
   * @param parent Le parent QObject
   */
  explicit FFmpegMediaService(QObject* parent = nullptr);
  ~FFmpegMediaService() override;

  /**
   * @brief Obtient le QVideoSink pour le rendu vidéo
   * @return Le QVideoSink actuel
   */
  QVideoSink* videoSink() const;

  /**
   * @brief Configure le QVideoSink pour le rendu vidéo
   * @param sink Le QVideoSink à utiliser
   */
  void setVideoSink(QVideoSink* sink);

  /**
   * @brief Lit un fichier média depuis une URL
   * @param source L'URL du fichier (doit être un fichier local)
   */
  Q_INVOKABLE void play(const QUrl& source);

  /**
   * @brief Lit un fichier média depuis un chemin de fichier
   * @param filePath Le chemin du fichier
   */
  Q_INVOKABLE void playFile(const QString& filePath);

  /**
   * @brief Arrête la lecture en cours
   */
  Q_INVOKABLE void stop();

  /**
   * @brief Met en pause la lecture
   */
  Q_INVOKABLE void pause();

  /**
   * @brief Reprend la lecture après une pause
   */
  Q_INVOKABLE void resume();

  /**
   * @brief Bascule entre pause et lecture
   */
  Q_INVOKABLE void togglePause();

  /**
   * @brief Vérifie si la lecture est en pause
   */
  bool isPaused() const;

  /**
   * @brief Obtient le niveau de volume (0.0 à 1.0)
   */
  float volume() const;

  /**
   * @brief Définit le niveau de volume (0.0 à 1.0)
   */
  void setVolume(float vol);

  /**
   * @brief Vérifie si le son est coupé
   */
  bool isMuted() const;

  /**
   * @brief Coupe ou active le son
   */
  void setMuted(bool muted);

  /**
   * @brief Bascule l'état muet
   */
  Q_INVOKABLE void toggleMute();

  /**
   * @brief Obtient la durée totale (pour VOD)
   */
  double duration() const;

  /**
   * @brief Obtient la position actuelle
   */
  double position() const;

  /**
   * @brief Seek à une position
   */
  Q_INVOKABLE void seek(double seconds);

  /**
   * @brief Démarre l'enregistrement du stream
   */
  Q_INVOKABLE void startRecording(const QString& outputPath);

  /**
   * @brief Arrête l'enregistrement
   */
  Q_INVOKABLE void stopRecording();
  bool isRecording() const;
  QString recordingPath() const;

  /**
   * @brief Offset par rapport au live (s)
   */
  double liveOffset() const { return m_liveOffset; }

signals:
  void videoSinkChanged();
  void playingChanged(bool playing);
  void pausedChanged(bool paused);
  void volumeChanged(float volume);
  void mutedChanged(bool muted);
  void durationChanged(double duration);
  void positionChanged(double position);
  void liveOffsetChanged(double offset);
  void recordingChanged(bool recording);
  void recordingPathChanged(const QString& path);
  void bufferingChanged(bool buffering);
  void errorOccurred(QString message);

private:
  void updateLiveOffset();
  QString buildRecordingPath(const QString& sourcePath) const;

  std::unique_ptr<MpvMediaSource> m_mpvSource;
  std::unique_ptr<FFmpegMediaSource> m_ffmpegSource;  // Fallback
  QVideoSink* m_videoSink = nullptr;
  bool m_useMpv = true;  // Utiliser libmpv par défaut
  double m_liveOffset = 0.0;
};

}  // namespace blueplayer::media

