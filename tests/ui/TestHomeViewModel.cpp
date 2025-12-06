#include <QtTest/QtTest>
#include <QSignalSpy>
#include "ui/HomeViewModel.hpp"

using namespace blueplayer::ui;

class TestHomeViewModel : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  
  void testConstructor();
  void testFollowedStreams();
  void testPlaceholderCards();
  void testSectionsData();
  void testTransformTwitchStreams();
  void testUpdateFollowedStreams();
};

void TestHomeViewModel::initTestCase() {
}

void TestHomeViewModel::cleanupTestCase() {
}

void TestHomeViewModel::testConstructor() {
  HomeViewModel viewModel;
  QVERIFY(viewModel.followedStreams().isEmpty());
  QVERIFY(!viewModel.placeholderCards().isEmpty());
}

void TestHomeViewModel::testFollowedStreams() {
  HomeViewModel viewModel;
  QVERIFY(viewModel.followedStreams().isEmpty());
  
  // Après mise à jour avec des streams, la liste ne devrait plus être vide
  QVariantList twitchStreams;
  QVariantMap stream1;
  stream1["user_name"] = "test_user1";
  stream1["title"] = "Test Stream 1";
  stream1["viewer_count"] = 100;
  twitchStreams.append(stream1);
  
  viewModel.updateFollowedStreams(twitchStreams);
  QVERIFY(!viewModel.followedStreams().isEmpty());
}

void TestHomeViewModel::testPlaceholderCards() {
  HomeViewModel viewModel;
  QVariantList placeholders = viewModel.placeholderCards();
  
  QVERIFY(!placeholders.isEmpty());
  // Les placeholders devraient avoir une structure cohérente
  QVERIFY(placeholders.size() > 0);
}

void TestHomeViewModel::testSectionsData() {
  HomeViewModel viewModel;
  QVariantList sections = viewModel.sectionsData();
  
  // Les sections devraient être initialisées
  QVERIFY(!sections.isEmpty());
}

void TestHomeViewModel::testTransformTwitchStreams() {
  HomeViewModel viewModel;
  
  QVariantList twitchStreams;
  QVariantMap stream1;
  stream1["user_name"] = "test_user";
  stream1["title"] = "Test Stream";
  stream1["viewer_count"] = 50;
  stream1["thumbnail_url"] = "https://example.com/thumb.jpg";
  twitchStreams.append(stream1);
  
  QVariantList transformed = viewModel.transformTwitchStreams(twitchStreams);
  
  QCOMPARE(transformed.size(), 1);
  QVERIFY(!transformed.isEmpty());
  
  QVariantMap transformedStream = transformed[0].toMap();
  QVERIFY(transformedStream.contains("name"));
  QVERIFY(transformedStream.contains("detail"));
  QVERIFY(transformedStream.contains("viewers"));
}

void TestHomeViewModel::testUpdateFollowedStreams() {
  HomeViewModel viewModel;
  QSignalSpy spy(&viewModel, &HomeViewModel::followedStreamsChanged);
  
  QVariantList twitchStreams;
  QVariantMap stream1;
  stream1["user_name"] = "test_user1";
  stream1["title"] = "Test Stream 1";
  stream1["viewer_count"] = 100;
  twitchStreams.append(stream1);
  
  viewModel.updateFollowedStreams(twitchStreams);
  
  QCOMPARE(spy.count(), 1);
  QVERIFY(!viewModel.followedStreams().isEmpty());
}

QTEST_MAIN(TestHomeViewModel)
#include "TestHomeViewModel.moc"






