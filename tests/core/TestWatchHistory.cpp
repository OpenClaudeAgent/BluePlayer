#include <QtTest/QtTest>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>

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

  // Tests saveWatchProgress (avec durée et calcul completed)
  void testSaveWatchProgress();
  void testSaveWatchProgressEmptyVideoId();
  void testSaveWatchProgressCompletedAt90Percent();
  void testSaveWatchProgressNotCompletedAt89Percent();
  void testSaveWatchProgressZeroDuration();

  // Tests getWatchProgress (struct complète)
  void testGetWatchProgress();
  void testGetWatchProgressEmptyVideoId();
  void testGetWatchProgressNonExistent();
  void testGetWatchProgressAfterSaveProgress();

  // Tests markAsCompleted
  void testMarkAsCompleted();
  void testMarkAsCompletedEmptyVideoId();
  void testMarkAsCompletedVerifyState();

  // Tests hasProgress
  void testHasProgress();
  void testHasProgressFalseForNonExistent();
  void testHasProgressEmptyVideoId();
  void testHasProgressFalseForZeroPosition();

  // Tests formatLastWatched (fonction statique)
  void testFormatLastWatchedJustNow();
  void testFormatLastWatchedMinutes();
  void testFormatLastWatchedOneMinute();
  void testFormatLastWatchedHours();
  void testFormatLastWatchedOneHour();
  void testFormatLastWatchedYesterday();
  void testFormatLastWatchedDays();
  void testFormatLastWatchedWeeks();
  void testFormatLastWatchedOneWeek();
  void testFormatLastWatchedMonths();
  void testFormatLastWatchedOneMonth();
  void testFormatLastWatchedInvalidDate();
  void testFormatLastWatchedFutureDate();

  // Tests du signal progressUpdated
  void testProgressUpdatedSignalOnSavePosition();
  void testProgressUpdatedSignalOnSaveProgress();
  void testProgressUpdatedSignalOnMarkAsCompleted();
  void testNoSignalOnEmptyVideoId();

private:
  WatchHistory* m_watchHistory = nullptr;
  QString m_testSettingsPath;
};

void TestWatchHistory::initTestCase() {
  // Use a unique path per test process to avoid conflicts during parallel execution
  QString uniqueId = QUuid::createUuid().toString(QUuid::Id128);
  m_testSettingsPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) 
                       + "/BluePlayerWatchHistoryTest_" + uniqueId;
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
  // Verify initial state - no positions saved yet
  QCOMPARE(history.getWatchPosition("nonexistent"), qint64(0));
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

// ===== Tests saveWatchProgress =====

void TestWatchHistory::testSaveWatchProgress() {
  m_watchHistory->saveWatchProgress("video_progress", 500, 1000);
  
  qint64 position = m_watchHistory->getWatchPosition("video_progress");
  QCOMPARE(position, qint64(500));
}

void TestWatchHistory::testSaveWatchProgressEmptyVideoId() {
  // Ne doit pas crasher avec un ID vide
  m_watchHistory->saveWatchProgress(QString(), 100, 200);
  
  // Vérifier que rien n'a été sauvegardé
  QVariantList videos = m_watchHistory->getVideosInProgress();
  QVERIFY(videos.isEmpty());
}

void TestWatchHistory::testSaveWatchProgressCompletedAt90Percent() {
  // 90% = complété
  m_watchHistory->saveWatchProgress("video_completed", 900, 1000);
  
  WatchProgress progress = m_watchHistory->getWatchProgress("video_completed");
  QVERIFY(progress.completed);
}

void TestWatchHistory::testSaveWatchProgressNotCompletedAt89Percent() {
  // 89% = pas encore complété
  m_watchHistory->saveWatchProgress("video_almost", 890, 1000);
  
  WatchProgress progress = m_watchHistory->getWatchProgress("video_almost");
  QVERIFY(!progress.completed);
}

void TestWatchHistory::testSaveWatchProgressZeroDuration() {
  // Durée 0 = pas de calcul de completion
  m_watchHistory->saveWatchProgress("video_zero_dur", 500, 0);
  
  WatchProgress progress = m_watchHistory->getWatchProgress("video_zero_dur");
  QVERIFY(!progress.completed);
  QCOMPARE(progress.lastPosition, qint64(500));
}

// ===== Tests getWatchProgress =====

void TestWatchHistory::testGetWatchProgress() {
  m_watchHistory->saveWatchProgress("video_full", 300, 600);
  
  WatchProgress progress = m_watchHistory->getWatchProgress("video_full");
  
  QCOMPARE(progress.vodId, QStringLiteral("video_full"));
  QCOMPARE(progress.lastPosition, qint64(300));
  QCOMPARE(progress.totalDuration, qint64(600));
  QVERIFY(progress.lastWatchedAt.isValid());
}

void TestWatchHistory::testGetWatchProgressEmptyVideoId() {
  WatchProgress progress = m_watchHistory->getWatchProgress(QString());
  
  QVERIFY(progress.vodId.isEmpty());
  QCOMPARE(progress.lastPosition, qint64(0));
  QCOMPARE(progress.totalDuration, qint64(0));
  QVERIFY(!progress.completed);
}

void TestWatchHistory::testGetWatchProgressNonExistent() {
  WatchProgress progress = m_watchHistory->getWatchProgress("nonexistent_video");
  
  QCOMPARE(progress.vodId, QStringLiteral("nonexistent_video"));
  QCOMPARE(progress.lastPosition, qint64(0));
  QCOMPARE(progress.totalDuration, qint64(0));
}

void TestWatchHistory::testGetWatchProgressAfterSaveProgress() {
  // Sauvegarder avec différentes positions pour vérifier l'écrasement
  m_watchHistory->saveWatchProgress("video_update", 100, 500);
  m_watchHistory->saveWatchProgress("video_update", 400, 500);
  
  WatchProgress progress = m_watchHistory->getWatchProgress("video_update");
  
  QCOMPARE(progress.lastPosition, qint64(400));
  QCOMPARE(progress.totalDuration, qint64(500));
}

// ===== Tests markAsCompleted =====

void TestWatchHistory::testMarkAsCompleted() {
  m_watchHistory->saveWatchPosition("video_to_complete", 100);
  m_watchHistory->markAsCompleted("video_to_complete");
  
  WatchProgress progress = m_watchHistory->getWatchProgress("video_to_complete");
  QVERIFY(progress.completed);
}

void TestWatchHistory::testMarkAsCompletedEmptyVideoId() {
  // Ne doit pas crasher avec un ID vide
  m_watchHistory->markAsCompleted(QString());
  
  // Aucune vidéo ne doit être marquée complétée
  QVariantList videos = m_watchHistory->getVideosInProgress();
  QVERIFY(videos.isEmpty());
}

void TestWatchHistory::testMarkAsCompletedVerifyState() {
  // Sauvegarder d'abord sans complétion
  m_watchHistory->saveWatchProgress("video_mark", 200, 1000);
  
  WatchProgress before = m_watchHistory->getWatchProgress("video_mark");
  QVERIFY(!before.completed);
  
  // Marquer comme complété
  m_watchHistory->markAsCompleted("video_mark");
  
  WatchProgress after = m_watchHistory->getWatchProgress("video_mark");
  QVERIFY(after.completed);
}

// ===== Tests hasProgress =====

void TestWatchHistory::testHasProgress() {
  m_watchHistory->saveWatchPosition("video_with_progress", 500);
  
  QVERIFY(m_watchHistory->hasProgress("video_with_progress"));
}

void TestWatchHistory::testHasProgressFalseForNonExistent() {
  QVERIFY(!m_watchHistory->hasProgress("video_nonexistent"));
}

void TestWatchHistory::testHasProgressEmptyVideoId() {
  QVERIFY(!m_watchHistory->hasProgress(QString()));
}

void TestWatchHistory::testHasProgressFalseForZeroPosition() {
  m_watchHistory->saveWatchPosition("video_zero_pos", 0);
  
  // Position 0 = pas de progression
  QVERIFY(!m_watchHistory->hasProgress("video_zero_pos"));
}

// ===== Tests formatLastWatched =====

void TestWatchHistory::testFormatLastWatchedJustNow() {
  QDateTime now = QDateTime::currentDateTime();
  QString result = WatchHistory::formatLastWatched(now);
  
  QVERIFY(result.contains("instant") || result.contains("Vu"));
}

void TestWatchHistory::testFormatLastWatchedMinutes() {
  QDateTime fiveMinutesAgo = QDateTime::currentDateTime().addSecs(-5 * 60);
  QString result = WatchHistory::formatLastWatched(fiveMinutesAgo);
  
  QVERIFY(result.contains("5") && result.contains("minute"));
}

void TestWatchHistory::testFormatLastWatchedOneMinute() {
  QDateTime oneMinuteAgo = QDateTime::currentDateTime().addSecs(-90); // 1.5 minutes -> 1 minute
  QString result = WatchHistory::formatLastWatched(oneMinuteAgo);
  
  QVERIFY(result.contains("minute"));
}

void TestWatchHistory::testFormatLastWatchedHours() {
  QDateTime threeHoursAgo = QDateTime::currentDateTime().addSecs(-3 * 3600);
  QString result = WatchHistory::formatLastWatched(threeHoursAgo);
  
  QVERIFY(result.contains("3") && result.contains("heure"));
}

void TestWatchHistory::testFormatLastWatchedOneHour() {
  QDateTime oneHourAgo = QDateTime::currentDateTime().addSecs(-3600);
  QString result = WatchHistory::formatLastWatched(oneHourAgo);
  
  QVERIFY(result.contains("1") && result.contains("heure"));
}

void TestWatchHistory::testFormatLastWatchedYesterday() {
  QDateTime yesterday = QDateTime::currentDateTime().addSecs(-86400);
  QString result = WatchHistory::formatLastWatched(yesterday);
  
  QVERIFY(result.contains("hier"));
}

void TestWatchHistory::testFormatLastWatchedDays() {
  QDateTime threeDaysAgo = QDateTime::currentDateTime().addSecs(-3 * 86400);
  QString result = WatchHistory::formatLastWatched(threeDaysAgo);
  
  QVERIFY(result.contains("3") && result.contains("jour"));
}

void TestWatchHistory::testFormatLastWatchedWeeks() {
  QDateTime twoWeeksAgo = QDateTime::currentDateTime().addSecs(-14 * 86400);
  QString result = WatchHistory::formatLastWatched(twoWeeksAgo);
  
  QVERIFY(result.contains("2") && result.contains("semaine"));
}

void TestWatchHistory::testFormatLastWatchedOneWeek() {
  QDateTime oneWeekAgo = QDateTime::currentDateTime().addSecs(-7 * 86400);
  QString result = WatchHistory::formatLastWatched(oneWeekAgo);
  
  QVERIFY(result.contains("1") && result.contains("semaine"));
}

void TestWatchHistory::testFormatLastWatchedMonths() {
  QDateTime twoMonthsAgo = QDateTime::currentDateTime().addSecs(-60 * 86400);
  QString result = WatchHistory::formatLastWatched(twoMonthsAgo);
  
  QVERIFY(result.contains("mois"));
}

void TestWatchHistory::testFormatLastWatchedOneMonth() {
  QDateTime oneMonthAgo = QDateTime::currentDateTime().addSecs(-30 * 86400);
  QString result = WatchHistory::formatLastWatched(oneMonthAgo);
  
  QVERIFY(result.contains("1") && result.contains("mois"));
}

void TestWatchHistory::testFormatLastWatchedInvalidDate() {
  QDateTime invalid;
  QString result = WatchHistory::formatLastWatched(invalid);
  
  QVERIFY(result.isEmpty());
}

void TestWatchHistory::testFormatLastWatchedFutureDate() {
  QDateTime future = QDateTime::currentDateTime().addDays(1);
  QString result = WatchHistory::formatLastWatched(future);
  
  // Date future = retourne vide
  QVERIFY(result.isEmpty());
}

// ===== Tests du signal progressUpdated =====

void TestWatchHistory::testProgressUpdatedSignalOnSavePosition() {
  QSignalSpy spy(m_watchHistory, &WatchHistory::progressUpdated);
  QVERIFY(spy.isValid());
  
  m_watchHistory->saveWatchPosition("video_signal_1", 100);
  
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("video_signal_1"));
}

void TestWatchHistory::testProgressUpdatedSignalOnSaveProgress() {
  QSignalSpy spy(m_watchHistory, &WatchHistory::progressUpdated);
  QVERIFY(spy.isValid());
  
  m_watchHistory->saveWatchProgress("video_signal_2", 200, 500);
  
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("video_signal_2"));
}

void TestWatchHistory::testProgressUpdatedSignalOnMarkAsCompleted() {
  QSignalSpy spy(m_watchHistory, &WatchHistory::progressUpdated);
  QVERIFY(spy.isValid());
  
  m_watchHistory->saveWatchPosition("video_signal_3", 50);
  spy.clear();  // Effacer le signal du saveWatchPosition
  
  m_watchHistory->markAsCompleted("video_signal_3");
  
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("video_signal_3"));
}

void TestWatchHistory::testNoSignalOnEmptyVideoId() {
  QSignalSpy spy(m_watchHistory, &WatchHistory::progressUpdated);
  QVERIFY(spy.isValid());
  
  m_watchHistory->saveWatchPosition(QString(), 100);
  m_watchHistory->saveWatchProgress(QString(), 100, 200);
  m_watchHistory->markAsCompleted(QString());
  
  QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(TestWatchHistory)
#include "TestWatchHistory.moc"
