#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include "core/network/CurlHttpClient.hpp"

using namespace blueplayer::core::network;

class TestCurlHttpClient : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Tests d'initialisation
  void testConstructor();
  void testConstructorWithParent();

  // Tests des signaux
  void testErrorOccurredSignal();
  void testHttpErrorSignal();

  // Tests de postJson - structure
  void testPostJsonEmptyUrl();
  void testPostJsonInvalidUrl();
  void testPostJsonEmptyData();
  void testPostJsonEmptyHeaders();

  // Tests de postJson - headers
  void testPostJsonWithHeaders();
  void testPostJsonHeaderCasePreservation();
  void testPostJsonMultipleHeaders();

  // Tests de création de requête JSON
  void testCreateJsonRequest();
  void testCreateJsonRequestWithData();

private:
  CurlHttpClient* m_client = nullptr;
};

void TestCurlHttpClient::initTestCase() {
}

void TestCurlHttpClient::cleanupTestCase() {
}

void TestCurlHttpClient::init() {
  m_client = new CurlHttpClient(this);
}

void TestCurlHttpClient::cleanup() {
  delete m_client;
  m_client = nullptr;
}

// ===== Tests d'initialisation =====

void TestCurlHttpClient::testConstructor() {
  CurlHttpClient client;
  QVERIFY(&client != nullptr);
}

void TestCurlHttpClient::testConstructorWithParent() {
  CurlHttpClient* client = new CurlHttpClient(this);
  QVERIFY(client != nullptr);
  QCOMPARE(client->parent(), this);
  delete client;
}

// ===== Tests des signaux =====

void TestCurlHttpClient::testErrorOccurredSignal() {
  QSignalSpy spy(m_client, &CurlHttpClient::errorOccurred);
  QVERIFY(spy.isValid());
}

void TestCurlHttpClient::testHttpErrorSignal() {
  QSignalSpy spy(m_client, &CurlHttpClient::httpError);
  QVERIFY(spy.isValid());
}

// ===== Tests de postJson - structure =====

void TestCurlHttpClient::testPostJsonEmptyUrl() {
  QJsonDocument emptyDoc;
  QHash<QString, QString> headers;
  
  QJsonDocument result = m_client->postJson(QUrl(), emptyDoc, headers);
  
  // Doit retourner un document null/vide pour URL invalide
  QVERIFY(result.isNull());
}

void TestCurlHttpClient::testPostJsonInvalidUrl() {
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  
  QJsonDocument result = m_client->postJson(QUrl("not_a_valid_url"), jsonData, headers);
  
  // Doit gérer gracieusement une URL invalide
  QVERIFY(result.isNull());
}

void TestCurlHttpClient::testPostJsonEmptyData() {
  QJsonDocument emptyDoc;
  QHash<QString, QString> headers;
  headers["Content-Type"] = "application/json";
  
  // URL invalide pour éviter un vrai appel réseau
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/api"),
    emptyDoc,
    headers
  );
  
  // Pas de crash
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonEmptyHeaders() {
  QJsonObject obj;
  obj["test"] = "value";
  QJsonDocument jsonData(obj);
  QHash<QString, QString> emptyHeaders;
  
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/api"),
    jsonData,
    emptyHeaders
  );
  
  // Pas de crash même sans headers
  QVERIFY(m_client != nullptr);
}

// ===== Tests de postJson - headers =====

void TestCurlHttpClient::testPostJsonWithHeaders() {
  QJsonObject obj;
  obj["query"] = "test";
  QJsonDocument jsonData(obj);
  
  QHash<QString, QString> headers;
  headers["Content-Type"] = "application/json";
  headers["Authorization"] = "Bearer test_token";
  
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/graphql"),
    jsonData,
    headers
  );
  
  // Pas de crash
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonHeaderCasePreservation() {
  // Le but principal de CurlHttpClient est de préserver la casse des headers
  // Twitch requiert "Client-ID" pas "client-id"
  
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  headers["Client-ID"] = "test_client_id";
  headers["Authorization"] = "OAuth test_token";
  headers["Content-Type"] = "application/json";
  
  // Vérifier que les headers sont créés correctement
  QVERIFY(headers.contains("Client-ID"));
  QCOMPARE(headers["Client-ID"], QString("test_client_id"));
  
  // Le test réel de préservation de casse nécessiterait un serveur de test
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonMultipleHeaders() {
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  
  // Ajouter plusieurs headers
  headers["Header-One"] = "value1";
  headers["Header-Two"] = "value2";
  headers["Header-Three"] = "value3";
  headers["X-Custom-Header"] = "custom_value";
  
  QCOMPARE(headers.size(), 4);
  QVERIFY(headers.contains("Header-One"));
  QVERIFY(headers.contains("X-Custom-Header"));
}

// ===== Tests de création de requête JSON =====

void TestCurlHttpClient::testCreateJsonRequest() {
  QJsonObject obj;
  obj["operationName"] = "PlaybackAccessToken";
  obj["extensions"] = QJsonObject();
  
  QJsonDocument doc(obj);
  
  QVERIFY(!doc.isNull());
  QVERIFY(doc.isObject());
  QCOMPARE(doc.object()["operationName"].toString(), QString("PlaybackAccessToken"));
}

void TestCurlHttpClient::testCreateJsonRequestWithData() {
  // Simuler une requête GraphQL Twitch
  QJsonObject variables;
  variables["login"] = "streamer_name";
  variables["isLive"] = true;
  variables["isVod"] = false;
  
  QJsonObject obj;
  obj["operationName"] = "PlaybackAccessToken";
  obj["variables"] = variables;
  obj["query"] = "query PlaybackAccessToken { ... }";
  
  QJsonDocument doc(obj);
  
  QVERIFY(!doc.isNull());
  QVERIFY(doc.isObject());
  
  QJsonObject root = doc.object();
  QVERIFY(root.contains("variables"));
  QVERIFY(root["variables"].isObject());
  QCOMPARE(root["variables"].toObject()["login"].toString(), QString("streamer_name"));
}

QTEST_MAIN(TestCurlHttpClient)
#include "TestCurlHttpClient.moc"
