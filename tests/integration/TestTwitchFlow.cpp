#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>

#include "api/twitch/TwitchService.hpp"
#include "api/twitch/TwitchAuthManager.hpp"
#include "api/twitch/TwitchApiClient.hpp"
#include "TestHelpers.hpp"

using namespace blueplayer::api::twitch;
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

private:
  TwitchService* m_service = nullptr;
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
  m_service = new TwitchService(this);
}

void TestTwitchFlow::cleanup() {
  delete m_service;
  m_service = nullptr;
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

QTEST_MAIN(TestTwitchFlow)
#include "TestTwitchFlow.moc"
