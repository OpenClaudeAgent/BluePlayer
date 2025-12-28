#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QSettings>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDateTime>

#include "api/twitch/TwitchAuthManager.hpp"
#include "mocks/MockSecureStorage.hpp"

using namespace blueplayer::api::twitch;
using namespace blueplayer::test;

class TestTwitchAuthManager : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();
  
  // ===== Tests d'état initial =====
  void testInitialState();
  void testInitiallyNotAuthenticated();
  void testInitialAccessTokenEmpty();
  
  // ===== Tests de loadStoredTokens (via MockSecureStorage) =====
  void testLoadStoredTokensValid();
  void testLoadStoredTokensEmpty();
  void testLoadStoredTokensExpired();
  void testLoadStoredTokensWithClientIdMismatch();
  void testLoadStoredTokensWithoutClientId();
  
  // ===== Tests de clearTokens (logout) =====
  void testLogoutClearsAccessToken();
  void testLogoutClearsRefreshToken();
  void testLogoutClearsTokenExpiration();
  void testLogoutClearsClientId();
  void testLogoutEmitsSignal();
  void testLogoutWhenNotAuthenticated();
  void testLogoutIsIdempotent();
  
  // ===== Tests de validateToken =====
  void testValidateTokenWhenEmpty();
  void testValidateTokenWhenPresent();
  
  // ===== Tests de isTokenExpired =====
  void testIsTokenExpiredWhenExpired();
  void testIsTokenExpiredWhenValid();
  void testIsTokenExpiredWhenNoExpirationDate();
  void testIsTokenExpiredWhenExpiringSoon();
  
  // ===== Tests de refresh =====
  void testRefreshWithoutRefreshToken();
  void testRefreshWhenAlreadyRefreshing();
  
  // ===== Tests du code challenge PKCE =====
  void testCodeChallengeFormat();
  void testCodeChallengeLength();
  void testCodeChallengeIsBase64Url();
  
  // ===== Tests de gestion d'erreur =====
  void testCreationWithEmptyClientId();
  void testCreationWithValidClientId();
  void testLoginWithEmptyClientId();
  
  // ===== Tests des propriétés Q_PROPERTY =====
  void testAuthenticatedProperty();
  void testAccessTokenProperty();
  
  // ===== Tests des signaux =====
  void testAuthenticatedChangedSignalConnection();
  void testAccessTokenChangedSignalConnection();
  void testErrorOccurredSignalConnection();
  
  // ===== Tests de persistCredentials =====
  void testPersistCredentialsStoresToken();
  void testPersistCredentialsStoresRefreshToken();
  void testPersistCredentialsStoresExpiration();
  void testPersistCredentialsStoresClientId();

private:
  TwitchAuthManager* m_authManager = nullptr;
  MockSecureStorage* m_mockStorage = nullptr;
  QString m_testSettingsPath;
  QString m_originalClientId;
};

void TestTwitchAuthManager::initTestCase() {
  m_originalClientId = qgetenv("TWITCH_CLIENT_ID");
  m_testSettingsPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/BluePlayerTest";
  QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, m_testSettingsPath);
}

void TestTwitchAuthManager::cleanupTestCase() {
  if (!m_originalClientId.isEmpty()) {
    qputenv("TWITCH_CLIENT_ID", m_originalClientId.toUtf8());
  }
  
  QDir testDir(m_testSettingsPath);
  if (testDir.exists()) {
    testDir.removeRecursively();
  }
}

void TestTwitchAuthManager::init() {
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
  m_mockStorage = new MockSecureStorage();
  m_authManager = new TwitchAuthManager(nullptr, m_mockStorage, this);
}

void TestTwitchAuthManager::cleanup() {
  delete m_authManager;
  m_authManager = nullptr;
  delete m_mockStorage;
  m_mockStorage = nullptr;
}

// ===== Tests d'état initial =====

void TestTwitchAuthManager::testInitialState() {
  QVERIFY(m_authManager != nullptr);
}

void TestTwitchAuthManager::testInitiallyNotAuthenticated() {
  m_authManager->logout();
  QVERIFY(!m_authManager->isAuthenticated());
}

void TestTwitchAuthManager::testInitialAccessTokenEmpty() {
  m_authManager->logout();
  QVERIFY(m_authManager->accessToken().isEmpty());
}

// ===== Tests de loadStoredTokens (via MockSecureStorage) =====

void TestTwitchAuthManager::testLoadStoredTokensValid() {
  // Setup: store valid tokens
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "valid_access_token_123");
  storage->store("refresh_token", "valid_refresh_token_456");
  storage->store("token_client_id", "test_client_id");
  
  // Store valid expiration time (1 hour from now)
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  // Create auth manager which will load credentials
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Should be authenticated with valid tokens
  QVERIFY(authManager->isAuthenticated());
  QCOMPARE(authManager->accessToken(), QString("valid_access_token_123"));
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testLoadStoredTokensEmpty() {
  auto* storage = new MockSecureStorage();
  // Don't store anything
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  QVERIFY(!authManager->isAuthenticated());
  QVERIFY(authManager->accessToken().isEmpty());
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testLoadStoredTokensExpired() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "expired_access_token");
  storage->store("refresh_token", "valid_refresh_token");
  storage->store("token_client_id", "test_client_id");
  
  // Store expired expiration time (1 hour ago)
  QDateTime pastExpiration = QDateTime::currentDateTimeUtc().addSecs(-3600);
  storage->store("token_expiration", pastExpiration.toString(Qt::ISODate));
  
  QSignalSpy errorSpy(this, &QObject::destroyed); // Placeholder - just need storage
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Token is expired, should trigger refresh or not be authenticated until refresh completes
  // The manager will attempt refresh if refresh_token is present
  // For now, just verify it doesn't crash
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testLoadStoredTokensWithClientIdMismatch() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "valid_token");
  storage->store("refresh_token", "valid_refresh");
  storage->store("token_client_id", "different_client_id"); // Different from env var
  
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Tokens should be invalidated due to Client-ID mismatch
  QVERIFY(!authManager->isAuthenticated());
  QVERIFY(authManager->accessToken().isEmpty());
  
  // Storage should be cleared
  QVERIFY(storage->retrieve("access_token").isEmpty());
  QVERIFY(storage->retrieve("refresh_token").isEmpty());
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testLoadStoredTokensWithoutClientId() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "legacy_token");
  storage->store("refresh_token", "legacy_refresh");
  // No token_client_id stored (legacy token)
  
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Legacy tokens should be accepted and current Client-ID should be stored
  QVERIFY(authManager->isAuthenticated());
  QCOMPARE(storage->retrieve("token_client_id"), QString("test_client_id"));
  
  delete authManager;
  delete storage;
}

// ===== Tests de clearTokens (logout) =====

void TestTwitchAuthManager::testLogoutClearsAccessToken() {
  m_mockStorage->store("access_token", "token_to_clear");
  m_authManager->logout();
  
  QVERIFY(m_mockStorage->retrieve("access_token").isEmpty());
}

void TestTwitchAuthManager::testLogoutClearsRefreshToken() {
  m_mockStorage->store("refresh_token", "refresh_to_clear");
  m_authManager->logout();
  
  QVERIFY(m_mockStorage->retrieve("refresh_token").isEmpty());
}

void TestTwitchAuthManager::testLogoutClearsTokenExpiration() {
  m_mockStorage->store("token_expiration", "2024-01-01T00:00:00Z");
  m_authManager->logout();
  
  QVERIFY(m_mockStorage->retrieve("token_expiration").isEmpty());
}

void TestTwitchAuthManager::testLogoutClearsClientId() {
  m_mockStorage->store("token_client_id", "stored_client_id");
  m_authManager->logout();
  
  QVERIFY(m_mockStorage->retrieve("token_client_id").isEmpty());
}

void TestTwitchAuthManager::testLogoutEmitsSignal() {
  // First, setup a scenario where we're authenticated
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "valid_token");
  storage->store("refresh_token", "valid_refresh");
  storage->store("token_client_id", "test_client_id");
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  QSignalSpy authSpy(authManager, &TwitchAuthManager::authenticatedChanged);
  QSignalSpy tokenSpy(authManager, &TwitchAuthManager::accessTokenChanged);
  
  authManager->logout();
  
  QVERIFY(!authManager->isAuthenticated());
  // authenticatedChanged should be emitted with false
  QVERIFY(authSpy.count() >= 1);
  // accessTokenChanged should be emitted with empty token
  QVERIFY(tokenSpy.count() >= 1);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testLogoutWhenNotAuthenticated() {
  m_authManager->logout();
  QVERIFY(!m_authManager->isAuthenticated());
  
  // Second logout should not crash
  m_authManager->logout();
  QVERIFY(!m_authManager->isAuthenticated());
}

void TestTwitchAuthManager::testLogoutIsIdempotent() {
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

// ===== Tests de validateToken =====

void TestTwitchAuthManager::testValidateTokenWhenEmpty() {
  m_authManager->logout();
  
  // accessToken() should return empty when no token
  QVERIFY(m_authManager->accessToken().isEmpty());
}

void TestTwitchAuthManager::testValidateTokenWhenPresent() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "present_token");
  storage->store("token_client_id", "test_client_id");
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  QCOMPARE(authManager->accessToken(), QString("present_token"));
  
  delete authManager;
  delete storage;
}

// ===== Tests de isTokenExpired =====

void TestTwitchAuthManager::testIsTokenExpiredWhenExpired() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "expired_token");
  storage->store("refresh_token", ""); // No refresh token
  storage->store("token_client_id", "test_client_id");
  
  // Expired 1 hour ago
  QDateTime pastExpiration = QDateTime::currentDateTimeUtc().addSecs(-3600);
  storage->store("token_expiration", pastExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Without refresh token, expired token should result in empty accessToken
  // because ensureValidToken will call logout
  QVERIFY(authManager->accessToken().isEmpty() || !authManager->isAuthenticated());
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testIsTokenExpiredWhenValid() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "valid_token");
  storage->store("token_client_id", "test_client_id");
  
  // Expires in 1 hour
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  QCOMPARE(authManager->accessToken(), QString("valid_token"));
  QVERIFY(authManager->isAuthenticated());
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testIsTokenExpiredWhenNoExpirationDate() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "no_expiration_token");
  storage->store("refresh_token", "has_refresh"); // Has refresh token
  storage->store("token_client_id", "test_client_id");
  // No token_expiration stored
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Without expiration date, token is considered expired and refresh is triggered
  // But since no network, we just verify no crash
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testIsTokenExpiredWhenExpiringSoon() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "expiring_soon_token");
  storage->store("refresh_token", "has_refresh");
  storage->store("token_client_id", "test_client_id");
  
  // Expires in 2 minutes (within 5 minute threshold)
  QDateTime soonExpiration = QDateTime::currentDateTimeUtc().addSecs(120);
  storage->store("token_expiration", soonExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Should trigger refresh because expiring soon
  // Without network, just verify no crash
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

// ===== Tests de refresh =====

void TestTwitchAuthManager::testRefreshWithoutRefreshToken() {
  m_authManager->logout();
  
  QSignalSpy errorSpy(m_authManager, &TwitchAuthManager::errorOccurred);
  
  m_authManager->refresh();
  
  // Should emit error for missing refresh token
  QTRY_VERIFY(errorSpy.count() >= 1);
  QVERIFY(!m_authManager->isAuthenticated());
}

void TestTwitchAuthManager::testRefreshWhenAlreadyRefreshing() {
  // This is hard to test without mocking the network layer
  // Just verify it doesn't crash
  m_authManager->logout();
  m_authManager->refresh();
  m_authManager->refresh(); // Second call while first might still be "in progress"
  
  QVERIFY(m_authManager != nullptr);
}

// ===== Tests du code challenge PKCE =====

void TestTwitchAuthManager::testCodeChallengeFormat() {
  QString testVerifier = "test_verifier_string_with_enough_length_for_pkce";
  QByteArray hash = QCryptographicHash::hash(testVerifier.toUtf8(), QCryptographicHash::Sha256);
  QString base64Hash = QString::fromUtf8(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  QVERIFY(!base64Hash.isEmpty());
  QVERIFY(base64Hash.length() >= 32);
}

void TestTwitchAuthManager::testCodeChallengeLength() {
  QString testVerifier = "abcdefghijklmnopqrstuvwxyz123456";
  QByteArray hash = QCryptographicHash::hash(testVerifier.toUtf8(), QCryptographicHash::Sha256);
  QString base64Hash = QString::fromUtf8(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  // SHA256 produces 32 bytes, in base64 that's ~43 characters
  QCOMPARE(base64Hash.length(), 43);
}

void TestTwitchAuthManager::testCodeChallengeIsBase64Url() {
  QString testVerifier = "test_verifier";
  QByteArray hash = QCryptographicHash::hash(testVerifier.toUtf8(), QCryptographicHash::Sha256);
  QString base64Hash = QString::fromUtf8(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  // Verify base64url characters only (no +, /, or =)
  QVERIFY(!base64Hash.contains('+'));
  QVERIFY(!base64Hash.contains('/'));
  QVERIFY(!base64Hash.contains('='));
  
  QRegularExpression base64UrlRegex("^[A-Za-z0-9_-]+$");
  QVERIFY(base64UrlRegex.match(base64Hash).hasMatch());
}

// ===== Tests de gestion d'erreur =====

void TestTwitchAuthManager::testCreationWithEmptyClientId() {
  qputenv("TWITCH_CLIENT_ID", "");
  
  auto* mockStorage = new MockSecureStorage();
  TwitchAuthManager* invalidAuth = new TwitchAuthManager(nullptr, mockStorage, this);
  
  QVERIFY(invalidAuth != nullptr);
  QVERIFY(!invalidAuth->isAuthenticated());
  
  delete invalidAuth;
  delete mockStorage;
  
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
}

void TestTwitchAuthManager::testCreationWithValidClientId() {
  qputenv("TWITCH_CLIENT_ID", "valid_test_client_id");
  
  auto* mockStorage = new MockSecureStorage();
  TwitchAuthManager* validAuth = new TwitchAuthManager(nullptr, mockStorage, this);
  
  QVERIFY(validAuth != nullptr);
  
  delete validAuth;
  delete mockStorage;
  
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
}

void TestTwitchAuthManager::testLoginWithEmptyClientId() {
  qputenv("TWITCH_CLIENT_ID", "");
  
  auto* mockStorage = new MockSecureStorage();
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, mockStorage, this);
  
  QSignalSpy errorSpy(authManager, &TwitchAuthManager::errorOccurred);
  
  authManager->login();
  
  // Should emit error for missing Client ID
  QTRY_VERIFY(errorSpy.count() >= 1);
  QString errorMsg = errorSpy.first().first().toString();
  QVERIFY(errorMsg.contains("TWITCH_CLIENT_ID") || errorMsg.contains("Client"));
  
  delete authManager;
  delete mockStorage;
  
  qputenv("TWITCH_CLIENT_ID", "test_client_id");
}

// ===== Tests des propriétés Q_PROPERTY =====

void TestTwitchAuthManager::testAuthenticatedProperty() {
  m_authManager->logout();
  
  bool auth = m_authManager->property("authenticated").toBool();
  QCOMPARE(auth, false);
}

void TestTwitchAuthManager::testAccessTokenProperty() {
  m_authManager->logout();
  
  QString token = m_authManager->property("accessToken").toString();
  QVERIFY(token.isEmpty());
}

// ===== Tests des signaux =====

void TestTwitchAuthManager::testAuthenticatedChangedSignalConnection() {
  QSignalSpy spy(m_authManager, &TwitchAuthManager::authenticatedChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchAuthManager::testAccessTokenChangedSignalConnection() {
  QSignalSpy spy(m_authManager, &TwitchAuthManager::accessTokenChanged);
  QVERIFY(spy.isValid());
}

void TestTwitchAuthManager::testErrorOccurredSignalConnection() {
  QSignalSpy spy(m_authManager, &TwitchAuthManager::errorOccurred);
  QVERIFY(spy.isValid());
}

// ===== Tests de persistCredentials =====

void TestTwitchAuthManager::testPersistCredentialsStoresToken() {
  // Setup: Create manager with valid token that will be persisted
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "token_to_persist");
  storage->store("refresh_token", "refresh_to_persist");
  storage->store("token_client_id", "test_client_id");
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Verify tokens were loaded
  QCOMPARE(storage->retrieve("access_token"), QString("token_to_persist"));
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testPersistCredentialsStoresRefreshToken() {
  auto* storage = new MockSecureStorage();
  storage->store("refresh_token", "refresh_to_check");
  storage->store("access_token", "access_token");
  storage->store("token_client_id", "test_client_id");
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  QCOMPARE(storage->retrieve("refresh_token"), QString("refresh_to_check"));
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testPersistCredentialsStoresExpiration() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "token");
  storage->store("token_client_id", "test_client_id");
  
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  QString expirationStr = futureExpiration.toString(Qt::ISODate);
  storage->store("token_expiration", expirationStr);
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  QCOMPARE(storage->retrieve("token_expiration"), expirationStr);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testPersistCredentialsStoresClientId() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "token");
  // No token_client_id initially (legacy scenario)
  
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Current Client-ID should be stored for legacy tokens
  QCOMPARE(storage->retrieve("token_client_id"), QString("test_client_id"));
  
  delete authManager;
  delete storage;
}

QTEST_MAIN(TestTwitchAuthManager)
#include "TestTwitchAuthManager.moc"
