#include "FileLogger.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>

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
    // Store logs in system cache directory (e.g., ~/Library/Caches/BluePlayer/logs)
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QString logDir = cacheDir + "/logs";
    QDir().mkpath(logDir);
    
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
        
        qInfo() << "FileLogger initialized:" << g_logPath;
    } else {
        fprintf(stderr, "[FileLogger] Warning: Could not create log file at %s\n", 
                g_logPath.toLocal8Bit().constData());
        g_logPath.clear();
    }
}

void FileLogger::shutdown() {
    if (!g_logFile) {
        return;
    }

    qInfo() << "FileLogger shutting down";
    
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
