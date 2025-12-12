#include <QtTest/QtTest>
#include <QSignalSpy>

#include "core/StateMachineLiveReplay.hpp"

using namespace blueplayer::core;

class TestStateMachineLiveReplay : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Tests des états initiaux
  void testInitialState();
  void testInitialProperties();

  // Tests des transitions depuis Live
  void testSeekBackFromLive();
  void testSeekToTimeFromLive();

  // Tests des transitions en mode Replay
  void testPlayPauseInReplay();
  void testSeekInReplay();
  void testJumpToLive();

  // Tests de la fenêtre de seekback dynamique
  void testWindowSeekbackEqualsCache();
  void testCacheDurationUpdate();

  // Tests des signaux
  void testStateChangedSignal();
  void testSeekRequestedSignal();
  void testReplayModeRequestedSignal();

  // Tests des mises à jour depuis le pipeline
  void testUpdateCurrentTime();
  void testUpdateLiveEdgeTime();
  void testUpdateBufferSeconds();

  // Tests de latence
  void testLatencyCalculation();

  // Tests du worker thread
  void testWorkerThreadEnableDisable();

  // Tests de reset
  void testReset();

  // Tests des noms d'états et événements
  void testStateNames();
  void testEventNames();

  // Tests isInReplay
  void testIsInReplayStates();

 private:
  StateMachineLiveReplay* m_stateMachine = nullptr;
};

void TestStateMachineLiveReplay::initTestCase() {
  // Configuration globale des tests
}

void TestStateMachineLiveReplay::cleanupTestCase() {
  // Nettoyage global
}

void TestStateMachineLiveReplay::init() {
  m_stateMachine = new StateMachineLiveReplay();
}

void TestStateMachineLiveReplay::cleanup() {
  delete m_stateMachine;
  m_stateMachine = nullptr;
}

void TestStateMachineLiveReplay::testInitialState() {
  QCOMPARE(m_stateMachine->state(), LiveReplayState::Live);
  QCOMPARE(m_stateMachine->stateInt(), static_cast<int>(LiveReplayState::Live));
}

void TestStateMachineLiveReplay::testInitialProperties() {
  QCOMPARE(m_stateMachine->currentTime(), 0.0);
  QCOMPARE(m_stateMachine->liveEdgeTime(), 0.0);
  QCOMPARE(m_stateMachine->cacheDuration(), 0.0);
  QCOMPARE(m_stateMachine->windowSeekback(), 0.0);
  QCOMPARE(m_stateMachine->bufferSeconds(), 0.0);
  QVERIFY(!m_stateMachine->isInReplay());
}

void TestStateMachineLiveReplay::testSeekBackFromLive() {
  // Configurer le cache et le live edge
  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);

  QSignalSpy seekSpy(m_stateMachine, &StateMachineLiveReplay::seekRequested);
  QSignalSpy stateSpy(m_stateMachine, &StateMachineLiveReplay::stateChanged);

  // Déclencher un seekback
  m_stateMachine->seekBack();

  // Attendre le traitement des événements
  QTest::qWait(50);

  QVERIFY(seekSpy.count() >= 1);
  // Vérifier que le temps cible est correct (liveEdge - cache = 100 - 60 = 40)
  if (seekSpy.count() > 0) {
    double targetTime = seekSpy.first().at(0).toDouble();
    QCOMPARE(targetTime, 40.0);
  }

  QVERIFY(m_stateMachine->isInReplay());
}

void TestStateMachineLiveReplay::testSeekToTimeFromLive() {
  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);

  QSignalSpy seekSpy(m_stateMachine, &StateMachineLiveReplay::seekRequested);

  // Seek vers un temps spécifique
  m_stateMachine->seekToTime(70.0);

  QTest::qWait(50);

  QVERIFY(seekSpy.count() >= 1);
  if (seekSpy.count() > 0) {
    double targetTime = seekSpy.first().at(0).toDouble();
    QCOMPARE(targetTime, 70.0);
  }
}

void TestStateMachineLiveReplay::testPlayPauseInReplay() {
  // Entrer en mode replay
  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);
  m_stateMachine->seekToTime(50.0);
  QTest::qWait(50);

  // Simuler la fin du seek
  m_stateMachine->notifySeekCompleted();
  QTest::qWait(50);

  QCOMPARE(m_stateMachine->state(), LiveReplayState::ReplayPlaying);

  // Pause
  QSignalSpy pauseSpy(m_stateMachine, &StateMachineLiveReplay::pauseRequested);
  m_stateMachine->pauseFromReplay();
  QTest::qWait(50);

  QCOMPARE(m_stateMachine->state(), LiveReplayState::ReplayPaused);
  QVERIFY(pauseSpy.count() >= 1);

  // Play
  QSignalSpy playSpy(m_stateMachine, &StateMachineLiveReplay::playRequested);
  m_stateMachine->playFromReplay();
  QTest::qWait(50);

  QCOMPARE(m_stateMachine->state(), LiveReplayState::ReplayPlaying);
  QVERIFY(playSpy.count() >= 1);
}

void TestStateMachineLiveReplay::testSeekInReplay() {
  // Entrer en mode replay
  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);
  m_stateMachine->seekToTime(50.0);
  QTest::qWait(50);
  m_stateMachine->notifySeekCompleted();
  QTest::qWait(50);

  QSignalSpy seekSpy(m_stateMachine, &StateMachineLiveReplay::seekRequested);

  // Faire un autre seek en mode replay
  m_stateMachine->seekToTime(60.0);
  QTest::qWait(50);

  QVERIFY(seekSpy.count() >= 1);
}

void TestStateMachineLiveReplay::testJumpToLive() {
  // Entrer en mode replay
  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);
  m_stateMachine->updateCurrentTime(50.0);
  m_stateMachine->seekToTime(50.0);
  QTest::qWait(50);
  m_stateMachine->notifySeekCompleted();
  QTest::qWait(50);

  QVERIFY(m_stateMachine->isInReplay());

  QSignalSpy liveSpy(m_stateMachine, &StateMachineLiveReplay::liveModeRequested);

  // Revenir au live
  m_stateMachine->jumpToLive();
  QTest::qWait(50);

  // Vérifier la transition vers CatchUpToLive (car latence > 2s)
  QVERIFY(m_stateMachine->state() == LiveReplayState::CatchUpToLive ||
          m_stateMachine->state() == LiveReplayState::Live);
}

void TestStateMachineLiveReplay::testWindowSeekbackEqualsCache() {
  m_stateMachine->updateCacheDuration(120.0);
  QTest::qWait(50);

  QCOMPARE(m_stateMachine->windowSeekback(), 120.0);
  QCOMPARE(m_stateMachine->windowSeekback(), m_stateMachine->cacheDuration());
}

void TestStateMachineLiveReplay::testCacheDurationUpdate() {
  QSignalSpy cacheSpy(m_stateMachine,
                      &StateMachineLiveReplay::cacheDurationChanged);
  QSignalSpy windowSpy(m_stateMachine,
                       &StateMachineLiveReplay::windowSeekbackChanged);

  m_stateMachine->updateCacheDuration(60.0);
  QTest::qWait(50);

  QVERIFY(cacheSpy.count() >= 1);
  QVERIFY(windowSpy.count() >= 1);
  QCOMPARE(m_stateMachine->cacheDuration(), 60.0);
}

void TestStateMachineLiveReplay::testStateChangedSignal() {
  QSignalSpy stateSpy(m_stateMachine, &StateMachineLiveReplay::stateChanged);

  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);
  m_stateMachine->seekToTime(50.0);
  QTest::qWait(50);

  QVERIFY(stateSpy.count() >= 1);
}

void TestStateMachineLiveReplay::testSeekRequestedSignal() {
  QSignalSpy seekSpy(m_stateMachine, &StateMachineLiveReplay::seekRequested);

  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);
  m_stateMachine->seekToTime(75.0);
  QTest::qWait(50);

  QVERIFY(seekSpy.count() >= 1);
  double requestedTime = seekSpy.first().at(0).toDouble();
  QCOMPARE(requestedTime, 75.0);
}

void TestStateMachineLiveReplay::testReplayModeRequestedSignal() {
  QSignalSpy replaySpy(m_stateMachine,
                       &StateMachineLiveReplay::replayModeRequested);

  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);
  m_stateMachine->seekToTime(50.0);
  QTest::qWait(50);

  QVERIFY(replaySpy.count() >= 1);
}

void TestStateMachineLiveReplay::testUpdateCurrentTime() {
  QSignalSpy timeSpy(m_stateMachine,
                     &StateMachineLiveReplay::currentTimeChanged);

  m_stateMachine->updateCurrentTime(50.0);

  QCOMPARE(m_stateMachine->currentTime(), 50.0);
  QVERIFY(timeSpy.count() >= 1);
}

void TestStateMachineLiveReplay::testUpdateLiveEdgeTime() {
  QSignalSpy timeSpy(m_stateMachine,
                     &StateMachineLiveReplay::liveEdgeTimeChanged);

  m_stateMachine->updateLiveEdgeTime(100.0);
  QTest::qWait(50);

  QCOMPARE(m_stateMachine->liveEdgeTime(), 100.0);
  QVERIFY(timeSpy.count() >= 1);
}

void TestStateMachineLiveReplay::testUpdateBufferSeconds() {
  QSignalSpy bufferSpy(m_stateMachine,
                       &StateMachineLiveReplay::bufferSecondsChanged);

  m_stateMachine->updateBufferSeconds(5.0);

  QCOMPARE(m_stateMachine->bufferSeconds(), 5.0);
  QVERIFY(bufferSpy.count() >= 1);
}

void TestStateMachineLiveReplay::testLatencyCalculation() {
  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCurrentTime(95.0);

  QCOMPARE(m_stateMachine->latency(), 5.0);
}

void TestStateMachineLiveReplay::testWorkerThreadEnableDisable() {
  QVERIFY(!m_stateMachine->isWorkerThreadEnabled());

  m_stateMachine->enableWorkerThread();
  QTest::qWait(100);

  QVERIFY(m_stateMachine->isWorkerThreadEnabled());

  m_stateMachine->disableWorkerThread();
  QTest::qWait(100);

  QVERIFY(!m_stateMachine->isWorkerThreadEnabled());
}

void TestStateMachineLiveReplay::testReset() {
  // Configurer un état non-initial
  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);
  m_stateMachine->updateCurrentTime(50.0);
  m_stateMachine->seekToTime(50.0);
  QTest::qWait(50);

  // Reset
  m_stateMachine->reset();

  QCOMPARE(m_stateMachine->state(), LiveReplayState::Live);
  QCOMPARE(m_stateMachine->currentTime(), 0.0);
  QCOMPARE(m_stateMachine->liveEdgeTime(), 0.0);
  QCOMPARE(m_stateMachine->cacheDuration(), 0.0);
  QVERIFY(!m_stateMachine->isInReplay());
}

void TestStateMachineLiveReplay::testStateNames() {
  QCOMPARE(StateMachineLiveReplay::stateName(LiveReplayState::Live),
           QString("Live"));
  QCOMPARE(StateMachineLiveReplay::stateName(LiveReplayState::SeekbackPrep),
           QString("SeekbackPrep"));
  QCOMPARE(StateMachineLiveReplay::stateName(LiveReplayState::SeekbackActive),
           QString("SeekbackActive"));
  QCOMPARE(StateMachineLiveReplay::stateName(LiveReplayState::SeekbackSeeking),
           QString("SeekbackSeeking"));
  QCOMPARE(StateMachineLiveReplay::stateName(LiveReplayState::ReplayPlaying),
           QString("ReplayPlaying"));
  QCOMPARE(StateMachineLiveReplay::stateName(LiveReplayState::ReplayPaused),
           QString("ReplayPaused"));
  QCOMPARE(StateMachineLiveReplay::stateName(LiveReplayState::CatchUpToLive),
           QString("CatchUpToLive"));
  QCOMPARE(StateMachineLiveReplay::stateName(LiveReplayState::Error),
           QString("Error"));
}

void TestStateMachineLiveReplay::testEventNames() {
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::SeekBackRequest),
           QString("SeekBackRequest"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::SeekToTime),
           QString("SeekToTime"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::PlayFromReplay),
           QString("PlayFromReplay"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::PauseFromReplay),
           QString("PauseFromReplay"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::JumpToLive),
           QString("JumpToLive"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::LiveEdgeAdvanced),
           QString("LiveEdgeAdvanced"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::BufferUnderrun),
           QString("BufferUnderrun"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::BufferRefill),
           QString("BufferRefill"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::EndOfReplay),
           QString("EndOfReplay"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::ErrorOccurred),
           QString("ErrorOccurred"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::SeekCompleted),
           QString("SeekCompleted"));
  QCOMPARE(StateMachineLiveReplay::eventName(LiveReplayEvent::CacheUpdated),
           QString("CacheUpdated"));
}

void TestStateMachineLiveReplay::testIsInReplayStates() {
  // Test que isInReplay retourne true pour les états replay
  m_stateMachine->updateLiveEdgeTime(100.0);
  m_stateMachine->updateCacheDuration(60.0);

  // Live -> SeekbackSeeking
  m_stateMachine->seekToTime(50.0);
  QTest::qWait(50);
  QVERIFY(m_stateMachine->isInReplay());

  // SeekbackSeeking -> ReplayPlaying
  m_stateMachine->notifySeekCompleted();
  QTest::qWait(50);
  QCOMPARE(m_stateMachine->state(), LiveReplayState::ReplayPlaying);
  QVERIFY(m_stateMachine->isInReplay());

  // ReplayPlaying -> ReplayPaused
  m_stateMachine->pauseFromReplay();
  QTest::qWait(50);
  QCOMPARE(m_stateMachine->state(), LiveReplayState::ReplayPaused);
  QVERIFY(m_stateMachine->isInReplay());
}

QTEST_MAIN(TestStateMachineLiveReplay)
#include "TestStateMachineLiveReplay.moc"



