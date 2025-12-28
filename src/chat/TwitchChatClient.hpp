#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <QAbstractSocket>
#include <memory>

// Forward declaration
class QWebSocket;

namespace BluePlayer {

/**
 * @brief Twitch IRC chat client using WebSocket
 *
 * Connects to Twitch IRC in anonymous mode (read-only) using justinfan username.
 * Parses messages with tags (badges, emotes, colors).
 */
class TwitchChatClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QString channel READ channel NOTIFY channelChanged)
    Q_PROPERTY(QString connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(bool canSendMessages READ canSendMessages NOTIFY canSendMessagesChanged)

public:
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Error
    };
    Q_ENUM(ConnectionState)

    explicit TwitchChatClient(QObject* parent = nullptr);
    ~TwitchChatClient() override;

    [[nodiscard]] bool isConnected() const;
    [[nodiscard]] QString channel() const;
    [[nodiscard]] QString connectionStatus() const;
    [[nodiscard]] bool canSendMessages() const;

public slots:
    /**
     * @brief Connect to a channel's chat
     * @param channelName The channel name (without #)
     */
    void connectToChannel(const QString& channelName);

    /**
     * @brief Disconnect from current channel
     */
    void disconnect();

    /**
     * @brief Set OAuth credentials for sending messages
     * @param token OAuth access token
     * @param username Twitch username
     */
    void setCredentials(const QString& token, const QString& username);

    /**
     * @brief Send a message to the current channel
     * @param message The message to send
     */
    void sendMessage(const QString& message);

signals:
    /**
     * @brief Emitted when a new chat message is received
     * @param message The parsed message as QVariantMap for QML
     */
    void messageReceived(const QVariantMap& message);

    /**
     * @brief Emitted when connection state changes
     */
    void connectedChanged();
    void channelChanged();
    void connectionStatusChanged();
    void canSendMessagesChanged();

    /**
     * @brief Emitted on connection error
     * @param error Error description
     */
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimer();

protected:
    // Protected for testability - allows test subclasses to access parsing methods
    
    /**
     * @brief Parse a raw IRC message line
     */
    void parseIrcMessage(const QString& rawMessage);

    /**
     * @brief Parse IRC tags (badges, color, emotes, etc.)
     */
    QVariantMap parseTags(const QString& tagsString);

    /**
     * @brief Parse emotes tag and split message into parts
     * @param emotesTag The emotes tag value (e.g., "25:0-4,6-10/1902:12-16")
     * @param message The raw message text
     * @return List of message parts (text and emotes)
     */
    QVariantList parseEmoteParts(const QString& emotesTag, const QString& message);

    /**
     * @brief Parse badges tag
     * @param badgesTag The badges tag value (e.g., "moderator/1,subscriber/12")
     * @return List of badge objects
     */
    QVariantList parseBadges(const QString& badgesTag);

private:

    /**
     * @brief Send raw IRC command
     */
    void sendRaw(const QString& command);

    /**
     * @brief Generate random justinfan username
     */
    static QString generateAnonUsername();

    /**
     * @brief Set connection state and emit signals
     */
    void setConnectionState(ConnectionState state);

    std::unique_ptr<QWebSocket> m_socket;
    QTimer m_reconnectTimer;
    QString m_channel;
    QString m_oauthToken;
    QString m_username;
    ConnectionState m_state{ConnectionState::Disconnected};
    int m_reconnectAttempts{0};
    static constexpr int MAX_RECONNECT_ATTEMPTS = 3;
    static constexpr int RECONNECT_DELAY_MS = 2000;
    static constexpr const char* TWITCH_IRC_URL = "wss://irc-ws.chat.twitch.tv:443";
};

} // namespace BluePlayer
