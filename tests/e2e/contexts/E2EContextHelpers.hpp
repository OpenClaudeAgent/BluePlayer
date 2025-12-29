#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QQmlContext>
#include <QQmlEngine>
#include <QString>

namespace blueplayer::test::e2e {

/**
 * @brief Common helpers for E2E test contexts
 * 
 * Provides shared functionality for all E2E contexts (authenticated, unauthenticated, etc.)
 */
namespace E2EContextHelpers {

/**
 * @brief Get the screenshots directory path
 * 
 * Returns a path inside the build directory for storing E2E screenshots.
 * Creates the directory if it doesn't exist.
 */
inline QString getScreenshotsPath()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString screenshotsPath = appDir + "/e2e_screenshots";
    QDir().mkpath(screenshotsPath);
    return screenshotsPath;
}

/**
 * @brief Register common context properties for E2E tests
 * 
 * Sets up QML context properties shared by all E2E contexts:
 * - E2E_SCREENSHOT_DIR: Directory for test screenshots
 */
inline void registerCommonContextProperties(QQmlEngine* engine)
{
    engine->rootContext()->setContextProperty("E2E_SCREENSHOT_DIR", getScreenshotsPath());
}

} // namespace E2EContextHelpers
} // namespace blueplayer::test::e2e
