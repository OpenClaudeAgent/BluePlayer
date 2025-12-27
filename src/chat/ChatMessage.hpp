#pragma once

#include <QObject>
#include <QString>
#include <QColor>
#include <QVariantList>
#include <QVariantMap>

namespace BluePlayer {

/**
 * @brief Represents a parsed chat message from Twitch IRC
 */
struct ChatMessage {
    Q_GADGET
    Q_PROPERTY(QString id MEMBER id)
    Q_PROPERTY(QString username MEMBER username)
    Q_PROPERTY(QString displayName MEMBER displayName)
    Q_PROPERTY(QString message MEMBER message)
    Q_PROPERTY(QString color MEMBER color)
    Q_PROPERTY(QVariantList badges MEMBER badges)
    Q_PROPERTY(QVariantList emoteParts MEMBER emoteParts)
    Q_PROPERTY(qint64 timestamp MEMBER timestamp)

public:
    QString id;              // Unique message ID
    QString username;        // Lowercase username
    QString displayName;     // Display name with original casing
    QString message;         // Raw message text
    QString color;           // Hex color (e.g., "#FF4500")
    QVariantList badges;     // List of badge objects [{type, version}]
    QVariantList emoteParts; // List of message parts [{type: "text"|"emote", content, emoteId?}]
    qint64 timestamp{0};     // Unix timestamp in milliseconds

    ChatMessage() = default;

    /**
     * @brief Convert to QVariantMap for QML consumption
     */
    QVariantMap toVariantMap() const {
        return {
            {"id", id},
            {"username", username},
            {"displayName", displayName},
            {"message", message},
            {"color", color},
            {"badges", badges},
            {"emoteParts", emoteParts},
            {"timestamp", timestamp}
        };
    }
};

/**
 * @brief Represents an emote position in a message
 */
struct EmotePosition {
    QString emoteId;
    int startIndex;
    int endIndex;
};

} // namespace BluePlayer

Q_DECLARE_METATYPE(BluePlayer::ChatMessage)
