#pragma once

#include <QObject>
#include <QString>
#include <QJsonDocument>

namespace blueplayer::core::network {

/**
 * @brief HTTP client using libcurl for requests that require exact header case
 * 
 * This is used specifically for Twitch GraphQL API which rejects lowercase headers.
 * Qt's QNetworkRequest normalizes headers to lowercase, so we use libcurl here.
 */
class CurlHttpClient : public QObject {
  Q_OBJECT

public:
  explicit CurlHttpClient(QObject* parent = nullptr);
  ~CurlHttpClient() override;

  /**
   * @brief Performs a POST request with exact header case preservation
   * @param url The URL to request
   * @param jsonData The JSON data to send
   * @param headers Headers with exact case (e.g., "Client-ID" not "client-id")
   * @return The JSON response document, or null on error
   */
  QJsonDocument postJson(const QUrl& url, 
                        const QJsonDocument& jsonData,
                        const QHash<QString, QString>& headers);

signals:
  /**
   * @brief Emitted when an error occurs
   * @param errorMessage The error message
   */
  void errorOccurred(const QString& errorMessage);
  
  /**
   * @brief Emitted when an HTTP error occurs (non-200 status)
   * @param statusCode The HTTP status code
   * @param errorMessage The error message from the response
   */
  void httpError(int statusCode, const QString& errorMessage);

private:
  class Impl;
  Impl* m_impl;
};

}  // namespace blueplayer::core::network


