#include "ui/CacheManagerViewModel.hpp"

#include <algorithm>

#include "core/Logger.hpp"

using blueplayer::core::LogCategory;
using blueplayer::core::Logger;

namespace blueplayer::ui {

CacheManagerViewModel::CacheManagerViewModel(QObject* parent) : QObject(parent) {}

void CacheManagerViewModel::initialize(blueplayer::core::CacheManager* cacheManager) {
  if (m_cacheManager != nullptr) {
    disconnect(m_cacheManager, nullptr, this, nullptr);
  }

  m_cacheManager = cacheManager;

  if (m_cacheManager != nullptr) {
    connect(m_cacheManager, &blueplayer::core::CacheManager::vodListChanged,
            this, &CacheManagerViewModel::onVodListChanged);
    connect(m_cacheManager, &blueplayer::core::CacheManager::vodCountChanged,
            this, &CacheManagerViewModel::onVodCountChanged);
    connect(m_cacheManager, &blueplayer::core::CacheManager::totalVodSizeChanged,
            this, &CacheManagerViewModel::onTotalSizeChanged);
    connect(m_cacheManager, &blueplayer::core::CacheManager::maxCacheSizeChanged,
            this, &CacheManagerViewModel::onMaxSizeChanged);

    // Émettre les signaux initiaux
    emit vodListChanged();
    emit vodCountChanged();
    emit totalSizeChanged();
    emit maxSizeChanged();
    emit usagePercentChanged();
  }
}

QVariantList CacheManagerViewModel::vodList() const {
  if (m_cacheManager == nullptr) {
    return QVariantList();
  }
  return applySortAndFilter(m_cacheManager->vodList());
}

int CacheManagerViewModel::vodCount() const {
  if (m_cacheManager == nullptr) {
    return 0;
  }
  return m_cacheManager->vodCount();
}

QString CacheManagerViewModel::totalSizeFormatted() const {
  if (m_cacheManager == nullptr) {
    return QStringLiteral("0 B");
  }
  return m_cacheManager->formattedTotalSize();
}

QString CacheManagerViewModel::maxSizeFormatted() const {
  if (m_cacheManager == nullptr) {
    return QStringLiteral("0 B");
  }
  return m_cacheManager->formattedMaxSize();
}

double CacheManagerViewModel::usagePercent() const {
  if (m_cacheManager == nullptr) {
    return 0.0;
  }
  return m_cacheManager->cacheUsagePercent();
}

qint64 CacheManagerViewModel::maxCacheSize() const {
  if (m_cacheManager == nullptr) {
    return 0;
  }
  return m_cacheManager->maxCacheSize();
}

void CacheManagerViewModel::setMaxCacheSize(qint64 size) {
  if (m_cacheManager != nullptr) {
    m_cacheManager->setMaxCacheSize(size);
  }
}

void CacheManagerViewModel::setSelectionMode(bool enabled) {
  if (m_selectionMode != enabled) {
    m_selectionMode = enabled;
    if (!enabled) {
      m_selectedIds.clear();
      emit selectedCountChanged();
    }
    emit selectionModeChanged();
  }
}

void CacheManagerViewModel::setSortField(const QString& field) {
  if (m_sortField != field) {
    m_sortField = field;
    emit sortFieldChanged();
    emit vodListChanged();
  }
}

void CacheManagerViewModel::setSortAscending(bool ascending) {
  if (m_sortAscending != ascending) {
    m_sortAscending = ascending;
    emit sortAscendingChanged();
    emit vodListChanged();
  }
}

void CacheManagerViewModel::setFilterStreamer(const QString& streamer) {
  if (m_filterStreamer != streamer) {
    m_filterStreamer = streamer;
    emit filterStreamerChanged();
    emit vodListChanged();
  }
}

void CacheManagerViewModel::toggleSelection(const QString& vodId, bool selected) {
  if (selected) {
    m_selectedIds.insert(vodId);
  } else {
    m_selectedIds.remove(vodId);
  }
  emit selectedCountChanged();
}

void CacheManagerViewModel::selectAll() {
  if (m_cacheManager == nullptr) {
    return;
  }

  QVariantList vods = m_cacheManager->vodList();
  for (const QVariant& vod : vods) {
    QString id = vod.toMap().value(QStringLiteral("id")).toString();
    m_selectedIds.insert(id);
  }
  emit selectedCountChanged();
}

void CacheManagerViewModel::deselectAll() {
  m_selectedIds.clear();
  emit selectedCountChanged();
}

bool CacheManagerViewModel::isSelected(const QString& vodId) const {
  return m_selectedIds.contains(vodId);
}

int CacheManagerViewModel::deleteSelected() {
  if (m_cacheManager == nullptr || m_selectedIds.isEmpty()) {
    return 0;
  }

  QStringList idsToDelete = m_selectedIds.values();
  int removed = m_cacheManager->removeVods(idsToDelete);

  m_selectedIds.clear();
  emit selectedCountChanged();

  Logger::info(LogCategory::UI,
               QString("Deleted %1 selected VODs").arg(removed));

  return removed;
}

bool CacheManagerViewModel::deleteVod(const QString& vodId) {
  if (m_cacheManager == nullptr) {
    return false;
  }

  bool removed = m_cacheManager->removeVod(vodId);

  if (removed) {
    m_selectedIds.remove(vodId);
    emit selectedCountChanged();
  }

  return removed;
}

int CacheManagerViewModel::clearAll() {
  if (m_cacheManager == nullptr) {
    return 0;
  }

  int removed = m_cacheManager->clearAllVods();

  m_selectedIds.clear();
  emit selectedCountChanged();

  Logger::info(LogCategory::UI, QString("Cleared all VODs: %1 removed").arg(removed));

  return removed;
}

QVariantMap CacheManagerViewModel::getVodDetails(const QString& vodId) const {
  if (m_cacheManager == nullptr) {
    return QVariantMap();
  }
  return m_cacheManager->getVodMetadata(vodId);
}

QStringList CacheManagerViewModel::getStreamerList() const {
  if (m_cacheManager == nullptr) {
    return QStringList();
  }

  QSet<QString> streamers;
  QVariantList vods = m_cacheManager->vodList();

  for (const QVariant& vod : vods) {
    QString streamer = vod.toMap().value(QStringLiteral("streamerName")).toString();
    if (!streamer.isEmpty()) {
      streamers.insert(streamer);
    }
  }

  QStringList result = streamers.values();
  result.sort();
  return result;
}

void CacheManagerViewModel::refresh() {
  emit vodListChanged();
  emit vodCountChanged();
  emit totalSizeChanged();
  emit usagePercentChanged();
}

void CacheManagerViewModel::onVodListChanged() {
  emit vodListChanged();
}

void CacheManagerViewModel::onVodCountChanged(int /*count*/) {
  emit vodCountChanged();
}

void CacheManagerViewModel::onTotalSizeChanged(qint64 /*size*/) {
  emit totalSizeChanged();
  emit usagePercentChanged();
}

void CacheManagerViewModel::onMaxSizeChanged(qint64 /*size*/) {
  emit maxSizeChanged();
  emit usagePercentChanged();
}

QVariantList CacheManagerViewModel::applySortAndFilter(const QVariantList& list) const {
  QVariantList result = list;

  // Filtrer par streamer si spécifié
  if (!m_filterStreamer.isEmpty()) {
    QVariantList filtered;
    for (const QVariant& item : result) {
      QString streamer = item.toMap().value(QStringLiteral("streamerName")).toString();
      if (streamer == m_filterStreamer) {
        filtered.append(item);
      }
    }
    result = filtered;
  }

  // Trier selon le champ et l'ordre
  std::sort(result.begin(), result.end(),
            [this](const QVariant& a, const QVariant& b) {
              QVariantMap mapA = a.toMap();
              QVariantMap mapB = b.toMap();

              QVariant valueA = mapA.value(m_sortField);
              QVariant valueB = mapB.value(m_sortField);

              bool lessThan = false;

              if (m_sortField == QStringLiteral("recordedAt") ||
                  m_sortField == QStringLiteral("lastPlayedAt")) {
                lessThan = valueA.toDateTime() < valueB.toDateTime();
              } else if (m_sortField == QStringLiteral("duration") ||
                         m_sortField == QStringLiteral("fileSize")) {
                lessThan = valueA.toLongLong() < valueB.toLongLong();
              } else {
                lessThan = valueA.toString() < valueB.toString();
              }

              return m_sortAscending ? lessThan : !lessThan;
            });

  return result;
}

}  // namespace blueplayer::ui
