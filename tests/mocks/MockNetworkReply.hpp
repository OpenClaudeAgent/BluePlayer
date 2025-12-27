#pragma once

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QBuffer>
#include <QTimer>
#include <QUrl>
#include <QByteArray>
#include <QVariant>

namespace blueplayer::test {

/**
 * @brief Mock implementation of QNetworkReply for unit testing
 * 
 * Provides a fully controllable QNetworkReply that can:
 * - Return predefined response data
 * - Simulate various HTTP status codes
 * - Simulate network errors (timeout, connection refused, etc.)
 * - Simulate response delays
 * - Track read operations
 * 
 * Usage:
 * @code
 *   auto reply = new MockNetworkReply();
 *   reply->setData(R"({"data": []})");
 *   reply->setHttpStatusCode(200);
 *   reply->finish(); // Emits finished() signal
 * @endcode
 */
class MockNetworkReply : public QNetworkReply {
  Q_OBJECT

public:
  /**
   * @brief Constructs a mock network reply
   * @param parent The parent QObject
   */
  explicit MockNetworkReply(QObject* parent = nullptr)
    : QNetworkReply(parent)
    , m_offset(0)
    , m_httpStatusCode(200)
    , m_networkError(QNetworkReply::NoError)
    , m_delayMs(0)
  {
    open(QIODevice::ReadOnly);
  }

  /**
   * @brief Factory method to create a successful JSON response
   * @param jsonData The JSON data to return
   * @param statusCode The HTTP status code (default: 200)
   * @param parent The parent QObject
   * @return A configured MockNetworkReply
   */
  static MockNetworkReply* createJsonResponse(
      const QByteArray& jsonData,
      int statusCode = 200,
      QObject* parent = nullptr)
  {
    auto* reply = new MockNetworkReply(parent);
    reply->setData(jsonData);
    reply->setHttpStatusCode(statusCode);
    reply->setContentType("application/json");
    return reply;
  }

  /**
   * @brief Factory method to create an error response
   * @param error The network error type
   * @param errorString The error message
   * @param parent The parent QObject
   * @return A configured MockNetworkReply with error
   */
  static MockNetworkReply* createErrorResponse(
      QNetworkReply::NetworkError error,
      const QString& errorString,
      QObject* parent = nullptr)
  {
    auto* reply = new MockNetworkReply(parent);
    reply->setNetworkError(error, errorString);
    return reply;
  }

  /**
   * @brief Factory method to create a timeout error response
   * @param parent The parent QObject
   * @return A configured MockNetworkReply with timeout error
   */
  static MockNetworkReply* createTimeoutResponse(QObject* parent = nullptr) {
    return createErrorResponse(
        QNetworkReply::TimeoutError,
        "Connection timed out",
        parent);
  }

  /**
   * @brief Factory method to create a connection refused error response
   * @param parent The parent QObject
   * @return A configured MockNetworkReply with connection refused error
   */
  static MockNetworkReply* createConnectionRefusedResponse(QObject* parent = nullptr) {
    return createErrorResponse(
        QNetworkReply::ConnectionRefusedError,
        "Connection refused by server",
        parent);
  }

  /**
   * @brief Factory method to create a host not found error response
   * @param parent The parent QObject
   * @return A configured MockNetworkReply with host not found error
   */
  static MockNetworkReply* createHostNotFoundResponse(QObject* parent = nullptr) {
    return createErrorResponse(
        QNetworkReply::HostNotFoundError,
        "Host not found",
        parent);
  }

  /**
   * @brief Factory method to create an SSL error response
   * @param parent The parent QObject
   * @return A configured MockNetworkReply with SSL error
   */
  static MockNetworkReply* createSslErrorResponse(QObject* parent = nullptr) {
    return createErrorResponse(
        QNetworkReply::SslHandshakeFailedError,
        "SSL handshake failed",
        parent);
  }

  /**
   * @brief Factory method to create an HTTP error response (4xx/5xx)
   * @param statusCode The HTTP status code
   * @param errorBody The error response body
   * @param parent The parent QObject
   * @return A configured MockNetworkReply with HTTP error
   */
  static MockNetworkReply* createHttpErrorResponse(
      int statusCode,
      const QByteArray& errorBody = {},
      QObject* parent = nullptr)
  {
    auto* reply = new MockNetworkReply(parent);
    reply->setData(errorBody);
    reply->setHttpStatusCode(statusCode);

    // Map HTTP status codes to appropriate network errors
    if (statusCode == 401 || statusCode == 403) {
      reply->setNetworkError(QNetworkReply::AuthenticationRequiredError,
                              "Authentication required");
    } else if (statusCode == 404) {
      reply->setNetworkError(QNetworkReply::ContentNotFoundError,
                              "Content not found");
    } else if (statusCode >= 400 && statusCode < 500) {
      reply->setNetworkError(QNetworkReply::ContentOperationNotPermittedError,
                              QString("HTTP error %1").arg(statusCode));
    } else if (statusCode >= 500) {
      reply->setNetworkError(QNetworkReply::InternalServerError,
                              QString("Server error %1").arg(statusCode));
    }

    return reply;
  }

  // QIODevice interface implementation
  void abort() override {
    m_offset = 0;
    m_data.clear();
    emit finished();
  }

  qint64 bytesAvailable() const override {
    return m_data.size() - m_offset + QIODevice::bytesAvailable();
  }

  bool isSequential() const override {
    return true;
  }

  qint64 size() const override {
    return m_data.size();
  }

  bool atEnd() const override {
    return m_offset >= m_data.size();
  }

  // Configuration methods

  /**
   * @brief Sets the response data
   * @param data The data to return when read
   */
  void setData(const QByteArray& data) {
    m_data = data;
    m_offset = 0;
  }

  /**
   * @brief Sets the HTTP status code
   * @param statusCode The HTTP status code
   */
  void setHttpStatusCode(int statusCode) {
    m_httpStatusCode = statusCode;
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, statusCode);
  }

  /**
   * @brief Sets the network error
   * @param error The error type
   * @param errorString The error message
   */
  void setNetworkError(QNetworkReply::NetworkError error, const QString& errorString) {
    m_networkError = error;
    setError(error, errorString);
  }

  /**
   * @brief Sets the content type header
   * @param contentType The content type (e.g., "application/json")
   */
  void setContentType(const QString& contentType) {
    setRawHeader("Content-Type", contentType.toUtf8());
  }

  /**
   * @brief Sets a response header (public wrapper for setRawHeader)
   * @param name The header name
   * @param value The header value
   */
  void setResponseHeader(const QByteArray& name, const QByteArray& value) {
    setRawHeader(name, value);
  }

  /**
   * @brief Sets the reply URL (public wrapper for setUrl)
   * @param url The URL
   */
  void setReplyUrl(const QUrl& url) {
    setUrl(url);
  }

  /**
   * @brief Sets the request that triggered this reply
   * @param request The original request
   */
  void setMockRequest(const QNetworkRequest& request) {
    setRequest(request);
    setUrl(request.url());
    setOperation(QNetworkAccessManager::GetOperation);
  }

  /**
   * @brief Sets a response delay before finishing
   * @param delayMs Delay in milliseconds
   */
  void setResponseDelay(int delayMs) {
    m_delayMs = delayMs;
  }

  /**
   * @brief Gets the HTTP status code
   * @return The HTTP status code
   */
  [[nodiscard]] int httpStatusCode() const {
    return m_httpStatusCode;
  }

  /**
   * @brief Gets the response data
   * @return The response data
   */
  [[nodiscard]] QByteArray responseData() const {
    return m_data;
  }

  /**
   * @brief Gets the number of bytes that have been read
   * @return Bytes read
   */
  [[nodiscard]] qint64 bytesRead() const {
    return m_offset;
  }

  /**
   * @brief Completes the request and emits finished signal
   * 
   * If a delay is set, the finish will be delayed accordingly.
   */
  void finish() {
    if (m_delayMs > 0) {
      QTimer::singleShot(m_delayMs, this, [this]() {
        setFinished(true);
        emit readyRead();
        emit finished();
      });
    } else {
      setFinished(true);
      emit readyRead();
      emit finished();
    }
  }

  /**
   * @brief Simulates a timeout by setting error and finishing
   */
  void simulateTimeout() {
    setNetworkError(QNetworkReply::TimeoutError, "Operation timed out");
    finish();
  }

  /**
   * @brief Simulates download progress
   * @param bytesReceived Current bytes received
   * @param bytesTotal Total bytes expected
   */
  void simulateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    emit downloadProgress(bytesReceived, bytesTotal);
  }

protected:
  qint64 readData(char* data, qint64 maxSize) override {
    if (m_offset >= m_data.size()) {
      return -1;  // EOF
    }

    qint64 bytesToCopy = qMin(maxSize, static_cast<qint64>(m_data.size()) - m_offset);
    memcpy(data, m_data.constData() + m_offset, bytesToCopy);
    m_offset += bytesToCopy;
    return bytesToCopy;
  }

  qint64 writeData(const char* /*data*/, qint64 /*maxSize*/) override {
    return -1;  // Read-only
  }

private:
  QByteArray m_data;
  qint64 m_offset;
  int m_httpStatusCode;
  QNetworkReply::NetworkError m_networkError;
  int m_delayMs;
};

}  // namespace blueplayer::test
