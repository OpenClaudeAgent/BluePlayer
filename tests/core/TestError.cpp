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

  // ===== Tests des opérateurs =====
  void testOperatorEqual();
  void testOperatorEqualDifferentCodes();
  void testIsCode();
  void testExplicitStringConversion();
  void testToQString();

  // ===== Tests de localizedMessage =====
  void testLocalizedMessageUnknown();
  void testLocalizedMessageInvalidArgument();
  void testLocalizedMessageInvalidState();
  void testLocalizedMessageNotInitialized();
  void testLocalizedMessageNetworkError();
  void testLocalizedMessageNetworkTimeout();
  void testLocalizedMessageNetworkConnectionRefused();
  void testLocalizedMessageInvalidResponse();
  void testLocalizedMessageTwitchNotAuthenticated();
  void testLocalizedMessageTwitchInvalidToken();
  void testLocalizedMessageTwitchApiError();
  void testLocalizedMessageTwitchRateLimitExceeded();
  void testLocalizedMessageMediaFileNotFound();
  void testLocalizedMessageMediaFormatNotSupported();
  void testLocalizedMessageMediaDecodeError();
  void testLocalizedMessageMediaDeviceError();
  void testLocalizedMessageConfigNotFound();
  void testLocalizedMessageConfigInvalid();

  // ===== Tests des ErrorCodes =====
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

// ===== Tests des opérateurs =====

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

// ===== Tests de localizedMessage =====

void TestError::testLocalizedMessageUnknown() {
  QString msg = Error::localizedMessage(ErrorCode::Unknown);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageInvalidArgument() {
  QString msg = Error::localizedMessage(ErrorCode::InvalidArgument);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageInvalidState() {
  QString msg = Error::localizedMessage(ErrorCode::InvalidState);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageNotInitialized() {
  QString msg = Error::localizedMessage(ErrorCode::NotInitialized);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageNetworkError() {
  QString msg = Error::localizedMessage(ErrorCode::NetworkError);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageNetworkTimeout() {
  QString msg = Error::localizedMessage(ErrorCode::NetworkTimeout);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageNetworkConnectionRefused() {
  QString msg = Error::localizedMessage(ErrorCode::NetworkConnectionRefused);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageInvalidResponse() {
  QString msg = Error::localizedMessage(ErrorCode::InvalidResponse);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageTwitchNotAuthenticated() {
  QString msg = Error::localizedMessage(ErrorCode::TwitchNotAuthenticated);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageTwitchInvalidToken() {
  QString msg = Error::localizedMessage(ErrorCode::TwitchInvalidToken);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageTwitchApiError() {
  QString msg = Error::localizedMessage(ErrorCode::TwitchApiError);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageTwitchRateLimitExceeded() {
  QString msg = Error::localizedMessage(ErrorCode::TwitchRateLimitExceeded);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageMediaFileNotFound() {
  QString msg = Error::localizedMessage(ErrorCode::MediaFileNotFound);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageMediaFormatNotSupported() {
  QString msg = Error::localizedMessage(ErrorCode::MediaFormatNotSupported);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageMediaDecodeError() {
  QString msg = Error::localizedMessage(ErrorCode::MediaDecodeError);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageMediaDeviceError() {
  QString msg = Error::localizedMessage(ErrorCode::MediaDeviceError);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageConfigNotFound() {
  QString msg = Error::localizedMessage(ErrorCode::ConfigNotFound);
  QVERIFY(!msg.isEmpty());
}

void TestError::testLocalizedMessageConfigInvalid() {
  QString msg = Error::localizedMessage(ErrorCode::ConfigInvalid);
  QVERIFY(!msg.isEmpty());
}

// ===== Tests des ErrorCodes =====

void TestError::testErrorCodeValues() {
  // General errors: 0-99
  QCOMPARE(static_cast<int>(ErrorCode::Unknown), 0);
  QCOMPARE(static_cast<int>(ErrorCode::InvalidArgument), 1);
  QCOMPARE(static_cast<int>(ErrorCode::InvalidState), 2);
  QCOMPARE(static_cast<int>(ErrorCode::NotInitialized), 3);

  // Network errors: 100-199
  QCOMPARE(static_cast<int>(ErrorCode::NetworkError), 100);
  QCOMPARE(static_cast<int>(ErrorCode::NetworkTimeout), 101);
  QCOMPARE(static_cast<int>(ErrorCode::NetworkConnectionRefused), 102);
  QCOMPARE(static_cast<int>(ErrorCode::InvalidResponse), 103);

  // Twitch errors: 200-299
  QCOMPARE(static_cast<int>(ErrorCode::TwitchNotAuthenticated), 200);
  QCOMPARE(static_cast<int>(ErrorCode::TwitchInvalidToken), 201);
  QCOMPARE(static_cast<int>(ErrorCode::TwitchApiError), 202);
  QCOMPARE(static_cast<int>(ErrorCode::TwitchRateLimitExceeded), 203);

  // Media errors: 300-399
  QCOMPARE(static_cast<int>(ErrorCode::MediaFileNotFound), 300);
  QCOMPARE(static_cast<int>(ErrorCode::MediaFormatNotSupported), 301);
  QCOMPARE(static_cast<int>(ErrorCode::MediaDecodeError), 302);
  QCOMPARE(static_cast<int>(ErrorCode::MediaDeviceError), 303);

  // Config errors: 400-499
  QCOMPARE(static_cast<int>(ErrorCode::ConfigNotFound), 400);
  QCOMPARE(static_cast<int>(ErrorCode::ConfigInvalid), 401);
}

QTEST_MAIN(TestError)
#include "TestError.moc"
