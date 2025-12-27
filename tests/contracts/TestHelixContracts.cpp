#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "ContractLoader.hpp"

using namespace blueplayer::test;

/**
 * @brief Contract tests for Twitch Helix API responses
 *
 * These tests verify that the application correctly handles the expected
 * API response formats. They use recorded API responses as contracts.
 */
class TestHelixContracts : public QObject {
    Q_OBJECT

private slots:
    // ========================================================================
    // Stream Endpoint Contract Tests
    // ========================================================================

    void testStreamContractLoads() {
        QJsonDocument doc = ContractLoader::loadStreamsSuccess();
        QVERIFY(!doc.isNull());
        QVERIFY(doc.isObject());
    }

    void testStreamContractHasDataArray() {
        QJsonDocument doc = ContractLoader::loadStreamsSuccess();
        QJsonObject root = doc.object();
        
        QVERIFY(root.contains(QStringLiteral("data")));
        QVERIFY(root[QStringLiteral("data")].isArray());
    }

    void testStreamContractHasPagination() {
        QJsonDocument doc = ContractLoader::loadStreamsSuccess();
        QJsonObject root = doc.object();
        
        QVERIFY(root.contains(QStringLiteral("pagination")));
        QVERIFY(root[QStringLiteral("pagination")].isObject());
    }

    void testStreamObjectHasRequiredFields() {
        QJsonDocument doc = ContractLoader::loadStreamsSuccess();
        QJsonArray streams = doc.object()[QStringLiteral("data")].toArray();
        
        QVERIFY(!streams.isEmpty());
        
        for (const QJsonValue& streamVal : streams) {
            QJsonObject stream = streamVal.toObject();
            QVERIFY2(ContractLoader::validateStreamObject(stream),
                     qPrintable(QStringLiteral("Stream missing required fields: %1")
                         .arg(stream[QStringLiteral("id")].toString())));
        }
    }

    void testStreamThumbnailUrlFormat() {
        QJsonDocument doc = ContractLoader::loadStreamsSuccess();
        QJsonArray streams = doc.object()[QStringLiteral("data")].toArray();
        
        for (const QJsonValue& streamVal : streams) {
            QJsonObject stream = streamVal.toObject();
            QString thumbnailUrl = stream[QStringLiteral("thumbnail_url")].toString();
            
            QVERIFY2(thumbnailUrl.contains(QStringLiteral("{width}")),
                     "thumbnail_url must contain {width} placeholder");
            QVERIFY2(thumbnailUrl.contains(QStringLiteral("{height}")),
                     "thumbnail_url must contain {height} placeholder");
        }
    }

    void testStreamViewerCountIsInteger() {
        QJsonDocument doc = ContractLoader::loadStreamsSuccess();
        QJsonArray streams = doc.object()[QStringLiteral("data")].toArray();
        
        for (const QJsonValue& streamVal : streams) {
            QJsonObject stream = streamVal.toObject();
            QVERIFY(stream[QStringLiteral("viewer_count")].isDouble());
            QVERIFY(stream[QStringLiteral("viewer_count")].toInt() >= 0);
        }
    }

    void testEmptyStreamsContractValid() {
        QJsonDocument doc = ContractLoader::loadStreamsEmpty();
        QVERIFY(!doc.isNull());
        
        QJsonObject root = doc.object();
        QVERIFY(root.contains(QStringLiteral("data")));
        QVERIFY(root[QStringLiteral("data")].isArray());
        QCOMPARE(root[QStringLiteral("data")].toArray().size(), 0);
    }

    // ========================================================================
    // User Endpoint Contract Tests
    // ========================================================================

    void testUserContractLoads() {
        QJsonDocument doc = ContractLoader::loadUsersSuccess();
        QVERIFY(!doc.isNull());
        QVERIFY(doc.isObject());
    }

    void testUserObjectHasRequiredFields() {
        QJsonDocument doc = ContractLoader::loadUsersSuccess();
        QJsonArray users = doc.object()[QStringLiteral("data")].toArray();
        
        QVERIFY(!users.isEmpty());
        
        for (const QJsonValue& userVal : users) {
            QJsonObject user = userVal.toObject();
            QVERIFY2(ContractLoader::validateUserObject(user),
                     qPrintable(QStringLiteral("User missing required fields: %1")
                         .arg(user[QStringLiteral("id")].toString())));
        }
    }

    void testUserProfileImageUrlValid() {
        QJsonDocument doc = ContractLoader::loadUsersSuccess();
        QJsonArray users = doc.object()[QStringLiteral("data")].toArray();
        
        for (const QJsonValue& userVal : users) {
            QJsonObject user = userVal.toObject();
            QString profileUrl = user[QStringLiteral("profile_image_url")].toString();
            
            QVERIFY(!profileUrl.isEmpty());
            QVERIFY(profileUrl.startsWith(QStringLiteral("https://")));
        }
    }

    // ========================================================================
    // Clip Endpoint Contract Tests
    // ========================================================================

    void testClipContractLoads() {
        QJsonDocument doc = ContractLoader::loadClipsSuccess();
        QVERIFY(!doc.isNull());
        QVERIFY(doc.isObject());
    }

    void testClipObjectHasRequiredFields() {
        QJsonDocument doc = ContractLoader::loadClipsSuccess();
        QJsonArray clips = doc.object()[QStringLiteral("data")].toArray();
        
        QVERIFY(!clips.isEmpty());
        
        for (const QJsonValue& clipVal : clips) {
            QJsonObject clip = clipVal.toObject();
            QVERIFY2(ContractLoader::validateClipObject(clip),
                     qPrintable(QStringLiteral("Clip missing required fields: %1")
                         .arg(clip[QStringLiteral("id")].toString())));
        }
    }

    void testClipDurationIsNumber() {
        QJsonDocument doc = ContractLoader::loadClipsSuccess();
        QJsonArray clips = doc.object()[QStringLiteral("data")].toArray();
        
        for (const QJsonValue& clipVal : clips) {
            QJsonObject clip = clipVal.toObject();
            QVERIFY(clip[QStringLiteral("duration")].isDouble());
            QVERIFY(clip[QStringLiteral("duration")].toDouble() > 0);
        }
    }

    // ========================================================================
    // Video Endpoint Contract Tests
    // ========================================================================

    void testVideoContractLoads() {
        QJsonDocument doc = ContractLoader::loadVideosSuccess();
        QVERIFY(!doc.isNull());
        QVERIFY(doc.isObject());
    }

    void testVideoDurationFormat() {
        QJsonDocument doc = ContractLoader::loadVideosSuccess();
        QJsonArray videos = doc.object()[QStringLiteral("data")].toArray();
        
        // Duration format: "1h2m3s"
        QRegularExpression durationRe(QStringLiteral("^(\\d+h)?(\\d+m)?(\\d+s)?$"));
        
        for (const QJsonValue& videoVal : videos) {
            QJsonObject video = videoVal.toObject();
            QString duration = video[QStringLiteral("duration")].toString();
            
            QVERIFY2(durationRe.match(duration).hasMatch(),
                     qPrintable(QStringLiteral("Invalid duration format: %1").arg(duration)));
        }
    }

    // ========================================================================
    // Category Endpoint Contract Tests
    // ========================================================================

    void testCategoryContractLoads() {
        QJsonDocument doc = ContractLoader::loadCategoriesSuccess();
        QVERIFY(!doc.isNull());
        QVERIFY(doc.isObject());
    }

    void testCategoryBoxArtUrlFormat() {
        QJsonDocument doc = ContractLoader::loadCategoriesSuccess();
        QJsonArray categories = doc.object()[QStringLiteral("data")].toArray();
        
        for (const QJsonValue& categoryVal : categories) {
            QJsonObject category = categoryVal.toObject();
            QString boxArtUrl = category[QStringLiteral("box_art_url")].toString();
            
            QVERIFY(!boxArtUrl.isEmpty());
            QVERIFY2(boxArtUrl.contains(QStringLiteral("{width}")),
                     "box_art_url must contain {width} placeholder");
            QVERIFY2(boxArtUrl.contains(QStringLiteral("{height}")),
                     "box_art_url must contain {height} placeholder");
        }
    }

    // ========================================================================
    // Error Response Contract Tests
    // ========================================================================

    void testUnauthorizedErrorContract() {
        QJsonDocument doc = ContractLoader::loadStreamsUnauthorized();
        QVERIFY(!doc.isNull());
        
        QJsonObject error = doc.object();
        QVERIFY(ContractLoader::validateErrorResponse(error));
        
        QCOMPARE(error[QStringLiteral("status")].toInt(), 401);
        QCOMPARE(error[QStringLiteral("error")].toString(), QStringLiteral("Unauthorized"));
        QVERIFY(!error[QStringLiteral("message")].toString().isEmpty());
    }

    void testRateLimitedErrorContract() {
        QJsonDocument doc = ContractLoader::loadStreamsRateLimited();
        QVERIFY(!doc.isNull());
        
        QJsonObject error = doc.object();
        QVERIFY(ContractLoader::validateErrorResponse(error));
        
        QCOMPARE(error[QStringLiteral("status")].toInt(), 429);
        QCOMPARE(error[QStringLiteral("error")].toString(), QStringLiteral("Too Many Requests"));
    }

    // ========================================================================
    // Backward Compatibility Tests
    // ========================================================================

    void testDeprecatedTagIdsFieldHandled() {
        // tag_ids was deprecated but may still be present
        QJsonDocument doc = ContractLoader::loadStreamsSuccess();
        QJsonArray streams = doc.object()[QStringLiteral("data")].toArray();
        
        // Should not crash when tag_ids is present (even if empty)
        for (const QJsonValue& streamVal : streams) {
            QJsonObject stream = streamVal.toObject();
            // tag_ids may be present or absent - both are valid
            if (stream.contains(QStringLiteral("tag_ids"))) {
                QVERIFY(stream[QStringLiteral("tag_ids")].isArray());
            }
        }
    }

    void testNewTagsFieldPresent() {
        // tags replaced tag_ids
        QJsonDocument doc = ContractLoader::loadStreamsSuccess();
        QJsonArray streams = doc.object()[QStringLiteral("data")].toArray();
        
        for (const QJsonValue& streamVal : streams) {
            QJsonObject stream = streamVal.toObject();
            // tags should be present in newer responses
            if (stream.contains(QStringLiteral("tags"))) {
                QVERIFY(stream[QStringLiteral("tags")].isArray());
            }
        }
    }
};

QTEST_MAIN(TestHelixContracts)
#include "TestHelixContracts.moc"
