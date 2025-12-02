#include <QtTest/QtTest>
#include "media/FFmpegBridge.hpp"

using namespace blueplayer::media;

class TestFFmpegBridge : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  
  void testEnsureInitialized();
  void testVersionSummary();
};

void TestFFmpegBridge::initTestCase() {
}

void TestFFmpegBridge::testEnsureInitialized() {
  // Test que l'initialisation ne plante pas
  FFmpegBridge::ensureInitialized();
  QVERIFY(true);
  
  // Test que l'appel multiple ne pose pas de problème
  FFmpegBridge::ensureInitialized();
  QVERIFY(true);
}

void TestFFmpegBridge::testVersionSummary() {
  FFmpegBridge::ensureInitialized();
  QString version = FFmpegBridge::versionSummary();
  
  // La version devrait contenir des informations sur FFmpeg
  QVERIFY(!version.isEmpty());
  QVERIFY(version.length() > 0);
}

QTEST_MAIN(TestFFmpegBridge)
#include "TestFFmpegBridge.moc"


