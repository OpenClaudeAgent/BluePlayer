#include "Setup.hpp"

#include "api/twitch/TwitchService.hpp"
#include "core/CacheManager.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlContext>

Setup::Setup() = default;
Setup::~Setup() = default;

void Setup::applicationAvailable()
{
    qInfo() << "[E2E Setup] Initializing E2E test environment...";
    
    // Initialize core Application (provides twitchService, cacheManager)
    m_coreApp = std::make_unique<blueplayer::core::Application>();
    m_coreApp->initialize();
    qInfo() << "[E2E Setup] Core Application initialized";

    qInfo() << "[E2E Setup] Starting mock servers...";

    // Create mock servers
    m_twitchServer = std::make_unique<blueplayer::test::e2e::MockTwitchServer>();
    m_hlsServer = std::make_unique<blueplayer::test::e2e::MockHlsServer>();

    // Start servers
    if (!m_twitchServer->start(static_cast<quint16>(m_twitchPort))) {
        qWarning() << "[E2E Setup] Failed to start MockTwitchServer on port" << m_twitchPort;
        return;
    }
    m_twitchPort = m_twitchServer->port();
    qInfo() << "[E2E Setup] MockTwitchServer started on port" << m_twitchPort;

    if (!m_hlsServer->start(static_cast<quint16>(m_hlsPort))) {
        qWarning() << "[E2E Setup] Failed to start MockHlsServer on port" << m_hlsPort;
        return;
    }
    m_hlsPort = m_hlsServer->port();
    qInfo() << "[E2E Setup] MockHlsServer started on port" << m_hlsPort;

    // Configure HLS server URL in Twitch server
    m_twitchServer->setHlsServerUrl(m_hlsServer->baseUrl());

    // Load fixtures
    QString fixturesPath = QDir::currentPath() + "/tests/e2e/fixtures";
    
    // Try alternative paths if not found
    if (!QDir(fixturesPath).exists()) {
        fixturesPath = QCoreApplication::applicationDirPath() + "/../../../tests/e2e/fixtures";
    }
    if (!QDir(fixturesPath).exists()) {
        // Fallback for running from build directory
        fixturesPath = QDir::currentPath() + "/../tests/e2e/fixtures";
    }

    qInfo() << "[E2E Setup] Loading fixtures from:" << fixturesPath;

    // Load streams.json
    QFile streamsFile(fixturesPath + "/streams.json");
    if (streamsFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(streamsFile.readAll());
        if (doc.isArray()) {
            m_twitchServer->setStreams(doc.array());
            qInfo() << "[E2E Setup] Loaded" << doc.array().size() << "streams";
            
            // Add HLS channels for each stream
            for (const QJsonValue& streamVal : doc.array()) {
                QJsonObject stream = streamVal.toObject();
                QString userLogin = stream["user_login"].toString();
                if (!userLogin.isEmpty()) {
                    m_hlsServer->addChannel(userLogin);
                }
            }
        }
    } else {
        qWarning() << "[E2E Setup] Could not load streams.json";
    }

    // Load users.json
    QFile usersFile(fixturesPath + "/users.json");
    if (usersFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(usersFile.readAll());
        if (doc.isArray()) {
            m_twitchServer->setUsers(doc.array());
            qInfo() << "[E2E Setup] Loaded" << doc.array().size() << "users";
        }
    } else {
        qWarning() << "[E2E Setup] Could not load users.json";
    }

    // Load auth_token.json
    QFile authFile(fixturesPath + "/auth_token.json");
    if (authFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(authFile.readAll());
        if (doc.isObject()) {
            QString token = doc.object()["access_token"].toString();
            m_twitchServer->setValidToken(token);
            qInfo() << "[E2E Setup] Loaded auth token";
        }
    } else {
        qWarning() << "[E2E Setup] Could not load auth_token.json";
    }

    // Set environment variables for the app to use mock servers
    qputenv("BLUEPLAYER_TEST_MODE", "1");
    qputenv("BLUEPLAYER_MOCK_API_URL", m_twitchServer->baseUrl().toUtf8());
    qputenv("BLUEPLAYER_MOCK_HLS_URL", m_hlsServer->baseUrl().toUtf8());

    qInfo() << "[E2E Setup] Environment configured:";
    qInfo() << "  BLUEPLAYER_TEST_MODE=1";
    qInfo() << "  BLUEPLAYER_MOCK_API_URL=" << m_twitchServer->baseUrl();
    qInfo() << "  BLUEPLAYER_MOCK_HLS_URL=" << m_hlsServer->baseUrl();
}

void Setup::qmlEngineAvailable(QQmlEngine* engine)
{
    qInfo() << "[E2E Setup] QML engine available, configuring import paths...";

    // Base path to source directory (from build/tests/e2e/)
    // Resolve to absolute path
    QString appDir = QCoreApplication::applicationDirPath();
    QDir srcDir(appDir + "/../../../src");
    QString srcPath = srcDir.absolutePath();
    QString uiPath = srcPath + "/ui";
    
    // Add import paths for QML modules
    engine->addImportPath(srcPath);
    engine->addImportPath(uiPath);
    engine->addImportPath(uiPath + "/components");
    engine->addImportPath(uiPath + "/themes");
    
    // Add build directory for generated files
    engine->addImportPath(appDir);
    engine->addImportPath(appDir + "/../src");

    // Expose the UI path as a context property for tests to use
    engine->rootContext()->setContextProperty("E2E_QML_PATH", uiPath);

    // Expose core services to QML (same as main.mm does)
    if (m_coreApp) {
        engine->rootContext()->setContextProperty("twitchService", 
            qobject_cast<QObject*>(m_coreApp->twitchService()));
        engine->rootContext()->setContextProperty("cacheManager", 
            qobject_cast<QObject*>(m_coreApp->cacheManager()));
        qInfo() << "[E2E Setup] Core services exposed to QML";
    }

    qInfo() << "[E2E Setup] Import paths configured:";
    qInfo() << "  - Source:" << srcPath;
    qInfo() << "  - UI:" << uiPath;
    qInfo() << "  - E2E_QML_PATH exposed to QML";
}

void Setup::cleanupTestCase()
{
    qInfo() << "[E2E Setup] Cleaning up...";

    if (m_twitchServer) {
        qInfo() << "[E2E Setup] MockTwitchServer handled" << m_twitchServer->requestCount() << "requests";
        m_twitchServer->stop();
        m_twitchServer.reset();
    }

    if (m_hlsServer) {
        qInfo() << "[E2E Setup] MockHlsServer handled" << m_hlsServer->requestCount() << "requests";
        m_hlsServer->stop();
        m_hlsServer.reset();
    }

    qInfo() << "[E2E Setup] Cleanup complete";
}
