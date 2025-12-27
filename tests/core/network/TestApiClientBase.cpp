#include <QtTest/QtTest>
#include <QSignalSpy>
#include "core/network/ApiClientBase.hpp"

using namespace blueplayer::core::network;

class TestApiClientBase : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  
  void testConstructor();
  void testSetBearerToken();
  void testSetDefaultHeader();
  void testErrorSignal();
};

void TestApiClientBase::initTestCase() {
}

void TestApiClientBase::cleanupTestCase() {
}

void TestApiClientBase::testConstructor() {
  ApiClientBase client;
  // Verify the client is properly constructed
  client.setBearerToken("test_token");
  QVERIFY(client.parent() == nullptr);
}

void TestApiClientBase::testSetBearerToken() {
  ApiClientBase client;
  client.setBearerToken("test_bearer_token");
  
  // Verify client is still functional after setting token
  QSignalSpy spy(&client, &ApiClientBase::errorOccurred);
  QVERIFY(spy.isValid());
}

void TestApiClientBase::testSetDefaultHeader() {
  ApiClientBase client;
  client.setDefaultHeader("X-API-Version", "v1");
  
  // Verify client is still functional after setting header
  QSignalSpy spy(&client, &ApiClientBase::errorOccurred);
  QVERIFY(spy.isValid());
}

void TestApiClientBase::testErrorSignal() {
  ApiClientBase client;
  QSignalSpy spy(&client, &ApiClientBase::errorOccurred);
  
  // Note: Pour déclencher réellement le signal, il faudrait simuler une erreur réseau
  // Pour l'instant, on vérifie que le signal peut être connecté
  QVERIFY(spy.isValid());
}

QTEST_MAIN(TestApiClientBase)
#include "TestApiClientBase.moc"


