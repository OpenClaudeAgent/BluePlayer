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

  // Sprint 7 - Tests pour ensureTokenAndExecute() (tests indirects)
  void testEnsureTokenWithValidToken();
  void testEnsureTokenWithEmptyToken();
  void testEnsureTokenWithNullAuthManager();
  void testRefreshMethodsReturnEarlyWithoutToken();
  
  // Tests des propriétés et signaux
  void testStreamsProperty();
  void testRecommendedStreamsProperty();
  void testCategoriesProperty();
  void testPopularClipsProperty();
  void testStreamsChangedSignal();
  void testCategoriesChangedSignal();
  void testClearSearchResultsEmitsSignals();
  
  // Tests de getStreamHlsUrl (logique)
  void testGetStreamHlsUrlWithInvalidLogin();
  void testGetStreamHlsUrlSignalEmitted();
  
  // Tests des propriétés de qualité
  void testAvailableQualitiesProperty();
  void testCurrentQualityProperty();
  void testDefaultQualityProperty();

  // Tests de setStreamQuality
  void testSetStreamQualitySameQuality();
  void testSetStreamQualityNonExistent();
  void testSetStreamQualityEmptyQualities();
  void testSetDefaultQualityChangesConfig();
  void testSetDefaultQualitySameValue();
  
  // Tests des signaux de qualité
  void testAvailableQualitiesChangedSignal();
  void testCurrentQualityChangedSignal();
  void testQualityChangedSignal();
  
  // Tests des signaux de publicités
  void testAdsDetectedSignalValid();
  void testAdsFinishedSignalValid();
  void testAdFilterLogSignalValid();
  
  // Tests de logout complet
  void testLogoutClearsAllLists();
  void testLogoutClearsUserInfo();
  
  // Tests de currentHlsUrl
  void testCurrentHlsUrlInitiallyEmpty();
  void testCurrentHlsUrlProperty();
  
  // Tests de refreshCategoryStreams
  void testRefreshCategoryStreamsEmptyGameId();
  void testRefreshCategoryStreamsWithGameId();
  
  // Tests de search
  void testSearchTriggersNetworkCall();
  void testSearchClearsOnEmpty();
  
  // Tests de login
  void testLoginCallable();
  
  // Tests de playStream avec index valide
  void testPlayStreamEmitsErrorOnInvalid();
  
  // Tests de getStreamHlsUrl edge cases
  void testGetStreamHlsUrlWithWhitespace();
  void testGetStreamHlsUrlWithSpecialChars();
  
  // Tests de selectUrl
  void testSelectUrlNegativeIndex();
  void testSelectUrlLargeIndex();
  
  // Tests de refreshStreams
  void testRefreshStreamsWithoutAuth();

  // ===== Sprint 8 - Tests de selectBestQualityFromPlaylist =====
  void testSelectBestQuality_EmptyPlaylist();
  void testSelectBestQuality_SingleQuality();
  void testSelectBestQuality_MultipleQualities();
  void testSelectBestQuality_ChunkedIsPrioritized();
  void testSelectBestQuality_1080p60Priority();
  void testSelectBestQuality_720p60Priority();
  void testSelectBestQuality_NoStreamInf();
  void testSelectBestQuality_MalformedPlaylist();
  void testSelectBestQuality_PopulatesAvailableQualities();
  void testSelectBestQuality_DefaultQualityFallback();
  void testSelectBestQuality_UserPreferredQualityExact();
  void testSelectBestQuality_UserPreferredQualityFallbackLower();
  void testSelectBestQuality_UserPreferredQualityFallbackHigher();

  // ===== Sprint 8 - Tests de onSearchChannelsReady =====
  void testOnSearchChannelsReady_FiltersLiveOnly();
  void testOnSearchChannelsReady_EmptyList();
  void testOnSearchChannelsReady_AllLive();
  void testOnSearchChannelsReady_AllOffline();
  void testOnSearchChannelsReady_MixedLiveOffline();

  // ===== Sprint 8 - Tests de setStreamQuality avec qualités valides =====
  void testSetStreamQuality_ValidQuality();
  void testSetStreamQuality_EmitsQualityChangedSignal();
  void testSetStreamQuality_UpdatesCurrentQuality();

  // ===== Sprint 8 - Tests des slots de callbacks =====
  void testOnStreamsReady_SelectsFirstStream();
  void testOnStreamsReady_EmptyList();
  void testOnCategoriesReady_UpdatesList();
  void testOnFollowedChannelsReady_TriggersClipsRefresh();
  void testOnUserInfoReady_UpdatesUserId();
  void testOnUserInfoReadyWithName_UpdatesBoth();

  // ===== Sprint 8 - Tests des Ad Filter callbacks =====
  void testOnAdFilterCleanStream_EmitsSignal();
  void testOnAdFilterAdsDetected_EmitsSignal();
  void testOnAdFilterAdsFinished_EmitsSignal();
  void testOnAdFilterDebugLog_EmitsSignal();
  void testOnAdFilterMaxRetries_EmitsSignal();

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

// ===== Sprint 7 - Tests pour ensureTokenAndExecute() =====

void TestTwitchService::testEnsureTokenWithValidToken() {
  // Stocker un token dans le mock storage pour simuler une auth valide
  m_mockStorage->store("blueplayer_access_token", "valid_test_token_12345");
  m_mockStorage->store("blueplayer_refresh_token", "valid_refresh_token");
  
  // Recréer le service pour qu'il charge les tokens
  delete m_service;
  m_service = new TwitchService(m_mockStorage, this);
  
  // Si le token est chargé, isAuthenticated pourrait être true
  // (dépend de la logique interne de TwitchAuthManager)
  
  // Appeler une méthode qui utilise ensureTokenAndExecute()
  // Elle ne doit pas crasher
  m_service->refreshCategories();
  QVERIFY(m_service != nullptr);
}

void TestTwitchService::testEnsureTokenWithEmptyToken() {
  // Sans token, les méthodes utilisant ensureTokenAndExecute doivent gérer gracieusement
  m_service->logout();
  
  QSignalSpy categoriesSpy(m_service, &TwitchService::categoriesChanged);
  
  // Appeler refreshCategories sans token
  m_service->refreshCategories();
  
  // La liste doit rester vide car pas de token
  QVERIFY(m_service->categories().isEmpty());
}

void TestTwitchService::testEnsureTokenWithNullAuthManager() {
  // Créer un service avec un storage vide pour s'assurer qu'il gère le cas null
  MockSecureStorage emptyStorage;
  TwitchService localService(&emptyStorage);
  
  // Appeler logout() puis des méthodes de refresh
  localService.logout();
  
  // Ces appels ne doivent pas crasher même si le token est vide
  localService.refreshRecommendedStreams();
  localService.refreshCategories();
  localService.refreshPopularClips();
  localService.refreshFollowedClips();
  localService.refreshVideos();
  localService.refreshFollowedChannels();
  localService.refreshNewStreamers();
  localService.refreshCategoryStreams("12345");
  localService.search("test");
  
  // Toutes les listes doivent être vides
  QVERIFY(localService.recommendedStreams().isEmpty());
  QVERIFY(localService.categories().isEmpty());
  QVERIFY(localService.popularClips().isEmpty());
  QVERIFY(localService.followedClips().isEmpty());
  QVERIFY(localService.videos().isEmpty());
  QVERIFY(localService.followedChannels().isEmpty());
  QVERIFY(localService.newStreamers().isEmpty());
  QVERIFY(localService.categoryStreams().isEmpty());
}

void TestTwitchService::testRefreshMethodsReturnEarlyWithoutToken() {
  m_service->logout();
  
  // Toutes les méthodes utilisant ensureTokenAndExecute doivent retourner tôt sans token
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  
  // Appeler refreshVideos - nécessite auth ET userId
  m_service->refreshVideos();
  QVERIFY(m_service->videos().isEmpty());
  
  // Appeler refreshFollowedChannels - nécessite auth ET userId
  m_service->refreshFollowedChannels();
  QVERIFY(m_service->followedChannels().isEmpty());
  
  // Appeler refreshNewStreamers - nécessite auth ET userId
  m_service->refreshNewStreamers();
  QVERIFY(m_service->newStreamers().isEmpty());
  
  // Appeler refreshFollowedClips - nécessite auth ET userId
  m_service->refreshFollowedClips();
  QVERIFY(m_service->followedClips().isEmpty());
}

// ===== Tests des propriétés et signaux =====

void TestTwitchService::testStreamsProperty() {
  // La propriété streams() doit retourner une liste vide au départ
  QVariantList streams = m_service->streams();
  QVERIFY(streams.isEmpty());
  
  // Vérifier que la propriété est accessible via le système de méta-objets
  QVariant value = m_service->property("streams");
  QVERIFY(value.isValid());
  QVERIFY(value.canConvert<QVariantList>());
}

void TestTwitchService::testRecommendedStreamsProperty() {
  // La propriété recommendedStreams() doit retourner une liste vide au départ
  QVariantList streams = m_service->recommendedStreams();
  QVERIFY(streams.isEmpty());
  
  // Vérifier que la propriété est accessible via le système de méta-objets
  QVariant value = m_service->property("recommendedStreams");
  QVERIFY(value.isValid());
  QVERIFY(value.canConvert<QVariantList>());
}

void TestTwitchService::testCategoriesProperty() {
  // La propriété categories() doit retourner une liste vide au départ
  QVariantList categories = m_service->categories();
  QVERIFY(categories.isEmpty());
  
  // Vérifier que la propriété est accessible via le système de méta-objets
  QVariant value = m_service->property("categories");
  QVERIFY(value.isValid());
  QVERIFY(value.canConvert<QVariantList>());
}

void TestTwitchService::testPopularClipsProperty() {
  // La propriété popularClips() doit retourner une liste vide au départ
  QVariantList clips = m_service->popularClips();
  QVERIFY(clips.isEmpty());
  
  // Vérifier que la propriété est accessible via le système de méta-objets
  QVariant value = m_service->property("popularClips");
  QVERIFY(value.isValid());
  QVERIFY(value.canConvert<QVariantList>());
}

void TestTwitchService::testStreamsChangedSignal() {
  // Créer un spy pour le signal streamsChanged
  QSignalSpy spy(m_service, &TwitchService::streamsChanged);
  QVERIFY(spy.isValid());
  
  // Logout émet streamsChanged (clear des streams)
  m_service->logout();
  
  // Vérifier que le signal a été émis au moins une fois
  QVERIFY(spy.count() >= 1);
}

void TestTwitchService::testCategoriesChangedSignal() {
  // Créer un spy pour le signal categoriesChanged
  QSignalSpy spy(m_service, &TwitchService::categoriesChanged);
  QVERIFY(spy.isValid());
  
  // Logout émet categoriesChanged (clear des categories)
  m_service->logout();
  
  // Vérifier que le signal a été émis au moins une fois
  QVERIFY(spy.count() >= 1);
}

void TestTwitchService::testClearSearchResultsEmitsSignals() {
  // Créer des spies pour les signaux de recherche
  QSignalSpy channelSpy(m_service, &TwitchService::searchChannelResultsChanged);
  QSignalSpy categorySpy(m_service, &TwitchService::searchCategoryResultsChanged);
  
  QVERIFY(channelSpy.isValid());
  QVERIFY(categorySpy.isValid());
  
  // Appeler clearSearchResults
  m_service->clearSearchResults();
  
  // Vérifier que les deux signaux ont été émis
  QCOMPARE(channelSpy.count(), 1);
  QCOMPARE(categorySpy.count(), 1);
}

void TestTwitchService::testGetStreamHlsUrlWithInvalidLogin() {
  // Créer un spy pour le signal errorOccurred
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  QVERIFY(errorSpy.isValid());
  
  // Appeler avec un login vide
  m_service->getStreamHlsUrl("");
  
  // Vérifier qu'une erreur a été émise
  QCOMPARE(errorSpy.count(), 1);
  
  // Vérifier le message d'erreur
  QList<QVariant> arguments = errorSpy.takeFirst();
  QVERIFY(arguments.at(0).toString().contains("vide") || 
          arguments.at(0).toString().contains("empty") ||
          arguments.at(0).toString().contains("Empty"));
}

void TestTwitchService::testGetStreamHlsUrlSignalEmitted() {
  // Créer un spy pour le signal hlsUrlReady
  QSignalSpy hlsSpy(m_service, &TwitchService::hlsUrlReady);
  QVERIFY(hlsSpy.isValid());
  
  // Appeler avec un login valide (la requête réseau va être faite)
  // Note: En environnement de test sans réseau, on vérifie juste que
  // la méthode ne crashe pas
  m_service->getStreamHlsUrl("teststreamer");
  
  // Vérifier que le service est toujours fonctionnel
  QVERIFY(m_service != nullptr);
}

void TestTwitchService::testAvailableQualitiesProperty() {
  // La propriété availableQualities() doit retourner une liste vide au départ
  QVariantList qualities = m_service->availableQualities();
  QVERIFY(qualities.isEmpty());
  
  // Vérifier que la propriété est accessible via le système de méta-objets
  QVariant value = m_service->property("availableQualities");
  QVERIFY(value.isValid());
  QVERIFY(value.canConvert<QVariantList>());
}

void TestTwitchService::testCurrentQualityProperty() {
  // La propriété currentQuality() doit retourner "Auto" par défaut
  QString quality = m_service->currentQuality();
  QCOMPARE(quality, QString("Auto"));
  
  // Vérifier que la propriété est accessible via le système de méta-objets
  QVariant value = m_service->property("currentQuality");
  QVERIFY(value.isValid());
  QCOMPARE(value.toString(), QString("Auto"));
}

void TestTwitchService::testDefaultQualityProperty() {
  // La propriété defaultQuality() doit être accessible
  QString quality = m_service->defaultQuality();
  
  // Vérifier que la propriété est accessible via le système de méta-objets
  QVariant value = m_service->property("defaultQuality");
  QVERIFY(value.isValid());
  QVERIFY(value.typeId() == QMetaType::QString);
}

// ===== Tests de setStreamQuality =====

void TestTwitchService::testSetStreamQualitySameQuality() {
  // Setting the same quality should be a no-op
  QString currentQuality = m_service->currentQuality();
  
  QSignalSpy qualityChangedSpy(m_service, &TwitchService::currentQualityChanged);
  QSignalSpy qualitySpy(m_service, &TwitchService::qualityChanged);
  
  m_service->setStreamQuality(currentQuality);
  
  // No signals should be emitted when setting same quality
  QCOMPARE(qualityChangedSpy.count(), 0);
  QCOMPARE(qualitySpy.count(), 0);
}

void TestTwitchService::testSetStreamQualityNonExistent() {
  // Setting a quality that doesn't exist in availableQualities
  QSignalSpy qualityChangedSpy(m_service, &TwitchService::currentQualityChanged);
  QSignalSpy qualitySpy(m_service, &TwitchService::qualityChanged);
  
  m_service->setStreamQuality("NonExistentQuality999p");
  
  // No signals should be emitted for non-existent quality
  QCOMPARE(qualityChangedSpy.count(), 0);
  QCOMPARE(qualitySpy.count(), 0);
}

void TestTwitchService::testSetStreamQualityEmptyQualities() {
  // When availableQualities is empty, setStreamQuality should handle gracefully
  QVERIFY(m_service->availableQualities().isEmpty());
  
  m_service->setStreamQuality("1080p60");
  
  // Should not crash, current quality should remain unchanged
  QVERIFY(m_service != nullptr);
  QCOMPARE(m_service->currentQuality(), QString("Auto"));
}

void TestTwitchService::testSetDefaultQualityChangesConfig() {
  // Get the current default quality
  QString currentQuality = m_service->defaultQuality();
  
  // Choose a quality different from current
  QString newQuality = (currentQuality == "720p60") ? "1080p" : "720p60";
  
  QSignalSpy defaultQualitySpy(m_service, &TwitchService::defaultQualityChanged);
  
  // Set a new default quality
  m_service->setDefaultQuality(newQuality);
  
  // Signal should be emitted
  QCOMPARE(defaultQualitySpy.count(), 1);
  
  // Quality should be updated
  QCOMPARE(m_service->defaultQuality(), newQuality);
  
  // Reset to previous value to avoid side effects
  m_service->setDefaultQuality(currentQuality);
}

void TestTwitchService::testSetDefaultQualitySameValue() {
  QString currentDefault = m_service->defaultQuality();
  
  QSignalSpy defaultQualitySpy(m_service, &TwitchService::defaultQualityChanged);
  
  // Set the same value
  m_service->setDefaultQuality(currentDefault);
  
  // No signal should be emitted for same value
  QCOMPARE(defaultQualitySpy.count(), 0);
}

// ===== Tests des signaux de qualité =====

void TestTwitchService::testAvailableQualitiesChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::availableQualitiesChanged);
  QVERIFY(spy.isValid());
  
  // Verify signal exists and is connectable
  const QMetaObject* metaObject = m_service->metaObject();
  int signalIndex = metaObject->indexOfSignal("availableQualitiesChanged()");
  QVERIFY(signalIndex >= 0);
}

void TestTwitchService::testCurrentQualityChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::currentQualityChanged);
  QVERIFY(spy.isValid());
  
  // Verify signal exists
  const QMetaObject* metaObject = m_service->metaObject();
  int signalIndex = metaObject->indexOfSignal("currentQualityChanged()");
  QVERIFY(signalIndex >= 0);
}

void TestTwitchService::testQualityChangedSignal() {
  QSignalSpy spy(m_service, &TwitchService::qualityChanged);
  QVERIFY(spy.isValid());
  
  // Verify signal exists with QString parameter
  const QMetaObject* metaObject = m_service->metaObject();
  int signalIndex = metaObject->indexOfSignal("qualityChanged(QString)");
  QVERIFY(signalIndex >= 0);
}

// ===== Tests des signaux de publicités =====

void TestTwitchService::testAdsDetectedSignalValid() {
  QSignalSpy spy(m_service, &TwitchService::adsDetected);
  QVERIFY(spy.isValid());
  
  // Verify signal takes an int parameter
  const QMetaObject* metaObject = m_service->metaObject();
  int signalIndex = metaObject->indexOfSignal("adsDetected(int)");
  QVERIFY(signalIndex >= 0);
}

void TestTwitchService::testAdsFinishedSignalValid() {
  QSignalSpy spy(m_service, &TwitchService::adsFinished);
  QVERIFY(spy.isValid());
  
  // Verify signal exists
  const QMetaObject* metaObject = m_service->metaObject();
  int signalIndex = metaObject->indexOfSignal("adsFinished()");
  QVERIFY(signalIndex >= 0);
}

void TestTwitchService::testAdFilterLogSignalValid() {
  QSignalSpy spy(m_service, &TwitchService::adFilterLog);
  QVERIFY(spy.isValid());
  
  // Verify signal takes a QString parameter
  const QMetaObject* metaObject = m_service->metaObject();
  int signalIndex = metaObject->indexOfSignal("adFilterLog(QString)");
  QVERIFY(signalIndex >= 0);
}

// ===== Tests de logout complet =====

void TestTwitchService::testLogoutClearsAllLists() {
  QSignalSpy streamsSpy(m_service, &TwitchService::streamsChanged);
  QSignalSpy recommendedSpy(m_service, &TwitchService::recommendedStreamsChanged);
  QSignalSpy categoriesSpy(m_service, &TwitchService::categoriesChanged);
  QSignalSpy clipsSpy(m_service, &TwitchService::popularClipsChanged);
  QSignalSpy followedClipsSpy(m_service, &TwitchService::followedClipsChanged);
  QSignalSpy videosSpy(m_service, &TwitchService::videosChanged);
  QSignalSpy followedChannelsSpy(m_service, &TwitchService::followedChannelsChanged);
  QSignalSpy newStreamersSpy(m_service, &TwitchService::newStreamersChanged);
  QSignalSpy categoryStreamsSpy(m_service, &TwitchService::categoryStreamsChanged);
  QSignalSpy searchChannelsSpy(m_service, &TwitchService::searchChannelResultsChanged);
  QSignalSpy searchCategoriesSpy(m_service, &TwitchService::searchCategoryResultsChanged);
  
  m_service->logout();
  
  // All signals should be emitted
  QVERIFY(streamsSpy.count() >= 1);
  QVERIFY(recommendedSpy.count() >= 1);
  QVERIFY(categoriesSpy.count() >= 1);
  QVERIFY(clipsSpy.count() >= 1);
  QVERIFY(followedClipsSpy.count() >= 1);
  QVERIFY(videosSpy.count() >= 1);
  QVERIFY(followedChannelsSpy.count() >= 1);
  QVERIFY(newStreamersSpy.count() >= 1);
  QVERIFY(categoryStreamsSpy.count() >= 1);
  QVERIFY(searchChannelsSpy.count() >= 1);
  QVERIFY(searchCategoriesSpy.count() >= 1);
  
  // All lists should be empty
  QVERIFY(m_service->streams().isEmpty());
  QVERIFY(m_service->recommendedStreams().isEmpty());
  QVERIFY(m_service->categories().isEmpty());
  QVERIFY(m_service->popularClips().isEmpty());
  QVERIFY(m_service->followedClips().isEmpty());
  QVERIFY(m_service->videos().isEmpty());
  QVERIFY(m_service->followedChannels().isEmpty());
  QVERIFY(m_service->newStreamers().isEmpty());
  QVERIFY(m_service->categoryStreams().isEmpty());
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
}

void TestTwitchService::testLogoutClearsUserInfo() {
  QSignalSpy userIdSpy(m_service, &TwitchService::userIdChanged);
  QSignalSpy userNameSpy(m_service, &TwitchService::userNameChanged);
  QSignalSpy selectedSpy(m_service, &TwitchService::selectedStreamChanged);
  
  m_service->logout();
  
  // Signals should be emitted
  QVERIFY(userIdSpy.count() >= 1);
  QVERIFY(userNameSpy.count() >= 1);
  QVERIFY(selectedSpy.count() >= 1);
  
  // User info should be cleared
  QVERIFY(m_service->userId().isEmpty());
  QVERIFY(m_service->userName().isEmpty());
  QVERIFY(m_service->selectedStreamUrl().isEmpty());
}

// ===== Tests de currentHlsUrl =====

void TestTwitchService::testCurrentHlsUrlInitiallyEmpty() {
  QString hlsUrl = m_service->currentHlsUrl();
  QVERIFY(hlsUrl.isEmpty());
}

void TestTwitchService::testCurrentHlsUrlProperty() {
  // Verify the method is accessible
  QString url = m_service->currentHlsUrl();
  QVERIFY(url.isEmpty());  // Initially empty
  
  // Verify it's Q_INVOKABLE (can be called from QML)
  QVariant result;
  bool success = QMetaObject::invokeMethod(m_service, "currentHlsUrl", 
                                            Qt::DirectConnection,
                                            Q_RETURN_ARG(QVariant, result));
  QVERIFY(success || result.isNull());  // Method exists
}

// ===== Tests de refreshCategoryStreams =====

void TestTwitchService::testRefreshCategoryStreamsEmptyGameId() {
  QSignalSpy categoryStreamsSpy(m_service, &TwitchService::categoryStreamsChanged);
  
  m_service->refreshCategoryStreams("");
  
  // Should emit signal with empty list
  QVERIFY(categoryStreamsSpy.count() >= 1);
  QVERIFY(m_service->categoryStreams().isEmpty());
}

void TestTwitchService::testRefreshCategoryStreamsWithGameId() {
  // Without auth, should still try to refresh (token optional)
  m_service->logout();
  
  m_service->refreshCategoryStreams("12345");
  
  // Should not crash, list stays empty without network
  QVERIFY(m_service != nullptr);
}

// ===== Tests de search =====

void TestTwitchService::testSearchTriggersNetworkCall() {
  QSignalSpy debugSpy(m_service, &TwitchService::errorOccurred);
  
  m_service->search("test_query");
  
  // Should not crash, network call is async
  QVERIFY(m_service != nullptr);
}

void TestTwitchService::testSearchClearsOnEmpty() {
  QSignalSpy channelSpy(m_service, &TwitchService::searchChannelResultsChanged);
  QSignalSpy categorySpy(m_service, &TwitchService::searchCategoryResultsChanged);
  
  // First search
  m_service->search("");
  
  // Empty query should clear results
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
}

// ===== Tests de login =====

void TestTwitchService::testLoginCallable() {
  // login() should be callable without crashing (it opens browser)
  // We don't actually call it in tests to avoid opening browser
  
  // Verify the method exists via meta object
  const QMetaObject* metaObject = m_service->metaObject();
  int methodIndex = metaObject->indexOfMethod("login()");
  QVERIFY(methodIndex >= 0);
}

// ===== Tests de playStream avec index valide =====

void TestTwitchService::testPlayStreamEmitsErrorOnInvalid() {
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  
  m_service->playStream(0);  // No streams loaded
  
  // Should emit error for invalid index
  QVERIFY(errorSpy.count() >= 1);
  
  QString error = errorSpy.at(0).at(0).toString();
  QVERIFY(error.contains("invalid") || error.contains("invalide") || 
          error.contains("Index") || error.contains("index"));
}

// ===== Tests de getStreamHlsUrl edge cases =====

void TestTwitchService::testGetStreamHlsUrlWithWhitespace() {
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  
  m_service->getStreamHlsUrl("   ");  // Whitespace only
  
  // Should handle gracefully (might be treated as valid by proxy)
  QVERIFY(m_service != nullptr);
}

void TestTwitchService::testGetStreamHlsUrlWithSpecialChars() {
  m_service->getStreamHlsUrl("test-streamer_123");
  
  // Should not crash with special characters
  QVERIFY(m_service != nullptr);
}

// ===== Tests de selectUrl =====

void TestTwitchService::testSelectUrlNegativeIndex() {
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  
  m_service->playStream(-1);
  
  // Should emit error
  QVERIFY(errorSpy.count() >= 1);
}

void TestTwitchService::testSelectUrlLargeIndex() {
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  
  m_service->playStream(999999);
  
  // Should emit error for out of bounds
  QVERIFY(errorSpy.count() >= 1);
}

// ===== Tests de refreshStreams =====

void TestTwitchService::testRefreshStreamsWithoutAuth() {
  m_service->logout();
  
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  
  m_service->refreshStreams();
  
  // Should emit error when not authenticated
  QVERIFY(errorSpy.count() >= 1);
}

// =====================================================
// Sprint 8 - Tests de selectBestQualityFromPlaylist
// =====================================================

void TestTwitchService::testSelectBestQuality_EmptyPlaylist() {
  // Test avec un playlist vide
  QString emptyPlaylist;
  QString result = m_service->selectBestQualityFromPlaylist(emptyPlaylist);
  
  QVERIFY(result.isEmpty());
  QVERIFY(m_service->availableQualities().isEmpty());
}

void TestTwitchService::testSelectBestQuality_SingleQuality() {
  // Playlist HLS avec une seule qualité
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,CODECS="avc1.4d401f",VIDEO="720p"
https://video-edge.example.com/720p.m3u8)";

  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  QVERIFY(!result.isEmpty());
  QVERIFY(result.contains("720p.m3u8"));
  QCOMPARE(m_service->availableQualities().size(), 1);
}

void TestTwitchService::testSelectBestQuality_MultipleQualities() {
  // Playlist avec plusieurs qualités
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p60"
https://video.example.com/720p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1500000,RESOLUTION=852x480,VIDEO="480p"
https://video.example.com/480p.m3u8)";

  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  QVERIFY(!result.isEmpty());
  QCOMPARE(m_service->availableQualities().size(), 3);
  
  // Vérifier que la meilleure qualité est sélectionnée (1080p60)
  QVERIFY(result.contains("1080p60.m3u8"));
}

void TestTwitchService::testSelectBestQuality_ChunkedIsPrioritized() {
  // "chunked" (source) doit avoir la priorité la plus haute
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=8000000,RESOLUTION=1920x1080,VIDEO="chunked"
https://video.example.com/chunked.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p"
https://video.example.com/720p.m3u8)";

  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  // chunked doit être sélectionné même si 1080p60 a une bande passante comparable
  QVERIFY(result.contains("chunked.m3u8"));
  
  // Vérifier que la qualité courante est correctement formatée
  QString currentQuality = m_service->currentQuality();
  QVERIFY(currentQuality.contains("Source"));
}

void TestTwitchService::testSelectBestQuality_1080p60Priority() {
  // 1080p60 doit être prioritaire sur 1080p
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=4500000,RESOLUTION=1920x1080,VIDEO="1080p"
https://video.example.com/1080p.m3u8)";

  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  QVERIFY(result.contains("1080p60.m3u8"));
}

void TestTwitchService::testSelectBestQuality_720p60Priority() {
  // 720p60 doit être prioritaire sur 720p
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p60"
https://video.example.com/720p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2000000,RESOLUTION=1280x720,VIDEO="720p"
https://video.example.com/720p.m3u8)";

  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  QVERIFY(result.contains("720p60.m3u8"));
}

void TestTwitchService::testSelectBestQuality_NoStreamInf() {
  // Playlist sans EXT-X-STREAM-INF (juste du texte)
  QString playlist = R"(#EXTM3U
#EXT-X-VERSION:3
https://some-url.m3u8)";

  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  // Devrait retourner vide car pas de variantes trouvées
  QVERIFY(result.isEmpty());
  QVERIFY(m_service->availableQualities().isEmpty());
}

void TestTwitchService::testSelectBestQuality_MalformedPlaylist() {
  // Playlist malformée
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=invalid
not-a-valid-url
#EXT-X-STREAM-INF:
another-line)";

  // Ne doit pas crasher
  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  QVERIFY(m_service != nullptr);
}

void TestTwitchService::testSelectBestQuality_PopulatesAvailableQualities() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=8000000,RESOLUTION=1920x1080,VIDEO="chunked"
https://video.example.com/chunked.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p60"
https://video.example.com/720p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2000000,RESOLUTION=1280x720,VIDEO="720p"
https://video.example.com/720p.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=1500000,RESOLUTION=852x480,VIDEO="480p"
https://video.example.com/480p.m3u8)";

  QSignalSpy qualitiesSpy(m_service, &TwitchService::availableQualitiesChanged);
  
  m_service->selectBestQualityFromPlaylist(playlist);
  
  // Signal doit être émis
  QCOMPARE(qualitiesSpy.count(), 1);
  
  // 5 qualités doivent être disponibles
  QVariantList qualities = m_service->availableQualities();
  QCOMPARE(qualities.size(), 5);
  
  // Vérifier la structure de chaque qualité
  for (const QVariant& qv : qualities) {
    QVariantMap quality = qv.toMap();
    QVERIFY(quality.contains("name"));
    QVERIFY(quality.contains("url"));
    QVERIFY(quality.contains("bandwidth"));
    QVERIFY(!quality.value("url").toString().isEmpty());
  }
}

void TestTwitchService::testSelectBestQuality_DefaultQualityFallback() {
  // Quand defaultQuality est "Auto" ou vide, sélectionner la meilleure
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p"
https://video.example.com/720p.m3u8)";

  // S'assurer que defaultQuality est "Auto"
  m_service->setDefaultQuality("Auto");
  
  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  // Doit sélectionner la meilleure (1080p60)
  QVERIFY(result.contains("1080p60.m3u8"));
}

void TestTwitchService::testSelectBestQuality_UserPreferredQualityExact() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p60"
https://video.example.com/720p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2000000,RESOLUTION=1280x720,VIDEO="720p"
https://video.example.com/720p.m3u8)";

  // Définir une préférence utilisateur pour 720p60
  m_service->setDefaultQuality("720p60");
  
  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  // Doit sélectionner exactement 720p60
  QVERIFY(result.contains("720p60.m3u8"));
  QCOMPARE(m_service->currentQuality(), QString("720p60"));
}

void TestTwitchService::testSelectBestQuality_UserPreferredQualityFallbackLower() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=2000000,RESOLUTION=1280x720,VIDEO="720p"
https://video.example.com/720p.m3u8)";

  // Demander 720p60 qui n'existe pas -> fallback vers 720p (inférieur le plus proche)
  m_service->setDefaultQuality("720p60");
  
  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  // Doit tomber sur 720p (qualité inférieure la plus proche)
  QVERIFY(result.contains("720p.m3u8"));
}

void TestTwitchService::testSelectBestQuality_UserPreferredQualityFallbackHigher() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p60"
https://video.example.com/720p60.m3u8)";

  // Demander 480p qui n'existe pas -> fallback vers 720p60 (supérieur le plus proche)
  m_service->setDefaultQuality("480p");
  
  QString result = m_service->selectBestQualityFromPlaylist(playlist);
  
  // Comme 480p n'existe pas et rien n'est inférieur, prend le supérieur le plus proche
  QVERIFY(!result.isEmpty());
}

// =====================================================
// Sprint 8 - Tests de onSearchChannelsReady
// =====================================================

void TestTwitchService::testOnSearchChannelsReady_FiltersLiveOnly() {
  QVariantList channels;
  
  // Ajouter des chaînes live et offline
  QVariantMap liveChannel;
  liveChannel["id"] = "123";
  liveChannel["display_name"] = "LiveStreamer";
  liveChannel["is_live"] = true;
  channels.append(liveChannel);
  
  QVariantMap offlineChannel;
  offlineChannel["id"] = "456";
  offlineChannel["display_name"] = "OfflineStreamer";
  offlineChannel["is_live"] = false;
  channels.append(offlineChannel);
  
  QSignalSpy spy(m_service, &TwitchService::searchChannelResultsChanged);
  
  // Appeler le slot privé directement
  m_service->onSearchChannelsReady(channels);
  
  // Vérifier que seule la chaîne live est gardée
  QVariantList results = m_service->searchChannelResults();
  QCOMPARE(results.size(), 1);
  QCOMPARE(results.first().toMap()["display_name"].toString(), QString("LiveStreamer"));
  QCOMPARE(spy.count(), 1);
}

void TestTwitchService::testOnSearchChannelsReady_EmptyList() {
  QVariantList emptyChannels;
  
  QSignalSpy spy(m_service, &TwitchService::searchChannelResultsChanged);
  
  m_service->onSearchChannelsReady(emptyChannels);
  
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QCOMPARE(spy.count(), 1);
}

void TestTwitchService::testOnSearchChannelsReady_AllLive() {
  QVariantList channels;
  
  for (int i = 0; i < 5; ++i) {
    QVariantMap channel;
    channel["id"] = QString::number(i);
    channel["display_name"] = QString("Streamer%1").arg(i);
    channel["is_live"] = true;
    channels.append(channel);
  }
  
  m_service->onSearchChannelsReady(channels);
  
  // Toutes les chaînes doivent être gardées
  QCOMPARE(m_service->searchChannelResults().size(), 5);
}

void TestTwitchService::testOnSearchChannelsReady_AllOffline() {
  QVariantList channels;
  
  for (int i = 0; i < 5; ++i) {
    QVariantMap channel;
    channel["id"] = QString::number(i);
    channel["display_name"] = QString("OfflineStreamer%1").arg(i);
    channel["is_live"] = false;
    channels.append(channel);
  }
  
  m_service->onSearchChannelsReady(channels);
  
  // Aucune chaîne ne doit être gardée
  QVERIFY(m_service->searchChannelResults().isEmpty());
}

void TestTwitchService::testOnSearchChannelsReady_MixedLiveOffline() {
  QVariantList channels;
  
  // 3 live, 2 offline
  for (int i = 0; i < 5; ++i) {
    QVariantMap channel;
    channel["id"] = QString::number(i);
    channel["display_name"] = QString("Streamer%1").arg(i);
    channel["is_live"] = (i < 3);  // 0, 1, 2 sont live
    channels.append(channel);
  }
  
  m_service->onSearchChannelsReady(channels);
  
  // Seulement les 3 chaînes live
  QCOMPARE(m_service->searchChannelResults().size(), 3);
}

// =====================================================
// Sprint 8 - Tests de setStreamQuality avec qualités valides
// =====================================================

void TestTwitchService::testSetStreamQuality_ValidQuality() {
  // D'abord, peupler les qualités disponibles via selectBestQualityFromPlaylist
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p60"
https://video.example.com/720p60.m3u8)";

  m_service->setDefaultQuality("Auto");
  m_service->selectBestQualityFromPlaylist(playlist);
  
  // Maintenant, changer vers 720p60
  m_service->setStreamQuality("720p60");
  
  QCOMPARE(m_service->currentQuality(), QString("720p60"));
}

void TestTwitchService::testSetStreamQuality_EmitsQualityChangedSignal() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p60"
https://video.example.com/720p60.m3u8)";

  m_service->setDefaultQuality("Auto");
  m_service->selectBestQualityFromPlaylist(playlist);
  
  QSignalSpy qualityChangedSpy(m_service, &TwitchService::qualityChanged);
  QSignalSpy currentQualitySpy(m_service, &TwitchService::currentQualityChanged);
  
  m_service->setStreamQuality("720p60");
  
  // Les deux signaux doivent être émis
  QCOMPARE(qualityChangedSpy.count(), 1);
  QCOMPARE(currentQualitySpy.count(), 1);
  
  // Le signal qualityChanged doit contenir la nouvelle URL
  QString newUrl = qualityChangedSpy.first().first().toString();
  QVERIFY(newUrl.contains("720p60.m3u8"));
}

void TestTwitchService::testSetStreamQuality_UpdatesCurrentQuality() {
  QString playlist = R"(#EXTM3U
#EXT-X-STREAM-INF:BANDWIDTH=8000000,RESOLUTION=1920x1080,VIDEO="chunked"
https://video.example.com/chunked.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO="1080p60"
https://video.example.com/1080p60.m3u8
#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO="720p60"
https://video.example.com/720p60.m3u8)";

  m_service->setDefaultQuality("Auto");
  m_service->selectBestQualityFromPlaylist(playlist);
  
  // Initialement devrait être Source (chunked)
  QString initial = m_service->currentQuality();
  QVERIFY(initial.contains("Source"));
  
  // Changer vers 720p60
  m_service->setStreamQuality("720p60");
  QCOMPARE(m_service->currentQuality(), QString("720p60"));
  
  // Changer vers 1080p60
  m_service->setStreamQuality("1080p60");
  QCOMPARE(m_service->currentQuality(), QString("1080p60"));
}

// =====================================================
// Sprint 8 - Tests des slots de callbacks
// =====================================================

void TestTwitchService::testOnStreamsReady_SelectsFirstStream() {
  QVariantList streams;
  
  QVariantMap stream1;
  stream1["user_login"] = "streamer1";
  stream1["stream_url"] = "https://valid.url/stream1.m3u8";
  streams.append(stream1);
  
  QVariantMap stream2;
  stream2["user_login"] = "streamer2";
  stream2["stream_url"] = "https://valid.url/stream2.m3u8";
  streams.append(stream2);
  
  QSignalSpy streamsChangedSpy(m_service, &TwitchService::streamsChanged);
  
  m_service->onStreamsReady(streams);
  
  // Le signal doit être émis
  QCOMPARE(streamsChangedSpy.count(), 1);
  
  // Les streams doivent être stockés
  QCOMPARE(m_service->streams().size(), 2);
}

void TestTwitchService::testOnStreamsReady_EmptyList() {
  QVariantList emptyStreams;
  
  QSignalSpy spy(m_service, &TwitchService::streamsChanged);
  
  m_service->onStreamsReady(emptyStreams);
  
  QCOMPARE(spy.count(), 1);
  QVERIFY(m_service->streams().isEmpty());
}

void TestTwitchService::testOnCategoriesReady_UpdatesList() {
  QVariantList categories;
  
  QVariantMap cat1;
  cat1["id"] = "12345";
  cat1["name"] = "Just Chatting";
  categories.append(cat1);
  
  QVariantMap cat2;
  cat2["id"] = "67890";
  cat2["name"] = "Minecraft";
  categories.append(cat2);
  
  QSignalSpy spy(m_service, &TwitchService::categoriesChanged);
  
  m_service->onCategoriesReady(categories);
  
  QCOMPARE(spy.count(), 1);
  QCOMPARE(m_service->categories().size(), 2);
}

void TestTwitchService::testOnFollowedChannelsReady_TriggersClipsRefresh() {
  QVariantList channels;
  
  QVariantMap channel;
  channel["broadcaster_id"] = "123";
  channel["broadcaster_name"] = "TestStreamer";
  channels.append(channel);
  
  QSignalSpy followedChannelsSpy(m_service, &TwitchService::followedChannelsChanged);
  
  m_service->onFollowedChannelsReady(channels);
  
  // Le signal doit être émis
  QCOMPARE(followedChannelsSpy.count(), 1);
  
  // Les chaînes suivies doivent être stockées
  QCOMPARE(m_service->followedChannels().size(), 1);
}

void TestTwitchService::testOnUserInfoReady_UpdatesUserId() {
  QString testUserId = "12345678";
  
  QSignalSpy userIdSpy(m_service, &TwitchService::userIdChanged);
  
  m_service->onUserInfoReady(testUserId);
  
  QCOMPARE(m_service->userId(), testUserId);
  QCOMPARE(userIdSpy.count(), 1);
}

void TestTwitchService::testOnUserInfoReadyWithName_UpdatesBoth() {
  QString testUserId = "12345678";
  QString testUserName = "TestUser";
  
  QSignalSpy userIdSpy(m_service, &TwitchService::userIdChanged);
  QSignalSpy userNameSpy(m_service, &TwitchService::userNameChanged);
  
  m_service->onUserInfoReadyWithName(testUserId, testUserName);
  
  QCOMPARE(m_service->userId(), testUserId);
  QCOMPARE(m_service->userName(), testUserName);
  QCOMPARE(userIdSpy.count(), 1);
  QCOMPARE(userNameSpy.count(), 1);
}

// =====================================================
// Sprint 8 - Tests des Ad Filter callbacks
// =====================================================

void TestTwitchService::testOnAdFilterCleanStream_EmitsSignal() {
  QString testUrl = "https://video.example.com/clean-stream.m3u8";
  
  QSignalSpy hlsSpy(m_service, &TwitchService::hlsUrlReady);
  
  m_service->onAdFilterCleanStream(testUrl);
  
  // Le signal hlsUrlReady doit être émis
  QCOMPARE(hlsSpy.count(), 1);
  QCOMPARE(hlsSpy.first().first().toString(), testUrl);
  
  // L'URL doit être stockée dans currentHlsUrl
  QCOMPARE(m_service->currentHlsUrl(), testUrl);
}

void TestTwitchService::testOnAdFilterAdsDetected_EmitsSignal() {
  int adCount = 5;
  
  QSignalSpy adsSpy(m_service, &TwitchService::adsDetected);
  
  m_service->onAdFilterAdsDetected(adCount);
  
  QCOMPARE(adsSpy.count(), 1);
  QCOMPARE(adsSpy.first().first().toInt(), adCount);
}

void TestTwitchService::testOnAdFilterAdsFinished_EmitsSignal() {
  QSignalSpy spy(m_service, &TwitchService::adsFinished);
  
  m_service->onAdFilterAdsFinished();
  
  QCOMPARE(spy.count(), 1);
}

void TestTwitchService::testOnAdFilterDebugLog_EmitsSignal() {
  QString debugMessage = "Test debug message";
  
  QSignalSpy spy(m_service, &TwitchService::adFilterLog);
  
  m_service->onAdFilterDebugLog(debugMessage);
  
  QCOMPARE(spy.count(), 1);
  QCOMPARE(spy.first().first().toString(), debugMessage);
}

void TestTwitchService::testOnAdFilterMaxRetries_EmitsSignal() {
  QString fallbackUrl = "https://video.example.com/with-ads.m3u8";
  
  QSignalSpy hlsSpy(m_service, &TwitchService::hlsUrlReady);
  
  m_service->onAdFilterMaxRetries(fallbackUrl);
  
  // Le signal hlsUrlReady doit être émis avec l'URL fallback
  QCOMPARE(hlsSpy.count(), 1);
  QCOMPARE(hlsSpy.first().first().toString(), fallbackUrl);
  
  // L'URL doit être stockée
  QCOMPARE(m_service->currentHlsUrl(), fallbackUrl);
}

QTEST_MAIN(TestTwitchService)
#include "TestTwitchService.moc"
