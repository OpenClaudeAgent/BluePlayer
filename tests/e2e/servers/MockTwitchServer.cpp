#include "MockTwitchServer.hpp"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

namespace blueplayer::test::e2e {

MockTwitchServer::MockTwitchServer(QObject* parent)
    : MockHttpServer(parent)
{
}

void MockTwitchServer::setStreams(const QJsonArray& streams)
{
    m_streams = streams;
}

void MockTwitchServer::setUsers(const QJsonArray& users)
{
    m_users = users;
}

void MockTwitchServer::setChannels(const QJsonArray& channels)
{
    m_channels = channels;
}

void MockTwitchServer::setValidToken(const QString& token)
{
    m_validToken = token;
}

void MockTwitchServer::setHlsServerUrl(const QString& hlsServerUrl)
{
    m_hlsServerUrl = hlsServerUrl;
}

void MockTwitchServer::simulateError(const QString& path, int statusCode, const QString& message)
{
    m_simulatedErrors[path] = qMakePair(statusCode, message);
}

void MockTwitchServer::clearErrors()
{
    m_simulatedErrors.clear();
}

QByteArray MockTwitchServer::handleRequest(const HttpRequest& request)
{
    QString path = request.cleanPath();

    // Check for simulated errors
    if (m_simulatedErrors.contains(path)) {
        auto error = m_simulatedErrors.take(path);
        return makeTwitchError(error.first, error.second);
    }

    // OAuth endpoints
    if (path == "/oauth2/validate") {
        return handleOAuthValidate(request);
    }
    if (path == "/oauth2/token") {
        return handleOAuthToken();
    }

    // Helix endpoints
    if (path == "/helix/streams") {
        return handleHelixStreams();
    }
    if (path == "/helix/streams/followed") {
        return handleHelixStreamsFollowed();
    }
    if (path == "/helix/users") {
        return handleHelixUsers(request);
    }
    if (path == "/helix/channels") {
        return handleHelixChannels();
    }
    if (path == "/helix/search/channels") {
        return handleHelixSearchChannels();
    }
    if (path == "/helix/games/top") {
        return handleHelixGamesTop();
    }
    if (path == "/helix/videos") {
        return handleHelixVideos();
    }
    if (path == "/helix/channels/followed") {
        return handleHelixChannelsFollowed();
    }
    if (path == "/helix/clips") {
        return handleHelixClips();
    }

    // HLS playlist endpoint (Usher-style)
    static QRegularExpression hlsPattern("/api/channel/hls/(\\w+)\\.m3u8");
    QRegularExpressionMatch match = hlsPattern.match(path);
    if (match.hasMatch()) {
        return handleHlsPlaylist(match.captured(1));
    }

    // Default: 404
    return makeTwitchError(404, "Not Found");
}

// ============================================================================
// OAuth Handlers
// ============================================================================

QByteArray MockTwitchServer::handleOAuthValidate(const HttpRequest& request)
{
    QString auth = request.headers.value("Authorization");
    if (auth.startsWith("OAuth ") || auth.startsWith("Bearer ")) {
        QString token = auth.section(' ', 1);
        if (token == m_validToken) {
            QJsonObject response;
            response["client_id"] = "test_client_id";
            response["login"] = "test_user";
            response["scopes"] = QJsonArray({"user:read:email", "chat:read", "chat:edit"});
            response["user_id"] = "12345";
            response["expires_in"] = 14400;
            return makeResponse(200, "OK", QJsonDocument(response).toJson());
        }
    }
    return makeTwitchError(401, "Invalid access token");
}

QByteArray MockTwitchServer::handleOAuthToken()
{
    QJsonObject response;
    response["access_token"] = m_validToken;
    response["refresh_token"] = "refresh_" + m_validToken;
    response["expires_in"] = 14400;
    response["scope"] = QJsonArray({"user:read:email", "chat:read", "chat:edit"});
    response["token_type"] = "bearer";
    return makeResponse(200, "OK", QJsonDocument(response).toJson());
}

// ============================================================================
// Helix Handlers
// ============================================================================

QByteArray MockTwitchServer::handleHelixStreams()
{
    return makeTwitchResponse(m_streams);
}

QByteArray MockTwitchServer::handleHelixStreamsFollowed()
{
    return makeTwitchResponse(m_streams);
}

QByteArray MockTwitchServer::handleHelixUsers(const HttpRequest& request)
{
    QString queryString = request.queryString();
    if (queryString.isEmpty() || !queryString.contains("id=")) {
        // Request for authenticated user
        QJsonArray authUser;
        for (const QJsonValue& userVal : m_users) {
            QJsonObject user = userVal.toObject();
            if (user["login"].toString() == "testuser" ||
                user["id"].toString() == "99999") {
                authUser.append(user);
                break;
            }
        }
        if (!authUser.isEmpty()) {
            return makeTwitchResponse(authUser);
        }
    }
    return makeTwitchResponse(m_users);
}

QByteArray MockTwitchServer::handleHelixChannels()
{
    return makeTwitchResponse(m_channels);
}

QByteArray MockTwitchServer::handleHelixSearchChannels()
{
    QJsonArray searchResults;
    for (const QJsonValue& userVal : m_users) {
        QJsonObject user = userVal.toObject();
        QJsonObject result;
        result["id"] = user["id"];
        result["broadcaster_login"] = user["login"];
        result["display_name"] = user["display_name"];
        result["game_id"] = "";
        result["game_name"] = "";
        result["is_live"] = true;
        result["thumbnail_url"] = user["profile_image_url"];
        searchResults.append(result);
    }
    return makeTwitchResponse(searchResults);
}

QByteArray MockTwitchServer::handleHelixGamesTop()
{
    QJsonArray categories;

    QJsonObject cat1;
    cat1["id"] = "509658";
    cat1["name"] = "Just Chatting";
    cat1["box_art_url"] = "https://static-cdn.jtvnw.net/ttv-boxart/509658-{width}x{height}.jpg";
    categories.append(cat1);

    QJsonObject cat2;
    cat2["id"] = "33214";
    cat2["name"] = "Fortnite";
    cat2["box_art_url"] = "https://static-cdn.jtvnw.net/ttv-boxart/33214-{width}x{height}.jpg";
    categories.append(cat2);

    return makeTwitchResponse(categories);
}

QByteArray MockTwitchServer::handleHelixVideos()
{
    return makeTwitchResponse(QJsonArray());
}

QByteArray MockTwitchServer::handleHelixChannelsFollowed()
{
    QJsonArray followedChannels;
    for (const QJsonValue& userVal : m_users) {
        QJsonObject user = userVal.toObject();
        if (user["login"].toString() == "testuser") {
            continue;
        }
        QJsonObject channel;
        channel["broadcaster_id"] = user["id"];
        channel["broadcaster_login"] = user["login"];
        channel["broadcaster_name"] = user["display_name"];
        channel["followed_at"] = "2024-01-01T00:00:00Z";
        followedChannels.append(channel);
    }
    return makeTwitchResponse(followedChannels);
}

QByteArray MockTwitchServer::handleHelixClips()
{
    return makeTwitchResponse(QJsonArray());
}

QByteArray MockTwitchServer::handleHlsPlaylist(const QString& channel)
{
    if (m_hlsServerUrl.isEmpty()) {
        return makeTwitchError(404, "Channel not found");
    }

    QString playlist = QString(
        "#EXTM3U\n"
        "#EXT-X-TWITCH-INFO:NODE=\"video-edge\",MANIFEST-NODE-TYPE=\"weaver\"\n"
        "#EXT-X-MEDIA:TYPE=VIDEO,GROUP-ID=\"chunked\",NAME=\"1080p60\",AUTOSELECT=YES\n"
        "#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,VIDEO=\"chunked\"\n"
        "%1/playlist/%2_chunked.m3u8\n"
        "#EXT-X-MEDIA:TYPE=VIDEO,GROUP-ID=\"720p\",NAME=\"720p\",AUTOSELECT=YES\n"
        "#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,VIDEO=\"720p\"\n"
        "%1/playlist/%2_720p.m3u8\n"
    ).arg(m_hlsServerUrl, channel);

    return makeResponse(200, "OK", playlist.toUtf8(), "application/vnd.apple.mpegurl");
}

} // namespace blueplayer::test::e2e
