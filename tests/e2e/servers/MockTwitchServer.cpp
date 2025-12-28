#include "MockTwitchServer.hpp"

#include <QDebug>
#include <QRegularExpression>

namespace blueplayer::test::e2e {

MockTwitchServer::MockTwitchServer(QObject* parent)
    : QObject(parent)
{
}

MockTwitchServer::~MockTwitchServer()
{
  stop();
}

bool MockTwitchServer::start(quint16 port)
{
  if (m_server) {
    qWarning() << "MockTwitchServer: Already running";
    return false;
  }

  m_server = new QTcpServer(this);

  connect(m_server, &QTcpServer::newConnection,
          this, &MockTwitchServer::handleNewConnection);

  if (!m_server->listen(QHostAddress::LocalHost, port)) {
    qWarning() << "MockTwitchServer: Failed to listen:" << m_server->errorString();
    delete m_server;
    m_server = nullptr;
    return false;
  }

  m_port = m_server->serverPort();
  qDebug() << "MockTwitchServer: Listening on port" << m_port;
  return true;
}

void MockTwitchServer::stop()
{
  if (m_server) {
    m_server->close();
    delete m_server;
    m_server = nullptr;
    m_port = 0;
  }
}

QString MockTwitchServer::baseUrl() const
{
  return QString("http://localhost:%1").arg(m_port);
}

quint16 MockTwitchServer::port() const
{
  return m_port;
}

bool MockTwitchServer::isRunning() const
{
  return m_server && m_server->isListening();
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

int MockTwitchServer::requestCount() const
{
  return m_requestLog.size();
}

QStringList MockTwitchServer::requestsTo(const QString& path) const
{
  QStringList result;
  for (const QString& log : m_requestLog) {
    if (log.contains(path)) {
      result.append(log);
    }
  }
  return result;
}

void MockTwitchServer::clearRequests()
{
  m_requestLog.clear();
}

void MockTwitchServer::simulateError(const QString& path, int statusCode, const QString& message)
{
  m_simulatedErrors[path] = qMakePair(statusCode, message);
}

void MockTwitchServer::clearErrors()
{
  m_simulatedErrors.clear();
}

void MockTwitchServer::handleNewConnection()
{
  while (m_server && m_server->hasPendingConnections()) {
    QTcpSocket* socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead,
            this, &MockTwitchServer::handleClientData);
    connect(socket, &QTcpSocket::disconnected,
            this, &MockTwitchServer::handleClientDisconnected);
  }
}

void MockTwitchServer::handleClientData()
{
  auto* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket) return;

  QByteArray data = socket->readAll();
  HttpRequest request = parseRequest(data);

  // Log the request
  QString logEntry = QString("%1 %2").arg(request.method, request.path);
  m_requestLog.append(logEntry);
  Q_EMIT requestReceived(request.method, request.path);

  // Handle the request
  QByteArray response = handleRequest(request);
  socket->write(response);
  socket->flush();
  socket->disconnectFromHost();
}

void MockTwitchServer::handleClientDisconnected()
{
  auto* socket = qobject_cast<QTcpSocket*>(sender());
  if (socket) {
    socket->deleteLater();
  }
}

MockTwitchServer::HttpRequest MockTwitchServer::parseRequest(const QByteArray& data)
{
  HttpRequest request;
  QString str = QString::fromUtf8(data);
  QStringList lines = str.split("\r\n");

  if (lines.isEmpty()) return request;

  // Parse request line
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

QByteArray MockTwitchServer::handleRequest(const HttpRequest& request)
{
  QString path = request.path.split('?').first(); // Remove query string

  // Check for simulated errors
  if (m_simulatedErrors.contains(path)) {
    auto error = m_simulatedErrors.take(path);
    return makeTwitchError(error.first, error.second);
  }

  // OAuth endpoints
  if (path == "/oauth2/validate") {
    // Check Authorization header
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

  if (path == "/oauth2/token") {
    QJsonObject response;
    response["access_token"] = m_validToken;
    response["refresh_token"] = "refresh_" + m_validToken;
    response["expires_in"] = 14400;
    response["scope"] = QJsonArray({"user:read:email", "chat:read", "chat:edit"});
    response["token_type"] = "bearer";
    return makeResponse(200, "OK", QJsonDocument(response).toJson());
  }

  // Helix endpoints
  if (path == "/helix/streams") {
    return makeTwitchResponse(m_streams);
  }

  if (path == "/helix/users") {
    return makeTwitchResponse(m_users);
  }

  if (path == "/helix/channels") {
    return makeTwitchResponse(m_channels);
  }

  // HLS playlist endpoint (Usher-style)
  static QRegularExpression hlsPattern("/api/channel/hls/(\\w+)\\.m3u8");
  QRegularExpressionMatch match = hlsPattern.match(path);
  if (match.hasMatch()) {
    QString channel = match.captured(1);
    if (!m_hlsServerUrl.isEmpty()) {
      // Return a redirect or the HLS playlist
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
    return makeTwitchError(404, "Channel not found");
  }

  // Default: 404
  return makeTwitchError(404, "Not Found");
}

QByteArray MockTwitchServer::makeResponse(int statusCode, const QString& statusText,
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

QByteArray MockTwitchServer::makeTwitchResponse(const QJsonArray& data)
{
  QJsonObject root;
  root["data"] = data;
  return makeResponse(200, "OK", QJsonDocument(root).toJson());
}

QByteArray MockTwitchServer::makeTwitchError(int statusCode, const QString& message)
{
  QJsonObject root;
  root["error"] = message;
  root["status"] = statusCode;
  root["message"] = message;
  return makeResponse(statusCode, message, QJsonDocument(root).toJson());
}

} // namespace blueplayer::test::e2e
