#pragma once

#include "MockNetworkReply.hpp"
#include "core/Error.hpp"
#include "core/network/IHttpClient.hpp"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QByteArray>
#include <QHash>
#include <QString>
#include <QQueue>
#include <QList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDateTime>
#include <functional>

namespace blueplayer::test {

/**
 * @brief Records information about a network request for verification
 */
struct RecordedRequest {
  enum class Method { GET, POST, PUT, DELETE_METHOD };

  Method method;
  QUrl url;
  QByteArray body;
  QHash<QString, QString> headers;
  QNetworkRequest originalRequest;
  qint64 timestamp;

  /**
   * @brief Checks if the request URL contains a specific path segment
   */
  [[nodiscard]] bool urlContains(const QString& segment) const {
    return url.toString().contains(segment);
  }

  /**
   * @brief Checks if the request has a specific header
   */
  [[nodiscard]] bool hasHeader(const QString& name) const {
    return headers.contains(name);
  }

  /**
   * @brief Gets a header value
   */
  [[nodiscard]] QString headerValue(const QString& name) const {
    return headers.value(name);
  }

  /**
   * @brief Gets the request body as JSON
   */
  [[nodiscard]] QJsonDocument bodyAsJson() const {
    return QJsonDocument::fromJson(body);
  }

  /**
   * @brief Returns method as string for debugging
   */
  [[nodiscard]] QString methodString() const {
    switch (method) {
      case Method::GET: return "GET";
      case Method::POST: return "POST";
      case Method::PUT: return "PUT";
      case Method::DELETE_METHOD: return "DELETE";
    }
    return "UNKNOWN";
  }
};

/**
 * @brief Predefined response to return from mock HTTP client
 */
struct MockResponse {
  QByteArray data;
  int httpStatusCode = 200;
  QNetworkReply::NetworkError networkError = QNetworkReply::NoError;
  QString errorString;
  QHash<QString, QString> headers;
  int delayMs = 0;

  /**
   * @brief Creates a successful JSON response
   */
  static MockResponse json(const QByteArray& data, int statusCode = 200) {
    MockResponse response;
    response.data = data;
    response.httpStatusCode = statusCode;
    response.headers["Content-Type"] = "application/json";
    return response;
  }

  /**
   * @brief Creates a successful JSON response from a QJsonDocument
   */
  static MockResponse json(const QJsonDocument& doc, int statusCode = 200) {
    return json(doc.toJson(QJsonDocument::Compact), statusCode);
  }

  /**
   * @brief Creates a successful JSON response from a QJsonObject
   */
  static MockResponse json(const QJsonObject& obj, int statusCode = 200) {
    return json(QJsonDocument(obj), statusCode);
  }

  /**
   * @brief Creates a Twitch-style API response with data array
   */
  static MockResponse twitchApiResponse(const QJsonArray& data, int statusCode = 200) {
    QJsonObject root;
    root["data"] = data;
    return json(root, statusCode);
  }

  /**
   * @brief Creates a Twitch-style error response
   */
  static MockResponse twitchError(const QString& message, int statusCode = 400) {
    QJsonObject root;
    root["error"] = "Error";
    root["status"] = statusCode;
    root["message"] = message;
    return json(root, statusCode);
  }

  /**
   * @brief Creates an error response
   */
  static MockResponse withError(QNetworkReply::NetworkError err, const QString& message) {
    MockResponse response;
    response.networkError = err;
    response.errorString = message;
    return response;
  }

  /**
   * @brief Creates a timeout error response
   */
  static MockResponse timeout() {
    return withError(QNetworkReply::TimeoutError, "Connection timed out");
  }

  /**
   * @brief Creates a connection refused error response
   */
  static MockResponse connectionRefused() {
    return withError(QNetworkReply::ConnectionRefusedError, "Connection refused by server");
  }

  /**
   * @brief Creates a host not found error response
   */
  static MockResponse hostNotFound() {
    return withError(QNetworkReply::HostNotFoundError, "Host not found");
  }

  /**
   * @brief Creates an SSL error response
   */
  static MockResponse sslError() {
    return withError(QNetworkReply::SslHandshakeFailedError, "SSL handshake failed");
  }

  /**
   * @brief Creates a 401 Unauthorized error response
   */
  static MockResponse unauthorized(const QString& message = "Unauthorized") {
    MockResponse response;
    response.httpStatusCode = 401;
    response.networkError = QNetworkReply::AuthenticationRequiredError;
    response.errorString = message;
    response.data = R"({"error":"Unauthorized","status":401,"message":")" + message.toUtf8() + R"("})";
    return response;
  }

  /**
   * @brief Creates a 404 Not Found error response
   */
  static MockResponse notFound(const QString& message = "Not found") {
    MockResponse response;
    response.httpStatusCode = 404;
    response.networkError = QNetworkReply::ContentNotFoundError;
    response.errorString = message;
    response.data = R"({"error":"Not Found","status":404,"message":")" + message.toUtf8() + R"("})";
    return response;
  }

  /**
   * @brief Creates a 500 Internal Server Error response
   */
  static MockResponse serverError(const QString& message = "Internal server error") {
    MockResponse response;
    response.httpStatusCode = 500;
    response.networkError = QNetworkReply::InternalServerError;
    response.errorString = message;
    response.data = R"({"error":"Internal Server Error","status":500,"message":")" + message.toUtf8() + R"("})";
    return response;
  }

  /**
   * @brief Sets a response delay
   */
  MockResponse& withDelay(int ms) {
    delayMs = ms;
    return *this;
  }

  /**
   * @brief Adds a custom header to the response
   */
  MockResponse& withHeader(const QString& name, const QString& value) {
    headers[name] = value;
    return *this;
  }
};

/**
 * @brief URL matcher for conditional responses
 */
using UrlMatcher = std::function<bool(const QUrl&)>;

/**
 * @brief Rule for matching URLs to responses
 */
struct ResponseRule {
  UrlMatcher matcher;
  MockResponse response;
  bool oneTime = false;
  bool used = false;
};

/**
 * @brief Mock HTTP client for unit testing
 * 
 * Implements IHttpClient interface for dependency injection in tests.
 * 
 * Provides a testable HTTP client that can:
 * - Return predefined responses from a queue
 * - Return responses based on URL matching rules
 * - Record all requests for verification
 * - Simulate various error conditions
 * - Support response delays
 * 
 * The client processes responses in this priority order:
 * 1. URL-based rules (first match wins)
 * 2. Queued responses (FIFO)
 * 3. Default response (if set)
 * 4. Empty 200 OK response
 * 
 * Usage:
 * @code
 *   MockHttpClient client;
 *   
 *   // Queue responses
 *   client.queueResponse(MockResponse::json(R"({"data": []})"));
 *   client.queueResponse(MockResponse::timeout());
 *   
 *   // Add URL-based rules
 *   client.whenUrl("/api/users").respond(MockResponse::json(userData));
 *   
 *   // Make requests
 *   auto reply = client.get(QUrl("https://api.example.com/users"));
 *   
 *   // Verify requests
 *   QCOMPARE(client.requestCount(), 1);
 *   QVERIFY(client.lastRequest().urlContains("/users"));
 * @endcode
 */
class MockHttpClient : public QObject, public blueplayer::core::network::IHttpClient {
  // Note: Q_OBJECT removed to avoid MOC namespace issues
  // Use callbacks instead of signals if needed

public:
  /**
   * @brief Constructs a mock HTTP client
   * @param parent The parent QObject
   */
  explicit MockHttpClient(QObject* parent = nullptr)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
  {
  }

  // ===== HTTP Methods =====

  // ===== IHttpClient interface implementation =====

  /**
   * @brief Performs a mock GET request
   * @param url The URL to request
   * @param headers HTTP headers
   * @return A MockNetworkReply configured with the next response
   */
  QNetworkReply* get(const QUrl& url, const QHash<QString, QString>& headers = {}) override {
    return createReply(RecordedRequest::Method::GET, url, {}, headers);
  }

  /**
   * @brief Performs a mock POST request
   * @param url The URL to request
   * @param data The POST data
   * @param headers HTTP headers
   * @return A MockNetworkReply configured with the next response
   */
  QNetworkReply* post(const QUrl& url, const QByteArray& data = {},
                         const QHash<QString, QString>& headers = {}) override {
    return createReply(RecordedRequest::Method::POST, url, data, headers);
  }

  /**
   * @brief Performs a mock PUT request
   * @param url The URL to request
   * @param data The PUT data
   * @param headers HTTP headers
   * @return A MockNetworkReply configured with the next response
   */
  QNetworkReply* put(const QUrl& url, const QByteArray& data = {},
                        const QHash<QString, QString>& headers = {}) override {
    return createReply(RecordedRequest::Method::PUT, url, data, headers);
  }

  /**
   * @brief Performs a mock DELETE request
   * @param url The URL to request
   * @param headers HTTP headers
   * @return A MockNetworkReply configured with the next response
   */
  QNetworkReply* deleteResource(const QUrl& url,
                                    const QHash<QString, QString>& headers = {}) override {
    return createReply(RecordedRequest::Method::DELETE_METHOD, url, {}, headers);
  }

  // ===== Response Configuration =====

  /**
   * @brief Queues a response to be returned for the next request
   * @param response The response to queue
   */
  void queueResponse(const MockResponse& response) {
    m_responseQueue.enqueue(response);
  }

  /**
   * @brief Queues multiple responses
   * @param responses The responses to queue
   */
  void queueResponses(const QList<MockResponse>& responses) {
    for (const auto& response : responses) {
      m_responseQueue.enqueue(response);
    }
  }

  /**
   * @brief Sets the default response when no queued/ruled response matches
   * @param response The default response
   */
  void setDefaultResponse(const MockResponse& response) {
    m_defaultResponse = response;
    m_hasDefaultResponse = true;
  }

  /**
   * @brief Clears the default response
   */
  void clearDefaultResponse() {
    m_hasDefaultResponse = false;
  }

  /**
   * @brief Clears all queued responses
   */
  void clearQueue() {
    m_responseQueue.clear();
  }

  // ===== URL-based Response Rules =====

  /**
   * @brief Helper class for fluent response rule configuration
   */
  class WhenBuilder {
  public:
    WhenBuilder(MockHttpClient* client, UrlMatcher matcher)
      : m_client(client), m_matcher(std::move(matcher)) {}

    /**
     * @brief Sets the response for matching URLs
     */
    MockHttpClient& respond(const MockResponse& response) {
      ResponseRule rule;
      rule.matcher = m_matcher;
      rule.response = response;
      m_client->m_responseRules.append(rule);
      return *m_client;
    }

    /**
     * @brief Sets a one-time response for matching URLs
     */
    MockHttpClient& respondOnce(const MockResponse& response) {
      ResponseRule rule;
      rule.matcher = m_matcher;
      rule.response = response;
      rule.oneTime = true;
      m_client->m_responseRules.append(rule);
      return *m_client;
    }

  private:
    MockHttpClient* m_client;
    UrlMatcher m_matcher;
  };

  /**
   * @brief Creates a URL matching rule for URLs containing the given string
   * @param urlPart The URL part to match
   * @return A WhenBuilder for configuring the response
   */
  WhenBuilder whenUrl(const QString& urlPart) {
    return WhenBuilder(this, [urlPart](const QUrl& url) {
      return url.toString().contains(urlPart);
    });
  }

  /**
   * @brief Creates a URL matching rule for URLs matching a regex
   * @param pattern The regex pattern
   * @return A WhenBuilder for configuring the response
   */
  WhenBuilder whenUrlMatches(const QRegularExpression& pattern) {
    return WhenBuilder(this, [pattern](const QUrl& url) {
      return pattern.match(url.toString()).hasMatch();
    });
  }

  /**
   * @brief Creates a URL matching rule for exact URL matches
   * @param exactUrl The exact URL to match
   * @return A WhenBuilder for configuring the response
   */
  WhenBuilder whenUrlEquals(const QString& exactUrl) {
    return WhenBuilder(this, [exactUrl](const QUrl& url) {
      return url.toString() == exactUrl;
    });
  }

  /**
   * @brief Creates a custom URL matching rule
   * @param matcher Custom matcher function
   * @return A WhenBuilder for configuring the response
   */
  WhenBuilder when(UrlMatcher matcher) {
    return WhenBuilder(this, std::move(matcher));
  }

  /**
   * @brief Clears all response rules
   */
  void clearRules() {
    m_responseRules.clear();
  }

  // ===== Request Recording & Verification =====

  /**
   * @brief Gets all recorded requests
   * @return List of recorded requests
   */
  [[nodiscard]] QList<RecordedRequest> requests() const {
    return m_recordedRequests;
  }

  /**
   * @brief Gets the number of recorded requests
   * @return Request count
   */
  [[nodiscard]] int requestCount() const {
    return m_recordedRequests.size();
  }

  /**
   * @brief Gets requests of a specific method type
   * @param method The HTTP method
   * @return List of matching requests
   */
  [[nodiscard]] QList<RecordedRequest> requestsByMethod(RecordedRequest::Method method) const {
    QList<RecordedRequest> result;
    for (const auto& req : m_recordedRequests) {
      if (req.method == method) {
        result.append(req);
      }
    }
    return result;
  }

  /**
   * @brief Gets the last recorded request
   * @return The last request, or a default-constructed RecordedRequest if none
   */
  [[nodiscard]] RecordedRequest lastRequest() const {
    return m_recordedRequests.isEmpty() ? RecordedRequest() : m_recordedRequests.last();
  }

  /**
   * @brief Gets a specific recorded request by index
   * @param index The request index
   * @return The request at the given index
   */
  [[nodiscard]] RecordedRequest requestAt(int index) const {
    return m_recordedRequests.value(index);
  }

  /**
   * @brief Checks if any request was made to a URL containing the given string
   * @param urlPart The URL part to search for
   * @return true if any matching request was made
   */
  [[nodiscard]] bool hasRequestTo(const QString& urlPart) const {
    for (const auto& req : m_recordedRequests) {
      if (req.urlContains(urlPart)) {
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Gets requests matching a URL part
   * @param urlPart The URL part to match
   * @return List of matching requests
   */
  [[nodiscard]] QList<RecordedRequest> requestsTo(const QString& urlPart) const {
    QList<RecordedRequest> result;
    for (const auto& req : m_recordedRequests) {
      if (req.urlContains(urlPart)) {
        result.append(req);
      }
    }
    return result;
  }

  /**
   * @brief Clears all recorded requests
   */
  void clearRequests() {
    m_recordedRequests.clear();
  }

  // ===== Configuration =====

  /**
   * @brief Sets the bearer token
   * @param token The bearer token
   */
  void setBearerToken(const QString& token) override {
    m_bearerToken = token;
  }

  /**
   * @brief Gets the bearer token
   * @return The bearer token
   */
  [[nodiscard]] QString bearerToken() const override {
    return m_bearerToken;
  }

  /**
   * @brief Sets a default header
   * @param name The header name
   * @param value The header value
   */
  void setDefaultHeader(const QString& name, const QString& value) override {
    m_defaultHeaders[name] = value;
  }

  /**
   * @brief Removes a default header
   * @param name The header name
   */
  void removeDefaultHeader(const QString& name) {
    m_defaultHeaders.remove(name);
  }

  /**
   * @brief Clears all default headers
   */
  void clearDefaultHeaders() {
    m_defaultHeaders.clear();
  }

  /**
   * @brief Gets the network manager (for compatibility)
   * @return The QNetworkAccessManager
   */
  [[nodiscard]] QNetworkAccessManager* networkManager() const {
    return m_networkManager;
  }

  /**
   * @brief Checks if there are queued responses remaining
   * @return true if queue is not empty
   */
  [[nodiscard]] bool hasQueuedResponses() const {
    return !m_responseQueue.isEmpty();
  }

  /**
   * @brief Gets the number of queued responses
   * @return Queue size
   */
  [[nodiscard]] int queuedResponseCount() const {
    return m_responseQueue.size();
  }

  /**
   * @brief Resets the mock client to initial state
   */
  void reset() {
    clearQueue();
    clearRules();
    clearRequests();
    clearDefaultResponse();
    clearDefaultHeaders();
    m_bearerToken.clear();
  }

  // Error callback (use instead of signals since Q_OBJECT is removed)
  using ErrorCallback = std::function<void(const blueplayer::core::Error&)>;
  void setErrorCallback(ErrorCallback callback) { m_errorCallback = callback; }
  
private:
  ErrorCallback m_errorCallback;

private:
  /**
   * @brief Creates a mock reply for a request
   */
  MockNetworkReply* createReply(RecordedRequest::Method method,
                                  const QUrl& url,
                                  const QByteArray& body,
                                  const QHash<QString, QString>& headers) {
    // Record the request
    RecordedRequest record;
    record.method = method;
    record.url = url;
    record.body = body;
    record.headers = headers;
    record.timestamp = QDateTime::currentMSecsSinceEpoch();

    // Add default headers
    for (auto it = m_defaultHeaders.constBegin(); it != m_defaultHeaders.constEnd(); ++it) {
      if (!record.headers.contains(it.key())) {
        record.headers[it.key()] = it.value();
      }
    }

    // Add bearer token if set
    if (!m_bearerToken.isEmpty() && !record.headers.contains("Authorization")) {
      record.headers["Authorization"] = QString("Bearer %1").arg(m_bearerToken);
    }

    m_recordedRequests.append(record);

    // Find the response to use
    MockResponse response = findResponse(url);

    // Create the mock reply
    auto* reply = new MockNetworkReply(this);
    reply->setData(response.data);
    reply->setHttpStatusCode(response.httpStatusCode);

    if (response.networkError != QNetworkReply::NoError) {
      reply->setNetworkError(response.networkError, response.errorString);
    }

    for (auto it = response.headers.constBegin(); it != response.headers.constEnd(); ++it) {
      reply->setResponseHeader(it.key().toUtf8(), it.value().toUtf8());
    }

    reply->setResponseDelay(response.delayMs);
    reply->setReplyUrl(url);

    // Auto-finish after a short delay to allow signal connections
    QTimer::singleShot(response.delayMs > 0 ? response.delayMs : 0, reply, &MockNetworkReply::finish);

    return reply;
  }

  /**
   * @brief Finds the appropriate response for a URL
   */
  MockResponse findResponse(const QUrl& url) {
    // Check URL-based rules first
    for (int i = 0; i < m_responseRules.size(); ++i) {
      auto& rule = m_responseRules[i];
      if (!rule.used && rule.matcher(url)) {
        if (rule.oneTime) {
          rule.used = true;
        }
        return rule.response;
      }
    }

    // Clean up used one-time rules
    m_responseRules.erase(
        std::remove_if(m_responseRules.begin(), m_responseRules.end(),
                        [](const ResponseRule& r) { return r.oneTime && r.used; }),
        m_responseRules.end());

    // Check queue
    if (!m_responseQueue.isEmpty()) {
      return m_responseQueue.dequeue();
    }

    // Use default response
    if (m_hasDefaultResponse) {
      return m_defaultResponse;
    }

    // Return empty 200 OK
    return MockResponse::json(QByteArray("{}"));
  }

  QNetworkAccessManager* m_networkManager;
  QQueue<MockResponse> m_responseQueue;
  QList<ResponseRule> m_responseRules;
  QList<RecordedRequest> m_recordedRequests;
  QHash<QString, QString> m_defaultHeaders;
  QString m_bearerToken;
  MockResponse m_defaultResponse;
  bool m_hasDefaultResponse = false;
};

}  // namespace blueplayer::test
