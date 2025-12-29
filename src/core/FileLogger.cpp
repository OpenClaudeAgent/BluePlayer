#include "FileLogger.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>

#include <algorithm>

namespace blueplayer::core {

static QFile* g_logFile = nullptr;
static QTextStream* g_logStream = nullptr;
static QMutex g_logMutex;
static QString g_logPath;

static const char* messageTypeToString(QtMsgType type) {
    switch (type) {
        case QtDebugMsg:    return "DEBUG";
        case QtInfoMsg:     return "INFO";
        case QtWarningMsg:  return "WARN";
        case QtCriticalMsg: return "ERROR";
        case QtFatalMsg:    return "FATAL";
    }
    return "UNKNOWN";
}

static void fileMessageHandler(QtMsgType type, const QMessageLogContext& /*context*/, const QString& msg) {
    QMutexLocker locker(&g_logMutex);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString formattedMessage = QString("[%1] [%2] %3")
        .arg(timestamp)
        .arg(messageTypeToString(type))
        .arg(msg);
    
    // Write to stderr (console)
    fprintf(stderr, "%s\n", formattedMessage.toLocal8Bit().constData());
    
    // Write to log file
    if (g_logStream) {
        *g_logStream << formattedMessage << "\n";
        g_logStream->flush();
    }
    
    if (type == QtFatalMsg) {
        abort();
    }
}

void FileLogger::initialize() {
    // Guard against double initialization
    if (g_logFile) {
        return;
    }
    
    // Store logs in system cache directory (e.g., ~/Library/Caches/BluePlayer/logs)
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString logDir = cacheDir + "/logs";
    QDir().mkpath(logDir);
    
    // Cleanup old log files before creating new one
    int deletedCount = cleanupOldLogs();
    
    // Create log file with timestamp in name
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    g_logPath = QString("%1/blueplayer_%2.log").arg(logDir, timestamp);
    
    g_logFile = new QFile(g_logPath);
    if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        g_logStream = new QTextStream(g_logFile);
        
        // Write header
        *g_logStream << "================================================================================\n";
        *g_logStream << "BluePlayer Log - " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        *g_logStream << "================================================================================\n\n";
        g_logStream->flush();
        
        // Install custom message handler
        qInstallMessageHandler(fileMessageHandler);
        
        qInfo() << "[Core] FileLogger initialized:" << g_logPath;
        if (deletedCount > 0) {
            qInfo() << "[Core] Log cleanup: removed" << deletedCount << "old log files";
        }
    } else {
        fprintf(stderr, "[FileLogger] Warning: Could not create log file at %s\n", 
                g_logPath.toLocal8Bit().constData());
        g_logPath.clear();
    }
}

int FileLogger::cleanupOldLogs(int maxAgeDays, int maxFiles) {
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString logDir = cacheDir + "/logs";
    QDir dir(logDir);
    
    if (!dir.exists()) {
        return 0;
    }
    
    int deletedCount = 0;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime cutoffDate = now.addDays(-maxAgeDays);
    
    // Get all log files sorted by modification time (oldest first)
    QFileInfoList logFiles = dir.entryInfoList(
        QStringList() << "blueplayer_*.log",
        QDir::Files,
        QDir::Time | QDir::Reversed  // Oldest first
    );
    
    // First pass: delete files older than maxAgeDays
    QFileInfoList remainingFiles;
    for (const QFileInfo& fileInfo : logFiles) {
        if (fileInfo.lastModified() < cutoffDate) {
            if (QFile::remove(fileInfo.absoluteFilePath())) {
                deletedCount++;
            }
        } else {
            remainingFiles.append(fileInfo);
        }
    }
    
    // Second pass: keep only maxFiles most recent files
    // remainingFiles is sorted oldest first, so we delete from the beginning
    while (remainingFiles.size() > maxFiles) {
        const QFileInfo& oldest = remainingFiles.first();
        if (QFile::remove(oldest.absoluteFilePath())) {
            deletedCount++;
        }
        remainingFiles.removeFirst();
    }
    
    return deletedCount;
}

void FileLogger::shutdown() {
    if (!g_logFile) {
        return;
    }

    qInfo() << "[Core] FileLogger shutting down";
    
    // Restore default message handler
    qInstallMessageHandler(nullptr);
    
    if (g_logStream) {
        g_logStream->flush();
        delete g_logStream;
        g_logStream = nullptr;
    }
    
    if (g_logFile) {
        g_logFile->close();
        delete g_logFile;
        g_logFile = nullptr;
    }
    
    g_logPath.clear();
}

QString FileLogger::currentLogPath() {
    return g_logPath;
}

} // namespace blueplayer::core
