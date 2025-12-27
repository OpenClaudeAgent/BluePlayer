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

  // Tests des catégories (data-driven)
  void testLogCategory_data();
  void testLogCategory();

  // Tests des niveaux de log (data-driven)
  void testLogLevel_data();
  void testLogLevel();

  // Tests de setLevel
  void testSetLevel();

  // Tests de levelFromEnvironment (data-driven)
  void testLevelFromEnvironment_data();
  void testLevelFromEnvironment();

  // Tests des méthodes de logging (data-driven)
  void testLogMethod_data();
  void testLogMethod();

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
}

void TestLogger::testInitializeMultipleTimes() {
  Logger::initialize();
  Logger::initialize();
  Logger::initialize();
}

// ===== Tests des catégories (data-driven) =====

void TestLogger::testLogCategory_data() {
  QTest::addColumn<int>("categoryIndex");

  QTest::newRow("Media") << 0;
  QTest::newRow("Twitch") << 1;
  QTest::newRow("UI") << 2;
  QTest::newRow("Core") << 3;
  QTest::newRow("Network") << 4;
}

void TestLogger::testLogCategory() {
  QFETCH(int, categoryIndex);

  const QLoggingCategory *categories[] = {
      &LogCategory::Media, &LogCategory::Twitch, &LogCategory::UI, &LogCategory::Core, &LogCategory::Network};

  QVERIFY(categories[categoryIndex]->categoryName() != nullptr);
}

// ===== Tests des niveaux de log (data-driven) =====

void TestLogger::testLogLevel_data() {
  QTest::addColumn<LogLevel>("level");
  QTest::addColumn<int>("expectedValue");

  QTest::newRow("Debug") << LogLevel::Debug << 0;
  QTest::newRow("Info") << LogLevel::Info << 1;
  QTest::newRow("Warning") << LogLevel::Warning << 2;
  QTest::newRow("Error") << LogLevel::Error << 3;
  QTest::newRow("Critical") << LogLevel::Critical << 4;
}

void TestLogger::testLogLevel() {
  QFETCH(LogLevel, level);
  QFETCH(int, expectedValue);

  QCOMPARE(static_cast<int>(level), expectedValue);
}

// ===== Tests de setLevel =====

void TestLogger::testSetLevel() {
  // Ne doit pas crasher
  Logger::setLevel(LogCategory::Core, LogLevel::Debug);
  Logger::setLevel(LogCategory::Core, LogLevel::Info);
  Logger::setLevel(LogCategory::Core, LogLevel::Warning);
  Logger::setLevel(LogCategory::Core, LogLevel::Error);
  Logger::setLevel(LogCategory::Core, LogLevel::Critical);
}

// ===== Tests de levelFromEnvironment (data-driven) =====

void TestLogger::testLevelFromEnvironment_data() {
  QTest::addColumn<QByteArray>("envValue");
  QTest::addColumn<LogLevel>("defaultLevel");
  QTest::addColumn<LogLevel>("expectedLevel");
  QTest::addColumn<bool>("setEnv");

  QTest::newRow("Default (no env)") << QByteArray() << LogLevel::Warning << LogLevel::Warning << false;
  QTest::newRow("debug") << QByteArray("debug") << LogLevel::Error << LogLevel::Debug << true;
  QTest::newRow("info") << QByteArray("info") << LogLevel::Error << LogLevel::Info << true;
  QTest::newRow("warning") << QByteArray("warning") << LogLevel::Error << LogLevel::Warning << true;
  QTest::newRow("error") << QByteArray("error") << LogLevel::Debug << LogLevel::Error << true;
}

void TestLogger::testLevelFromEnvironment() {
  QFETCH(QByteArray, envValue);
  QFETCH(LogLevel, defaultLevel);
  QFETCH(LogLevel, expectedLevel);
  QFETCH(bool, setEnv);

  const char *envVar = setEnv ? "TEST_LOG_LEVEL" : "NONEXISTENT_ENV_VAR";

  if (setEnv) {
    qputenv("TEST_LOG_LEVEL", envValue);
  }

  LogLevel level = Logger::levelFromEnvironment(envVar, defaultLevel);
  QCOMPARE(level, expectedLevel);

  if (setEnv) {
    qunsetenv("TEST_LOG_LEVEL");
  }
}

// ===== Tests des méthodes de logging (data-driven) =====

void TestLogger::testLogMethod_data() {
  QTest::addColumn<LogLevel>("level");
  QTest::addColumn<QString>("message");

  QTest::newRow("Debug") << LogLevel::Debug << "Test debug message";
  QTest::newRow("Info") << LogLevel::Info << "Test info message";
  QTest::newRow("Warning") << LogLevel::Warning << "Test warning message";
  QTest::newRow("Error") << LogLevel::Error << "Test error message";
  QTest::newRow("Critical") << LogLevel::Critical << "Test critical message";
}

void TestLogger::testLogMethod() {
  QFETCH(LogLevel, level);
  QFETCH(QString, message);

  // Ne doit pas crasher
  switch (level) {
  case LogLevel::Debug:
    Logger::debug(LogCategory::Core, message);
    break;
  case LogLevel::Info:
    Logger::info(LogCategory::Core, message);
    break;
  case LogLevel::Warning:
    Logger::warning(LogCategory::Core, message);
    break;
  case LogLevel::Error:
    Logger::error(LogCategory::Core, message);
    break;
  case LogLevel::Critical:
    Logger::critical(LogCategory::Core, message);
    break;
  }
}

// ===== Tests avec messages spéciaux =====

void TestLogger::testLogEmptyMessage() {
  Logger::info(LogCategory::Core, "");
}

void TestLogger::testLogLongMessage() {
  QString longMessage = QString(10000, 'x');
  Logger::info(LogCategory::Core, longMessage);
}

void TestLogger::testLogUnicodeMessage() {
  Logger::info(LogCategory::Core, "Test unicode: Bonjour! Привет! 你好!");
}

QTEST_MAIN(TestLogger)
#include "TestLogger.moc"
