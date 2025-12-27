#pragma once

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QUrl>

class QNetworkReply;

namespace blueplayer::core::network {

/**
 * @brief Interface for HTTP clients to enable dependency injection and mocking
 *
 * This interface defines the core HTTP operations needed by API clients.
 * Implementations can use Qt's QNetworkAccessManager, libcurl, or be mocked
 * for testing purposes.
 */
class IHttpClient {
public:
  virtual ~IHttpClient() = default;

  /**
   * @brief Performs a GET request
   * @param url The URL to request
   * @param headers Optional HTTP headers
   * @return The QNetworkReply to track the request
   */
  virtual QNetworkReply* get(const QUrl& url, const QHash<QString, QString>& headers = {}) = 0;

  /**
   * @brief Performs a POST request
   * @param url The URL to request
   * @param data The data to send
   * @param headers Optional HTTP headers
   * @return The QNetworkReply to track the request
   */
  virtual QNetworkReply* post(const QUrl& url, const QByteArray& data = {},
                              const QHash<QString, QString>& headers = {}) = 0;

  /**
   * @brief Performs a PUT request
   * @param url The URL to request
   * @param data The data to send
   * @param headers Optional HTTP headers
   * @return The QNetworkReply to track the request
   */
  virtual QNetworkReply* put(const QUrl& url, const QByteArray& data = {},
                             const QHash<QString, QString>& headers = {}) = 0;

  /**
   * @brief Performs a DELETE request
   * @param url The URL to request
   * @param headers Optional HTTP headers
   * @return The QNetworkReply to track the request
   */
  virtual QNetworkReply* deleteResource(const QUrl& url, const QHash<QString, QString>& headers = {}) = 0;

  /**
   * @brief Sets the Bearer token for authentication
   * @param token The Bearer token
   */
  virtual void setBearerToken(const QString& token) = 0;

  /**
   * @brief Sets a default header for all requests
   * @param name The header name
   * @param value The header value
   */
  virtual void setDefaultHeader(const QString& name, const QString& value) = 0;

  /**
   * @brief Gets the current bearer token
   * @return The bearer token, or empty string if not set
   */
  [[nodiscard]] virtual QString bearerToken() const = 0;
};

}  // namespace blueplayer::core::network
