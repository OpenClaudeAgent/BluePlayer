#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QNetworkCacheMetaData>
#include <QDateTime>
#include <QBuffer>
#include <QUrl>
#include "core/NetworkCache.hpp"

using namespace blueplayer::core;

/**
 * @brief Sous-classe testable de NetworkCache
 * 
 * Expose la méthode protégée isEntryValid() pour les tests unitaires.
 */
class TestableNetworkCache : public NetworkCache {
public:
  using NetworkCache::NetworkCache;
  
  // Expose la méthode protégée pour les tests
  [[nodiscard]] bool testIsEntryValid(const QNetworkCacheMetaData& metaData) const {
    return isEntryValid(metaData);
  }
};

class TestNetworkCache : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();
  
  // Tests du constructeur
  void testConstructorDefaults();
  
  // Tests de configuration
  void testConfigure();
  void testConfigureSize();
  void testConfigureTTL();
  void testConfigureMultipleTimes();
  
  // Tests du TTL
  void testCacheTTLAfterConfigure();
  void testCacheTTLCustomValue();
  
  // Tests de la taille maximale (data-driven)
  void testMaximumCacheSizeDefault();
  void testMaximumCacheSizeConfigure_data();
  void testMaximumCacheSizeConfigure();
  
  // Tests des métadonnées
  void testMetadataCreation();
  void testMetadataWithExpiration();
  void testMetadataWithLastModified();
  
  // Tests du comportement du cache
  void testCacheDirectory();
  void testCacheClear();
  
  // Tests de isEntryValid()
  void testIsEntryValidWithInvalidMetadata();
  void testIsEntryValidWithValidExpiration();
  void testIsEntryValidWithExpiredExpiration();
  void testIsEntryValidWithValidLastModified();
  void testIsEntryValidWithExpiredLastModified();
  void testIsEntryValidWithNoDatesFallback();
  void testIsEntryValidWithBothDates();

private:
  TestableNetworkCache* m_cache = nullptr;
  QTemporaryDir* m_tempDir = nullptr;
};

void TestNetworkCache::initTestCase() {
}

void TestNetworkCache::cleanupTestCase() {
}

void TestNetworkCache::init() {
  m_tempDir = new QTemporaryDir();
  m_cache = new TestableNetworkCache();
  m_cache->setCacheDirectory(m_tempDir->path());
}

void TestNetworkCache::cleanup() {
  delete m_cache;
  m_cache = nullptr;
  delete m_tempDir;
  m_tempDir = nullptr;
}

// ===== Tests du constructeur =====

void TestNetworkCache::testConstructorDefaults() {
  NetworkCache cache;
  // Verify default TTL is 5 minutes
  QCOMPARE(cache.cacheTTL(), 300);
  // Verify default size is positive
  QVERIFY(cache.maximumCacheSize() > 0);
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

void TestNetworkCache::testMaximumCacheSizeConfigure_data() {
  QTest::addColumn<qint64>("size");
  QTest::addColumn<QString>("description");

  QTest::newRow("small (1 MB)") << qint64(1024 * 1024) << "1 MB";
  QTest::newRow("medium (75 MB)") << qint64(75 * 1024 * 1024) << "75 MB";
  QTest::newRow("large (1 GB)") << qint64(1024LL * 1024 * 1024) << "1 GB";
}

void TestNetworkCache::testMaximumCacheSizeConfigure() {
  QFETCH(qint64, size);
  
  m_cache->configure(size, 300);
  QCOMPARE(m_cache->maximumCacheSize(), size);
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

// ===== Tests de isEntryValid() =====

void TestNetworkCache::testIsEntryValidWithInvalidMetadata() {
  // Arrange - Créer des métadonnées invalides (pas d'URL)
  QNetworkCacheMetaData metaData;
  
  // Assert - Les métadonnées sans URL sont invalides
  QVERIFY(!metaData.isValid());
  QVERIFY(!m_cache->testIsEntryValid(metaData));
}

void TestNetworkCache::testIsEntryValidWithValidExpiration() {
  // Arrange - Créer des métadonnées avec expiration dans le futur
  QNetworkCacheMetaData metaData;
  metaData.setUrl(QUrl("https://example.com/test"));
  metaData.setExpirationDate(QDateTime::currentDateTime().addSecs(3600)); // +1h
  
  // Assert - L'entrée doit être valide
  QVERIFY(metaData.isValid());
  QVERIFY(m_cache->testIsEntryValid(metaData));
}

void TestNetworkCache::testIsEntryValidWithExpiredExpiration() {
  // Arrange - Créer des métadonnées avec expiration dans le passé
  QNetworkCacheMetaData metaData;
  metaData.setUrl(QUrl("https://example.com/test"));
  metaData.setExpirationDate(QDateTime::currentDateTime().addSecs(-3600)); // -1h
  
  // Assert - L'entrée doit être invalide (expirée)
  QVERIFY(metaData.isValid());
  QVERIFY(!m_cache->testIsEntryValid(metaData));
}

void TestNetworkCache::testIsEntryValidWithValidLastModified() {
  // Arrange - Configurer un TTL de 300 secondes (5 min)
  m_cache->configure(10 * 1024 * 1024, 300);
  
  // Créer des métadonnées avec lastModified récent (< TTL)
  QNetworkCacheMetaData metaData;
  metaData.setUrl(QUrl("https://example.com/test"));
  // lastModified il y a 60 secondes (< 300s TTL)
  metaData.setLastModified(QDateTime::currentDateTime().addSecs(-60));
  
  // Assert - L'entrée doit être valide (TTL non dépassé)
  QVERIFY(metaData.isValid());
  QVERIFY(m_cache->testIsEntryValid(metaData));
}

void TestNetworkCache::testIsEntryValidWithExpiredLastModified() {
  // Arrange - Configurer un TTL de 300 secondes (5 min)
  m_cache->configure(10 * 1024 * 1024, 300);
  
  // Créer des métadonnées avec lastModified ancien (> TTL)
  QNetworkCacheMetaData metaData;
  metaData.setUrl(QUrl("https://example.com/test"));
  // lastModified il y a 600 secondes (> 300s TTL)
  metaData.setLastModified(QDateTime::currentDateTime().addSecs(-600));
  
  // Assert - L'entrée doit être invalide (TTL dépassé)
  QVERIFY(metaData.isValid());
  QVERIFY(!m_cache->testIsEntryValid(metaData));
}

void TestNetworkCache::testIsEntryValidWithNoDatesFallback() {
  // Arrange - Créer des métadonnées valides sans dates
  QNetworkCacheMetaData metaData;
  metaData.setUrl(QUrl("https://example.com/test"));
  // Pas de date d'expiration ni de lastModified
  
  // Assert - L'entrée doit être valide (fallback)
  QVERIFY(metaData.isValid());
  QVERIFY(m_cache->testIsEntryValid(metaData));
}

void TestNetworkCache::testIsEntryValidWithBothDates() {
  // Arrange - Configurer un TTL court
  m_cache->configure(10 * 1024 * 1024, 60);
  
  // Créer des métadonnées avec les deux dates
  // expirationDate valide, lastModified expiré
  QNetworkCacheMetaData metaData;
  metaData.setUrl(QUrl("https://example.com/test"));
  metaData.setExpirationDate(QDateTime::currentDateTime().addSecs(3600)); // +1h
  metaData.setLastModified(QDateTime::currentDateTime().addSecs(-120)); // -2min (> 60s TTL)
  
  // Assert - L'entrée doit être valide car expirationDate prend priorité
  QVERIFY(metaData.isValid());
  QVERIFY(m_cache->testIsEntryValid(metaData));
}

QTEST_MAIN(TestNetworkCache)
#include "TestNetworkCache.moc"
