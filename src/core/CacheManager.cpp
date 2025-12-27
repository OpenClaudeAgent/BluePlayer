#include "CacheManager.hpp"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <algorithm>

#include "Logger.hpp"

namespace blueplayer::core {

CacheManager::CacheManager(QObject* parent) : QObject(parent) {
  // Définir le répertoire de cache
  m_cacheDirectory =
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
      QStringLiteral("/BluePlayer/vods");
  ensureCacheDirectoryExists();

  // Charger les métadonnées existantes
  loadMetadata();

  LOG_DEBUG(Core, "CacheManager created, cache directory: " + m_cacheDirectory);
}

CacheManager::~CacheManager() {
  stopCleanupService();
  saveMetadata();
  LOG_DEBUG(Core, "CacheManager destroyed");
}

// ===== Implémentations existantes (cache live) =====

double CacheManager::cacheDuration() const {
  double start = m_cacheStartTime.load(std::memory_order_acquire);
  double end = m_cacheEndTime.load(std::memory_order_acquire);
  return std::max(0.0, end - start);
}

bool CacheManager::isTimeInCache(double time) const {
  double start = m_cacheStartTime.load(std::memory_order_acquire);
  double end = m_cacheEndTime.load(std::memory_order_acquire);
  return time >= start && time <= end;
}

void CacheManager::startCaching(double startTime) {
  QMutexLocker locker(&m_mutex);

  bool wasNotCaching = !m_isCaching.exchange(true, std::memory_order_acq_rel);

  m_cacheStartTime.store(startTime, std::memory_order_release);
  m_cacheEndTime.store(startTime, std::memory_order_release);
  m_cacheSizeBytes.store(0, std::memory_order_release);

  if (wasNotCaching) {
    LOG_INFO(Core,
             QString("CacheManager: Started caching at time %1").arg(startTime));
    emit isCachingChanged(true);
  }

  emitChanges();
}

void CacheManager::stopCaching() {
  bool wasCaching = m_isCaching.exchange(false, std::memory_order_acq_rel);

  if (wasCaching) {
    LOG_INFO(Core, QString("CacheManager: Stopped caching. Final duration: %1s")
                       .arg(cacheDuration()));
    emit isCachingChanged(false);
  }
}

void CacheManager::updateCacheEnd(double endTime) {
  double currentEnd = m_cacheEndTime.load(std::memory_order_acquire);

  if (endTime > currentEnd) {
    m_cacheEndTime.store(endTime, std::memory_order_release);
    emitChanges();
  }
}

void CacheManager::updateCacheSize(qint64 sizeBytes) {
  qint64 oldSize =
      m_cacheSizeBytes.exchange(sizeBytes, std::memory_order_acq_rel);

  if (oldSize != sizeBytes) {
    emit cacheSizeBytesChanged(sizeBytes);
  }
}

void CacheManager::reset() {
  QMutexLocker locker(&m_mutex);

  m_isCaching.store(false, std::memory_order_release);
  m_cacheStartTime.store(0.0, std::memory_order_release);
  m_cacheEndTime.store(0.0, std::memory_order_release);
  m_cacheSizeBytes.store(0, std::memory_order_release);

  m_lastEmittedDuration = -1.0;
  m_lastEmittedStartTime = -1.0;
  m_lastEmittedEndTime = -1.0;

  LOG_INFO(Core, "CacheManager reset");

  emit isCachingChanged(false);
  emit cacheDurationChanged(0.0);
  emit cacheStartTimeChanged(0.0);
  emit cacheEndTimeChanged(0.0);
  emit cacheSizeBytesChanged(0);
  emit cacheUpdated(0.0);
}

double CacheManager::clampToCache(double time) const {
  double start = m_cacheStartTime.load(std::memory_order_acquire);
  double end = m_cacheEndTime.load(std::memory_order_acquire);
  return std::clamp(time, start, end);
}

void CacheManager::emitChanges() {
  double currentDuration = cacheDuration();
  double currentStartTime = m_cacheStartTime.load(std::memory_order_acquire);
  double currentEndTime = m_cacheEndTime.load(std::memory_order_acquire);

  constexpr double threshold = 0.1;

  if (std::abs(currentDuration - m_lastEmittedDuration) > threshold) {
    m_lastEmittedDuration = currentDuration;
    emit cacheDurationChanged(currentDuration);
    emit cacheUpdated(currentDuration);
  }

  if (std::abs(currentStartTime - m_lastEmittedStartTime) > threshold) {
    m_lastEmittedStartTime = currentStartTime;
    emit cacheStartTimeChanged(currentStartTime);
  }

  if (std::abs(currentEndTime - m_lastEmittedEndTime) > threshold) {
    m_lastEmittedEndTime = currentEndTime;
    emit cacheEndTimeChanged(currentEndTime);
  }
}

// ===== Nouvelles implémentations (gestion VOD) =====

qint64 CacheManager::totalVodSize() const {
  QMutexLocker locker(&m_mutex);
  qint64 total = 0;
  for (const auto& vod : m_vodMetadataList) {
    total += vod.fileSize;
  }
  return total;
}

int CacheManager::vodCount() const {
  QMutexLocker locker(&m_mutex);
  return static_cast<int>(m_vodMetadataList.size());
}

QVariantList CacheManager::vodList() const {
  QMutexLocker locker(&m_mutex);
  QVariantList list;
  list.reserve(static_cast<int>(m_vodMetadataList.size()));
  for (const auto& vod : m_vodMetadataList) {
    list.append(vod.toVariantMap());
  }
  return list;
}

double CacheManager::cacheUsagePercent() const {
  if (m_maxCacheSize <= 0) {
    return 0.0;
  }
  return (static_cast<double>(totalVodSize()) /
          static_cast<double>(m_maxCacheSize)) *
         100.0;
}

QString CacheManager::formattedTotalSize() const {
  return VodMetadata::formatFileSize(totalVodSize());
}

QString CacheManager::formattedMaxSize() const {
  return VodMetadata::formatFileSize(m_maxCacheSize);
}

void CacheManager::setMaxCacheSize(qint64 maxSize) {
  if (maxSize != m_maxCacheSize && maxSize > 0) {
    m_maxCacheSize = maxSize;
    emit maxCacheSizeChanged(maxSize);
    checkCacheThreshold();
    saveMetadata();
  }
}

bool CacheManager::addVodFromQml(const QVariantMap& metadata) {
  VodMetadata vod;
  vod.id = VodMetadata::generateId();
  vod.streamerLogin = metadata.value(QStringLiteral("streamerLogin")).toString();
  vod.streamerName = metadata.value(QStringLiteral("streamerName")).toString();
  vod.streamTitle = metadata.value(QStringLiteral("streamTitle")).toString();
  vod.filePath = metadata.value(QStringLiteral("filePath")).toString();
  vod.duration = metadata.value(QStringLiteral("duration")).toLongLong();
  vod.gameCategory = metadata.value(QStringLiteral("gameCategory")).toString();
  vod.thumbnailPath = metadata.value(QStringLiteral("thumbnailPath")).toString();
  vod.recordedAt = QDateTime::currentDateTime();
  vod.lastPlayedAt = QDateTime();
  vod.watchPosition = 0;
  
  // Calculer la taille du fichier
  QFileInfo fileInfo(vod.filePath);
  if (fileInfo.exists()) {
    vod.fileSize = fileInfo.size();
  } else {
    vod.fileSize = 0;
  }
  
  return addVod(vod);
}

bool CacheManager::addVod(const VodMetadata& metadata) {
  if (!metadata.isValid()) {
    LOG_WARNING(Core, "Attempted to add invalid VOD metadata");
    return false;
  }

  {
    QMutexLocker locker(&m_mutex);

    // Vérifier si une VOD avec le même ID existe déjà
    auto it = std::find_if(m_vodMetadataList.begin(), m_vodMetadataList.end(),
                           [&metadata](const VodMetadata& existing) {
                             return existing.id == metadata.id;
                           });

    if (it != m_vodMetadataList.end()) {
      // Mettre à jour l'existante
      *it = metadata;
    } else {
      m_vodMetadataList.push_back(metadata);
    }
  }

  LOG_INFO(Core, QString("VOD added: %1 - %2")
                     .arg(metadata.streamerName)
                     .arg(metadata.streamTitle));

  emit vodAdded(metadata.id);
  emit vodCountChanged(vodCount());
  emit totalVodSizeChanged(totalVodSize());
  emit vodListChanged();

  saveMetadata();
  checkCacheThreshold();

  return true;
}

bool CacheManager::removeVod(const QString& vodId) {
  VodMetadata removedVod;

  {
    QMutexLocker locker(&m_mutex);

    auto it = std::find_if(m_vodMetadataList.begin(), m_vodMetadataList.end(),
                           [&vodId](const VodMetadata& vod) {
                             return vod.id == vodId;
                           });

    if (it == m_vodMetadataList.end()) {
      LOG_WARNING(Core, QString("VOD not found for removal: %1").arg(vodId));
      return false;
    }

    removedVod = *it;
    m_vodMetadataList.erase(it);
  }

  // Supprimer le fichier
  if (!deleteVodFile(removedVod.filePath)) {
    LOG_WARNING(Core, QString("Failed to delete VOD file: %1")
                          .arg(removedVod.filePath));
  }

  // Supprimer la miniature si elle existe
  if (!removedVod.thumbnailPath.isEmpty()) {
    deleteVodFile(removedVod.thumbnailPath);
  }

  LOG_INFO(Core, QString("VOD removed: %1").arg(vodId));

  emit vodRemoved(vodId);
  emit vodCountChanged(vodCount());
  emit totalVodSizeChanged(totalVodSize());
  emit vodListChanged();

  saveMetadata();

  return true;
}

int CacheManager::removeVods(const QStringList& vodIds) {
  int removed = 0;
  for (const QString& id : vodIds) {
    if (removeVod(id)) {
      ++removed;
    }
  }
  return removed;
}

int CacheManager::clearAllVods() {
  int count = 0;
  qint64 freedBytes = 0;

  {
    QMutexLocker locker(&m_mutex);
    count = static_cast<int>(m_vodMetadataList.size());

    for (const auto& vod : m_vodMetadataList) {
      freedBytes += vod.fileSize;
      deleteVodFile(vod.filePath);
      if (!vod.thumbnailPath.isEmpty()) {
        deleteVodFile(vod.thumbnailPath);
      }
    }

    m_vodMetadataList.clear();
  }

  LOG_INFO(Core, QString("All VODs cleared: %1 files, %2 freed")
                     .arg(count)
                     .arg(VodMetadata::formatFileSize(freedBytes)));

  emit vodsCleared();
  emit vodCountChanged(0);
  emit totalVodSizeChanged(0);
  emit vodListChanged();

  saveMetadata();

  return count;
}

void CacheManager::updateWatchPosition(const QString& vodId, qint64 position) {
  QMutexLocker locker(&m_mutex);

  auto it = std::find_if(m_vodMetadataList.begin(), m_vodMetadataList.end(),
                         [&vodId](const VodMetadata& vod) {
                           return vod.id == vodId;
                         });

  if (it != m_vodMetadataList.end()) {
    it->watchPosition = position;
    locker.unlock();
    saveMetadata();
    emit vodListChanged();
  }
}

void CacheManager::markAsPlayed(const QString& vodId) {
  QMutexLocker locker(&m_mutex);

  auto it = std::find_if(m_vodMetadataList.begin(), m_vodMetadataList.end(),
                         [&vodId](const VodMetadata& vod) {
                           return vod.id == vodId;
                         });

  if (it != m_vodMetadataList.end()) {
    it->lastPlayedAt = QDateTime::currentDateTime();
    locker.unlock();
    saveMetadata();
    emit vodListChanged();
  }
}

QVariantMap CacheManager::getVodMetadata(const QString& vodId) const {
  QMutexLocker locker(&m_mutex);

  auto it = std::find_if(m_vodMetadataList.begin(), m_vodMetadataList.end(),
                         [&vodId](const VodMetadata& vod) {
                           return vod.id == vodId;
                         });

  if (it != m_vodMetadataList.end()) {
    return it->toVariantMap();
  }

  return QVariantMap();
}

QVariantList CacheManager::searchVods(const QString& query) const {
  QMutexLocker locker(&m_mutex);
  QVariantList results;
  
  if (query.isEmpty()) {
    return results;
  }
  
  QString lowerQuery = query.toLower();
  
  for (const VodMetadata& vod : m_vodMetadataList) {
    // Rechercher dans le nom du streamer, le titre et la catégorie
    bool matchesStreamer = vod.streamerName.toLower().contains(lowerQuery);
    bool matchesTitle = vod.streamTitle.toLower().contains(lowerQuery);
    bool matchesGame = vod.gameCategory.toLower().contains(lowerQuery);
    
    if (matchesStreamer || matchesTitle || matchesGame) {
      results.append(vod.toVariantMap());
    }
  }
  
  LOG_DEBUG(Core, QString("searchVods('%1') found %2 results").arg(query).arg(results.size()));
  return results;
}

void CacheManager::startCleanupService(int intervalMs) {
  if (m_cleanupTimer == nullptr) {
    m_cleanupTimer = new QTimer(this);
    connect(m_cleanupTimer, &QTimer::timeout, this,
            &CacheManager::performCleanup);
  }

  m_cleanupTimer->start(intervalMs);
  LOG_INFO(Core, QString("Cache cleanup service started, interval: %1ms")
                     .arg(intervalMs));
}

void CacheManager::stopCleanupService() {
  if (m_cleanupTimer != nullptr) {
    m_cleanupTimer->stop();
    LOG_INFO(Core, "Cache cleanup service stopped");
  }
}

int CacheManager::performCleanup() {
  double usage = cacheUsagePercent();

  if (usage < kCleanupTriggerThreshold * 100.0) {
    return 0;
  }

  LOG_INFO(Core, QString("Cache cleanup triggered, usage: %1%").arg(usage, 0, 'f', 1));

  // Trier par LRU (moins récemment utilisé en premier)
  sortVodsByLru();

  int removed = 0;
  qint64 freedBytes = 0;
  double targetUsage = kCleanupTargetThreshold * 100.0;

  while (cacheUsagePercent() > targetUsage && !m_vodMetadataList.empty()) {
    VodMetadata oldestVod;

    {
      QMutexLocker locker(&m_mutex);
      if (m_vodMetadataList.empty()) {
        break;
      }
      oldestVod = m_vodMetadataList.front();
    }

    qint64 fileSize = oldestVod.fileSize;
    if (removeVod(oldestVod.id)) {
      ++removed;
      freedBytes += fileSize;
    }
  }

  if (removed > 0) {
    LOG_INFO(Core, QString("Cache cleanup completed: %1 VODs removed, %2 freed")
                       .arg(removed)
                       .arg(VodMetadata::formatFileSize(freedBytes)));
    emit cleanupPerformed(removed, freedBytes);
  }

  return removed;
}

void CacheManager::saveMetadata() {
  ensureCacheDirectoryExists();

  QJsonObject root;
  root[QStringLiteral("maxCacheSize")] = m_maxCacheSize;

  QJsonArray vodsArray;
  {
    QMutexLocker locker(&m_mutex);
    for (const auto& vod : m_vodMetadataList) {
      vodsArray.append(vod.toJson());
    }
  }
  root[QStringLiteral("vods")] = vodsArray;

  QFile file(metadataFilePath());
  if (file.open(QIODevice::WriteOnly)) {
    file.write(QJsonDocument(root).toJson());
    file.close();
    LOG_DEBUG(Core, "VOD metadata saved");
  } else {
    LOG_ERROR(Core, QString("Failed to save VOD metadata: %1")
                        .arg(file.errorString()));
  }
}

void CacheManager::loadMetadata() {
  QFile file(metadataFilePath());
  if (!file.exists()) {
    LOG_DEBUG(Core, "No existing VOD metadata file");
    return;
  }

  if (!file.open(QIODevice::ReadOnly)) {
    LOG_ERROR(Core, QString("Failed to open VOD metadata: %1")
                        .arg(file.errorString()));
    return;
  }

  QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
  file.close();

  if (!doc.isObject()) {
    LOG_ERROR(Core, "Invalid VOD metadata format");
    return;
  }

  QJsonObject root = doc.object();

  if (root.contains(QStringLiteral("maxCacheSize"))) {
    m_maxCacheSize = root[QStringLiteral("maxCacheSize")].toInteger();
  }

  QJsonArray vodsArray = root[QStringLiteral("vods")].toArray();

  {
    QMutexLocker locker(&m_mutex);
    m_vodMetadataList.clear();
    m_vodMetadataList.reserve(static_cast<size_t>(vodsArray.size()));

    for (const auto& vodValue : vodsArray) {
      VodMetadata vod = VodMetadata::fromJson(vodValue.toObject());

      // Vérifier que le fichier existe toujours
      if (QFile::exists(vod.filePath)) {
        m_vodMetadataList.push_back(vod);
      } else {
        LOG_WARNING(Core, QString("VOD file missing, removing from index: %1")
                              .arg(vod.filePath));
      }
    }
  }

  LOG_INFO(Core, QString("Loaded %1 VOD metadata entries")
                     .arg(m_vodMetadataList.size()));

  emit vodCountChanged(vodCount());
  emit totalVodSizeChanged(totalVodSize());
  emit vodListChanged();
}

QString CacheManager::metadataFilePath() const {
  return m_cacheDirectory + QStringLiteral("/vod_metadata.json");
}

void CacheManager::ensureCacheDirectoryExists() {
  QDir dir(m_cacheDirectory);
  if (!dir.exists()) {
    if (!dir.mkpath(QStringLiteral("."))) {
      LOG_ERROR(Core, QString("Failed to create cache directory: %1")
                          .arg(m_cacheDirectory));
    }
  }
}

bool CacheManager::deleteVodFile(const QString& filePath) {
  if (filePath.isEmpty()) {
    return true;
  }

  QFile file(filePath);
  if (!file.exists()) {
    return true;
  }

  if (!file.remove()) {
    LOG_ERROR(Core,
              QString("Failed to delete file: %1").arg(file.errorString()));
    return false;
  }

  return true;
}

void CacheManager::sortVodsByLru() {
  QMutexLocker locker(&m_mutex);

  std::sort(m_vodMetadataList.begin(), m_vodMetadataList.end(),
            [](const VodMetadata& a, const VodMetadata& b) {
              // Si lastPlayedAt n'est pas défini, utiliser recordedAt
              QDateTime aTime =
                  a.lastPlayedAt.isValid() ? a.lastPlayedAt : a.recordedAt;
              QDateTime bTime =
                  b.lastPlayedAt.isValid() ? b.lastPlayedAt : b.recordedAt;
              return aTime < bTime;  // Plus ancien en premier
            });
}

void CacheManager::checkCacheThreshold() {
  double usage = cacheUsagePercent();
  if (usage >= kCleanupTriggerThreshold * 100.0) {
    emit cacheThresholdReached(usage);
    // Déclencher le nettoyage automatique si le service est actif
    if (m_cleanupTimer != nullptr && m_cleanupTimer->isActive()) {
      performCleanup();
    }
  }
}

QString CacheManager::downloadThumbnail(const QString& url, const QString& filename) {
  if (url.isEmpty() || filename.isEmpty()) {
    LOG_WARNING(Core, "downloadThumbnail: empty url or filename");
    return QString();
  }

  ensureCacheDirectoryExists();
  
  QString localPath = m_cacheDirectory + QStringLiteral("/") + filename;
  
  // Vérifier si le fichier existe déjà
  if (QFile::exists(localPath)) {
    LOG_DEBUG(Core, QString("Thumbnail already exists: %1").arg(localPath));
    return localPath;
  }

  // Remplacer les placeholders Twitch {width} et {height} par des dimensions raisonnables
  QString processedUrl = url;
  processedUrl.replace(QStringLiteral("{width}"), QStringLiteral("440"));
  processedUrl.replace(QStringLiteral("{height}"), QStringLiteral("248"));
  // Certaines URLs utilisent %{width} et %{height}
  processedUrl.replace(QStringLiteral("%{width}"), QStringLiteral("440"));
  processedUrl.replace(QStringLiteral("%{height}"), QStringLiteral("248"));

  LOG_INFO(Core, QString("Downloading thumbnail from: %1").arg(processedUrl));

  QNetworkAccessManager manager;
  QUrl requestUrl{processedUrl};
  QNetworkRequest request{requestUrl};
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       QNetworkRequest::NoLessSafeRedirectPolicy);

  QNetworkReply* reply = manager.get(request);

  // Attendre la fin du téléchargement de manière synchrone
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  
  // Timeout de 10 secondes
  QTimer timeoutTimer;
  timeoutTimer.setSingleShot(true);
  QObject::connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
  timeoutTimer.start(10000);
  
  loop.exec();

  if (!timeoutTimer.isActive()) {
    // Timeout occurred
    LOG_WARNING(Core, "Thumbnail download timeout");
    reply->abort();
    reply->deleteLater();
    return QString();
  }
  
  timeoutTimer.stop();

  if (reply->error() != QNetworkReply::NoError) {
    LOG_WARNING(Core, QString("Thumbnail download failed: %1").arg(reply->errorString()));
    reply->deleteLater();
    return QString();
  }

  QByteArray data = reply->readAll();
  reply->deleteLater();

  if (data.isEmpty()) {
    LOG_WARNING(Core, "Thumbnail download returned empty data");
    return QString();
  }

  // Sauvegarder le fichier
  QFile file(localPath);
  if (!file.open(QIODevice::WriteOnly)) {
    LOG_ERROR(Core, QString("Failed to create thumbnail file: %1").arg(file.errorString()));
    return QString();
  }

  file.write(data);
  file.close();

  LOG_INFO(Core, QString("Thumbnail saved to: %1").arg(localPath));
  return localPath;
}

}  // namespace blueplayer::core
