#include "MockHttpServer.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace blueplayer::test::e2e {

// ============================================================================
// HttpRequest helpers
// ============================================================================

QString MockHttpServer::HttpRequest::queryString() const
{
    int qPos = path.indexOf('?');
    return (qPos >= 0) ? path.mid(qPos + 1) : QString();
}

QString MockHttpServer::HttpRequest::cleanPath() const
{
    return path.split('?').first();
}

QString MockHttpServer::HttpRequest::queryParam(const QString& name, const QString& defaultValue) const
{
    QUrlQuery query(queryString());
    return query.hasQueryItem(name) ? query.queryItemValue(name) : defaultValue;
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

MockHttpServer::MockHttpServer(QObject* parent)
    : QObject(parent)
{
}

MockHttpServer::~MockHttpServer()
{
    stop();
}

// ============================================================================
// Lifecycle
// ============================================================================

bool MockHttpServer::start(quint16 port)
{
    if (m_server) {
        qWarning() << serverName() << ": Already running";
        return false;
    }

    m_server = new QTcpServer(this);

    connect(m_server, &QTcpServer::newConnection,
            this, &MockHttpServer::handleNewConnection);

    if (!m_server->listen(QHostAddress::LocalHost, port)) {
        qWarning() << serverName() << ": Failed to listen:" << m_server->errorString();
        delete m_server;
        m_server = nullptr;
        return false;
    }

    m_port = m_server->serverPort();
    qDebug() << serverName() << ": Listening on port" << m_port;
    return true;
}

void MockHttpServer::stop()
{
    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = nullptr;
        m_port = 0;
    }
}

bool MockHttpServer::isRunning() const
{
    return m_server && m_server->isListening();
}

quint16 MockHttpServer::port() const
{
    return m_port;
}

QString MockHttpServer::baseUrl() const
{
    return QString("http://localhost:%1").arg(m_port);
}

// ============================================================================
// Request Tracking
// ============================================================================

int MockHttpServer::requestCount() const
{
    return m_requestLog.size();
}

QStringList MockHttpServer::requestsTo(const QString& pathPattern) const
{
    QStringList result;
    for (const QString& log : m_requestLog) {
        if (log.contains(pathPattern)) {
            result.append(log);
        }
    }
    return result;
}

void MockHttpServer::clearRequests()
{
    m_requestLog.clear();
}

QStringList MockHttpServer::requestLog() const
{
    return m_requestLog;
}

// ============================================================================
// Response Helpers
// ============================================================================

QByteArray MockHttpServer::makeResponse(int statusCode, const QString& statusText,
                                         const QByteArray& body,
                                         const QString& contentType)
{
    QByteArray response;
    response.append(QString("HTTP/1.1 %1 %2\r\n").arg(statusCode).arg(statusText).toUtf8());
    response.append(QString("Content-Type: %1\r\n").arg(contentType).toUtf8());
    response.append(QString("Content-Length: %1\r\n").arg(body.size()).toUtf8());
    response.append("Connection: close\r\n");
    response.append("Access-Control-Allow-Origin: *\r\n");
    response.append("\r\n");
    response.append(body);
    return response;
}

QByteArray MockHttpServer::makeTwitchResponse(const QJsonArray& data)
{
    QJsonObject root;
    root["data"] = data;
    return makeResponse(200, "OK", QJsonDocument(root).toJson());
}

QByteArray MockHttpServer::makeTwitchError(int statusCode, const QString& message)
{
    QJsonObject root;
    root["error"] = message;
    root["status"] = statusCode;
    root["message"] = message;
    return makeResponse(statusCode, message, QJsonDocument(root).toJson());
}

QByteArray MockHttpServer::make404(const QString& message)
{
    return makeResponse(404, "Not Found", message.toUtf8(), "text/plain");
}

// ============================================================================
// Request Parsing
// ============================================================================

MockHttpServer::HttpRequest MockHttpServer::parseRequest(const QByteArray& data)
{
    HttpRequest request;
    QString str = QString::fromUtf8(data);
    QStringList lines = str.split("\r\n");

    if (lines.isEmpty()) {
        return request;
    }

    // Parse request line: GET /path HTTP/1.1
    QStringList requestLine = lines.first().split(' ');
    if (requestLine.size() >= 3) {
        request.method = requestLine[0];
        request.path = requestLine[1];
        request.version = requestLine[2];
    }

    // Parse headers
    int bodyStart = -1;
    for (int i = 1; i < lines.size(); ++i) {
        if (lines[i].isEmpty()) {
            bodyStart = i + 1;
            break;
        }
        int colonPos = lines[i].indexOf(':');
        if (colonPos > 0) {
            QString key = lines[i].left(colonPos).trimmed();
            QString value = lines[i].mid(colonPos + 1).trimmed();
            request.headers[key] = value;
        }
    }

    // Parse body
    if (bodyStart > 0 && bodyStart < lines.size()) {
        request.body = lines.mid(bodyStart).join("\r\n").toUtf8();
    }

    return request;
}

// ============================================================================
// Connection Handling
// ============================================================================

void MockHttpServer::handleNewConnection()
{
    while (m_server && m_server->hasPendingConnections()) {
        QTcpSocket* socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead,
                this, &MockHttpServer::handleClientData);
        connect(socket, &QTcpSocket::disconnected,
                this, &MockHttpServer::handleClientDisconnected);
    }
}

void MockHttpServer::handleClientData()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) {
        return;
    }

    QByteArray data = socket->readAll();
    HttpRequest request = parseRequest(data);

    // Log the request
    QString logEntry = QString("%1 %2").arg(request.method, request.path);
    m_requestLog.append(logEntry);
    Q_EMIT requestReceived(request.method, request.path);

    // Handle the request (implemented by derived classes)
    QByteArray response = handleRequest(request);
    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

void MockHttpServer::handleClientDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

} // namespace blueplayer::test::e2e
