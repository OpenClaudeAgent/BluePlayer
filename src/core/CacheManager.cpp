#include "CacheManager.hpp"

#include <algorithm>

#include "Logger.hpp"

namespace blueplayer::core {

CacheManager::CacheManager(QObject* parent) : QObject(parent) {
  LOG_DEBUG(Core, "CacheManager created");
}

CacheManager::~CacheManager() {
  LOG_DEBUG(Core, "CacheManager destroyed");
}

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

  // Ne mettre à jour que si le nouveau temps est plus grand
  if (endTime > currentEnd) {
    m_cacheEndTime.store(endTime, std::memory_order_release);
    emitChanges();
  }
}

void CacheManager::updateCacheSize(qint64 sizeBytes) {
  qint64 oldSize = m_cacheSizeBytes.exchange(sizeBytes, std::memory_order_acq_rel);

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

  // Émettre seulement si les valeurs ont changé significativement
  constexpr double threshold = 0.1;  // 100ms de seuil

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

}  // namespace blueplayer::core



