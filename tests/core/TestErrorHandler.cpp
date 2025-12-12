#include <QtTest/QtTest>
#include "core/ErrorHandler.hpp"
#include "core/Error.hpp"

using namespace blueplayer::core;

class TestErrorHandler : public QObject {
  Q_OBJECT

private slots:
  void testNetworkError();
  void testNetworkErrorWithDetails();
  void testTwitchApiError();
  void testTwitchApiErrorWithDetails();
  void testTwitchAuthError();
  void testTwitchAuthErrorWithDetails();
  void testMediaError();
  void testMediaErrorWithDetails();
  void testValidationError();
  void testValidationErrorWithDetails();
  void testToString();
};

void TestErrorHandler::testNetworkError() {
  Error error = ErrorHandler::networkError("TestContext");
  
  QCOMPARE(error.code(), ErrorCode::NetworkError);
  QVERIFY(error.message().contains("Erreur réseau"));
  QCOMPARE(error.context(), QString("TestContext"));
}

void TestErrorHandler::testNetworkErrorWithDetails() {
  Error error = ErrorHandler::networkError("TestContext", "Connection timeout");
  
  QCOMPARE(error.code(), ErrorCode::NetworkError);
  QVERIFY(error.message().contains("Connection timeout"));
  QCOMPARE(error.context(), QString("TestContext"));
}

void TestErrorHandler::testTwitchApiError() {
  Error error = ErrorHandler::twitchApiError("TwitchContext");
  
  QCOMPARE(error.code(), ErrorCode::TwitchApiError);
  QVERIFY(error.message().contains("Erreur de l'API Twitch"));
  QCOMPARE(error.context(), QString("TwitchContext"));
}

void TestErrorHandler::testTwitchApiErrorWithDetails() {
  Error error = ErrorHandler::twitchApiError("TwitchContext", "Rate limit exceeded");
  
  QCOMPARE(error.code(), ErrorCode::TwitchApiError);
  QVERIFY(error.message().contains("Rate limit exceeded"));
  QCOMPARE(error.context(), QString("TwitchContext"));
}

void TestErrorHandler::testTwitchAuthError() {
  Error error = ErrorHandler::twitchAuthError("AuthContext");
  
  QCOMPARE(error.code(), ErrorCode::TwitchNotAuthenticated);
  QVERIFY(error.message().contains("Erreur d'authentification Twitch"));
  QCOMPARE(error.context(), QString("AuthContext"));
}

void TestErrorHandler::testTwitchAuthErrorWithDetails() {
  Error error = ErrorHandler::twitchAuthError("AuthContext", "Invalid token");
  
  QCOMPARE(error.code(), ErrorCode::TwitchNotAuthenticated);
  QVERIFY(error.message().contains("Invalid token"));
  QCOMPARE(error.context(), QString("AuthContext"));
}

void TestErrorHandler::testMediaError() {
  Error error = ErrorHandler::mediaError(ErrorCode::MediaFileNotFound, "MediaContext");
  
  QCOMPARE(error.code(), ErrorCode::MediaFileNotFound);
  QVERIFY(error.message().contains("Fichier média introuvable"));
  QCOMPARE(error.context(), QString("MediaContext"));
}

void TestErrorHandler::testMediaErrorWithDetails() {
  Error error = ErrorHandler::mediaError(ErrorCode::MediaFormatNotSupported, "MediaContext", "Unsupported codec");
  
  QCOMPARE(error.code(), ErrorCode::MediaFormatNotSupported);
  QVERIFY(error.message().contains("Unsupported codec"));
  QCOMPARE(error.context(), QString("MediaContext"));
}

void TestErrorHandler::testValidationError() {
  Error error = ErrorHandler::validationError("ValidationContext");
  
  QCOMPARE(error.code(), ErrorCode::InvalidArgument);
  QVERIFY(error.message().contains("Erreur de validation"));
  QCOMPARE(error.context(), QString("ValidationContext"));
}

void TestErrorHandler::testValidationErrorWithDetails() {
  Error error = ErrorHandler::validationError("ValidationContext", "Invalid URL format");
  
  QCOMPARE(error.code(), ErrorCode::InvalidArgument);
  QVERIFY(error.message().contains("Invalid URL format"));
  QCOMPARE(error.context(), QString("ValidationContext"));
}

void TestErrorHandler::testToString() {
  Error error = ErrorHandler::networkError("TestContext", "Test details");
  QString str = ErrorHandler::toString(error);
  
  QVERIFY(str.contains("TestContext"));
  QVERIFY(str.contains("Test details") || str.contains("Erreur réseau"));
}

QTEST_MAIN(TestErrorHandler)
#include "TestErrorHandler.moc"











