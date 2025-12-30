#include "BaseE2EContext.hpp"
#include "E2EContextHelpers.hpp"

#include <clocale>

#include "api/twitch/TwitchService.hpp"
#include "chat/TwitchChatClient.hpp"
#include "core/Application.hpp"
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

BaseE2EContext::BaseE2EContext(QObject* parent)
    : QObject(parent)
{
}

BaseE2EContext::~BaseE2EContext() = default;

void BaseE2EContext::applicationAvailable()
{
    log("Initializing...");

    setupLocale();
    
    // Create fixture loader first
    m_fixtureLoader = std::make_unique<FixtureLoader>();
    if (!m_fixtureLoader->isValid()) {
        warn("Fixture loader failed to find fixtures directory");
    }

    startMockServers();
    loadFixtures();
    configureEnvironment();
    createApplication();

    onSetupComplete();

    log("Setup complete");
}

void BaseE2EContext::qmlEngineAvailable(QQmlEngine* engine)
{
    log("Configuring QML engine...");

    registerQmlTypes();
    configureImportPaths(engine);
    exposeServices(engine);

    onQmlEngineConfigured(engine);

    log("QML engine configured");
}

void BaseE2EContext::cleanupTestCase()
{
    log("Cleaning up...");

    if (m_twitchServer) {
        log(QString("MockTwitchServer handled %1 requests").arg(m_twitchServer->requestCount()));
        m_twitchServer->stop();
        m_twitchServer.reset();
    }

    if (m_hlsServer) {
        log(QString("MockHlsServer handled %1 requests").arg(m_hlsServer->requestCount()));
        m_hlsServer->stop();
        m_hlsServer.reset();
    }

    if (m_ircServer) {
        log(QString("MockIrcServer handled %1 requests").arg(m_ircServer->requestCount()));
        m_ircServer->stop();
        m_ircServer.reset();
    }

    log("Cleanup complete");
}

// ============================================================================
// Private Implementation
// ============================================================================

void BaseE2EContext::setupLocale()
{
    // Required for mpv to work correctly
    std::setlocale(LC_NUMERIC, "C");
}

void BaseE2EContext::startMockServers()
{
    log("Starting mock servers...");

    m_twitchServer = std::make_unique<MockTwitchServer>();
    m_hlsServer = std::make_unique<MockHlsServer>();

    if (!m_twitchServer->start(static_cast<quint16>(m_twitchPort))) {
        warn("Failed to start MockTwitchServer");
        return;
    }
    m_twitchPort = m_twitchServer->port();
    log(QString("MockTwitchServer on port %1").arg(m_twitchPort));

    if (!m_hlsServer->start(static_cast<quint16>(m_hlsPort))) {
        warn("Failed to start MockHlsServer");
        return;
    }
    m_hlsPort = m_hlsServer->port();
    log(QString("MockHlsServer on port %1").arg(m_hlsPort));

    m_twitchServer->setHlsServerUrl(m_hlsServer->baseUrl());

    // Load test video segment
    if (m_fixtureLoader && m_fixtureLoader->exists("test_segment.ts")) {
        m_hlsServer->loadSegmentFromFile(m_fixtureLoader->fullPath("test_segment.ts"));
    } else {
        warn("test_segment.ts not found, using minimal segment");
    }

    // Start MockIrcServer for chat testing
    m_ircServer = std::make_unique<MockIrcServer>();
    if (!m_ircServer->start()) {
        warn("Failed to start MockIrcServer");
    } else {
        log(QString("MockIrcServer on port %1").arg(m_ircServer->port()));
    }
}

void BaseE2EContext::loadFixtures()
{
    if (!m_fixtureLoader || !m_fixtureLoader->isValid()) {
        return;
    }

    log("Loading fixtures...");

    // Load auth_token.json
    auto authToken = m_fixtureLoader->loadJsonObject("auth_token.json");
    if (authToken) {
        m_accessToken = (*authToken)["access_token"].toString();
        m_refreshToken = (*authToken)["refresh_token"].toString();
        m_twitchServer->setValidToken(m_accessToken);
        log("Auth token loaded");
    }

    // Load streams.json
    auto streams = m_fixtureLoader->loadJsonArray("streams.json");
    if (streams) {
        m_twitchServer->setStreams(*streams);
        log(QString("Loaded %1 streams").arg(streams->size()));

        // Add channels to HLS server
        for (const QJsonValue& streamVal : *streams) {
            QJsonObject stream = streamVal.toObject();
            QString userLogin = stream["user_login"].toString();
            if (!userLogin.isEmpty()) {
                m_hlsServer->addChannel(userLogin);
            }
        }
    }

    // Load users.json
    auto users = m_fixtureLoader->loadJsonArray("users.json");
    if (users) {
        m_twitchServer->setUsers(*users);
        log(QString("Loaded %1 users").arg(users->size()));
    }

    // Load search_results.json (optional)
    auto searchResults = m_fixtureLoader->loadJsonArray("search_results.json");
    if (searchResults) {
        m_twitchServer->setSearchResults(*searchResults);
        log(QString("Loaded %1 search results").arg(searchResults->size()));
    }

    // Load videos.json (optional)
    auto videos = m_fixtureLoader->loadJsonArray("videos.json");
    if (videos) {
        m_twitchServer->setVideos(*videos);
        log(QString("Loaded %1 videos").arg(videos->size()));
    }
}

void BaseE2EContext::configureEnvironment()
{
    qputenv("BLUEPLAYER_API_URL", m_twitchServer->baseUrl().toUtf8());
    qputenv("BLUEPLAYER_HLS_PROXY_URL", m_hlsServer->baseUrl().toUtf8());
    
    // Set IRC mock server URL for chat testing
    if (m_ircServer) {
        qputenv("BLUEPLAYER_IRC_URL", m_ircServer->url().toUtf8());
    }

    if (qgetenv("TWITCH_CLIENT_ID").isEmpty()) {
        qputenv("TWITCH_CLIENT_ID", "e2e_test_client_id");
    }

    // Load Config
    blueplayer::core::Config::instance().load();
}

void BaseE2EContext::createApplication()
{
    m_mockStorage = std::make_unique<MockSecureStorage>();

    // Inject credentials if needed
    if (shouldInjectCredentials() && !m_accessToken.isEmpty()) {
        QString clientId = QString::fromUtf8(qgetenv("TWITCH_CLIENT_ID"));
        m_mockStorage->setCredentials(m_accessToken, m_refreshToken, clientId, 14400);
        log("Credentials injected");
    } else {
        log("No credentials injected (unauthenticated mode)");
    }

    // Create Application
    m_coreApp = std::make_unique<blueplayer::core::Application>(m_mockStorage.get());
    m_coreApp->initialize();

    // Clear VOD cache to ensure clean state for each test run
    if (m_coreApp->cacheManager()) {
        int cleared = m_coreApp->cacheManager()->clearAllVods();
        if (cleared > 0) {
            log(QString("Cleared %1 stale VODs from previous test runs").arg(cleared));
        }
    }

    if (m_coreApp->twitchService()) {
        log(QString("Ready, authenticated: %1")
            .arg(m_coreApp->twitchService()->isAuthenticated() ? "true" : "false"));
    }
}

void BaseE2EContext::registerQmlTypes()
{
    qmlRegisterType<blueplayer::media::MpvQuickItem>("BluePlayer.Media", 1, 0, "MpvQuickItem");
    qmlRegisterType<blueplayer::api::twitch::TwitchService>("BluePlayer.Twitch", 1, 0, "TwitchService");
    qmlRegisterType<blueplayer::ui::HomeViewModel>("BluePlayer.UI", 1, 0, "HomeViewModel");
    qmlRegisterType<blueplayer::ui::CacheManagerViewModel>("BluePlayer.UI", 1, 0, "CacheManagerViewModel");
    qmlRegisterType<BluePlayer::TwitchChatClient>("BluePlayer.Chat", 1, 0, "TwitchChatClient");
}

void BaseE2EContext::configureImportPaths(QQmlEngine* engine)
{
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

    // Set context property for QML path
    engine->rootContext()->setContextProperty("E2E_QML_PATH", uiPath);
    E2EContextHelpers::registerCommonContextProperties(engine);
}

void BaseE2EContext::exposeServices(QQmlEngine* engine)
{
    if (m_coreApp) {
        engine->rootContext()->setContextProperty("twitchService",
            qobject_cast<QObject*>(m_coreApp->twitchService()));
        engine->rootContext()->setContextProperty("cacheManager",
            qobject_cast<QObject*>(m_coreApp->cacheManager()));
    }
    
    // Expose mock IRC server for chat testing
    if (m_ircServer) {
        engine->rootContext()->setContextProperty("mockIrcServer",
            qobject_cast<QObject*>(m_ircServer.get()));
    }
    
    // Expose fixtures path for tests that need to reference fixture files
    if (m_fixtureLoader && m_fixtureLoader->isValid()) {
        engine->rootContext()->setContextProperty("E2E_FIXTURES_PATH",
            m_fixtureLoader->fixturesPath());
    }
}

void BaseE2EContext::log(const QString& message) const
{
    qInfo().noquote() << QString("[E2E Context: %1] %2").arg(contextName(), message);
}

void BaseE2EContext::warn(const QString& message) const
{
    qWarning().noquote() << QString("[E2E Context: %1] %2").arg(contextName(), message);
}

} // namespace blueplayer::test::e2e
