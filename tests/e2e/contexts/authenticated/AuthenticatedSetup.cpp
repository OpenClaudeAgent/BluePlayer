#include "AuthenticatedSetup.hpp"

#include "api/twitch/TwitchService.hpp"
#include "chat/TwitchChatClient.hpp"
#include "core/CacheManager.hpp"
#include "core/Config.hpp"
#include "media/MpvQuickItem.hpp"
#include "ui/CacheManagerViewModel.hpp"
#include "ui/HomeViewModel.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlContext>
#include <qqml.h>

namespace blueplayer::test::e2e {

AuthenticatedSetup::AuthenticatedSetup() = default;
AuthenticatedSetup::~AuthenticatedSetup() = default;

void AuthenticatedSetup::applicationAvailable()
{
    qInfo() << "[E2E Context: Authenticated] Initializing...";
    
    // Find fixtures path
    QString fixturesPath = QDir::currentPath() + "/tests/e2e/fixtures";
    if (!QDir(fixturesPath).exists()) {
        fixturesPath = QCoreApplication::applicationDirPath() + "/../../../../../tests/e2e/fixtures";
    }
    if (!QDir(fixturesPath).exists()) {
        fixturesPath = QCoreApplication::applicationDirPath() + "/../../fixtures";
    }
    qInfo() << "[E2E Context: Authenticated] Fixtures path:" << fixturesPath;

    // ========================================================================
    // STEP 1: Start mock servers
    // ========================================================================
    qInfo() << "[E2E Context: Authenticated] Starting mock servers...";

    m_twitchServer = std::make_unique<MockTwitchServer>();
    m_hlsServer = std::make_unique<MockHlsServer>();

    if (!m_twitchServer->start(static_cast<quint16>(m_twitchPort))) {
        qWarning() << "[E2E Context: Authenticated] Failed to start MockTwitchServer";
        return;
    }
    m_twitchPort = m_twitchServer->port();
    qInfo() << "[E2E Context: Authenticated] MockTwitchServer on port" << m_twitchPort;

    if (!m_hlsServer->start(static_cast<quint16>(m_hlsPort))) {
        qWarning() << "[E2E Context: Authenticated] Failed to start MockHlsServer";
        return;
    }
    m_hlsPort = m_hlsServer->port();
    qInfo() << "[E2E Context: Authenticated] MockHlsServer on port" << m_hlsPort;

    m_twitchServer->setHlsServerUrl(m_hlsServer->baseUrl());

    // ========================================================================
    // STEP 2: Load fixtures
    // ========================================================================
    qInfo() << "[E2E Context: Authenticated] Loading fixtures...";

    QString accessToken;
    QString refreshToken;
    
    // Load auth_token.json
    {
        QFile authFile(fixturesPath + "/auth_token.json");
        if (authFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(authFile.readAll());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                accessToken = obj["access_token"].toString();
                refreshToken = obj["refresh_token"].toString();
                m_twitchServer->setValidToken(accessToken);
                qInfo() << "[E2E Context: Authenticated] Auth token loaded";
            }
            authFile.close();
        }
    }

    // Load streams.json
    {
        QFile streamsFile(fixturesPath + "/streams.json");
        if (streamsFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(streamsFile.readAll());
            QJsonArray streamsArray;
            
            if (doc.isObject() && doc.object().contains("data")) {
                streamsArray = doc.object()["data"].toArray();
            } else if (doc.isArray()) {
                streamsArray = doc.array();
            }
            
            if (!streamsArray.isEmpty()) {
                m_twitchServer->setStreams(streamsArray);
                qInfo() << "[E2E Context: Authenticated] Loaded" << streamsArray.size() << "streams";
                
                for (const QJsonValue& streamVal : streamsArray) {
                    QJsonObject stream = streamVal.toObject();
                    QString userLogin = stream["user_login"].toString();
                    if (!userLogin.isEmpty()) {
                        m_hlsServer->addChannel(userLogin);
                    }
                }
            }
        }
    }

    // Load users.json
    {
        QFile usersFile(fixturesPath + "/users.json");
        if (usersFile.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(usersFile.readAll());
            QJsonArray usersArray;
            
            if (doc.isObject() && doc.object().contains("data")) {
                usersArray = doc.object()["data"].toArray();
            } else if (doc.isArray()) {
                usersArray = doc.array();
            }
            
            if (!usersArray.isEmpty()) {
                m_twitchServer->setUsers(usersArray);
                qInfo() << "[E2E Context: Authenticated] Loaded" << usersArray.size() << "users";
            }
        }
    }

    // ========================================================================
    // STEP 3: Configure environment
    // ========================================================================
    qputenv("BLUEPLAYER_API_URL", m_twitchServer->baseUrl().toUtf8());
    qputenv("BLUEPLAYER_HLS_PROXY_URL", m_hlsServer->baseUrl().toUtf8());
    
    if (qgetenv("TWITCH_CLIENT_ID").isEmpty()) {
        qputenv("TWITCH_CLIENT_ID", "e2e_test_client_id");
    }

    // ========================================================================
    // STEP 4: Load Config
    // ========================================================================
    blueplayer::core::Config::instance().load();

    // ========================================================================
    // STEP 5: Create MockSecureStorage WITH credentials
    // ========================================================================
    m_mockStorage = std::make_unique<MockSecureStorage>();
    
    if (!accessToken.isEmpty()) {
        QString clientId = QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID"));
        m_mockStorage->setCredentials(accessToken, refreshToken, clientId, 14400);
        qInfo() << "[E2E Context: Authenticated] Credentials injected";
    }

    // ========================================================================
    // STEP 6: Create Application (will be authenticated)
    // ========================================================================
    m_coreApp = std::make_unique<blueplayer::core::Application>(m_mockStorage.get());
    m_coreApp->initialize();
    
    if (m_coreApp->twitchService()) {
        qInfo() << "[E2E Context: Authenticated] Ready, authenticated:" 
                << m_coreApp->twitchService()->isAuthenticated();
    }
}

void AuthenticatedSetup::qmlEngineAvailable(QQmlEngine* engine)
{
    qInfo() << "[E2E Context: Authenticated] Configuring QML engine...";

    // Register QML types
    qmlRegisterType<blueplayer::media::MpvQuickItem>("BluePlayer.Media", 1, 0, "MpvQuickItem");
    qmlRegisterType<blueplayer::api::twitch::TwitchService>("BluePlayer.Twitch", 1, 0, "TwitchService");
    qmlRegisterType<blueplayer::ui::HomeViewModel>("BluePlayer.UI", 1, 0, "HomeViewModel");
    qmlRegisterType<blueplayer::ui::CacheManagerViewModel>("BluePlayer.UI", 1, 0, "CacheManagerViewModel");
    qmlRegisterType<BluePlayer::TwitchChatClient>("BluePlayer.Chat", 1, 0, "TwitchChatClient");

    // Configure import paths
    QString appDir = QCoreApplication::applicationDirPath();
    QDir srcDir(appDir + "/../../../../../src");
    QString srcPath = srcDir.absolutePath();
    QString uiPath = srcPath + "/ui";
    
    engine->addImportPath(srcPath);
    engine->addImportPath(uiPath);
    engine->addImportPath(uiPath + "/components");
    engine->addImportPath(uiPath + "/themes");
    engine->addImportPath(appDir);
    engine->addImportPath(appDir + "/../src");

    // Context properties
    engine->rootContext()->setContextProperty("E2E_QML_PATH", uiPath);

    QString screenshotsPath = appDir + "/screenshots";
    QDir().mkpath(screenshotsPath);
    engine->rootContext()->setContextProperty("E2E_SCREENSHOTS_PATH", screenshotsPath);

    // Expose services
    if (m_coreApp) {
        engine->rootContext()->setContextProperty("twitchService", 
            qobject_cast<QObject*>(m_coreApp->twitchService()));
        engine->rootContext()->setContextProperty("cacheManager", 
            qobject_cast<QObject*>(m_coreApp->cacheManager()));
    }

    qInfo() << "[E2E Context: Authenticated] QML engine configured";
}

void AuthenticatedSetup::cleanupTestCase()
{
    qInfo() << "[E2E Context: Authenticated] Cleaning up...";

    if (m_twitchServer) {
        qInfo() << "[E2E Context: Authenticated] MockTwitchServer handled" 
                << m_twitchServer->requestCount() << "requests";
        m_twitchServer->stop();
        m_twitchServer.reset();
    }

    if (m_hlsServer) {
        qInfo() << "[E2E Context: Authenticated] MockHlsServer handled" 
                << m_hlsServer->requestCount() << "requests";
        m_hlsServer->stop();
        m_hlsServer.reset();
    }

    qInfo() << "[E2E Context: Authenticated] Cleanup complete";
}

} // namespace blueplayer::test::e2e
