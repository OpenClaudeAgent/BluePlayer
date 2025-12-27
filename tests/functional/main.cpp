/**
 * @file main.cpp
 * @brief Entry point for Qt Quick functional UI tests
 *
 * This file sets up the Qt Quick Test environment for running QML-based
 * functional tests. It configures import paths so that tests can find
 * the application's QML components.
 *
 * Usage:
 *   ./test_functional_ui                    # Run all tests
 *   ./test_functional_ui -functions         # List all test functions
 *   ./test_functional_ui TestName::test_x   # Run specific test
 */

#include <QtQuickTest>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDir>
#include <QDebug>

/**
 * @brief Setup class for configuring the QML test environment
 *
 * This class is instantiated before tests run and provides a hook
 * to configure the QML engine with necessary import paths and
 * context properties (like mock services).
 */
class Setup : public QObject
{
    Q_OBJECT

public:
    Setup() = default;

public slots:
    /**
     * @brief Called when the QML engine is available
     * @param engine The QML engine instance used by tests
     *
     * Use this method to:
     * - Add import paths for your QML components
     * - Set context properties for mock services
     * - Configure any global test settings
     */
    void qmlEngineAvailable(QQmlEngine* engine)
    {
        // Get the source directory from compile-time definition
        QString sourceDir = QStringLiteral(QUICK_TEST_SOURCE_DIR);
        QString projectRoot = QDir(sourceDir).absolutePath() + "/../..";

        // Add import paths for application QML components
        QStringList importPaths = {
            projectRoot + "/src/ui",
            projectRoot + "/src/ui/components",
            projectRoot + "/src/ui/themes"
        };

        for (const QString& path : importPaths) {
            QString absolutePath = QDir(path).absolutePath();
            if (QDir(absolutePath).exists()) {
                engine->addImportPath(absolutePath);
                qDebug() << "Added QML import path:" << absolutePath;
            } else {
                qWarning() << "QML import path does not exist:" << absolutePath;
            }
        }

        // Log all import paths for debugging
        qDebug() << "All QML import paths:" << engine->importPathList();

        // Here you can inject mock services if needed:
        // engine->rootContext()->setContextProperty("mockTwitchService", &m_mockTwitchService);
    }

    /**
     * @brief Called after all tests have completed
     *
     * Use this for any global cleanup if needed.
     */
    void cleanupTestCase()
    {
        qDebug() << "Functional UI tests completed";
    }

private:
    // Add mock service members here if needed
    // MockTwitchService m_mockTwitchService;
};

// Main entry point - runs all tst_*.qml files in QUICK_TEST_SOURCE_DIR
QUICK_TEST_MAIN_WITH_SETUP(FunctionalUITests, Setup)

#include "main.moc"
