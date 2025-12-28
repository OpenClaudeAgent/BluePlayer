#pragma once

#include "servers/MockHlsServer.hpp"
#include "servers/MockTwitchServer.hpp"

#include <QObject>
#include <QQmlEngine>
#include <memory>

/**
 * @brief Setup class for Qt Quick Tests
 * 
 * This class is called by QUICK_TEST_MAIN_WITH_SETUP to:
 * 1. Start mock servers before tests run
 * 2. Configure QQmlEngine with proper import paths
 * 3. Clean up after tests complete
 */
class Setup : public QObject
{
    Q_OBJECT

public:
    Setup();
    ~Setup();

public slots:
    /**
     * Called after QApplication is available but before QQmlEngine.
     * Used to start mock servers and set environment variables.
     */
    void applicationAvailable();

    /**
     * Called when QQmlEngine is ready.
     * Used to add import paths for our QML modules.
     */
    void qmlEngineAvailable(QQmlEngine* engine);

    /**
     * Called after all tests complete.
     * Used to stop mock servers and clean up.
     */
    void cleanupTestCase();

private:
    std::unique_ptr<blueplayer::test::e2e::MockTwitchServer> m_twitchServer;
    std::unique_ptr<blueplayer::test::e2e::MockHlsServer> m_hlsServer;
    
    int m_twitchPort = 8080;
    int m_hlsPort = 8081;
};
