#include <QtTest/QtTest>
#include <QVideoSink>
#include "media/FFmpegMediaSource.hpp"

using namespace blueplayer::media;

class TestFFmpegMediaSource : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  
  void testConstructor();
  void testVideoSink();
  void testSetVideoSink();
  void testStop();
};

void TestFFmpegMediaSource::initTestCase() {
}

void TestFFmpegMediaSource::cleanupTestCase() {
}

void TestFFmpegMediaSource::testConstructor() {
  FFmpegMediaSource source;
  QVERIFY(source.videoSink() == nullptr);
}

void TestFFmpegMediaSource::testVideoSink() {
  FFmpegMediaSource source;
  QVERIFY(source.videoSink() == nullptr);
  
  QVideoSink* sink = new QVideoSink(this);
  source.setVideoSink(sink);
  QCOMPARE(source.videoSink(), sink);
}

void TestFFmpegMediaSource::testSetVideoSink() {
  FFmpegMediaSource source;
  
  QVideoSink* sink1 = new QVideoSink(this);
  source.setVideoSink(sink1);
  QCOMPARE(source.videoSink(), sink1);
  
  QVideoSink* sink2 = new QVideoSink(this);
  source.setVideoSink(sink2);
  QCOMPARE(source.videoSink(), sink2);
  
  source.setVideoSink(nullptr);
  QVERIFY(source.videoSink() == nullptr);
}

void TestFFmpegMediaSource::testStop() {
  FFmpegMediaSource source;
  
  // Test que stop() ne plante pas même si rien n'est en cours
  source.stop();
  QVERIFY(true);
  
  // Note: Les tests d'ouverture et de lecture nécessiteraient des fichiers média réels
  // ou des mocks plus complexes
}

QTEST_MAIN(TestFFmpegMediaSource)
#include "TestFFmpegMediaSource.moc"


