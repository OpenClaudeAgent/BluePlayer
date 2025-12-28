#pragma once

#include <QHash>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <memory>

namespace blueplayer::core {

/**
 * @brief Classe de configuration centralisée pour BluePlayer
 * 
 * Gère la configuration depuis les variables d'environnement et les fichiers de config.
 * Fournit un accès unifié à toutes les configurations de l'application.
 */
class Config {
public:
  /**
   * @brief Obtient l'instance singleton de la configuration
   * @return Référence à l'instance de configuration
   */
  static Config& instance();

  /**
   * @brief Charge la configuration depuis les variables d'environnement et fichiers
   */
  void load();

  // Twitch API Configuration
  QString twitchClientId() const { return m_twitchClientId; }
  QString twitchClientSecret() const { return m_twitchClientSecret; }
  QString twitchRedirectUri() const { return m_twitchRedirectUri; }
  quint16 twitchRedirectPort() const { return m_twitchRedirectPort; }
  QString twitchTlsCertPath() const { return m_twitchTlsCertPath; }
  QString twitchTlsKeyPath() const { return m_twitchTlsKeyPath; }

  // FFmpeg Configuration
  QString ffmpegDir() const { return m_ffmpegDir; }
  QStringList ffmpegIncludeDirs() const { return m_ffmpegIncludeDirs; }
  QStringList ffmpegLibraryDirs() const { return m_ffmpegLibraryDirs; }

  // Qt Configuration
  QString qt6Dir() const { return m_qt6Dir; }

  // Network Configuration
  int networkCacheSize() const { return m_networkCacheSize; }
  int networkCacheTTL() const { return m_networkCacheTTL; }

  // Test Mode Configuration
  /**
   * @brief Vérifie si l'application est en mode test E2E
   * @return true si BLUEPLAYER_TEST_MODE=1
   */
  bool isTestMode() const { return m_testMode; }
  
  /**
   * @brief Retourne l'URL de base pour l'API Twitch (mock ou production)
   * @return URL comme "https://api.twitch.tv" ou "http://localhost:12345"
   */
  QString twitchApiBaseUrl() const { return m_twitchApiBaseUrl; }
  
  /**
   * @brief Retourne l'URL de base pour le serveur HLS (mock)
   * @return URL comme "http://localhost:12346" ou vide si pas en mode test
   */
  QString hlsServerBaseUrl() const { return m_hlsServerBaseUrl; }

  // Logging Configuration
  QString logLevel(const QString& category) const;

  // User Preferences (persisted via QSettings)
  /**
   * @brief Obtient la qualité de stream par défaut
   * @return Le nom de la qualité (ex: "Auto", "1080p60", "720p")
   */
  QString defaultQuality() const;
  
  /**
   * @brief Définit la qualité de stream par défaut
   * @param quality Le nom de la qualité à sauvegarder
   */
  void setDefaultQuality(const QString& quality);

private:
  Config() = default;
  ~Config() = default;
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  void loadFromEnvironment();
  void loadFromFile(const QString& configPath);
  QString getEnvVar(const QString& key, const QString& defaultValue = QString()) const;

  // Twitch
  QString m_twitchClientId;
  QString m_twitchClientSecret;
  QString m_twitchRedirectUri;
  quint16 m_twitchRedirectPort = 8443;
  QString m_twitchTlsCertPath;
  QString m_twitchTlsKeyPath;

  // FFmpeg
  QString m_ffmpegDir;
  QStringList m_ffmpegIncludeDirs;
  QStringList m_ffmpegLibraryDirs;

  // Qt
  QString m_qt6Dir;

  // Network
  int m_networkCacheSize = 50 * 1024 * 1024;  // 50 MB par défaut
  int m_networkCacheTTL = 300;  // 5 minutes par défaut

  // Logging
  QHash<QString, QString> m_logLevels;

  // Test Mode
  bool m_testMode = false;
  QString m_twitchApiBaseUrl = QStringLiteral("https://api.twitch.tv");
  QString m_hlsServerBaseUrl;
};

}  // namespace blueplayer::core

