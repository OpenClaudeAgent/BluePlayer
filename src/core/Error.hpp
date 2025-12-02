#pragma once

#include <QString>

namespace blueplayer::core {

/**
 * @brief Codes d'erreur standardisés pour BluePlayer
 */
enum class ErrorCode {
  // Erreurs générales
  Unknown = 0,
  InvalidArgument = 1,
  InvalidState = 2,
  NotInitialized = 3,

  // Erreurs réseau
  NetworkError = 100,
  NetworkTimeout = 101,
  NetworkConnectionRefused = 102,
  InvalidResponse = 103,

  // Erreurs Twitch API
  TwitchNotAuthenticated = 200,
  TwitchInvalidToken = 201,
  TwitchApiError = 202,
  TwitchRateLimitExceeded = 203,

  // Erreurs média
  MediaFileNotFound = 300,
  MediaFormatNotSupported = 301,
  MediaDecodeError = 302,
  MediaDeviceError = 303,

  // Erreurs de configuration
  ConfigNotFound = 400,
  ConfigInvalid = 401
};

/**
 * @brief Classe représentant une erreur dans BluePlayer
 * 
 * Fournit un code d'erreur standardisé, un message localisé et un contexte optionnel.
 */
class Error {
public:
  /**
   * @brief Constructeur par défaut (erreur inconnue)
   */
  Error();

  /**
   * @brief Constructeur avec code d'erreur
   * @param code Le code d'erreur
   */
  explicit Error(ErrorCode code);

  /**
   * @brief Constructeur avec code et message personnalisé
   * @param code Le code d'erreur
   * @param message Le message d'erreur
   */
  Error(ErrorCode code, const QString& message);

  /**
   * @brief Constructeur avec code, message et contexte
   * @param code Le code d'erreur
   * @param message Le message d'erreur
   * @param context Le contexte additionnel
   */
  Error(ErrorCode code, const QString& message, const QString& context);

  /**
   * @brief Obtient le code d'erreur
   * @return Le code d'erreur
   */
  [[nodiscard]] ErrorCode code() const { return m_code; }

  /**
   * @brief Obtient le message d'erreur
   * @return Le message d'erreur
   */
  [[nodiscard]] QString message() const { return m_message; }

  /**
   * @brief Obtient le contexte additionnel
   * @return Le contexte
   */
  [[nodiscard]] QString context() const { return m_context; }

  /**
   * @brief Obtient une représentation complète de l'erreur
   * @return Le message complet avec contexte
   */
  [[nodiscard]] QString toString() const;

  /**
   * @brief Vérifie si l'erreur est valide (non vide)
   * @return true si l'erreur est valide
   */
  [[nodiscard]] bool isValid() const { return m_code != ErrorCode::Unknown || !m_message.isEmpty(); }

  /**
   * @brief Obtient le message d'erreur localisé pour un code donné
   * @param code Le code d'erreur
   * @return Le message localisé
   */
  static QString localizedMessage(ErrorCode code);

  /**
   * @brief Convertit l'erreur en QString pour compatibilité QML
   * @return Le message d'erreur sous forme de QString
   */
  [[nodiscard]] QString toQString() const { return toString(); }

  /**
   * @brief Opérateur de conversion implicite vers QString pour compatibilité QML
   * @return Le message d'erreur sous forme de QString
   */
  operator QString() const { return toString(); }

private:
  ErrorCode m_code = ErrorCode::Unknown;
  QString m_message;
  QString m_context;
};

}  // namespace blueplayer::core

