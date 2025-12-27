#include <QtTest/QtTest>
#include <QSignalSpy>

#include "api/twitch/TwitchService.hpp"
#include "mocks/MockSecureStorage.hpp"

using namespace blueplayer::api::twitch;
using namespace blueplayer::test;

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

  // Data-driven test: properties initially empty (consolidates 14 tests)
  void testPropertyInitiallyEmpty_data();
  void testPropertyInitiallyEmpty();

  // Data-driven test: signal validity (consolidates 18 tests)
  void testSignalValidity_data();
  void testSignalValidity();

  // Tests de logout
  void testLogout();
  void testLogoutClearsAuthentication();
  void testLogoutIsIdempotent();

  // Data-driven test: refresh without auth (consolidates 9 tests)
  void testRefreshWithoutAuth_data();
  void testRefreshWithoutAuth();

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
  MockSecureStorage* m_mockStorage = nullptr;
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
  m_mockStorage = new MockSecureStorage();
  m_service = new TwitchService(m_mockStorage, this);
}

void TestTwitchService::cleanup() {
  delete m_service;
  m_service = nullptr;
  delete m_mockStorage;
  m_mockStorage = nullptr;
}

// ===== Tests d'initialisation =====

void TestTwitchService::testConstructor() {
  MockSecureStorage mockStorage;
  TwitchService service(&mockStorage);
  QVERIFY(!service.isAuthenticated());
}

void TestTwitchService::testInitialState() {
  QVERIFY(m_service != nullptr);
  QVERIFY(m_service->streams().isEmpty());
  QVERIFY(m_service->categories().isEmpty());
}

// ===== Data-driven test: properties initially empty =====

void TestTwitchService::testPropertyInitiallyEmpty_data() {
  QTest::addColumn<QString>("propertyName");
  QTest::addColumn<bool>("requiresLogout");

  // List properties
  QTest::newRow("streams") << "streams" << false;
  QTest::newRow("recommendedStreams") << "recommendedStreams" << false;
  QTest::newRow("categories") << "categories" << false;
  QTest::newRow("popularClips") << "popularClips" << false;
  QTest::newRow("followedClips") << "followedClips" << false;
  QTest::newRow("videos") << "videos" << false;
  QTest::newRow("followedChannels") << "followedChannels" << false;
  QTest::newRow("newStreamers") << "newStreamers" << false;
  QTest::newRow("categoryStreams") << "categoryStreams" << false;
  QTest::newRow("searchChannelResults") << "searchChannelResults" << false;
  QTest::newRow("searchCategoryResults") << "searchCategoryResults" << false;

  // String properties
  QTest::newRow("userId") << "userId" << true;
  QTest::newRow("userName") << "userName" << true;
  QTest::newRow("selectedStreamUrl") << "selectedStreamUrl" << false;
  // Bool property
  QTest::newRow("authenticated") << "authenticated" << true;
}

void TestTwitchService::testPropertyInitiallyEmpty() {
  QFETCH(QString, propertyName);
  QFETCH(bool, requiresLogout);

  if (requiresLogout) {
    m_service->logout();
  }

  QVariant value = m_service->property(propertyName.toLatin1().constData());
  QVERIFY2(value.isValid(), qPrintable("Property " + propertyName + " not found"));

  if (value.typeId() == QMetaType::Bool) {
    QVERIFY2(!value.toBool(), qPrintable(propertyName + " should be false"));
  } else if (value.typeId() == QMetaType::QString) {
    QVERIFY2(value.toString().isEmpty(), qPrintable(propertyName + " should be empty"));
  } else if (value.canConvert<QVariantList>()) {
    QVERIFY2(value.toList().isEmpty(), qPrintable(propertyName + " should be empty"));
  } else {
    QFAIL(qPrintable("Unknown property type for " + propertyName));
  }
}

// ===== Data-driven test: signal validity =====

void TestTwitchService::testSignalValidity_data() {
  QTest::addColumn<QString>("signalName");

  QTest::newRow("authenticatedChanged") << "authenticatedChanged";
  QTest::newRow("streamsChanged") << "streamsChanged";
  QTest::newRow("recommendedStreamsChanged") << "recommendedStreamsChanged";
  QTest::newRow("categoriesChanged") << "categoriesChanged";
  QTest::newRow("popularClipsChanged") << "popularClipsChanged";
  QTest::newRow("followedClipsChanged") << "followedClipsChanged";
  QTest::newRow("videosChanged") << "videosChanged";
  QTest::newRow("followedChannelsChanged") << "followedChannelsChanged";
  QTest::newRow("newStreamersChanged") << "newStreamersChanged";
  QTest::newRow("categoryStreamsChanged") << "categoryStreamsChanged";
  QTest::newRow("searchChannelResultsChanged") << "searchChannelResultsChanged";
  QTest::newRow("searchCategoryResultsChanged") << "searchCategoryResultsChanged";
  QTest::newRow("userIdChanged") << "userIdChanged";
  QTest::newRow("userNameChanged") << "userNameChanged";
  QTest::newRow("hlsUrlReady") << "hlsUrlReady";
  QTest::newRow("errorOccurred") << "errorOccurred";
  QTest::newRow("adsDetected") << "adsDetected";
  QTest::newRow("adsFinished") << "adsFinished";
}

void TestTwitchService::testSignalValidity() {
  QFETCH(QString, signalName);

  const QMetaObject* metaObject = m_service->metaObject();
  int signalIndex = -1;

  for (int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i) {
    QMetaMethod method = metaObject->method(i);
    if (method.methodType() == QMetaMethod::Signal &&
        QString(method.name()) == signalName) {
      signalIndex = i;
      break;
    }
  }

  QVERIFY2(signalIndex >= 0, qPrintable("Signal " + signalName + " not found"));

  QMetaMethod signalMethod = metaObject->method(signalIndex);
  QString normalizedSignal = QString("2") + signalMethod.methodSignature();
  QSignalSpy spy(m_service, normalizedSignal.toLatin1().constData());

  QVERIFY2(spy.isValid(), qPrintable("Signal " + signalName + " spy is not valid"));
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

// ===== Data-driven test: refresh without auth =====

void TestTwitchService::testRefreshWithoutAuth_data() {
  QTest::addColumn<QString>("refreshMethod");
  QTest::addColumn<QString>("propertyName");
  QTest::addColumn<QString>("categoryId");

  QTest::newRow("streams") << "refreshStreams" << "streams" << "";
  QTest::newRow("recommendedStreams") << "refreshRecommendedStreams" << "recommendedStreams" << "";
  QTest::newRow("categories") << "refreshCategories" << "categories" << "";
  QTest::newRow("popularClips") << "refreshPopularClips" << "popularClips" << "";
  QTest::newRow("followedClips") << "refreshFollowedClips" << "followedClips" << "";
  QTest::newRow("videos") << "refreshVideos" << "videos" << "";
  QTest::newRow("followedChannels") << "refreshFollowedChannels" << "followedChannels" << "";
  QTest::newRow("newStreamers") << "refreshNewStreamers" << "newStreamers" << "";
  QTest::newRow("categoryStreams") << "refreshCategoryStreams" << "categoryStreams" << "12345";
}

void TestTwitchService::testRefreshWithoutAuth() {
  QFETCH(QString, refreshMethod);
  QFETCH(QString, propertyName);
  QFETCH(QString, categoryId);

  m_service->logout();

  // Invoke the refresh method
  if (categoryId.isEmpty()) {
    QMetaObject::invokeMethod(m_service, refreshMethod.toLatin1().constData());
  } else {
    QMetaObject::invokeMethod(m_service, refreshMethod.toLatin1().constData(),
                              Q_ARG(QString, categoryId));
  }

  // Verify property is still empty
  QVariant value = m_service->property(propertyName.toLatin1().constData());
  QVERIFY2(value.isValid(), qPrintable("Property " + propertyName + " not found"));
  QVERIFY2(value.toList().isEmpty(),
           qPrintable(propertyName + " should be empty after refresh without auth"));
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
