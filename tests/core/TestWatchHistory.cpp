#include <QtTest/QtTest>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

#include "core/WatchHistory.hpp"

using namespace blueplayer::core;

class TestWatchHistory : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Tests d'initialisation
  void testConstructor();
  
  // Tests de sauvegarde de position
  void testSaveWatchPosition();
  void testSaveWatchPositionZero();
  void testSaveWatchPositionLarge();
  void testSaveWatchPositionOverwrite();
  void testSaveWatchPositionMultipleVideos();
  
  // Tests de récupération de position
  void testGetWatchPosition();
  void testGetWatchPositionNonExistent();
  void testGetWatchPositionAfterSave();
  
  // Tests de la liste des vidéos en cours
  void testGetVideosInProgressEmpty();
  void testGetVideosInProgressAfterSave();
  void testGetVideosInProgressMultiple();
  
  // Tests de suppression
  void testRemoveVideo();
  void testRemoveVideoNonExistent();
  void testRemoveVideoAndVerify();
  
  // Tests d'effacement de l'historique
  void testClearHistory();
  void testClearHistoryEmpty();
  void testClearHistoryMultiple();
  
  // Tests de persistance
  void testPersistenceAcrossInstances();

private:
  WatchHistory* m_watchHistory = nullptr;
  QString m_testSettingsPath;
};

void TestWatchHistory::initTestCase() {
  // Utiliser un chemin de test isolé
  m_testSettingsPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) 
                       + "/BluePlayerWatchHistoryTest";
  QDir().mkpath(m_testSettingsPath);
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, m_testSettingsPath);
}

void TestWatchHistory::cleanupTestCase() {
  // Nettoyer le répertoire de test
  QDir testDir(m_testSettingsPath);
  if (testDir.exists()) {
    testDir.removeRecursively();
  }
}

void TestWatchHistory::init() {
  m_watchHistory = new WatchHistory(this);
  m_watchHistory->clearHistory();
}

void TestWatchHistory::cleanup() {
  if (m_watchHistory) {
    m_watchHistory->clearHistory();
    delete m_watchHistory;
    m_watchHistory = nullptr;
  }
}

// ===== Tests d'initialisation =====

void TestWatchHistory::testConstructor() {
  WatchHistory history;
  QVERIFY(&history != nullptr);
}

// ===== Tests de sauvegarde de position =====

void TestWatchHistory::testSaveWatchPosition() {
  m_watchHistory->saveWatchPosition("video123", 1500);
  
  qint64 position = m_watchHistory->getWatchPosition("video123");
  QCOMPARE(position, qint64(1500));
}

void TestWatchHistory::testSaveWatchPositionZero() {
  m_watchHistory->saveWatchPosition("video456", 0);
  
  qint64 position = m_watchHistory->getWatchPosition("video456");
  QCOMPARE(position, qint64(0));
}

void TestWatchHistory::testSaveWatchPositionLarge() {
  qint64 largePosition = 36000; // 10 heures en secondes
  m_watchHistory->saveWatchPosition("longvideo", largePosition);
  
  qint64 position = m_watchHistory->getWatchPosition("longvideo");
  QCOMPARE(position, largePosition);
}

void TestWatchHistory::testSaveWatchPositionOverwrite() {
  m_watchHistory->saveWatchPosition("video789", 1000);
  m_watchHistory->saveWatchPosition("video789", 2000);
  m_watchHistory->saveWatchPosition("video789", 3000);
  
  qint64 position = m_watchHistory->getWatchPosition("video789");
  QCOMPARE(position, qint64(3000));
}

void TestWatchHistory::testSaveWatchPositionMultipleVideos() {
  m_watchHistory->saveWatchPosition("video1", 100);
  m_watchHistory->saveWatchPosition("video2", 200);
  m_watchHistory->saveWatchPosition("video3", 300);
  
  QCOMPARE(m_watchHistory->getWatchPosition("video1"), qint64(100));
  QCOMPARE(m_watchHistory->getWatchPosition("video2"), qint64(200));
  QCOMPARE(m_watchHistory->getWatchPosition("video3"), qint64(300));
}

// ===== Tests de récupération de position =====

void TestWatchHistory::testGetWatchPosition() {
  m_watchHistory->saveWatchPosition("testVideo", 500);
  
  qint64 position = m_watchHistory->getWatchPosition("testVideo");
  QCOMPARE(position, qint64(500));
}

void TestWatchHistory::testGetWatchPositionNonExistent() {
  qint64 position = m_watchHistory->getWatchPosition("nonexistent");
  QCOMPARE(position, qint64(0));
}

void TestWatchHistory::testGetWatchPositionAfterSave() {
  // Vérifier que la position est bien récupérée après sauvegarde
  QString videoId = "unique_video_id";
  qint64 expectedPosition = 12345;
  
  m_watchHistory->saveWatchPosition(videoId, expectedPosition);
  qint64 retrievedPosition = m_watchHistory->getWatchPosition(videoId);
  
  QCOMPARE(retrievedPosition, expectedPosition);
}

// ===== Tests de la liste des vidéos en cours =====

void TestWatchHistory::testGetVideosInProgressEmpty() {
  QVariantList videos = m_watchHistory->getVideosInProgress();
  QVERIFY(videos.isEmpty());
}

void TestWatchHistory::testGetVideosInProgressAfterSave() {
  m_watchHistory->saveWatchPosition("video_in_progress", 500);
  
  QVariantList videos = m_watchHistory->getVideosInProgress();
  QVERIFY(!videos.isEmpty());
  QCOMPARE(videos.size(), 1);
}

void TestWatchHistory::testGetVideosInProgressMultiple() {
  m_watchHistory->saveWatchPosition("video_a", 100);
  m_watchHistory->saveWatchPosition("video_b", 200);
  m_watchHistory->saveWatchPosition("video_c", 300);
  
  QVariantList videos = m_watchHistory->getVideosInProgress();
  QCOMPARE(videos.size(), 3);
}

// ===== Tests de suppression =====

void TestWatchHistory::testRemoveVideo() {
  m_watchHistory->saveWatchPosition("to_remove", 1000);
  m_watchHistory->removeVideo("to_remove");
  
  qint64 position = m_watchHistory->getWatchPosition("to_remove");
  QCOMPARE(position, qint64(0));
}

void TestWatchHistory::testRemoveVideoNonExistent() {
  // Ne doit pas crasher
  m_watchHistory->removeVideo("does_not_exist");
  QVERIFY(m_watchHistory != nullptr);
}

void TestWatchHistory::testRemoveVideoAndVerify() {
  m_watchHistory->saveWatchPosition("keep_this", 100);
  m_watchHistory->saveWatchPosition("remove_this", 200);
  
  m_watchHistory->removeVideo("remove_this");
  
  // "keep_this" doit toujours exister
  QCOMPARE(m_watchHistory->getWatchPosition("keep_this"), qint64(100));
  
  // "remove_this" doit être supprimé
  QCOMPARE(m_watchHistory->getWatchPosition("remove_this"), qint64(0));
  
  // La liste ne doit contenir qu'une vidéo
  QVariantList videos = m_watchHistory->getVideosInProgress();
  QCOMPARE(videos.size(), 1);
}

// ===== Tests d'effacement de l'historique =====

void TestWatchHistory::testClearHistory() {
  m_watchHistory->saveWatchPosition("video1", 100);
  m_watchHistory->saveWatchPosition("video2", 200);
  
  m_watchHistory->clearHistory();
  
  QCOMPARE(m_watchHistory->getWatchPosition("video1"), qint64(0));
  QCOMPARE(m_watchHistory->getWatchPosition("video2"), qint64(0));
  QVERIFY(m_watchHistory->getVideosInProgress().isEmpty());
}

void TestWatchHistory::testClearHistoryEmpty() {
  // Clear sur un historique vide ne doit pas crasher
  m_watchHistory->clearHistory();
  QVERIFY(m_watchHistory->getVideosInProgress().isEmpty());
}

void TestWatchHistory::testClearHistoryMultiple() {
  // Plusieurs appels à clearHistory doivent être idempotents
  m_watchHistory->saveWatchPosition("video", 500);
  m_watchHistory->clearHistory();
  m_watchHistory->clearHistory();
  m_watchHistory->clearHistory();
  
  QVERIFY(m_watchHistory->getVideosInProgress().isEmpty());
}

// ===== Tests de persistance =====

void TestWatchHistory::testPersistenceAcrossInstances() {
  // Sauvegarder avec une instance
  m_watchHistory->saveWatchPosition("persistent_video", 999);
  delete m_watchHistory;
  
  // Créer une nouvelle instance et vérifier la persistance
  m_watchHistory = new WatchHistory(this);
  qint64 position = m_watchHistory->getWatchPosition("persistent_video");
  
  QCOMPARE(position, qint64(999));
}

QTEST_MAIN(TestWatchHistory)
#include "TestWatchHistory.moc"
