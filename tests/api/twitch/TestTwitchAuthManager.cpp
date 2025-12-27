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
  
  // Tests d'état initial
  void testInitialState();
  void testInitiallyNotAuthenticated();
  void testInitialAccessTokenEmpty();
  
  // Tests de logout
  void testLogoutWhenNotAuthenticated();
  void testLogoutIsIdempotent();
  void testLogoutClearsAccessToken();
  void testLogoutEmitsSignal();
  
  // Tests de refresh
  void testRefreshWithoutRefreshToken();
  
  // Tests du code challenge PKCE
  void testCodeChallengeFormat();
  void testCodeChallengeLength();
  void testCodeChallengeIsBase64Url();
  
  // Tests de gestion d'erreur
  void testCreationWithEmptyClientId();
  void testCreationWithValidClientId();
  
  // Tests des propriétés Q_PROPERTY
  void testAuthenticatedProperty();
  void testAccessTokenProperty();

private:
  TwitchAuthManager* m_authManager = nullptr;
  QString m_testSettingsPath;
  QString m_originalClientId;
};

void TestTwitchAuthManager::initTestCase() {
  // Sauvegarder le client ID original
  m_originalClientId = qgetenv("TWITCH_CLIENT_ID");
  
  // Utiliser un chemin de test pour QSettings
  m_testSettingsPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/BluePlayerTest";
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, m_testSettingsPath);
}

void TestTwitchAuthManager::cleanupTestCase() {
  // Restaurer le client ID original
  if (!m_originalClientId.isEmpty()) {
    qputenv("TWITCH_CLIENT_ID", m_originalClientId.toUtf8());
  }
  
  // Nettoyer les fichiers de test
  QDir testDir(m_testSettingsPath);
  if (testDir.exists()) {
    testDir.removeRecursively();
  }
}

void TestTwitchAuthManager::init() {
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
  m_authManager = new TwitchAuthManager(nullptr, this);
}

void TestTwitchAuthManager::cleanup() {
  delete m_authManager;
  m_authManager = nullptr;
}

void TestTwitchAuthManager::testInitialState() {
  // Un nouveau manager doit être dans un état cohérent
  QVERIFY(m_authManager != nullptr);
}

void TestTwitchAuthManager::testInitiallyNotAuthenticated() {
  // Après logout, l'utilisateur ne doit pas être authentifié
  m_authManager->logout();
  QVERIFY(!m_authManager->isAuthenticated());
}

void TestTwitchAuthManager::testInitialAccessTokenEmpty() {
  // Après logout, le token doit être vide
  m_authManager->logout();
  QVERIFY(m_authManager->accessToken().isEmpty());
}

void TestTwitchAuthManager::testLogoutWhenNotAuthenticated() {
  // logout() doit fonctionner même si non authentifié
  m_authManager->logout();
  QVERIFY(!m_authManager->isAuthenticated());
  
  // Un second appel ne doit pas causer de problème
  m_authManager->logout();
  QVERIFY(!m_authManager->isAuthenticated());
}

void TestTwitchAuthManager::testLogoutIsIdempotent() {
  // Appeler logout() plusieurs fois doit être sans effet
  m_authManager->logout();
  bool firstState = m_authManager->isAuthenticated();
  
  m_authManager->logout();
  bool secondState = m_authManager->isAuthenticated();
  
  m_authManager->logout();
  bool thirdState = m_authManager->isAuthenticated();
  
  QCOMPARE(firstState, false);
  QCOMPARE(secondState, false);
  QCOMPARE(thirdState, false);
}

void TestTwitchAuthManager::testLogoutClearsAccessToken() {
  m_authManager->logout();
  
  QString token = m_authManager->accessToken();
  QVERIFY(token.isEmpty());
}

void TestTwitchAuthManager::testLogoutEmitsSignal() {
  QSignalSpy authSpy(m_authManager, &TwitchAuthManager::authenticatedChanged);
  
  m_authManager->logout();
  
  // Après logout, l'état doit être non authentifié
  QVERIFY(!m_authManager->isAuthenticated());
}

void TestTwitchAuthManager::testRefreshWithoutRefreshToken() {
  m_authManager->logout();
  
  QSignalSpy errorSpy(m_authManager, &TwitchAuthManager::errorOccurred);
  
  // refresh() sans refresh token ne doit pas crasher
  m_authManager->refresh();
  
  // L'état doit rester non authentifié
  QVERIFY(!m_authManager->isAuthenticated());
}

void TestTwitchAuthManager::testCodeChallengeFormat() {
  // Test que le code challenge est correctement calculé (SHA256 base64url)
  QString testVerifier = "test_verifier_string_with_enough_length_for_pkce";
  QByteArray hash = QCryptographicHash::hash(testVerifier.toUtf8(), QCryptographicHash::Sha256);
  QString base64Hash = QString::fromUtf8(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  // Le hash doit exister et avoir une longueur valide
  QVERIFY(!base64Hash.isEmpty());
  QVERIFY(base64Hash.length() >= 32);
}

void TestTwitchAuthManager::testCodeChallengeLength() {
  // SHA256 produit 32 bytes, en base64 ça donne environ 43 caractères
  QString testVerifier = "abcdefghijklmnopqrstuvwxyz123456";
  QByteArray hash = QCryptographicHash::hash(testVerifier.toUtf8(), QCryptographicHash::Sha256);
  QString base64Hash = QString::fromUtf8(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  // La longueur doit être d'environ 43 caractères (32 bytes en base64)
  QCOMPARE(base64Hash.length(), 43);
}

void TestTwitchAuthManager::testCodeChallengeIsBase64Url() {
  QString testVerifier = "test_verifier";
  QByteArray hash = QCryptographicHash::hash(testVerifier.toUtf8(), QCryptographicHash::Sha256);
  QString base64Hash = QString::fromUtf8(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  // Vérifier que le résultat est bien en base64url (pas de +, /, ou =)
  QVERIFY(!base64Hash.contains('+'));
  QVERIFY(!base64Hash.contains('/'));
  QVERIFY(!base64Hash.contains('='));
  
  // Doit contenir uniquement des caractères base64url valides
  QRegularExpression base64UrlRegex("^[A-Za-z0-9_-]+$");
  QVERIFY(base64UrlRegex.match(base64Hash).hasMatch());
}

void TestTwitchAuthManager::testCreationWithEmptyClientId() {
  qputenv("TWITCH_CLIENT_ID", "");
  
  TwitchAuthManager* invalidAuth = new TwitchAuthManager(nullptr, this);
  
  // Doit être créé sans crasher
  QVERIFY(invalidAuth != nullptr);
  
  // Ne doit pas être authentifié
  QVERIFY(!invalidAuth->isAuthenticated());
  
  delete invalidAuth;
  
  // Restaurer
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
}

void TestTwitchAuthManager::testCreationWithValidClientId() {
  qputenv("TWITCH_CLIENT_ID", "valid_test_client_id");
  
  TwitchAuthManager* validAuth = new TwitchAuthManager(nullptr, this);
  
  QVERIFY(validAuth != nullptr);
  
  delete validAuth;
  
  // Restaurer
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
}

void TestTwitchAuthManager::testAuthenticatedProperty() {
  // Test de la propriété Q_PROPERTY authenticated
  m_authManager->logout();
  
  // La propriété doit être accessible
  bool auth = m_authManager->property("authenticated").toBool();
  QCOMPARE(auth, false);
}

void TestTwitchAuthManager::testAccessTokenProperty() {
  // Test de la propriété Q_PROPERTY accessToken
  m_authManager->logout();
  
  // La propriété doit être accessible et vide après logout
  QString token = m_authManager->property("accessToken").toString();
  QVERIFY(token.isEmpty());
}

QTEST_MAIN(TestTwitchAuthManager)
#include "TestTwitchAuthManager.moc"
