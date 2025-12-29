#pragma once

#include "mocks/MockSecureStorage.hpp"
#include "servers/MockHlsServer.hpp"
#include "servers/MockTwitchServer.hpp"
#include "core/Application.hpp"

#include <QObject>
#include <QQmlEngine>
#include <memory>

namespace blueplayer::test::e2e {

/**
 * @brief Setup context for authenticated user tests
 * 
 * This context:
 * 1. Starts mock servers (Twitch API + HLS)
 * 2. Loads fixtures (auth token, streams, users)
 * 3. Injects credentials into MockSecureStorage
 * 4. Initializes Application as authenticated user
 * 
 * Use this context for flows that require authentication:
 * - Open stream, search, home view, etc.
 */
class AuthenticatedSetup : public QObject
{
    Q_OBJECT

public:
    AuthenticatedSetup();
    ~AuthenticatedSetup();

public slots:
    void applicationAvailable();
    void qmlEngineAvailable(QQmlEngine* engine);
    void cleanupTestCase();

private:
    std::unique_ptr<MockSecureStorage> m_mockStorage;
    std::unique_ptr<MockTwitchServer> m_twitchServer;
    std::unique_ptr<MockHlsServer> m_hlsServer;
    std::unique_ptr<blueplayer::core::Application> m_coreApp;
    
    int m_twitchPort = 8080;
    int m_hlsPort = 8081;
};

} // namespace blueplayer::test::e2e
