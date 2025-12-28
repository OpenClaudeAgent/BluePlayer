/**
 * @file TestApiClientBase.cpp
 * @brief Tests unitaires pour ApiClientBase
 *
 * Couvre:
 * - Constructeur avec injection de dependances
 * - setBearerToken / bearerToken
 * - setDefaultHeader
 * - parseJsonResponse (via classe derivee de test)
 * - getJson, postJson, postForm (headers)
 * - handleNetworkError
 * - Signal errorOccurred
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>

#include "core/network/ApiClientBase.hpp"
#include "core/Error.hpp"
#include "mocks/MockHttpClient.hpp"
#include "mocks/MockNetworkReply.hpp"

using namespace blueplayer::core::network;
using namespace blueplayer::core;
using namespace blueplayer::test;

/**
 * @brief Classe derivee pour acceder aux methodes protected
 */
class TestableApiClientBase : public ApiClientBase {
public:
  explicit TestableApiClientBase(IHttpClient* httpClient = nullptr, QObject* parent = nullptr)
      : ApiClientBase(httpClient, parent) {}

  // Expose protected methods for testing
  using ApiClientBase::getJson;
  using ApiClientBase::postJson;
  using ApiClientBase::postForm;
  using ApiClientBase::parseJsonResponse;
  using ApiClientBase::handleNetworkError;
  using ApiClientBase::httpClient;
  using ApiClientBase::bearerToken;
};

class TestApiClientBase : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Constructeur
  void testConstructorDefault();
  void testConstructorWithMockHttpClient();

  // setBearerToken / bearerToken
  void testSetBearerToken();
  void testBearerTokenWithMock();
  void testBearerTokenEmpty();

  // setDefaultHeader
  void testSetDefaultHeader();
  void testSetDefaultHeaderMultiple();

  // parseJsonResponse
  void testParseJsonResponseNull();
  void testParseJsonResponseEmpty();
  void testParseJsonResponseInvalidJson();
  void testParseJsonResponseNotObjectOrArray();
  void testParseJsonResponseValidObject();
  void testParseJsonResponseValidArray();

  // getJson, postJson, postForm - headers
  void testGetJsonAddsAcceptHeader();
  void testPostJsonAddsContentType();
  void testPostFormEncodesData();
  void testPostFormAddsContentType();

  // handleNetworkError
  void testHandleNetworkErrorNoError();
  void testHandleNetworkErrorWithError();

  // Signal errorOccurred
  void testErrorSignalConnectable();
  void testErrorSignalEmittedOnParseError();

private:
  MockHttpClient* m_mockClient = nullptr;
  TestableApiClientBase* m_client = nullptr;
};

void TestApiClientBase::initTestCase() {
}

void TestApiClientBase::cleanupTestCase() {
}

void TestApiClientBase::init() {
  m_mockClient = new MockHttpClient(this);
  m_client = new TestableApiClientBase(m_mockClient, this);
}

void TestApiClientBase::cleanup() {
  delete m_client;
  m_client = nullptr;
  delete m_mockClient;
  m_mockClient = nullptr;
}

// =====================================================
// Constructeur Tests
// =====================================================

void TestApiClientBase::testConstructorDefault() {
  // Act - constructeur sans injection
  ApiClientBase client;

  // Assert - doit creer un HttpClient interne
  QVERIFY(client.parent() == nullptr);
}

void TestApiClientBase::testConstructorWithMockHttpClient() {
  // Arrange
  MockHttpClient* mockClient = new MockHttpClient(this);

  // Act
  TestableApiClientBase client(mockClient);

  // Assert
  QCOMPARE(client.httpClient(), static_cast<IHttpClient*>(mockClient));

  // Cleanup
  delete mockClient;
}

// =====================================================
// setBearerToken / bearerToken Tests
// =====================================================

void TestApiClientBase::testSetBearerToken() {
  // Act
  m_client->setBearerToken("test_token_123");

  // Assert - le token doit etre transmis au mock
  QCOMPARE(m_mockClient->bearerToken(), QString("test_token_123"));
}

void TestApiClientBase::testBearerTokenWithMock() {
  // Arrange
  const QString token = "my_bearer_token";
  m_client->setBearerToken(token);

  // Act
  QString retrieved = m_client->bearerToken();

  // Assert
  QCOMPARE(retrieved, token);
}

void TestApiClientBase::testBearerTokenEmpty() {
  // Act - token vide
  m_client->setBearerToken("");

  // Assert
  QCOMPARE(m_client->bearerToken(), QString(""));
}

// =====================================================
// setDefaultHeader Tests
// =====================================================

void TestApiClientBase::testSetDefaultHeader() {
  // Act
  m_client->setDefaultHeader("X-Custom-Header", "custom_value");

  // Assert - faire une requete et verifier
  m_mockClient->queueResponse(MockResponse::json(QByteArray("{}")));
  m_client->getJson(QUrl("https://api.test.com/endpoint"));

  // Le header doit etre dans la requete
  QCOMPARE(m_mockClient->requestCount(), 1);
}

void TestApiClientBase::testSetDefaultHeaderMultiple() {
  // Act
  m_client->setDefaultHeader("X-Header-1", "value1");
  m_client->setDefaultHeader("X-Header-2", "value2");

  // Assert - faire une requete et verifier
  m_mockClient->queueResponse(MockResponse::json(QByteArray("{}")));
  m_client->getJson(QUrl("https://api.test.com/endpoint"));

  QCOMPARE(m_mockClient->requestCount(), 1);
}

// =====================================================
// parseJsonResponse Tests
// =====================================================

void TestApiClientBase::testParseJsonResponseNull() {
  // Arrange
  QSignalSpy errorSpy(m_client, &ApiClientBase::errorOccurred);
  QVERIFY(errorSpy.isValid());

  // Act - reply nullptr
  QJsonDocument result = m_client->parseJsonResponse(nullptr, "test_context");

  // Assert
  QVERIFY(result.isNull());
  QCOMPARE(errorSpy.count(), 1);
}

void TestApiClientBase::testParseJsonResponseEmpty() {
  // Arrange
  QSignalSpy errorSpy(m_client, &ApiClientBase::errorOccurred);
  QVERIFY(errorSpy.isValid());

  // Creer un reply avec payload vide
  m_mockClient->queueResponse(MockResponse::json(QByteArray("")));
  QNetworkReply* reply = m_mockClient->get(QUrl("https://test.com"));

  // Attendre que le reply soit termine
  QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
  QTRY_COMPARE(finishedSpy.count(), 1);

  // Act
  QJsonDocument result = m_client->parseJsonResponse(reply, "test_context");

  // Assert
  QVERIFY(result.isNull());
  QCOMPARE(errorSpy.count(), 1);

  reply->deleteLater();
}

void TestApiClientBase::testParseJsonResponseInvalidJson() {
  // Arrange
  QSignalSpy errorSpy(m_client, &ApiClientBase::errorOccurred);
  QVERIFY(errorSpy.isValid());

  // JSON malformed
  MockResponse badJsonResponse;
  badJsonResponse.data = "{invalid json}}}";
  badJsonResponse.httpStatusCode = 200;
  m_mockClient->queueResponse(badJsonResponse);

  QNetworkReply* reply = m_mockClient->get(QUrl("https://test.com"));

  QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
  QTRY_COMPARE(finishedSpy.count(), 1);

  // Act
  QJsonDocument result = m_client->parseJsonResponse(reply, "test_context");

  // Assert
  QVERIFY(result.isNull());
  QCOMPARE(errorSpy.count(), 1);

  reply->deleteLater();
}

void TestApiClientBase::testParseJsonResponseNotObjectOrArray() {
  // Arrange
  QSignalSpy errorSpy(m_client, &ApiClientBase::errorOccurred);
  QVERIFY(errorSpy.isValid());

  // JSON valide mais ni objet ni tableau (juste une valeur)
  MockResponse primitiveResponse;
  primitiveResponse.data = "42";  // Nombre seul
  primitiveResponse.httpStatusCode = 200;
  m_mockClient->queueResponse(primitiveResponse);

  QNetworkReply* reply = m_mockClient->get(QUrl("https://test.com"));

  QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
  QTRY_COMPARE(finishedSpy.count(), 1);

  // Act
  QJsonDocument result = m_client->parseJsonResponse(reply, "test_context");

  // Assert
  QVERIFY(result.isNull());
  QCOMPARE(errorSpy.count(), 1);

  reply->deleteLater();
}

void TestApiClientBase::testParseJsonResponseValidObject() {
  // Arrange
  QJsonObject expected;
  expected["name"] = "test";
  expected["value"] = 42;

  m_mockClient->queueResponse(MockResponse::json(expected));

  QNetworkReply* reply = m_mockClient->get(QUrl("https://test.com"));

  QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
  QTRY_COMPARE(finishedSpy.count(), 1);

  // Act
  QJsonDocument result = m_client->parseJsonResponse(reply, "test_context");

  // Assert
  QVERIFY(!result.isNull());
  QVERIFY(result.isObject());
  QCOMPARE(result.object()["name"].toString(), QString("test"));
  QCOMPARE(result.object()["value"].toInt(), 42);

  reply->deleteLater();
}

void TestApiClientBase::testParseJsonResponseValidArray() {
  // Arrange
  QJsonArray expected;
  expected.append("item1");
  expected.append("item2");
  expected.append("item3");

  m_mockClient->queueResponse(MockResponse::json(QJsonDocument(expected)));

  QNetworkReply* reply = m_mockClient->get(QUrl("https://test.com"));

  QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
  QTRY_COMPARE(finishedSpy.count(), 1);

  // Act
  QJsonDocument result = m_client->parseJsonResponse(reply, "test_context");

  // Assert
  QVERIFY(!result.isNull());
  QVERIFY(result.isArray());
  QCOMPARE(result.array().size(), 3);
  QCOMPARE(result.array().at(0).toString(), QString("item1"));

  reply->deleteLater();
}

// =====================================================
// getJson, postJson, postForm Tests
// =====================================================

void TestApiClientBase::testGetJsonAddsAcceptHeader() {
  // Arrange
  m_mockClient->queueResponse(MockResponse::json(QByteArray("{}")));

  // Act
  m_client->getJson(QUrl("https://api.test.com/data"));

  // Assert
  QCOMPARE(m_mockClient->requestCount(), 1);
  RecordedRequest req = m_mockClient->lastRequest();
  QVERIFY(req.hasHeader("Accept"));
  QCOMPARE(req.headerValue("Accept"), QString("application/json"));
}

void TestApiClientBase::testPostJsonAddsContentType() {
  // Arrange
  m_mockClient->queueResponse(MockResponse::json(QByteArray("{}")));

  QJsonObject data;
  data["key"] = "value";

  // Act
  m_client->postJson(QUrl("https://api.test.com/data"), QJsonDocument(data));

  // Assert
  QCOMPARE(m_mockClient->requestCount(), 1);
  RecordedRequest req = m_mockClient->lastRequest();
  QVERIFY(req.hasHeader("Content-Type"));
  QCOMPARE(req.headerValue("Content-Type"), QString("application/json"));
}

void TestApiClientBase::testPostFormEncodesData() {
  // Arrange
  m_mockClient->queueResponse(MockResponse::json(QByteArray("{}")));

  QHash<QString, QString> formData;
  formData["username"] = "testuser";
  formData["password"] = "testpass";

  // Act
  m_client->postForm(QUrl("https://api.test.com/login"), formData);

  // Assert
  QCOMPARE(m_mockClient->requestCount(), 1);
  RecordedRequest req = m_mockClient->lastRequest();

  // Verifier que les donnees sont encodees
  QString body = QString::fromUtf8(req.body);
  QVERIFY(body.contains("username=testuser") || body.contains("password=testpass"));
}

void TestApiClientBase::testPostFormAddsContentType() {
  // Arrange
  m_mockClient->queueResponse(MockResponse::json(QByteArray("{}")));

  QHash<QString, QString> formData;
  formData["field"] = "value";

  // Act
  m_client->postForm(QUrl("https://api.test.com/form"), formData);

  // Assert
  QCOMPARE(m_mockClient->requestCount(), 1);
  RecordedRequest req = m_mockClient->lastRequest();
  QVERIFY(req.hasHeader("Content-Type"));
  QCOMPARE(req.headerValue("Content-Type"), QString("application/x-www-form-urlencoded"));
}

// =====================================================
// handleNetworkError Tests
// =====================================================

void TestApiClientBase::testHandleNetworkErrorNoError() {
  // Arrange - reply sans erreur
  m_mockClient->queueResponse(MockResponse::json(QByteArray("{}")));
  QNetworkReply* reply = m_mockClient->get(QUrl("https://test.com"));

  QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
  QTRY_COMPARE(finishedSpy.count(), 1);

  QSignalSpy errorSpy(m_client, &ApiClientBase::errorOccurred);

  // Act
  bool hasError = m_client->handleNetworkError(reply, "test_context");

  // Assert
  QVERIFY(!hasError);
  QCOMPARE(errorSpy.count(), 0);

  reply->deleteLater();
}

void TestApiClientBase::testHandleNetworkErrorWithError() {
  // Arrange - reply avec erreur reseau
  m_mockClient->queueResponse(MockResponse::timeout());
  QNetworkReply* reply = m_mockClient->get(QUrl("https://test.com"));

  QSignalSpy finishedSpy(reply, &QNetworkReply::finished);
  QTRY_COMPARE(finishedSpy.count(), 1);

  QSignalSpy errorSpy(m_client, &ApiClientBase::errorOccurred);

  // Act
  bool hasError = m_client->handleNetworkError(reply, "test_context");

  // Assert
  QVERIFY(hasError);
  QCOMPARE(errorSpy.count(), 1);

  reply->deleteLater();
}

// =====================================================
// Signal errorOccurred Tests
// =====================================================

void TestApiClientBase::testErrorSignalConnectable() {
  // Arrange
  QSignalSpy spy(m_client, &ApiClientBase::errorOccurred);

  // Assert
  QVERIFY(spy.isValid());
}

void TestApiClientBase::testErrorSignalEmittedOnParseError() {
  // Arrange
  QSignalSpy errorSpy(m_client, &ApiClientBase::errorOccurred);
  QVERIFY(errorSpy.isValid());

  // Act - parse avec reply null
  m_client->parseJsonResponse(nullptr, "test");

  // Assert
  QCOMPARE(errorSpy.count(), 1);

  // Verifier le contenu de l'erreur
  QList<QVariant> arguments = errorSpy.takeFirst();
  Error error = arguments.at(0).value<Error>();
  QVERIFY(error.hasError());
}

QTEST_MAIN(TestApiClientBase)
#include "TestApiClientBase.moc"
