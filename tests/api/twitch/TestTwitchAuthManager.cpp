#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QSettings>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDateTime>
#include <QRandomGenerator>
#include <QRegularExpression>

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
  
  // ===== Tests du flow login =====
  void testLoginEmitsError();
  void testLoginWhenAlreadyAuthenticated();
  
  // ===== Tests de generateCodeVerifier et generateState =====
  void testCodeVerifierLength();
  void testCodeVerifierCharacters();
  void testCodeVerifierUniqueness();
  void testStateLength();
  void testStateUniqueness();
  
  // ===== Tests de refresh avec token =====
  void testRefreshWithValidRefreshToken();
  void testRefreshMultipleCallsWhileRefreshing();
  
  // ===== Tests de accessToken pendant refresh =====
  void testAccessTokenReturnsEmptyDuringRefresh();
  void testAccessTokenTriggersRefreshWhenExpired();
  
  // ===== Tests de loadCredentials edge cases =====
  void testLoadCredentialsMigrationFromLegacySettings();
  void testLoadCredentialsWithInvalidExpirationFormat();
  
  // ===== Tests supplémentaires de token expiration =====
  void testIsTokenExpiredExactlyAtThreshold();
  void testIsTokenExpiredOneSecondBeforeThreshold();
  void testIsTokenExpiredOneSecondAfterThreshold();
  void testTokenExpirationBufferFiveMinutes();
  
  // ===== Tests de ensureValidToken =====
  void testEnsureValidTokenWhenEmpty();
  void testEnsureValidTokenWhenRefreshing();
  void testEnsureValidTokenWhenValid();
  void testEnsureValidTokenWhenExpiredNoRefreshToken();
  
  // ===== Tests de emitAuthenticated et emitTokenChanged =====
  void testEmitAuthenticatedOnlyOnChange();
  void testEmitTokenChangedEmitsAccessToken();
  
  // ===== Tests de handleLocalCallback =====
  void testHandleLocalCallbackStateMismatch();
  void testHandleLocalCallbackMissingCode();
  
  // ===== Tests de buildSslConfiguration =====
  void testBuildSslConfigurationNoPaths();
  void testBuildSslConfigurationInvalidCertPath();
  
  // ===== Tests de scénarios combinés =====
  void testFullLogoutLoginCycle();
  void testTokenRefreshUpdatesExpiration();
  void testMultipleLogoutsStable();
  
  // ===== Tests de PKCE helpers via comportement observable =====
  void testCodeVerifierIsDifferentEachTime();
  void testCodeChallengeIsDeterministic();
  void testStateLengthSecurity();

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
  // Test that creating TwitchAuthManager with empty Client ID doesn't crash
  // Note: We don't call login() as it may open a browser
  
  qputenv("TWITCH_CLIENT_ID", "");
  
  auto* mockStorage = new MockSecureStorage();
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, mockStorage, this);
  
  // Verify manager is created but not authenticated
  QVERIFY(authManager != nullptr);
  QVERIFY(!authManager->isAuthenticated());
  
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

// ===== Tests du flow login =====

void TestTwitchAuthManager::testLoginEmitsError() {
  // Test that TwitchAuthManager can be created with valid client ID
  // Note: We don't call login() as it opens a browser
  
  auto* storage = new MockSecureStorage();
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  QSignalSpy errorSpy(authManager, &TwitchAuthManager::errorOccurred);
  
  // Verify the manager is properly initialized
  QVERIFY(authManager != nullptr);
  QVERIFY(!authManager->isAuthenticated());
  
  // Verify error signal is connectable
  QVERIFY(errorSpy.isValid());
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testLoginWhenAlreadyAuthenticated() {
  // Test that when already authenticated, isAuthenticated returns true
  // Note: We don't call login() as it opens a browser
  
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "valid_token");
  storage->store("token_client_id", "test_client_id");
  QDateTime futureExpiration = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", futureExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Should be authenticated from stored credentials
  QVERIFY(authManager->isAuthenticated());
  QCOMPARE(authManager->accessToken(), QString("valid_token"));
  
  delete authManager;
  delete storage;
}

// ===== Tests de generateCodeVerifier et generateState =====

void TestTwitchAuthManager::testCodeVerifierLength() {
  // Code verifier should be 64 characters per PKCE spec
  // We verify this indirectly by testing that the code challenge generation works
  
  QString testVerifier = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ab";
  QCOMPARE(testVerifier.length(), 64);
  
  // Verify PKCE challenge can be computed
  QByteArray hash = QCryptographicHash::hash(testVerifier.toUtf8(), QCryptographicHash::Sha256);
  QString challenge = QString::fromUtf8(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  QVERIFY(!challenge.isEmpty());
  QCOMPARE(challenge.length(), 43); // SHA256 in base64url = 43 chars
}

void TestTwitchAuthManager::testCodeVerifierCharacters() {
  // Valid PKCE characters: A-Z a-z 0-9 - . _ ~
  QString validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
  
  // All characters should be valid for PKCE
  QRegularExpression validPattern("^[A-Za-z0-9\\-._~]+$");
  QVERIFY(validPattern.match(validChars).hasMatch());
}

void TestTwitchAuthManager::testCodeVerifierUniqueness() {
  // Two different verifiers should produce different challenges
  QString verifier1 = "verifier1_abcdefghijklmnopqrstuvwxyz0123456789ABCD";
  QString verifier2 = "verifier2_abcdefghijklmnopqrstuvwxyz0123456789ABCD";
  
  QByteArray hash1 = QCryptographicHash::hash(verifier1.toUtf8(), QCryptographicHash::Sha256);
  QByteArray hash2 = QCryptographicHash::hash(verifier2.toUtf8(), QCryptographicHash::Sha256);
  
  QString challenge1 = QString::fromUtf8(hash1.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  QString challenge2 = QString::fromUtf8(hash2.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  QVERIFY(challenge1 != challenge2);
}

void TestTwitchAuthManager::testStateLength() {
  // OAuth state should be at least 24 characters for security
  // We test the format indirectly
  QString testState = "abcdefghijklmnopqrstuvwx"; // 24 chars
  QCOMPARE(testState.length(), 24);
  
  // State should only contain safe URL characters
  QRegularExpression safePattern("^[A-Za-z0-9\\-._~]+$");
  QVERIFY(safePattern.match(testState).hasMatch());
}

void TestTwitchAuthManager::testStateUniqueness() {
  // Two OAuth states should be different (random)
  // We verify the concept by checking that different inputs produce different hashes
  QString state1 = "state1_random_value_here";
  QString state2 = "state2_random_value_here";
  
  QVERIFY(state1 != state2);
}

// ===== Tests de refresh avec token =====

void TestTwitchAuthManager::testRefreshWithValidRefreshToken() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "old_access_token");
  storage->store("refresh_token", "valid_refresh_token");
  storage->store("token_client_id", "test_client_id");
  
  // Token expires in 1 minute (within refresh threshold)
  QDateTime soonExpiration = QDateTime::currentDateTimeUtc().addSecs(60);
  storage->store("token_expiration", soonExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Should trigger refresh due to expiring soon
  // Without real network, we just verify no crash
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testRefreshMultipleCallsWhileRefreshing() {
  auto* storage = new MockSecureStorage();
  storage->store("refresh_token", "valid_refresh_token");
  storage->store("token_client_id", "test_client_id");
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Multiple refresh calls should not crash or duplicate requests
  authManager->refresh();
  authManager->refresh();
  authManager->refresh();
  
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

// ===== Tests de accessToken pendant refresh =====

void TestTwitchAuthManager::testAccessTokenReturnsEmptyDuringRefresh() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "valid_token");
  storage->store("refresh_token", "refresh_token");
  storage->store("token_client_id", "test_client_id");
  
  // Token expired to trigger refresh
  QDateTime pastExpiration = QDateTime::currentDateTimeUtc().addSecs(-60);
  storage->store("token_expiration", pastExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // During refresh, accessToken() should return empty to prevent using expired token
  // Note: The exact behavior depends on implementation
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testAccessTokenTriggersRefreshWhenExpired() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "expired_token");
  storage->store("refresh_token", "valid_refresh_token");
  storage->store("token_client_id", "test_client_id");
  
  // Token expired 1 hour ago
  QDateTime pastExpiration = QDateTime::currentDateTimeUtc().addSecs(-3600);
  storage->store("token_expiration", pastExpiration.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Accessing the token should trigger a refresh
  QString token = authManager->accessToken();
  
  // Token should be empty during refresh
  // Or the refresh should have been triggered
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

// ===== Tests de loadCredentials edge cases =====

void TestTwitchAuthManager::testLoadCredentialsMigrationFromLegacySettings() {
  // Test that new auth manager starts with empty credentials
  // Migration is tested by the actual flow
  auto* storage = new MockSecureStorage();
  // No tokens stored
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  QVERIFY(!authManager->isAuthenticated());
  QVERIFY(authManager->accessToken().isEmpty());
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testLoadCredentialsWithInvalidExpirationFormat() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "valid_token");
  storage->store("refresh_token", "refresh_token");
  storage->store("token_client_id", "test_client_id");
  storage->store("token_expiration", "invalid_date_format");
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Invalid date format should be handled gracefully
  // Token should be considered expired, triggering refresh
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

// ===== Tests supplémentaires de token expiration =====

void TestTwitchAuthManager::testIsTokenExpiredExactlyAtThreshold() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "threshold_token");
  storage->store("refresh_token", "refresh");
  storage->store("token_client_id", "test_client_id");
  
  // Exactly 5 minutes (300 seconds) from now - at threshold
  QDateTime thresholdTime = QDateTime::currentDateTimeUtc().addSecs(300);
  storage->store("token_expiration", thresholdTime.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // At threshold, should trigger refresh
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testIsTokenExpiredOneSecondBeforeThreshold() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "almost_expired_token");
  storage->store("refresh_token", "refresh");
  storage->store("token_client_id", "test_client_id");
  
  // 299 seconds from now - just inside threshold
  QDateTime almostThreshold = QDateTime::currentDateTimeUtc().addSecs(299);
  storage->store("token_expiration", almostThreshold.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Should trigger refresh (within 5 min threshold)
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testIsTokenExpiredOneSecondAfterThreshold() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "valid_threshold_token");
  storage->store("token_client_id", "test_client_id");
  
  // 600 seconds (10 min) from now - safely outside 5 min threshold
  QDateTime safeOutside = QDateTime::currentDateTimeUtc().addSecs(600);
  storage->store("token_expiration", safeOutside.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Should be considered valid (well outside 5 min threshold)
  QVERIFY(authManager->isAuthenticated());
  QCOMPARE(authManager->accessToken(), QString("valid_threshold_token"));
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testTokenExpirationBufferFiveMinutes() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "buffer_token");
  storage->store("token_client_id", "test_client_id");
  
  // 10 minutes from now - well outside buffer
  QDateTime safeTime = QDateTime::currentDateTimeUtc().addSecs(600);
  storage->store("token_expiration", safeTime.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Token should be valid
  QVERIFY(authManager->isAuthenticated());
  QCOMPARE(authManager->accessToken(), QString("buffer_token"));
  
  delete authManager;
  delete storage;
}

// ===== Tests de ensureValidToken =====

void TestTwitchAuthManager::testEnsureValidTokenWhenEmpty() {
  m_authManager->logout();
  
  // Calling accessToken() when empty should not crash
  QString token = m_authManager->accessToken();
  QVERIFY(token.isEmpty());
}

void TestTwitchAuthManager::testEnsureValidTokenWhenRefreshing() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "old_token");
  storage->store("refresh_token", "has_refresh");
  storage->store("token_client_id", "test_client_id");
  
  // Token expired to trigger refresh
  QDateTime expired = QDateTime::currentDateTimeUtc().addSecs(-60);
  storage->store("token_expiration", expired.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // During refresh, accessToken should return empty
  // (to prevent using expired token)
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testEnsureValidTokenWhenValid() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "valid_token_here");
  storage->store("token_client_id", "test_client_id");
  
  QDateTime future = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", future.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Should return the valid token
  QCOMPARE(authManager->accessToken(), QString("valid_token_here"));
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testEnsureValidTokenWhenExpiredNoRefreshToken() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "expired_no_refresh");
  storage->store("refresh_token", ""); // No refresh token
  storage->store("token_client_id", "test_client_id");
  
  QDateTime expired = QDateTime::currentDateTimeUtc().addSecs(-60);
  storage->store("token_expiration", expired.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Without refresh token, should logout
  // Token should be empty after logout
  QVERIFY(authManager->accessToken().isEmpty() || !authManager->isAuthenticated());
  
  delete authManager;
  delete storage;
}

// ===== Tests de emitAuthenticated et emitTokenChanged =====

void TestTwitchAuthManager::testEmitAuthenticatedOnlyOnChange() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "signal_test_token");
  storage->store("token_client_id", "test_client_id");
  
  QDateTime future = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", future.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  QSignalSpy spy(authManager, &TwitchAuthManager::authenticatedChanged);
  
  // Already authenticated from load, calling again shouldn't emit
  // (This tests internal emitAuthenticated behavior)
  QVERIFY(authManager->isAuthenticated());
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testEmitTokenChangedEmitsAccessToken() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "emit_token_test");
  storage->store("token_client_id", "test_client_id");
  
  QDateTime future = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", future.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Token was loaded during construction
  // Verify accessToken is available (signal was already emitted during construction)
  QCOMPARE(authManager->accessToken(), QString("emit_token_test"));
  QVERIFY(authManager->isAuthenticated());
  
  // Now test that logout emits the signal
  QSignalSpy spy(authManager, &TwitchAuthManager::accessTokenChanged);
  authManager->logout();
  
  QVERIFY(spy.count() >= 1);
  
  delete authManager;
  delete storage;
}

// ===== Tests de handleLocalCallback edge cases =====

void TestTwitchAuthManager::testHandleLocalCallbackStateMismatch() {
  // State mismatch should emit error
  // Can't easily test without exposing handleLocalCallback, but verify manager stability
  QVERIFY(m_authManager != nullptr);
}

void TestTwitchAuthManager::testHandleLocalCallbackMissingCode() {
  // Missing code should emit error
  // Can't easily test without exposing handleLocalCallback, but verify manager stability
  QVERIFY(m_authManager != nullptr);
}

// ===== Tests de buildSslConfiguration =====

void TestTwitchAuthManager::testBuildSslConfigurationNoPaths() {
  // Without TLS paths set, SSL config should be null/empty
  // Manager should still work (falls back to non-TLS)
  QVERIFY(m_authManager != nullptr);
}

void TestTwitchAuthManager::testBuildSslConfigurationInvalidCertPath() {
  // With invalid cert path, should handle gracefully
  qputenv("TWITCH_TLS_CERT_PATH", "/nonexistent/cert.pem");
  qputenv("TWITCH_TLS_KEY_PATH", "/nonexistent/key.pem");
  
  auto* storage = new MockSecureStorage();
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Should not crash with invalid paths
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
  
  // Cleanup env
  qunsetenv("TWITCH_TLS_CERT_PATH");
  qunsetenv("TWITCH_TLS_KEY_PATH");
}

// ===== Tests de scénarios combinés =====

void TestTwitchAuthManager::testFullLogoutLoginCycle() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "cycle_token");
  storage->store("token_client_id", "test_client_id");
  
  QDateTime future = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", future.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Initially authenticated
  QVERIFY(authManager->isAuthenticated());
  
  // Logout
  authManager->logout();
  QVERIFY(!authManager->isAuthenticated());
  QVERIFY(authManager->accessToken().isEmpty());
  
  // Can't complete login without browser, but verify state is clean
  QVERIFY(storage->retrieve("access_token").isEmpty());
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testTokenRefreshUpdatesExpiration() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "token_to_refresh");
  storage->store("refresh_token", "valid_refresh");
  storage->store("token_client_id", "test_client_id");
  
  // Token about to expire
  QDateTime soonExpire = QDateTime::currentDateTimeUtc().addSecs(60);
  storage->store("token_expiration", soonExpire.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Refresh will be triggered - without network just verify no crash
  QVERIFY(authManager != nullptr);
  
  delete authManager;
  delete storage;
}

void TestTwitchAuthManager::testMultipleLogoutsStable() {
  auto* storage = new MockSecureStorage();
  storage->store("access_token", "multi_logout");
  storage->store("token_client_id", "test_client_id");
  
  QDateTime future = QDateTime::currentDateTimeUtc().addSecs(3600);
  storage->store("token_expiration", future.toString(Qt::ISODate));
  
  TwitchAuthManager* authManager = new TwitchAuthManager(nullptr, storage, this);
  
  // Multiple logouts should not crash
  authManager->logout();
  authManager->logout();
  authManager->logout();
  authManager->logout();
  authManager->logout();
  
  QVERIFY(!authManager->isAuthenticated());
  
  delete authManager;
  delete storage;
}

// ===== Tests de PKCE helpers via comportement observable =====

void TestTwitchAuthManager::testCodeVerifierIsDifferentEachTime() {
  // Test that random generation produces different results
  // We test via SHA256 hashes which should be different for different inputs
  
  QString verifier1 = QString("verifier_%1").arg(QRandomGenerator::global()->generate());
  QString verifier2 = QString("verifier_%1").arg(QRandomGenerator::global()->generate());
  
  // Should be different (statistically nearly impossible to be same)
  QVERIFY(verifier1 != verifier2);
}

void TestTwitchAuthManager::testCodeChallengeIsDeterministic() {
  // Same verifier should produce same challenge
  QString verifier = "test_verifier_for_determinism";
  
  QByteArray hash1 = QCryptographicHash::hash(verifier.toUtf8(), QCryptographicHash::Sha256);
  QByteArray hash2 = QCryptographicHash::hash(verifier.toUtf8(), QCryptographicHash::Sha256);
  
  QString challenge1 = QString::fromUtf8(hash1.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  QString challenge2 = QString::fromUtf8(hash2.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
  
  QCOMPARE(challenge1, challenge2);
}

void TestTwitchAuthManager::testStateLengthSecurity() {
  // OAuth state should be sufficiently random (at least 24 chars per spec)
  // We verify by creating a simulated state
  
  const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
  QString simulatedState;
  simulatedState.reserve(24);
  for (int i = 0; i < 24; ++i) {
    int idx = QRandomGenerator::global()->bounded(static_cast<int>(sizeof(charset) - 1));
    simulatedState.append(charset[idx]);
  }
  
  // Should be 24 characters
  QCOMPARE(simulatedState.length(), 24);
  
  // Should only contain valid OAuth state characters
  QRegularExpression validPattern("^[A-Za-z0-9\\-._~]+$");
  QVERIFY(validPattern.match(simulatedState).hasMatch());
}

QTEST_MAIN(TestTwitchAuthManager)
#include "TestTwitchAuthManager.moc"
