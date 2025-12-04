#include "core/ErrorHandler.hpp"

namespace blueplayer::core {

Error ErrorHandler::networkError(const QString& context, const QString& details) {
  if (details.isEmpty()) {
    return Error(ErrorCode::NetworkError, QStringLiteral("Erreur réseau"), context);
  }
  return Error(ErrorCode::NetworkError, QStringLiteral("Erreur réseau: %1").arg(details), context);
}

Error ErrorHandler::twitchApiError(const QString& context, const QString& details) {
  if (details.isEmpty()) {
    return Error(ErrorCode::TwitchApiError, QStringLiteral("Erreur de l'API Twitch"), context);
  }
  return Error(ErrorCode::TwitchApiError, QStringLiteral("Erreur de l'API Twitch: %1").arg(details), context);
}

Error ErrorHandler::twitchAuthError(const QString& context, const QString& details) {
  if (details.isEmpty()) {
    return Error(ErrorCode::TwitchNotAuthenticated, QStringLiteral("Erreur d'authentification Twitch"), context);
  }
  return Error(ErrorCode::TwitchNotAuthenticated, QStringLiteral("Erreur d'authentification Twitch: %1").arg(details), context);
}

Error ErrorHandler::mediaError(ErrorCode code, const QString& context, const QString& details) {
  if (details.isEmpty()) {
    return Error(code, Error::localizedMessage(code), context);
  }
  return Error(code, QStringLiteral("%1: %2").arg(Error::localizedMessage(code), details), context);
}

Error ErrorHandler::validationError(const QString& context, const QString& details) {
  if (details.isEmpty()) {
    return Error(ErrorCode::InvalidArgument, QStringLiteral("Erreur de validation"), context);
  }
  return Error(ErrorCode::InvalidArgument, QStringLiteral("Erreur de validation: %1").arg(details), context);
}

}  // namespace blueplayer::core





