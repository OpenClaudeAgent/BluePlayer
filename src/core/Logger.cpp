#include "core/Logger.hpp"

#include <QLoggingCategory>
#include <QString>
#include <QDebug>

namespace blueplayer::core {

// Définition des catégories de logging
QLoggingCategory LogCategory::Media("blueplayer.media");
QLoggingCategory LogCategory::Twitch("blueplayer.twitch");
QLoggingCategory LogCategory::UI("blueplayer.ui");
QLoggingCategory LogCategory::Core("blueplayer.core");
QLoggingCategory LogCategory::Network("blueplayer.network");

bool Logger::s_initialized = false;

void Logger::initialize() {
  if (s_initialized) {
    return;
  }

  // Configurer les niveaux depuis les variables d'environnement
  setLevel(LogCategory::Media, levelFromEnvironment("BLUEPLAYER_LOG_MEDIA", LogLevel::Info));
  setLevel(LogCategory::Twitch, levelFromEnvironment("BLUEPLAYER_LOG_TWITCH", LogLevel::Info));
  setLevel(LogCategory::UI, levelFromEnvironment("BLUEPLAYER_LOG_UI", LogLevel::Warning));
  setLevel(LogCategory::Core, levelFromEnvironment("BLUEPLAYER_LOG_CORE", LogLevel::Info));
  setLevel(LogCategory::Network, levelFromEnvironment("BLUEPLAYER_LOG_NETWORK", LogLevel::Info));

  s_initialized = true;
}

void Logger::setLevel(QLoggingCategory& category, LogLevel level) {
  switch (level) {
    case LogLevel::Debug:
      category.setEnabled(QtDebugMsg, true);
      category.setEnabled(QtInfoMsg, true);
      category.setEnabled(QtWarningMsg, true);
      category.setEnabled(QtCriticalMsg, true);
      break;
    case LogLevel::Info:
      category.setEnabled(QtDebugMsg, false);
      category.setEnabled(QtInfoMsg, true);
      category.setEnabled(QtWarningMsg, true);
      category.setEnabled(QtCriticalMsg, true);
      break;
    case LogLevel::Warning:
      category.setEnabled(QtDebugMsg, false);
      category.setEnabled(QtInfoMsg, false);
      category.setEnabled(QtWarningMsg, true);
      category.setEnabled(QtCriticalMsg, true);
      break;
    case LogLevel::Error:
      category.setEnabled(QtDebugMsg, false);
      category.setEnabled(QtInfoMsg, false);
      category.setEnabled(QtWarningMsg, false);
      category.setEnabled(QtCriticalMsg, true);
      break;
    case LogLevel::Critical:
      category.setEnabled(QtDebugMsg, false);
      category.setEnabled(QtInfoMsg, false);
      category.setEnabled(QtWarningMsg, false);
      category.setEnabled(QtCriticalMsg, true);
      break;
  }
}

LogLevel Logger::levelFromEnvironment(const QString& envVar, LogLevel defaultLevel) {
  const QByteArray value = qgetenv(envVar.toUtf8().constData());
  if (value.isEmpty()) {
    return defaultLevel;
  }

  const QString levelStr = QString::fromUtf8(value).toLower();
  if (levelStr == "debug") {
    return LogLevel::Debug;
  } else if (levelStr == "info") {
    return LogLevel::Info;
  } else if (levelStr == "warning") {
    return LogLevel::Warning;
  } else if (levelStr == "error") {
    return LogLevel::Error;
  } else if (levelStr == "critical") {
    return LogLevel::Critical;
  }

  return defaultLevel;
}

void Logger::debug(const QLoggingCategory& category, const QString& message) {
  if (category.isDebugEnabled()) {
    qDebug() << category.categoryName() << message;
  }
}

void Logger::info(const QLoggingCategory& category, const QString& message) {
  if (category.isInfoEnabled()) {
    qInfo() << category.categoryName() << message;
  }
}

void Logger::warning(const QLoggingCategory& category, const QString& message) {
  if (category.isWarningEnabled()) {
    qWarning() << category.categoryName() << message;
  }
}

void Logger::error(const QLoggingCategory& category, const QString& message) {
  if (category.isCriticalEnabled()) {
    qCritical() << category.categoryName() << message;
  }
}

void Logger::critical(const QLoggingCategory& category, const QString& message) {
  if (category.isCriticalEnabled()) {
    qCritical() << category.categoryName() << message;
  }
}

}  // namespace blueplayer::core

