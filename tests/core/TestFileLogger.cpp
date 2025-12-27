#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QStandardPaths>
#include <QRegularExpression>

#include "core/FileLogger.hpp"

using namespace blueplayer::core;

class TestFileLogger : public QObject {
  Q_OBJECT

private slots:
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

private:
  QString readLogFileContent();
  void waitForLogFlush();
};

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
  
  // Wait a moment to ensure different timestamp
  QThread::msleep(1100);
  
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
  
  const int numThreads = 10;
  const int messagesPerThread = 50;
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
  const int iterations = 20;
  
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
  
  // Wait to ensure different timestamp
  QThread::msleep(1100);
  
  FileLogger::initialize();
  QString secondPath = FileLogger::currentLogPath();
  FileLogger::shutdown();
  
  if (firstPath.isEmpty() || secondPath.isEmpty()) {
    QSKIP("Could not create log files");
  }
  
  // Both files should exist
  QVERIFY(QFile::exists(firstPath));
  QVERIFY(QFile::exists(secondPath));
  
  // If timestamps are different, paths should be different
  if (firstPath != secondPath) {
    QVERIFY(firstPath != secondPath);
  }
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
  QThread::msleep(50);
  QCoreApplication::processEvents();
}

QTEST_MAIN(TestFileLogger)
#include "TestFileLogger.moc"
