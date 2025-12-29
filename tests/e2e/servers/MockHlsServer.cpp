#include "MockHlsServer.hpp"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QThread>

namespace blueplayer::test::e2e {

MockHlsServer::MockHlsServer(QObject* parent)
    : MockHttpServer(parent)
{
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

int MockHlsServer::segmentRequestCount() const
{
    return m_segmentRequestCount;
}

void MockHlsServer::setResponseDelay(int delayMs)
{
    m_responseDelayMs = delayMs;
}

void MockHlsServer::simulateStall(bool stall)
{
    m_stalled = stall;
}

QByteArray MockHlsServer::handleRequest(const HttpRequest& request)
{
    QString path = request.cleanPath();

    // Apply delay if configured
    if (m_responseDelayMs > 0) {
        QThread::msleep(m_responseDelayMs);
    }

    // Live stream endpoint: /live/{channel}
    static QRegularExpression livePattern("/live/(\\w+)");
    QRegularExpressionMatch liveMatch = livePattern.match(path);
    if (liveMatch.hasMatch()) {
        return handleLiveChannel(liveMatch.captured(1));
    }

    // Master playlist: /playlist/{channel}_master.m3u8
    static QRegularExpression masterPattern("/playlist/(\\w+)_master\\.m3u8");
    QRegularExpressionMatch masterMatch = masterPattern.match(path);
    if (masterMatch.hasMatch()) {
        return handleMasterPlaylist(masterMatch.captured(1));
    }

    // Media playlist: /playlist/{channel}_{quality}.m3u8
    static QRegularExpression mediaPattern("/playlist/(\\w+)_(\\w+)\\.m3u8");
    QRegularExpressionMatch mediaMatch = mediaPattern.match(path);
    if (mediaMatch.hasMatch()) {
        return handleMediaPlaylist(mediaMatch.captured(1), mediaMatch.captured(2));
    }

    // Segment: /segments/{channel}_{quality}_{number}.ts
    static QRegularExpression segmentPattern("/segments/(\\w+)_(\\w+)_(\\d+)\\.ts");
    QRegularExpressionMatch segmentMatch = segmentPattern.match(path);
    if (segmentMatch.hasMatch()) {
        return handleSegment(segmentMatch.captured(1), segmentMatch.captured(3).toInt());
    }

    return make404("Not found");
}

QByteArray MockHlsServer::handleLiveChannel(const QString& channel)
{
    if (!m_channels.contains(channel)) {
        return make404("Channel not found");
    }
    Q_EMIT playlistRequested(channel, "master");
    return handleMasterPlaylist(channel);
}

QByteArray MockHlsServer::handleMasterPlaylist(const QString& channel)
{
    if (!m_channels.contains(channel)) {
        return make404("Channel not found");
    }

    Q_EMIT playlistRequested(channel, "master");

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

QByteArray MockHlsServer::handleMediaPlaylist(const QString& channel, const QString& quality)
{
    if (!m_channels.contains(channel)) {
        return make404("Channel not found");
    }

    Q_EMIT playlistRequested(channel, quality);

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

    for (int i = 0; i < segmentCount; ++i) {
        int segNum = config.mediaSequence + i;
        playlist += QString("#EXTINF:%1.000,\n").arg(duration);
        playlist += QString("%1/segments/%2_%3_%4.ts\n")
                        .arg(baseUrl(), channel, quality)
                        .arg(segNum);
    }

    config.mediaSequence++;

    return makeResponse(200, "OK", playlist.toUtf8(), "application/vnd.apple.mpegurl");
}

QByteArray MockHlsServer::handleSegment(const QString& channel, int segmentNumber)
{
    if (m_stalled) {
        return QByteArray();
    }

    m_segmentRequestCount++;
    Q_EMIT segmentRequested(channel, segmentNumber);

    return makeResponse(200, "OK", makeMinimalSegment(), "video/mp2t");
}

QByteArray MockHlsServer::makeMinimalSegment()
{
    if (!m_segmentData.isEmpty()) {
        return m_segmentData;
    }

    static const unsigned char minimalTs[] = {
        0x47, 0x40, 0x00, 0x10, 0x00,
        0x00, 0xB0, 0x0D, 0x00, 0x01, 0xC1, 0x00, 0x00,
        0x00, 0x01, 0xF0, 0x00, 0x2A, 0xB1, 0x04, 0xB2,
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

} // namespace blueplayer::test::e2e
