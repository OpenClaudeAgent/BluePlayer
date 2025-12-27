#include <QtTest/QtTest>
#include <QRegularExpression>
#include <QStringList>

#include "ContractLoader.hpp"

using namespace blueplayer::test;

/**
 * @brief Contract tests for HLS playlist format
 */
class TestHlsContracts : public QObject {
    Q_OBJECT

private:
    // Helper to parse HLS master playlist variants
    struct StreamVariant {
        QString name;
        QString url;
        int bandwidth = 0;
        int width = 0;
        int height = 0;
    };
    
    QList<StreamVariant> parseVariants(const QString& playlist) {
        QList<StreamVariant> variants;
        QStringList lines = playlist.split(QLatin1Char('\n'));
        
        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i].trimmed();
            
            if (line.startsWith(QStringLiteral("#EXT-X-STREAM-INF:"))) {
                StreamVariant variant;
                
                // Extract BANDWIDTH
                QRegularExpression bandwidthRe(QStringLiteral("BANDWIDTH=(\\d+)"));
                QRegularExpressionMatch match = bandwidthRe.match(line);
                if (match.hasMatch()) {
                    variant.bandwidth = match.captured(1).toInt();
                }
                
                // Extract RESOLUTION
                QRegularExpression resolutionRe(QStringLiteral("RESOLUTION=(\\d+)x(\\d+)"));
                match = resolutionRe.match(line);
                if (match.hasMatch()) {
                    variant.width = match.captured(1).toInt();
                    variant.height = match.captured(2).toInt();
                }
                
                // Extract VIDEO (quality name)
                QRegularExpression videoRe(QStringLiteral("VIDEO=\"([^\"]+)\""));
                match = videoRe.match(line);
                if (match.hasMatch()) {
                    variant.name = match.captured(1);
                }
                
                // Next line should be the URL
                if (i + 1 < lines.size()) {
                    QString nextLine = lines[i + 1].trimmed();
                    if (!nextLine.isEmpty() && !nextLine.startsWith(QLatin1Char('#'))) {
                        variant.url = nextLine;
                        variants.append(variant);
                    }
                }
            }
        }
        
        return variants;
    }

private slots:
    // ========================================================================
    // Master Playlist Contract Tests
    // ========================================================================

    void testMasterPlaylistLoads() {
        QString playlist = ContractLoader::loadMasterPlaylist();
        QVERIFY(!playlist.isEmpty());
    }

    void testMasterPlaylistStartsWithExtm3u() {
        QString playlist = ContractLoader::loadMasterPlaylist();
        QVERIFY(playlist.trimmed().startsWith(QStringLiteral("#EXTM3U")));
    }

    void testMasterPlaylistHasVariants() {
        QString playlist = ContractLoader::loadMasterPlaylist();
        QList<StreamVariant> variants = parseVariants(playlist);
        
        QVERIFY(!variants.isEmpty());
    }

    void testMasterPlaylistHasChunkedVariant() {
        QString playlist = ContractLoader::loadMasterPlaylist();
        QList<StreamVariant> variants = parseVariants(playlist);
        
        bool hasChunked = false;
        for (const StreamVariant& v : variants) {
            if (v.name == QStringLiteral("chunked")) {
                hasChunked = true;
                break;
            }
        }
        
        QVERIFY2(hasChunked, "Master playlist should have 'chunked' (source) variant");
    }

    void testVariantsHaveValidBandwidth() {
        QString playlist = ContractLoader::loadMasterPlaylist();
        QList<StreamVariant> variants = parseVariants(playlist);
        
        for (const StreamVariant& v : variants) {
            QVERIFY2(v.bandwidth > 0,
                     qPrintable(QStringLiteral("Variant %1 has invalid bandwidth: %2")
                         .arg(v.name).arg(v.bandwidth)));
        }
    }

    void testVariantsHaveValidUrls() {
        QString playlist = ContractLoader::loadMasterPlaylist();
        QList<StreamVariant> variants = parseVariants(playlist);
        
        for (const StreamVariant& v : variants) {
            QVERIFY2(!v.url.isEmpty(),
                     qPrintable(QStringLiteral("Variant %1 has empty URL").arg(v.name)));
            QVERIFY2(v.url.startsWith(QStringLiteral("https://")),
                     qPrintable(QStringLiteral("Variant %1 URL must be HTTPS: %2")
                         .arg(v.name, v.url)));
        }
    }

    void testMasterPlaylistHasTwitchInfo() {
        QString playlist = ContractLoader::loadMasterPlaylist();
        QVERIFY(playlist.contains(QStringLiteral("#EXT-X-TWITCH-INFO:")));
    }

    // ========================================================================
    // Clean Variant Playlist Contract Tests
    // ========================================================================

    void testVariantCleanLoads() {
        QString playlist = ContractLoader::loadVariantClean();
        QVERIFY(!playlist.isEmpty());
    }

    void testVariantCleanHasNoAdMarkers() {
        QString playlist = ContractLoader::loadVariantClean();
        QVERIFY(!ContractLoader::containsAdMarkers(playlist));
    }

    void testVariantCleanHasMediaSequence() {
        QString playlist = ContractLoader::loadVariantClean();
        QVERIFY(playlist.contains(QStringLiteral("#EXT-X-MEDIA-SEQUENCE:")));
    }

    void testVariantCleanHasTargetDuration() {
        QString playlist = ContractLoader::loadVariantClean();
        QVERIFY(playlist.contains(QStringLiteral("#EXT-X-TARGETDURATION:")));
    }

    void testVariantCleanHasSegments() {
        QString playlist = ContractLoader::loadVariantClean();
        QVERIFY(playlist.contains(QStringLiteral("#EXTINF:")));
        
        // Should have .ts segment URLs
        QRegularExpression segmentRe(QStringLiteral("https://.*\\.ts"));
        QVERIFY(segmentRe.match(playlist).hasMatch());
    }

    void testVariantCleanSegmentsAreLive() {
        QString playlist = ContractLoader::loadVariantClean();
        
        // Live segments have ",live" after EXTINF duration
        QRegularExpression liveRe(QStringLiteral("#EXTINF:\\d+\\.\\d+,live"));
        QVERIFY(liveRe.match(playlist).hasMatch());
    }

    // ========================================================================
    // Variant Playlist with Ads Contract Tests
    // ========================================================================

    void testVariantWithAdsLoads() {
        QString playlist = ContractLoader::loadVariantWithAds();
        QVERIFY(!playlist.isEmpty());
    }

    void testVariantWithAdsHasAdMarkers() {
        QString playlist = ContractLoader::loadVariantWithAds();
        QVERIFY(ContractLoader::containsAdMarkers(playlist));
    }

    void testVariantWithAdsHasDiscontinuity() {
        QString playlist = ContractLoader::loadVariantWithAds();
        QVERIFY(playlist.contains(QStringLiteral("#EXT-X-DISCONTINUITY")));
    }

    void testVariantWithAdsHasDateRange() {
        QString playlist = ContractLoader::loadVariantWithAds();
        QVERIFY(playlist.contains(QStringLiteral("#EXT-X-DATERANGE:")));
        QVERIFY(playlist.contains(QStringLiteral("twitch-stitched-ad")));
    }

    void testVariantWithAdsHasScte35Markers() {
        QString playlist = ContractLoader::loadVariantWithAds();
        QVERIFY(playlist.contains(QStringLiteral("#EXT-X-SCTE35-OUT:")));
        QVERIFY(playlist.contains(QStringLiteral("#EXT-X-SCTE35-IN:")));
    }

    void testVariantWithAdsHasAdSegments() {
        QString playlist = ContractLoader::loadVariantWithAds();
        
        // Ad segments have ",live-ad" after EXTINF duration
        QRegularExpression adRe(QStringLiteral("#EXTINF:\\d+\\.\\d+,live-ad"));
        QVERIFY(adRe.match(playlist).hasMatch());
    }

    void testVariantWithAdsHasDifferentSegmentDomain() {
        QString playlist = ContractLoader::loadVariantWithAds();
        
        // Ad segments come from cloudfront, not video-edge
        QVERIFY(playlist.contains(QStringLiteral("cloudfront.net")));
    }

    // ========================================================================
    // Ad Detection Logic Tests
    // ========================================================================

    void testAdDetectionIdentifiesCleanPlaylist() {
        QString clean = ContractLoader::loadVariantClean();
        QVERIFY(!ContractLoader::containsAdMarkers(clean));
    }

    void testAdDetectionIdentifiesAdPlaylist() {
        QString withAds = ContractLoader::loadVariantWithAds();
        QVERIFY(ContractLoader::containsAdMarkers(withAds));
    }
};

QTEST_MAIN(TestHlsContracts)
#include "TestHlsContracts.moc"
