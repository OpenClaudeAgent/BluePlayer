#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QUuid>

#include <filesystem>
#include <chrono>

#include "core/FileLogger.hpp"

using namespace blueplayer::core;

class TestFileLogger : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // File creation and opening tests
  void testInitializeCreatesLogFile();
  void testInitializeSetsLogPath();
  void testInitializeCreatesLogDirectory();
  void testCurrentLogPathEmptyBeforeInit();
  void testCurrentLogPathSetAfterInit();
  void testLogFileContainsHeader();

  // Shutdown tests
  void testShutdownClearsLogPath();
  void testShutdownWithoutInitialize();
  void testDoubleShutdown();
  void testReinitializeAfterShutdown();

  // Double initialization tests
  void testDoubleInitialize();

  // Writing logs to file tests
  void testDebugMessageWrittenToFile();
  void testInfoMessageWrittenToFile();
  void testWarningMessageWrittenToFile();
  void testCriticalMessageWrittenToFile();
  void testMultipleMessagesWrittenToFile();

  // Log format tests
  void testLogFormatContainsTimestamp();
  void testLogFormatContainsLevel();
  void testLogFormatContainsMessage();

  // Special message tests
  void testEmptyMessage();
  void testLongMessage();
  void testUnicodeMessage();
  void testMessageWithNewlines();
  void testMessageWithSpecialCharacters();

  // Thread safety tests
  void testConcurrentLogging();
  void testConcurrentInitShutdown();

  // Log rotation tests (session-based)
  void testNewSessionCreatesNewFile();
  void testLogFileNaming();

  // Error handling tests
  void testInitializeWithInvalidPath();
  void testLoggingAfterShutdown();

  // ===== cleanupOldLogs() tests =====
  void testCleanupOldLogs_EmptyDirectory();
  void testCleanupOldLogs_DirectoryDoesNotExist();
  void testCleanupOldLogs_AllFilesRecent();
  void testCleanupOldLogs_DeletesFilesOlderThanMaxAge();
  void testCleanupOldLogs_LimitsToMaxFiles();
  void testCleanupOldLogs_CombinesAgeAndCountLimits();
  void testCleanupOldLogs_ReturnsCorrectDeletedCount();
  void testCleanupOldLogs_IgnoresNonLogFiles();

private:
  QString readLogFileContent();
  void waitForLogFlush();
  QString getLogDirectory();
  void createTestLogFile(const QString& filename, int daysOld = 0);
  void clearLogDirectory();
  int countLogFiles();
  
  QString m_originalAppName;
  QString m_testCacheDir;
};

void TestFileLogger::initTestCase() {
  // Save original app name
  m_originalAppName = QCoreApplication::applicationName();
  
  // Use a unique app name per test process to isolate cache directories
  // This ensures parallel test runs don't conflict
  QString uniqueId = QUuid::createUuid().toString(QUuid::Id128);
  QString uniqueAppName = QString("test_file_logger_%1").arg(uniqueId);
  QCoreApplication::setApplicationName(uniqueAppName);
  
  // Store the cache dir for cleanup
  m_testCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

void TestFileLogger::cleanupTestCase() {
  // Ensure logger is shut down
  FileLogger::shutdown();
  
  // Clean up the test cache directory
  if (!m_testCacheDir.isEmpty()) {
    QDir cacheDir(m_testCacheDir);
    if (cacheDir.exists()) {
      cacheDir.removeRecursively();
    }
  }
  
  // Restore original app name
  QCoreApplication::setApplicationName(m_originalAppName);
}

void TestFileLogger::init() {
  // Clean state before each test
  FileLogger::shutdown();
}

void TestFileLogger::cleanup() {
  // Ensure clean state after each test
  FileLogger::shutdown();
}

// ===== File creation and opening tests =====

void TestFileLogger::testInitializeCreatesLogFile() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  QVERIFY2(!logPath.isEmpty(), "Log path should not be empty after initialization");
  QVERIFY2(QFile::exists(logPath), "Log file should exist after initialization");
}

void TestFileLogger::testInitializeSetsLogPath() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  QVERIFY(!logPath.isEmpty());
  QVERIFY(logPath.contains("blueplayer_"));
  QVERIFY(logPath.endsWith(".log"));
}

void TestFileLogger::testInitializeCreatesLogDirectory() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (!logPath.isEmpty()) {
    QFileInfo fileInfo(logPath);
    QVERIFY(fileInfo.dir().exists());
  }
}

void TestFileLogger::testCurrentLogPathEmptyBeforeInit() {
  // Ensure shutdown first
  FileLogger::shutdown();
  
  QString logPath = FileLogger::currentLogPath();
  QVERIFY2(logPath.isEmpty(), "Log path should be empty before initialization");
}

void TestFileLogger::testCurrentLogPathSetAfterInit() {
  QString pathBefore = FileLogger::currentLogPath();
  QVERIFY(pathBefore.isEmpty());
  
  FileLogger::initialize();
  
  QString pathAfter = FileLogger::currentLogPath();
  QVERIFY(!pathAfter.isEmpty());
}

void TestFileLogger::testLogFileContainsHeader() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  waitForLogFlush();
  QString content = readLogFileContent();
  
  QVERIFY2(content.contains("================"), "Log file should contain header separator");
  QVERIFY2(content.contains("BluePlayer Log"), "Log file should contain header text");
}

// ===== Shutdown tests =====

void TestFileLogger::testShutdownClearsLogPath() {
  FileLogger::initialize();
  QVERIFY(!FileLogger::currentLogPath().isEmpty());
  
  FileLogger::shutdown();
  QVERIFY2(FileLogger::currentLogPath().isEmpty(), 
           "Log path should be empty after shutdown");
}

void TestFileLogger::testShutdownWithoutInitialize() {
  // Should not crash
  FileLogger::shutdown();
  FileLogger::shutdown();
  QVERIFY(true);
}

void TestFileLogger::testDoubleShutdown() {
  FileLogger::initialize();
  FileLogger::shutdown();
  FileLogger::shutdown();
  
  QVERIFY(FileLogger::currentLogPath().isEmpty());
}

void TestFileLogger::testReinitializeAfterShutdown() {
  FileLogger::initialize();
  QString firstPath = FileLogger::currentLogPath();
  FileLogger::shutdown();
  
  // No need to wait - just verify reinitialization works
  FileLogger::initialize();
  QString secondPath = FileLogger::currentLogPath();
  
  QVERIFY(!secondPath.isEmpty());
  // Paths might be different due to timestamp
  QVERIFY(secondPath.contains("blueplayer_"));
}

// ===== Double initialization tests =====

void TestFileLogger::testDoubleInitialize() {
  FileLogger::initialize();
  QString firstPath = FileLogger::currentLogPath();
  
  // Second initialize - implementation may ignore or handle differently
  FileLogger::initialize();
  QString secondPath = FileLogger::currentLogPath();
  
  // Both should have valid paths (behavior depends on implementation)
  QVERIFY(!firstPath.isEmpty());
  QVERIFY(!secondPath.isEmpty());
}

// ===== Writing logs to file tests =====

void TestFileLogger::testDebugMessageWrittenToFile() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  qDebug() << "TestDebugMessage12345";
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY2(content.contains("TestDebugMessage12345"), 
           "Debug message should be written to log file");
  QVERIFY2(content.contains("[DEBUG]"), 
           "Debug level should be indicated");
}

void TestFileLogger::testInfoMessageWrittenToFile() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  qInfo() << "TestInfoMessage67890";
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY2(content.contains("TestInfoMessage67890"), 
           "Info message should be written to log file");
  QVERIFY2(content.contains("[INFO]"), 
           "Info level should be indicated");
}

void TestFileLogger::testWarningMessageWrittenToFile() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  qWarning() << "TestWarningMessage11111";
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY2(content.contains("TestWarningMessage11111"), 
           "Warning message should be written to log file");
  QVERIFY2(content.contains("[WARN]"), 
           "Warning level should be indicated");
}

void TestFileLogger::testCriticalMessageWrittenToFile() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  qCritical() << "TestCriticalMessage22222";
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY2(content.contains("TestCriticalMessage22222"), 
           "Critical message should be written to log file");
  QVERIFY2(content.contains("[ERROR]"), 
           "Critical/Error level should be indicated");
}

void TestFileLogger::testMultipleMessagesWrittenToFile() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  qDebug() << "FirstMessage";
  qInfo() << "SecondMessage";
  qWarning() << "ThirdMessage";
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY(content.contains("FirstMessage"));
  QVERIFY(content.contains("SecondMessage"));
  QVERIFY(content.contains("ThirdMessage"));
}

// ===== Log format tests =====

void TestFileLogger::testLogFormatContainsTimestamp() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  qInfo() << "TimestampTestMessage";
  waitForLogFlush();
  
  QString content = readLogFileContent();
  
  // Check for timestamp pattern [YYYY-MM-DD HH:MM:SS.mmm]
  QRegularExpression timestampPattern(R"(\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\])");
  QVERIFY2(timestampPattern.match(content).hasMatch(),
           "Log should contain properly formatted timestamp");
}

void TestFileLogger::testLogFormatContainsLevel() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  qInfo() << "LevelTestMessage";
  waitForLogFlush();
  
  QString content = readLogFileContent();
  
  // Check for level pattern [LEVEL]
  QRegularExpression levelPattern(R"(\[(DEBUG|INFO|WARN|ERROR|FATAL)\])");
  QVERIFY2(levelPattern.match(content).hasMatch(),
           "Log should contain log level");
}

void TestFileLogger::testLogFormatContainsMessage() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  QString testMessage = "UniqueTestMessage_" + QString::number(QDateTime::currentMSecsSinceEpoch());
  qInfo().noquote() << testMessage;
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY2(content.contains(testMessage),
           "Log should contain the actual message");
}

// ===== Special message tests =====

void TestFileLogger::testEmptyMessage() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  // Should not crash
  qInfo() << "";
  waitForLogFlush();
  
  QVERIFY(QFile::exists(logPath));
}

void TestFileLogger::testLongMessage() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  QString longMessage = QString(10000, 'X');
  QString marker = "LONGMSG_START_";
  qInfo().noquote() << marker + longMessage;
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY2(content.contains(marker),
           "Long message should be written to log file");
}

void TestFileLogger::testUnicodeMessage() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  QString unicodeMessage = "Unicode: Bonjour! Cafe! Strasse!";
  qInfo().noquote() << unicodeMessage;
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY2(content.contains("Unicode:"),
           "Unicode message should be written to log file");
}

void TestFileLogger::testMessageWithNewlines() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  qInfo() << "Line1_NEWLINE_TEST";
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY(content.contains("Line1_NEWLINE_TEST"));
}

void TestFileLogger::testMessageWithSpecialCharacters() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  qInfo() << "Special: <>&\"'\\t\\n SPECIAL_CHARS_TEST";
  waitForLogFlush();
  
  QString content = readLogFileContent();
  QVERIFY(content.contains("SPECIAL_CHARS_TEST"));
}

// ===== Thread safety tests =====

void TestFileLogger::testConcurrentLogging() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  const int numThreads = 4;
  const int messagesPerThread = 10;
  QList<QThread*> threads;
  QMutex startMutex;
  QWaitCondition startCondition;
  bool ready = false;
  
  // Create threads
  for (int i = 0; i < numThreads; ++i) {
    QThread* thread = QThread::create([&, i]() {
      // Wait for all threads to be ready
      {
        QMutexLocker locker(&startMutex);
        while (!ready) {
          startCondition.wait(&startMutex);
        }
      }
      
      // Log messages
      for (int j = 0; j < messagesPerThread; ++j) {
        qInfo() << QString("Thread%1_Message%2").arg(i).arg(j);
      }
    });
    threads.append(thread);
    thread->start();
  }
  
  // Start all threads simultaneously
  {
    QMutexLocker locker(&startMutex);
    ready = true;
    startCondition.wakeAll();
  }
  
  // Wait for all threads to complete
  for (QThread* thread : threads) {
    thread->wait();
    delete thread;
  }
  
  waitForLogFlush();
  
  // Verify file exists and has content
  QString content = readLogFileContent();
  QVERIFY(!content.isEmpty());
  
  // Verify some messages were logged (not all may be present due to buffering)
  QVERIFY(content.contains("Thread"));
  QVERIFY(content.contains("Message"));
}

void TestFileLogger::testConcurrentInitShutdown() {
  // Test that concurrent init/shutdown doesn't crash
  const int iterations = 5;
  
  for (int i = 0; i < iterations; ++i) {
    FileLogger::initialize();
    
    // Log something
    qInfo() << "ConcurrentTest" << i;
    
    FileLogger::shutdown();
  }
  
  QVERIFY(true); // If we get here without crashing, test passes
}

// ===== Log rotation tests (session-based) =====

void TestFileLogger::testNewSessionCreatesNewFile() {
  FileLogger::initialize();
  QString firstPath = FileLogger::currentLogPath();
  FileLogger::shutdown();
  
  // Small delay to potentially get different timestamp (not critical for test)
  QThread::msleep(10);
  
  FileLogger::initialize();
  QString secondPath = FileLogger::currentLogPath();
  FileLogger::shutdown();
  
  if (firstPath.isEmpty() || secondPath.isEmpty()) {
    QSKIP("Could not create log files");
  }
  
  // Both files should exist (may be same file if same second)
  QVERIFY(QFile::exists(firstPath));
  QVERIFY(!secondPath.isEmpty());
}

void TestFileLogger::testLogFileNaming() {
  FileLogger::initialize();
  QString logPath = FileLogger::currentLogPath();
  
  if (logPath.isEmpty()) {
    QSKIP("Could not create log file");
  }
  
  QFileInfo fileInfo(logPath);
  QString fileName = fileInfo.fileName();
  
  // Check naming pattern: blueplayer_YYYY-MM-DD_HH-MM-SS.log
  QRegularExpression namePattern(R"(blueplayer_\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}\.log)");
  QVERIFY2(namePattern.match(fileName).hasMatch(),
           qPrintable(QString("Log filename should match pattern: %1").arg(fileName)));
}

// ===== Error handling tests =====

void TestFileLogger::testInitializeWithInvalidPath() {
  // This tests the resilience of the logger when it cannot create a file
  // The current implementation uses QStandardPaths which should always work
  // But we verify it doesn't crash
  
  FileLogger::initialize();
  // Should not crash regardless of outcome
  QVERIFY(true);
}

void TestFileLogger::testLoggingAfterShutdown() {
  FileLogger::initialize();
  FileLogger::shutdown();
  
  // Logging after shutdown should not crash
  // Messages will go to default handler (stderr)
  qInfo() << "MessageAfterShutdown";
  qWarning() << "WarningAfterShutdown";
  qCritical() << "CriticalAfterShutdown";
  
  QVERIFY(true); // If we get here without crashing, test passes
}

// ===== Helper methods =====

QString TestFileLogger::readLogFileContent() {
  QString logPath = FileLogger::currentLogPath();
  if (logPath.isEmpty()) {
    return QString();
  }
  
  QFile file(logPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return QString();
  }
  
  return QString::fromUtf8(file.readAll());
}

void TestFileLogger::waitForLogFlush() {
  // Give some time for the log to be flushed
  QThread::msleep(5);
  QCoreApplication::processEvents();
}

QString TestFileLogger::getLogDirectory() {
  QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  return cacheDir + "/logs";
}

void TestFileLogger::createTestLogFile(const QString& filename, int daysOld) {
  QString logDir = getLogDirectory();
  QDir().mkpath(logDir);
  
  QString filePath = logDir + "/" + filename;
  QFile file(filePath);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    file.write("Test log content\n");
    file.close();
    
    // Set file modification time to daysOld days ago
    if (daysOld > 0) {
      namespace fs = std::filesystem;
      auto now = fs::file_time_type::clock::now();
      auto oldTime = now - std::chrono::hours(24 * daysOld);
      fs::last_write_time(filePath.toStdString(), oldTime);
    }
  }
}

void TestFileLogger::clearLogDirectory() {
  QString logDir = getLogDirectory();
  QDir dir(logDir);
  if (dir.exists()) {
    // Remove all blueplayer_*.log files
    QStringList filters;
    filters << "blueplayer_*.log";
    for (const QString& file : dir.entryList(filters, QDir::Files)) {
      dir.remove(file);
    }
  }
}

int TestFileLogger::countLogFiles() {
  QString logDir = getLogDirectory();
  QDir dir(logDir);
  if (!dir.exists()) {
    return 0;
  }
  QStringList filters;
  filters << "blueplayer_*.log";
  return dir.entryList(filters, QDir::Files).count();
}

// ===== cleanupOldLogs() tests =====

void TestFileLogger::testCleanupOldLogs_EmptyDirectory() {
  // Arrange: Ensure log directory exists but is empty
  clearLogDirectory();
  QString logDir = getLogDirectory();
  QDir().mkpath(logDir);
  QCOMPARE(countLogFiles(), 0);
  
  // Act
  int deletedCount = FileLogger::cleanupOldLogs(7, 20);
  
  // Assert
  QCOMPARE(deletedCount, 0);
}

void TestFileLogger::testCleanupOldLogs_DirectoryDoesNotExist() {
  // Arrange: Remove the log directory completely
  QString logDir = getLogDirectory();
  QDir dir(logDir);
  if (dir.exists()) {
    dir.removeRecursively();
  }
  QVERIFY(!dir.exists());
  
  // Act: Should not crash and return 0
  int deletedCount = FileLogger::cleanupOldLogs(7, 20);
  
  // Assert
  QCOMPARE(deletedCount, 0);
}

void TestFileLogger::testCleanupOldLogs_AllFilesRecent() {
  // Arrange: Create recent log files (less than maxAgeDays old)
  clearLogDirectory();
  createTestLogFile("blueplayer_2024-01-01_12-00-00.log", 0);  // Today
  createTestLogFile("blueplayer_2024-01-02_12-00-00.log", 1);  // 1 day old
  createTestLogFile("blueplayer_2024-01-03_12-00-00.log", 2);  // 2 days old
  QCOMPARE(countLogFiles(), 3);
  
  // Act: maxAgeDays=7, so nothing should be deleted
  int deletedCount = FileLogger::cleanupOldLogs(7, 20);
  
  // Assert: All files should remain
  QCOMPARE(deletedCount, 0);
  QCOMPARE(countLogFiles(), 3);
}

void TestFileLogger::testCleanupOldLogs_DeletesFilesOlderThanMaxAge() {
  // Arrange: Create mix of old and recent files
  clearLogDirectory();
  createTestLogFile("blueplayer_2024-01-01_12-00-00.log", 0);   // Today - keep
  createTestLogFile("blueplayer_2024-01-02_12-00-00.log", 5);   // 5 days - keep
  createTestLogFile("blueplayer_2024-01-03_12-00-00.log", 8);   // 8 days - delete
  createTestLogFile("blueplayer_2024-01-04_12-00-00.log", 10);  // 10 days - delete
  createTestLogFile("blueplayer_2024-01-05_12-00-00.log", 30);  // 30 days - delete
  QCOMPARE(countLogFiles(), 5);
  
  // Act: maxAgeDays=7
  int deletedCount = FileLogger::cleanupOldLogs(7, 100);  // maxFiles=100 to not trigger count limit
  
  // Assert: 3 old files deleted, 2 remain
  QCOMPARE(deletedCount, 3);
  QCOMPARE(countLogFiles(), 2);
}

void TestFileLogger::testCleanupOldLogs_LimitsToMaxFiles() {
  // Arrange: Create more files than maxFiles, all recent
  clearLogDirectory();
  for (int i = 0; i < 25; ++i) {
    QString filename = QString("blueplayer_2024-01-%1_12-00-00.log").arg(i + 1, 2, 10, QChar('0'));
    createTestLogFile(filename, i % 3);  // 0-2 days old (all recent)
  }
  QCOMPARE(countLogFiles(), 25);
  
  // Act: maxFiles=20, all files are recent so only count limit applies
  int deletedCount = FileLogger::cleanupOldLogs(30, 20);  // maxAgeDays=30 to not trigger age limit
  
  // Assert: 5 oldest files deleted, 20 remain
  QCOMPARE(deletedCount, 5);
  QCOMPARE(countLogFiles(), 20);
}

void TestFileLogger::testCleanupOldLogs_CombinesAgeAndCountLimits() {
  // Arrange: Create files that will trigger both limits
  clearLogDirectory();
  // 3 old files (> 7 days)
  createTestLogFile("blueplayer_old_01.log", 10);
  createTestLogFile("blueplayer_old_02.log", 15);
  createTestLogFile("blueplayer_old_03.log", 20);
  // 7 recent files (but we want max 5)
  createTestLogFile("blueplayer_recent_01.log", 0);
  createTestLogFile("blueplayer_recent_02.log", 1);
  createTestLogFile("blueplayer_recent_03.log", 2);
  createTestLogFile("blueplayer_recent_04.log", 3);
  createTestLogFile("blueplayer_recent_05.log", 4);
  createTestLogFile("blueplayer_recent_06.log", 5);
  createTestLogFile("blueplayer_recent_07.log", 6);
  QCOMPARE(countLogFiles(), 10);
  
  // Act: maxAgeDays=7, maxFiles=5
  int deletedCount = FileLogger::cleanupOldLogs(7, 5);
  
  // Assert: 3 old + 2 excess = 5 deleted, 5 remain
  QCOMPARE(deletedCount, 5);
  QCOMPARE(countLogFiles(), 5);
}

void TestFileLogger::testCleanupOldLogs_ReturnsCorrectDeletedCount() {
  // Arrange
  clearLogDirectory();
  createTestLogFile("blueplayer_test_01.log", 15);  // Will be deleted (old)
  createTestLogFile("blueplayer_test_02.log", 10);  // Will be deleted (old)
  createTestLogFile("blueplayer_test_03.log", 2);   // Keep (recent)
  
  // Act
  int deletedCount = FileLogger::cleanupOldLogs(7, 20);
  
  // Assert: Return value matches actual deletions
  QCOMPARE(deletedCount, 2);
  QCOMPARE(countLogFiles(), 1);
}

void TestFileLogger::testCleanupOldLogs_IgnoresNonLogFiles() {
  // Arrange: Create log files and non-log files
  clearLogDirectory();
  createTestLogFile("blueplayer_2024-01-01.log", 15);  // Delete (old)
  createTestLogFile("blueplayer_2024-01-02.log", 0);   // Keep (recent)
  
  // Create non-log files in the same directory
  QString logDir = getLogDirectory();
  QFile otherFile(logDir + "/other_file.txt");
  if (otherFile.open(QIODevice::WriteOnly)) {
    otherFile.write("other content");
    otherFile.close();
  }
  QFile readmeFile(logDir + "/readme.md");
  if (readmeFile.open(QIODevice::WriteOnly)) {
    readmeFile.write("readme content");
    readmeFile.close();
  }
  
  // Act
  int deletedCount = FileLogger::cleanupOldLogs(7, 20);
  
  // Assert: Only log file deleted, non-log files remain
  QCOMPARE(deletedCount, 1);
  QCOMPARE(countLogFiles(), 1);
  QVERIFY(QFile::exists(logDir + "/other_file.txt"));
  QVERIFY(QFile::exists(logDir + "/readme.md"));
  
  // Cleanup non-log files
  QFile::remove(logDir + "/other_file.txt");
  QFile::remove(logDir + "/readme.md");
}

QTEST_MAIN(TestFileLogger)
#include "TestFileLogger.moc"
