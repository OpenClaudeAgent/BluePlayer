#ifndef BLUEPLAYER_TEST_E2E_MOCKHLSSERVER_HPP
#define BLUEPLAYER_TEST_E2E_MOCKHLSSERVER_HPP

#include "MockHttpServer.hpp"

#include <QHash>

namespace blueplayer::test::e2e {

/**
 * @brief Mock HLS server for E2E testing.
 *
 * Serves HLS playlists and segments for video playback testing.
 * Inherits common TCP/HTTP functionality from MockHttpServer.
 *
 * Endpoints:
 * - /live/{channel} - Redirects to master playlist
 * - /playlist/{channel}_master.m3u8 - Master playlist with quality variants
 * - /playlist/{channel}_{quality}.m3u8 - Media playlist with segments
 * - /segments/{channel}_{quality}_{number}.ts - Video segments
 *
 * Usage:
 * @code
 *   MockHlsServer server;
 *   server.addChannel("streamer123");
 *   server.start();
 *   QString hlsUrl = server.baseUrl() + "/live/streamer123";
 * @endcode
 */
class MockHlsServer : public MockHttpServer
{
    Q_OBJECT

public:
    explicit MockHlsServer(QObject* parent = nullptr);
    ~MockHlsServer() override = default;

    // =========================================================================
    // Channel Configuration
    // =========================================================================

    /**
     * @brief Add a channel that the server will serve HLS for.
     * @param channelName Name of the channel
     * @param segmentCount Number of segments in the playlist
     * @param segmentDuration Duration of each segment in seconds
     */
    void addChannel(const QString& channelName, int segmentCount = 2, int segmentDuration = 1);

    /**
     * @brief Remove a channel.
     */
    void removeChannel(const QString& channelName);

    /**
     * @brief Clear all channels.
     */
    void clearChannels();

    // =========================================================================
    // Segment Management
    // =========================================================================

    /**
     * @brief Load a video segment from file to serve for all segment requests.
     * @param filePath Path to the .ts segment file
     * @return true if file was loaded successfully
     */
    bool loadSegmentFromFile(const QString& filePath);

    /**
     * @brief Get the number of segment requests received.
     */
    int segmentRequestCount() const;

    // =========================================================================
    // Simulation
    // =========================================================================

    /**
     * @brief Simulate network delay for responses.
     * @param delayMs Delay in milliseconds (0 = no delay)
     */
    void setResponseDelay(int delayMs);

    /**
     * @brief Simulate a stall (stop serving segments).
     */
    void simulateStall(bool stall);

Q_SIGNALS:
    /**
     * @brief Emitted when a playlist is requested.
     */
    void playlistRequested(const QString& channel, const QString& quality);

    /**
     * @brief Emitted when a segment is requested.
     */
    void segmentRequested(const QString& channel, int segmentNumber);

protected:
    QString serverName() const override { return "MockHlsServer"; }
    QByteArray handleRequest(const HttpRequest& request) override;

private:
    struct ChannelConfig {
        int segmentCount = 3;
        int segmentDuration = 2;
        int mediaSequence = 0;
    };

    // Endpoint handlers
    QByteArray handleLiveChannel(const QString& channel);
    QByteArray handleMasterPlaylist(const QString& channel);
    QByteArray handleMediaPlaylist(const QString& channel, const QString& quality);
    QByteArray handleSegment(const QString& channel, int segmentNumber);
    QByteArray makeMinimalSegment();

    // Channel configurations
    QHash<QString, ChannelConfig> m_channels;

    // Request tracking
    int m_segmentRequestCount = 0;

    // Simulation
    int m_responseDelayMs = 0;
    bool m_stalled = false;

    // Video segment data
    QByteArray m_segmentData;
};

} // namespace blueplayer::test::e2e

#endif // BLUEPLAYER_TEST_E2E_MOCKHLSSERVER_HPP
