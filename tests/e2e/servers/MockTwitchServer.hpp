#pragma once

#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>

namespace blueplayer::test::e2e {

/**
 * @brief Mock Twitch API server for E2E testing
 *
 * Simulates Twitch API endpoints using a simple HTTP server:
 * - OAuth: /oauth2/token, /oauth2/validate
 * - Helix: /helix/streams, /helix/users, /helix/channels
 * - Usher: /api/channel/hls/{channel}.m3u8
 *
 * Uses QTcpServer for maximum compatibility (no Qt HttpServer dependency).
 *
 * Usage:
 * @code
 *   MockTwitchServer server;
 *   server.setStreams(streamsJson);
 *   server.start();
 *   
 *   // Use server.baseUrl() as the API endpoint
 *   QString apiUrl = server.baseUrl();
 * @endcode
 */
class MockTwitchServer : public QObject {
  Q_OBJECT

public:
  explicit MockTwitchServer(QObject* parent = nullptr);
  ~MockTwitchServer() override;

  /**
   * @brief Starts the mock server
   * @param port Port to listen on (0 = auto-select)
   * @return true if server started successfully
   */
  bool start(quint16 port = 0);

  /**
   * @brief Stops the mock server
   */
  void stop();

  /**
   * @brief Returns the base URL of the mock server
   * @return URL like "http://localhost:12345"
   */
  [[nodiscard]] QString baseUrl() const;

  /**
   * @brief Returns the port the server is listening on
   */
  [[nodiscard]] quint16 port() const;

  /**
   * @brief Check if server is running
   */
  [[nodiscard]] bool isRunning() const;

  // ===== Fixture Configuration =====

  /**
   * @brief Sets the streams data to return from /helix/streams
   */
  void setStreams(const QJsonArray& streams);

  /**
   * @brief Sets the users data to return from /helix/users
   */
  void setUsers(const QJsonArray& users);

  /**
   * @brief Sets the channels data to return from /helix/channels
   */
  void setChannels(const QJsonArray& channels);

  /**
   * @brief Sets a valid OAuth token for authentication
   */
  void setValidToken(const QString& token);

  /**
   * @brief Sets the HLS playlist URL template
   * The {channel} placeholder will be replaced with the channel name
   */
  void setHlsServerUrl(const QString& hlsServerUrl);

  // ===== Request Tracking =====

  /**
   * @brief Returns the number of requests received
   */
  [[nodiscard]] int requestCount() const;

  /**
   * @brief Returns requests to a specific endpoint
   */
  [[nodiscard]] QStringList requestsTo(const QString& path) const;

  /**
   * @brief Clears request history
   */
  void clearRequests();

  // ===== Error Simulation =====

  /**
   * @brief Simulates an error for the next request to a path
   */
  void simulateError(const QString& path, int statusCode, const QString& message);

  /**
   * @brief Clears all error simulations
   */
  void clearErrors();

Q_SIGNALS:
  /**
   * @brief Emitted when a request is received
   */
  void requestReceived(const QString& method, const QString& path);

private Q_SLOTS:
  void handleNewConnection();
  void handleClientData();
  void handleClientDisconnected();

private:
  struct HttpRequest {
    QString method;
    QString path;
    QString version;
    QHash<QString, QString> headers;
    QByteArray body;
  };

  HttpRequest parseRequest(const QByteArray& data);
  QByteArray handleRequest(const HttpRequest& request);
  QByteArray makeResponse(int statusCode, const QString& statusText,
                          const QByteArray& body,
                          const QString& contentType = "application/json");
  QByteArray makeTwitchResponse(const QJsonArray& data);
  QByteArray makeTwitchError(int statusCode, const QString& message);

  QPointer<QTcpServer> m_server;
  quint16 m_port = 0;

  // Fixture data
  QJsonArray m_streams;
  QJsonArray m_users;
  QJsonArray m_channels;
  QString m_validToken = "test_token_12345";
  QString m_hlsServerUrl;

  // Request tracking
  QStringList m_requestLog;

  // Error simulation
  QHash<QString, QPair<int, QString>> m_simulatedErrors;
};

} // namespace blueplayer::test::e2e
