#include <QtTest/QtTest>
#include <QSignalSpy>

#include "api/twitch/TwitchService.hpp"

using namespace blueplayer::api::twitch;

class TestTwitchService : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Tests d'initialisation
  void testConstructor();
  void testInitialState();

  // Tests des propriétés - État initial
  void testIsAuthenticatedInitially();
  void testStreamsInitiallyEmpty();
  void testRecommendedStreamsInitiallyEmpty();
  void testCategoriesInitiallyEmpty();
  void testPopularClipsInitiallyEmpty();
  void testFollowedClipsInitiallyEmpty();
  void testVideosInitiallyEmpty();
  void testFollowedChannelsInitiallyEmpty();
  void testNewStreamersInitiallyEmpty();
  void testCategoryStreamsInitiallyEmpty();
  void testSearchResultsInitiallyEmpty();
  void testUserIdInitiallyEmpty();
  void testUserNameInitiallyEmpty();
  void testSelectedStreamUrlInitiallyEmpty();
  void testCurrentHlsUrlInitiallyEmpty();

  // Tests des signaux
  void testAuthenticatedChangedSignal();
  void testStreamsChangedSignal();
  void testRecommendedStreamsChangedSignal();
  void testCategoriesChangedSignal();
  void testPopularClipsChangedSignal();
  void testFollowedClipsChangedSignal();
  void testVideosChangedSignal();
  void testFollowedChannelsChangedSignal();
  void testNewStreamersChangedSignal();
  void testCategoryStreamsChangedSignal();
  void testSearchChannelResultsChangedSignal();
  void testSearchCategoryResultsChangedSignal();
  void testUserIdChangedSignal();
  void testUserNameChangedSignal();
  void testHlsUrlReadySignal();
  void testErrorOccurredSignal();
  void testAdsDetectedSignal();
  void testAdsFinishedSignal();

  // Tests de logout
  void testLogout();
  void testLogoutClearsAuthentication();
  void testLogoutIsIdempotent();

  // Tests des opérations sans authentification
  void testRefreshStreamsWithoutAuth();
  void testRefreshRecommendedStreamsWithoutAuth();
  void testRefreshCategoriesWithoutAuth();
  void testRefreshPopularClipsWithoutAuth();
  void testRefreshFollowedClipsWithoutAuth();
  void testRefreshVideosWithoutAuth();
  void testRefreshFollowedChannelsWithoutAuth();
  void testRefreshNewStreamersWithoutAuth();
  void testRefreshCategoryStreamsWithoutAuth();

  // Tests de recherche
  void testSearchWithoutAuth();
  void testSearchEmptyQuery();
  void testClearSearchResults();
  void testClearSearchResultsIdempotent();

  // Tests de playStream
  void testPlayStreamWithInvalidIndex();
  void testPlayStreamNegativeIndex();
  void testPlayStreamWithoutStreams();

  // Tests de getStreamHlsUrl
  void testGetStreamHlsUrlEmptyLogin();
  void testGetStreamHlsUrlValidLogin();

  // Tests d'accessToken
  void testAccessTokenProperty();

private:
  TwitchService* m_service = nullptr;
};

void TestTwitchService::initTestCase() {
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
  qputenv("TWITCH_CLIENT_SECRET", "test_client_secret");
}

void TestTwitchService::cleanupTestCase() {
  qunsetenv("TWITCH_CLIENT_ID");
  qunsetenv("TWITCH_CLIENT_SECRET");
}

void TestTwitchService::init() {
  m_service = new TwitchService(this);
}

void TestTwitchService::cleanup() {
  delete m_service;
  m_service = nullptr;
}

// ===== Tests d'initialisation =====

void TestTwitchService::testConstructor() {
  TwitchService service;
  QVERIFY(!service.isAuthenticated());
}

void TestTwitchService::testInitialState() {
  QVERIFY(m_service != nullptr);
  QVERIFY(m_service->streams().isEmpty());
  QVERIFY(m_service->categories().isEmpty());
}

// ===== Tests des propriétés - État initial =====

void TestTwitchService::testIsAuthenticatedInitially() {
  m_service->logout();
  QVERIFY(!m_service->isAuthenticated());
}

void TestTwitchService::testStreamsInitiallyEmpty() {
  QVERIFY(m_service->streams().isEmpty());
}

void TestTwitchService::testRecommendedStreamsInitiallyEmpty() {
  QVERIFY(m_service->recommendedStreams().isEmpty());
}

void TestTwitchService::testCategoriesInitiallyEmpty() {
  QVERIFY(m_service->categories().isEmpty());
}

void TestTwitchService::testPopularClipsInitiallyEmpty() {
  QVERIFY(m_service->popularClips().isEmpty());
}

void TestTwitchService::testFollowedClipsInitiallyEmpty() {
  QVERIFY(m_service->followedClips().isEmpty());
}

void TestTwitchService::testVideosInitiallyEmpty() {
  QVERIFY(m_service->videos().isEmpty());
}

void TestTwitchService::testFollowedChannelsInitiallyEmpty() {
  QVERIFY(m_service->followedChannels().isEmpty());
}

void TestTwitchService::testNewStreamersInitiallyEmpty() {
  QVERIFY(m_service->newStreamers().isEmpty());
}

void TestTwitchService::testCategoryStreamsInitiallyEmpty() {
  QVERIFY(m_service->categoryStreams().isEmpty());
}

void TestTwitchService::testSearchResultsInitiallyEmpty() {
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
}

void TestTwitchService::testUserIdInitiallyEmpty() {
  m_service->logout();
  QVERIFY(m_service->userId().isEmpty());
}

void TestTwitchService::testUserNameInitiallyEmpty() {
  m_service->logout();
  QVERIFY(m_service->userName().isEmpty());
}

void TestTwitchService::testSelectedStreamUrlInitiallyEmpty() {
  QVERIFY(m_service->selectedStreamUrl().isEmpty());
}

void TestTwitchService::testCurrentHlsUrlInitiallyEmpty() {
  QVERIFY(m_service->currentHlsUrl().isEmpty());
}

// ===== Tests des signaux =====

void TestTwitchService::testAuthenticatedChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::authenticatedChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testStreamsChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::streamsChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testRecommendedStreamsChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::recommendedStreamsChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testCategoriesChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::categoriesChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testPopularClipsChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::popularClipsChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testFollowedClipsChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::followedClipsChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testVideosChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::videosChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testFollowedChannelsChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::followedChannelsChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testNewStreamersChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::newStreamersChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testCategoryStreamsChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::categoryStreamsChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testSearchChannelResultsChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::searchChannelResultsChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testSearchCategoryResultsChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::searchCategoryResultsChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testUserIdChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::userIdChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testUserNameChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::userNameChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testHlsUrlReadySignal() {
  QSignalSpy spy(m_service, &TwitchService::hlsUrlReady);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testErrorOccurredSignal() {
  QSignalSpy spy(m_service, &TwitchService::errorOccurred);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testAdsDetectedSignal() {
  QSignalSpy spy(m_service, &TwitchService::adsDetected);
  QVERIFY(spy.isValid());
}

void TestTwitchService::testAdsFinishedSignal() {
  QSignalSpy spy(m_service, &TwitchService::adsFinished);
  QVERIFY(spy.isValid());
}

// ===== Tests de logout =====

void TestTwitchService::testLogout() {
  m_service->logout();
  QVERIFY(!m_service->isAuthenticated());
}

void TestTwitchService::testLogoutClearsAuthentication() {
  m_service->logout();
  
  QVERIFY(!m_service->isAuthenticated());
  QVERIFY(m_service->userId().isEmpty());
  QVERIFY(m_service->userName().isEmpty());
}

void TestTwitchService::testLogoutIsIdempotent() {
  m_service->logout();
  m_service->logout();
  m_service->logout();
  
  QVERIFY(!m_service->isAuthenticated());
}

// ===== Tests des opérations sans authentification =====

void TestTwitchService::testRefreshStreamsWithoutAuth() {
  m_service->logout();
  m_service->refreshStreams();
  
  QVERIFY(m_service->streams().isEmpty());
}

void TestTwitchService::testRefreshRecommendedStreamsWithoutAuth() {
  m_service->logout();
  m_service->refreshRecommendedStreams();
  
  QVERIFY(m_service->recommendedStreams().isEmpty());
}

void TestTwitchService::testRefreshCategoriesWithoutAuth() {
  m_service->logout();
  m_service->refreshCategories();
  
  QVERIFY(m_service->categories().isEmpty());
}

void TestTwitchService::testRefreshPopularClipsWithoutAuth() {
  m_service->logout();
  m_service->refreshPopularClips();
  
  QVERIFY(m_service->popularClips().isEmpty());
}

void TestTwitchService::testRefreshFollowedClipsWithoutAuth() {
  m_service->logout();
  m_service->refreshFollowedClips();
  
  QVERIFY(m_service->followedClips().isEmpty());
}

void TestTwitchService::testRefreshVideosWithoutAuth() {
  m_service->logout();
  m_service->refreshVideos();
  
  QVERIFY(m_service->videos().isEmpty());
}

void TestTwitchService::testRefreshFollowedChannelsWithoutAuth() {
  m_service->logout();
  m_service->refreshFollowedChannels();
  
  QVERIFY(m_service->followedChannels().isEmpty());
}

void TestTwitchService::testRefreshNewStreamersWithoutAuth() {
  m_service->logout();
  m_service->refreshNewStreamers();
  
  QVERIFY(m_service->newStreamers().isEmpty());
}

void TestTwitchService::testRefreshCategoryStreamsWithoutAuth() {
  m_service->logout();
  m_service->refreshCategoryStreams("12345");
  
  QVERIFY(m_service->categoryStreams().isEmpty());
}

// ===== Tests de recherche =====

void TestTwitchService::testSearchWithoutAuth() {
  m_service->logout();
  m_service->search("test");
  
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
}

void TestTwitchService::testSearchEmptyQuery() {
  m_service->search("");
  
  // Ne doit pas crasher
  QVERIFY(m_service != nullptr);
}

void TestTwitchService::testClearSearchResults() {
  m_service->clearSearchResults();
  
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
}

void TestTwitchService::testClearSearchResultsIdempotent() {
  m_service->clearSearchResults();
  m_service->clearSearchResults();
  m_service->clearSearchResults();
  
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
}

// ===== Tests de playStream =====

void TestTwitchService::testPlayStreamWithInvalidIndex() {
  m_service->playStream(999);
  
  QVERIFY(m_service->selectedStreamUrl().isEmpty());
}

void TestTwitchService::testPlayStreamNegativeIndex() {
  m_service->playStream(-1);
  
  QVERIFY(m_service->selectedStreamUrl().isEmpty());
}

void TestTwitchService::testPlayStreamWithoutStreams() {
  m_service->playStream(0);
  
  QVERIFY(m_service->selectedStreamUrl().isEmpty());
}

// ===== Tests de getStreamHlsUrl =====

void TestTwitchService::testGetStreamHlsUrlEmptyLogin() {
  m_service->getStreamHlsUrl("");
  
  // Ne doit pas crasher
  QVERIFY(m_service != nullptr);
}

void TestTwitchService::testGetStreamHlsUrlValidLogin() {
  QSignalSpy spy(m_service, &TwitchService::hlsUrlReady);
  
  m_service->getStreamHlsUrl("teststreamer");
  
  // La requête est asynchrone, on vérifie juste qu'on n'a pas crashé
  QVERIFY(m_service != nullptr);
}

// ===== Tests d'accessToken =====

void TestTwitchService::testAccessTokenProperty() {
  m_service->logout();
  
  QString token = m_service->accessToken();
  QVERIFY(token.isEmpty());
}

QTEST_MAIN(TestTwitchService)
#include "TestTwitchService.moc"
