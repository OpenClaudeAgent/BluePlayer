#pragma once

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>

namespace blueplayer::test::e2e {

/**
 * @brief Mock HLS server for E2E testing
 *
 * Serves HLS playlists and segments for video playback testing:
 * - Master playlists (.m3u8) with quality variants
 * - Media playlists (.m3u8) with segment lists
 * - Minimal video segments (.ts)
 *
 * Usage:
 * @code
 *   MockHlsServer server;
 *   server.addChannel("streamer123");
 *   server.start();
 *   
 *   // Use server.baseUrl() for HLS URLs
 * @endcode
 */
class MockHlsServer : public QObject {
  Q_OBJECT

public:
  explicit MockHlsServer(QObject* parent = nullptr);
  ~MockHlsServer() override;

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

  // ===== Channel Configuration =====

  /**
   * @brief Adds a channel that the server will serve HLS for
   * @param channelName Name of the channel
   * @param segmentCount Number of segments in the playlist (default: 3)
   * @param segmentDuration Duration of each segment in seconds (default: 2)
   */
  void addChannel(const QString& channelName, int segmentCount = 3, int segmentDuration = 2);

  /**
   * @brief Removes a channel
   */
  void removeChannel(const QString& channelName);

  /**
   * @brief Clears all channels
   */
  void clearChannels();

  // ===== Request Tracking =====

  /**
   * @brief Returns the number of requests received
   */
  [[nodiscard]] int requestCount() const;

  /**
   * @brief Returns the number of segment requests
   */
  [[nodiscard]] int segmentRequestCount() const;

  /**
   * @brief Clears request counters
   */
  void clearRequests();

  // ===== Simulation =====

  /**
   * @brief Simulates network delay for responses
   * @param delayMs Delay in milliseconds (0 = no delay)
   */
  void setResponseDelay(int delayMs);

  /**
   * @brief Simulates a stall (stop serving segments)
   */
  void simulateStall(bool stall);

Q_SIGNALS:
  /**
   * @brief Emitted when a playlist is requested
   */
  void playlistRequested(const QString& channel, const QString& quality);

  /**
   * @brief Emitted when a segment is requested
   */
  void segmentRequested(const QString& channel, int segmentNumber);

private Q_SLOTS:
  void handleNewConnection();
  void handleClientData();
  void handleClientDisconnected();

private:
  struct ChannelConfig {
    int segmentCount = 3;
    int segmentDuration = 2;
    int mediaSequence = 0;
  };

  QByteArray handleRequest(const QString& path);
  QByteArray makeMasterPlaylist(const QString& channel);
  QByteArray makeMediaPlaylist(const QString& channel, const QString& quality);
  QByteArray makeMinimalSegment();
  QByteArray makeResponse(int statusCode, const QString& statusText,
                          const QByteArray& body, const QString& contentType);

  QPointer<QTcpServer> m_server;
  quint16 m_port = 0;

  // Channel configurations
  QHash<QString, ChannelConfig> m_channels;

  // Request tracking
  int m_requestCount = 0;
  int m_segmentRequestCount = 0;

  // Simulation
  int m_responseDelayMs = 0;
  bool m_stalled = false;
};

} // namespace blueplayer::test::e2e
