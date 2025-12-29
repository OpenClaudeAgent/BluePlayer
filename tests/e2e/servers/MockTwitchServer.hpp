#ifndef BLUEPLAYER_TEST_E2E_MOCKTWITCHSERVER_HPP
#define BLUEPLAYER_TEST_E2E_MOCKTWITCHSERVER_HPP

#include "MockHttpServer.hpp"

#include <QJsonArray>
#include <QMap>
#include <QPair>

namespace blueplayer::test::e2e {

/**
 * @brief Mock Twitch API server for E2E testing.
 *
 * Simulates Twitch Helix API endpoints and OAuth validation.
 * Inherits common TCP/HTTP functionality from MockHttpServer.
 *
 * Endpoints:
 * - OAuth: /oauth2/token, /oauth2/validate
 * - Helix: /helix/streams, /helix/users, /helix/channels, etc.
 * - Usher: /api/channel/hls/{channel}.m3u8
 *
 * Usage:
 * @code
 *   MockTwitchServer server;
 *   server.setStreams(streamsJson);
 *   server.start();
 *   QString apiUrl = server.baseUrl();
 * @endcode
 */
class MockTwitchServer : public MockHttpServer
{
    Q_OBJECT

public:
    explicit MockTwitchServer(QObject* parent = nullptr);
    ~MockTwitchServer() override = default;

    // =========================================================================
    // Data Configuration
    // =========================================================================

    /**
     * @brief Set the streams to return from /helix/streams.
     */
    void setStreams(const QJsonArray& streams);

    /**
     * @brief Set the users to return from /helix/users.
     */
    void setUsers(const QJsonArray& users);

    /**
     * @brief Set the channels to return from /helix/channels.
     */
    void setChannels(const QJsonArray& channels);

    /**
     * @brief Set the valid OAuth token for authentication.
     */
    void setValidToken(const QString& token);

    /**
     * @brief Set the HLS server URL for playlist generation.
     */
    void setHlsServerUrl(const QString& hlsServerUrl);

    /**
     * @brief Set the search results to return from /helix/search/channels.
     */
    void setSearchResults(const QJsonArray& searchResults);

    /**
     * @brief Set the videos to return from /helix/videos.
     */
    void setVideos(const QJsonArray& videos);

    // =========================================================================
    // Error Simulation
    // =========================================================================

    /**
     * @brief Simulate an error for the next request to a path.
     * @param path Path to match
     * @param statusCode HTTP status code to return
     * @param message Error message
     */
    void simulateError(const QString& path, int statusCode, const QString& message);

    /**
     * @brief Clear all simulated errors.
     */
    void clearErrors();

protected:
    QString serverName() const override { return "MockTwitchServer"; }
    QByteArray handleRequest(const HttpRequest& request) override;

private:
    // Endpoint handlers
    QByteArray handleOAuthValidate(const HttpRequest& request);
    QByteArray handleOAuthToken();
    QByteArray handleHelixStreams();
    QByteArray handleHelixStreamsFollowed();
    QByteArray handleHelixUsers(const HttpRequest& request);
    QByteArray handleHelixChannels();
    QByteArray handleHelixSearchChannels();
    QByteArray handleHelixGamesTop();
    QByteArray handleHelixVideos();
    QByteArray handleHelixChannelsFollowed();
    QByteArray handleHelixClips();
    QByteArray handleHlsPlaylist(const QString& channel);

    // Data
    QJsonArray m_streams;
    QJsonArray m_users;
    QJsonArray m_channels;
    QJsonArray m_searchResults;
    QJsonArray m_videos;
    QString m_validToken = "test_token_12345";
    QString m_hlsServerUrl;
    QMap<QString, QPair<int, QString>> m_simulatedErrors;
};

} // namespace blueplayer::test::e2e

#endif // BLUEPLAYER_TEST_E2E_MOCKTWITCHSERVER_HPP
