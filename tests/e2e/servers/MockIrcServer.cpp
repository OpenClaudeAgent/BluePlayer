#include "MockIrcServer.hpp"

#include <QDebug>
#include <QUuid>
#include <QDateTime>

namespace blueplayer::test::e2e {

MockIrcServer::MockIrcServer(QObject* parent)
    : QObject(parent)
{
    m_messageTimer.setSingleShot(true);
    connect(&m_messageTimer, &QTimer::timeout, this, [this]() {
        if (!m_pendingMessages.isEmpty()) {
            auto msg = m_pendingMessages.takeFirst();
            sendChatMessage(msg.first, msg.second);
            
            if (!m_pendingMessages.isEmpty()) {
                m_messageTimer.start(m_messageDelayMs);
            }
        }
    });
}

MockIrcServer::~MockIrcServer() {
    stop();
}

bool MockIrcServer::start() {
    m_server = std::make_unique<QWebSocketServer>(
        QStringLiteral("MockIrcServer"),
        QWebSocketServer::NonSecureMode,
        this
    );

    // Listen on any available port
    if (!m_server->listen(QHostAddress::LocalHost, 0)) {
        qWarning() << "MockIrcServer: Failed to start:" << m_server->errorString();
        return false;
    }

    connect(m_server.get(), &QWebSocketServer::newConnection,
            this, &MockIrcServer::onNewConnection);

    qDebug() << "MockIrcServer" << ": Listening on port" << m_server->serverPort();
    return true;
}

void MockIrcServer::stop() {
    m_messageTimer.stop();
    m_pendingMessages.clear();
    
    for (QWebSocket* client : m_clients) {
        client->close();
        client->deleteLater();
    }
    m_clients.clear();
    
    if (m_server) {
        m_server->close();
        m_server.reset();
    }
    
    m_joinedChannel.clear();
    m_clientMessages.clear();
}

quint16 MockIrcServer::port() const {
    return m_server ? m_server->serverPort() : 0;
}

QString MockIrcServer::url() const {
    return QStringLiteral("ws://localhost:%1").arg(port());
}

void MockIrcServer::sendChatMessage(const QString& username,
                                    const QString& message,
                                    const QString& color,
                                    const QString& badges) {
    if (m_clients.isEmpty() || m_joinedChannel.isEmpty()) {
        qWarning() << "MockIrcServer: Cannot send message - no client or channel";
        return;
    }
    
    QString privmsg = buildPrivmsg(username, message, color, badges);
    sendToAllClients(privmsg);
}

void MockIrcServer::sendChatMessagesWithDelay(
    const QList<QPair<QString, QString>>& messages,
    int delayMs)
{
    m_pendingMessages = messages;
    m_messageDelayMs = delayMs;
    
    if (!m_pendingMessages.isEmpty()) {
        // Send first message immediately
        auto msg = m_pendingMessages.takeFirst();
        sendChatMessage(msg.first, msg.second);
        
        // Schedule remaining messages
        if (!m_pendingMessages.isEmpty()) {
            m_messageTimer.start(delayMs);
        }
    }
}

QStringList MockIrcServer::clientMessages() const {
    return m_clientMessages;
}

void MockIrcServer::clearClientMessages() {
    m_clientMessages.clear();
}

bool MockIrcServer::hasConnectedClient() const {
    return !m_clients.isEmpty() && !m_joinedChannel.isEmpty();
}

QString MockIrcServer::joinedChannel() const {
    return m_joinedChannel;
}

void MockIrcServer::onNewConnection() {
    QWebSocket* client = m_server->nextPendingConnection();
    if (!client) return;
    
    m_clients.append(client);
    
    connect(client, &QWebSocket::textMessageReceived,
            this, &MockIrcServer::onTextMessageReceived);
    connect(client, &QWebSocket::disconnected,
            this, &MockIrcServer::onClientDisconnected);
    
    qDebug() << "MockIrcServer: Client connected";
    emit clientConnected();
}

void MockIrcServer::onClientDisconnected() {
    QWebSocket* client = qobject_cast<QWebSocket*>(sender());
    if (client) {
        m_clients.removeOne(client);
        client->deleteLater();
        qDebug() << "MockIrcServer: Client disconnected";
    }
}

void MockIrcServer::onTextMessageReceived(const QString& message) {
    QWebSocket* client = qobject_cast<QWebSocket*>(sender());
    if (!client) return;
    
    m_requestCount++;
    
    // IRC messages can contain multiple lines
    const QStringList lines = message.split(QStringLiteral("\r\n"), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (!line.isEmpty()) {
            handleIrcCommand(client, line);
        }
    }
}

void MockIrcServer::handleIrcCommand(QWebSocket* client, const QString& line) {
    qDebug() << "MockIrcServer: Received:" << line;
    
    // Handle CAP REQ
    if (line.startsWith(QStringLiteral("CAP REQ"))) {
        sendToClient(client, QStringLiteral(":tmi.twitch.tv CAP * ACK :twitch.tv/tags twitch.tv/commands"));
        return;
    }
    
    // Handle PASS (ignore, accept any)
    if (line.startsWith(QStringLiteral("PASS"))) {
        return;
    }
    
    // Handle NICK
    if (line.startsWith(QStringLiteral("NICK"))) {
        // Send welcome messages
        QString nick = line.mid(5).trimmed();
        sendToClient(client, QStringLiteral(":tmi.twitch.tv 001 %1 :Welcome, GLHF!").arg(nick));
        sendToClient(client, QStringLiteral(":tmi.twitch.tv 002 %1 :Your host is tmi.twitch.tv").arg(nick));
        sendToClient(client, QStringLiteral(":tmi.twitch.tv 003 %1 :This server is rather new").arg(nick));
        sendToClient(client, QStringLiteral(":tmi.twitch.tv 004 %1 :-").arg(nick));
        sendToClient(client, QStringLiteral(":tmi.twitch.tv 375 %1 :-").arg(nick));
        sendToClient(client, QStringLiteral(":tmi.twitch.tv 372 %1 :You are in a maze of twisty passages, all alike.").arg(nick));
        sendToClient(client, QStringLiteral(":tmi.twitch.tv 376 %1 :>").arg(nick));
        return;
    }
    
    // Handle JOIN
    if (line.startsWith(QStringLiteral("JOIN"))) {
        QString channel = line.mid(5).trimmed();
        if (channel.startsWith(QLatin1Char('#'))) {
            channel = channel.mid(1);
        }
        m_joinedChannel = channel;
        
        // Send JOIN confirmation
        sendToClient(client, QStringLiteral(":testuser!testuser@testuser.tmi.twitch.tv JOIN #%1").arg(channel));
        sendToClient(client, QStringLiteral(":testuser.tmi.twitch.tv 353 testuser = #%1 :testuser").arg(channel));
        sendToClient(client, QStringLiteral(":testuser.tmi.twitch.tv 366 testuser #%1 :End of /NAMES list").arg(channel));
        
        qDebug() << "MockIrcServer: Client joined channel:" << channel;
        emit clientJoined(channel);
        return;
    }
    
    // Handle PART
    if (line.startsWith(QStringLiteral("PART"))) {
        m_joinedChannel.clear();
        return;
    }
    
    // Handle PRIVMSG (client sending a message)
    if (line.startsWith(QStringLiteral("PRIVMSG"))) {
        // Format: PRIVMSG #channel :message
        int colonIdx = line.indexOf(QStringLiteral(" :"));
        if (colonIdx > 0) {
            QString messageContent = line.mid(colonIdx + 2);
            m_clientMessages.append(messageContent);
            qDebug() << "MockIrcServer: Client sent message:" << messageContent;
            emit clientMessageReceived(messageContent);
            
            // Echo the message back to the client (like real Twitch IRC)
            // This makes the message appear in the sender's chat UI
            if (m_echoMessages) {
                sendChatMessage(m_echoUsername, messageContent, m_echoColor);
            }
        }
        return;
    }
    
    // Handle PONG
    if (line.startsWith(QStringLiteral("PONG"))) {
        return;
    }
}

void MockIrcServer::sendToClient(QWebSocket* client, const QString& message) {
    if (client && client->isValid()) {
        client->sendTextMessage(message + QStringLiteral("\r\n"));
    }
}

void MockIrcServer::sendToAllClients(const QString& message) {
    for (QWebSocket* client : m_clients) {
        sendToClient(client, message);
    }
}

QString MockIrcServer::buildPrivmsg(const QString& username,
                                    const QString& message,
                                    const QString& color,
                                    const QString& badges) {
    // Build a Twitch-style PRIVMSG with tags
    // Format: @tags :user!user@user.tmi.twitch.tv PRIVMSG #channel :message
    
    QString msgId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    
    QStringList tags;
    tags << QStringLiteral("id=%1").arg(msgId);
    tags << QStringLiteral("display-name=%1").arg(username);
    tags << QStringLiteral("color=%1").arg(color);
    tags << QStringLiteral("tmi-sent-ts=%1").arg(timestamp);
    
    if (!badges.isEmpty()) {
        tags << QStringLiteral("badges=%1").arg(badges);
    }
    
    QString userLower = username.toLower();
    QString result = QStringLiteral("@%1 :%2!%2@%2.tmi.twitch.tv PRIVMSG #%3 :%4")
        .arg(tags.join(QLatin1Char(';')))
        .arg(userLower)
        .arg(m_joinedChannel)
        .arg(message);
    
    m_messageId++;
    return result;
}

} // namespace blueplayer::test::e2e
