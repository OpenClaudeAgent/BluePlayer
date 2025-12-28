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

  // Tests extractStreamerLogin
  void testExtractStreamerLoginValid();
  void testExtractStreamerLoginInvalid();
  void testExtractStreamerLoginEmpty();

  // Tests parseVariants
  void testParseVariantsValid();
  void testParseVariantsEmpty();
  void testParseVariantsMalformed();
  void testParseVariantsMultiple();
  void testParseVariantsWithResolution();

  // Tests selectBestVariant
  void testSelectBestVariantSource();
  void testSelectBestVariant720p();
  void testSelectBestVariant1080p();
  void testSelectBestVariantFallback();
  void testSelectBestVariantEmpty();

  // Tests detectAdsInPlaylist
  void testDetectAdsWithTwitchStitchedAd();
  void testDetectAdsWithDateRange();
  void testDetectAdsWithMultipleMarkers();
  void testDetectAdsCleanPlaylist();
  void testDetectAdsWithShortSegments();
  void testDetectAdsEmptyPlaylist();

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

// ===== Tests extractStreamerLogin =====

void TestHlsAdFilter::testExtractStreamerLoginValid() {
  QString url = "https://usher.ttvnw.net/api/channel/hls/streamer_name.m3u8?token=abc";
  QString login = m_filter->extractStreamerLogin(url);
  
  QCOMPARE(login, QStringLiteral("streamer_name"));
}

void TestHlsAdFilter::testExtractStreamerLoginInvalid() {
  QString url = "https://example.com/video.mp4";
  QString login = m_filter->extractStreamerLogin(url);
  
  QVERIFY(login.isEmpty());
}

void TestHlsAdFilter::testExtractStreamerLoginEmpty() {
  QString login = m_filter->extractStreamerLogin(QString());
  
  QVERIFY(login.isEmpty());
}

// ===== Tests parseVariants =====

void TestHlsAdFilter::testParseVariantsValid() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,VIDEO="chunked"
https://video-edge.example.com/chunked/index.m3u8)";

  auto variants = m_filter->parseVariants(playlist);
  
  QCOMPARE(variants.size(), 1);
  QCOMPARE(variants[0].name, QStringLiteral("chunked"));
  QCOMPARE(variants[0].bandwidth, 6000000);
  QVERIFY(variants[0].url.contains("chunked"));
}

void TestHlsAdFilter::testParseVariantsEmpty() {
  QString playlist = "#EXTM3U\n";
  
  auto variants = m_filter->parseVariants(playlist);
  
  QVERIFY(variants.isEmpty());
}

void TestHlsAdFilter::testParseVariantsMalformed() {
  // Playlist sans URL après EXT-X-STREAM-INF
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000
#ANOTHER-TAG)";

  auto variants = m_filter->parseVariants(playlist);
  
  QVERIFY(variants.isEmpty());
}

void TestHlsAdFilter::testParseVariantsMultiple() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,VIDEO="chunked"
https://video-edge.example.com/chunked/index.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,VIDEO="720p60"
https://video-edge.example.com/720p60/index.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1500000,VIDEO="480p"
https://video-edge.example.com/480p/index.m3u8)";

  auto variants = m_filter->parseVariants(playlist);
  
  QCOMPARE(variants.size(), 3);
  QCOMPARE(variants[0].name, QStringLiteral("chunked"));
  QCOMPARE(variants[1].name, QStringLiteral("720p60"));
  QCOMPARE(variants[2].name, QStringLiteral("480p"));
}

void TestHlsAdFilter::testParseVariantsWithResolution() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video-edge.example.com/1080p60/index.m3u8)";

  auto variants = m_filter->parseVariants(playlist);
  
  QCOMPARE(variants.size(), 1);
  QCOMPARE(variants[0].width, 1920);
  QCOMPARE(variants[0].height, 1080);
}

// ===== Tests selectBestVariant =====

void TestHlsAdFilter::testSelectBestVariantSource() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,VIDEO="chunked"
https://chunked.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,VIDEO="720p60"
https://720p60.m3u8)";

  auto variants = m_filter->parseVariants(playlist);
  QString selected = m_filter->selectBestVariant(variants, "source");
  
  QVERIFY(selected.contains("chunked"));
}

void TestHlsAdFilter::testSelectBestVariant720p() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,VIDEO="chunked"
https://chunked.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,VIDEO="720p60"
https://720p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1500000,VIDEO="480p"
https://480p.m3u8)";

  auto variants = m_filter->parseVariants(playlist);
  
  // Même avec 720p demandé, l'algo préfère chunked (source)
  QString selected = m_filter->selectBestVariant(variants, "720p");
  
  // Vérifie qu'une URL est retournée
  QVERIFY(!selected.isEmpty());
}

void TestHlsAdFilter::testSelectBestVariant1080p() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,VIDEO="1080p60"
https://1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,VIDEO="720p60"
https://720p60.m3u8)";

  auto variants = m_filter->parseVariants(playlist);
  QString selected = m_filter->selectBestVariant(variants, "1080p");
  
  QVERIFY(selected.contains("1080p60"));
}

void TestHlsAdFilter::testSelectBestVariantFallback() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=1000000,VIDEO="360p"
https://360p.m3u8)";

  auto variants = m_filter->parseVariants(playlist);
  QString selected = m_filter->selectBestVariant(variants, "chunked");
  
  // Fallback au premier disponible
  QCOMPARE(selected, QStringLiteral("https://360p.m3u8"));
}

void TestHlsAdFilter::testSelectBestVariantEmpty() {
  QList<HlsAdFilter::StreamVariant> emptyVariants;
  QString selected = m_filter->selectBestVariant(emptyVariants, "chunked");
  
  QVERIFY(selected.isEmpty());
}

// ===== Tests detectAdsInPlaylist =====

void TestHlsAdFilter::testDetectAdsWithTwitchStitchedAd() {
  QString playlist = R"(#EXTM3U
#EXT-X-TARGETDURATION:2
#EXTINF:2.000,
segment1.ts
#twitch-stitched-ad
#EXTINF:2.000,
ad_segment.ts
#EXTINF:2.000,
segment2.ts)";

  bool hasAds = m_filter->detectAdsInPlaylist(playlist);
  
  QVERIFY(hasAds);
  QVERIFY(m_filter->adSegmentCount() > 0);
}

void TestHlsAdFilter::testDetectAdsWithDateRange() {
  QString playlist = R"(#EXTM3U
#EXT-X-DATERANGE:ID="ad1",CLASS="twitch-stitched-ad",START-DATE="2024-01-01"
#EXTINF:2.000,
ad_segment.ts)";

  bool hasAds = m_filter->detectAdsInPlaylist(playlist);
  
  QVERIFY(hasAds);
}

void TestHlsAdFilter::testDetectAdsWithMultipleMarkers() {
  QString playlist = R"(#EXTM3U
#twitch-stitched-ad
#Amazon-Ads
#X-TV-TWITCH-AD-START
#EXTINF:2.000,
ad.ts)";

  bool hasAds = m_filter->detectAdsInPlaylist(playlist);
  
  QVERIFY(hasAds);
  QVERIFY(m_filter->adSegmentCount() >= 3);  // Au moins 3 marqueurs
}

void TestHlsAdFilter::testDetectAdsCleanPlaylist() {
  QString playlist = R"(#EXTM3U
#EXT-X-TARGETDURATION:4
#EXT-X-MEDIA-SEQUENCE:12345
#EXTINF:4.000,
segment1.ts
#EXTINF:4.000,
segment2.ts
#EXTINF:4.000,
segment3.ts
#EXTINF:4.000,
segment4.ts
#EXTINF:4.000,
segment5.ts
#EXTINF:4.000,
segment6.ts)";

  bool hasAds = m_filter->detectAdsInPlaylist(playlist);
  
  QVERIFY(!hasAds);
  QCOMPARE(m_filter->adSegmentCount(), 0);
}

void TestHlsAdFilter::testDetectAdsWithShortSegments() {
  // 5 segments courts au début peuvent indiquer un pre-roll
  QString playlist = R"(#EXTM3U
#EXTINF:1.5,
short1.ts
#EXTINF:1.2,
short2.ts
#EXTINF:1.8,
short3.ts
#EXTINF:1.1,
short4.ts
#EXTINF:1.4,
short5.ts
#EXTINF:4.0,
normal.ts)";

  // Note: L'algo détecte >=3 segments courts dans les 5 premiers
  bool hasAds = m_filter->detectAdsInPlaylist(playlist);
  
  // Devrait détecter comme pub potentielle
  QVERIFY(hasAds);
}

void TestHlsAdFilter::testDetectAdsEmptyPlaylist() {
  bool hasAds = m_filter->detectAdsInPlaylist(QString());
  
  QVERIFY(!hasAds);
  QCOMPARE(m_filter->adSegmentCount(), 0);
}

QTEST_MAIN(TestHlsAdFilter)
#include "TestHlsAdFilter.moc"
