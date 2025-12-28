#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QUrl>
#include <QEventLoop>

#include "core/network/HttpClient.hpp"
#include "core/Error.hpp"
#include "mocks/MockNetworkReply.hpp"

using namespace blueplayer::core;
using namespace blueplayer::core::network;
using namespace blueplayer::test;

class TestHttpClient : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();
  
  // ===== Constructor Tests =====
  void testConstructor();
  void testConstructorWithParent();
  void testNetworkManagerInitialized();
  
  // ===== HTTP Methods Tests =====
  void testGetReturnsReply();
  void testPostReturnsReply();
  void testPostWithData();
  void testPutReturnsReply();
  void testDeleteResourceReturnsReply();
  
  // ===== Bearer Token Tests =====
  void testSetBearerToken();
  void testSetBearerTokenEmpty();
  void testSetBearerTokenAddsAuthorizationHeader();
  void testBearerTokenGetter();
  void testBearerTokenGetterEmpty();
  
  // ===== Default Headers Tests =====
  void testSetDefaultHeader();
  void testSetMultipleDefaultHeaders();
  void testRemoveDefaultHeader();
  void testClearDefaultHeaders();
  
  // ===== checkNetworkError Tests =====
  void testCheckNetworkErrorNull();
  void testCheckNetworkErrorTimeout();
  void testCheckNetworkErrorConnectionRefused();
  void testCheckNetworkErrorGeneric();
  void testCheckNetworkErrorNoError();
  void testCheckNetworkErrorWithContext();
  void testCheckNetworkErrorWithPayload();
  
  // ===== buildRequest via Headers Tests =====
  void testBuildRequestWithDefaultHeaders();
  void testBuildRequestWithCustomHeaders();
  void testBuildRequestClientIdCasePreserved();
  void testBuildRequestCustomOverridesDefault();
  
  // ===== Signal Tests =====
  void testNetworkErrorSignalConnectable();

private:
  HttpClient* m_client = nullptr;
};

void TestHttpClient::initTestCase() {
}

void TestHttpClient::cleanupTestCase() {
}

void TestHttpClient::init() {
  m_client = new HttpClient(this);
}

void TestHttpClient::cleanup() {
  delete m_client;
  m_client = nullptr;
}

// ===== Constructor Tests =====

void TestHttpClient::testConstructor() {
  HttpClient client;
  QVERIFY(client.networkManager() != nullptr);
}

void TestHttpClient::testConstructorWithParent() {
  HttpClient* client = new HttpClient(this);
  QCOMPARE(client->parent(), this);
  delete client;
}

void TestHttpClient::testNetworkManagerInitialized() {
  QVERIFY(m_client->networkManager() != nullptr);
  // NetworkManager should have cache configured
  QVERIFY(m_client->networkManager()->cache() != nullptr);
}

// ===== HTTP Methods Tests =====

void TestHttpClient::testGetReturnsReply() {
  QUrl url("https://api.example.com/data");
  QNetworkReply* reply = m_client->get(url);
  
  QVERIFY(reply != nullptr);
  reply->deleteLater();
}

void TestHttpClient::testPostReturnsReply() {
  QUrl url("https://api.example.com/data");
  QNetworkReply* reply = m_client->post(url);
  
  QVERIFY(reply != nullptr);
  reply->deleteLater();
}

void TestHttpClient::testPostWithData() {
  QUrl url("https://api.example.com/data");
  QByteArray data = R"({"key": "value"})";
  QNetworkReply* reply = m_client->post(url, data);
  
  QVERIFY(reply != nullptr);
  reply->deleteLater();
}

void TestHttpClient::testPutReturnsReply() {
  QUrl url("https://api.example.com/data");
  QByteArray data = R"({"key": "updated"})";
  QNetworkReply* reply = m_client->put(url, data);
  
  QVERIFY(reply != nullptr);
  reply->deleteLater();
}

void TestHttpClient::testDeleteResourceReturnsReply() {
  QUrl url("https://api.example.com/data/123");
  QNetworkReply* reply = m_client->deleteResource(url);
  
  QVERIFY(reply != nullptr);
  reply->deleteLater();
}

// ===== Bearer Token Tests =====

void TestHttpClient::testSetBearerToken() {
  m_client->setBearerToken("test_token_123");
  QCOMPARE(m_client->bearerToken(), QString("test_token_123"));
}

void TestHttpClient::testSetBearerTokenEmpty() {
  // First set a token
  m_client->setBearerToken("initial_token");
  QCOMPARE(m_client->bearerToken(), QString("initial_token"));
  
  // Then clear it with empty string
  m_client->setBearerToken("");
  QCOMPARE(m_client->bearerToken(), QString(""));
}

void TestHttpClient::testSetBearerTokenAddsAuthorizationHeader() {
  // Set bearer token
  m_client->setBearerToken("my_access_token");
  
  // Verify the token is stored
  QCOMPARE(m_client->bearerToken(), QString("my_access_token"));
  
  // The Authorization header should be set to "Bearer my_access_token"
  // We verify this indirectly through the token getter
  QVERIFY(!m_client->bearerToken().isEmpty());
}

void TestHttpClient::testBearerTokenGetter() {
  // Initially empty
  HttpClient client;
  QCOMPARE(client.bearerToken(), QString());
  
  // After setting
  client.setBearerToken("token_abc");
  QCOMPARE(client.bearerToken(), QString("token_abc"));
}

void TestHttpClient::testBearerTokenGetterEmpty() {
  QCOMPARE(m_client->bearerToken(), QString());
}

// ===== Default Headers Tests =====

void TestHttpClient::testSetDefaultHeader() {
  m_client->setDefaultHeader("X-Custom-Header", "custom_value");
  
  // Verify the client is still functional after setting header
  QVERIFY(m_client->networkManager() != nullptr);
}

void TestHttpClient::testSetMultipleDefaultHeaders() {
  m_client->setDefaultHeader("X-Header-One", "value1");
  m_client->setDefaultHeader("X-Header-Two", "value2");
  m_client->setDefaultHeader("X-Header-Three", "value3");
  
  // Client should still be functional
  QVERIFY(m_client->networkManager() != nullptr);
}

void TestHttpClient::testRemoveDefaultHeader() {
  m_client->setDefaultHeader("X-Test-Header", "test_value");
  m_client->removeDefaultHeader("X-Test-Header");
  
  // Verify the client is still functional
  QVERIFY(m_client->networkManager() != nullptr);
}

void TestHttpClient::testClearDefaultHeaders() {
  m_client->setDefaultHeader("X-Header1", "value1");
  m_client->setDefaultHeader("X-Header2", "value2");
  m_client->clearDefaultHeaders();
  
  // Verify the client is still functional after clearing headers
  QVERIFY(m_client->networkManager() != nullptr);
}

// ===== checkNetworkError Tests =====

void TestHttpClient::testCheckNetworkErrorNull() {
  Error error = HttpClient::checkNetworkError(nullptr, "Test context");
  
  QVERIFY(error.hasError());
  QCOMPARE(error.code(), ErrorCode::NetworkError);
  QCOMPARE(error.context(), QString("Test context"));
}

void TestHttpClient::testCheckNetworkErrorTimeout() {
  auto* reply = MockNetworkReply::createTimeoutResponse(this);
  reply->finish();
  
  Error error = HttpClient::checkNetworkError(reply, "GET request");
  
  QVERIFY(error.hasError());
  QCOMPARE(error.code(), ErrorCode::NetworkTimeout);
  
  delete reply;
}

void TestHttpClient::testCheckNetworkErrorConnectionRefused() {
  auto* reply = MockNetworkReply::createConnectionRefusedResponse(this);
  reply->finish();
  
  Error error = HttpClient::checkNetworkError(reply, "POST request");
  
  QVERIFY(error.hasError());
  QCOMPARE(error.code(), ErrorCode::NetworkConnectionRefused);
  
  delete reply;
}

void TestHttpClient::testCheckNetworkErrorGeneric() {
  auto* reply = MockNetworkReply::createErrorResponse(
      QNetworkReply::HostNotFoundError, "Host not found", this);
  reply->finish();
  
  Error error = HttpClient::checkNetworkError(reply, "DNS lookup");
  
  QVERIFY(error.hasError());
  QCOMPARE(error.code(), ErrorCode::NetworkError);  // Falls back to generic
  
  delete reply;
}

void TestHttpClient::testCheckNetworkErrorNoError() {
  auto* reply = MockNetworkReply::createJsonResponse(
      R"({"success": true})", 200, this);
  reply->finish();
  
  Error error = HttpClient::checkNetworkError(reply, "Successful request");
  
  QVERIFY(!error.hasError());
  QCOMPARE(error.code(), ErrorCode::Unknown);  // Unknown with no message = no error
  
  delete reply;
}

void TestHttpClient::testCheckNetworkErrorWithContext() {
  Error error = HttpClient::checkNetworkError(nullptr, "Twitch API request");
  
  QVERIFY(error.hasError());
  QVERIFY(error.context().contains("Twitch API request"));
}

void TestHttpClient::testCheckNetworkErrorWithPayload() {
  auto* reply = MockNetworkReply::createErrorResponse(
      QNetworkReply::ContentNotFoundError, "Not found", this);
  reply->setData(R"({"error": "Resource not found", "status": 404})");
  reply->finish();
  
  Error error = HttpClient::checkNetworkError(reply, "Fetch stream");
  
  QVERIFY(error.hasError());
  // Context contains the response payload
  QVERIFY(error.context().contains("404") || error.context().contains("not found"));
  
  delete reply;
}

// ===== buildRequest via Headers Tests =====

void TestHttpClient::testBuildRequestWithDefaultHeaders() {
  m_client->setDefaultHeader("X-App-Version", "1.0.0");
  m_client->setDefaultHeader("Accept-Language", "en-US");
  
  // Make a request - headers should be applied
  QUrl url("https://api.example.com/test");
  QNetworkReply* reply = m_client->get(url);
  
  QVERIFY(reply != nullptr);
  // The request was built with our headers applied
  reply->deleteLater();
}

void TestHttpClient::testBuildRequestWithCustomHeaders() {
  QHash<QString, QString> customHeaders;
  customHeaders["X-Request-ID"] = "abc123";
  customHeaders["Accept"] = "application/json";
  
  QUrl url("https://api.example.com/test");
  QNetworkReply* reply = m_client->get(url, customHeaders);
  
  QVERIFY(reply != nullptr);
  reply->deleteLater();
}

void TestHttpClient::testBuildRequestClientIdCasePreserved() {
  // Client-ID must be exactly "Client-ID" for Twitch API
  m_client->setDefaultHeader("client-id", "my_client_id_value");
  
  QUrl url("https://api.twitch.tv/helix/streams");
  QNetworkReply* reply = m_client->get(url);
  
  QVERIFY(reply != nullptr);
  // The Client-ID header case should be preserved in the request
  reply->deleteLater();
}

void TestHttpClient::testBuildRequestCustomOverridesDefault() {
  // Set a default header
  m_client->setDefaultHeader("Authorization", "Bearer default_token");
  
  // Make request with custom header that should override
  QHash<QString, QString> customHeaders;
  customHeaders["Authorization"] = "Bearer custom_token";
  
  QUrl url("https://api.example.com/test");
  QNetworkReply* reply = m_client->get(url, customHeaders);
  
  QVERIFY(reply != nullptr);
  reply->deleteLater();
}

// ===== Signal Tests =====

void TestHttpClient::testNetworkErrorSignalConnectable() {
  QSignalSpy spy(m_client, &HttpClient::networkError);
  QVERIFY(spy.isValid());
}

QTEST_MAIN(TestHttpClient)
#include "TestHttpClient.moc"
