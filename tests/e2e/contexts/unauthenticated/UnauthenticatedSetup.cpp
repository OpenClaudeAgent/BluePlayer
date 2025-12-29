#include "UnauthenticatedSetup.hpp"

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
#include <QQmlContext>
#include <qqml.h>

namespace blueplayer::test::e2e {

UnauthenticatedSetup::UnauthenticatedSetup() = default;
UnauthenticatedSetup::~UnauthenticatedSetup() = default;

void UnauthenticatedSetup::applicationAvailable()
{
    qInfo() << "[E2E Context: Unauthenticated] Initializing...";

    // ========================================================================
    // STEP 1: Start mock servers
    // ========================================================================
    m_twitchServer = std::make_unique<MockTwitchServer>();
    m_hlsServer = std::make_unique<MockHlsServer>();

    if (!m_twitchServer->start(static_cast<quint16>(m_twitchPort))) {
        qWarning() << "[E2E Context: Unauthenticated] Failed to start MockTwitchServer";
        return;
    }
    m_twitchPort = m_twitchServer->port();
    qInfo() << "[E2E Context: Unauthenticated] MockTwitchServer on port" << m_twitchPort;

    if (!m_hlsServer->start(static_cast<quint16>(m_hlsPort))) {
        qWarning() << "[E2E Context: Unauthenticated] Failed to start MockHlsServer";
        return;
    }
    m_hlsPort = m_hlsServer->port();
    qInfo() << "[E2E Context: Unauthenticated] MockHlsServer on port" << m_hlsPort;

    m_twitchServer->setHlsServerUrl(m_hlsServer->baseUrl());

    // ========================================================================
    // STEP 2: Configure environment
    // ========================================================================
    qputenv("BLUEPLAYER_API_URL", m_twitchServer->baseUrl().toUtf8());
    qputenv("BLUEPLAYER_HLS_PROXY_URL", m_hlsServer->baseUrl().toUtf8());

    if (qgetenv("TWITCH_CLIENT_ID").isEmpty()) {
        qputenv("TWITCH_CLIENT_ID", "e2e_test_client_id");
    }

    // ========================================================================
    // STEP 3: Load Config
    // ========================================================================
    blueplayer::core::Config::instance().load();

    // ========================================================================
    // STEP 4: Create MockSecureStorage WITHOUT credentials (empty)
    // ========================================================================
    m_mockStorage = std::make_unique<MockSecureStorage>();
    // NOTE: We do NOT call setCredentials() - storage remains empty
    qInfo() << "[E2E Context: Unauthenticated] Mock storage created (NO credentials)";

    // ========================================================================
    // STEP 5: Create Application (will NOT be authenticated)
    // ========================================================================
    m_coreApp = std::make_unique<blueplayer::core::Application>(m_mockStorage.get());
    m_coreApp->initialize();

    if (m_coreApp->twitchService()) {
        bool isAuth = m_coreApp->twitchService()->isAuthenticated();
        qInfo() << "[E2E Context: Unauthenticated] Ready, authenticated:" << isAuth;
        
        if (isAuth) {
            qWarning() << "[E2E Context: Unauthenticated] WARNING: Should NOT be authenticated!";
        }
    }
}

void UnauthenticatedSetup::qmlEngineAvailable(QQmlEngine* engine)
{
    qInfo() << "[E2E Context: Unauthenticated] Configuring QML engine...";

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

    qInfo() << "[E2E Context: Unauthenticated] QML engine configured";
}

void UnauthenticatedSetup::cleanupTestCase()
{
    qInfo() << "[E2E Context: Unauthenticated] Cleaning up...";

    if (m_twitchServer) {
        qInfo() << "[E2E Context: Unauthenticated] MockTwitchServer handled" 
                << m_twitchServer->requestCount() << "requests";
        m_twitchServer->stop();
        m_twitchServer.reset();
    }

    if (m_hlsServer) {
        qInfo() << "[E2E Context: Unauthenticated] MockHlsServer handled" 
                << m_hlsServer->requestCount() << "requests";
        m_hlsServer->stop();
        m_hlsServer.reset();
    }

    qInfo() << "[E2E Context: Unauthenticated] Cleanup complete";
}

} // namespace blueplayer::test::e2e
