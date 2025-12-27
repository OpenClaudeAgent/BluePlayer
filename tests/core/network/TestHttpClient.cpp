#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QUrl>
#include "core/network/HttpClient.hpp"
#include "core/Error.hpp"

using namespace blueplayer::core;
using namespace blueplayer::core::network;

class TestHttpClient : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  
  void testConstructor();
  void testSetBearerToken();
  void testSetDefaultHeader();
  void testRemoveDefaultHeader();
  void testClearDefaultHeaders();
  void testCheckNetworkError();
};

void TestHttpClient::initTestCase() {
}

void TestHttpClient::cleanupTestCase() {
}

void TestHttpClient::testConstructor() {
  HttpClient client;
  QVERIFY(client.networkManager() != nullptr);
}

void TestHttpClient::testSetBearerToken() {
  HttpClient client;
  client.setBearerToken("test_token_123");
  
  // Verify the token is stored correctly
  QCOMPARE(client.bearerToken(), QString("test_token_123"));
}

void TestHttpClient::testSetDefaultHeader() {
  HttpClient client;
  client.setDefaultHeader("X-Custom-Header", "custom_value");
  
  // Verify the client is still functional after setting header
  QVERIFY(client.networkManager() != nullptr);
}

void TestHttpClient::testRemoveDefaultHeader() {
  HttpClient client;
  client.setDefaultHeader("X-Test-Header", "test_value");
  client.removeDefaultHeader("X-Test-Header");
  
  // Verify the client is still functional
  QVERIFY(client.networkManager() != nullptr);
}

void TestHttpClient::testClearDefaultHeaders() {
  HttpClient client;
  client.setDefaultHeader("X-Header1", "value1");
  client.setDefaultHeader("X-Header2", "value2");
  client.clearDefaultHeaders();
  
  // Verify the client is still functional after clearing headers
  QVERIFY(client.networkManager() != nullptr);
}

void TestHttpClient::testCheckNetworkError() {
  // Test with null reply
  Error error1 = HttpClient::checkNetworkError(nullptr, "Test context");
  QCOMPARE(error1.code(), blueplayer::core::ErrorCode::NetworkError);
  QCOMPARE(error1.context(), QString("Test context"));
}

QTEST_MAIN(TestHttpClient)
#include "TestHttpClient.moc"


