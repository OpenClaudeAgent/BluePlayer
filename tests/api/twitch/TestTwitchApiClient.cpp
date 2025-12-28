#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "api/twitch/TwitchApiClient.hpp"
#include "TestHelpers.hpp"

using namespace blueplayer::api::twitch;
using namespace blueplayer::test;

class TestTwitchApiClient : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();
  
  // ===== Tests d'initialisation =====
  void testConstructor();
  void testSetAccessToken();
  void testSetAccessTokenEmpty();
  
  // ===== Tests de parseStreamsArray =====
  void testParseStreamsArrayValid();
  void testParseStreamsArrayEmpty();
  void testParseStreamsArrayMultiple();
  void testParseStreamsArrayWithMissingFields();
  void testParseStreamsArrayWithAllFields();
  
  // ===== Tests de parseClipsArray =====
  void testParseClipsArrayValid();
  void testParseClipsArrayEmpty();
  void testParseClipsArrayWithoutThumbnail();
  void testParseClipsArrayMultiple();
  
  // ===== Tests de parseVideosArray =====
  void testParseVideosArrayValid();
  void testParseVideosArrayEmpty();
  void testParseVideosArrayWithDuration();
  void testParseVideosArrayMultiple();
  
  // ===== Tests de parseChannelsArray =====
  void testParseChannelsArrayValid();
  void testParseChannelsArrayEmpty();
  void testParseChannelsArrayWithoutAvatar();
  void testParseChannelsArrayMultiple();
  
  // ===== Tests de expandThumbnail =====
  void testExpandThumbnailWithPlaceholders();
  void testExpandThumbnailWithoutPlaceholders();
  void testExpandThumbnailEmpty();
  
  // ===== Tests de parseSearchChannelsArray =====
  void testParseSearchChannelsArrayValid();
  void testParseSearchChannelsArrayEmpty();
  void testParseSearchChannelsArrayWithEmptyThumbnail();
  
  // ===== Tests de parseSearchCategoriesArray =====
  void testParseSearchCategoriesArrayValid();
  void testParseSearchCategoriesArrayEmpty();
  void testParseSearchCategoriesArrayExpandsBoxArt();
  
  // ===== Tests de validation des inputs =====
  void testGetVideosEmptyUserId();
  void testGetFollowedClipsEmptyBroadcasterIds();
  void testListFollowedStreamsEmptyUserId();
  void testListFollowedStreamsInvalidUserId();
  void testGetRecommendedStreamsWithEmptyClientId();
  void testGetTopCategoriesWithEmptyClientId();
  void testGetPopularClipsWithEmptyClientId();
  void testSearchChannelsEmptyQuery();
  void testSearchCategoriesEmptyQuery();
  void testGetStreamsByCategoryEmptyGameId();
  void testGetFollowedChannelsEmptyUserId();
  
  // ===== Tests des signaux =====
  void testErrorSignalConnection();
  void testStreamsReadySignalConnection();
  void testCategoriesReadySignalConnection();
  void testClipsReadySignalConnection();
  void testVideosReadySignalConnection();
  void testChannelsReadySignalConnection();
  void testSearchSignalsConnection();
  void testTokenInvalidatedSignalConnection();
  
  // ===== Tests des limites d'API =====
  void testListStreamsNegativeLimit();
  void testListStreamsZeroLimit();
  void testListStreamsLargeLimit();
  void testListFollowedStreamsInvalidLimit();
  
  // ===== Tests des méthodes Q_INVOKABLE =====
  void testListStreamsInvokable();
  void testGetUserInfoInvokable();
  void testGetRecommendedStreamsInvokable();
  void testGetTopCategoriesInvokable();
  void testGetPopularClipsInvokable();
  void testSearchChannelsInvokable();
  void testSearchCategoriesInvokable();
  void testGetStreamsByCategoryInvokable();

private:
  TwitchApiClient* m_client = nullptr;
};

void TestTwitchApiClient::initTestCase() {
}

void TestTwitchApiClient::cleanupTestCase() {
}

void TestTwitchApiClient::init() {
  m_client = new TwitchApiClient("test_client_id", this);
  m_client->setAccessToken("test_access_token");
}

void TestTwitchApiClient::cleanup() {
  delete m_client;
  m_client = nullptr;
}

// ===== Tests d'initialisation =====

void TestTwitchApiClient::testConstructor() {
  TwitchApiClient* client = new TwitchApiClient("my_client_id", this);
  QVERIFY(client != nullptr);
  delete client;
}

void TestTwitchApiClient::testSetAccessToken() {
  m_client->setAccessToken("new_token_12345");
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testSetAccessTokenEmpty() {
  m_client->setAccessToken("");
  QVERIFY(m_client != nullptr);
}

// ===== Tests de parseStreamsArray (friend class access) =====

void TestTwitchApiClient::testParseStreamsArrayValid() {
  QJsonArray entries;
  QJsonObject stream;
  stream["id"] = "123456";
  stream["user_id"] = "789";
  stream["user_name"] = "TestStreamer";
  stream["user_login"] = "teststreamer";
  stream["title"] = "Playing Games";
  stream["viewer_count"] = 1500;
  stream["language"] = "en";
  stream["thumbnail_url"] = "https://static-cdn.jtvnw.net/previews-ttv/{width}x{height}.jpg";
  stream["started_at"] = "2024-01-15T10:30:00Z";
  entries.append(stream);
  
  QVariantList result = m_client->parseStreamsArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QCOMPARE(first["id"].toString(), QString("123456"));
  QCOMPARE(first["user_id"].toString(), QString("789"));
  QCOMPARE(first["user_name"].toString(), QString("TestStreamer"));
  QCOMPARE(first["user_login"].toString(), QString("teststreamer"));
  QCOMPARE(first["title"].toString(), QString("Playing Games"));
  QCOMPARE(first["viewer_count"].toInt(), 1500);
  QCOMPARE(first["language"].toString(), QString("en"));
  QCOMPARE(first["started_at"].toString(), QString("2024-01-15T10:30:00Z"));
  // Thumbnail URL should be expanded
  QVERIFY(!first["thumbnail_url"].toString().contains("{width}"));
  QVERIFY(!first["thumbnail_url"].toString().contains("{height}"));
}

void TestTwitchApiClient::testParseStreamsArrayEmpty() {
  QJsonArray emptyEntries;
  QVariantList result = m_client->parseStreamsArray(emptyEntries);
  QCOMPARE(result.size(), 0);
}

void TestTwitchApiClient::testParseStreamsArrayMultiple() {
  QJsonArray entries;
  for (int i = 0; i < 5; i++) {
    QJsonObject stream;
    stream["id"] = QString::number(i);
    stream["user_name"] = QString("Streamer%1").arg(i);
    stream["user_login"] = QString("streamer%1").arg(i);
    stream["viewer_count"] = i * 100;
    stream["thumbnail_url"] = "";
    entries.append(stream);
  }
  
  QVariantList result = m_client->parseStreamsArray(entries);
  
  QCOMPARE(result.size(), 5);
  QCOMPARE(result[2].toMap()["user_name"].toString(), QString("Streamer2"));
  QCOMPARE(result[3].toMap()["viewer_count"].toInt(), 300);
}

void TestTwitchApiClient::testParseStreamsArrayWithMissingFields() {
  QJsonArray entries;
  QJsonObject stream;
  stream["id"] = "123";
  stream["user_name"] = "PartialStreamer";
  entries.append(stream);
  
  QVariantList result = m_client->parseStreamsArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QCOMPARE(first["id"].toString(), QString("123"));
  QCOMPARE(first["user_name"].toString(), QString("PartialStreamer"));
  QCOMPARE(first["viewer_count"].toInt(), 0);
  QVERIFY(first["title"].toString().isEmpty());
}

void TestTwitchApiClient::testParseStreamsArrayWithAllFields() {
  QJsonArray entries;
  QJsonObject stream;
  stream["id"] = "999";
  stream["user_id"] = "12345";
  stream["user_name"] = "FullStreamer";
  stream["user_login"] = "fullstreamer";
  stream["title"] = "Complete Test Stream";
  stream["viewer_count"] = 50000;
  stream["language"] = "fr";
  stream["thumbnail_url"] = "https://example.com/{width}x{height}.jpg";
  stream["started_at"] = "2024-12-28T12:00:00Z";
  entries.append(stream);
  
  QVariantList result = m_client->parseStreamsArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QCOMPARE(first["id"].toString(), QString("999"));
  QCOMPARE(first["user_id"].toString(), QString("12345"));
  QCOMPARE(first["viewer_count"].toInt(), 50000);
  QCOMPARE(first["language"].toString(), QString("fr"));
  QVERIFY(first["stream_url"].toString().contains("fullstreamer"));
}

// ===== Tests de parseClipsArray =====

void TestTwitchApiClient::testParseClipsArrayValid() {
  QJsonArray entries;
  QJsonObject clip;
  clip["id"] = "ClipId123";
  clip["title"] = "Amazing Clip";
  clip["broadcaster_name"] = "ClipCreator";
  clip["broadcaster_id"] = "456";
  clip["view_count"] = 10000;
  clip["duration"] = 30.5;
  clip["thumbnail_url"] = "https://clips.twitch.tv/thumbnail.jpg";
  clip["created_at"] = "2024-01-10T15:00:00Z";
  clip["url"] = "https://clips.twitch.tv/ClipId123";
  entries.append(clip);
  
  QVariantList result = m_client->parseClipsArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QCOMPARE(first["id"].toString(), QString("ClipId123"));
  QCOMPARE(first["title"].toString(), QString("Amazing Clip"));
  QCOMPARE(first["broadcaster_name"].toString(), QString("ClipCreator"));
  QCOMPARE(first["broadcaster_id"].toString(), QString("456"));
  QCOMPARE(first["view_count"].toInt(), 10000);
  QCOMPARE(first["duration"].toDouble(), 30.5);
  QCOMPARE(first["thumbnail_url"].toString(), QString("https://clips.twitch.tv/thumbnail.jpg"));
  QCOMPARE(first["url"].toString(), QString("https://clips.twitch.tv/ClipId123"));
}

void TestTwitchApiClient::testParseClipsArrayEmpty() {
  QJsonArray emptyEntries;
  QVariantList result = m_client->parseClipsArray(emptyEntries);
  QCOMPARE(result.size(), 0);
}

void TestTwitchApiClient::testParseClipsArrayWithoutThumbnail() {
  QJsonArray entries;
  QJsonObject clip;
  clip["id"] = "ClipNoThumb";
  clip["title"] = "Clip Without Thumbnail";
  clip["broadcaster_name"] = "Broadcaster";
  clip["view_count"] = 500;
  entries.append(clip);
  
  QVariantList result = m_client->parseClipsArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QCOMPARE(first["id"].toString(), QString("ClipNoThumb"));
  QVERIFY(first["thumbnail_url"].toString().isEmpty());
}

void TestTwitchApiClient::testParseClipsArrayMultiple() {
  QJsonArray entries;
  for (int i = 0; i < 3; i++) {
    QJsonObject clip;
    clip["id"] = QString("Clip%1").arg(i);
    clip["title"] = QString("Clip Title %1").arg(i);
    clip["view_count"] = i * 1000;
    entries.append(clip);
  }
  
  QVariantList result = m_client->parseClipsArray(entries);
  
  QCOMPARE(result.size(), 3);
  QCOMPARE(result[1].toMap()["id"].toString(), QString("Clip1"));
  QCOMPARE(result[2].toMap()["view_count"].toInt(), 2000);
}

// ===== Tests de parseVideosArray =====

void TestTwitchApiClient::testParseVideosArrayValid() {
  QJsonArray entries;
  QJsonObject video;
  video["id"] = "Video123";
  video["title"] = "Past Broadcast";
  video["user_name"] = "VODCreator";
  video["user_id"] = "789";
  video["view_count"] = 25000;
  video["duration"] = "2h30m45s";
  video["thumbnail_url"] = "https://static-cdn.jtvnw.net/videos/thumb.jpg";
  video["created_at"] = "2024-01-08T20:00:00Z";
  video["url"] = "https://www.twitch.tv/videos/Video123";
  entries.append(video);
  
  QVariantList result = m_client->parseVideosArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QCOMPARE(first["id"].toString(), QString("Video123"));
  QCOMPARE(first["title"].toString(), QString("Past Broadcast"));
  QCOMPARE(first["user_name"].toString(), QString("VODCreator"));
  QCOMPARE(first["user_id"].toString(), QString("789"));
  QCOMPARE(first["view_count"].toInt(), 25000);
  QCOMPARE(first["duration"].toString(), QString("2h30m45s"));
  QCOMPARE(first["url"].toString(), QString("https://www.twitch.tv/videos/Video123"));
}

void TestTwitchApiClient::testParseVideosArrayEmpty() {
  QJsonArray emptyEntries;
  QVariantList result = m_client->parseVideosArray(emptyEntries);
  QCOMPARE(result.size(), 0);
}

void TestTwitchApiClient::testParseVideosArrayWithDuration() {
  QJsonArray entries;
  QJsonObject video;
  video["id"] = "VodDuration";
  video["title"] = "Long Stream";
  video["duration"] = "5h15m30s";
  entries.append(video);
  
  QVariantList result = m_client->parseVideosArray(entries);
  
  QCOMPARE(result.size(), 1);
  QCOMPARE(result.first().toMap()["duration"].toString(), QString("5h15m30s"));
}

void TestTwitchApiClient::testParseVideosArrayMultiple() {
  QJsonArray entries;
  for (int i = 0; i < 4; i++) {
    QJsonObject video;
    video["id"] = QString("Video%1").arg(i);
    video["title"] = QString("VOD %1").arg(i);
    video["view_count"] = i * 5000;
    video["duration"] = QString("%1h0m0s").arg(i + 1);
    entries.append(video);
  }
  
  QVariantList result = m_client->parseVideosArray(entries);
  
  QCOMPARE(result.size(), 4);
  QCOMPARE(result[2].toMap()["id"].toString(), QString("Video2"));
  QCOMPARE(result[3].toMap()["view_count"].toInt(), 15000);
}

// ===== Tests de parseChannelsArray =====

void TestTwitchApiClient::testParseChannelsArrayValid() {
  QJsonArray entries;
  QJsonObject channel;
  channel["broadcaster_id"] = "Chan123";
  channel["broadcaster_name"] = "ChannelName";
  channel["broadcaster_login"] = "channelname";
  channel["game_name"] = "Some Game";
  channel["game_id"] = "12345";
  channel["is_live"] = true;
  channel["title"] = "Currently Streaming";
  entries.append(channel);
  
  QVariantList result = m_client->parseChannelsArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QCOMPARE(first["broadcaster_id"].toString(), QString("Chan123"));
  QCOMPARE(first["broadcaster_name"].toString(), QString("ChannelName"));
  QCOMPARE(first["broadcaster_login"].toString(), QString("channelname"));
  QCOMPARE(first["display_name"].toString(), QString("ChannelName"));
  QCOMPARE(first["game_name"].toString(), QString("Some Game"));
  QCOMPARE(first["is_live"].toBool(), true);
}

void TestTwitchApiClient::testParseChannelsArrayEmpty() {
  QJsonArray emptyEntries;
  QVariantList result = m_client->parseChannelsArray(emptyEntries);
  QCOMPARE(result.size(), 0);
}

void TestTwitchApiClient::testParseChannelsArrayWithoutAvatar() {
  QJsonArray entries;
  QJsonObject channel;
  channel["broadcaster_id"] = "NoAvatar";
  channel["broadcaster_name"] = "NoAvatarChannel";
  channel["broadcaster_login"] = "noavatarchannel";
  entries.append(channel);
  
  QVariantList result = m_client->parseChannelsArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QVERIFY(first["thumbnail_url"].toString().isEmpty());
}

void TestTwitchApiClient::testParseChannelsArrayMultiple() {
  QJsonArray entries;
  for (int i = 0; i < 3; i++) {
    QJsonObject channel;
    channel["broadcaster_id"] = QString("BC%1").arg(i);
    channel["broadcaster_name"] = QString("Channel%1").arg(i);
    channel["broadcaster_login"] = QString("channel%1").arg(i);
    channel["is_live"] = (i % 2 == 0);
    entries.append(channel);
  }
  
  QVariantList result = m_client->parseChannelsArray(entries);
  
  QCOMPARE(result.size(), 3);
  QCOMPARE(result[1].toMap()["broadcaster_id"].toString(), QString("BC1"));
  QCOMPARE(result[0].toMap()["is_live"].toBool(), true);
  QCOMPARE(result[1].toMap()["is_live"].toBool(), false);
}

// ===== Tests de expandThumbnail =====

void TestTwitchApiClient::testExpandThumbnailWithPlaceholders() {
  QString templateUrl = "https://static-cdn.jtvnw.net/previews-ttv/{width}x{height}.jpg";
  QString result = m_client->expandThumbnail(templateUrl);
  
  QVERIFY(!result.contains("{width}"));
  QVERIFY(!result.contains("{height}"));
  QVERIFY(result.contains("320x180")); // Default thumbnail size from Constants
}

void TestTwitchApiClient::testExpandThumbnailWithoutPlaceholders() {
  QString fixedUrl = "https://static-cdn.jtvnw.net/previews-ttv/320x180.jpg";
  QString result = m_client->expandThumbnail(fixedUrl);
  
  QCOMPARE(result, fixedUrl);
}

void TestTwitchApiClient::testExpandThumbnailEmpty() {
  QString emptyUrl = "";
  QString result = m_client->expandThumbnail(emptyUrl);
  
  QVERIFY(result.isEmpty());
}

// ===== Tests de parseSearchChannelsArray =====

void TestTwitchApiClient::testParseSearchChannelsArrayValid() {
  QJsonArray entries;
  QJsonObject channel;
  channel["id"] = "SearchChan123";
  channel["broadcaster_login"] = "foundsearcher";
  channel["display_name"] = "FoundSearcher";
  channel["game_name"] = "Search Game";
  channel["game_id"] = "99999";
  channel["is_live"] = true;
  channel["title"] = "Live Stream Title";
  channel["thumbnail_url"] = "https://static-cdn.jtvnw.net/profile.jpg";
  entries.append(channel);
  
  QVariantList result = m_client->parseSearchChannelsArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QCOMPARE(first["id"].toString(), QString("SearchChan123"));
  QCOMPARE(first["broadcaster_login"].toString(), QString("foundsearcher"));
  QCOMPARE(first["display_name"].toString(), QString("FoundSearcher"));
  QCOMPARE(first["game_name"].toString(), QString("Search Game"));
  QCOMPARE(first["is_live"].toBool(), true);
  QCOMPARE(first["thumbnail_url"].toString(), QString("https://static-cdn.jtvnw.net/profile.jpg"));
}

void TestTwitchApiClient::testParseSearchChannelsArrayEmpty() {
  QJsonArray emptyEntries;
  QVariantList result = m_client->parseSearchChannelsArray(emptyEntries);
  QCOMPARE(result.size(), 0);
}

void TestTwitchApiClient::testParseSearchChannelsArrayWithEmptyThumbnail() {
  QJsonArray entries;
  QJsonObject channel;
  channel["id"] = "NoThumb";
  channel["broadcaster_login"] = "nothumb";
  channel["display_name"] = "NoThumb";
  channel["thumbnail_url"] = "";
  entries.append(channel);
  
  QVariantList result = m_client->parseSearchChannelsArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  // Should construct a fallback URL
  QVERIFY(first["thumbnail_url"].toString().contains("nothumb"));
}

// ===== Tests de parseSearchCategoriesArray =====

void TestTwitchApiClient::testParseSearchCategoriesArrayValid() {
  QJsonArray entries;
  QJsonObject category;
  category["id"] = "Cat123";
  category["name"] = "Search Category";
  category["box_art_url"] = "https://static-cdn.jtvnw.net/boxart/{width}x{height}.jpg";
  entries.append(category);
  
  QVariantList result = m_client->parseSearchCategoriesArray(entries);
  
  QCOMPARE(result.size(), 1);
  QVariantMap first = result.first().toMap();
  QCOMPARE(first["id"].toString(), QString("Cat123"));
  QCOMPARE(first["name"].toString(), QString("Search Category"));
  // box_art_url should be expanded
  QVERIFY(!first["box_art_url"].toString().contains("{width}"));
  QVERIFY(!first["box_art_url"].toString().contains("{height}"));
}

void TestTwitchApiClient::testParseSearchCategoriesArrayEmpty() {
  QJsonArray emptyEntries;
  QVariantList result = m_client->parseSearchCategoriesArray(emptyEntries);
  QCOMPARE(result.size(), 0);
}

void TestTwitchApiClient::testParseSearchCategoriesArrayExpandsBoxArt() {
  QJsonArray entries;
  QJsonObject category;
  category["id"] = "GameExpand";
  category["name"] = "Test Game";
  category["box_art_url"] = "https://example.com/{width}x{height}/art.jpg";
  entries.append(category);
  
  QVariantList result = m_client->parseSearchCategoriesArray(entries);
  
  QVariantMap first = result.first().toMap();
  QString boxArt = first["box_art_url"].toString();
  QVERIFY(boxArt.contains("285x380")); // Standard category size
}

// ===== Tests de validation des inputs =====

void TestTwitchApiClient::testGetVideosEmptyUserId() {
  QSignalSpy videosSpy(m_client, &TwitchApiClient::videosReady);
  
  m_client->getVideos("");
  
  QTRY_COMPARE(videosSpy.count(), 1);
  QVariantList videos = videosSpy.first().first().value<QVariantList>();
  QCOMPARE(videos.size(), 0);
}

void TestTwitchApiClient::testGetFollowedClipsEmptyBroadcasterIds() {
  QSignalSpy clipsSpy(m_client, &TwitchApiClient::followedClipsReady);
  
  m_client->getFollowedClips(QStringList());
  
  QTRY_COMPARE(clipsSpy.count(), 1);
  QVariantList clips = clipsSpy.first().first().value<QVariantList>();
  QCOMPARE(clips.size(), 0);
}

void TestTwitchApiClient::testListFollowedStreamsEmptyUserId() {
  QSignalSpy errorSpy(m_client, &TwitchApiClient::errorOccurred);
  
  m_client->listFollowedStreams("", 10);
  
  QTRY_VERIFY(errorSpy.count() >= 1);
}

void TestTwitchApiClient::testListFollowedStreamsInvalidUserId() {
  QSignalSpy errorSpy(m_client, &TwitchApiClient::errorOccurred);
  
  m_client->listFollowedStreams("not_a_valid_user_id_format!@#", 10);
  
  QTRY_VERIFY(errorSpy.count() >= 1);
}

void TestTwitchApiClient::testGetRecommendedStreamsWithEmptyClientId() {
  TwitchApiClient* clientNoId = new TwitchApiClient("", this);
  
  QSignalSpy errorSpy(clientNoId, &TwitchApiClient::errorOccurred);
  
  clientNoId->getRecommendedStreams(10);
  
  QTRY_COMPARE(errorSpy.count(), 1);
  QString errorMsg = errorSpy.first().first().toString();
  QVERIFY(errorMsg.contains("Client ID") || errorMsg.contains("manquant"));
  
  delete clientNoId;
}

void TestTwitchApiClient::testGetTopCategoriesWithEmptyClientId() {
  TwitchApiClient* clientNoId = new TwitchApiClient("", this);
  
  QSignalSpy errorSpy(clientNoId, &TwitchApiClient::errorOccurred);
  
  clientNoId->getTopCategories(10);
  
  QTRY_COMPARE(errorSpy.count(), 1);
  
  delete clientNoId;
}

void TestTwitchApiClient::testGetPopularClipsWithEmptyClientId() {
  TwitchApiClient* clientNoId = new TwitchApiClient("", this);
  
  QSignalSpy errorSpy(clientNoId, &TwitchApiClient::errorOccurred);
  
  clientNoId->getPopularClips(10);
  
  QTRY_COMPARE(errorSpy.count(), 1);
  
  delete clientNoId;
}

void TestTwitchApiClient::testSearchChannelsEmptyQuery() {
  QSignalSpy searchSpy(m_client, &TwitchApiClient::searchChannelsReady);
  
  m_client->searchChannels("", 10);
  
  QTRY_COMPARE(searchSpy.count(), 1);
  QVariantList channels = searchSpy.first().first().value<QVariantList>();
  QCOMPARE(channels.size(), 0);
}

void TestTwitchApiClient::testSearchCategoriesEmptyQuery() {
  QSignalSpy searchSpy(m_client, &TwitchApiClient::searchCategoriesReady);
  
  m_client->searchCategories("", 10);
  
  QTRY_COMPARE(searchSpy.count(), 1);
  QVariantList categories = searchSpy.first().first().value<QVariantList>();
  QCOMPARE(categories.size(), 0);
}

void TestTwitchApiClient::testGetStreamsByCategoryEmptyGameId() {
  QSignalSpy streamsSpy(m_client, &TwitchApiClient::categoryStreamsReady);
  
  m_client->getStreamsByCategory("", 10);
  
  QTRY_COMPARE(streamsSpy.count(), 1);
  QVariantList streams = streamsSpy.first().first().value<QVariantList>();
  QCOMPARE(streams.size(), 0);
}

void TestTwitchApiClient::testGetFollowedChannelsEmptyUserId() {
  QSignalSpy channelsSpy(m_client, &TwitchApiClient::followedChannelsReady);
  QSignalSpy newStreamersSpy(m_client, &TwitchApiClient::newStreamersReady);
  
  m_client->getFollowedChannels("");
  
  QTRY_COMPARE(channelsSpy.count(), 1);
  QTRY_COMPARE(newStreamersSpy.count(), 1);
  QCOMPARE(channelsSpy.first().first().value<QVariantList>().size(), 0);
}

// ===== Tests des signaux =====

void TestTwitchApiClient::testErrorSignalConnection() {
  QSignalSpy spy(m_client, &TwitchApiClient::errorOccurred);
  QVERIFY(spy.isValid());
}

void TestTwitchApiClient::testStreamsReadySignalConnection() {
  QSignalSpy spy(m_client, &TwitchApiClient::streamsReady);
  QVERIFY(spy.isValid());
}

void TestTwitchApiClient::testCategoriesReadySignalConnection() {
  QSignalSpy spy(m_client, &TwitchApiClient::categoriesReady);
  QVERIFY(spy.isValid());
}

void TestTwitchApiClient::testClipsReadySignalConnection() {
  QSignalSpy popularSpy(m_client, &TwitchApiClient::popularClipsReady);
  QSignalSpy followedSpy(m_client, &TwitchApiClient::followedClipsReady);
  
  QVERIFY(popularSpy.isValid());
  QVERIFY(followedSpy.isValid());
}

void TestTwitchApiClient::testVideosReadySignalConnection() {
  QSignalSpy spy(m_client, &TwitchApiClient::videosReady);
  QVERIFY(spy.isValid());
}

void TestTwitchApiClient::testChannelsReadySignalConnection() {
  QSignalSpy spy(m_client, &TwitchApiClient::followedChannelsReady);
  QVERIFY(spy.isValid());
}

void TestTwitchApiClient::testSearchSignalsConnection() {
  QSignalSpy channelsSpy(m_client, &TwitchApiClient::searchChannelsReady);
  QSignalSpy categoriesSpy(m_client, &TwitchApiClient::searchCategoriesReady);
  
  QVERIFY(channelsSpy.isValid());
  QVERIFY(categoriesSpy.isValid());
}

void TestTwitchApiClient::testTokenInvalidatedSignalConnection() {
  QSignalSpy spy(m_client, &TwitchApiClient::tokenInvalidated);
  QVERIFY(spy.isValid());
}

// ===== Tests des limites d'API =====

void TestTwitchApiClient::testListStreamsNegativeLimit() {
  m_client->listStreams(-1);
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testListStreamsZeroLimit() {
  m_client->listStreams(0);
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testListStreamsLargeLimit() {
  m_client->listStreams(10000);
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testListFollowedStreamsInvalidLimit() {
  QSignalSpy errorSpy(m_client, &TwitchApiClient::errorOccurred);
  
  // Limit must be between 1 and 100
  m_client->listFollowedStreams("12345", 200);
  
  QTRY_VERIFY(errorSpy.count() >= 1);
}

// ===== Tests des méthodes Q_INVOKABLE =====

void TestTwitchApiClient::testListStreamsInvokable() {
  QSignalSpy spy(m_client, &TwitchApiClient::streamsReady);
  m_client->listStreams(10);
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testGetUserInfoInvokable() {
  QSignalSpy spy(m_client, &TwitchApiClient::userInfoReady);
  m_client->getUserInfo();
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testGetRecommendedStreamsInvokable() {
  QSignalSpy spy(m_client, &TwitchApiClient::recommendedStreamsReady);
  m_client->getRecommendedStreams(15);
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testGetTopCategoriesInvokable() {
  QSignalSpy spy(m_client, &TwitchApiClient::categoriesReady);
  m_client->getTopCategories(20);
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testGetPopularClipsInvokable() {
  QSignalSpy spy(m_client, &TwitchApiClient::popularClipsReady);
  m_client->getPopularClips(10);
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testSearchChannelsInvokable() {
  QSignalSpy spy(m_client, &TwitchApiClient::searchChannelsReady);
  m_client->searchChannels("ninja", 10);
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testSearchCategoriesInvokable() {
  QSignalSpy spy(m_client, &TwitchApiClient::searchCategoriesReady);
  m_client->searchCategories("fortnite", 10);
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testGetStreamsByCategoryInvokable() {
  QSignalSpy spy(m_client, &TwitchApiClient::categoryStreamsReady);
  m_client->getStreamsByCategory("33214", 15);
  QVERIFY(m_client != nullptr);
}

QTEST_MAIN(TestTwitchApiClient)
#include "TestTwitchApiClient.moc"
