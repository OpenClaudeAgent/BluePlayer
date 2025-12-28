#include "core/Config.hpp"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>

namespace blueplayer::core {

Config& Config::instance() {
  static Config instance;
  return instance;
}

void Config::load() {
  loadFromEnvironment();
  
  // Essayer de charger depuis un fichier de configuration si disponible
  const QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + 
                             "/BluePlayer/config.json";
  if (QFile::exists(configPath)) {
    loadFromFile(configPath);
  }
}

void Config::loadFromEnvironment() {
  // Twitch Configuration
  m_twitchClientId = getEnvVar("TWITCH_CLIENT_ID");
  m_twitchClientSecret = getEnvVar("TWITCH_CLIENT_SECRET");
  m_twitchRedirectUri = getEnvVar("TWITCH_REDIRECT_URI");
  
  const QString redirectPortStr = getEnvVar("TWITCH_REDIRECT_PORT");
  if (!redirectPortStr.isEmpty()) {
    bool ok = false;
    const int port = redirectPortStr.toInt(&ok);
    if (ok && port > 0 && port < 65536) {
      m_twitchRedirectPort = static_cast<quint16>(port);
    }
  }
  
  m_twitchTlsCertPath = getEnvVar("TWITCH_TLS_CERT_PATH");
  m_twitchTlsKeyPath = getEnvVar("TWITCH_TLS_KEY_PATH");

  // FFmpeg Configuration
  m_ffmpegDir = getEnvVar("FFmpeg_DIR");
  const QString ffmpegIncludeDirs = getEnvVar("FFMPEG_INCLUDE_DIRS");
  if (!ffmpegIncludeDirs.isEmpty()) {
    m_ffmpegIncludeDirs = ffmpegIncludeDirs.split(":", Qt::SkipEmptyParts);
  }
  const QString ffmpegLibraryDirs = getEnvVar("FFMPEG_LIBRARY_DIRS");
  if (!ffmpegLibraryDirs.isEmpty()) {
    m_ffmpegLibraryDirs = ffmpegLibraryDirs.split(":", Qt::SkipEmptyParts);
  }

  // Qt Configuration
  m_qt6Dir = getEnvVar("Qt6_DIR");

  // Network Configuration
  const QString cacheSizeStr = getEnvVar("BLUEPLAYER_NETWORK_CACHE_SIZE");
  if (!cacheSizeStr.isEmpty()) {
    bool ok = false;
    const int size = cacheSizeStr.toInt(&ok);
    if (ok && size > 0) {
      m_networkCacheSize = size;
    }
  }

  const QString cacheTTLStr = getEnvVar("BLUEPLAYER_NETWORK_CACHE_TTL");
  if (!cacheTTLStr.isEmpty()) {
    bool ok = false;
    const int ttl = cacheTTLStr.toInt(&ok);
    if (ok && ttl > 0) {
      m_networkCacheTTL = ttl;
    }
  }

  // Logging Configuration
  m_logLevels["Media"] = getEnvVar("BLUEPLAYER_LOG_MEDIA", "info");
  m_logLevels["Twitch"] = getEnvVar("BLUEPLAYER_LOG_TWITCH", "info");
  m_logLevels["UI"] = getEnvVar("BLUEPLAYER_LOG_UI", "warning");
  m_logLevels["Core"] = getEnvVar("BLUEPLAYER_LOG_CORE", "info");
  m_logLevels["Network"] = getEnvVar("BLUEPLAYER_LOG_NETWORK", "info");
}

void Config::loadFromFile(const QString& configPath) {
  QFile file(configPath);
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  const QByteArray data = file.readAll();
  const QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isObject()) {
    return;
  }

  const QJsonObject root = doc.object();

  // Twitch
  if (root.contains("twitch")) {
    const QJsonObject twitch = root["twitch"].toObject();
    if (twitch.contains("clientId")) {
      m_twitchClientId = twitch["clientId"].toString();
    }
    if (twitch.contains("clientSecret")) {
      m_twitchClientSecret = twitch["clientSecret"].toString();
    }
    if (twitch.contains("redirectUri")) {
      m_twitchRedirectUri = twitch["redirectUri"].toString();
    }
    if (twitch.contains("redirectPort")) {
      m_twitchRedirectPort = static_cast<quint16>(twitch["redirectPort"].toInt());
    }
    if (twitch.contains("tlsCertPath")) {
      m_twitchTlsCertPath = twitch["tlsCertPath"].toString();
    }
    if (twitch.contains("tlsKeyPath")) {
      m_twitchTlsKeyPath = twitch["tlsKeyPath"].toString();
    }
  }

  // Network
  if (root.contains("network")) {
    const QJsonObject network = root["network"].toObject();
    if (network.contains("cacheSize")) {
      m_networkCacheSize = network["cacheSize"].toInt();
    }
    if (network.contains("cacheTTL")) {
      m_networkCacheTTL = network["cacheTTL"].toInt();
    }
  }

  // Logging
  if (root.contains("logging")) {
    const QJsonObject logging = root["logging"].toObject();
    for (auto it = logging.begin(); it != logging.end(); ++it) {
      m_logLevels[it.key()] = it.value().toString();
    }
  }
}

QString Config::getEnvVar(const QString& key, const QString& defaultValue) const {
  const QByteArray value = qgetenv(key.toUtf8().constData());
  if (value.isEmpty()) {
    return defaultValue;
  }
  return QString::fromUtf8(value);
}

QString Config::logLevel(const QString& category) const {
  return m_logLevels.value(category, "info");
}

QString Config::defaultQuality() const {
  QSettings settings(QStringLiteral("BluePlayer"), QStringLiteral("BluePlayer"));
  return settings.value(QStringLiteral("playback/defaultQuality"), QStringLiteral("Auto")).toString();
}

void Config::setDefaultQuality(const QString& quality) {
  QSettings settings(QStringLiteral("BluePlayer"), QStringLiteral("BluePlayer"));
  settings.setValue(QStringLiteral("playback/defaultQuality"), quality);
  settings.sync();
}

}  // namespace blueplayer::core












