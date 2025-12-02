#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace blueplayer::core {

/**
 * @brief Classe pour gérer l'historique de visionnage local
 * 
 * Stocke les positions de lecture des VODs pour permettre de reprendre la lecture
 * où l'utilisateur s'est arrêté. Utilise QSettings pour la persistance.
 */
class WatchHistory : public QObject {
  Q_OBJECT

public:
  explicit WatchHistory(QObject* parent = nullptr);
  ~WatchHistory() override;

  /**
   * @brief Sauvegarde la position de lecture d'un VOD
   * @param videoId L'ID du VOD Twitch
   * @param position La position en secondes
   */
  void saveWatchPosition(const QString& videoId, qint64 position);

  /**
   * @brief Récupère la position de lecture sauvegardée d'un VOD
   * @param videoId L'ID du VOD Twitch
   * @return La position en secondes, ou 0 si aucune position n'est sauvegardée
   */
  [[nodiscard]] qint64 getWatchPosition(const QString& videoId) const;

  /**
   * @brief Récupère la liste des VODs en cours de visionnage
   * @return Liste de maps contenant videoId, position, et lastWatched timestamp
   */
  [[nodiscard]] QVariantList getVideosInProgress() const;

  /**
   * @brief Efface l'historique de visionnage
   */
  void clearHistory();

  /**
   * @brief Supprime l'entrée d'un VOD spécifique
   * @param videoId L'ID du VOD à supprimer
   */
  void removeVideo(const QString& videoId);

private:
  QSettings* m_settings = nullptr;
  static constexpr const char* kWatchHistoryGroup = "WatchHistory";
  static constexpr const char* kPositionKey = "position";
  static constexpr const char* kLastWatchedKey = "lastWatched";
};

}  // namespace blueplayer::core

