#include <QtTest/QtTest>
#include <QTemporaryFile>
#include <QDir>
#include "core/InputValidator.hpp"

using namespace blueplayer::core;

class TestInputValidator : public QObject {
  Q_OBJECT

private slots:
  void testIsValidUrl();
  void testIsValidUrlInvalid();
  void testIsValidFilePath();
  void testIsValidFilePathInvalid();
  void testIsValidTwitchUserId();
  void testIsValidTwitchUserIdInvalid();
  void testIsValidTwitchUsername();
  void testIsValidTwitchUsernameInvalid();
  void testSanitizeString();
  void testIsValidJson();
  void testIsValidJsonInvalid();
  void testIsValidOAuthToken();
  void testIsValidOAuthTokenInvalid();
};

void TestInputValidator::testIsValidUrl() {
  QVERIFY(InputValidator::isValidUrl("https://www.example.com"));
  QVERIFY(InputValidator::isValidUrl("http://example.com/path"));
  QVERIFY(InputValidator::isValidUrl("https://subdomain.example.com:8080/path?query=value"));
}

void TestInputValidator::testIsValidUrlInvalid() {
  QVERIFY(!InputValidator::isValidUrl(""));
  QVERIFY(!InputValidator::isValidUrl("not a url"));
  QVERIFY(!InputValidator::isValidUrl("ftp://example.com"));  // Seulement http/https
  QVERIFY(!InputValidator::isValidUrl("://example.com"));  // Pas de schéma
  QVERIFY(!InputValidator::isValidUrl("https://"));  // Pas de host
}

void TestInputValidator::testIsValidFilePath() {
  QTemporaryFile tempFile;
  QVERIFY(tempFile.open());
  QString filePath = tempFile.fileName();
  tempFile.close();
  
  QVERIFY(InputValidator::isValidFilePath(filePath));
}

void TestInputValidator::testIsValidFilePathInvalid() {
  QVERIFY(!InputValidator::isValidFilePath(""));
  QVERIFY(!InputValidator::isValidFilePath("/nonexistent/file.txt"));
  QVERIFY(!InputValidator::isValidFilePath("../../../etc/passwd"));  // Path traversal
  QVERIFY(!InputValidator::isValidFilePath("file\0name.txt"));  // Null byte
}

void TestInputValidator::testIsValidTwitchUserId() {
  QVERIFY(InputValidator::isValidTwitchUserId("123456789"));
  QVERIFY(InputValidator::isValidTwitchUserId("1"));
  QVERIFY(InputValidator::isValidTwitchUserId("999999999999"));
}

void TestInputValidator::testIsValidTwitchUserIdInvalid() {
  QVERIFY(!InputValidator::isValidTwitchUserId(""));
  QVERIFY(!InputValidator::isValidTwitchUserId("abc123"));
  QVERIFY(!InputValidator::isValidTwitchUserId("12.34"));
  QVERIFY(!InputValidator::isValidTwitchUserId("-123"));
}

void TestInputValidator::testIsValidTwitchUsername() {
  QVERIFY(InputValidator::isValidTwitchUsername("testuser"));
  QVERIFY(InputValidator::isValidTwitchUsername("TestUser123"));
  QVERIFY(InputValidator::isValidTwitchUsername("user_name"));
  QVERIFY(InputValidator::isValidTwitchUsername("abcd"));  // Minimum 4 caractères
  QVERIFY(InputValidator::isValidTwitchUsername(QString("a").repeated(25)));  // Maximum 25 caractères
}

void TestInputValidator::testIsValidTwitchUsernameInvalid() {
  QVERIFY(!InputValidator::isValidTwitchUsername(""));
  QVERIFY(!InputValidator::isValidTwitchUsername("abc"));  // Trop court (< 4)
  QVERIFY(!InputValidator::isValidTwitchUsername(QString("a").repeated(26)));  // Trop long (> 25)
  QVERIFY(!InputValidator::isValidTwitchUsername("user-name"));  // Tirets non autorisés
  QVERIFY(!InputValidator::isValidTwitchUsername("user name"));  // Espaces non autorisés
}

void TestInputValidator::testSanitizeString() {
  // Créer une chaîne avec des caractères de contrôle en utilisant QByteArray
  // car QString tronque au '\x00'
  QByteArray inputBytes;
  inputBytes.append("Test");
  inputBytes.append('\x00');
  inputBytes.append("String");
  inputBytes.append('\x1F');
  inputBytes.append('\x7F');
  QString input = QString::fromUtf8(inputBytes);
  
  QString sanitized = InputValidator::sanitizeString(input);
  
  QVERIFY(!sanitized.contains('\x00'));
  QVERIFY(!sanitized.contains('\x1F'));
  QVERIFY(!sanitized.contains('\x7F'));
  QCOMPARE(sanitized, QString("TestString"));
  
  // Test de limitation de longueur
  QString longString = QString("a").repeated(20000);
  QString sanitizedLong = InputValidator::sanitizeString(longString);
  QVERIFY(sanitizedLong.length() <= 10000);
}

void TestInputValidator::testIsValidJson() {
  QVERIFY(InputValidator::isValidJson("{}"));
  QVERIFY(InputValidator::isValidJson(R"({"key": "value"})"));
  QVERIFY(InputValidator::isValidJson(R"([1, 2, 3])"));
  QVERIFY(InputValidator::isValidJson(R"({"nested": {"key": "value"}})"));
}

void TestInputValidator::testIsValidJsonInvalid() {
  QVERIFY(!InputValidator::isValidJson(""));
  QVERIFY(!InputValidator::isValidJson("{"));
  QVERIFY(!InputValidator::isValidJson("{key: value}"));
  QVERIFY(!InputValidator::isValidJson("not json"));
}

void TestInputValidator::testIsValidOAuthToken() {
  QString validToken = QString("a").repeated(50);  // 50 caractères
  QVERIFY(InputValidator::isValidOAuthToken(validToken));
  
  QString minToken = QString("a").repeated(20);  // Minimum 20 caractères
  QVERIFY(InputValidator::isValidOAuthToken(minToken));
  
  QString maxToken = QString("a").repeated(2000);  // Maximum 2000 caractères
  QVERIFY(InputValidator::isValidOAuthToken(maxToken));
}

void TestInputValidator::testIsValidOAuthTokenInvalid() {
  QVERIFY(!InputValidator::isValidOAuthToken(""));
  QVERIFY(!InputValidator::isValidOAuthToken("short"));  // Trop court (< 20)
  QVERIFY(!InputValidator::isValidOAuthToken(QString("a").repeated(19)));  // Trop court
  QVERIFY(!InputValidator::isValidOAuthToken(QString("a").repeated(2001)));  // Trop long (> 2000)
  QVERIFY(!InputValidator::isValidOAuthToken("token with spaces"));  // Espaces non autorisés
  QVERIFY(!InputValidator::isValidOAuthToken("token@invalid"));  // @ non autorisé
}

QTEST_MAIN(TestInputValidator)
#include "TestInputValidator.moc"


