#include "TwitchChatClient.hpp"
#include "ChatMessage.hpp"

#include <QtWebSockets/QWebSocket>
#include <QRandomGenerator>
#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>
#include <QUrl>
#include <QUuid>

namespace BluePlayer {

TwitchChatClient::TwitchChatClient(QObject* parent)
    : QObject(parent)
    , m_socket(std::make_unique<QWebSocket>())
{
    // Connect socket signals
    connect(m_socket.get(), &QWebSocket::connected,
            this, &TwitchChatClient::onConnected);
    connect(m_socket.get(), &QWebSocket::disconnected,
            this, &TwitchChatClient::onDisconnected);
    connect(m_socket.get(), &QWebSocket::textMessageReceived,
            this, &TwitchChatClient::onTextMessageReceived);
    connect(m_socket.get(), &QWebSocket::errorOccurred,
            this, &TwitchChatClient::onError);

    // Setup reconnect timer
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout,
            this, &TwitchChatClient::onReconnectTimer);
}

TwitchChatClient::~TwitchChatClient() {
    disconnect();
}

bool TwitchChatClient::isConnected() const {
    return m_state == ConnectionState::Connected;
}

QString TwitchChatClient::channel() const {
    return m_channel;
}

QString TwitchChatClient::connectionStatus() const {
    switch (m_state) {
        case ConnectionState::Disconnected:
            return QStringLiteral("disconnected");
        case ConnectionState::Connecting:
            return QStringLiteral("connecting");
        case ConnectionState::Connected:
            return QStringLiteral("connected");
        case ConnectionState::Error:
            return QStringLiteral("error");
    }
    return QStringLiteral("unknown");
}

bool TwitchChatClient::canSendMessages() const {
    return !m_oauthToken.isEmpty() && !m_username.isEmpty() && isConnected();
}

void TwitchChatClient::setCredentials(const QString& token, const QString& username) {
    m_oauthToken = token;
    m_username = username.toLower();
    emit canSendMessagesChanged();
}

void TwitchChatClient::sendMessage(const QString& message) {
    if (!canSendMessages()) {
        return;
    }
    
    if (message.isEmpty() || m_channel.isEmpty()) {
        return;
    }
    
    // Sanitize message to prevent IRC injection
    QString sanitized = message;
    sanitized.remove(QLatin1Char('\r'));
    sanitized.remove(QLatin1Char('\n'));
    sanitized = sanitized.trimmed();
    
    // Validate message length (Twitch limit is 500 characters)
    constexpr int kMaxMessageLength = 500;
    if (sanitized.isEmpty() || sanitized.length() > kMaxMessageLength) {
        return;
    }
    
    QString cmd = QStringLiteral("PRIVMSG #%1 :%2").arg(m_channel, sanitized);
    sendRaw(cmd);
    qInfo() << "[Chat] Message sent to channel:" << m_channel;
    
    // Emit local echo immediately (Twitch doesn't send back our own messages)
    ChatMessage localMsg;
    localMsg.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    localMsg.username = m_username;
    localMsg.displayName = m_username;
    localMsg.message = sanitized;
    localMsg.color = QStringLiteral("#8A2BE2"); // Default color
    localMsg.timestamp = QDateTime::currentMSecsSinceEpoch();
    emit messageReceived(localMsg.toVariantMap());
}

void TwitchChatClient::connectToChannel(const QString& channelName) {
    if (channelName.isEmpty()) {
        return;
    }

    // Disconnect from current channel if any
    if (m_state != ConnectionState::Disconnected) {
        disconnect();
    }

    m_channel = channelName.toLower();
    m_reconnectAttempts = 0;
    setConnectionState(ConnectionState::Connecting);

    // Allow override via environment variable for E2E testing
    QString ircUrl = QString::fromUtf8(qgetenv("BLUEPLAYER_IRC_URL"));
    if (ircUrl.isEmpty()) {
        ircUrl = QString::fromLatin1(TWITCH_IRC_URL);
    } else {
        qInfo() << "[Chat] Using IRC proxy:" << ircUrl;
    }
    m_socket->open(QUrl(ircUrl));
}

void TwitchChatClient::disconnect() {
    m_reconnectTimer.stop();
    m_reconnectAttempts = 0;

    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        // Leave channel first
        if (!m_channel.isEmpty() && m_state == ConnectionState::Connected) {
            sendRaw(QStringLiteral("PART #%1").arg(m_channel));
        }
        m_socket->close();
    }

    m_channel.clear();
    setConnectionState(ConnectionState::Disconnected);
}

void TwitchChatClient::onConnected() {
    qInfo() << "[Chat] Connected to channel:" << m_channel;

    // Request capabilities for tags (badges, emotes, colors)
    sendRaw(QStringLiteral("CAP REQ :twitch.tv/tags twitch.tv/commands"));

    // Use OAuth if credentials are available, otherwise anonymous (read-only)
    if (!m_oauthToken.isEmpty() && !m_username.isEmpty()) {
        sendRaw(QStringLiteral("PASS oauth:%1").arg(m_oauthToken));
        sendRaw(QStringLiteral("NICK %1").arg(m_username));
    } else {
        QString anonUser = generateAnonUsername();
        sendRaw(QStringLiteral("PASS oauth:anonymous"));
        sendRaw(QStringLiteral("NICK %1").arg(anonUser));
    }

    // Join channel
    sendRaw(QStringLiteral("JOIN #%1").arg(m_channel));

    setConnectionState(ConnectionState::Connected);
    m_reconnectAttempts = 0;
}

void TwitchChatClient::onDisconnected() {
    qInfo() << "[Chat] Disconnected from channel:" << m_channel;

    if (m_state == ConnectionState::Connected && !m_channel.isEmpty()) {
        // Unexpected disconnect - try to reconnect
        if (m_reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            setConnectionState(ConnectionState::Connecting);
            m_reconnectTimer.start(RECONNECT_DELAY_MS);
        } else {
            setConnectionState(ConnectionState::Error);
            emit errorOccurred(tr("Connection lost after %1 attempts").arg(MAX_RECONNECT_ATTEMPTS));
        }
    } else {
        setConnectionState(ConnectionState::Disconnected);
    }
}

void TwitchChatClient::onTextMessageReceived(const QString& message) {
    // IRC messages can contain multiple lines
    const QStringList lines = message.split(QStringLiteral("\r\n"), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (line.isEmpty()) continue;

        // Handle PING/PONG to keep connection alive
        if (line.startsWith(QStringLiteral("PING"))) {
            sendRaw(QStringLiteral("PONG :tmi.twitch.tv"));
            continue;
        }

        parseIrcMessage(line);
    }
}

void TwitchChatClient::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error)
    qWarning() << "[Chat] Connection error:" << m_socket->errorString();
    setConnectionState(ConnectionState::Error);
    emit errorOccurred(m_socket->errorString());
}

void TwitchChatClient::onReconnectTimer() {
    if (m_channel.isEmpty()) return;

    m_reconnectAttempts++;
    qDebug() << "[TwitchChatClient] Reconnect attempt" << m_reconnectAttempts;

    m_socket->open(QUrl(QString::fromLatin1(TWITCH_IRC_URL)));
}

void TwitchChatClient::parseIrcMessage(const QString& rawMessage) {
    // RAW message logging removed - was generating 59% of log volume

    // Twitch IRC format with tags:
    // @tags :user!user@user.tmi.twitch.tv COMMAND #channel :message

    QString line = rawMessage;
    QVariantMap tags;

    // Parse tags if present (starts with @)
    if (line.startsWith(QLatin1Char('@'))) {
        qsizetype spaceIdx = line.indexOf(QLatin1Char(' '));
        if (spaceIdx > 0) {
            QString tagsStr = line.mid(1, spaceIdx - 1);
            tags = parseTags(tagsStr);
            line = line.mid(spaceIdx + 1);
        }
    }

    // Skip prefix if present (starts with :)
    QString prefix;
    if (line.startsWith(QLatin1Char(':'))) {
        qsizetype spaceIdx = line.indexOf(QLatin1Char(' '));
        if (spaceIdx > 0) {
            prefix = line.mid(1, spaceIdx - 1);
            line = line.mid(spaceIdx + 1);
        }
    }

    // Parse command and params
    qsizetype colonIdx = line.indexOf(QStringLiteral(" :"));
    QString commandPart = colonIdx > 0 ? line.left(colonIdx) : line;
    QString trailing = colonIdx > 0 ? line.mid(colonIdx + 2) : QString();

    QStringList parts = commandPart.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;

    QString command = parts.takeFirst();

    // Handle NOTICE (errors and info from Twitch)
    if (command == QStringLiteral("NOTICE")) {
        qWarning() << "[Chat] NOTICE from Twitch:" << trailing;
        // Check for auth failure
        if (trailing.contains(QStringLiteral("Login authentication failed")) ||
            trailing.contains(QStringLiteral("Improperly formatted auth"))) {
            setConnectionState(ConnectionState::Error);
            emit errorOccurred(trailing);
        }
        return;
    }

    // Handle PRIVMSG (chat message)
    if (command == QStringLiteral("PRIVMSG") && !parts.isEmpty()) {
        QString targetChannel = parts.first();
        if (targetChannel.startsWith(QLatin1Char('#'))) {
            targetChannel = targetChannel.mid(1);
        }

        // Build message object
        QVariantMap msg;
        msg[QStringLiteral("id")] = tags.value(QStringLiteral("id"), QUuid::createUuid().toString());
        msg[QStringLiteral("username")] = tags.value(QStringLiteral("display-name"), prefix.section(QLatin1Char('!'), 0, 0));
        msg[QStringLiteral("displayName")] = tags.value(QStringLiteral("display-name"), prefix.section(QLatin1Char('!'), 0, 0));
        msg[QStringLiteral("message")] = trailing;
        msg[QStringLiteral("color")] = tags.value(QStringLiteral("color"), QStringLiteral("#AAAAAA"));
        msg[QStringLiteral("channel")] = targetChannel;
        msg[QStringLiteral("timestamp")] = QDateTime::currentMSecsSinceEpoch();

        // Parse badges
        QString badgesTag = tags.value(QStringLiteral("badges")).toString();
        msg[QStringLiteral("badges")] = parseBadges(badgesTag);

        // Parse emotes and split message into parts
        QString emotesTag = tags.value(QStringLiteral("emotes")).toString();
        msg[QStringLiteral("emoteParts")] = parseEmoteParts(emotesTag, trailing);

        emit messageReceived(msg);
    }
}

QVariantMap TwitchChatClient::parseTags(const QString& tagsString) {
    QVariantMap result;
    const QStringList tagPairs = tagsString.split(QLatin1Char(';'), Qt::SkipEmptyParts);

    for (const QString& pair : tagPairs) {
        qsizetype eqIdx = pair.indexOf(QLatin1Char('='));
        if (eqIdx > 0) {
            QString key = pair.left(eqIdx);
            QString value = pair.mid(eqIdx + 1);
            // Unescape IRC tag values
            value.replace(QStringLiteral("\\s"), QStringLiteral(" "));
            value.replace(QStringLiteral("\\n"), QStringLiteral("\n"));
            value.replace(QStringLiteral("\\r"), QStringLiteral("\r"));
            value.replace(QStringLiteral("\\:"), QStringLiteral(";"));
            value.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
            result[key] = value;
        }
    }

    return result;
}

QVariantList TwitchChatClient::parseEmoteParts(const QString& emotesTag, const QString& message) {
    QVariantList parts;

    if (emotesTag.isEmpty()) {
        // No emotes - just text
        QVariantMap textPart;
        textPart[QStringLiteral("type")] = QStringLiteral("text");
        textPart[QStringLiteral("content")] = message;
        parts.append(textPart);
        return parts;
    }

    // Parse emote positions
    // Format: emoteId:start-end,start-end/emoteId:start-end
    struct EmotePos {
        QString emoteId;
        qsizetype start;
        qsizetype end;
    };
    QList<EmotePos> emotePositions;

    const QStringList emoteGroups = emotesTag.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (const QString& group : emoteGroups) {
        qsizetype colonIdx = group.indexOf(QLatin1Char(':'));
        if (colonIdx <= 0) continue;

        QString emoteId = group.left(colonIdx);
        QString positions = group.mid(colonIdx + 1);

        const QStringList ranges = positions.split(QLatin1Char(','), Qt::SkipEmptyParts);
        for (const QString& range : ranges) {
            qsizetype dashIdx = range.indexOf(QLatin1Char('-'));
            if (dashIdx <= 0) continue;

            bool okStart = false, okEnd = false;
            qsizetype start = range.left(dashIdx).toLongLong(&okStart);
            qsizetype end = range.mid(dashIdx + 1).toLongLong(&okEnd);

            if (okStart && okEnd && start >= 0 && end >= start) {
                emotePositions.append({emoteId, start, end});
            }
        }
    }

    // Sort by start position
    std::sort(emotePositions.begin(), emotePositions.end(),
              [](const EmotePos& a, const EmotePos& b) { return a.start < b.start; });

    // Build parts list
    qsizetype currentPos = 0;
    for (const EmotePos& ep : emotePositions) {
        // Add text before this emote
        if (ep.start > currentPos) {
            QVariantMap textPart;
            textPart[QStringLiteral("type")] = QStringLiteral("text");
            textPart[QStringLiteral("content")] = message.mid(currentPos, ep.start - currentPos);
            parts.append(textPart);
        }

        // Add emote
        QVariantMap emotePart;
        emotePart[QStringLiteral("type")] = QStringLiteral("emote");
        emotePart[QStringLiteral("emoteId")] = ep.emoteId;
        emotePart[QStringLiteral("content")] = message.mid(ep.start, ep.end - ep.start + 1);
        parts.append(emotePart);

        currentPos = ep.end + 1;
    }

    // Add remaining text after last emote
    if (currentPos < message.length()) {
        QVariantMap textPart;
        textPart[QStringLiteral("type")] = QStringLiteral("text");
        textPart[QStringLiteral("content")] = message.mid(currentPos);
        parts.append(textPart);
    }

    return parts;
}

QVariantList TwitchChatClient::parseBadges(const QString& badgesTag) {
    QVariantList badges;

    if (badgesTag.isEmpty()) {
        return badges;
    }

    // Format: badge/version,badge/version
    const QStringList badgePairs = badgesTag.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString& pair : badgePairs) {
        qsizetype slashIdx = pair.indexOf(QLatin1Char('/'));
        if (slashIdx > 0) {
            QVariantMap badge;
            badge[QStringLiteral("type")] = pair.left(slashIdx);
            badge[QStringLiteral("version")] = pair.mid(slashIdx + 1);
            badges.append(badge);
        }
    }

    return badges;
}

void TwitchChatClient::sendRaw(const QString& command) {
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->sendTextMessage(command + QStringLiteral("\r\n"));
    }
}

QString TwitchChatClient::generateAnonUsername() {
    // justinfan followed by random number (1-80000)
    int randomNum = QRandomGenerator::global()->bounded(1, 80001);
    return QStringLiteral("justinfan%1").arg(randomNum);
}

void TwitchChatClient::setConnectionState(ConnectionState state) {
    if (m_state == state) return;

    m_state = state;
    emit connectedChanged();
    emit connectionStatusChanged();
}

} // namespace BluePlayer
