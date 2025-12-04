#include "core/network/CurlHttpClient.hpp"

#include "core/Logger.hpp"

#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QByteArray>

// libcurl headers
#include <curl/curl.h>

#include <memory>
#include <sstream>

namespace blueplayer::core::network {

class CurlHttpClient::Impl {
public:
  struct WriteData {
    std::string data;
  };

  static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    WriteData* writeData = static_cast<WriteData*>(userp);
    size_t totalSize = size * nmemb;
    writeData->data.append(static_cast<char*>(contents), totalSize);
    return totalSize;
  }

  CURL* curl = nullptr;
  bool initialized = false;

  Impl() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    initialized = (curl != nullptr);
  }

  ~Impl() {
    if (curl) {
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
  }
};

CurlHttpClient::CurlHttpClient(QObject* parent)
    : QObject(parent),
      m_impl(new Impl()) {
  if (!m_impl->initialized) {
    Logger::error(LogCategory::Network, QStringLiteral("Failed to initialize libcurl"));
  }
}

CurlHttpClient::~CurlHttpClient() {
  delete m_impl;
}

QJsonDocument CurlHttpClient::postJson(const QUrl& url,
                                       const QJsonDocument& jsonData,
                                       const QHash<QString, QString>& headers) {
  if (!m_impl->initialized || !m_impl->curl) {
    emit errorOccurred(QStringLiteral("libcurl not initialized"));
    return QJsonDocument();
  }

  CURL* curl = m_impl->curl;
  
  // Reset curl handle for new request
  curl_easy_reset(curl);

  // Set URL
  QByteArray urlBytes = url.toString().toUtf8();
  curl_easy_setopt(curl, CURLOPT_URL, urlBytes.constData());

  // Set POST data
  QByteArray jsonBytes = jsonData.toJson(QJsonDocument::Compact);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBytes.constData());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonBytes.size());

  // Build header list with EXACT case preservation
  struct curl_slist* headerList = nullptr;
  
  // Add Content-Type first
  headerList = curl_slist_append(headerList, "Content-Type: application/json");
  
  // Add custom headers with exact case
  for (auto it = headers.constBegin(); it != headers.constEnd(); ++it) {
    QString headerLine = QStringLiteral("%1: %2").arg(it.key(), it.value());
    QByteArray headerBytes = headerLine.toUtf8();
    headerList = curl_slist_append(headerList, headerBytes.constData());
    
    QString logValue = (it.key().compare("Authorization", Qt::CaseInsensitive) == 0) 
                       ? QStringLiteral("%1: Bearer ***").arg(it.key())
                       : headerLine;
    Logger::debug(LogCategory::Network, 
                  QStringLiteral("[CurlHttpClient] Adding header with exact case: %1").arg(logValue));
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

  // Set write callback
  Impl::WriteData writeData;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Impl::WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);

  // Set timeout
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

  // Follow redirects
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

  // Perform request
  CURLcode res = curl_easy_perform(curl);

  // Get HTTP response code
  long responseCode = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

  // Clean up header list
  curl_slist_free_all(headerList);

  if (res != CURLE_OK) {
    QString errorMsg = QStringLiteral("libcurl error: %1").arg(curl_easy_strerror(res));
    Logger::error(LogCategory::Network, errorMsg);
    emit errorOccurred(errorMsg);
    return QJsonDocument();
  }

  Logger::debug(LogCategory::Network, 
                QStringLiteral("[CurlHttpClient] Response code: %1, size: %2 bytes")
                .arg(responseCode).arg(writeData.data.size()));

  // Log response for debugging
  Logger::debug(LogCategory::Network, 
                QStringLiteral("[CurlHttpClient] Response body: %1").arg(QString::fromStdString(writeData.data).left(500)));
  
  if (responseCode != 200) {
    QString errorMsg = QStringLiteral("HTTP error %1: %2")
                       .arg(responseCode)
                       .arg(QString::fromStdString(writeData.data));
    Logger::error(LogCategory::Network, errorMsg);
    
    // Log detailed error information
    if (responseCode == 400) {
      Logger::error(LogCategory::Network, QStringLiteral("[CurlHttpClient] Bad Request (400) - Check Client-ID and Authorization headers"));
      Logger::error(LogCategory::Network, QStringLiteral("[CurlHttpClient] Verify token was generated with same Client-ID"));
    }
    
    emit errorOccurred(errorMsg);
    emit httpError(static_cast<int>(responseCode), QString::fromStdString(writeData.data));
    return QJsonDocument();
  }

  // Parse JSON response
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(
      QByteArray::fromStdString(writeData.data), &parseError);

  if (parseError.error != QJsonParseError::NoError) {
    QString errorMsg = QStringLiteral("JSON parse error: %1").arg(parseError.errorString());
    Logger::error(LogCategory::Network, errorMsg);
    emit errorOccurred(errorMsg);
    return QJsonDocument();
  }

  return doc;
}

}  // namespace blueplayer::core::network

