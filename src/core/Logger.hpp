#pragma once

#include <QLoggingCategory>
#include <QString>

namespace blueplayer::core {

/**
 * @brief Catégories de logging pour organiser les logs par domaine
 */
class LogCategory {
public:
  static QLoggingCategory Media;
  static QLoggingCategory Twitch;
  static QLoggingCategory UI;
  static QLoggingCategory Core;
  static QLoggingCategory Network;
};

/**
 * @brief Niveaux de logging disponibles
 */
enum class LogLevel {
  Debug = 0,
  Info = 1,
  Warning = 2,
  Error = 3,
  Critical = 4
};

/**
 * @brief Classe utilitaire pour le logging structuré
 * 
 * Fournit des méthodes statiques pour logger avec des catégories et niveaux.
 * Remplace l'utilisation directe de qDebug(), qInfo(), etc.
 */
class Logger {
public:
  /**
   * @brief Initialise le système de logging
   * 
   * Configure les niveaux de log depuis les variables d'environnement ou
   * utilise les valeurs par défaut.
   */
  static void initialize();

  /**
   * @brief Configure le niveau de log pour une catégorie
   * @param category La catégorie à configurer
   * @param level Le niveau de log minimum
   */
  static void setLevel(QLoggingCategory& category, LogLevel level);

  /**
   * @brief Obtient le niveau de log depuis une variable d'environnement
   * @param envVar Le nom de la variable d'environnement
   * @param defaultLevel Le niveau par défaut si la variable n'est pas définie
   * @return Le niveau de log configuré
   */
  static LogLevel levelFromEnvironment(const QString& envVar, LogLevel defaultLevel);

  /**
   * @brief Log un message de debug
   * @param category La catégorie du log
   * @param message Le message à logger
   */
  static void debug(const QLoggingCategory& category, const QString& message);

  /**
   * @brief Log un message d'information
   * @param category La catégorie du log
   * @param message Le message à logger
   */
  static void info(const QLoggingCategory& category, const QString& message);

  /**
   * @brief Log un message d'avertissement
   * @param category La catégorie du log
   * @param message Le message à logger
   */
  static void warning(const QLoggingCategory& category, const QString& message);

  /**
   * @brief Log un message d'erreur
   * @param category La catégorie du log
   * @param message Le message à logger
   */
  static void error(const QLoggingCategory& category, const QString& message);

  /**
   * @brief Log un message critique
   * @param category La catégorie du log
   * @param message Le message à logger
   */
  static void critical(const QLoggingCategory& category, const QString& message);

private:
  static bool s_initialized;
};

// Macros de commodité pour simplifier l'utilisation
#define LOG_DEBUG(category, message) \
  blueplayer::core::Logger::debug(blueplayer::core::LogCategory::category, message)

#define LOG_INFO(category, message) \
  blueplayer::core::Logger::info(blueplayer::core::LogCategory::category, message)

#define LOG_WARNING(category, message) \
  blueplayer::core::Logger::warning(blueplayer::core::LogCategory::category, message)

#define LOG_ERROR(category, message) \
  blueplayer::core::Logger::error(blueplayer::core::LogCategory::category, message)

#define LOG_CRITICAL(category, message) \
  blueplayer::core::Logger::critical(blueplayer::core::LogCategory::category, message)

}  // namespace blueplayer::core




