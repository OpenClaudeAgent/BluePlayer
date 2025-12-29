#ifndef BLUEPLAYER_TEST_E2E_BASEE2ECONTEXT_HPP
#define BLUEPLAYER_TEST_E2E_BASEE2ECONTEXT_HPP

#include "../fixtures/FixtureLoader.hpp"
#include "../mocks/MockSecureStorage.hpp"
#include "../servers/MockHlsServer.hpp"
#include "../servers/MockIrcServer.hpp"
#include "../servers/MockTwitchServer.hpp"

#include <QQmlEngine>

#include <memory>

namespace blueplayer::core {
class Application;
}

namespace blueplayer::test::e2e {

/**
 * @brief Base class for E2E test contexts.
 *
 * Provides common setup functionality for E2E tests:
 * - Mock server lifecycle (Twitch API, HLS)
 * - Fixture loading
 * - Environment configuration
 * - QML type registration
 * - Service exposure to QML
 *
 * Derived classes only need to implement:
 * - shouldInjectCredentials() - whether to authenticate
 * - contextName() - for logging
 *
 * Usage with Qt Quick Test:
 * @code
 * class AuthenticatedSetup : public BaseE2EContext
 * {
 * protected:
 *     QString contextName() const override { return "Authenticated"; }
 *     bool shouldInjectCredentials() const override { return true; }
 * };
 * QUICK_TEST_MAIN_WITH_SETUP(e2e_test, AuthenticatedSetup)
 * @endcode
 */
class BaseE2EContext : public QObject
{
    Q_OBJECT

public:
    explicit BaseE2EContext(QObject* parent = nullptr);
    ~BaseE2EContext() override;

public Q_SLOTS:
    /**
     * @brief Called before QML engine is created.
     * Sets up mock servers, fixtures, and creates the Application.
     */
    void applicationAvailable();

    /**
     * @brief Called after QML engine is created.
     * Registers types and exposes services to QML.
     */
    void qmlEngineAvailable(QQmlEngine* engine);

    /**
     * @brief Called after tests complete.
     * Cleans up resources.
     */
    void cleanupTestCase();

protected:
    /**
     * @brief Whether to inject authentication credentials.
     * Override in derived classes.
     */
    virtual bool shouldInjectCredentials() const = 0;

    /**
     * @brief Get the context name for logging.
     * Override in derived classes.
     */
    virtual QString contextName() const = 0;

    /**
     * @brief Hook for derived classes to perform additional setup.
     * Called after base setup is complete.
     */
    virtual void onSetupComplete() {}

    /**
     * @brief Hook for derived classes to perform additional QML setup.
     * Called after base QML configuration is complete.
     */
    virtual void onQmlEngineConfigured(QQmlEngine* engine) { Q_UNUSED(engine) }

    // Accessors for derived classes
    MockTwitchServer* twitchServer() const { return m_twitchServer.get(); }
    MockHlsServer* hlsServer() const { return m_hlsServer.get(); }
    E2E::MockIrcServer* ircServer() const { return m_ircServer.get(); }
    FixtureLoader* fixtureLoader() const { return m_fixtureLoader.get(); }
    blueplayer::core::Application* application() const { return m_coreApp.get(); }

private:
    void setupLocale();
    void startMockServers();
    void loadFixtures();
    void configureEnvironment();
    void createApplication();
    void registerQmlTypes();
    void configureImportPaths(QQmlEngine* engine);
    void exposeServices(QQmlEngine* engine);

    void log(const QString& message) const;
    void warn(const QString& message) const;

    // Mock servers
    std::unique_ptr<MockTwitchServer> m_twitchServer;
    std::unique_ptr<MockHlsServer> m_hlsServer;
    std::unique_ptr<E2E::MockIrcServer> m_ircServer;
    int m_twitchPort = 0;
    int m_hlsPort = 0;

    // Fixtures
    std::unique_ptr<FixtureLoader> m_fixtureLoader;

    // Mock storage
    std::unique_ptr<MockSecureStorage> m_mockStorage;

    // Application
    std::unique_ptr<blueplayer::core::Application> m_coreApp;

    // Credentials (loaded from fixtures)
    QString m_accessToken;
    QString m_refreshToken;
};

} // namespace blueplayer::test::e2e

#endif // BLUEPLAYER_TEST_E2E_BASEE2ECONTEXT_HPP
