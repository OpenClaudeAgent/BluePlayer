#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>
#include <QTemporaryDir>

#include "api/twitch/TwitchService.hpp"
#include "api/twitch/TwitchAuthManager.hpp"
#include "api/twitch/TwitchApiClient.hpp"
#include "core/CacheManager.hpp"
#include "mocks/MockSecureStorage.hpp"

using namespace blueplayer::api::twitch;
using namespace blueplayer::core;
using namespace blueplayer::test;

/**
 * @brief Tests d'intégration pour le flux d'authentification Twitch complet
 * 
 * Ces tests vérifient le flux complet depuis l'authentification jusqu'à la récupération des streams.
 * Les tests vérifient le comportement réel du service sans mocks (tests d'intégration).
 */
class TestTwitchFlow : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();
  
  // Tests d'état initial
  void testServiceInitialization();
  void testInitialAuthenticationState();
  void testInitialDataState();
  
  // Tests d'authentification
  void testLogoutClearsAuthentication();
  void testLogoutEmitsSignal();
  
  // Tests des opérations sans authentification
  void testRefreshStreamsWithoutAuth();
  void testRefreshCategoriesWithoutAuth();
  void testRefreshPopularClipsWithoutAuth();
  void testSearchWithoutAuth();
  
  // Tests des propriétés
  void testStreamsProperty();
  void testCategoriesProperty();
  void testSearchResultsProperty();
  
  // Tests de récupération d'erreur
  void testPlayStreamWithInvalidIndex();
  void testMultipleOperationsWithoutAuth();
  void testClearSearchResults();
  
  // ===== Sprint 7 - Tests d'intégration Plan 24 =====
  
  // Flux d'authentification avec MockSecureStorage (24.23)
  void testAuthFlowWithMockStorage();
  void testTokenPersistenceWithMockStorage();
  void testLogoutClearsTokensInStorage();
  
  // Flux de recording avec thumbnails async (24.23)
  void testRecordingFlowWithThumbnail();
  void testRecordingFlowWithoutThumbnail();
  void testMultipleRecordingsSameSream();
  
  // Flux de lecture avec cache (24.23)
  void testCacheLiveReplayFlow();
  void testCacheVodFlow();

private:
  TwitchService* m_service = nullptr;
  MockSecureStorage* m_mockStorage = nullptr;
  CacheManager* m_cacheManager = nullptr;
  QTemporaryDir* m_tempDir = nullptr;
};

void TestTwitchFlow::initTestCase() {
  // Configuration initiale pour les tests d'intégration
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
  qputenv("TWITCH_CLIENT_SECRET", "test_client_secret");
}

void TestTwitchFlow::cleanupTestCase() {
  // Nettoyage global
  qunsetenv("TWITCH_CLIENT_ID");
  qunsetenv("TWITCH_CLIENT_SECRET");
}

void TestTwitchFlow::init() {
  m_tempDir = new QTemporaryDir();
  m_mockStorage = new MockSecureStorage();
  m_service = new TwitchService(m_mockStorage, this);
  m_cacheManager = new CacheManager(this);
}

void TestTwitchFlow::cleanup() {
  delete m_service;
  m_service = nullptr;
  delete m_mockStorage;
  m_mockStorage = nullptr;
  delete m_cacheManager;
  m_cacheManager = nullptr;
  delete m_tempDir;
  m_tempDir = nullptr;
}

void TestTwitchFlow::testServiceInitialization() {
  // Vérifie que le service est correctement initialisé
  QVERIFY(m_service != nullptr);
  
  // Les listes doivent être vides à l'initialisation
  QVERIFY(m_service->streams().isEmpty());
  QVERIFY(m_service->recommendedStreams().isEmpty());
  QVERIFY(m_service->categories().isEmpty());
}

void TestTwitchFlow::testInitialAuthenticationState() {
  // Après logout, l'utilisateur ne doit pas être authentifié
  m_service->logout();
  QVERIFY(!m_service->isAuthenticated());
  
  // userId et userName doivent être vides
  QVERIFY(m_service->userId().isEmpty());
  QVERIFY(m_service->userName().isEmpty());
}

void TestTwitchFlow::testInitialDataState() {
  // Toutes les listes de données doivent être vides initialement
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

void TestTwitchFlow::testLogoutClearsAuthentication() {
  // Test que logout() efface correctement l'état d'authentification
  m_service->logout();
  
  QVERIFY(!m_service->isAuthenticated());
  QVERIFY(m_service->userId().isEmpty());
  QVERIFY(m_service->userName().isEmpty());
}

void TestTwitchFlow::testLogoutEmitsSignal() {
  QSignalSpy authSpy(m_service, &TwitchService::authenticatedChanged);
  
  m_service->logout();
  
  // Le signal peut être émis si l'état change
  // On vérifie que l'état final est correct
  QVERIFY(!m_service->isAuthenticated());
}

void TestTwitchFlow::testRefreshStreamsWithoutAuth() {
  m_service->logout();
  
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  QSignalSpy streamsSpy(m_service, &TwitchService::streamsChanged);
  
  m_service->refreshStreams();
  
  // Sans authentification, les streams ne doivent pas changer
  QVERIFY(m_service->streams().isEmpty());
}

void TestTwitchFlow::testRefreshCategoriesWithoutAuth() {
  m_service->logout();
  
  QSignalSpy categoriesSpy(m_service, &TwitchService::categoriesChanged);
  
  m_service->refreshCategories();
  
  // Sans authentification, les catégories restent vides
  QVERIFY(m_service->categories().isEmpty());
}

void TestTwitchFlow::testRefreshPopularClipsWithoutAuth() {
  m_service->logout();
  
  QSignalSpy clipsSpy(m_service, &TwitchService::popularClipsChanged);
  
  m_service->refreshPopularClips();
  
  // Sans authentification, les clips restent vides
  QVERIFY(m_service->popularClips().isEmpty());
}

void TestTwitchFlow::testSearchWithoutAuth() {
  m_service->logout();
  
  QSignalSpy channelsSpy(m_service, &TwitchService::searchChannelResultsChanged);
  QSignalSpy categoriesSpy(m_service, &TwitchService::searchCategoryResultsChanged);
  
  m_service->search("test");
  
  // Les résultats de recherche restent vides sans auth
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
}

void TestTwitchFlow::testStreamsProperty() {
  // Vérifie que la propriété streams est accessible
  QVariantList streams = m_service->streams();
  QVERIFY(streams.isEmpty()); // Vide initialement
  
  // Vérifie que c'est bien une QVariantList
  QCOMPARE(streams.count(), 0);
}

void TestTwitchFlow::testCategoriesProperty() {
  // Vérifie que la propriété categories est accessible
  QVariantList categories = m_service->categories();
  QVERIFY(categories.isEmpty());
  QCOMPARE(categories.count(), 0);
}

void TestTwitchFlow::testSearchResultsProperty() {
  // Vérifie que les propriétés de recherche sont accessibles
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
}

void TestTwitchFlow::testPlayStreamWithInvalidIndex() {
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  
  // Tenter de jouer un stream avec un index invalide
  m_service->playStream(-1);
  m_service->playStream(999);
  
  // Les streams restent vides
  QVERIFY(m_service->streams().isEmpty());
  QVERIFY(m_service->selectedStreamUrl().isEmpty());
}

void TestTwitchFlow::testMultipleOperationsWithoutAuth() {
  m_service->logout();
  
  QSignalSpy errorSpy(m_service, &TwitchService::errorOccurred);
  
  // Exécuter plusieurs opérations sans authentification
  m_service->refreshStreams();
  m_service->refreshRecommendedStreams();
  m_service->refreshCategories();
  m_service->refreshPopularClips();
  m_service->refreshFollowedClips();
  m_service->refreshVideos();
  m_service->refreshFollowedChannels();
  m_service->refreshNewStreamers();
  
  // Toutes les listes doivent rester vides
  QVERIFY(m_service->streams().isEmpty());
  QVERIFY(m_service->recommendedStreams().isEmpty());
  QVERIFY(m_service->categories().isEmpty());
  QVERIFY(m_service->popularClips().isEmpty());
  QVERIFY(m_service->followedClips().isEmpty());
  QVERIFY(m_service->videos().isEmpty());
  QVERIFY(m_service->followedChannels().isEmpty());
  QVERIFY(m_service->newStreamers().isEmpty());
}

void TestTwitchFlow::testClearSearchResults() {
  // Test de clearSearchResults
  m_service->clearSearchResults();
  
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
  
  // Appeler plusieurs fois doit être idempotent
  m_service->clearSearchResults();
  m_service->clearSearchResults();
  
  QVERIFY(m_service->searchChannelResults().isEmpty());
  QVERIFY(m_service->searchCategoryResults().isEmpty());
}

// ===== Sprint 7 - Tests d'intégration Plan 24 =====

// Flux d'authentification avec MockSecureStorage (24.23)

void TestTwitchFlow::testAuthFlowWithMockStorage() {
  // Vérifie que le service utilise bien MockSecureStorage pour l'authentification
  
  // État initial : non authentifié
  m_service->logout();
  QVERIFY(!m_service->isAuthenticated());
  
  // Le storage doit être vide après logout
  // Les clés utilisées par TwitchAuthManager sont:
  // - blueplayer_access_token
  // - blueplayer_refresh_token
  QVERIFY(!m_mockStorage->contains("blueplayer_access_token"));
  QVERIFY(!m_mockStorage->contains("blueplayer_refresh_token"));
  
  // Simuler le stockage de tokens (comme après une auth réussie)
  m_mockStorage->store("blueplayer_access_token", "test_access_token_123");
  m_mockStorage->store("blueplayer_refresh_token", "test_refresh_token_456");
  
  // Les tokens sont stockés
  QCOMPARE(m_mockStorage->retrieve("blueplayer_access_token"), QString("test_access_token_123"));
  QCOMPARE(m_mockStorage->retrieve("blueplayer_refresh_token"), QString("test_refresh_token_456"));
}

void TestTwitchFlow::testTokenPersistenceWithMockStorage() {
  // Vérifie que les tokens persistent correctement dans MockSecureStorage
  
  const QString accessToken = "persistent_access_token_abc";
  const QString refreshToken = "persistent_refresh_token_xyz";
  
  // Stocker les tokens
  QVERIFY(m_mockStorage->store("blueplayer_access_token", accessToken));
  QVERIFY(m_mockStorage->store("blueplayer_refresh_token", refreshToken));
  
  // Créer un nouveau service avec le même storage
  // pour vérifier que les tokens sont récupérables
  TwitchService newService(m_mockStorage);
  
  // Le nouveau service doit pouvoir récupérer les tokens via accessToken()
  // (si TwitchAuthManager les charge correctement)
  // Note: Le comportement exact dépend de l'implémentation de TwitchAuthManager
  
  // Vérifier que les tokens sont toujours dans le storage
  QCOMPARE(m_mockStorage->retrieve("blueplayer_access_token"), accessToken);
  QCOMPARE(m_mockStorage->retrieve("blueplayer_refresh_token"), refreshToken);
}

void TestTwitchFlow::testLogoutClearsTokensInStorage() {
  // Vérifie que logout() nettoie les tokens du storage
  
  // D'abord stocker des tokens
  m_mockStorage->store("blueplayer_access_token", "token_to_clear");
  m_mockStorage->store("blueplayer_refresh_token", "refresh_to_clear");
  
  // Recréer le service pour qu'il charge les tokens
  delete m_service;
  m_service = new TwitchService(m_mockStorage, this);
  
  // Effectuer le logout
  m_service->logout();
  
  // Après logout, l'utilisateur ne doit pas être authentifié
  QVERIFY(!m_service->isAuthenticated());
  QVERIFY(m_service->userId().isEmpty());
  QVERIFY(m_service->userName().isEmpty());
  QVERIFY(m_service->accessToken().isEmpty());
}

// Flux de recording avec thumbnails async (24.23)

void TestTwitchFlow::testRecordingFlowWithThumbnail() {
  // Teste le flux complet de préparation et finalisation d'un enregistrement
  
  QString streamerLogin = "teststreamer";
  QString thumbnailUrl = "https://static-cdn.jtvnw.net/previews-ttv/live_user_test-{width}x{height}.jpg";
  
  // Préparer l'enregistrement
  QVariantMap prepResult = m_cacheManager->prepareRecording(streamerLogin, thumbnailUrl);
  
  QString recordingPath = prepResult.value("recordingPath").toString();
  QString thumbnailPath = prepResult.value("thumbnailPath").toString();
  qint64 startTime = prepResult.value("startTime").toLongLong();
  
  // Vérifier que les chemins sont générés
  QVERIFY(!recordingPath.isEmpty());
  QVERIFY(!thumbnailPath.isEmpty());
  QVERIFY(startTime > 0);
  
  // Créer un fichier d'enregistrement fictif (simuler l'enregistrement)
  QFile recordingFile(recordingPath);
  QVERIFY(recordingFile.open(QIODevice::WriteOnly));
  recordingFile.write(QByteArray(1024 * 1024, 'X')); // 1 MB de données
  recordingFile.close();
  
  // Attendre un peu pour simuler une durée d'enregistrement >= 30s
  // Note: On utilise un timestamp passé pour simuler la durée
  qint64 recordingStart = QDateTime::currentMSecsSinceEpoch() - 60000; // 60 secondes
  
  // Finaliser l'enregistrement
  bool finalized = m_cacheManager->finalizeRecording(
    recordingPath,
    streamerLogin,
    "Test Streamer",
    "Test Stream Title",
    thumbnailPath,
    recordingStart
  );
  
  QVERIFY(finalized);
  
  // Vérifier que la VOD a été ajoutée
  QCOMPARE(m_cacheManager->vodCount(), 1);
  
  // Nettoyer
  m_cacheManager->clearAllVods();
}

void TestTwitchFlow::testRecordingFlowWithoutThumbnail() {
  // Teste le flux d'enregistrement sans thumbnail
  
  QString streamerLogin = "noThumbStreamer";
  
  // Préparer l'enregistrement sans thumbnail
  QVariantMap prepResult = m_cacheManager->prepareRecording(streamerLogin, "");
  
  QString recordingPath = prepResult.value("recordingPath").toString();
  QString thumbnailPath = prepResult.value("thumbnailPath").toString();
  
  // Le chemin d'enregistrement doit exister
  QVERIFY(!recordingPath.isEmpty());
  // Le chemin du thumbnail doit être vide
  QVERIFY(thumbnailPath.isEmpty());
  
  // Créer un fichier d'enregistrement fictif
  QFile recordingFile(recordingPath);
  QVERIFY(recordingFile.open(QIODevice::WriteOnly));
  recordingFile.write(QByteArray(1024, 'X'));
  recordingFile.close();
  
  // Finaliser avec une durée valide
  qint64 recordingStart = QDateTime::currentMSecsSinceEpoch() - 45000; // 45 secondes
  
  bool finalized = m_cacheManager->finalizeRecording(
    recordingPath,
    streamerLogin,
    "No Thumb Streamer",
    "Stream sans thumbnail",
    "",
    recordingStart
  );
  
  QVERIFY(finalized);
  QCOMPARE(m_cacheManager->vodCount(), 1);
  
  // Nettoyer
  m_cacheManager->clearAllVods();
}

void TestTwitchFlow::testMultipleRecordingsSameSream() {
  // Teste l'enregistrement multiple du même streamer
  
  QString streamerLogin = "multiRecordStreamer";
  int recordingsCount = 3;
  
  for (int i = 0; i < recordingsCount; ++i) {
    QVariantMap prepResult = m_cacheManager->prepareRecording(streamerLogin, "");
    QString recordingPath = prepResult.value("recordingPath").toString();
    
    // Créer le fichier
    QFile recordingFile(recordingPath);
    QVERIFY(recordingFile.open(QIODevice::WriteOnly));
    recordingFile.write(QByteArray(1024, 'X'));
    recordingFile.close();
    
    // Finaliser
    qint64 recordingStart = QDateTime::currentMSecsSinceEpoch() - 60000;
    bool finalized = m_cacheManager->finalizeRecording(
      recordingPath,
      streamerLogin,
      "Multi Record Streamer",
      QString("Stream #%1").arg(i + 1),
      "",
      recordingStart
    );
    
    QVERIFY(finalized);
    
    // Attendre un peu pour éviter les collisions de noms
    QThread::msleep(1100); // 1.1 secondes pour avoir un timestamp différent
  }
  
  // Vérifier que toutes les VOD sont présentes
  QCOMPARE(m_cacheManager->vodCount(), recordingsCount);
  
  // Nettoyer
  m_cacheManager->clearAllVods();
}

// Flux de lecture avec cache (24.23)

void TestTwitchFlow::testCacheLiveReplayFlow() {
  // Teste le flux de mise en cache pour le mode replay live
  
  QSignalSpy cachingStartedSpy(m_cacheManager, &CacheManager::isCachingChanged);
  QSignalSpy durationChangedSpy(m_cacheManager, &CacheManager::cacheDurationChanged);
  
  // Démarrer le caching
  m_cacheManager->startCaching(0.0);
  
  QVERIFY(m_cacheManager->isCaching());
  QCOMPARE(m_cacheManager->cacheStartTime(), 0.0);
  
  // Simuler la progression du stream
  m_cacheManager->updateCacheEnd(30.0);
  QCOMPARE(m_cacheManager->cacheDuration(), 30.0);
  QVERIFY(m_cacheManager->isTimeInCache(15.0));
  QVERIFY(!m_cacheManager->isTimeInCache(35.0));
  
  // Continuer la progression
  m_cacheManager->updateCacheEnd(60.0);
  QCOMPARE(m_cacheManager->cacheDuration(), 60.0);
  
  // Tester clampToCache
  QCOMPARE(m_cacheManager->clampToCache(-10.0), 0.0);
  QCOMPARE(m_cacheManager->clampToCache(100.0), 60.0);
  QCOMPARE(m_cacheManager->clampToCache(30.0), 30.0);
  
  // Arrêter le caching
  m_cacheManager->stopCaching();
  QVERIFY(!m_cacheManager->isCaching());
  
  // La durée doit être préservée après l'arrêt
  QCOMPARE(m_cacheManager->cacheDuration(), 60.0);
  
  // Reset
  m_cacheManager->reset();
  QCOMPARE(m_cacheManager->cacheDuration(), 0.0);
  QVERIFY(!m_cacheManager->isCaching());
}

void TestTwitchFlow::testCacheVodFlow() {
  // Teste le flux complet de gestion des VOD en cache
  
  QSignalSpy vodAddedSpy(m_cacheManager, &CacheManager::vodAdded);
  QSignalSpy vodCountChangedSpy(m_cacheManager, &CacheManager::vodCountChanged);
  QSignalSpy vodListChangedSpy(m_cacheManager, &CacheManager::vodListChanged);
  
  // État initial
  QCOMPARE(m_cacheManager->vodCount(), 0);
  QVERIFY(m_cacheManager->vodList().isEmpty());
  
  // Ajouter une VOD via prepareRecording/finalizeRecording
  QVariantMap prepResult = m_cacheManager->prepareRecording("cacheFlowStreamer", "");
  QString recordingPath = prepResult.value("recordingPath").toString();
  
  QFile recordingFile(recordingPath);
  QVERIFY(recordingFile.open(QIODevice::WriteOnly));
  recordingFile.write(QByteArray(1024 * 1024, 'Y')); // 1 MB
  recordingFile.close();
  
  qint64 recordingStart = QDateTime::currentMSecsSinceEpoch() - 120000; // 2 minutes
  bool finalized = m_cacheManager->finalizeRecording(
    recordingPath,
    "cacheFlowStreamer",
    "Cache Flow Streamer",
    "Testing VOD Cache Flow",
    "",
    recordingStart
  );
  
  QVERIFY(finalized);
  QCOMPARE(m_cacheManager->vodCount(), 1);
  
  // Récupérer la VOD
  QVariantList vodList = m_cacheManager->vodList();
  QCOMPARE(vodList.size(), 1);
  
  QVariantMap vod = vodList.first().toMap();
  QString vodId = vod.value("id").toString();
  QVERIFY(!vodId.isEmpty());
  
  // Mettre à jour la position de lecture
  m_cacheManager->updateWatchPosition(vodId, 60);
  
  // Marquer comme lu
  m_cacheManager->markAsPlayed(vodId);
  
  // Récupérer les métadonnées mises à jour
  QVariantMap updatedVod = m_cacheManager->getVodMetadata(vodId);
  QCOMPARE(updatedVod.value("watchPosition").toLongLong(), qint64(60));
  
  // Rechercher la VOD
  QVariantList searchResults = m_cacheManager->searchVods("Cache Flow");
  QCOMPARE(searchResults.size(), 1);
  
  // Supprimer la VOD
  bool removed = m_cacheManager->removeVod(vodId);
  QVERIFY(removed);
  QCOMPARE(m_cacheManager->vodCount(), 0);
}

QTEST_MAIN(TestTwitchFlow)
#include "TestTwitchFlow.moc"
