#include "core/Error.hpp"

#include <QCoreApplication>

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
      return QCoreApplication::translate("Error", "Unknown error.");
    case ErrorCode::InvalidArgument:
      return QCoreApplication::translate("Error", "Invalid argument.");
    case ErrorCode::InvalidState:
      return QCoreApplication::translate("Error", "Invalid state.");
    case ErrorCode::NotInitialized:
      return QCoreApplication::translate("Error", "Not initialized.");

    case ErrorCode::NetworkError:
      return QCoreApplication::translate("Error", "Network error.");
    case ErrorCode::NetworkTimeout:
      return QCoreApplication::translate("Error", "Network timeout.");
    case ErrorCode::NetworkConnectionRefused:
      return QCoreApplication::translate("Error", "Network connection refused.");
    case ErrorCode::InvalidResponse:
      return QCoreApplication::translate("Error", "Invalid response.");

    case ErrorCode::TwitchNotAuthenticated:
      return QCoreApplication::translate("Error", "Please authenticate first.");
    case ErrorCode::TwitchInvalidToken:
      return QCoreApplication::translate("Error", "Invalid Twitch access token.");
    case ErrorCode::TwitchApiError:
      return QCoreApplication::translate("Error", "Twitch API error.");
    case ErrorCode::TwitchRateLimitExceeded:
      return QCoreApplication::translate("Error", "Twitch rate limit exceeded.");

    case ErrorCode::MediaFileNotFound:
      return QCoreApplication::translate("Error", "Media file not found.");
    case ErrorCode::MediaFormatNotSupported:
      return QCoreApplication::translate("Error", "Media format not supported.");
    case ErrorCode::MediaDecodeError:
      return QCoreApplication::translate("Error", "Media decode error.");
    case ErrorCode::MediaDeviceError:
      return QCoreApplication::translate("Error", "Media device error.");

    case ErrorCode::ConfigNotFound:
      return QCoreApplication::translate("Error", "Configuration not found.");
    case ErrorCode::ConfigInvalid:
      return QCoreApplication::translate("Error", "Invalid configuration.");
  }
  return QCoreApplication::translate("Error", "Unknown error.");
}

}  // namespace blueplayer::core












