#include <QtTest/QtTest>
#include <QSignalSpy>

#include "core/CacheManager.hpp"

using namespace blueplayer::core;

class TestCacheManager : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Tests des états initiaux
  void testInitialState();

  // Tests du cycle de vie du cache
  void testStartCaching();
  void testStopCaching();
  void testStartStopCycle();

  // Tests des mises à jour
  void testUpdateCacheEnd();
  void testUpdateCacheEndOnlyIncreases();
  void testUpdateCacheSize();

  // Tests de la durée du cache
  void testCacheDurationCalculation();
  void testCacheDurationDynamic();

  // Tests de validation temporelle
  void testIsTimeInCache();
  void testClampToCache();

  // Tests des signaux
  void testCacheDurationChangedSignal();
  void testCacheUpdatedSignal();
  void testIsCachingChangedSignal();

  // Tests de reset
  void testReset();

  // Tests des accesseurs
  void testOldestNewestAvailableTime();

  // Tests de performance (seuils de mise à jour)
  void testUpdateThreshold();

 private:
  CacheManager* m_cacheManager = nullptr;
};

void TestCacheManager::initTestCase() {
  // Configuration globale des tests
}

void TestCacheManager::cleanupTestCase() {
  // Nettoyage global
}

void TestCacheManager::init() {
  m_cacheManager = new CacheManager();
}

void TestCacheManager::cleanup() {
  delete m_cacheManager;
  m_cacheManager = nullptr;
}

void TestCacheManager::testInitialState() {
  QCOMPARE(m_cacheManager->cacheDuration(), 0.0);
  QCOMPARE(m_cacheManager->cacheStartTime(), 0.0);
  QCOMPARE(m_cacheManager->cacheEndTime(), 0.0);
  QCOMPARE(m_cacheManager->cacheSizeBytes(), qint64(0));
  QVERIFY(!m_cacheManager->isCaching());
}

void TestCacheManager::testStartCaching() {
  QSignalSpy cachingSpy(m_cacheManager, &CacheManager::isCachingChanged);

  m_cacheManager->startCaching(10.0);

  QVERIFY(m_cacheManager->isCaching());
  QCOMPARE(m_cacheManager->cacheStartTime(), 10.0);
  QCOMPARE(m_cacheManager->cacheEndTime(), 10.0);
  QCOMPARE(m_cacheManager->cacheDuration(), 0.0);
  QVERIFY(cachingSpy.count() >= 1);
  QCOMPARE(cachingSpy.last().at(0).toBool(), true);
}

void TestCacheManager::testStopCaching() {
  m_cacheManager->startCaching(0.0);
  m_cacheManager->updateCacheEnd(60.0);

  QSignalSpy cachingSpy(m_cacheManager, &CacheManager::isCachingChanged);

  m_cacheManager->stopCaching();

  QVERIFY(!m_cacheManager->isCaching());
  // La durée du cache doit être préservée après l'arrêt
  QCOMPARE(m_cacheManager->cacheDuration(), 60.0);
  QVERIFY(cachingSpy.count() >= 1);
  QCOMPARE(cachingSpy.last().at(0).toBool(), false);
}

void TestCacheManager::testStartStopCycle() {
  // Premier cycle
  m_cacheManager->startCaching(0.0);
  QVERIFY(m_cacheManager->isCaching());

  m_cacheManager->updateCacheEnd(30.0);
  QCOMPARE(m_cacheManager->cacheDuration(), 30.0);

  m_cacheManager->stopCaching();
  QVERIFY(!m_cacheManager->isCaching());

  // Deuxième cycle
  m_cacheManager->startCaching(100.0);
  QVERIFY(m_cacheManager->isCaching());
  QCOMPARE(m_cacheManager->cacheStartTime(), 100.0);
  QCOMPARE(m_cacheManager->cacheEndTime(), 100.0);
}

void TestCacheManager::testUpdateCacheEnd() {
  m_cacheManager->startCaching(0.0);

  QSignalSpy endSpy(m_cacheManager, &CacheManager::cacheEndTimeChanged);

  m_cacheManager->updateCacheEnd(30.0);

  QCOMPARE(m_cacheManager->cacheEndTime(), 30.0);
  QVERIFY(endSpy.count() >= 1);
}

void TestCacheManager::testUpdateCacheEndOnlyIncreases() {
  m_cacheManager->startCaching(0.0);
  m_cacheManager->updateCacheEnd(50.0);

  QCOMPARE(m_cacheManager->cacheEndTime(), 50.0);

  // Tentative de diminuer le temps de fin (doit être ignorée)
  m_cacheManager->updateCacheEnd(30.0);

  QCOMPARE(m_cacheManager->cacheEndTime(), 50.0);

  // Augmenter doit fonctionner
  m_cacheManager->updateCacheEnd(60.0);

  QCOMPARE(m_cacheManager->cacheEndTime(), 60.0);
}

void TestCacheManager::testUpdateCacheSize() {
  QSignalSpy sizeSpy(m_cacheManager, &CacheManager::cacheSizeBytesChanged);

  m_cacheManager->updateCacheSize(1024 * 1024);  // 1 MB

  QCOMPARE(m_cacheManager->cacheSizeBytes(), qint64(1024 * 1024));
  QVERIFY(sizeSpy.count() >= 1);
}

void TestCacheManager::testCacheDurationCalculation() {
  m_cacheManager->startCaching(10.0);
  m_cacheManager->updateCacheEnd(70.0);

  // Duration = endTime - startTime = 70 - 10 = 60
  QCOMPARE(m_cacheManager->cacheDuration(), 60.0);
}

void TestCacheManager::testCacheDurationDynamic() {
  m_cacheManager->startCaching(0.0);

  // Simuler un stream en direct qui avance
  for (double t = 10.0; t <= 120.0; t += 10.0) {
    m_cacheManager->updateCacheEnd(t);
    QCOMPARE(m_cacheManager->cacheDuration(), t);
  }

  QCOMPARE(m_cacheManager->cacheDuration(), 120.0);
}

void TestCacheManager::testIsTimeInCache() {
  m_cacheManager->startCaching(10.0);
  m_cacheManager->updateCacheEnd(60.0);

  // Temps dans le cache
  QVERIFY(m_cacheManager->isTimeInCache(10.0));
  QVERIFY(m_cacheManager->isTimeInCache(35.0));
  QVERIFY(m_cacheManager->isTimeInCache(60.0));

  // Temps hors du cache
  QVERIFY(!m_cacheManager->isTimeInCache(5.0));
  QVERIFY(!m_cacheManager->isTimeInCache(9.99));
  QVERIFY(!m_cacheManager->isTimeInCache(60.01));
  QVERIFY(!m_cacheManager->isTimeInCache(100.0));
}

void TestCacheManager::testClampToCache() {
  m_cacheManager->startCaching(10.0);
  m_cacheManager->updateCacheEnd(60.0);

  // Temps dans le cache (pas de changement)
  QCOMPARE(m_cacheManager->clampToCache(35.0), 35.0);

  // Temps avant le cache (clampé au début)
  QCOMPARE(m_cacheManager->clampToCache(5.0), 10.0);
  QCOMPARE(m_cacheManager->clampToCache(-10.0), 10.0);

  // Temps après le cache (clampé à la fin)
  QCOMPARE(m_cacheManager->clampToCache(70.0), 60.0);
  QCOMPARE(m_cacheManager->clampToCache(1000.0), 60.0);
}

void TestCacheManager::testCacheDurationChangedSignal() {
  QSignalSpy durationSpy(m_cacheManager, &CacheManager::cacheDurationChanged);

  m_cacheManager->startCaching(0.0);
  m_cacheManager->updateCacheEnd(30.0);

  QVERIFY(durationSpy.count() >= 1);
  double lastDuration = durationSpy.last().at(0).toDouble();
  QCOMPARE(lastDuration, 30.0);
}

void TestCacheManager::testCacheUpdatedSignal() {
  QSignalSpy updatedSpy(m_cacheManager, &CacheManager::cacheUpdated);

  m_cacheManager->startCaching(0.0);
  m_cacheManager->updateCacheEnd(30.0);

  QVERIFY(updatedSpy.count() >= 1);
  double lastDuration = updatedSpy.last().at(0).toDouble();
  QCOMPARE(lastDuration, 30.0);
}

void TestCacheManager::testIsCachingChangedSignal() {
  QSignalSpy cachingSpy(m_cacheManager, &CacheManager::isCachingChanged);

  m_cacheManager->startCaching(0.0);
  QVERIFY(cachingSpy.count() >= 1);
  QCOMPARE(cachingSpy.last().at(0).toBool(), true);

  m_cacheManager->stopCaching();
  QVERIFY(cachingSpy.count() >= 2);
  QCOMPARE(cachingSpy.last().at(0).toBool(), false);
}

void TestCacheManager::testReset() {
  m_cacheManager->startCaching(10.0);
  m_cacheManager->updateCacheEnd(100.0);
  m_cacheManager->updateCacheSize(1024 * 1024);

  QSignalSpy resetSpy(m_cacheManager, &CacheManager::cacheUpdated);

  m_cacheManager->reset();

  QCOMPARE(m_cacheManager->cacheStartTime(), 0.0);
  QCOMPARE(m_cacheManager->cacheEndTime(), 0.0);
  QCOMPARE(m_cacheManager->cacheDuration(), 0.0);
  QCOMPARE(m_cacheManager->cacheSizeBytes(), qint64(0));
  QVERIFY(!m_cacheManager->isCaching());
  QVERIFY(resetSpy.count() >= 1);
}

void TestCacheManager::testOldestNewestAvailableTime() {
  m_cacheManager->startCaching(15.0);
  m_cacheManager->updateCacheEnd(75.0);

  QCOMPARE(m_cacheManager->oldestAvailableTime(), 15.0);
  QCOMPARE(m_cacheManager->newestAvailableTime(), 75.0);
}

void TestCacheManager::testUpdateThreshold() {
  // Le seuil de mise à jour est de 0.1 secondes
  // Les mises à jour inférieures au seuil ne doivent pas émettre de signaux

  m_cacheManager->startCaching(0.0);
  m_cacheManager->updateCacheEnd(10.0);

  QSignalSpy durationSpy(m_cacheManager, &CacheManager::cacheDurationChanged);

  // Petite mise à jour (< 0.1s) - ne devrait pas émettre
  m_cacheManager->updateCacheEnd(10.05);
  QCOMPARE(durationSpy.count(), 0);

  // Mise à jour significative (>= 0.1s) - devrait émettre
  m_cacheManager->updateCacheEnd(10.2);
  QVERIFY(durationSpy.count() >= 1);
}

QTEST_MAIN(TestCacheManager)
#include "TestCacheManager.moc"



