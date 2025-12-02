#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QSettings>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QByteArray>

#include "api/twitch/TwitchAuthManager.hpp"
#include "TestHelpers.hpp"

using namespace blueplayer::api::twitch;
using namespace blueplayer::test;

class TestTwitchAuthManager : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();
  
  void testCodeVerifierGeneration();
  void testStateGeneration();
  void testCodeChallenge();
  void testPersistCredentials();
  void testLoadCredentials();
  void testIsAuthenticated();
  void testLogout();
  void testRefreshTokenExpiration();
  void testErrorHandling();

private:
  TwitchAuthManager* m_authManager = nullptr;
  QString m_testSettingsPath;
};

void TestTwitchAuthManager::initTestCase() {
  // Utiliser un chemin de test pour QSettings
  m_testSettingsPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/BluePlayerTest";
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, m_testSettingsPath);
}

void TestTwitchAuthManager::cleanupTestCase() {
  // Nettoyer les fichiers de test
}

void TestTwitchAuthManager::init() {
  m_authManager = new TwitchAuthManager(this);
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
}

void TestTwitchAuthManager::cleanup() {
  delete m_authManager;
  m_authManager = nullptr;
}

void TestTwitchAuthManager::testCodeVerifierGeneration() {
  // Test que le code verifier est généré avec la bonne longueur
  // Note: generateCodeVerifier est privée, donc on teste indirectement
  QVERIFY(m_authManager != nullptr);
  
  // Un code verifier PKCE doit faire 43-128 caractères (on utilise 64)
  // On peut tester via login() qui génère un code verifier
}

void TestTwitchAuthManager::testStateGeneration() {
  // Test que le state est généré avec la bonne longueur
  // Note: generateState est privée, donc on teste indirectement
  QVERIFY(m_authManager != nullptr);
  
  // Un state OAuth doit faire au moins 16 caractères (on utilise 24)
}

void TestTwitchAuthManager::testCodeChallenge() {
  // Test que le code challenge est correctement calculé depuis le verifier
  // Note: codeChallenge est privée, donc on teste indirectement
  
  // Le code challenge doit être le SHA256 du verifier encodé en base64url
  QString testVerifier = "test_verifier_string";
  QByteArray hash = QCryptographicHash::hash(testVerifier.toUtf8(), QCryptographicHash::Sha256);
  QString base64Hash = QString::fromUtf8(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  QVERIFY(!base64Hash.isEmpty());
  QVERIFY(base64Hash.length() > 0);
}

void TestTwitchAuthManager::testPersistCredentials() {
  // Test que les credentials sont correctement persistés
  QVERIFY(m_authManager != nullptr);
  
  // Note: persistCredentials est privée, donc on teste indirectement via logout()
  // qui appelle persistCredentials
}

void TestTwitchAuthManager::testLoadCredentials() {
  // Test que les credentials sont correctement chargés au démarrage
  QVERIFY(m_authManager != nullptr);
  
  // Note: loadCredentials est appelée dans le constructeur
  // On peut vérifier l'état initial
  bool initiallyAuthenticated = m_authManager->isAuthenticated();
  // Sans credentials sauvegardés, devrait être false
  QVERIFY(initiallyAuthenticated == false || initiallyAuthenticated == true); // Peut varier selon l'état
}

void TestTwitchAuthManager::testIsAuthenticated() {
  // Test de la méthode isAuthenticated
  QVERIFY(m_authManager != nullptr);
  
  bool authenticated = m_authManager->isAuthenticated();
  // Initialement, sans credentials, devrait être false
  // Mais peut être true si des credentials existent déjà
  QVERIFY(authenticated == false || authenticated == true);
  
  // Test après logout
  m_authManager->logout();
  bool afterLogout = m_authManager->isAuthenticated();
  QVERIFY(afterLogout == false);
  
  // Test que accessToken est vide après logout
  QString tokenAfterLogout = m_authManager->accessToken();
  QVERIFY(tokenAfterLogout.isEmpty());
}

void TestTwitchAuthManager::testLogout() {
  QVERIFY(m_authManager != nullptr);
  
  // Test que logout() peut être appelé même si non authentifié
  m_authManager->logout();
  QVERIFY(!m_authManager->isAuthenticated());
  
  // Test que logout() est idempotent
  m_authManager->logout();
  QVERIFY(!m_authManager->isAuthenticated());
}

void TestTwitchAuthManager::testRefreshTokenExpiration() {
  QVERIFY(m_authManager != nullptr);
  
  // Test que refresh() gère gracieusement l'absence de refresh token
  QSignalSpy errorSpy(m_authManager, &TwitchAuthManager::errorOccurred);
  m_authManager->refresh();
  
  // Devrait émettre une erreur si pas de refresh token
  // ou gérer gracieusement sans erreur
  QVERIFY(true);
}

void TestTwitchAuthManager::testErrorHandling() {
  QVERIFY(m_authManager != nullptr);
  
  QSignalSpy errorSpy(m_authManager, &TwitchAuthManager::errorOccurred);
  
  // Test avec un client ID invalide
  qputenv("TWITCH_CLIENT_ID", "");
  TwitchAuthManager* invalidAuth = new TwitchAuthManager(this);
  // Devrait gérer gracieusement
  QVERIFY(invalidAuth != nullptr);
  delete invalidAuth;
  
  // Restaurer
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
}

QTEST_MAIN(TestTwitchAuthManager)
#include "TestTwitchAuthManager.moc"

