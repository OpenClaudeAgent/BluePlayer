#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace blueplayer::core {

/**
 * @brief Structure représentant la progression de visionnage d'un VOD
 */
struct WatchProgress {
  QString vodId;           ///< ID unique du VOD
  qint64 lastPosition;     ///< Position en secondes
  qint64 totalDuration;    ///< Durée totale en secondes
  QDateTime lastWatchedAt; ///< Date de dernier visionnage
  bool completed;          ///< true si vu à > 90%

  /**
   * @brief Calcule le pourcentage de progression
   * @return Pourcentage entre 0 et 100
   */
  [[nodiscard]] double progressPercent() const {
    if (totalDuration <= 0) return 0.0;
    return (static_cast<double>(lastPosition) / static_cast<double>(totalDuration)) * 100.0;
  }

  /**
   * @brief Vérifie si le VOD est considéré comme complété (> 90%)
   * @return true si complété
   */
  [[nodiscard]] bool isCompleted() const {
    return progressPercent() >= 90.0;
  }

  /**
   * @brief Convertit en QVariantMap pour QML
   */
  [[nodiscard]] QVariantMap toVariantMap() const {
    QVariantMap map;
    map[QStringLiteral("vodId")] = vodId;
    map[QStringLiteral("lastPosition")] = lastPosition;
    map[QStringLiteral("totalDuration")] = totalDuration;
    map[QStringLiteral("lastWatchedAt")] = lastWatchedAt;
    map[QStringLiteral("completed")] = completed;
    map[QStringLiteral("progressPercent")] = progressPercent();
    return map;
  }
};

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
   * @param videoId L'ID du VOD
   * @param position La position en secondes
   */
  void saveWatchPosition(const QString& videoId, qint64 position);

  /**
   * @brief Sauvegarde la progression complète d'un VOD
   * @param videoId L'ID du VOD
   * @param position La position en secondes
   * @param totalDuration La durée totale en secondes
   */
  void saveWatchProgress(const QString& videoId, qint64 position, qint64 totalDuration);

  /**
   * @brief Récupère la position de lecture sauvegardée d'un VOD
   * @param videoId L'ID du VOD
   * @return La position en secondes, ou 0 si aucune position n'est sauvegardée
   */
  [[nodiscard]] qint64 getWatchPosition(const QString& videoId) const;

  /**
   * @brief Récupère la progression complète d'un VOD
   * @param videoId L'ID du VOD
   * @return Structure WatchProgress ou vide si non trouvé
   */
  [[nodiscard]] WatchProgress getWatchProgress(const QString& videoId) const;

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

  /**
   * @brief Marque un VOD comme complété
   * @param videoId L'ID du VOD
   */
  void markAsCompleted(const QString& videoId);

  /**
   * @brief Vérifie si un VOD a une progression sauvegardée
   * @param videoId L'ID du VOD
   * @return true si une progression existe
   */
  [[nodiscard]] bool hasProgress(const QString& videoId) const;

  /**
   * @brief Formate la date de dernier visionnage en texte relatif
   * @param lastWatchedAt La date à formater
   * @return Texte comme "Vu il y a 2 jours"
   */
  [[nodiscard]] static QString formatLastWatched(const QDateTime& lastWatchedAt);

signals:
  /**
   * @brief Émis quand la progression d'un VOD est mise à jour
   * @param vodId L'ID du VOD
   */
  void progressUpdated(const QString& vodId);

private:
  QSettings* m_settings = nullptr;
  static constexpr const char* kWatchHistoryGroup = "WatchHistory";
  static constexpr const char* kPositionKey = "position";
  static constexpr const char* kDurationKey = "duration";
  static constexpr const char* kLastWatchedKey = "lastWatched";
  static constexpr const char* kCompletedKey = "completed";
};

}  // namespace blueplayer::core

