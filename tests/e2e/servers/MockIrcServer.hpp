#pragma once

#include <QObject>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include <QList>
#include <QString>
#include <QTimer>
#include <memory>

namespace E2E {

/**
 * @brief Mock IRC server for E2E testing of TwitchChatClient
 *
 * Simulates Twitch IRC over WebSocket:
 * - Handles CAP, PASS, NICK, JOIN handshake
 * - Can inject PRIVMSG to simulate incoming chat messages
 * - Tracks outgoing PRIVMSG from client
 */
class MockIrcServer : public QObject {
    Q_OBJECT

public:
    explicit MockIrcServer(QObject* parent = nullptr);
    ~MockIrcServer() override;

    /**
     * @brief Start the server on a random available port
     * @return true if started successfully
     */
    bool start();

    /**
     * @brief Stop the server
     */
    void stop();

    /**
     * @brief Get the port the server is listening on
     */
    [[nodiscard]] quint16 port() const;

    /**
     * @brief Get WebSocket URL for client connection
     */
    [[nodiscard]] QString url() const;

    /**
     * @brief Simulate an incoming chat message from another user
     * @param username Display name of the sender
     * @param message The chat message
     * @param color Optional color (default purple)
     * @param badges Optional badges string (e.g., "moderator/1,subscriber/12")
     */
    Q_INVOKABLE void sendChatMessage(const QString& username,
                                     const QString& message,
                                     const QString& color = "#9147FF",
                                     const QString& badges = "");

    /**
     * @brief Simulate multiple chat messages with delay
     * @param messages List of {username, message} pairs
     * @param delayMs Delay between messages in ms
     */
    void sendChatMessagesWithDelay(const QList<QPair<QString, QString>>& messages,
                                   int delayMs = 500);

    /**
     * @brief Get list of messages sent by the client (outgoing PRIVMSG)
     */
    Q_INVOKABLE [[nodiscard]] QStringList clientMessages() const;

    /**
     * @brief Clear the list of client messages
     */
    Q_INVOKABLE void clearClientMessages();

    /**
     * @brief Check if a client is connected and joined a channel
     */
    Q_INVOKABLE [[nodiscard]] bool hasConnectedClient() const;

    /**
     * @brief Get the channel the client joined
     */
    Q_INVOKABLE [[nodiscard]] QString joinedChannel() const;
    
    /**
     * @brief Get number of messages received from client
     */
    Q_INVOKABLE [[nodiscard]] int clientMessageCount() const { return m_clientMessages.count(); }

    /**
     * @brief Get total number of requests handled
     */
    [[nodiscard]] int requestCount() const { return m_requestCount; }
    
    /**
     * @brief Enable/disable echoing client messages back (like real Twitch IRC)
     * @param enable Whether to echo messages
     * @param username Username to show for echoed messages
     * @param color Color for echoed messages
     */
    Q_INVOKABLE void setEchoMessages(bool enable, 
                                     const QString& username = "TestSender",
                                     const QString& color = "#9147FF") {
        m_echoMessages = enable;
        m_echoUsername = username;
        m_echoColor = color;
    }

signals:
    /**
     * @brief Emitted when a client connects
     */
    void clientConnected();

    /**
     * @brief Emitted when a client joins a channel
     */
    void clientJoined(const QString& channel);

    /**
     * @brief Emitted when a client sends a chat message
     */
    void clientMessageReceived(const QString& message);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onTextMessageReceived(const QString& message);

private:
    void handleIrcCommand(QWebSocket* client, const QString& line);
    void sendToClient(QWebSocket* client, const QString& message);
    void sendToAllClients(const QString& message);
    QString buildPrivmsg(const QString& username,
                         const QString& message,
                         const QString& color,
                         const QString& badges);

    std::unique_ptr<QWebSocketServer> m_server;
    QList<QWebSocket*> m_clients;
    QStringList m_clientMessages;
    QString m_joinedChannel;
    int m_requestCount{0};
    int m_messageId{1};

    // For delayed messages
    QList<QPair<QString, QString>> m_pendingMessages;
    QTimer m_messageTimer;
    int m_messageDelayMs{500};
    
    // For echoing client messages back
    bool m_echoMessages{false};
    QString m_echoUsername{"TestSender"};
    QString m_echoColor{"#9147FF"};
};

} // namespace E2E
