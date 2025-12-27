#include <QtTest/QtTest>
#include <QLoggingCategory>

#include "core/Logger.hpp"

using namespace blueplayer::core;

class TestLogger : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Tests d'initialisation
  void testInitialize();
  void testInitializeMultipleTimes();

  // Tests des catégories
  void testLogCategoryMedia();
  void testLogCategoryTwitch();
  void testLogCategoryUI();
  void testLogCategoryCore();
  void testLogCategoryNetwork();

  // Tests des niveaux de log
  void testLogLevelDebug();
  void testLogLevelInfo();
  void testLogLevelWarning();
  void testLogLevelError();
  void testLogLevelCritical();

  // Tests de setLevel
  void testSetLevel();

  // Tests de levelFromEnvironment
  void testLevelFromEnvironmentDefault();
  void testLevelFromEnvironmentDebug();
  void testLevelFromEnvironmentInfo();
  void testLevelFromEnvironmentWarning();
  void testLevelFromEnvironmentError();

  // Tests des méthodes de logging
  void testDebugLog();
  void testInfoLog();
  void testWarningLog();
  void testErrorLog();
  void testCriticalLog();

  // Tests avec messages spéciaux
  void testLogEmptyMessage();
  void testLogLongMessage();
  void testLogUnicodeMessage();
};

void TestLogger::initTestCase() {
  Logger::initialize();
}

void TestLogger::cleanupTestCase() {
}

// ===== Tests d'initialisation =====

void TestLogger::testInitialize() {
  // Ne doit pas crasher même appelé plusieurs fois
  Logger::initialize();
  QVERIFY(true);
}

void TestLogger::testInitializeMultipleTimes() {
  Logger::initialize();
  Logger::initialize();
  Logger::initialize();
  QVERIFY(true);
}

// ===== Tests des catégories =====

void TestLogger::testLogCategoryMedia() {
  QVERIFY(LogCategory::Media.categoryName() != nullptr);
}

void TestLogger::testLogCategoryTwitch() {
  QVERIFY(LogCategory::Twitch.categoryName() != nullptr);
}

void TestLogger::testLogCategoryUI() {
  QVERIFY(LogCategory::UI.categoryName() != nullptr);
}

void TestLogger::testLogCategoryCore() {
  QVERIFY(LogCategory::Core.categoryName() != nullptr);
}

void TestLogger::testLogCategoryNetwork() {
  QVERIFY(LogCategory::Network.categoryName() != nullptr);
}

// ===== Tests des niveaux de log =====

void TestLogger::testLogLevelDebug() {
  QCOMPARE(static_cast<int>(LogLevel::Debug), 0);
}

void TestLogger::testLogLevelInfo() {
  QCOMPARE(static_cast<int>(LogLevel::Info), 1);
}

void TestLogger::testLogLevelWarning() {
  QCOMPARE(static_cast<int>(LogLevel::Warning), 2);
}

void TestLogger::testLogLevelError() {
  QCOMPARE(static_cast<int>(LogLevel::Error), 3);
}

void TestLogger::testLogLevelCritical() {
  QCOMPARE(static_cast<int>(LogLevel::Critical), 4);
}

// ===== Tests de setLevel =====

void TestLogger::testSetLevel() {
  // Ne doit pas crasher
  Logger::setLevel(LogCategory::Core, LogLevel::Debug);
  Logger::setLevel(LogCategory::Core, LogLevel::Info);
  Logger::setLevel(LogCategory::Core, LogLevel::Warning);
  Logger::setLevel(LogCategory::Core, LogLevel::Error);
  Logger::setLevel(LogCategory::Core, LogLevel::Critical);
  QVERIFY(true);
}

// ===== Tests de levelFromEnvironment =====

void TestLogger::testLevelFromEnvironmentDefault() {
  LogLevel level = Logger::levelFromEnvironment("NONEXISTENT_ENV_VAR", LogLevel::Warning);
  QCOMPARE(level, LogLevel::Warning);
}

void TestLogger::testLevelFromEnvironmentDebug() {
  qputenv("TEST_LOG_LEVEL", "debug");
  LogLevel level = Logger::levelFromEnvironment("TEST_LOG_LEVEL", LogLevel::Error);
  QCOMPARE(level, LogLevel::Debug);
  qunsetenv("TEST_LOG_LEVEL");
}

void TestLogger::testLevelFromEnvironmentInfo() {
  qputenv("TEST_LOG_LEVEL", "info");
  LogLevel level = Logger::levelFromEnvironment("TEST_LOG_LEVEL", LogLevel::Error);
  QCOMPARE(level, LogLevel::Info);
  qunsetenv("TEST_LOG_LEVEL");
}

void TestLogger::testLevelFromEnvironmentWarning() {
  qputenv("TEST_LOG_LEVEL", "warning");
  LogLevel level = Logger::levelFromEnvironment("TEST_LOG_LEVEL", LogLevel::Error);
  QCOMPARE(level, LogLevel::Warning);
  qunsetenv("TEST_LOG_LEVEL");
}

void TestLogger::testLevelFromEnvironmentError() {
  qputenv("TEST_LOG_LEVEL", "error");
  LogLevel level = Logger::levelFromEnvironment("TEST_LOG_LEVEL", LogLevel::Debug);
  QCOMPARE(level, LogLevel::Error);
  qunsetenv("TEST_LOG_LEVEL");
}

// ===== Tests des méthodes de logging =====

void TestLogger::testDebugLog() {
  // Ne doit pas crasher
  Logger::debug(LogCategory::Core, "Test debug message");
  QVERIFY(true);
}

void TestLogger::testInfoLog() {
  Logger::info(LogCategory::Core, "Test info message");
  QVERIFY(true);
}

void TestLogger::testWarningLog() {
  Logger::warning(LogCategory::Core, "Test warning message");
  QVERIFY(true);
}

void TestLogger::testErrorLog() {
  Logger::error(LogCategory::Core, "Test error message");
  QVERIFY(true);
}

void TestLogger::testCriticalLog() {
  Logger::critical(LogCategory::Core, "Test critical message");
  QVERIFY(true);
}

// ===== Tests avec messages spéciaux =====

void TestLogger::testLogEmptyMessage() {
  Logger::info(LogCategory::Core, "");
  QVERIFY(true);
}

void TestLogger::testLogLongMessage() {
  QString longMessage = QString(10000, 'x');
  Logger::info(LogCategory::Core, longMessage);
  QVERIFY(true);
}

void TestLogger::testLogUnicodeMessage() {
  Logger::info(LogCategory::Core, "Test unicode: Bonjour! Привет! 你好! 🎮");
  QVERIFY(true);
}

QTEST_MAIN(TestLogger)
#include "TestLogger.moc"
