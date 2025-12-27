#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "ContractLoader.hpp"

using namespace blueplayer::test;

/**
 * @brief Contract tests for Twitch OAuth2 API responses
 */
class TestOAuthContracts : public QObject {
    Q_OBJECT

private slots:
    // ========================================================================
    // Token Success Contract Tests
    // ========================================================================

    void testTokenSuccessContractLoads() {
        QJsonDocument doc = ContractLoader::loadTokenSuccess();
        QVERIFY(!doc.isNull());
        QVERIFY(doc.isObject());
    }

    void testTokenSuccessHasRequiredFields() {
        QJsonDocument doc = ContractLoader::loadTokenSuccess();
        QJsonObject token = doc.object();
        
        QVERIFY(ContractLoader::validateTokenResponse(token));
    }

    void testAccessTokenNotEmpty() {
        QJsonDocument doc = ContractLoader::loadTokenSuccess();
        QJsonObject token = doc.object();
        
        QString accessToken = token[QStringLiteral("access_token")].toString();
        QVERIFY(!accessToken.isEmpty());
    }

    void testRefreshTokenNotEmpty() {
        QJsonDocument doc = ContractLoader::loadTokenSuccess();
        QJsonObject token = doc.object();
        
        QString refreshToken = token[QStringLiteral("refresh_token")].toString();
        QVERIFY(!refreshToken.isEmpty());
    }

    void testExpiresInPositive() {
        QJsonDocument doc = ContractLoader::loadTokenSuccess();
        QJsonObject token = doc.object();
        
        int expiresIn = token[QStringLiteral("expires_in")].toInt();
        QVERIFY(expiresIn > 0);
    }

    void testTokenTypeIsBearer() {
        QJsonDocument doc = ContractLoader::loadTokenSuccess();
        QJsonObject token = doc.object();
        
        QString tokenType = token[QStringLiteral("token_type")].toString();
        QCOMPARE(tokenType, QStringLiteral("bearer"));
    }

    void testScopeIsArray() {
        QJsonDocument doc = ContractLoader::loadTokenSuccess();
        QJsonObject token = doc.object();
        
        QVERIFY(token.contains(QStringLiteral("scope")));
        QVERIFY(token[QStringLiteral("scope")].isArray());
    }

    // ========================================================================
    // Token Error Contract Tests
    // ========================================================================

    void testInvalidRefreshTokenContract() {
        QJsonDocument doc = ContractLoader::loadTokenInvalidRefresh();
        QVERIFY(!doc.isNull());
        
        QJsonObject error = doc.object();
        QVERIFY(error.contains(QStringLiteral("status")));
        QVERIFY(error.contains(QStringLiteral("message")));
        
        QCOMPARE(error[QStringLiteral("status")].toInt(), 400);
        QVERIFY(error[QStringLiteral("message")].toString().contains(QStringLiteral("refresh token")));
    }
};

QTEST_MAIN(TestOAuthContracts)
#include "TestOAuthContracts.moc"
