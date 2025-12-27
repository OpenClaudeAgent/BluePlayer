#pragma once

#include <QString>

namespace blueplayer::core {

/**
 * @brief File logging system for BluePlayer
 * 
 * Redirects all Qt log messages (qDebug, qWarning, qInfo, qCritical)
 * to both console and a timestamped log file.
 * 
 * Log files are created in the logs/ directory with format:
 * blueplayer_YYYY-MM-DD_HH-MM-SS.log
 * 
 * Each log line has format:
 * [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] message
 */
class FileLogger {
public:
    /**
     * @brief Initialize the file logging system
     * 
     * Creates a new log file and installs the Qt message handler.
     * Should be called once at application startup after QApplication is created.
     */
    static void initialize();

    /**
     * @brief Shutdown the file logging system
     * 
     * Flushes and closes the log file, releases resources.
     * Should be called before application exit.
     */
    static void shutdown();

    /**
     * @brief Get the path to the current log file
     * @return Path to the log file, or empty string if not initialized
     */
    static QString currentLogPath();

private:
    FileLogger() = default;
};

} // namespace blueplayer::core
