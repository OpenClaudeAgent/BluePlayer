#include "core/Error.hpp"

namespace blueplayer::core {

Error::Error() : m_code(ErrorCode::Unknown) {}

Error::Error(ErrorCode code) : m_code(code), m_message(localizedMessage(code)) {}

Error::Error(ErrorCode code, const QString& message) : m_code(code), m_message(message) {}

Error::Error(ErrorCode code, const QString& message, const QString& context)
    : m_code(code), m_message(message), m_context(context) {}

QString Error::toString() const {
  if (m_context.isEmpty()) {
    return m_message;
  }
  return QStringLiteral("%1 (Contexte: %2)").arg(m_message, m_context);
}

QString Error::localizedMessage(ErrorCode code) {
  switch (code) {
    case ErrorCode::Unknown:
      return QStringLiteral("Erreur inconnue.");
    case ErrorCode::InvalidArgument:
      return QStringLiteral("Argument invalide.");
    case ErrorCode::InvalidState:
      return QStringLiteral("État invalide.");
    case ErrorCode::NotInitialized:
      return QStringLiteral("Non initialisé.");

    case ErrorCode::NetworkError:
      return QStringLiteral("Erreur réseau.");
    case ErrorCode::NetworkTimeout:
      return QStringLiteral("Délai d'attente réseau dépassé.");
    case ErrorCode::NetworkConnectionRefused:
      return QStringLiteral("Connexion réseau refusée.");
    case ErrorCode::InvalidResponse:
      return QStringLiteral("Réponse invalide.");

    case ErrorCode::TwitchNotAuthenticated:
      return QStringLiteral("Authentifiez-vous d'abord.");
    case ErrorCode::TwitchInvalidToken:
      return QStringLiteral("Jeton d'accès Twitch invalide.");
    case ErrorCode::TwitchApiError:
      return QStringLiteral("Erreur de l'API Twitch.");
    case ErrorCode::TwitchRateLimitExceeded:
      return QStringLiteral("Limite de taux Twitch dépassée.");

    case ErrorCode::MediaFileNotFound:
      return QStringLiteral("Fichier média introuvable.");
    case ErrorCode::MediaFormatNotSupported:
      return QStringLiteral("Format média non supporté.");
    case ErrorCode::MediaDecodeError:
      return QStringLiteral("Erreur de décodage média.");
    case ErrorCode::MediaDeviceError:
      return QStringLiteral("Erreur de périphérique média.");

    case ErrorCode::ConfigNotFound:
      return QStringLiteral("Configuration introuvable.");
    case ErrorCode::ConfigInvalid:
      return QStringLiteral("Configuration invalide.");
  }
  return QStringLiteral("Erreur inconnue.");
}

}  // namespace blueplayer::core



