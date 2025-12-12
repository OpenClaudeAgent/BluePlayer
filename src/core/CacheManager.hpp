#pragma once

#include <QMutex>
#include <QObject>
#include <atomic>
#include <chrono>

namespace blueplayer::core {

/**
 * @brief Gestionnaire du cache vidéo pour le mode replay
 *
 * Gère le cache vidéo en mémoire et expose la durée disponible
 * pour le seekback. Le cache est dynamique et s'étend avec le stream.
 *
 * Caractéristiques:
 * - Pas de plafond: le cache stocke toute la durée du stream
 * - Mise à jour en temps réel de la durée disponible
 * - Signaux pour synchroniser avec la state machine et l'UI
 */
class CacheManager : public QObject {
  Q_OBJECT

  Q_PROPERTY(double cacheDuration READ cacheDuration NOTIFY cacheDurationChanged)
  Q_PROPERTY(
      double cacheStartTime READ cacheStartTime NOTIFY cacheStartTimeChanged)
  Q_PROPERTY(double cacheEndTime READ cacheEndTime NOTIFY cacheEndTimeChanged)
  Q_PROPERTY(qint64 cacheSizeBytes READ cacheSizeBytes NOTIFY cacheSizeBytesChanged)
  Q_PROPERTY(bool isCaching READ isCaching NOTIFY isCachingChanged)

 public:
  /**
   * @brief Constructeur
   * @param parent QObject parent
   */
  explicit CacheManager(QObject* parent = nullptr);

  /**
   * @brief Destructeur
   */
  ~CacheManager() override;

  // ===== Getters =====

  /**
   * @brief Obtient la durée totale du cache en secondes
   * @return La durée du cache (cacheEndTime - cacheStartTime)
   */
  [[nodiscard]] double cacheDuration() const;

  /**
   * @brief Obtient le temps de début du cache
   * @return Le temps de début en secondes
   */
  [[nodiscard]] double cacheStartTime() const {
    return m_cacheStartTime.load(std::memory_order_acquire);
  }

  /**
   * @brief Obtient le temps de fin du cache (= live edge)
   * @return Le temps de fin en secondes
   */
  [[nodiscard]] double cacheEndTime() const {
    return m_cacheEndTime.load(std::memory_order_acquire);
  }

  /**
   * @brief Obtient la taille du cache en octets
   * @return La taille en octets
   */
  [[nodiscard]] qint64 cacheSizeBytes() const {
    return m_cacheSizeBytes.load(std::memory_order_acquire);
  }

  /**
   * @brief Vérifie si le cache est actif
   * @return true si le cache est en cours d'écriture
   */
  [[nodiscard]] bool isCaching() const {
    return m_isCaching.load(std::memory_order_acquire);
  }

  /**
   * @brief Vérifie si un temps donné est dans le cache
   * @param time Le temps à vérifier
   * @return true si le temps est dans la plage du cache
   */
  [[nodiscard]] bool isTimeInCache(double time) const;

  /**
   * @brief Obtient le temps le plus ancien disponible dans le cache
   * @return Le temps minimum accessible pour le replay
   */
  [[nodiscard]] double oldestAvailableTime() const {
    return m_cacheStartTime.load(std::memory_order_acquire);
  }

  /**
   * @brief Obtient le temps le plus récent disponible dans le cache
   * @return Le temps maximum accessible (live edge)
   */
  [[nodiscard]] double newestAvailableTime() const {
    return m_cacheEndTime.load(std::memory_order_acquire);
  }

 public slots:
  /**
   * @brief Démarre le cache pour un nouveau stream
   * @param startTime Le temps de début du stream
   */
  void startCaching(double startTime = 0.0);

  /**
   * @brief Arrête le cache
   */
  void stopCaching();

  /**
   * @brief Met à jour la fin du cache (appelé quand le live edge avance)
   * @param endTime Le nouveau temps de fin
   */
  void updateCacheEnd(double endTime);

  /**
   * @brief Met à jour la taille du cache en octets
   * @param sizeBytes La nouvelle taille
   */
  void updateCacheSize(qint64 sizeBytes);

  /**
   * @brief Réinitialise le cache
   */
  void reset();

  /**
   * @brief Clampe un temps dans la plage du cache
   * @param time Le temps à clamper
   * @return Le temps clampé entre cacheStartTime et cacheEndTime
   */
  [[nodiscard]] double clampToCache(double time) const;

 signals:
  /**
   * @brief Émis quand la durée du cache change
   * @param duration La nouvelle durée en secondes
   */
  void cacheDurationChanged(double duration);

  /**
   * @brief Émis quand le temps de début du cache change
   * @param startTime Le nouveau temps de début
   */
  void cacheStartTimeChanged(double startTime);

  /**
   * @brief Émis quand le temps de fin du cache change
   * @param endTime Le nouveau temps de fin
   */
  void cacheEndTimeChanged(double endTime);

  /**
   * @brief Émis quand la taille du cache change
   * @param sizeBytes La nouvelle taille en octets
   */
  void cacheSizeBytesChanged(qint64 sizeBytes);

  /**
   * @brief Émis quand l'état de cache change
   * @param isCaching true si le cache est actif
   */
  void isCachingChanged(bool isCaching);

  /**
   * @brief Émis pour notifier la state machine d'une mise à jour du cache
   * @param duration La nouvelle durée du cache
   */
  void cacheUpdated(double duration);

 private:
  /**
   * @brief Émet les signaux de changement si nécessaire
   */
  void emitChanges();

  // ===== État atomique pour performances =====
  std::atomic<double> m_cacheStartTime{0.0};
  std::atomic<double> m_cacheEndTime{0.0};
  std::atomic<qint64> m_cacheSizeBytes{0};
  std::atomic<bool> m_isCaching{false};

  // ===== Cache des dernières valeurs émises =====
  double m_lastEmittedDuration = -1.0;
  double m_lastEmittedStartTime = -1.0;
  double m_lastEmittedEndTime = -1.0;

  // ===== Mutex pour les opérations composées =====
  mutable QMutex m_mutex;
};

}  // namespace blueplayer::core



