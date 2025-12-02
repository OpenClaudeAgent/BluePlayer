#include "core/NetworkCache.hpp"

#include "core/Config.hpp"

#include <QDateTime>
#include <QStandardPaths>

namespace blueplayer::core {

NetworkCache::NetworkCache(QObject* parent) : QNetworkDiskCache(parent) {
  // Configurer le répertoire de cache
  const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/network";
  setCacheDirectory(cacheDir);
  
  // Configurer depuis Config
  const Config& config = Config::instance();
  configure(config.networkCacheSize(), config.networkCacheTTL());
}

void NetworkCache::configure(int cacheSize, int cacheTTL) {
  setMaximumCacheSize(cacheSize);
  m_cacheTTL = cacheTTL;
}

bool NetworkCache::isEntryValid(const QNetworkCacheMetaData& metaData) const {
  // Vérifier d'abord la validité de base
  if (metaData.isValid() == false) {
    return false;
  }

  // Vérifier le TTL personnalisé
  const QDateTime expirationDate = metaData.expirationDate();
  if (expirationDate.isValid()) {
    return QDateTime::currentDateTime() < expirationDate;
  }

  // Si pas de date d'expiration, utiliser le TTL par défaut
  const QDateTime lastModified = metaData.lastModified();
  if (lastModified.isValid()) {
    return lastModified.secsTo(QDateTime::currentDateTime()) < m_cacheTTL;
  }

  return true;
}

}  // namespace blueplayer::core

