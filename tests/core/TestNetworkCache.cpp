#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QNetworkCacheMetaData>
#include <QDateTime>
#include <QBuffer>
#include <QUrl>
#include "core/NetworkCache.hpp"

using namespace blueplayer::core;

class TestNetworkCache : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();
  
  // Tests du constructeur
  void testConstructor();
  void testConstructorDefaultTTL();
  
  // Tests de configuration
  void testConfigure();
  void testConfigureSize();
  void testConfigureTTL();
  void testConfigureMultipleTimes();
  
  // Tests du TTL
  void testCacheTTLDefault();
  void testCacheTTLAfterConfigure();
  void testCacheTTLCustomValue();
  
  // Tests de la taille maximale
  void testMaximumCacheSizeDefault();
  void testMaximumCacheSizeAfterConfigure();
  void testMaximumCacheSizeLarge();
  void testMaximumCacheSizeSmall();
  
  // Tests des métadonnées
  void testMetadataCreation();
  void testMetadataWithExpiration();
  void testMetadataWithLastModified();
  
  // Tests du comportement du cache
  void testCacheDirectory();
  void testCacheClear();

private:
  NetworkCache* m_cache = nullptr;
  QTemporaryDir* m_tempDir = nullptr;
};

void TestNetworkCache::initTestCase() {
}

void TestNetworkCache::cleanupTestCase() {
}

void TestNetworkCache::init() {
  m_tempDir = new QTemporaryDir();
  m_cache = new NetworkCache();
  m_cache->setCacheDirectory(m_tempDir->path());
}

void TestNetworkCache::cleanup() {
  delete m_cache;
  m_cache = nullptr;
  delete m_tempDir;
  m_tempDir = nullptr;
}

// ===== Tests du constructeur =====

void TestNetworkCache::testConstructor() {
  NetworkCache cache;
  QVERIFY(true); // Cache created successfully
}

void TestNetworkCache::testConstructorDefaultTTL() {
  NetworkCache cache;
  QCOMPARE(cache.cacheTTL(), 300);  // 5 minutes par défaut
}

// ===== Tests de configuration =====

void TestNetworkCache::testConfigure() {
  m_cache->configure(100 * 1024 * 1024, 600);  // 100 MB, 10 minutes
  
  QCOMPARE(m_cache->cacheTTL(), 600);
  QCOMPARE(m_cache->maximumCacheSize(), qint64(100 * 1024 * 1024));
}

void TestNetworkCache::testConfigureSize() {
  qint64 size = 50 * 1024 * 1024;  // 50 MB
  m_cache->configure(size, 300);
  
  QCOMPARE(m_cache->maximumCacheSize(), size);
}

void TestNetworkCache::testConfigureTTL() {
  int ttl = 1800;  // 30 minutes
  m_cache->configure(50 * 1024 * 1024, ttl);
  
  QCOMPARE(m_cache->cacheTTL(), ttl);
}

void TestNetworkCache::testConfigureMultipleTimes() {
  m_cache->configure(10 * 1024 * 1024, 60);
  QCOMPARE(m_cache->cacheTTL(), 60);
  
  m_cache->configure(20 * 1024 * 1024, 120);
  QCOMPARE(m_cache->cacheTTL(), 120);
  
  m_cache->configure(30 * 1024 * 1024, 180);
  QCOMPARE(m_cache->cacheTTL(), 180);
  QCOMPARE(m_cache->maximumCacheSize(), qint64(30 * 1024 * 1024));
}

// ===== Tests du TTL =====

void TestNetworkCache::testCacheTTLDefault() {
  NetworkCache cache;
  QCOMPARE(cache.cacheTTL(), 300);
}

void TestNetworkCache::testCacheTTLAfterConfigure() {
  m_cache->configure(50 * 1024 * 1024, 900);
  QCOMPARE(m_cache->cacheTTL(), 900);
}

void TestNetworkCache::testCacheTTLCustomValue() {
  m_cache->configure(50 * 1024 * 1024, 3600);  // 1 heure
  QCOMPARE(m_cache->cacheTTL(), 3600);
}

// ===== Tests de la taille maximale =====

void TestNetworkCache::testMaximumCacheSizeDefault() {
  NetworkCache cache;
  // La taille par défaut de QNetworkDiskCache
  qint64 defaultSize = cache.maximumCacheSize();
  QVERIFY(defaultSize > 0);
}

void TestNetworkCache::testMaximumCacheSizeAfterConfigure() {
  qint64 expectedSize = 75 * 1024 * 1024;  // 75 MB
  m_cache->configure(expectedSize, 300);
  QCOMPARE(m_cache->maximumCacheSize(), expectedSize);
}

void TestNetworkCache::testMaximumCacheSizeLarge() {
  qint64 largeSize = 1024LL * 1024 * 1024;  // 1 GB
  m_cache->configure(largeSize, 300);
  QCOMPARE(m_cache->maximumCacheSize(), largeSize);
}

void TestNetworkCache::testMaximumCacheSizeSmall() {
  qint64 smallSize = 1024 * 1024;  // 1 MB
  m_cache->configure(smallSize, 300);
  QCOMPARE(m_cache->maximumCacheSize(), smallSize);
}

// ===== Tests des métadonnées =====

void TestNetworkCache::testMetadataCreation() {
  QNetworkCacheMetaData metaData;
  metaData.setUrl(QUrl("https://api.twitch.tv/helix/streams"));
  
  QVERIFY(metaData.url().isValid());
  QCOMPARE(metaData.url().toString(), QString("https://api.twitch.tv/helix/streams"));
}

void TestNetworkCache::testMetadataWithExpiration() {
  QNetworkCacheMetaData metaData;
  metaData.setUrl(QUrl("https://example.com/data"));
  
  QDateTime expiration = QDateTime::currentDateTimeUtc().addSecs(300);
  metaData.setExpirationDate(expiration);
  
  QVERIFY(metaData.expirationDate().isValid());
  QCOMPARE(metaData.expirationDate(), expiration);
}

void TestNetworkCache::testMetadataWithLastModified() {
  QNetworkCacheMetaData metaData;
  metaData.setUrl(QUrl("https://example.com/resource"));
  
  QDateTime lastModified = QDateTime::currentDateTimeUtc();
  metaData.setLastModified(lastModified);
  
  QVERIFY(metaData.lastModified().isValid());
  QCOMPARE(metaData.lastModified(), lastModified);
}

// ===== Tests du comportement du cache =====

void TestNetworkCache::testCacheDirectory() {
  QString cacheDir = m_cache->cacheDirectory();
  QVERIFY(!cacheDir.isEmpty());
  QVERIFY(QDir(cacheDir).exists());
}

void TestNetworkCache::testCacheClear() {
  // Configurer le cache
  m_cache->configure(10 * 1024 * 1024, 300);
  
  // Vider le cache ne doit pas crasher
  m_cache->clear();
  
  // La taille du cache doit être 0 après clear
  QCOMPARE(m_cache->cacheSize(), qint64(0));
}

QTEST_MAIN(TestNetworkCache)
#include "TestNetworkCache.moc"
