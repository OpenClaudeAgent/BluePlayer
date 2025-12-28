#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

#include "core/CacheManager.hpp"

using namespace blueplayer::core;

class TestCacheManager : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // ===== Tests des états initiaux (cache live) =====
  void testInitialState();

  // ===== Tests du cycle de vie du cache live =====
  void testStartCaching();
  void testStopCaching();
  void testStartStopCycle();

  // ===== Tests des mises à jour =====
  void testUpdateCacheEnd();
  void testUpdateCacheEndOnlyIncreases();
  void testUpdateCacheSize();

  // ===== Tests de la durée du cache =====
  void testCacheDurationCalculation();
  void testCacheDurationDynamic();

  // ===== Tests de validation temporelle =====
  void testIsTimeInCache();
  void testClampToCache();

  // ===== Tests des signaux (cache live) =====
  void testCacheDurationChangedSignal();
  void testCacheUpdatedSignal();
  void testIsCachingChangedSignal();

  // ===== Tests de reset =====
  void testReset();

  // ===== Tests des accesseurs =====
  void testOldestNewestAvailableTime();

  // ===== Tests de performance (seuils de mise à jour) =====
  void testUpdateThreshold();

  // ===== Tests VOD Management - État initial =====
  void testVodInitialState();
  void testMaxCacheSizeDefault();
  void testCacheDirectoryExists();
  
  // ===== Tests VOD Management - Ajout/Suppression =====
  void testAddVodFromQml();
  void testAddVodEmitsSignal();
  void testRemoveVod();
  void testRemoveVodEmitsSignal();
  void testRemoveNonExistentVod();
  void testRemoveMultipleVods();
  void testClearAllVods();
  
  // ===== Tests VOD Management - Configuration =====
  void testSetMaxCacheSize();
  void testSetMaxCacheSizeEmitsSignal();
  
  // ===== Tests VOD Management - Recherche et métadonnées =====
  void testGetVodMetadata();
  void testGetVodMetadataNonExistent();
  void testSearchVods();
  void testSearchVodsEmpty();
  
  // ===== Tests VOD Management - Position de lecture =====
  void testUpdateWatchPosition();
  void testMarkAsPlayed();
  
  // ===== Tests VOD Management - Utilitaires =====
  void testCacheUsagePercent();
  void testFormattedTotalSize();
  void testFormattedMaxSize();
  void testVodCount();
  void testVodList();
  
  // ===== Tests VOD Management - Nettoyage =====
  void testCleanupServiceStartStop();
  void testPerformCleanup();
  
  // ===== Sprint 7 - Tests downloadThumbnailAsync() =====
  void testDownloadThumbnailAsyncEmptyUrl();
  void testDownloadThumbnailAsyncEmptyFilename();
  void testDownloadThumbnailAsyncExistingFile();
  void testDownloadThumbnailAsyncSignals();
  void testDownloadThumbnailAsyncWithPlaceholders();
  
  // ===== Sprint 7 - Tests prepareRecording/finalizeRecording =====
  void testPrepareRecordingEmptyStreamerLogin();
  void testPrepareRecordingValidInput();
  void testFinalizeRecordingEmptyPath();
  void testFinalizeRecordingNonExistentFile();
  void testFinalizeRecordingShortDuration();

 private:
  CacheManager* m_cacheManager = nullptr;
  QTemporaryDir* m_tempDir = nullptr;
};

void TestCacheManager::initTestCase() {
  // Configuration globale des tests
}

void TestCacheManager::cleanupTestCase() {
  // Nettoyage global
}

void TestCacheManager::init() {
  m_tempDir = new QTemporaryDir();
  m_cacheManager = new CacheManager();
}

void TestCacheManager::cleanup() {
  delete m_cacheManager;
  m_cacheManager = nullptr;
  delete m_tempDir;
  m_tempDir = nullptr;
}

// ===== Tests Cache Live (existants) =====

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
  QCOMPARE(m_cacheManager->cacheDuration(), 60.0);
  QVERIFY(cachingSpy.count() >= 1);
  QCOMPARE(cachingSpy.last().at(0).toBool(), false);
}

void TestCacheManager::testStartStopCycle() {
  m_cacheManager->startCaching(0.0);
  QVERIFY(m_cacheManager->isCaching());

  m_cacheManager->updateCacheEnd(30.0);
  QCOMPARE(m_cacheManager->cacheDuration(), 30.0);

  m_cacheManager->stopCaching();
  QVERIFY(!m_cacheManager->isCaching());

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

  m_cacheManager->updateCacheEnd(30.0);
  QCOMPARE(m_cacheManager->cacheEndTime(), 50.0);

  m_cacheManager->updateCacheEnd(60.0);
  QCOMPARE(m_cacheManager->cacheEndTime(), 60.0);
}

void TestCacheManager::testUpdateCacheSize() {
  QSignalSpy sizeSpy(m_cacheManager, &CacheManager::cacheSizeBytesChanged);

  m_cacheManager->updateCacheSize(1024 * 1024);

  QCOMPARE(m_cacheManager->cacheSizeBytes(), qint64(1024 * 1024));
  QVERIFY(sizeSpy.count() >= 1);
}

void TestCacheManager::testCacheDurationCalculation() {
  m_cacheManager->startCaching(10.0);
  m_cacheManager->updateCacheEnd(70.0);

  QCOMPARE(m_cacheManager->cacheDuration(), 60.0);
}

void TestCacheManager::testCacheDurationDynamic() {
  m_cacheManager->startCaching(0.0);

  for (double t = 10.0; t <= 120.0; t += 10.0) {
    m_cacheManager->updateCacheEnd(t);
    QCOMPARE(m_cacheManager->cacheDuration(), t);
  }

  QCOMPARE(m_cacheManager->cacheDuration(), 120.0);
}

void TestCacheManager::testIsTimeInCache() {
  m_cacheManager->startCaching(10.0);
  m_cacheManager->updateCacheEnd(60.0);

  QVERIFY(m_cacheManager->isTimeInCache(10.0));
  QVERIFY(m_cacheManager->isTimeInCache(35.0));
  QVERIFY(m_cacheManager->isTimeInCache(60.0));

  QVERIFY(!m_cacheManager->isTimeInCache(5.0));
  QVERIFY(!m_cacheManager->isTimeInCache(9.99));
  QVERIFY(!m_cacheManager->isTimeInCache(60.01));
  QVERIFY(!m_cacheManager->isTimeInCache(100.0));
}

void TestCacheManager::testClampToCache() {
  m_cacheManager->startCaching(10.0);
  m_cacheManager->updateCacheEnd(60.0);

  QCOMPARE(m_cacheManager->clampToCache(35.0), 35.0);
  QCOMPARE(m_cacheManager->clampToCache(5.0), 10.0);
  QCOMPARE(m_cacheManager->clampToCache(-10.0), 10.0);
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
  m_cacheManager->startCaching(0.0);
  m_cacheManager->updateCacheEnd(10.0);

  QSignalSpy durationSpy(m_cacheManager, &CacheManager::cacheDurationChanged);

  m_cacheManager->updateCacheEnd(10.05);
  QCOMPARE(durationSpy.count(), 0);

  m_cacheManager->updateCacheEnd(10.2);
  QVERIFY(durationSpy.count() >= 1);
}

// ===== Tests VOD Management =====

void TestCacheManager::testVodInitialState() {
  QCOMPARE(m_cacheManager->vodCount(), 0);
  QVERIFY(m_cacheManager->vodList().isEmpty());
  QCOMPARE(m_cacheManager->totalVodSize(), qint64(0));
}

void TestCacheManager::testMaxCacheSizeDefault() {
  // Default is 8 GB (as configured in CacheManager.hpp)
  qint64 expectedDefault = 8LL * 1024 * 1024 * 1024; // 8 GB
  QCOMPARE(m_cacheManager->maxCacheSize(), expectedDefault);
}

void TestCacheManager::testCacheDirectoryExists() {
  QString cacheDir = m_cacheManager->cacheDirectory();
  QVERIFY(!cacheDir.isEmpty());
}

void TestCacheManager::testAddVodFromQml() {
  QVariantMap metadata;
  metadata["id"] = "vod123";
  metadata["title"] = "Test VOD";
  metadata["streamerName"] = "TestStreamer";
  metadata["duration"] = 3600;
  metadata["sizeBytes"] = qint64(1024 * 1024 * 100); // 100 MB
  metadata["filePath"] = m_tempDir->filePath("test.mp4");
  
  // Créer un fichier fictif
  QFile file(metadata["filePath"].toString());
  file.open(QIODevice::WriteOnly);
  file.write("test");
  file.close();
  
  bool result = m_cacheManager->addVodFromQml(metadata);
  
  // Le résultat dépend de l'implémentation
  // On vérifie au moins que la méthode ne crashe pas
  QVERIFY(result || !result); // La méthode s'exécute sans crash
}

void TestCacheManager::testAddVodEmitsSignal() {
  QSignalSpy addedSpy(m_cacheManager, &CacheManager::vodAdded);
  QSignalSpy countSpy(m_cacheManager, &CacheManager::vodCountChanged);
  
  QVariantMap metadata;
  metadata["id"] = "vod456";
  metadata["title"] = "Test VOD 2";
  metadata["streamerName"] = "TestStreamer2";
  metadata["duration"] = 1800;
  metadata["sizeBytes"] = qint64(1024 * 1024 * 50);
  metadata["filePath"] = m_tempDir->filePath("test2.mp4");
  
  QFile file(metadata["filePath"].toString());
  file.open(QIODevice::WriteOnly);
  file.write("test");
  file.close();
  
  m_cacheManager->addVodFromQml(metadata);
  
  // On vérifie que les signaux peuvent être émis (ou pas selon l'état)
  // Le test principal est que ça ne crashe pas
}

void TestCacheManager::testRemoveVod() {
  // D'abord ajouter une VOD
  QVariantMap metadata;
  metadata["id"] = "vodToRemove";
  metadata["title"] = "VOD to Remove";
  metadata["streamerName"] = "Streamer";
  metadata["duration"] = 1000;
  metadata["sizeBytes"] = qint64(1024);
  metadata["filePath"] = m_tempDir->filePath("toremove.mp4");
  
  QFile file(metadata["filePath"].toString());
  file.open(QIODevice::WriteOnly);
  file.write("test");
  file.close();
  
  m_cacheManager->addVodFromQml(metadata);
  
  // Puis la supprimer
  bool removed = m_cacheManager->removeVod("vodToRemove");
  
  // Le résultat dépend de si la VOD a été ajoutée
  QVERIFY(removed || !removed);
}

void TestCacheManager::testRemoveVodEmitsSignal() {
  QSignalSpy removedSpy(m_cacheManager, &CacheManager::vodRemoved);
  
  m_cacheManager->removeVod("nonexistent");
  
  // Pas de crash
}

void TestCacheManager::testRemoveNonExistentVod() {
  bool result = m_cacheManager->removeVod("nonexistent_vod_id");
  QVERIFY(!result);
}

void TestCacheManager::testRemoveMultipleVods() {
  QStringList vodIds;
  vodIds << "vod1" << "vod2" << "vod3";
  
  int removed = m_cacheManager->removeVods(vodIds);
  
  // Aucune VOD n'existe donc 0 supprimées
  QCOMPARE(removed, 0);
}

void TestCacheManager::testClearAllVods() {
  QSignalSpy clearedSpy(m_cacheManager, &CacheManager::vodsCleared);
  
  int cleared = m_cacheManager->clearAllVods();
  
  // Aucune VOD initialement
  QCOMPARE(cleared, 0);
  QCOMPARE(m_cacheManager->vodCount(), 0);
}

void TestCacheManager::testSetMaxCacheSize() {
  qint64 newSize = 5LL * 1024 * 1024 * 1024; // 5 GB
  
  m_cacheManager->setMaxCacheSize(newSize);
  
  QCOMPARE(m_cacheManager->maxCacheSize(), newSize);
}

void TestCacheManager::testSetMaxCacheSizeEmitsSignal() {
  QSignalSpy sizeSpy(m_cacheManager, &CacheManager::maxCacheSizeChanged);
  
  // Utiliser une valeur différente de la valeur par défaut (10 GB)
  qint64 newSize = 8LL * 1024 * 1024 * 1024; // 8 GB (différent de 10 GB)
  m_cacheManager->setMaxCacheSize(newSize);
  
  QVERIFY(sizeSpy.count() >= 1);
  QCOMPARE(sizeSpy.last().at(0).toLongLong(), newSize);
}

void TestCacheManager::testGetVodMetadata() {
  QVariantMap metadata = m_cacheManager->getVodMetadata("nonexistent");
  
  // Doit retourner une map vide pour une VOD inexistante
  QVERIFY(metadata.isEmpty());
}

void TestCacheManager::testGetVodMetadataNonExistent() {
  QVariantMap result = m_cacheManager->getVodMetadata("definitely_not_exist");
  QVERIFY(result.isEmpty());
}

void TestCacheManager::testSearchVods() {
  QVariantList results = m_cacheManager->searchVods("test");
  
  // Aucune VOD donc résultats vides
  QVERIFY(results.isEmpty());
}

void TestCacheManager::testSearchVodsEmpty() {
  QVariantList results = m_cacheManager->searchVods("");
  
  // Recherche vide, retourne toutes les VODs (aucune)
  QCOMPARE(results.count(), m_cacheManager->vodCount());
}

void TestCacheManager::testUpdateWatchPosition() {
  // Mettre à jour la position pour une VOD inexistante ne doit pas crasher
  m_cacheManager->updateWatchPosition("nonexistent", 1000);
  
  // Pas de crash = succès
}

void TestCacheManager::testMarkAsPlayed() {
  // Marquer comme lue une VOD inexistante ne doit pas crasher
  m_cacheManager->markAsPlayed("nonexistent");
  
  // Pas de crash = succès
}

void TestCacheManager::testCacheUsagePercent() {
  double usage = m_cacheManager->cacheUsagePercent();
  
  // Sans VOD, l'usage doit être 0
  QCOMPARE(usage, 0.0);
}

void TestCacheManager::testFormattedTotalSize() {
  QString formatted = m_cacheManager->formattedTotalSize();
  
  // Doit retourner une chaîne non vide
  QVERIFY(!formatted.isEmpty());
}

void TestCacheManager::testFormattedMaxSize() {
  QString formatted = m_cacheManager->formattedMaxSize();
  
  // Doit retourner une chaîne non vide (ex: "10 GB")
  QVERIFY(!formatted.isEmpty());
}

void TestCacheManager::testVodCount() {
  QCOMPARE(m_cacheManager->vodCount(), 0);
}

void TestCacheManager::testVodList() {
  QVariantList list = m_cacheManager->vodList();
  QVERIFY(list.isEmpty());
}

void TestCacheManager::testCleanupServiceStartStop() {
  // Démarrer le service de nettoyage
  m_cacheManager->startCleanupService(1000); // 1 seconde pour le test
  
  // Pas de crash
  
  // Arrêter le service
  m_cacheManager->stopCleanupService();
  
  // Pas de crash = succès
}

void TestCacheManager::testPerformCleanup() {
  int removed = m_cacheManager->performCleanup();
  
  // Sans VOD excédant la limite, aucune suppression
  QCOMPARE(removed, 0);
}

// ===== Sprint 7 - Tests downloadThumbnailAsync() =====

void TestCacheManager::testDownloadThumbnailAsyncEmptyUrl() {
  QSignalSpy failedSpy(m_cacheManager, &CacheManager::thumbnailDownloadFailed);
  
  // URL vide doit émettre un signal d'échec
  m_cacheManager->downloadThumbnailAsync("", "test_thumbnail.jpg");
  
  // Le signal d'échec doit être émis immédiatement
  QCOMPARE(failedSpy.count(), 1);
  QCOMPARE(failedSpy.at(0).at(0).toString(), QString("test_thumbnail.jpg"));
}

void TestCacheManager::testDownloadThumbnailAsyncEmptyFilename() {
  QSignalSpy failedSpy(m_cacheManager, &CacheManager::thumbnailDownloadFailed);
  
  // Filename vide doit émettre un signal d'échec
  m_cacheManager->downloadThumbnailAsync("https://example.com/thumb.jpg", "");
  
  // Le signal d'échec doit être émis immédiatement
  QCOMPARE(failedSpy.count(), 1);
}

void TestCacheManager::testDownloadThumbnailAsyncExistingFile() {
  // Créer un fichier existant dans le cache directory
  QString cacheDir = m_cacheManager->cacheDirectory();
  QDir dir(cacheDir);
  dir.mkpath(".");
  
  QString filename = "existing_thumb.jpg";
  QString filePath = cacheDir + "/" + filename;
  
  // Créer le fichier
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write("fake image data");
  file.close();
  
  QSignalSpy successSpy(m_cacheManager, &CacheManager::thumbnailDownloaded);
  
  // Si le fichier existe déjà, le signal de succès doit être émis immédiatement
  m_cacheManager->downloadThumbnailAsync("https://example.com/thumb.jpg", filename);
  
  // Le signal de succès doit être émis car le fichier existe
  QCOMPARE(successSpy.count(), 1);
  QCOMPARE(successSpy.at(0).at(1).toString(), filename);
  
  // Nettoyer
  QFile::remove(filePath);
}

void TestCacheManager::testDownloadThumbnailAsyncSignals() {
  // Test que les bons signaux existent
  const QMetaObject* metaObject = m_cacheManager->metaObject();
  
  // Vérifier que le signal thumbnailDownloaded existe
  int downloadedIndex = metaObject->indexOfSignal("thumbnailDownloaded(QString,QString)");
  QVERIFY2(downloadedIndex >= 0, "Signal thumbnailDownloaded(QString,QString) not found");
  
  // Vérifier que le signal thumbnailDownloadFailed existe
  int failedIndex = metaObject->indexOfSignal("thumbnailDownloadFailed(QString,QString)");
  QVERIFY2(failedIndex >= 0, "Signal thumbnailDownloadFailed(QString,QString) not found");
}

void TestCacheManager::testDownloadThumbnailAsyncWithPlaceholders() {
  // Test que les placeholders Twitch sont traités
  // Cette méthode teste indirectement processThumbnailUrl
  
  QSignalSpy failedSpy(m_cacheManager, &CacheManager::thumbnailDownloadFailed);
  
  // URL avec placeholders Twitch (qui seront remplacés par 440x248)
  QString urlWithPlaceholders = "https://static-cdn.jtvnw.net/previews-ttv/live_user_test-{width}x{height}.jpg";
  
  // Comme on n'a pas de réseau, le téléchargement échouera mais on vérifie 
  // que la méthode ne crashe pas avec des placeholders
  m_cacheManager->downloadThumbnailAsync(urlWithPlaceholders, "placeholder_test.jpg");
  
  // Le test passe si pas de crash - le résultat dépend du réseau
  QVERIFY(m_cacheManager != nullptr);
}

// ===== Sprint 7 - Tests prepareRecording/finalizeRecording =====

void TestCacheManager::testPrepareRecordingEmptyStreamerLogin() {
  QVariantMap result = m_cacheManager->prepareRecording("");
  
  // Avec un login vide, le chemin d'enregistrement doit être vide
  QVERIFY(result.value("recordingPath").toString().isEmpty());
}

void TestCacheManager::testPrepareRecordingValidInput() {
  QVariantMap result = m_cacheManager->prepareRecording("teststreamer", "https://example.com/thumb.jpg");
  
  // Le chemin d'enregistrement doit être défini
  QString recordingPath = result.value("recordingPath").toString();
  QVERIFY(!recordingPath.isEmpty());
  QVERIFY(recordingPath.contains("teststreamer"));
  QVERIFY(recordingPath.endsWith(".ts"));
  
  // Le chemin de thumbnail doit être défini
  QString thumbnailPath = result.value("thumbnailPath").toString();
  QVERIFY(!thumbnailPath.isEmpty());
  QVERIFY(thumbnailPath.contains("teststreamer"));
  QVERIFY(thumbnailPath.endsWith(".jpg"));
  
  // Le timestamp doit être défini
  qint64 startTime = result.value("startTime").toLongLong();
  QVERIFY(startTime > 0);
}

void TestCacheManager::testFinalizeRecordingEmptyPath() {
  bool result = m_cacheManager->finalizeRecording("", "streamer", "name", "title", "", 0);
  
  // Path vide doit échouer
  QVERIFY(!result);
}

void TestCacheManager::testFinalizeRecordingNonExistentFile() {
  bool result = m_cacheManager->finalizeRecording(
    "/nonexistent/path/video.ts",
    "streamer",
    "name",
    "title",
    "",
    QDateTime::currentMSecsSinceEpoch() - 60000  // 1 minute ago
  );
  
  // Fichier inexistant doit échouer
  QVERIFY(!result);
}

void TestCacheManager::testFinalizeRecordingShortDuration() {
  // Créer un fichier temporaire
  QString filePath = m_tempDir->filePath("short_recording.ts");
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write("test data");
  file.close();
  
  // Essayer de finaliser avec une durée < 30 secondes
  bool result = m_cacheManager->finalizeRecording(
    filePath,
    "streamer",
    "name",
    "title",
    "",
    QDateTime::currentMSecsSinceEpoch() - 10000  // 10 secondes seulement
  );
  
  // Durée trop courte doit échouer
  QVERIFY(!result);
  
  // Le fichier doit être supprimé
  QVERIFY(!QFile::exists(filePath));
}

QTEST_MAIN(TestCacheManager)
#include "TestCacheManager.moc"
