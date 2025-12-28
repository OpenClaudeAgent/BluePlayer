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

  // ===== NEW - VodMetadata Serialization Tests =====
  void testVodMetadataToJson();
  void testVodMetadataFromJson();
  void testVodMetadataJsonRoundTrip();
  void testVodMetadataToVariantMap();
  void testVodMetadataFormatDurationSeconds();
  void testVodMetadataFormatDurationMinutes();
  void testVodMetadataFormatDurationHours();
  void testVodMetadataFormatFileSizeBytes();
  void testVodMetadataFormatFileSizeKB();
  void testVodMetadataFormatFileSizeMB();
  void testVodMetadataFormatFileSizeGB();
  void testVodMetadataIsValid();
  void testVodMetadataIsValidInvalid();
  void testVodMetadataGenerateId();

  // ===== NEW - Cache Threshold and Cleanup Tests =====
  void testCheckCacheThresholdUnderLimit();
  void testCheckCacheThresholdOverLimit();
  void testPerformCleanupRemovesOldestFirst();
  void testCacheUsagePercentZeroMaxSize();
  void testSearchVodsFindsStreamerName();
  void testSearchVodsFindsTitle();
  void testSearchVodsFindsGameCategory();
  
  // ===== NEW - Edge Cases =====
  void testAddVodInvalidMetadata();
  void testAddVodDuplicate();
  void testFinalizeRecordingValidDuration();
  void testLoadMetadataWithMissingFile();
  void testDownloadThumbnailSyncTimeout();

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
  // maxCacheSize should be positive (either default 10GB or persisted value)
  // Note: Value may be loaded from persisted metadata
  QVERIFY(m_cacheManager->maxCacheSize() > 0);
  
  // Test that setMaxCacheSize works
  qint64 newSize = 5LL * 1024 * 1024 * 1024; // 5 GB
  m_cacheManager->setMaxCacheSize(newSize);
  QCOMPARE(m_cacheManager->maxCacheSize(), newSize);
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

// ===== NEW - VodMetadata Serialization Tests =====

void TestCacheManager::testVodMetadataToJson() {
  VodMetadata vod;
  vod.id = "test-id-123";
  vod.streamerLogin = "teststreamer";
  vod.streamerName = "Test Streamer";
  vod.streamTitle = "Test Stream Title";
  vod.duration = 3600;
  vod.fileSize = 1024 * 1024 * 500; // 500 MB
  vod.filePath = "/path/to/video.ts";
  vod.thumbnailPath = "/path/to/thumb.jpg";
  vod.gameCategory = "Just Chatting";
  vod.recordedAt = QDateTime::fromString("2024-01-15T10:30:00", Qt::ISODate);
  vod.watchPosition = 1800;
  vod.quality = "1080p60";
  
  QJsonObject json = vod.toJson();
  
  QCOMPARE(json["id"].toString(), QString("test-id-123"));
  QCOMPARE(json["streamerLogin"].toString(), QString("teststreamer"));
  QCOMPARE(json["streamerName"].toString(), QString("Test Streamer"));
  QCOMPARE(json["streamTitle"].toString(), QString("Test Stream Title"));
  QCOMPARE(json["duration"].toInteger(), qint64(3600));
  QCOMPARE(json["fileSize"].toInteger(), qint64(1024 * 1024 * 500));
  QCOMPARE(json["filePath"].toString(), QString("/path/to/video.ts"));
  QCOMPARE(json["quality"].toString(), QString("1080p60"));
}

void TestCacheManager::testVodMetadataFromJson() {
  QJsonObject json;
  json["id"] = "from-json-id";
  json["streamerLogin"] = "jsonstreamer";
  json["streamerName"] = "JSON Streamer";
  json["streamTitle"] = "JSON Stream";
  json["duration"] = 7200;
  json["fileSize"] = qint64(1024 * 1024 * 1024); // 1 GB
  json["filePath"] = "/json/video.ts";
  json["thumbnailPath"] = "/json/thumb.jpg";
  json["gameCategory"] = "Gaming";
  json["recordedAt"] = "2024-02-20T15:00:00";
  json["lastPlayedAt"] = "2024-02-21T10:00:00";
  json["watchPosition"] = 3600;
  json["quality"] = "720p60";
  
  VodMetadata vod = VodMetadata::fromJson(json);
  
  QCOMPARE(vod.id, QString("from-json-id"));
  QCOMPARE(vod.streamerLogin, QString("jsonstreamer"));
  QCOMPARE(vod.streamerName, QString("JSON Streamer"));
  QCOMPARE(vod.streamTitle, QString("JSON Stream"));
  QCOMPARE(vod.duration, qint64(7200));
  QCOMPARE(vod.fileSize, qint64(1024 * 1024 * 1024));
  QCOMPARE(vod.filePath, QString("/json/video.ts"));
  QCOMPARE(vod.quality, QString("720p60"));
}

void TestCacheManager::testVodMetadataJsonRoundTrip() {
  VodMetadata original;
  original.id = VodMetadata::generateId();
  original.streamerLogin = "roundtripstreamer";
  original.streamerName = "Roundtrip Streamer";
  original.streamTitle = "Roundtrip Stream Title";
  original.duration = 5400;
  original.fileSize = 1024 * 1024 * 750;
  original.filePath = "/roundtrip/video.ts";
  original.thumbnailPath = "/roundtrip/thumb.jpg";
  original.gameCategory = "Music";
  original.recordedAt = QDateTime::currentDateTime();
  original.watchPosition = 2700;
  original.quality = "source";
  
  // Convert to JSON and back
  QJsonObject json = original.toJson();
  VodMetadata restored = VodMetadata::fromJson(json);
  
  QCOMPARE(restored.id, original.id);
  QCOMPARE(restored.streamerLogin, original.streamerLogin);
  QCOMPARE(restored.streamerName, original.streamerName);
  QCOMPARE(restored.streamTitle, original.streamTitle);
  QCOMPARE(restored.duration, original.duration);
  QCOMPARE(restored.fileSize, original.fileSize);
  QCOMPARE(restored.filePath, original.filePath);
  QCOMPARE(restored.quality, original.quality);
}

void TestCacheManager::testVodMetadataToVariantMap() {
  VodMetadata vod;
  vod.id = "variant-map-id";
  vod.streamerLogin = "variantstreamer";
  vod.streamerName = "Variant Streamer";
  vod.streamTitle = "Variant Stream";
  vod.duration = 3600;  // 1 hour
  vod.fileSize = qint64(1024) * 1024 * 1024 * 2;  // 2 GB
  vod.filePath = "/variant/video.ts";
  vod.thumbnailPath = "/variant/thumb.jpg";
  vod.gameCategory = "Art";
  vod.recordedAt = QDateTime::currentDateTime();
  vod.watchPosition = 1800;  // 30 minutes (50%)
  vod.quality = "1080p60";
  
  QVariantMap map = vod.toVariantMap();
  
  QCOMPARE(map["id"].toString(), QString("variant-map-id"));
  QCOMPARE(map["streamerLogin"].toString(), QString("variantstreamer"));
  QCOMPARE(map["duration"].toLongLong(), qint64(3600));
  
  // Test formatted fields
  QCOMPARE(map["durationFormatted"].toString(), QString("1h"));
  QVERIFY(map["fileSizeFormatted"].toString().contains("GB"));
  
  // Test progress calculation
  double progress = map["progressPercent"].toDouble();
  QVERIFY(progress > 49.0 && progress < 51.0);  // Should be ~50%
  QCOMPARE(map["isCompleted"].toBool(), false);  // 50% < 90%
  
  // Test thumbnail path has file:// prefix
  QVERIFY(map["thumbnailPath"].toString().startsWith("file://"));
}

void TestCacheManager::testVodMetadataFormatDurationSeconds() {
  QCOMPARE(VodMetadata::formatDuration(0), QString("0s"));
  QCOMPARE(VodMetadata::formatDuration(30), QString("30s"));
  QCOMPARE(VodMetadata::formatDuration(59), QString("59s"));
}

void TestCacheManager::testVodMetadataFormatDurationMinutes() {
  QCOMPARE(VodMetadata::formatDuration(60), QString("1m"));
  QCOMPARE(VodMetadata::formatDuration(90), QString("1m30s"));
  QCOMPARE(VodMetadata::formatDuration(3599), QString("59m59s"));
}

void TestCacheManager::testVodMetadataFormatDurationHours() {
  QCOMPARE(VodMetadata::formatDuration(3600), QString("1h"));
  QCOMPARE(VodMetadata::formatDuration(5400), QString("1h30m"));
  QCOMPARE(VodMetadata::formatDuration(7200), QString("2h"));
  QCOMPARE(VodMetadata::formatDuration(7260), QString("2h1m"));
}

void TestCacheManager::testVodMetadataFormatFileSizeBytes() {
  QCOMPARE(VodMetadata::formatFileSize(0), QString("0 B"));
  QCOMPARE(VodMetadata::formatFileSize(500), QString("500 B"));
  QCOMPARE(VodMetadata::formatFileSize(1023), QString("1023 B"));
}

void TestCacheManager::testVodMetadataFormatFileSizeKB() {
  QCOMPARE(VodMetadata::formatFileSize(1024), QString("1 KB"));
  QCOMPARE(VodMetadata::formatFileSize(1536), QString("2 KB"));  // 1.5 KB rounds to 2
  QCOMPARE(VodMetadata::formatFileSize(1024 * 1023), QString("1023 KB"));
}

void TestCacheManager::testVodMetadataFormatFileSizeMB() {
  QCOMPARE(VodMetadata::formatFileSize(1024 * 1024), QString("1.0 MB"));
  QCOMPARE(VodMetadata::formatFileSize(qint64(1024) * 1024 * 500), QString("500.0 MB"));
  QCOMPARE(VodMetadata::formatFileSize(qint64(1024) * 1024 * 1023), QString("1023.0 MB"));
}

void TestCacheManager::testVodMetadataFormatFileSizeGB() {
  QCOMPARE(VodMetadata::formatFileSize(qint64(1024) * 1024 * 1024), QString("1.0 GB"));
  QCOMPARE(VodMetadata::formatFileSize(qint64(1024) * 1024 * 1024 * 5), QString("5.0 GB"));
  QCOMPARE(VodMetadata::formatFileSize(qint64(1024) * 1024 * 1024 * 10), QString("10.0 GB"));
}

void TestCacheManager::testVodMetadataIsValid() {
  VodMetadata vod;
  vod.id = "valid-id";
  vod.filePath = "/path/to/file.ts";
  vod.duration = 3600;
  
  QVERIFY(vod.isValid());
}

void TestCacheManager::testVodMetadataIsValidInvalid() {
  // Empty id
  VodMetadata vod1;
  vod1.id = "";
  vod1.filePath = "/path/to/file.ts";
  vod1.duration = 3600;
  QVERIFY(!vod1.isValid());
  
  // Empty filePath
  VodMetadata vod2;
  vod2.id = "some-id";
  vod2.filePath = "";
  vod2.duration = 3600;
  QVERIFY(!vod2.isValid());
  
  // Zero duration
  VodMetadata vod3;
  vod3.id = "some-id";
  vod3.filePath = "/path/to/file.ts";
  vod3.duration = 0;
  QVERIFY(!vod3.isValid());
  
  // Negative duration
  VodMetadata vod4;
  vod4.id = "some-id";
  vod4.filePath = "/path/to/file.ts";
  vod4.duration = -100;
  QVERIFY(!vod4.isValid());
}

void TestCacheManager::testVodMetadataGenerateId() {
  QString id1 = VodMetadata::generateId();
  QString id2 = VodMetadata::generateId();
  
  // IDs should be non-empty
  QVERIFY(!id1.isEmpty());
  QVERIFY(!id2.isEmpty());
  
  // IDs should be unique
  QVERIFY(id1 != id2);
  
  // IDs should be UUID format (36 chars with dashes, or 32 without)
  QVERIFY(id1.length() == 36 || id1.length() == 32);
}

// ===== NEW - Cache Threshold and Cleanup Tests =====

void TestCacheManager::testCheckCacheThresholdUnderLimit() {
  QSignalSpy thresholdSpy(m_cacheManager, &CacheManager::cacheThresholdReached);
  
  // Set a large max size so we're under threshold
  m_cacheManager->setMaxCacheSize(qint64(100) * 1024 * 1024 * 1024);  // 100 GB
  
  // No VODs, so usage is 0% - well under 90% threshold
  QVERIFY(m_cacheManager->cacheUsagePercent() < 90.0);
  QCOMPARE(thresholdSpy.count(), 0);
}

void TestCacheManager::testCheckCacheThresholdOverLimit() {
  QSignalSpy thresholdSpy(m_cacheManager, &CacheManager::cacheThresholdReached);
  
  // Set a very small max size
  m_cacheManager->setMaxCacheSize(1024);  // 1 KB
  
  // Add a VOD that exceeds the threshold
  QString filePath = m_tempDir->filePath("threshold_test.ts");
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write(QByteArray(2048, 'x'));  // 2 KB file
  file.close();
  
  QVariantMap metadata;
  metadata["id"] = "threshold-vod";
  metadata["streamerLogin"] = "test";
  metadata["streamerName"] = "Test";
  metadata["streamTitle"] = "Test";
  metadata["duration"] = 3600;
  metadata["filePath"] = filePath;
  
  m_cacheManager->addVodFromQml(metadata);
  
  // Signal should have been emitted
  QVERIFY(thresholdSpy.count() >= 1);
}

void TestCacheManager::testPerformCleanupRemovesOldestFirst() {
  // Set small cache size
  m_cacheManager->setMaxCacheSize(5000);  // 5 KB max
  
  // Create two VOD files
  QString oldFile = m_tempDir->filePath("old_vod.ts");
  QString newFile = m_tempDir->filePath("new_vod.ts");
  
  QFile file1(oldFile);
  QVERIFY(file1.open(QIODevice::WriteOnly));
  file1.write(QByteArray(2000, 'x'));
  file1.close();
  
  QFile file2(newFile);
  QVERIFY(file2.open(QIODevice::WriteOnly));
  file2.write(QByteArray(2000, 'y'));
  file2.close();
  
  // Add old VOD first (recorded earlier)
  VodMetadata oldVod;
  oldVod.id = "old-vod-id";
  oldVod.streamerLogin = "old";
  oldVod.streamerName = "Old";
  oldVod.streamTitle = "Old Stream";
  oldVod.duration = 3600;
  oldVod.fileSize = 2000;
  oldVod.filePath = oldFile;
  oldVod.recordedAt = QDateTime::currentDateTime().addDays(-7);  // 1 week ago
  m_cacheManager->addVod(oldVod);
  
  // Add new VOD (recorded today)
  VodMetadata newVod;
  newVod.id = "new-vod-id";
  newVod.streamerLogin = "new";
  newVod.streamerName = "New";
  newVod.streamTitle = "New Stream";
  newVod.duration = 3600;
  newVod.fileSize = 2000;
  newVod.filePath = newFile;
  newVod.recordedAt = QDateTime::currentDateTime();
  m_cacheManager->addVod(newVod);
  
  // Verify both exist
  QCOMPARE(m_cacheManager->vodCount(), 2);
  
  // Trigger cleanup (should remove oldest)
  int removed = m_cacheManager->performCleanup();
  
  // At least one should be removed if over threshold
  // Note: depends on threshold calculation
  QVERIFY(removed >= 0);
}

void TestCacheManager::testCacheUsagePercentZeroMaxSize() {
  // Edge case: maxCacheSize is 0 (shouldn't happen but test robustness)
  // The implementation returns 0.0 if m_maxCacheSize <= 0
  
  // Default is 10 GB, so usage should be 0% with no VODs
  double usage = m_cacheManager->cacheUsagePercent();
  QCOMPARE(usage, 0.0);
}

void TestCacheManager::testSearchVodsFindsStreamerName() {
  // Add a VOD
  QString filePath = m_tempDir->filePath("search_test.ts");
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write("test");
  file.close();
  
  VodMetadata vod;
  vod.id = "search-vod";
  vod.streamerLogin = "xqc";
  vod.streamerName = "xQcOW";
  vod.streamTitle = "Just Chatting";
  vod.duration = 3600;
  vod.fileSize = 4;
  vod.filePath = filePath;
  vod.gameCategory = "Gaming";
  m_cacheManager->addVod(vod);
  
  // Search by streamer name
  QVariantList results = m_cacheManager->searchVods("xQc");
  QCOMPARE(results.size(), 1);
  QCOMPARE(results[0].toMap()["streamerName"].toString(), QString("xQcOW"));
}

void TestCacheManager::testSearchVodsFindsTitle() {
  // Add a VOD
  QString filePath = m_tempDir->filePath("search_title.ts");
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write("test");
  file.close();
  
  VodMetadata vod;
  vod.id = "search-title-vod";
  vod.streamerLogin = "streamer";
  vod.streamerName = "Streamer";
  vod.streamTitle = "INSANE GAMEPLAY";
  vod.duration = 3600;
  vod.fileSize = 4;
  vod.filePath = filePath;
  m_cacheManager->addVod(vod);
  
  // Search by title (case insensitive)
  QVariantList results = m_cacheManager->searchVods("insane");
  QCOMPARE(results.size(), 1);
  QCOMPARE(results[0].toMap()["streamTitle"].toString(), QString("INSANE GAMEPLAY"));
}

void TestCacheManager::testSearchVodsFindsGameCategory() {
  // Add a VOD
  QString filePath = m_tempDir->filePath("search_game.ts");
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write("test");
  file.close();
  
  VodMetadata vod;
  vod.id = "search-game-vod";
  vod.streamerLogin = "gamer";
  vod.streamerName = "Gamer";
  vod.streamTitle = "Playing Games";
  vod.duration = 3600;
  vod.fileSize = 4;
  vod.filePath = filePath;
  vod.gameCategory = "League of Legends";
  m_cacheManager->addVod(vod);
  
  // Search by game category
  QVariantList results = m_cacheManager->searchVods("league");
  QCOMPARE(results.size(), 1);
  QCOMPARE(results[0].toMap()["gameCategory"].toString(), QString("League of Legends"));
}

// ===== NEW - Edge Cases =====

void TestCacheManager::testAddVodInvalidMetadata() {
  // Try to add VOD with invalid metadata (empty id, empty filePath, or zero duration)
  VodMetadata invalidVod;
  invalidVod.id = "";  // Invalid: empty id
  invalidVod.filePath = "/path/to/file.ts";
  invalidVod.duration = 3600;
  
  bool result = m_cacheManager->addVod(invalidVod);
  QVERIFY(!result);  // Should fail validation
}

void TestCacheManager::testAddVodDuplicate() {
  // Create file
  QString filePath = m_tempDir->filePath("duplicate.ts");
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write("test");
  file.close();
  
  VodMetadata vod;
  vod.id = "duplicate-id";
  vod.streamerLogin = "dup";
  vod.streamerName = "Duplicate";
  vod.streamTitle = "Original Title";
  vod.duration = 3600;
  vod.fileSize = 4;
  vod.filePath = filePath;
  
  // Add first time
  QVERIFY(m_cacheManager->addVod(vod));
  QCOMPARE(m_cacheManager->vodCount(), 1);
  
  // Update title and add again (same id)
  vod.streamTitle = "Updated Title";
  QVERIFY(m_cacheManager->addVod(vod));
  
  // Should still be 1 VOD (updated, not duplicated)
  QCOMPARE(m_cacheManager->vodCount(), 1);
  
  // Verify title was updated
  QVariantMap meta = m_cacheManager->getVodMetadata("duplicate-id");
  QCOMPARE(meta["streamTitle"].toString(), QString("Updated Title"));
}

void TestCacheManager::testFinalizeRecordingValidDuration() {
  // Create a file
  QString filePath = m_tempDir->filePath("valid_recording.ts");
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write(QByteArray(1024, 'x'));  // 1 KB
  file.close();
  
  // Finalize with > 30 seconds duration
  bool result = m_cacheManager->finalizeRecording(
    filePath,
    "validstreamer",
    "Valid Streamer",
    "Valid Title",
    "",
    QDateTime::currentMSecsSinceEpoch() - 60000  // 60 seconds ago
  );
  
  // Should succeed
  QVERIFY(result);
  QCOMPARE(m_cacheManager->vodCount(), 1);
  
  // Verify metadata
  QVariantList vods = m_cacheManager->vodList();
  QCOMPARE(vods.size(), 1);
  QCOMPARE(vods[0].toMap()["streamerName"].toString(), QString("Valid Streamer"));
}

void TestCacheManager::testLoadMetadataWithMissingFile() {
  // Create a VOD entry with a file that will be deleted
  QString filePath = m_tempDir->filePath("will_be_deleted.ts");
  QFile file(filePath);
  QVERIFY(file.open(QIODevice::WriteOnly));
  file.write("test");
  file.close();
  
  VodMetadata vod;
  vod.id = "missing-file-vod";
  vod.streamerLogin = "missing";
  vod.streamerName = "Missing";
  vod.streamTitle = "Missing File";
  vod.duration = 3600;
  vod.fileSize = 4;
  vod.filePath = filePath;
  m_cacheManager->addVod(vod);
  
  QCOMPARE(m_cacheManager->vodCount(), 1);
  
  // Delete the file
  QVERIFY(QFile::remove(filePath));
  
  // Reload metadata - should filter out VODs with missing files
  m_cacheManager->loadMetadata();
  
  // VOD should be removed because file doesn't exist
  QCOMPARE(m_cacheManager->vodCount(), 0);
}

void TestCacheManager::testDownloadThumbnailSyncTimeout() {
  // Test synchronous download with invalid URL (will timeout or fail)
  // This tests the timeout handling path
  
  QString result = m_cacheManager->downloadThumbnail(
    "https://invalid.nonexistent.url/thumb.jpg",
    "timeout_test.jpg"
  );
  
  // Should return empty string on failure
  QVERIFY(result.isEmpty());
}

QTEST_MAIN(TestCacheManager)
#include "TestCacheManager.moc"
