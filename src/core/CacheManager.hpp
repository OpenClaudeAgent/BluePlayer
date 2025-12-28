#pragma once

#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <atomic>
#include <vector>

#include "VodMetadata.hpp"

namespace blueplayer::core {

/**
 * @brief Gestionnaire du cache vidéo pour le mode replay et les VOD
 *
 * Gère le cache vidéo en mémoire, les métadonnées des VOD enregistrées,
 * et le nettoyage automatique selon la stratégie LRU.
 *
 * Caractéristiques:
 * - Cache dynamique pour le mode replay en direct
 * - Stockage persistant des métadonnées VOD
 * - Service de nettoyage automatique en arrière-plan
 * - Signaux pour synchroniser avec l'UI
 */
class CacheManager : public QObject {
  Q_OBJECT

  // Propriétés pour le cache live (existant)
  Q_PROPERTY(double cacheDuration READ cacheDuration NOTIFY cacheDurationChanged)
  Q_PROPERTY(double cacheStartTime READ cacheStartTime NOTIFY cacheStartTimeChanged)
  Q_PROPERTY(double cacheEndTime READ cacheEndTime NOTIFY cacheEndTimeChanged)
  Q_PROPERTY(qint64 cacheSizeBytes READ cacheSizeBytes NOTIFY cacheSizeBytesChanged)
  Q_PROPERTY(bool isCaching READ isCaching NOTIFY isCachingChanged)

  // Nouvelles propriétés pour la gestion des VOD
  Q_PROPERTY(qint64 totalVodSize READ totalVodSize NOTIFY totalVodSizeChanged)
  Q_PROPERTY(qint64 maxCacheSize READ maxCacheSize WRITE setMaxCacheSize NOTIFY maxCacheSizeChanged)
  Q_PROPERTY(int vodCount READ vodCount NOTIFY vodCountChanged)
  Q_PROPERTY(QVariantList vodList READ vodList NOTIFY vodListChanged)
  Q_PROPERTY(QString cacheDirectory READ cacheDirectory CONSTANT)

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

  // ===== Getters existants (cache live) =====

  [[nodiscard]] double cacheDuration() const;
  [[nodiscard]] double cacheStartTime() const {
    return m_cacheStartTime.load(std::memory_order_acquire);
  }
  [[nodiscard]] double cacheEndTime() const {
    return m_cacheEndTime.load(std::memory_order_acquire);
  }
  [[nodiscard]] qint64 cacheSizeBytes() const {
    return m_cacheSizeBytes.load(std::memory_order_acquire);
  }
  [[nodiscard]] bool isCaching() const {
    return m_isCaching.load(std::memory_order_acquire);
  }
  [[nodiscard]] bool isTimeInCache(double time) const;
  [[nodiscard]] double oldestAvailableTime() const {
    return m_cacheStartTime.load(std::memory_order_acquire);
  }
  [[nodiscard]] double newestAvailableTime() const {
    return m_cacheEndTime.load(std::memory_order_acquire);
  }

  // ===== Nouveaux getters (gestion VOD) =====

  /**
   * @brief Obtient la taille totale des VOD en cache
   * @return La taille totale en octets
   */
  [[nodiscard]] qint64 totalVodSize() const;

  /**
   * @brief Obtient la taille maximale du cache configurée
   * @return La taille max en octets
   */
  [[nodiscard]] qint64 maxCacheSize() const { return m_maxCacheSize; }

  /**
   * @brief Obtient le nombre de VOD en cache
   * @return Le nombre de VOD
   */
  [[nodiscard]] int vodCount() const;

  /**
   * @brief Obtient la liste des VOD pour l'UI
   * @return QVariantList des métadonnées VOD
   */
  [[nodiscard]] QVariantList vodList() const;

  /**
   * @brief Obtient le chemin du répertoire de cache
   * @return Le chemin absolu
   */
  [[nodiscard]] QString cacheDirectory() const { return m_cacheDirectory; }

  /**
   * @brief Obtient le pourcentage d'utilisation du cache
   * @return Pourcentage entre 0 et 100
   */
  [[nodiscard]] Q_INVOKABLE double cacheUsagePercent() const;

  /**
   * @brief Obtient la taille formatée du cache utilisé
   * @return Chaîne formatée (ex: "2.5 GB")
   */
  [[nodiscard]] Q_INVOKABLE QString formattedTotalSize() const;

  /**
   * @brief Obtient la taille max formatée
   * @return Chaîne formatée (ex: "10 GB")
   */
  [[nodiscard]] Q_INVOKABLE QString formattedMaxSize() const;

 public slots:
  // ===== Slots existants (cache live) =====
  void startCaching(double startTime = 0.0);
  void stopCaching();
  void updateCacheEnd(double endTime);
  void updateCacheSize(qint64 sizeBytes);
  void reset();
  [[nodiscard]] double clampToCache(double time) const;

  // ===== Nouveaux slots (gestion VOD) =====

  /**
   * @brief Définit la taille maximale du cache
   * @param maxSize Taille max en octets
   */
  void setMaxCacheSize(qint64 maxSize);

  /**
   * @brief Ajoute une VOD au cache
   * @param metadata Les métadonnées de la VOD
   * @return true si ajoutée avec succès
   */
  bool addVod(const VodMetadata& metadata);

  /**
   * @brief Ajoute une VOD au cache depuis QML
   * @param metadata Les métadonnées sous forme de QVariantMap
   * @return true si ajoutée avec succès
   */
  Q_INVOKABLE bool addVodFromQml(const QVariantMap& metadata);

  /**
   * @brief Télécharge un thumbnail depuis une URL et le sauvegarde (SYNCHRONE - DEPRECATED)
   * @param url L'URL du thumbnail
   * @param filename Le nom du fichier local
   * @return Le chemin local du fichier ou vide si échec
   * @deprecated Utiliser downloadThumbnailAsync() pour ne pas bloquer le thread UI
   */
  Q_INVOKABLE QString downloadThumbnail(const QString& url, const QString& filename);

  /**
   * @brief Télécharge un thumbnail de façon asynchrone
   * @param url L'URL du thumbnail
   * @param filename Le nom du fichier local
   * 
   * Le signal thumbnailDownloaded() sera émis quand le téléchargement est terminé.
   * Le signal thumbnailDownloadFailed() sera émis en cas d'échec.
   */
  Q_INVOKABLE void downloadThumbnailAsync(const QString& url, const QString& filename);

  /**
   * @brief Supprime une VOD du cache par ID
   * @param vodId L'ID de la VOD
   * @return true si supprimée avec succès
   */
  Q_INVOKABLE bool removeVod(const QString& vodId);

  /**
   * @brief Supprime plusieurs VOD du cache
   * @param vodIds Liste des IDs à supprimer
   * @return Nombre de VOD supprimées
   */
  Q_INVOKABLE int removeVods(const QStringList& vodIds);

  /**
   * @brief Vide entièrement le cache
   * @return Nombre de VOD supprimées
   */
  Q_INVOKABLE int clearAllVods();

  /**
   * @brief Met à jour la position de lecture d'une VOD
   * @param vodId L'ID de la VOD
   * @param position Position en secondes
   */
  Q_INVOKABLE void updateWatchPosition(const QString& vodId, qint64 position);

  /**
   * @brief Marque une VOD comme lue (met à jour lastPlayedAt)
   * @param vodId L'ID de la VOD
   */
  Q_INVOKABLE void markAsPlayed(const QString& vodId);

  /**
   * @brief Obtient les métadonnées d'une VOD
   * @param vodId L'ID de la VOD
   * @return Les métadonnées ou une structure vide si non trouvée
   */
  Q_INVOKABLE QVariantMap getVodMetadata(const QString& vodId) const;

  /**
   * @brief Recherche des VOD par nom de streamer ou titre
   * @param query Le terme de recherche
   * @return Liste des VOD correspondantes
   */
  Q_INVOKABLE QVariantList searchVods(const QString& query) const;

  /**
   * @brief Démarre le service de nettoyage automatique
   * @param intervalMs Intervalle de vérification en millisecondes (défaut: 5 min)
   */
  void startCleanupService(int intervalMs = 300000);

  /**
   * @brief Arrête le service de nettoyage automatique
   */
  void stopCleanupService();

  /**
   * @brief Force un nettoyage immédiat du cache
   * @return Nombre de VOD supprimées
   */
  Q_INVOKABLE int performCleanup();

  /**
   * @brief Sauvegarde les métadonnées sur le disque
   */
  void saveMetadata();

  /**
   * @brief Charge les métadonnées depuis le disque
   */
  void loadMetadata();

  // ===== Recording helpers (pour simplifier QML) =====

  /**
   * @brief Prépare un enregistrement et télécharge le thumbnail
   * @param streamerLogin Login du streamer
   * @param thumbnailUrl URL du thumbnail (optionnel)
   * @return Structure avec recordingPath et thumbnailPath
   */
  Q_INVOKABLE QVariantMap prepareRecording(const QString& streamerLogin, 
                                            const QString& thumbnailUrl = QString());

  /**
   * @brief Finalise un enregistrement et l'ajoute aux métadonnées VOD
   * @param recordingPath Chemin du fichier enregistré
   * @param streamerLogin Login du streamer
   * @param streamerName Nom du streamer
   * @param streamTitle Titre du stream
   * @param thumbnailPath Chemin du thumbnail
   * @param recordingStartMs Timestamp de début (ms depuis epoch)
   * @param quality Qualité de l'enregistrement (ex: "1080p60")
   * @return true si finalisé avec succès (durée >= 30s)
   */
  Q_INVOKABLE bool finalizeRecording(const QString& recordingPath,
                                      const QString& streamerLogin,
                                      const QString& streamerName,
                                      const QString& streamTitle,
                                      const QString& thumbnailPath,
                                      qint64 recordingStartMs,
                                      const QString& quality = QString());

 signals:
  // ===== Signaux existants =====
  void cacheDurationChanged(double duration);
  void cacheStartTimeChanged(double startTime);
  void cacheEndTimeChanged(double endTime);
  void cacheSizeBytesChanged(qint64 sizeBytes);
  void isCachingChanged(bool isCaching);
  void cacheUpdated(double duration);

  // ===== Nouveaux signaux =====
  void totalVodSizeChanged(qint64 totalSize);
  void maxCacheSizeChanged(qint64 maxSize);
  void vodCountChanged(int count);
  void vodListChanged();
  void vodAdded(const QString& vodId);
  void vodRemoved(const QString& vodId);
  void vodsCleared();
  void cleanupPerformed(int removedCount, qint64 freedBytes);
  void cacheThresholdReached(double usagePercent);
  
  // Signaux pour téléchargement asynchrone des thumbnails
  void thumbnailDownloaded(const QString& localPath, const QString& filename);
  void thumbnailDownloadFailed(const QString& filename, const QString& error);

 private:
  void emitChanges();
  void checkCacheThreshold();
  QString metadataFilePath() const;
  void ensureCacheDirectoryExists();
  bool deleteVodFile(const QString& filePath);
  void sortVodsByLru();
  QString processThumbnailUrl(const QString& url) const;
  void handleThumbnailReply(QNetworkReply* reply, const QString& filename, const QString& localPath);

  // ===== État atomique pour cache live =====
  std::atomic<double> m_cacheStartTime{0.0};
  std::atomic<double> m_cacheEndTime{0.0};
  std::atomic<qint64> m_cacheSizeBytes{0};
  std::atomic<bool> m_isCaching{false};

  double m_lastEmittedDuration = -1.0;
  double m_lastEmittedStartTime = -1.0;
  double m_lastEmittedEndTime = -1.0;

  // ===== État pour gestion VOD =====
  std::vector<VodMetadata> m_vodMetadataList;
  qint64 m_maxCacheSize = 10LL * 1024 * 1024 * 1024;  // 10 GB par défaut
  QString m_cacheDirectory;
  QTimer* m_cleanupTimer = nullptr;
  
  // Seuils pour le nettoyage automatique
  static constexpr double kCleanupTriggerThreshold = 0.90;  // 90%
  static constexpr double kCleanupTargetThreshold = 0.80;   // 80%

  mutable QMutex m_mutex;
  
  // Network manager pour les téléchargements asynchrones
  QNetworkAccessManager* m_networkManager = nullptr;
};

}  // namespace blueplayer::core
