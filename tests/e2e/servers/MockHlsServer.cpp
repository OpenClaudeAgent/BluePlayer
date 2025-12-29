#include "MockHlsServer.hpp"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QThread>

namespace blueplayer::test::e2e {

MockHlsServer::MockHlsServer(QObject* parent)
    : QObject(parent)
{
}

MockHlsServer::~MockHlsServer()
{
  stop();
}

bool MockHlsServer::start(quint16 port)
{
  if (m_server) {
    qWarning() << "MockHlsServer: Already running";
    return false;
  }

  m_server = new QTcpServer(this);

  connect(m_server, &QTcpServer::newConnection,
          this, &MockHlsServer::handleNewConnection);

  if (!m_server->listen(QHostAddress::LocalHost, port)) {
    qWarning() << "MockHlsServer: Failed to listen:" << m_server->errorString();
    delete m_server;
    m_server = nullptr;
    return false;
  }

  m_port = m_server->serverPort();
  qDebug() << "MockHlsServer: Listening on port" << m_port;
  return true;
}

void MockHlsServer::stop()
{
  if (m_server) {
    m_server->close();
    delete m_server;
    m_server = nullptr;
    m_port = 0;
  }
}

QString MockHlsServer::baseUrl() const
{
  return QString("http://localhost:%1").arg(m_port);
}

quint16 MockHlsServer::port() const
{
  return m_port;
}

bool MockHlsServer::isRunning() const
{
  return m_server && m_server->isListening();
}

void MockHlsServer::addChannel(const QString& channelName, int segmentCount, int segmentDuration)
{
  ChannelConfig config;
  config.segmentCount = segmentCount;
  config.segmentDuration = segmentDuration;
  config.mediaSequence = 0;
  m_channels[channelName] = config;
}

void MockHlsServer::removeChannel(const QString& channelName)
{
  m_channels.remove(channelName);
}

void MockHlsServer::clearChannels()
{
  m_channels.clear();
}

int MockHlsServer::requestCount() const
{
  return m_requestCount;
}

int MockHlsServer::segmentRequestCount() const
{
  return m_segmentRequestCount;
}

void MockHlsServer::clearRequests()
{
  m_requestCount = 0;
  m_segmentRequestCount = 0;
}

void MockHlsServer::setResponseDelay(int delayMs)
{
  m_responseDelayMs = delayMs;
}

void MockHlsServer::simulateStall(bool stall)
{
  m_stalled = stall;
}

bool MockHlsServer::loadSegmentFromFile(const QString& filePath)
{
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "MockHlsServer: Failed to load segment from" << filePath;
    return false;
  }
  
  m_segmentData = file.readAll();
  file.close();
  
  qDebug() << "MockHlsServer: Loaded segment" << m_segmentData.size() << "bytes from" << filePath;
  return true;
}

void MockHlsServer::handleNewConnection()
{
  while (m_server && m_server->hasPendingConnections()) {
    QTcpSocket* socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead,
            this, &MockHlsServer::handleClientData);
    connect(socket, &QTcpSocket::disconnected,
            this, &MockHlsServer::handleClientDisconnected);
  }
}

void MockHlsServer::handleClientData()
{
  auto* socket = qobject_cast<QTcpSocket*>(sender());
  if (!socket) return;

  QByteArray data = socket->readAll();
  QString request = QString::fromUtf8(data);

  // Parse the request line
  QStringList lines = request.split("\r\n");
  if (lines.isEmpty()) {
    socket->disconnectFromHost();
    return;
  }

  QStringList requestLine = lines.first().split(' ');
  if (requestLine.size() < 2) {
    socket->disconnectFromHost();
    return;
  }

  QString path = requestLine[1];
  m_requestCount++;

  // Apply delay if configured
  if (m_responseDelayMs > 0) {
    QThread::msleep(m_responseDelayMs);
  }

  QByteArray response = handleRequest(path);
  socket->write(response);
  socket->flush();
  socket->disconnectFromHost();
}

void MockHlsServer::handleClientDisconnected()
{
  auto* socket = qobject_cast<QTcpSocket*>(sender());
  if (socket) {
    socket->deleteLater();
  }
}

QByteArray MockHlsServer::handleRequest(const QString& path)
{
  // Extract path without query string
  QString cleanPath = path.split('?').first();

  // Live stream endpoint: /live/{channel} (redirects to master playlist)
  static QRegularExpression livePattern("/live/(\\w+)");
  QRegularExpressionMatch liveMatch = livePattern.match(cleanPath);
  if (liveMatch.hasMatch()) {
    QString channel = liveMatch.captured(1);
    if (m_channels.contains(channel)) {
      Q_EMIT playlistRequested(channel, "master");
      return makeMasterPlaylist(channel);
    }
    return makeResponse(404, "Not Found", "Channel not found", "text/plain");
  }

  // Master playlist: /playlist/{channel}_master.m3u8
  static QRegularExpression masterPattern("/playlist/(\\w+)_master\\.m3u8");
  QRegularExpressionMatch masterMatch = masterPattern.match(cleanPath);
  if (masterMatch.hasMatch()) {
    QString channel = masterMatch.captured(1);
    if (m_channels.contains(channel)) {
      Q_EMIT playlistRequested(channel, "master");
      return makeMasterPlaylist(channel);
    }
    return makeResponse(404, "Not Found", "Channel not found", "text/plain");
  }

  // Media playlist: /playlist/{channel}_{quality}.m3u8
  static QRegularExpression mediaPattern("/playlist/(\\w+)_(\\w+)\\.m3u8");
  QRegularExpressionMatch mediaMatch = mediaPattern.match(cleanPath);
  if (mediaMatch.hasMatch()) {
    QString channel = mediaMatch.captured(1);
    QString quality = mediaMatch.captured(2);
    if (m_channels.contains(channel)) {
      Q_EMIT playlistRequested(channel, quality);
      return makeMediaPlaylist(channel, quality);
    }
    return makeResponse(404, "Not Found", "Channel not found", "text/plain");
  }

  // Segment: /segments/{channel}_{quality}_{number}.ts
  static QRegularExpression segmentPattern("/segments/(\\w+)_(\\w+)_(\\d+)\\.ts");
  QRegularExpressionMatch segmentMatch = segmentPattern.match(cleanPath);
  if (segmentMatch.hasMatch()) {
    QString channel = segmentMatch.captured(1);
    int segmentNumber = segmentMatch.captured(3).toInt();

    if (m_stalled) {
      // Simulate stall by not responding (let connection timeout)
      return QByteArray();
    }

    m_segmentRequestCount++;
    Q_EMIT segmentRequested(channel, segmentNumber);
    return makeResponse(200, "OK", makeMinimalSegment(), "video/mp2t");
  }

  return makeResponse(404, "Not Found", "Not found", "text/plain");
}

QByteArray MockHlsServer::makeMasterPlaylist(const QString& channel)
{
  QString playlist = QString(
    "#EXTM3U\n"
    "#EXT-X-VERSION:3\n"
    "#EXT-X-STREAM-INF:BANDWIDTH=6000000,RESOLUTION=1920x1080,CODECS=\"avc1.4d401f,mp4a.40.2\"\n"
    "%1/playlist/%2_chunked.m3u8\n"
    "#EXT-X-STREAM-INF:BANDWIDTH=3000000,RESOLUTION=1280x720,CODECS=\"avc1.4d401f,mp4a.40.2\"\n"
    "%1/playlist/%2_720p.m3u8\n"
    "#EXT-X-STREAM-INF:BANDWIDTH=1500000,RESOLUTION=854x480,CODECS=\"avc1.4d401e,mp4a.40.2\"\n"
    "%1/playlist/%2_480p.m3u8\n"
  ).arg(baseUrl(), channel);

  return makeResponse(200, "OK", playlist.toUtf8(), "application/vnd.apple.mpegurl");
}

QByteArray MockHlsServer::makeMediaPlaylist(const QString& channel, const QString& quality)
{
  if (!m_channels.contains(channel)) {
    return makeResponse(404, "Not Found", "Channel not found", "text/plain");
  }

  ChannelConfig& config = m_channels[channel];
  int duration = config.segmentDuration;
  int segmentCount = config.segmentCount;

  QString playlist = QString(
    "#EXTM3U\n"
    "#EXT-X-VERSION:6\n"
    "#EXT-X-TARGETDURATION:%1\n"
    "#EXT-X-MEDIA-SEQUENCE:%2\n"
    "#EXT-X-PLAYLIST-TYPE:EVENT\n"
  ).arg(duration).arg(config.mediaSequence);

  // Add segments
  for (int i = 0; i < segmentCount; ++i) {
    int segNum = config.mediaSequence + i;
    playlist += QString("#EXTINF:%1.000,\n").arg(duration);
    playlist += QString("%1/segments/%2_%3_%4.ts\n")
                    .arg(baseUrl(), channel, quality)
                    .arg(segNum);
  }

  // Increment media sequence for live-like behavior
  config.mediaSequence++;

  return makeResponse(200, "OK", playlist.toUtf8(), "application/vnd.apple.mpegurl");
}

QByteArray MockHlsServer::makeMinimalSegment()
{
  // If we have loaded a real segment, use it
  if (!m_segmentData.isEmpty()) {
    return m_segmentData;
  }

  // Fallback: Create a minimal valid MPEG-TS segment
  // This is a minimal PAT + PMT + PES with empty audio/video
  // Just enough to not cause parsing errors

  static const unsigned char minimalTs[] = {
    // Sync byte + minimal TS packet (188 bytes total)
    0x47, 0x40, 0x00, 0x10, 0x00, // PAT
    0x00, 0xB0, 0x0D, 0x00, 0x01, 0xC1, 0x00, 0x00,
    0x00, 0x01, 0xF0, 0x00, 0x2A, 0xB1, 0x04, 0xB2,
    // Padding to 188 bytes
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF
  };

  return QByteArray(reinterpret_cast<const char*>(minimalTs), sizeof(minimalTs));
}

QByteArray MockHlsServer::makeResponse(int statusCode, const QString& statusText,
                                        const QByteArray& body, const QString& contentType)
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

} // namespace blueplayer::test::e2e
