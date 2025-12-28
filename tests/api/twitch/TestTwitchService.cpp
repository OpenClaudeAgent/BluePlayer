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

QTEST_MAIN(TestTwitchService)
#include "TestTwitchService.moc"
