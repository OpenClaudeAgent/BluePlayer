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

/**
 * @brief Service de lecture média utilisant FFmpeg
 * 
 * Fournit une interface Qt pour lire des fichiers vidéo avec FFmpeg.
 * Gère l'ouverture, la lecture et l'arrêt des fichiers média.
 * Supporte la pause/reprise et le contrôle du volume audio.
 */
class FFmpegMediaService : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
  Q_PROPERTY(bool paused READ isPaused NOTIFY pausedChanged)
  Q_PROPERTY(float volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)

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

signals:
  void videoSinkChanged();
  void playingChanged(bool playing);
  void pausedChanged(bool paused);
  void volumeChanged(float volume);
  void mutedChanged(bool muted);
  void errorOccurred(QString message);

private:
  std::unique_ptr<FFmpegMediaSource> m_source;
  QVideoSink* m_videoSink = nullptr;
};

}  // namespace blueplayer::media

