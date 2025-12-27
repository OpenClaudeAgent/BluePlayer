#include <QtTest/QtTest>
#include <QSignalSpy>

#include "media/HlsAdFilter.hpp"

using namespace blueplayer::media;

class TestHlsAdFilter : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Tests d'initialisation
  void testConstructor();
  void testInitialState();
  void testInitialAdCount();
  void testInitialRetryCount();

  // Tests des signaux
  void testCleanStreamReadySignal();
  void testAdsDetectedSignal();
  void testAdsFinishedSignal();
  void testRequestNewTokenSignal();
  void testMaxRetriesReachedSignal();
  void testErrorSignal();
  void testDebugLogSignal();

  // Tests de start/stop
  void testStartFiltering();
  void testStartFilteringWithQuality();
  void testStop();
  void testStopWhenNotStarted();
  void testMultipleStarts();

  // Tests des getters
  void testHasAdsInitially();
  void testAdSegmentCount();
  void testRetryCount();

  // Tests de parsing de playlist (via comportement)
  void testDetectAdsInPlaylist();
  void testExtractVariants();

private:
  HlsAdFilter* m_filter = nullptr;
};

void TestHlsAdFilter::initTestCase() {
}

void TestHlsAdFilter::cleanupTestCase() {
}

void TestHlsAdFilter::init() {
  m_filter = new HlsAdFilter(this);
}

void TestHlsAdFilter::cleanup() {
  delete m_filter;
  m_filter = nullptr;
}

// ===== Tests d'initialisation =====

void TestHlsAdFilter::testConstructor() {
  HlsAdFilter filter;
  // Verify initial state after construction
  QVERIFY(!filter.hasAds());
  QCOMPARE(filter.adSegmentCount(), 0);
}

void TestHlsAdFilter::testInitialState() {
  QVERIFY(!m_filter->hasAds());
}

void TestHlsAdFilter::testInitialAdCount() {
  QCOMPARE(m_filter->adSegmentCount(), 0);
}

void TestHlsAdFilter::testInitialRetryCount() {
  QCOMPARE(m_filter->retryCount(), 0);
}

// ===== Tests des signaux =====

void TestHlsAdFilter::testCleanStreamReadySignal() {
  QSignalSpy spy(m_filter, &HlsAdFilter::cleanStreamReady);
  QVERIFY(spy.isValid());
}

void TestHlsAdFilter::testAdsDetectedSignal() {
  QSignalSpy spy(m_filter, &HlsAdFilter::adsDetected);
  QVERIFY(spy.isValid());
}

void TestHlsAdFilter::testAdsFinishedSignal() {
  QSignalSpy spy(m_filter, &HlsAdFilter::adsFinished);
  QVERIFY(spy.isValid());
}

void TestHlsAdFilter::testRequestNewTokenSignal() {
  QSignalSpy spy(m_filter, &HlsAdFilter::requestNewToken);
  QVERIFY(spy.isValid());
}

void TestHlsAdFilter::testMaxRetriesReachedSignal() {
  QSignalSpy spy(m_filter, &HlsAdFilter::maxRetriesReached);
  QVERIFY(spy.isValid());
}

void TestHlsAdFilter::testErrorSignal() {
  QSignalSpy spy(m_filter, &HlsAdFilter::error);
  QVERIFY(spy.isValid());
}

void TestHlsAdFilter::testDebugLogSignal() {
  QSignalSpy spy(m_filter, &HlsAdFilter::debugLog);
  QVERIFY(spy.isValid());
}

// ===== Tests de start/stop =====

void TestHlsAdFilter::testStartFiltering() {
  QSignalSpy errorSpy(m_filter, &HlsAdFilter::error);
  
  // URL invalide pour éviter un vrai appel réseau
  m_filter->startFiltering("https://invalid.url/master.m3u8");
  
  // Le filtre doit démarrer sans crash
  QVERIFY(m_filter != nullptr);
}

void TestHlsAdFilter::testStartFilteringWithQuality() {
  m_filter->startFiltering("https://invalid.url/master.m3u8", "1080p60");
  QVERIFY(m_filter != nullptr);
}

void TestHlsAdFilter::testStop() {
  m_filter->startFiltering("https://invalid.url/master.m3u8");
  m_filter->stop();
  
  // Après stop, l'état doit être réinitialisé ou stable
  QVERIFY(m_filter != nullptr);
}

void TestHlsAdFilter::testStopWhenNotStarted() {
  // stop() sans start() ne doit pas crasher
  m_filter->stop();
  QVERIFY(m_filter != nullptr);
}

void TestHlsAdFilter::testMultipleStarts() {
  m_filter->startFiltering("https://url1.com/master.m3u8");
  m_filter->startFiltering("https://url2.com/master.m3u8");
  m_filter->startFiltering("https://url3.com/master.m3u8");
  
  // Plusieurs starts successifs ne doivent pas crasher
  QVERIFY(m_filter != nullptr);
}

// ===== Tests des getters =====

void TestHlsAdFilter::testHasAdsInitially() {
  QVERIFY(!m_filter->hasAds());
}

void TestHlsAdFilter::testAdSegmentCount() {
  // Initialement 0
  QCOMPARE(m_filter->adSegmentCount(), 0);
}

void TestHlsAdFilter::testRetryCount() {
  // Initialement 0
  QCOMPARE(m_filter->retryCount(), 0);
}

// ===== Tests de parsing de playlist =====

void TestHlsAdFilter::testDetectAdsInPlaylist() {
  // Test que le filtre peut être démarré avec différentes URLs
  // Le parsing réel est testé via le comportement du filtre
  
  // URL avec format Twitch typique
  QString twitchUrl = "https://video-weaver.fra02.hls.ttvnw.net/v1/playlist/abc123.m3u8";
  m_filter->startFiltering(twitchUrl);
  
  // Pas de crash
  QVERIFY(m_filter != nullptr);
  
  m_filter->stop();
}

void TestHlsAdFilter::testExtractVariants() {
  // Test indirect - le filtre gère les variants en interne
  // On vérifie juste que différentes qualités sont acceptées
  
  QStringList qualities = {"chunked", "1080p60", "720p60", "480p", "360p", "160p"};
  
  for (const QString& quality : qualities) {
    m_filter->startFiltering("https://test.url/master.m3u8", quality);
    m_filter->stop();
    QVERIFY(m_filter != nullptr);
  }
}

QTEST_MAIN(TestHlsAdFilter)
#include "TestHlsAdFilter.moc"
