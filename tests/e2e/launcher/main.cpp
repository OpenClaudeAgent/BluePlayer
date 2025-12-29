/**
 * E2E Launcher - Standalone app launcher with mock servers
 * 
 * This launches the BluePlayer app with:
 * - Mock Twitch API server
 * - Mock HLS server  
 * - Mock authentication (pre-authenticated)
 * 
 * Usage: ./e2e_launcher
 * 
 * The app will start on the Home view, fully authenticated,
 * with all API calls going to the mock servers.
 */

#include "mocks/MockSecureStorage.hpp"
#include "servers/MockHlsServer.hpp"
#include "servers/MockTwitchServer.hpp"

#include "api/twitch/TwitchService.hpp"
#include "chat/TwitchChatClient.hpp"
#include "core/Application.hpp"
#include "core/CacheManager.hpp"
#include "core/Config.hpp"
#include "media/MpvQuickItem.hpp"
#include "ui/CacheManagerViewModel.hpp"
#include "ui/HomeViewModel.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <qqml.h>

#include <memory>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("BluePlayer E2E");
    app.setOrganizationName("BluePlayer");

    qInfo() << "========================================";
    qInfo() << "BluePlayer E2E Launcher";
    qInfo() << "========================================";

    // ========================================================================
    // STEP 1: Find fixtures path
    // Executable is in build/tests/e2e/launcher/
    // ========================================================================
    QString fixturesPath = QDir::currentPath() + "/tests/e2e/fixtures";
    if (!QDir(fixturesPath).exists()) {
        fixturesPath = QCoreApplication::applicationDirPath() + "/../../../../tests/e2e/fixtures";
    }
    if (!QDir(fixturesPath).exists()) {
        fixturesPath = QCoreApplication::applicationDirPath() + "/../fixtures";
    }
    qInfo() << "[E2E Launcher] Fixtures path:" << fixturesPath;

    // ========================================================================
    // STEP 2: Start mock servers
    // ========================================================================
    auto twitchServer = std::make_unique<blueplayer::test::e2e::MockTwitchServer>();
    auto hlsServer = std::make_unique<blueplayer::test::e2e::MockHlsServer>();

    if (!twitchServer->start(8080)) {
        qCritical() << "[E2E Launcher] Failed to start MockTwitchServer";
        return 1;
    }
    qInfo() << "[E2E Launcher] MockTwitchServer on port" << twitchServer->port();

    if (!hlsServer->start(8081)) {
        qCritical() << "[E2E Launcher] Failed to start MockHlsServer";
        return 1;
    }
    qInfo() << "[E2E Launcher] MockHlsServer on port" << hlsServer->port();

    twitchServer->setHlsServerUrl(hlsServer->baseUrl());

    // ========================================================================
    // STEP 3: Load fixtures
    // ========================================================================
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
                twitchServer->setValidToken(accessToken);
                qInfo() << "[E2E Launcher] Auth token loaded";
            }
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
            twitchServer->setStreams(streamsArray);
            qInfo() << "[E2E Launcher] Loaded" << streamsArray.size() << "streams";
            
            for (const QJsonValue& streamVal : streamsArray) {
                QJsonObject stream = streamVal.toObject();
                QString userLogin = stream["user_login"].toString();
                if (!userLogin.isEmpty()) {
                    hlsServer->addChannel(userLogin);
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
            twitchServer->setUsers(usersArray);
            qInfo() << "[E2E Launcher] Loaded" << usersArray.size() << "users";
        }
    }

    // ========================================================================
    // STEP 4: Set environment variables
    // ========================================================================
    qputenv("BLUEPLAYER_API_URL", twitchServer->baseUrl().toUtf8());
    qputenv("BLUEPLAYER_HLS_PROXY_URL", hlsServer->baseUrl().toUtf8());
    
    if (qgetenv("TWITCH_CLIENT_ID").isEmpty()) {
        qputenv("TWITCH_CLIENT_ID", "e2e_test_client_id");
    }

    qInfo() << "[E2E Launcher] Environment configured";
    qInfo() << "  BLUEPLAYER_API_URL=" << twitchServer->baseUrl();
    qInfo() << "  BLUEPLAYER_HLS_PROXY_URL=" << hlsServer->baseUrl();

    // ========================================================================
    // STEP 5: Load config (picks up env vars)
    // ========================================================================
    blueplayer::core::Config::instance().load();
    qInfo() << "[E2E Launcher] Config loaded";

    // ========================================================================
    // STEP 6: Create mock storage with credentials
    // ========================================================================
    auto mockStorage = std::make_unique<blueplayer::test::e2e::MockSecureStorage>();
    QString clientId = QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID"));
    mockStorage->setCredentials(accessToken, refreshToken, clientId, 14400);
    qInfo() << "[E2E Launcher] Mock credentials configured";

    // ========================================================================
    // STEP 7: Create core application
    // ========================================================================
    auto coreApp = std::make_unique<blueplayer::core::Application>(mockStorage.get());
    coreApp->initialize();
    qInfo() << "[E2E Launcher] Core app initialized, authenticated:" 
            << coreApp->twitchService()->isAuthenticated();

    // ========================================================================
    // STEP 8: Register QML types
    // ========================================================================
    qmlRegisterType<blueplayer::media::MpvQuickItem>("BluePlayer.Media", 1, 0, "MpvQuickItem");
    qmlRegisterType<blueplayer::api::twitch::TwitchService>("BluePlayer.Twitch", 1, 0, "TwitchService");
    qmlRegisterType<blueplayer::ui::HomeViewModel>("BluePlayer.UI", 1, 0, "HomeViewModel");
    qmlRegisterType<blueplayer::ui::CacheManagerViewModel>("BluePlayer.UI", 1, 0, "CacheManagerViewModel");
    qmlRegisterType<BluePlayer::TwitchChatClient>("BluePlayer.Chat", 1, 0, "TwitchChatClient");
    qInfo() << "[E2E Launcher] QML types registered";

    // ========================================================================
    // STEP 9: Create QML engine and load app
    // ========================================================================
    QQmlApplicationEngine engine;

    // Add import paths
    // Executable is in build/tests/e2e/launcher/
    QString appDir = QCoreApplication::applicationDirPath();
    QDir srcDir(appDir + "/../../../../src");
    QString srcPath = srcDir.absolutePath();
    QString uiPath = srcPath + "/ui";

    engine.addImportPath(srcPath);
    engine.addImportPath(uiPath);
    engine.addImportPath(uiPath + "/components");
    engine.addImportPath(uiPath + "/themes");
    engine.addImportPath(appDir);
    engine.addImportPath(appDir + "/../src");

    // Expose services to QML
    engine.rootContext()->setContextProperty("twitchService", coreApp->twitchService());
    engine.rootContext()->setContextProperty("cacheManager", coreApp->cacheManager());

    qInfo() << "[E2E Launcher] Loading main.qml from:" << uiPath;

    // Load main.qml
    QUrl mainUrl = QUrl::fromLocalFile(uiPath + "/main.qml");
    engine.load(mainUrl);

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "[E2E Launcher] Failed to load main.qml";
        return 1;
    }

    qInfo() << "========================================";
    qInfo() << "BluePlayer E2E App Started!";
    qInfo() << "- Authenticated: YES";
    qInfo() << "- Mock API:" << twitchServer->baseUrl();
    qInfo() << "- Mock HLS:" << hlsServer->baseUrl();
    qInfo() << "========================================";

    return app.exec();
}
