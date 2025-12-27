#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace blueplayer::test {

/**
 * @brief Factory class for creating Twitch test data
 *
 * Provides static factory methods to create consistent test data for
 * Twitch API responses. All methods return QVariantMap or QVariantList
 * for easy integration with Qt's JSON and model systems.
 *
 * Usage:
 * @code
 * auto stream = TwitchTestData::createStream("streamer1", "Playing games", 1000, "Minecraft");
 * auto streams = TwitchTestData::createStreams(5);
 * auto response = TwitchTestData::createApiResponse(streams);
 * @endcode
 */
class TwitchTestData {
public:
    // ========================================================================
    // Stream Factory Methods
    // ========================================================================

    /**
     * @brief Creates a stream data object
     * @param userName The broadcaster's display name
     * @param title The stream title
     * @param viewers Number of current viewers
     * @param gameName The game/category being streamed
     * @return QVariantMap representing a Twitch stream
     */
    static QVariantMap createStream(const QString& userName,
                                    const QString& title,
                                    int viewers,
                                    const QString& gameName) {
        static int streamIdCounter = 1;
        static int userIdCounter = 1000;

        QVariantMap stream;
        stream["id"] = QString::number(streamIdCounter++);
        stream["user_id"] = QString::number(userIdCounter++);
        stream["user_login"] = userName.toLower().replace(" ", "_");
        stream["user_name"] = userName;
        stream["game_id"] = QString::number(qHash(gameName) % 100000);
        stream["game_name"] = gameName;
        stream["type"] = "live";
        stream["title"] = title;
        stream["viewer_count"] = viewers;
        stream["started_at"] = "2024-01-15T10:00:00Z";
        stream["language"] = "en";
        stream["thumbnail_url"] =
            QString("https://static-cdn.jtvnw.net/previews-ttv/live_user_%1-{width}x{height}.jpg")
                .arg(userName.toLower());
        stream["is_mature"] = false;

        return stream;
    }

    /**
     * @brief Creates a list of test streams with auto-generated data
     * @param count Number of streams to generate
     * @return QVariantList containing stream objects
     */
    static QVariantList createStreams(int count) {
        static const QStringList games = {"Minecraft", "Fortnite",    "League of Legends",
                                          "Valorant",  "Just Chatting"};

        QVariantList streams;
        for (int i = 0; i < count; ++i) {
            QString userName = QString("Streamer%1").arg(i + 1);
            QString title = QString("Stream #%1 - Come hang out!").arg(i + 1);
            int viewers = 100 + (i * 50);
            QString game = games[i % games.size()];

            streams.append(createStream(userName, title, viewers, game));
        }
        return streams;
    }

    // ========================================================================
    // Category Factory Methods
    // ========================================================================

    /**
     * @brief Creates a category/game data object
     * @param name The category display name
     * @param id The category ID (auto-generated if empty)
     * @return QVariantMap representing a Twitch category
     */
    static QVariantMap createCategory(const QString& name, const QString& id = QString()) {
        QVariantMap category;
        category["id"] = id.isEmpty() ? QString::number(qHash(name) % 100000) : id;
        category["name"] = name;
        category["box_art_url"] =
            QString("https://static-cdn.jtvnw.net/ttv-boxart/%1-{width}x{height}.jpg")
                .arg(QString(name).replace(" ", "%20"));
        category["igdb_id"] = QString::number(qHash(name) % 50000);

        return category;
    }

    // ========================================================================
    // Clip Factory Methods
    // ========================================================================

    /**
     * @brief Creates a clip data object
     * @param title The clip title
     * @param broadcaster The broadcaster's display name
     * @param views Number of clip views
     * @return QVariantMap representing a Twitch clip
     */
    static QVariantMap createClip(const QString& title, const QString& broadcaster, int views) {
        static int clipIdCounter = 1;

        QVariantMap clip;
        clip["id"] = QString("clip_%1").arg(clipIdCounter++);
        clip["url"] = QString("https://clips.twitch.tv/clip_%1").arg(clipIdCounter);
        clip["embed_url"] = QString("https://clips.twitch.tv/embed?clip=clip_%1").arg(clipIdCounter);
        clip["broadcaster_id"] = QString::number(qHash(broadcaster) % 100000);
        clip["broadcaster_name"] = broadcaster;
        clip["creator_id"] = "12345";
        clip["creator_name"] = "ClipCreator";
        clip["video_id"] = "";
        clip["game_id"] = "12345";
        clip["language"] = "en";
        clip["title"] = title;
        clip["view_count"] = views;
        clip["created_at"] = "2024-01-10T15:30:00Z";
        clip["thumbnail_url"] =
            QString("https://clips-media-assets2.twitch.tv/%1-preview-480x272.jpg")
                .arg(clip["id"].toString());
        clip["duration"] = 30.0;
        clip["vod_offset"] = 3600;

        return clip;
    }

    // ========================================================================
    // Video/VOD Factory Methods
    // ========================================================================

    /**
     * @brief Creates a video/VOD data object
     * @param title The video title
     * @param userName The broadcaster's display name
     * @param views Number of video views
     * @return QVariantMap representing a Twitch video
     */
    static QVariantMap createVideo(const QString& title, const QString& userName, int views) {
        static int videoIdCounter = 1000000;

        QVariantMap video;
        video["id"] = QString::number(videoIdCounter++);
        video["stream_id"] = QString::number(videoIdCounter + 500000);
        video["user_id"] = QString::number(qHash(userName) % 100000);
        video["user_login"] = userName.toLower().replace(" ", "_");
        video["user_name"] = userName;
        video["title"] = title;
        video["description"] = QString("VOD from %1's stream").arg(userName);
        video["created_at"] = "2024-01-14T18:00:00Z";
        video["published_at"] = "2024-01-14T18:00:00Z";
        video["url"] = QString("https://www.twitch.tv/videos/%1").arg(video["id"].toString());
        video["thumbnail_url"] =
            QString("https://static-cdn.jtvnw.net/cf_vods/%1//thumb/thumb0-{width}x{height}.jpg")
                .arg(video["id"].toString());
        video["viewable"] = "public";
        video["view_count"] = views;
        video["language"] = "en";
        video["type"] = "archive";
        video["duration"] = "3h25m10s";
        video["muted_segments"] = QVariantList();

        return video;
    }

    // ========================================================================
    // Channel Factory Methods
    // ========================================================================

    /**
     * @brief Creates a channel/user data object
     * @param displayName The channel's display name
     * @param login The channel's login name (lowercase)
     * @param isLive Whether the channel is currently live
     * @return QVariantMap representing a Twitch channel
     */
    static QVariantMap createChannel(const QString& displayName,
                                     const QString& login,
                                     bool isLive) {
        QVariantMap channel;
        channel["broadcaster_type"] = "partner";
        channel["description"] = QString("Welcome to %1's channel!").arg(displayName);
        channel["display_name"] = displayName;
        channel["id"] = QString::number(qHash(login) % 100000);
        channel["login"] = login.isEmpty() ? displayName.toLower().replace(" ", "_") : login;
        channel["offline_image_url"] = "";
        channel["profile_image_url"] =
            QString("https://static-cdn.jtvnw.net/jtv_user_pictures/%1-profile.png").arg(login);
        channel["type"] = "";
        channel["view_count"] = 0;  // Deprecated but still present
        channel["created_at"] = "2020-01-01T00:00:00Z";

        // Extended fields for search results
        channel["is_live"] = isLive;
        channel["game_id"] = isLive ? "12345" : "";
        channel["game_name"] = isLive ? "Just Chatting" : "";
        channel["title"] = isLive ? QString("%1's stream").arg(displayName) : "";

        return channel;
    }

    // ========================================================================
    // VOD Metadata Factory Methods
    // ========================================================================

    /**
     * @brief Creates VOD metadata for caching
     * @param id The VOD ID
     * @param title The VOD title
     * @param streamer The streamer's name
     * @param size The cached file size in bytes
     * @return QVariantMap representing VOD metadata
     */
    static QVariantMap createVodMetadata(const QString& id,
                                         const QString& title,
                                         const QString& streamer,
                                         qint64 size) {
        QVariantMap metadata;
        metadata["id"] = id;
        metadata["title"] = title;
        metadata["streamer"] = streamer;
        metadata["streamerName"] = streamer;
        metadata["thumbnailUrl"] =
            QString("https://static-cdn.jtvnw.net/cf_vods/%1/thumb.jpg").arg(id);
        metadata["duration"] = "2h30m00s";
        metadata["durationSeconds"] = 9000;
        metadata["size"] = size;
        metadata["cachedAt"] = "2024-01-15T12:00:00Z";
        metadata["quality"] = "source";
        metadata["filePath"] = QString("/cache/vods/%1.mp4").arg(id);

        return metadata;
    }

    // ========================================================================
    // API Response Builders
    // ========================================================================

    /**
     * @brief Wraps data in a standard Twitch API response format
     * @param data QVariantList of data objects
     * @return JSON string in format {"data": [...]}
     */
    static QString createApiResponse(const QVariantList& data) {
        QVariantMap response;
        response["data"] = data;

        QJsonDocument doc = QJsonDocument::fromVariant(response);
        return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    }

    /**
     * @brief Wraps a single data object in a standard Twitch API response
     * @param data Single data object
     * @return JSON string in format {"data": [object]}
     */
    static QString createApiResponse(const QVariantMap& data) {
        return createApiResponse(QVariantList{data});
    }

    /**
     * @brief Creates a Twitch API error response
     * @param error Error type (e.g., "Unauthorized", "Not Found")
     * @param status HTTP status code
     * @param message Detailed error message
     * @return JSON string in Twitch error format
     */
    static QString createErrorResponse(const QString& error, int status, const QString& message) {
        QVariantMap response;
        response["error"] = error;
        response["status"] = status;
        response["message"] = message;

        QJsonDocument doc = QJsonDocument::fromVariant(response);
        return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    }

    // ========================================================================
    // Pagination Helpers
    // ========================================================================

    /**
     * @brief Creates a paginated API response with cursor
     * @param data QVariantList of data objects
     * @param cursor Pagination cursor for next page (empty if last page)
     * @return JSON string with data and pagination
     */
    static QString createPaginatedResponse(const QVariantList& data, const QString& cursor) {
        QVariantMap response;
        response["data"] = data;

        QVariantMap pagination;
        if (!cursor.isEmpty()) {
            pagination["cursor"] = cursor;
        }
        response["pagination"] = pagination;

        QJsonDocument doc = QJsonDocument::fromVariant(response);
        return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    }

private:
    TwitchTestData() = delete;  // Static-only class
};

}  // namespace blueplayer::test
