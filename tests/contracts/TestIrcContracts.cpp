#include <QtTest/QtTest>
#include <QRegularExpression>
#include <QVariantMap>
#include <QVariantList>

#include "ContractLoader.hpp"

using namespace blueplayer::test;

/**
 * @brief Contract tests for Twitch IRC message format
 */
class TestIrcContracts : public QObject {
    Q_OBJECT

private:
    // Helper to parse IRC message (simplified version for testing)
    QVariantMap parseIrcMessage(const QString& rawMessage) {
        QVariantMap result;
        QString line = rawMessage;
        QVariantMap tags;
        
        // Parse tags if present (starts with @)
        if (line.startsWith(QLatin1Char('@'))) {
            qsizetype spaceIdx = line.indexOf(QLatin1Char(' '));
            if (spaceIdx > 0) {
                QString tagsStr = line.mid(1, spaceIdx - 1);
                
                // Parse tag key=value pairs
                const QStringList tagPairs = tagsStr.split(QLatin1Char(';'), Qt::SkipEmptyParts);
                for (const QString& pair : tagPairs) {
                    qsizetype eqIdx = pair.indexOf(QLatin1Char('='));
                    if (eqIdx > 0) {
                        QString key = pair.left(eqIdx);
                        QString value = pair.mid(eqIdx + 1);
                        tags[key] = value;
                    }
                }
                
                line = line.mid(spaceIdx + 1);
            }
        }
        
        result[QStringLiteral("tags")] = tags;
        
        // Skip prefix if present (starts with :)
        QString prefix;
        if (line.startsWith(QLatin1Char(':'))) {
            qsizetype spaceIdx = line.indexOf(QLatin1Char(' '));
            if (spaceIdx > 0) {
                prefix = line.mid(1, spaceIdx - 1);
                line = line.mid(spaceIdx + 1);
            }
        }
        result[QStringLiteral("prefix")] = prefix;
        
        // Parse command and trailing
        qsizetype colonIdx = line.indexOf(QStringLiteral(" :"));
        QString commandPart = colonIdx > 0 ? line.left(colonIdx) : line;
        QString trailing = colonIdx > 0 ? line.mid(colonIdx + 2) : QString();
        
        QStringList parts = commandPart.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            result[QStringLiteral("command")] = parts.takeFirst();
            result[QStringLiteral("params")] = parts;
        }
        result[QStringLiteral("trailing")] = trailing;
        
        return result;
    }

private slots:
    // ========================================================================
    // PRIVMSG Contract Tests
    // ========================================================================

    void testPrivmsgWithEmotesLoads() {
        QString raw = ContractLoader::loadPrivmsgWithEmotes();
        QVERIFY(!raw.isEmpty());
    }

    void testPrivmsgWithEmotesParsesToValidStructure() {
        QString raw = ContractLoader::loadPrivmsgWithEmotes();
        QVariantMap msg = parseIrcMessage(raw);
        
        QCOMPARE(msg[QStringLiteral("command")].toString(), QStringLiteral("PRIVMSG"));
        QVERIFY(!msg[QStringLiteral("trailing")].toString().isEmpty());
    }

    void testPrivmsgHasDisplayNameTag() {
        QString raw = ContractLoader::loadPrivmsgWithEmotes();
        QVariantMap msg = parseIrcMessage(raw);
        QVariantMap tags = msg[QStringLiteral("tags")].toMap();
        
        QVERIFY(tags.contains(QStringLiteral("display-name")));
        QVERIFY(!tags[QStringLiteral("display-name")].toString().isEmpty());
    }

    void testPrivmsgHasColorTag() {
        QString raw = ContractLoader::loadPrivmsgWithEmotes();
        QVariantMap msg = parseIrcMessage(raw);
        QVariantMap tags = msg[QStringLiteral("tags")].toMap();
        
        QVERIFY(tags.contains(QStringLiteral("color")));
        QString color = tags[QStringLiteral("color")].toString();
        QVERIFY(color.startsWith(QLatin1Char('#')));
        QCOMPARE(color.length(), 7);  // #RRGGBB
    }

    void testPrivmsgHasIdTag() {
        QString raw = ContractLoader::loadPrivmsgWithEmotes();
        QVariantMap msg = parseIrcMessage(raw);
        QVariantMap tags = msg[QStringLiteral("tags")].toMap();
        
        QVERIFY(tags.contains(QStringLiteral("id")));
        // ID should be a UUID-like string
        QString id = tags[QStringLiteral("id")].toString();
        QVERIFY(!id.isEmpty());
    }

    void testPrivmsgEmotesTagFormat() {
        QString raw = ContractLoader::loadPrivmsgWithEmotes();
        QVariantMap msg = parseIrcMessage(raw);
        QVariantMap tags = msg[QStringLiteral("tags")].toMap();
        
        QVERIFY(tags.contains(QStringLiteral("emotes")));
        QString emotes = tags[QStringLiteral("emotes")].toString();
        
        // Format: emoteId:start-end,start-end/emoteId:start-end
        // Example: 25:0-4,6-10/1902:12-16
        if (!emotes.isEmpty()) {
            QRegularExpression emoteRe(QStringLiteral("^(\\d+:\\d+-\\d+(,\\d+-\\d+)*/?)+$"));
            QVERIFY2(emoteRe.match(emotes).hasMatch(),
                     qPrintable(QStringLiteral("Invalid emotes format: %1").arg(emotes)));
        }
    }

    void testPrivmsgWithBadgesParsesToValidStructure() {
        QString raw = ContractLoader::loadPrivmsgWithBadges();
        QVariantMap msg = parseIrcMessage(raw);
        
        QCOMPARE(msg[QStringLiteral("command")].toString(), QStringLiteral("PRIVMSG"));
    }

    void testPrivmsgBadgesTagFormat() {
        QString raw = ContractLoader::loadPrivmsgWithBadges();
        QVariantMap msg = parseIrcMessage(raw);
        QVariantMap tags = msg[QStringLiteral("tags")].toMap();
        
        QVERIFY(tags.contains(QStringLiteral("badges")));
        QString badges = tags[QStringLiteral("badges")].toString();
        
        // Format: badge/version,badge/version
        // Example: moderator/1,subscriber/24,bits/1000
        if (!badges.isEmpty()) {
            QRegularExpression badgeRe(QStringLiteral("^([a-z_]+/\\d+,?)+$"));
            QVERIFY2(badgeRe.match(badges).hasMatch(),
                     qPrintable(QStringLiteral("Invalid badges format: %1").arg(badges)));
        }
    }

    // ========================================================================
    // NOTICE Contract Tests
    // ========================================================================

    void testNoticeAuthFailedLoads() {
        QString raw = ContractLoader::loadNoticeAuthFailed();
        QVERIFY(!raw.isEmpty());
    }

    void testNoticeAuthFailedHasCorrectCommand() {
        QString raw = ContractLoader::loadNoticeAuthFailed();
        QVariantMap msg = parseIrcMessage(raw);
        
        QCOMPARE(msg[QStringLiteral("command")].toString(), QStringLiteral("NOTICE"));
    }

    void testNoticeAuthFailedHasAuthMessage() {
        QString raw = ContractLoader::loadNoticeAuthFailed();
        QVariantMap msg = parseIrcMessage(raw);
        
        QString trailing = msg[QStringLiteral("trailing")].toString();
        QVERIFY(trailing.contains(QStringLiteral("authentication failed"), Qt::CaseInsensitive));
    }
};

QTEST_MAIN(TestIrcContracts)
#include "TestIrcContracts.moc"
