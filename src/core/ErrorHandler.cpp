#include "core/ErrorHandler.hpp"

namespace blueplayer::core {

Error ErrorHandler::createError(ErrorCode code, const QString& baseMessage,
                                const QString& context, const QString& details) {
  if (details.isEmpty()) {
    return Error(code, baseMessage, context);
  }
  return Error(code, QStringLiteral("%1: %2").arg(baseMessage, details), context);
}

Error ErrorHandler::networkError(const QString& context, const QString& details) {
  return createError(ErrorCode::NetworkError,
                     QStringLiteral("Erreur réseau"),
                     context, details);
}

Error ErrorHandler::twitchApiError(const QString& context, const QString& details) {
  return createError(ErrorCode::TwitchApiError,
                     QStringLiteral("Erreur de l'API Twitch"),
                     context, details);
}

Error ErrorHandler::twitchAuthError(const QString& context, const QString& details) {
  return createError(ErrorCode::TwitchNotAuthenticated,
                     QStringLiteral("Erreur d'authentification Twitch"),
                     context, details);
}

Error ErrorHandler::mediaError(ErrorCode code, const QString& context, const QString& details) {
  return createError(code, Error::localizedMessage(code), context, details);
}

Error ErrorHandler::validationError(const QString& context, const QString& details) {
  return createError(ErrorCode::InvalidArgument,
                     QStringLiteral("Erreur de validation"),
                     context, details);
}

}  // namespace blueplayer::core












