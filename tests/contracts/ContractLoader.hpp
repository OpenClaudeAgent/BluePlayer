#pragma once

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QStringList>

namespace blueplayer::test {

/**
 * @brief Utility class for loading and validating API contract files
 *
 * Provides methods to load recorded API responses and validate
 * that production code correctly handles the expected contract formats.
 */
class ContractLoader {
public:
    /**
     * @brief Base directory for contract files
     */
    static QString contractsDir() {
        // Look for contracts relative to test executable or source
        QStringList searchPaths = {
            QStringLiteral("contracts"),
            QStringLiteral("../contracts"),
            QStringLiteral("tests/contracts"),
            QStringLiteral("../tests/contracts"),
            QStringLiteral("../../tests/contracts")
        };
        
        for (const QString& path : searchPaths) {
            QDir dir(path);
            if (dir.exists() && dir.exists(QStringLiteral("helix"))) {
                return dir.absolutePath();
            }
        }
        
        // Fallback to absolute path during development
        return QStringLiteral("/Users/user/Projects/BluePlayer/tests/contracts");
    }

    // ========================================================================
    // Helix API Contracts
    // ========================================================================

    static QByteArray loadHelixContract(const QString& name) {
        return loadFile(QStringLiteral("helix/%1").arg(name));
    }

    static QJsonDocument loadStreamsSuccess() {
        return QJsonDocument::fromJson(loadHelixContract(QStringLiteral("streams_success.json")));
    }

    static QJsonDocument loadStreamsEmpty() {
        return QJsonDocument::fromJson(loadHelixContract(QStringLiteral("streams_empty.json")));
    }

    static QJsonDocument loadStreamsUnauthorized() {
        return QJsonDocument::fromJson(loadHelixContract(QStringLiteral("streams_unauthorized.json")));
    }

    static QJsonDocument loadStreamsRateLimited() {
        return QJsonDocument::fromJson(loadHelixContract(QStringLiteral("streams_rate_limited.json")));
    }

    static QJsonDocument loadUsersSuccess() {
        return QJsonDocument::fromJson(loadHelixContract(QStringLiteral("users_success.json")));
    }

    static QJsonDocument loadClipsSuccess() {
        return QJsonDocument::fromJson(loadHelixContract(QStringLiteral("clips_success.json")));
    }

    static QJsonDocument loadVideosSuccess() {
        return QJsonDocument::fromJson(loadHelixContract(QStringLiteral("videos_success.json")));
    }

    static QJsonDocument loadCategoriesSuccess() {
        return QJsonDocument::fromJson(loadHelixContract(QStringLiteral("categories_success.json")));
    }

    // ========================================================================
    // OAuth Contracts
    // ========================================================================

    static QByteArray loadOAuthContract(const QString& name) {
        return loadFile(QStringLiteral("oauth/%1").arg(name));
    }

    static QJsonDocument loadTokenSuccess() {
        return QJsonDocument::fromJson(loadOAuthContract(QStringLiteral("token_success.json")));
    }

    static QJsonDocument loadTokenInvalidRefresh() {
        return QJsonDocument::fromJson(loadOAuthContract(QStringLiteral("token_invalid_refresh.json")));
    }

    // ========================================================================
    // GraphQL Contracts
    // ========================================================================

    static QByteArray loadGraphQLContract(const QString& name) {
        return loadFile(QStringLiteral("graphql/%1").arg(name));
    }

    static QJsonDocument loadPlaybackTokenSuccess() {
        return QJsonDocument::fromJson(loadGraphQLContract(QStringLiteral("playback_token_success.json")));
    }

    // ========================================================================
    // IRC Contracts
    // ========================================================================

    static QString loadIrcContract(const QString& name) {
        return QString::fromUtf8(loadFile(QStringLiteral("irc/%1").arg(name)));
    }

    static QString loadPrivmsgWithEmotes() {
        return loadIrcContract(QStringLiteral("privmsg_with_emotes.txt")).trimmed();
    }

    static QString loadPrivmsgWithBadges() {
        return loadIrcContract(QStringLiteral("privmsg_with_badges.txt")).trimmed();
    }

    static QString loadNoticeAuthFailed() {
        return loadIrcContract(QStringLiteral("notice_auth_failed.txt")).trimmed();
    }

    // ========================================================================
    // HLS Contracts
    // ========================================================================

    static QString loadHlsContract(const QString& name) {
        return QString::fromUtf8(loadFile(QStringLiteral("hls/%1").arg(name)));
    }

    static QString loadMasterPlaylist() {
        return loadHlsContract(QStringLiteral("master_playlist.m3u8"));
    }

    static QString loadVariantClean() {
        return loadHlsContract(QStringLiteral("variant_clean.m3u8"));
    }

    static QString loadVariantWithAds() {
        return loadHlsContract(QStringLiteral("variant_with_ads.m3u8"));
    }

    // ========================================================================
    // Validation Helpers
    // ========================================================================

    /**
     * @brief Validate that a JSON object has all required fields for a stream
     */
    static bool validateStreamObject(const QJsonObject& stream) {
        QStringList requiredFields = {
            QStringLiteral("id"),
            QStringLiteral("user_id"),
            QStringLiteral("user_login"),
            QStringLiteral("user_name"),
            QStringLiteral("game_id"),
            QStringLiteral("game_name"),
            QStringLiteral("type"),
            QStringLiteral("title"),
            QStringLiteral("viewer_count"),
            QStringLiteral("started_at"),
            QStringLiteral("language"),
            QStringLiteral("thumbnail_url"),
            QStringLiteral("is_mature")
        };

        for (const QString& field : requiredFields) {
            if (!stream.contains(field)) {
                return false;
            }
        }

        // Validate thumbnail_url format
        QString thumbnailUrl = stream[QStringLiteral("thumbnail_url")].toString();
        if (!thumbnailUrl.contains(QStringLiteral("{width}")) ||
            !thumbnailUrl.contains(QStringLiteral("{height}"))) {
            return false;
        }

        return true;
    }

    /**
     * @brief Validate that a JSON object has all required fields for a user
     */
    static bool validateUserObject(const QJsonObject& user) {
        QStringList requiredFields = {
            QStringLiteral("id"),
            QStringLiteral("login"),
            QStringLiteral("display_name"),
            QStringLiteral("type"),
            QStringLiteral("broadcaster_type"),
            QStringLiteral("description"),
            QStringLiteral("profile_image_url"),
            QStringLiteral("offline_image_url"),
            QStringLiteral("created_at")
        };

        for (const QString& field : requiredFields) {
            if (!user.contains(field)) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Validate that a JSON object has all required fields for a clip
     */
    static bool validateClipObject(const QJsonObject& clip) {
        QStringList requiredFields = {
            QStringLiteral("id"),
            QStringLiteral("url"),
            QStringLiteral("broadcaster_id"),
            QStringLiteral("broadcaster_name"),
            QStringLiteral("creator_id"),
            QStringLiteral("creator_name"),
            QStringLiteral("title"),
            QStringLiteral("view_count"),
            QStringLiteral("created_at"),
            QStringLiteral("thumbnail_url"),
            QStringLiteral("duration")
        };

        for (const QString& field : requiredFields) {
            if (!clip.contains(field)) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief Validate error response format
     */
    static bool validateErrorResponse(const QJsonObject& error) {
        return error.contains(QStringLiteral("error")) &&
               error.contains(QStringLiteral("status")) &&
               error.contains(QStringLiteral("message"));
    }

    /**
     * @brief Validate OAuth token response format
     */
    static bool validateTokenResponse(const QJsonObject& token) {
        QStringList requiredFields = {
            QStringLiteral("access_token"),
            QStringLiteral("refresh_token"),
            QStringLiteral("expires_in"),
            QStringLiteral("token_type")
        };

        for (const QString& field : requiredFields) {
            if (!token.contains(field)) {
                return false;
            }
        }

        return token[QStringLiteral("token_type")].toString() == QStringLiteral("bearer");
    }

    /**
     * @brief Check if HLS playlist contains ad markers
     */
    static bool containsAdMarkers(const QString& playlist) {
        return playlist.contains(QStringLiteral("twitch-stitched-ad")) ||
               playlist.contains(QStringLiteral("EXT-X-SCTE35-OUT")) ||
               playlist.contains(QStringLiteral("live-ad"));
    }

private:
    static QByteArray loadFile(const QString& relativePath) {
        QString fullPath = QStringLiteral("%1/%2").arg(contractsDir(), relativePath);
        QFile file(fullPath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("ContractLoader: Failed to open %s", qPrintable(fullPath));
            return QByteArray();
        }
        return file.readAll();
    }
};

} // namespace blueplayer::test
