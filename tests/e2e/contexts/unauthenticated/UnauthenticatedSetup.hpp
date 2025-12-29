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
 * @brief Setup context for unauthenticated user tests
 * 
 * This context:
 * 1. Starts mock servers (Twitch API + HLS)
 * 2. Does NOT inject credentials
 * 3. Initializes Application as unauthenticated user
 * 
 * Use this context for flows that don't require authentication:
 * - Login view, onboarding, etc.
 */
class UnauthenticatedSetup : public QObject
{
    Q_OBJECT

public:
    UnauthenticatedSetup();
    ~UnauthenticatedSetup();

public slots:
    void applicationAvailable();
    void qmlEngineAvailable(QQmlEngine* engine);
    void cleanupTestCase();

private:
    std::unique_ptr<MockSecureStorage> m_mockStorage;
    std::unique_ptr<MockTwitchServer> m_twitchServer;
    std::unique_ptr<MockHlsServer> m_hlsServer;
    std::unique_ptr<blueplayer::core::Application> m_coreApp;
    
    int m_twitchPort = 8082;  // Different ports to avoid conflicts
    int m_hlsPort = 8083;
};

} // namespace blueplayer::test::e2e
