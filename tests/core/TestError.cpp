/**
 * @file TestError.cpp
 * @brief Tests unitaires pour la classe Error
 */

#include <QtTest/QtTest>
#include "core/Error.hpp"

using namespace blueplayer::core;

class TestError : public QObject {
  Q_OBJECT

private slots:
  // ===== Tests des constructeurs =====
  void testDefaultConstructor();
  void testConstructorWithCode();
  void testConstructorWithCodeAndMessage();
  void testConstructorWithCodeMessageAndContext();

  // ===== Tests des getters =====
  void testCode();
  void testMessage();
  void testContext();

  // ===== Tests de hasError =====
  void testHasErrorWithUnknownCode();
  void testHasErrorWithErrorCode();
  void testHasErrorWithMessage();

  // ===== Tests de toString =====
  void testToStringWithoutContext();
  void testToStringWithContext();

  // ===== Tests des operateurs =====
  void testOperatorEqual();
  void testOperatorEqualDifferentCodes();
  void testIsCode();
  void testExplicitStringConversion();
  void testToQString();

  // ===== Tests de localizedMessage (data-driven) =====
  void testLocalizedMessage_data();
  void testLocalizedMessage();

  // ===== Tests des ErrorCodes (data-driven) =====
  void testErrorCodeValues_data();
  void testErrorCodeValues();
};

// ===== Tests des constructeurs =====

void TestError::testDefaultConstructor() {
  Error error;
  QCOMPARE(error.code(), ErrorCode::Unknown);
  QVERIFY(error.message().isEmpty());
  QVERIFY(error.context().isEmpty());
}

void TestError::testConstructorWithCode() {
  Error error(ErrorCode::NetworkError);
  QCOMPARE(error.code(), ErrorCode::NetworkError);
  QVERIFY(!error.message().isEmpty());  // Should have localized message
  QVERIFY(error.context().isEmpty());
}

void TestError::testConstructorWithCodeAndMessage() {
  Error error(ErrorCode::InvalidArgument, "Custom message");
  QCOMPARE(error.code(), ErrorCode::InvalidArgument);
  QCOMPARE(error.message(), QString("Custom message"));
  QVERIFY(error.context().isEmpty());
}

void TestError::testConstructorWithCodeMessageAndContext() {
  Error error(ErrorCode::TwitchApiError, "API failed", "getStreams");
  QCOMPARE(error.code(), ErrorCode::TwitchApiError);
  QCOMPARE(error.message(), QString("API failed"));
  QCOMPARE(error.context(), QString("getStreams"));
}

// ===== Tests des getters =====

void TestError::testCode() {
  Error error(ErrorCode::MediaDecodeError);
  QCOMPARE(error.code(), ErrorCode::MediaDecodeError);
}

void TestError::testMessage() {
  Error error(ErrorCode::Unknown, "Test message");
  QCOMPARE(error.message(), QString("Test message"));
}

void TestError::testContext() {
  Error error(ErrorCode::Unknown, "msg", "ctx");
  QCOMPARE(error.context(), QString("ctx"));
}

// ===== Tests de hasError =====

void TestError::testHasErrorWithUnknownCode() {
  Error error;
  QVERIFY(!error.hasError());  // Unknown code and empty message = no error
}

void TestError::testHasErrorWithErrorCode() {
  Error error(ErrorCode::NetworkError);
  QVERIFY(error.hasError());
}

void TestError::testHasErrorWithMessage() {
  Error error(ErrorCode::Unknown, "Some error message");
  QVERIFY(error.hasError());  // Has message even with Unknown code
}

// ===== Tests de toString =====

void TestError::testToStringWithoutContext() {
  Error error(ErrorCode::Unknown, "Error message");
  QCOMPARE(error.toString(), QString("Error message"));
}

void TestError::testToStringWithContext() {
  Error error(ErrorCode::Unknown, "Error message", "context info");
  QVERIFY(error.toString().contains("Error message"));
  QVERIFY(error.toString().contains("context info"));
}

// ===== Tests des operateurs =====

void TestError::testOperatorEqual() {
  Error error1(ErrorCode::NetworkError);
  Error error2(ErrorCode::NetworkError);
  QVERIFY(error1 == error2);
}

void TestError::testOperatorEqualDifferentCodes() {
  Error error1(ErrorCode::NetworkError);
  Error error2(ErrorCode::NetworkTimeout);
  QVERIFY(!(error1 == error2));
}

void TestError::testIsCode() {
  Error error(ErrorCode::TwitchApiError);
  QVERIFY(error.isCode(ErrorCode::TwitchApiError));
  QVERIFY(!error.isCode(ErrorCode::NetworkError));
}

void TestError::testExplicitStringConversion() {
  Error error(ErrorCode::Unknown, "Test conversion");
  QString str = static_cast<QString>(error);
  QCOMPARE(str, QString("Test conversion"));
}

void TestError::testToQString() {
  Error error(ErrorCode::Unknown, "QML test");
  QCOMPARE(error.toQString(), QString("QML test"));
}

// ===== Tests de localizedMessage (data-driven) =====

void TestError::testLocalizedMessage_data() {
  QTest::addColumn<ErrorCode>("code");

  // General errors
  QTest::newRow("Unknown") << ErrorCode::Unknown;
  QTest::newRow("InvalidArgument") << ErrorCode::InvalidArgument;
  QTest::newRow("InvalidState") << ErrorCode::InvalidState;
  QTest::newRow("NotInitialized") << ErrorCode::NotInitialized;

  // Network errors
  QTest::newRow("NetworkError") << ErrorCode::NetworkError;
  QTest::newRow("NetworkTimeout") << ErrorCode::NetworkTimeout;
  QTest::newRow("NetworkConnectionRefused") << ErrorCode::NetworkConnectionRefused;
  QTest::newRow("InvalidResponse") << ErrorCode::InvalidResponse;

  // Twitch errors
  QTest::newRow("TwitchNotAuthenticated") << ErrorCode::TwitchNotAuthenticated;
  QTest::newRow("TwitchInvalidToken") << ErrorCode::TwitchInvalidToken;
  QTest::newRow("TwitchApiError") << ErrorCode::TwitchApiError;
  QTest::newRow("TwitchRateLimitExceeded") << ErrorCode::TwitchRateLimitExceeded;

  // Media errors
  QTest::newRow("MediaFileNotFound") << ErrorCode::MediaFileNotFound;
  QTest::newRow("MediaFormatNotSupported") << ErrorCode::MediaFormatNotSupported;
  QTest::newRow("MediaDecodeError") << ErrorCode::MediaDecodeError;
  QTest::newRow("MediaDeviceError") << ErrorCode::MediaDeviceError;

  // Config errors
  QTest::newRow("ConfigNotFound") << ErrorCode::ConfigNotFound;
  QTest::newRow("ConfigInvalid") << ErrorCode::ConfigInvalid;
}

void TestError::testLocalizedMessage() {
  QFETCH(ErrorCode, code);
  QString msg = Error::localizedMessage(code);
  QVERIFY(!msg.isEmpty());
}

// ===== Tests des ErrorCodes (data-driven) =====

void TestError::testErrorCodeValues_data() {
  QTest::addColumn<ErrorCode>("code");
  QTest::addColumn<int>("expectedValue");

  // General errors: 0-99
  QTest::newRow("Unknown") << ErrorCode::Unknown << 0;
  QTest::newRow("InvalidArgument") << ErrorCode::InvalidArgument << 1;
  QTest::newRow("InvalidState") << ErrorCode::InvalidState << 2;
  QTest::newRow("NotInitialized") << ErrorCode::NotInitialized << 3;

  // Network errors: 100-199
  QTest::newRow("NetworkError") << ErrorCode::NetworkError << 100;
  QTest::newRow("NetworkTimeout") << ErrorCode::NetworkTimeout << 101;
  QTest::newRow("NetworkConnectionRefused") << ErrorCode::NetworkConnectionRefused << 102;
  QTest::newRow("InvalidResponse") << ErrorCode::InvalidResponse << 103;

  // Twitch errors: 200-299
  QTest::newRow("TwitchNotAuthenticated") << ErrorCode::TwitchNotAuthenticated << 200;
  QTest::newRow("TwitchInvalidToken") << ErrorCode::TwitchInvalidToken << 201;
  QTest::newRow("TwitchApiError") << ErrorCode::TwitchApiError << 202;
  QTest::newRow("TwitchRateLimitExceeded") << ErrorCode::TwitchRateLimitExceeded << 203;

  // Media errors: 300-399
  QTest::newRow("MediaFileNotFound") << ErrorCode::MediaFileNotFound << 300;
  QTest::newRow("MediaFormatNotSupported") << ErrorCode::MediaFormatNotSupported << 301;
  QTest::newRow("MediaDecodeError") << ErrorCode::MediaDecodeError << 302;
  QTest::newRow("MediaDeviceError") << ErrorCode::MediaDeviceError << 303;

  // Config errors: 400-499
  QTest::newRow("ConfigNotFound") << ErrorCode::ConfigNotFound << 400;
  QTest::newRow("ConfigInvalid") << ErrorCode::ConfigInvalid << 401;
}

void TestError::testErrorCodeValues() {
  QFETCH(ErrorCode, code);
  QFETCH(int, expectedValue);
  QCOMPARE(static_cast<int>(code), expectedValue);
}

QTEST_MAIN(TestError)
#include "TestError.moc"
