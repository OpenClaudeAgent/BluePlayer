#include <QtTest/QtTest>
#include <QSignalSpy>
#include "ui/HomeViewModel.hpp"

using namespace blueplayer::ui;

class TestHomeViewModel : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();
  
  // Tests d'état initial
  void testConstructor();
  void testInitialFollowedStreamsEmpty();
  void testInitialRecommendedStreamsEmpty();
  void testInitialCategoriesEmpty();
  void testInitialClipsEmpty();
  void testInitialVideosEmpty();
  void testInitialChannelsEmpty();
  
  // Tests des placeholders
  void testPlaceholderCards();
  void testPlaceholderCardsNotEmpty();
  
  // Tests des sections
  void testSectionsData();
  void testSectionsDataNotEmpty();
  
  // Tests de transformation
  void testTransformTwitchStreams();
  void testTransformTwitchStreamsEmpty();
  void testTransformTwitchStreamsMultiple();
  void testTransformTwitchStreamsFields();
  
  // Tests de mise à jour - Followed Streams
  void testUpdateFollowedStreams();
  void testUpdateFollowedStreamsEmitsSignal();
  void testUpdateFollowedStreamsEmpty();
  
  // Tests de mise à jour - Recommended Streams
  void testUpdateRecommendedStreams();
  void testUpdateRecommendedStreamsEmitsSignal();
  
  // Tests de mise à jour - Categories
  void testUpdateCategories();
  void testUpdateCategoriesEmitsSignal();
  
  // Tests de mise à jour - Popular Clips
  void testUpdatePopularClips();
  void testUpdatePopularClipsEmitsSignal();
  
  // Tests de mise à jour - Followed Clips
  void testUpdateFollowedClips();
  void testUpdateFollowedClipsEmitsSignal();
  
  // Tests de mise à jour - Videos
  void testUpdateVideos();
  void testUpdateVideosEmitsSignal();
  
  // Tests de mise à jour - Followed Channels
  void testUpdateFollowedChannels();
  void testUpdateFollowedChannelsEmitsSignal();
  
  // Tests de mise à jour - New Streamers
  void testUpdateNewStreamers();
  void testUpdateNewStreamersEmitsSignal();
  
  // Tests de mise à jour - Category Streams
  void testUpdateCategoryStreams();
  void testUpdateCategoryStreamsEmitsSignal();

private:
  HomeViewModel* m_viewModel = nullptr;
  
  QVariantMap createTestStream(const QString& userName, const QString& title, int viewers);
  QVariantMap createTestCategory(const QString& name, const QString& id);
  QVariantMap createTestClip(const QString& title, const QString& broadcaster);
  QVariantMap createTestVideo(const QString& title, const QString& userName);
  QVariantMap createTestChannel(const QString& displayName, const QString& login);
};

void TestHomeViewModel::initTestCase() {
}

void TestHomeViewModel::cleanupTestCase() {
}

void TestHomeViewModel::init() {
  m_viewModel = new HomeViewModel();
}

void TestHomeViewModel::cleanup() {
  delete m_viewModel;
  m_viewModel = nullptr;
}

QVariantMap TestHomeViewModel::createTestStream(const QString& userName, const QString& title, int viewers) {
  QVariantMap stream;
  stream["user_name"] = userName;
  stream["title"] = title;
  stream["viewer_count"] = viewers;
  stream["thumbnail_url"] = "https://example.com/thumb.jpg";
  stream["game_name"] = "Test Game";
  stream["user_login"] = userName.toLower();
  return stream;
}

QVariantMap TestHomeViewModel::createTestCategory(const QString& name, const QString& id) {
  QVariantMap category;
  category["name"] = name;
  category["id"] = id;
  category["box_art_url"] = "https://example.com/boxart.jpg";
  return category;
}

QVariantMap TestHomeViewModel::createTestClip(const QString& title, const QString& broadcaster) {
  QVariantMap clip;
  clip["title"] = title;
  clip["broadcaster_name"] = broadcaster;
  clip["view_count"] = 1000;
  clip["thumbnail_url"] = "https://example.com/clip.jpg";
  clip["url"] = "https://clips.twitch.tv/test";
  return clip;
}

QVariantMap TestHomeViewModel::createTestVideo(const QString& title, const QString& userName) {
  QVariantMap video;
  video["title"] = title;
  video["user_name"] = userName;
  video["view_count"] = 5000;
  video["thumbnail_url"] = "https://example.com/video.jpg";
  video["duration"] = "1h30m";
  return video;
}

QVariantMap TestHomeViewModel::createTestChannel(const QString& displayName, const QString& login) {
  QVariantMap channel;
  channel["display_name"] = displayName;
  channel["broadcaster_login"] = login;
  channel["thumbnail_url"] = "https://example.com/channel.jpg";
  channel["is_live"] = false;
  return channel;
}

// ===== Tests d'état initial =====

void TestHomeViewModel::testConstructor() {
  QVERIFY(m_viewModel != nullptr);
  QVERIFY(m_viewModel->followedStreams().isEmpty());
  QVERIFY(!m_viewModel->placeholderCards().isEmpty());
}

void TestHomeViewModel::testInitialFollowedStreamsEmpty() {
  QVERIFY(m_viewModel->followedStreams().isEmpty());
}

void TestHomeViewModel::testInitialRecommendedStreamsEmpty() {
  QVERIFY(m_viewModel->recommendedStreams().isEmpty());
}

void TestHomeViewModel::testInitialCategoriesEmpty() {
  QVERIFY(m_viewModel->categories().isEmpty());
}

void TestHomeViewModel::testInitialClipsEmpty() {
  QVERIFY(m_viewModel->popularClips().isEmpty());
  QVERIFY(m_viewModel->followedClips().isEmpty());
}

void TestHomeViewModel::testInitialVideosEmpty() {
  QVERIFY(m_viewModel->videos().isEmpty());
}

void TestHomeViewModel::testInitialChannelsEmpty() {
  QVERIFY(m_viewModel->followedChannels().isEmpty());
  QVERIFY(m_viewModel->newStreamers().isEmpty());
  QVERIFY(m_viewModel->categoryStreams().isEmpty());
}

// ===== Tests des placeholders =====

void TestHomeViewModel::testPlaceholderCards() {
  QVariantList placeholders = m_viewModel->placeholderCards();
  QVERIFY(!placeholders.isEmpty());
}

void TestHomeViewModel::testPlaceholderCardsNotEmpty() {
  QVariantList placeholders = m_viewModel->placeholderCards();
  QVERIFY(placeholders.size() > 0);
}

// ===== Tests des sections =====

void TestHomeViewModel::testSectionsData() {
  QVariantList sections = m_viewModel->sectionsData();
  QVERIFY(!sections.isEmpty());
}

void TestHomeViewModel::testSectionsDataNotEmpty() {
  QVariantList sections = m_viewModel->sectionsData();
  QVERIFY(sections.count() > 0);
}

// ===== Tests de transformation =====

void TestHomeViewModel::testTransformTwitchStreams() {
  QVariantList twitchStreams;
  twitchStreams.append(createTestStream("test_user", "Test Stream", 50));
  
  QVariantList transformed = m_viewModel->transformTwitchStreams(twitchStreams);
  
  QCOMPARE(transformed.size(), 1);
  QVERIFY(!transformed.isEmpty());
}

void TestHomeViewModel::testTransformTwitchStreamsEmpty() {
  QVariantList emptyList;
  QVariantList transformed = m_viewModel->transformTwitchStreams(emptyList);
  
  QVERIFY(transformed.isEmpty());
}

void TestHomeViewModel::testTransformTwitchStreamsMultiple() {
  QVariantList twitchStreams;
  twitchStreams.append(createTestStream("user1", "Stream 1", 100));
  twitchStreams.append(createTestStream("user2", "Stream 2", 200));
  twitchStreams.append(createTestStream("user3", "Stream 3", 300));
  
  QVariantList transformed = m_viewModel->transformTwitchStreams(twitchStreams);
  
  QCOMPARE(transformed.size(), 3);
}

void TestHomeViewModel::testTransformTwitchStreamsFields() {
  QVariantList twitchStreams;
  twitchStreams.append(createTestStream("test_user", "Test Stream", 50));
  
  QVariantList transformed = m_viewModel->transformTwitchStreams(twitchStreams);
  
  QCOMPARE(transformed.size(), 1);
  
  QVariantMap transformedStream = transformed[0].toMap();
  QVERIFY(transformedStream.contains("name"));
  QVERIFY(transformedStream.contains("detail"));
  QVERIFY(transformedStream.contains("viewers"));
}

// ===== Tests Followed Streams =====

void TestHomeViewModel::testUpdateFollowedStreams() {
  QVariantList twitchStreams;
  twitchStreams.append(createTestStream("test_user1", "Test Stream 1", 100));
  
  m_viewModel->updateFollowedStreams(twitchStreams);
  
  QVERIFY(!m_viewModel->followedStreams().isEmpty());
  QCOMPARE(m_viewModel->followedStreams().size(), 1);
}

void TestHomeViewModel::testUpdateFollowedStreamsEmitsSignal() {
  QSignalSpy spy(m_viewModel, &HomeViewModel::followedStreamsChanged);
  
  QVariantList twitchStreams;
  twitchStreams.append(createTestStream("test_user1", "Test Stream 1", 100));
  
  m_viewModel->updateFollowedStreams(twitchStreams);
  
  QCOMPARE(spy.count(), 1);
}

void TestHomeViewModel::testUpdateFollowedStreamsEmpty() {
  // D'abord ajouter des streams
  QVariantList twitchStreams;
  twitchStreams.append(createTestStream("user", "Stream", 50));
  m_viewModel->updateFollowedStreams(twitchStreams);
  
  // Puis mettre à jour avec une liste vide
  QVariantList emptyList;
  m_viewModel->updateFollowedStreams(emptyList);
  
  QVERIFY(m_viewModel->followedStreams().isEmpty());
}

// ===== Tests Recommended Streams =====

void TestHomeViewModel::testUpdateRecommendedStreams() {
  QVariantList twitchStreams;
  twitchStreams.append(createTestStream("recommended_user", "Recommended Stream", 500));
  
  m_viewModel->updateRecommendedStreams(twitchStreams);
  
  QVERIFY(!m_viewModel->recommendedStreams().isEmpty());
}

void TestHomeViewModel::testUpdateRecommendedStreamsEmitsSignal() {
  QSignalSpy spy(m_viewModel, &HomeViewModel::recommendedStreamsChanged);
  
  QVariantList twitchStreams;
  twitchStreams.append(createTestStream("recommended_user", "Recommended Stream", 500));
  
  m_viewModel->updateRecommendedStreams(twitchStreams);
  
  QCOMPARE(spy.count(), 1);
}

// ===== Tests Categories =====

void TestHomeViewModel::testUpdateCategories() {
  QVariantList categories;
  categories.append(createTestCategory("Just Chatting", "509658"));
  categories.append(createTestCategory("Fortnite", "33214"));
  
  m_viewModel->updateCategories(categories);
  
  QVERIFY(!m_viewModel->categories().isEmpty());
  QCOMPARE(m_viewModel->categories().size(), 2);
}

void TestHomeViewModel::testUpdateCategoriesEmitsSignal() {
  QSignalSpy spy(m_viewModel, &HomeViewModel::categoriesChanged);
  
  QVariantList categories;
  categories.append(createTestCategory("Just Chatting", "509658"));
  
  m_viewModel->updateCategories(categories);
  
  QCOMPARE(spy.count(), 1);
}

// ===== Tests Popular Clips =====

void TestHomeViewModel::testUpdatePopularClips() {
  QVariantList clips;
  clips.append(createTestClip("Epic Moment", "PopularStreamer"));
  
  m_viewModel->updatePopularClips(clips);
  
  QVERIFY(!m_viewModel->popularClips().isEmpty());
}

void TestHomeViewModel::testUpdatePopularClipsEmitsSignal() {
  QSignalSpy spy(m_viewModel, &HomeViewModel::popularClipsChanged);
  
  QVariantList clips;
  clips.append(createTestClip("Epic Moment", "PopularStreamer"));
  
  m_viewModel->updatePopularClips(clips);
  
  QCOMPARE(spy.count(), 1);
}

// ===== Tests Followed Clips =====

void TestHomeViewModel::testUpdateFollowedClips() {
  QVariantList clips;
  clips.append(createTestClip("Funny Clip", "FollowedStreamer"));
  
  m_viewModel->updateFollowedClips(clips);
  
  QVERIFY(!m_viewModel->followedClips().isEmpty());
}

void TestHomeViewModel::testUpdateFollowedClipsEmitsSignal() {
  QSignalSpy spy(m_viewModel, &HomeViewModel::followedClipsChanged);
  
  QVariantList clips;
  clips.append(createTestClip("Funny Clip", "FollowedStreamer"));
  
  m_viewModel->updateFollowedClips(clips);
  
  QCOMPARE(spy.count(), 1);
}

// ===== Tests Videos =====

void TestHomeViewModel::testUpdateVideos() {
  QVariantList videos;
  videos.append(createTestVideo("Full Stream VOD", "Streamer123"));
  
  m_viewModel->updateVideos(videos);
  
  QVERIFY(!m_viewModel->videos().isEmpty());
}

void TestHomeViewModel::testUpdateVideosEmitsSignal() {
  QSignalSpy spy(m_viewModel, &HomeViewModel::videosChanged);
  
  QVariantList videos;
  videos.append(createTestVideo("Full Stream VOD", "Streamer123"));
  
  m_viewModel->updateVideos(videos);
  
  QCOMPARE(spy.count(), 1);
}

// ===== Tests Followed Channels =====

void TestHomeViewModel::testUpdateFollowedChannels() {
  QVariantList channels;
  channels.append(createTestChannel("CoolStreamer", "coolstreamer"));
  
  m_viewModel->updateFollowedChannels(channels);
  
  QVERIFY(!m_viewModel->followedChannels().isEmpty());
}

void TestHomeViewModel::testUpdateFollowedChannelsEmitsSignal() {
  QSignalSpy spy(m_viewModel, &HomeViewModel::followedChannelsChanged);
  
  QVariantList channels;
  channels.append(createTestChannel("CoolStreamer", "coolstreamer"));
  
  m_viewModel->updateFollowedChannels(channels);
  
  QCOMPARE(spy.count(), 1);
}

// ===== Tests New Streamers =====

void TestHomeViewModel::testUpdateNewStreamers() {
  QVariantList streamers;
  streamers.append(createTestStream("new_streamer", "First Stream!", 10));
  
  m_viewModel->updateNewStreamers(streamers);
  
  QVERIFY(!m_viewModel->newStreamers().isEmpty());
}

void TestHomeViewModel::testUpdateNewStreamersEmitsSignal() {
  QSignalSpy spy(m_viewModel, &HomeViewModel::newStreamersChanged);
  
  QVariantList streamers;
  streamers.append(createTestStream("new_streamer", "First Stream!", 10));
  
  m_viewModel->updateNewStreamers(streamers);
  
  QCOMPARE(spy.count(), 1);
}

// ===== Tests Category Streams =====

void TestHomeViewModel::testUpdateCategoryStreams() {
  QVariantList streams;
  streams.append(createTestStream("category_streamer", "Playing Game X", 1000));
  
  m_viewModel->updateCategoryStreams(streams);
  
  QVERIFY(!m_viewModel->categoryStreams().isEmpty());
}

void TestHomeViewModel::testUpdateCategoryStreamsEmitsSignal() {
  QSignalSpy spy(m_viewModel, &HomeViewModel::categoryStreamsChanged);
  
  QVariantList streams;
  streams.append(createTestStream("category_streamer", "Playing Game X", 1000));
  
  m_viewModel->updateCategoryStreams(streams);
  
  QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestHomeViewModel)
#include "TestHomeViewModel.moc"
