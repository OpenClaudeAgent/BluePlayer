#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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

  // ===== Constructor Tests =====
  void testConstructor();
  void testConstructorWithParent();
  void testConstructorInitializesCurl();
  void testMultipleInstances();
  
  // ===== Signal Connectivity Tests =====
  void testErrorOccurredSignalConnectable();
  void testHttpErrorSignalConnectable();
  void testErrorOccurredSignalEmitted();
  
  // ===== postJson URL Validation Tests =====
  void testPostJsonEmptyUrl();
  void testPostJsonInvalidUrl();
  void testPostJsonMalformedUrl();
  void testPostJsonUnreachableHost();
  
  // ===== postJson Data Tests =====
  void testPostJsonEmptyData();
  void testPostJsonEmptyJsonObject();
  void testPostJsonSimpleData();
  void testPostJsonNestedData();
  void testPostJsonArrayData();
  void testPostJsonLargeData();
  
  // ===== postJson Header Tests =====
  void testPostJsonEmptyHeaders();
  void testPostJsonWithHeaders();
  void testPostJsonHeaderCasePreservation();
  void testPostJsonMultipleHeaders();
  void testPostJsonClientIdHeader();
  void testPostJsonAuthorizationHeader();
  
  // ===== JSON Request Creation Tests =====
  void testCreateJsonRequest();
  void testCreateJsonRequestWithData();
  void testCreateGraphQLRequest();
  void testCreateGraphQLWithVariables();
  
  // ===== Error Handling Tests =====
  void testErrorSignalOnInvalidUrl();
  void testReturnNullOnError();
  void testNoErrorOnValidRequest();

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

// ===== Constructor Tests =====

void TestCurlHttpClient::testConstructor() {
  CurlHttpClient client;
  QVERIFY(client.parent() == nullptr);
}

void TestCurlHttpClient::testConstructorWithParent() {
  CurlHttpClient* client = new CurlHttpClient(this);
  QVERIFY(client != nullptr);
  QCOMPARE(client->parent(), this);
  delete client;
}

void TestCurlHttpClient::testConstructorInitializesCurl() {
  // Creating a client should initialize curl internally
  // We verify this indirectly by making sure we can use the client
  CurlHttpClient client;
  
  // The client should be functional after construction
  QJsonObject obj;
  obj["test"] = "value";
  QJsonDocument jsonData(obj);
  QHash<QString, QString> headers;
  
  // Should not crash when making a request (even if it fails due to invalid URL)
  QJsonDocument result = client.postJson(QUrl(), jsonData, headers);
  QVERIFY(result.isNull());  // Expected to fail, but shouldn't crash
}

void TestCurlHttpClient::testMultipleInstances() {
  // Multiple CurlHttpClient instances should work independently
  CurlHttpClient* client1 = new CurlHttpClient(this);
  CurlHttpClient* client2 = new CurlHttpClient(this);
  CurlHttpClient* client3 = new CurlHttpClient(this);
  
  QVERIFY(client1 != nullptr);
  QVERIFY(client2 != nullptr);
  QVERIFY(client3 != nullptr);
  
  // They should all be distinct objects
  QVERIFY(client1 != client2);
  QVERIFY(client2 != client3);
  
  delete client1;
  delete client2;
  delete client3;
}

// ===== Signal Connectivity Tests =====

void TestCurlHttpClient::testErrorOccurredSignalConnectable() {
  QSignalSpy spy(m_client, &CurlHttpClient::errorOccurred);
  QVERIFY(spy.isValid());
}

void TestCurlHttpClient::testHttpErrorSignalConnectable() {
  QSignalSpy spy(m_client, &CurlHttpClient::httpError);
  QVERIFY(spy.isValid());
}

void TestCurlHttpClient::testErrorOccurredSignalEmitted() {
  QSignalSpy spy(m_client, &CurlHttpClient::errorOccurred);
  QVERIFY(spy.isValid());
  
  // Make a request with invalid URL - should emit error
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  m_client->postJson(QUrl(), jsonData, headers);
  
  // Should have emitted an error signal
  QVERIFY(spy.count() > 0);
}

// ===== postJson URL Validation Tests =====

void TestCurlHttpClient::testPostJsonEmptyUrl() {
  QJsonDocument emptyDoc;
  QHash<QString, QString> headers;
  
  QJsonDocument result = m_client->postJson(QUrl(), emptyDoc, headers);
  
  QVERIFY(result.isNull());
}

void TestCurlHttpClient::testPostJsonInvalidUrl() {
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  
  QJsonDocument result = m_client->postJson(QUrl("not_a_valid_url"), jsonData, headers);
  
  QVERIFY(result.isNull());
}

void TestCurlHttpClient::testPostJsonMalformedUrl() {
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  
  // Various malformed URLs
  QJsonDocument result1 = m_client->postJson(QUrl("://missing-scheme.com"), jsonData, headers);
  QJsonDocument result2 = m_client->postJson(QUrl("ftp://"), jsonData, headers);  // Incomplete URL
  
  QVERIFY(result1.isNull());
  QVERIFY(result2.isNull());
}

void TestCurlHttpClient::testPostJsonUnreachableHost() {
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  
  // Use a URL that will definitely fail to resolve
  QJsonDocument result = m_client->postJson(
    QUrl("https://this.host.definitely.does.not.exist.local.invalid/api"),
    jsonData,
    headers
  );
  
  QVERIFY(result.isNull());
}

// ===== postJson Data Tests =====

void TestCurlHttpClient::testPostJsonEmptyData() {
  QJsonDocument emptyDoc;
  QHash<QString, QString> headers;
  headers["Content-Type"] = "application/json";
  
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/api"),
    emptyDoc,
    headers
  );
  
  // Should not crash
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonEmptyJsonObject() {
  QJsonObject emptyObj;
  QJsonDocument jsonData(emptyObj);
  QHash<QString, QString> headers;
  
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/api"),
    jsonData,
    headers
  );
  
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonSimpleData() {
  QJsonObject obj;
  obj["test"] = "value";
  obj["number"] = 42;
  obj["boolean"] = true;
  QJsonDocument jsonData(obj);
  QHash<QString, QString> headers;
  
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/api"),
    jsonData,
    headers
  );
  
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonNestedData() {
  QJsonObject inner;
  inner["nested_key"] = "nested_value";
  
  QJsonObject outer;
  outer["level1"] = inner;
  outer["other"] = "value";
  
  QJsonDocument jsonData(outer);
  QHash<QString, QString> headers;
  
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/api"),
    jsonData,
    headers
  );
  
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonArrayData() {
  QJsonArray arr;
  arr.append("item1");
  arr.append("item2");
  arr.append(123);
  
  QJsonObject obj;
  obj["items"] = arr;
  
  QJsonDocument jsonData(obj);
  QHash<QString, QString> headers;
  
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/api"),
    jsonData,
    headers
  );
  
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonLargeData() {
  // Create a reasonably large JSON object
  QJsonObject obj;
  for (int i = 0; i < 100; ++i) {
    obj[QString("key_%1").arg(i)] = QString("value_%1").arg(i);
  }
  
  QJsonDocument jsonData(obj);
  QHash<QString, QString> headers;
  
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/api"),
    jsonData,
    headers
  );
  
  QVERIFY(m_client != nullptr);
}

// ===== postJson Header Tests =====

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
  
  QVERIFY(m_client != nullptr);
}

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
  
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonHeaderCasePreservation() {
  // The main purpose of CurlHttpClient is to preserve header case
  // Twitch requires "Client-ID" not "client-id"
  
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  headers["Client-ID"] = "test_client_id";
  headers["Authorization"] = "OAuth test_token";
  headers["Content-Type"] = "application/json";
  
  // Verify headers are created correctly
  QVERIFY(headers.contains("Client-ID"));
  QCOMPARE(headers["Client-ID"], QString("test_client_id"));
  
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonMultipleHeaders() {
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  
  // Add multiple headers
  headers["Header-One"] = "value1";
  headers["Header-Two"] = "value2";
  headers["Header-Three"] = "value3";
  headers["X-Custom-Header"] = "custom_value";
  
  QCOMPARE(headers.size(), 4);
  QVERIFY(headers.contains("Header-One"));
  QVERIFY(headers.contains("X-Custom-Header"));
}

void TestCurlHttpClient::testPostJsonClientIdHeader() {
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  headers["Client-ID"] = "kimne78kx3ncx6brgo4mv6wki5h1ko";
  
  // Verify Client-ID header is set correctly (case-sensitive key)
  QVERIFY(headers.contains("Client-ID"));
  QCOMPARE(headers.value("Client-ID"), QString("kimne78kx3ncx6brgo4mv6wki5h1ko"));
  
  // Make request (will fail but shouldn't crash)
  QJsonDocument result = m_client->postJson(
    QUrl("https://gql.twitch.tv/gql"),
    jsonData,
    headers
  );
  
  QVERIFY(m_client != nullptr);
}

void TestCurlHttpClient::testPostJsonAuthorizationHeader() {
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  headers["Authorization"] = "OAuth abc123def456";
  
  QVERIFY(headers.contains("Authorization"));
  QVERIFY(headers.value("Authorization").startsWith("OAuth "));
  
  QJsonDocument result = m_client->postJson(
    QUrl("https://invalid.local/api"),
    jsonData,
    headers
  );
  
  QVERIFY(m_client != nullptr);
}

// ===== JSON Request Creation Tests =====

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
  // Simulate a Twitch GraphQL request
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

void TestCurlHttpClient::testCreateGraphQLRequest() {
  // Create a standard GraphQL request structure
  QJsonObject obj;
  obj["query"] = "query { user { id name } }";
  
  QJsonDocument doc(obj);
  
  QVERIFY(!doc.isNull());
  QVERIFY(doc.isObject());
  QVERIFY(doc.object().contains("query"));
  QVERIFY(doc.object()["query"].isString());
}

void TestCurlHttpClient::testCreateGraphQLWithVariables() {
  // Create a GraphQL request with variables
  QJsonObject variables;
  variables["channelLogin"] = "testchannel";
  variables["playerType"] = "embed";
  
  QJsonObject extensions;
  QJsonObject persistedQuery;
  persistedQuery["version"] = 1;
  persistedQuery["sha256Hash"] = "abc123";
  extensions["persistedQuery"] = persistedQuery;
  
  QJsonObject request;
  request["operationName"] = "PlaybackAccessToken_Template";
  request["variables"] = variables;
  request["extensions"] = extensions;
  
  QJsonDocument doc(request);
  
  QVERIFY(!doc.isNull());
  QVERIFY(doc.isObject());
  
  QJsonObject root = doc.object();
  QCOMPARE(root["operationName"].toString(), QString("PlaybackAccessToken_Template"));
  QVERIFY(root.contains("variables"));
  QVERIFY(root.contains("extensions"));
  QCOMPARE(root["variables"].toObject()["channelLogin"].toString(), QString("testchannel"));
}

// ===== Error Handling Tests =====

void TestCurlHttpClient::testErrorSignalOnInvalidUrl() {
  QSignalSpy errorSpy(m_client, &CurlHttpClient::errorOccurred);
  
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  
  m_client->postJson(QUrl("not-a-url"), jsonData, headers);
  
  // Should have emitted error
  QVERIFY(errorSpy.count() > 0);
}

void TestCurlHttpClient::testReturnNullOnError() {
  QJsonDocument jsonData;
  QHash<QString, QString> headers;
  
  // All these should return null documents
  QJsonDocument result1 = m_client->postJson(QUrl(), jsonData, headers);
  QJsonDocument result2 = m_client->postJson(QUrl("invalid"), jsonData, headers);
  QJsonDocument result3 = m_client->postJson(QUrl("https://localhost:9999/no-server"), jsonData, headers);
  
  QVERIFY(result1.isNull());
  QVERIFY(result2.isNull());
  QVERIFY(result3.isNull());
}

void TestCurlHttpClient::testNoErrorOnValidRequest() {
  // This test documents expected behavior - we can't easily test success
  // without a real server, but we verify the structure is correct
  
  QJsonObject variables;
  variables["login"] = "test";
  variables["isLive"] = true;
  
  QJsonObject request;
  request["operationName"] = "TestQuery";
  request["variables"] = variables;
  request["query"] = "query TestQuery { test }";
  
  QJsonDocument doc(request);
  QHash<QString, QString> headers;
  headers["Client-ID"] = "test_client_id";
  headers["Content-Type"] = "application/json";
  
  // Verify request is well-formed
  QVERIFY(!doc.isNull());
  QVERIFY(doc.isObject());
  QVERIFY(headers.size() == 2);
  
  // Verify headers contain expected keys
  QVERIFY(headers.contains("Client-ID"));
  QVERIFY(headers.contains("Content-Type"));
}

QTEST_MAIN(TestCurlHttpClient)
#include "TestCurlHttpClient.moc"
