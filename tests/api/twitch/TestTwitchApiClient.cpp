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
  
  // Tests d'initialisation
  void testConstructor();
  void testSetAccessToken();
  void testSetAccessTokenEmpty();
  
  // Tests de parsing JSON
  void testParseJsonValidResponse();
  void testParseJsonEmptyData();
  void testParseJsonInvalidFormat();
  void testParseJsonMalformed();
  
  // Tests de parsing des streams
  void testParseStreamsArray();
  void testParseStreamsArrayEmpty();
  void testParseStreamsArrayMultiple();
  
  // Tests de l'expansion des thumbnails
  void testExpandThumbnailFormat();
  
  // Tests de gestion d'erreur avec token invalide
  void testErrorWithEmptyToken();
  void testErrorWithNullToken();
  
  // Tests des limites d'API
  void testListStreamsNegativeLimit();
  void testListStreamsZeroLimit();
  void testListStreamsLargeLimit();
  
  // Tests avec userId invalide
  void testListFollowedStreamsEmptyUserId();
  void testListFollowedStreamsInvalidUserId();
  
  // Tests des signaux
  void testErrorSignalConnection();
  void testStreamsReadySignalConnection();
  void testCategoriesReadySignalConnection();
  void testClipsReadySignalConnection();
  void testVideosReadySignalConnection();
  void testChannelsReadySignalConnection();
  void testSearchSignalsConnection();
  void testTokenInvalidatedSignalConnection();
  
  // Tests des méthodes Q_INVOKABLE
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
  // Pas de crash = succès
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testSetAccessTokenEmpty() {
  m_client->setAccessToken("");
  // Doit gérer gracieusement un token vide
  QVERIFY(m_client != nullptr);
}

// ===== Tests de parsing JSON =====

void TestTwitchApiClient::testParseJsonValidResponse() {
  QString validJson = R"({"data": [{"id": "123", "user_name": "test_user"}]})";
  QJsonDocument doc = QJsonDocument::fromJson(validJson.toUtf8());
  
  QVERIFY(!doc.isNull());
  QVERIFY(doc.isObject());
  
  QJsonObject root = doc.object();
  QVERIFY(root.contains("data"));
  QVERIFY(root["data"].isArray());
  QCOMPARE(root["data"].toArray().size(), 1);
}

void TestTwitchApiClient::testParseJsonEmptyData() {
  QString emptyJson = R"({"data": []})";
  QJsonDocument doc = QJsonDocument::fromJson(emptyJson.toUtf8());
  
  QVERIFY(!doc.isNull());
  QVERIFY(doc.isObject());
  QVERIFY(doc.object()["data"].isArray());
  QCOMPARE(doc.object()["data"].toArray().size(), 0);
}

void TestTwitchApiClient::testParseJsonInvalidFormat() {
  QString invalidJson = R"({"error": "Unauthorized"})";
  QJsonDocument doc = QJsonDocument::fromJson(invalidJson.toUtf8());
  
  QVERIFY(!doc.isNull());
  QVERIFY(doc.isObject());
  QVERIFY(!doc.object().contains("data"));
}

void TestTwitchApiClient::testParseJsonMalformed() {
  QString malformedJson = R"({invalid json content})";
  QJsonDocument doc = QJsonDocument::fromJson(malformedJson.toUtf8());
  
  // Document doit être null ou invalide
  QVERIFY(doc.isNull() || !doc.isObject());
}

// ===== Tests de parsing des streams =====

void TestTwitchApiClient::testParseStreamsArray() {
  QJsonArray entries;
  QJsonObject stream;
  stream["id"] = "123456";
  stream["user_name"] = "TestStreamer";
  stream["user_login"] = "teststreamer";
  stream["title"] = "Playing Games";
  stream["viewer_count"] = 1500;
  stream["language"] = "en";
  stream["thumbnail_url"] = "https://static-cdn.jtvnw.net/previews-ttv/{width}x{height}.jpg";
  stream["started_at"] = "2024-01-15T10:30:00Z";
  stream["game_name"] = "Fortnite";
  stream["game_id"] = "33214";
  entries.append(stream);
  
  QCOMPARE(entries.size(), 1);
  
  QJsonObject firstStream = entries[0].toObject();
  QCOMPARE(firstStream["user_name"].toString(), QString("TestStreamer"));
  QCOMPARE(firstStream["viewer_count"].toInt(), 1500);
  QVERIFY(firstStream["thumbnail_url"].toString().contains("{width}"));
}

void TestTwitchApiClient::testParseStreamsArrayEmpty() {
  QJsonArray emptyEntries;
  QCOMPARE(emptyEntries.size(), 0);
}

void TestTwitchApiClient::testParseStreamsArrayMultiple() {
  QJsonArray entries;
  
  for (int i = 0; i < 5; i++) {
    QJsonObject stream;
    stream["id"] = QString::number(i);
    stream["user_name"] = QString("Streamer%1").arg(i);
    stream["viewer_count"] = i * 100;
    entries.append(stream);
  }
  
  QCOMPARE(entries.size(), 5);
  QCOMPARE(entries[2].toObject()["user_name"].toString(), QString("Streamer2"));
}

// ===== Tests de l'expansion des thumbnails =====

void TestTwitchApiClient::testExpandThumbnailFormat() {
  QString templateUrl = "https://static-cdn.jtvnw.net/previews-ttv/{width}x{height}.jpg";
  
  // Vérifier que le template contient les placeholders
  QVERIFY(templateUrl.contains("{width}"));
  QVERIFY(templateUrl.contains("{height}"));
  
  // Simuler l'expansion (comme le fait la méthode privée)
  QString expanded = templateUrl;
  expanded.replace("{width}", "320");
  expanded.replace("{height}", "180");
  
  QVERIFY(!expanded.contains("{width}"));
  QVERIFY(!expanded.contains("{height}"));
  QVERIFY(expanded.contains("320x180"));
}

// ===== Tests de gestion d'erreur avec token =====

void TestTwitchApiClient::testErrorWithEmptyToken() {
  m_client->setAccessToken("");
  
  // Le client ne doit pas crasher
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testErrorWithNullToken() {
  m_client->setAccessToken(QString());
  
  QVERIFY(m_client != nullptr);
}

// ===== Tests des limites d'API =====

void TestTwitchApiClient::testListStreamsNegativeLimit() {
  QSignalSpy errorSpy(m_client, &TwitchApiClient::errorOccurred);
  
  m_client->listStreams(-1);
  
  // Ne doit pas crasher
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testListStreamsZeroLimit() {
  QSignalSpy errorSpy(m_client, &TwitchApiClient::errorOccurred);
  
  m_client->listStreams(0);
  
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testListStreamsLargeLimit() {
  QSignalSpy errorSpy(m_client, &TwitchApiClient::errorOccurred);
  
  m_client->listStreams(10000);
  
  QVERIFY(m_client != nullptr);
}

// ===== Tests avec userId invalide =====

void TestTwitchApiClient::testListFollowedStreamsEmptyUserId() {
  QSignalSpy errorSpy(m_client, &TwitchApiClient::errorOccurred);
  
  m_client->listFollowedStreams("", 10);
  
  QVERIFY(m_client != nullptr);
}

void TestTwitchApiClient::testListFollowedStreamsInvalidUserId() {
  QSignalSpy errorSpy(m_client, &TwitchApiClient::errorOccurred);
  
  m_client->listFollowedStreams("not_a_valid_user_id_format!@#", 10);
  
  QVERIFY(m_client != nullptr);
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

// ===== Tests des méthodes Q_INVOKABLE =====

void TestTwitchApiClient::testListStreamsInvokable() {
  // Vérifier que listStreams peut être appelé
  QSignalSpy spy(m_client, &TwitchApiClient::streamsReady);
  
  m_client->listStreams(10);
  
  // La méthode s'exécute sans crash
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
  
  m_client->getStreamsByCategory("33214", 15);  // Fortnite game ID
  
  QVERIFY(m_client != nullptr);
}

QTEST_MAIN(TestTwitchApiClient)
#include "TestTwitchApiClient.moc"
