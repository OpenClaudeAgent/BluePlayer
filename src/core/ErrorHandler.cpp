#include "core/ErrorHandler.hpp"

#include <QCoreApplication>

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
                     QCoreApplication::translate("ErrorHandler", "Network error"),
                     context, details);
}

Error ErrorHandler::twitchApiError(const QString& context, const QString& details) {
  return createError(ErrorCode::TwitchApiError,
                     QCoreApplication::translate("ErrorHandler", "Twitch API error"),
                     context, details);
}

Error ErrorHandler::twitchAuthError(const QString& context, const QString& details) {
  return createError(ErrorCode::TwitchNotAuthenticated,
                     QCoreApplication::translate("ErrorHandler", "Twitch authentication error"),
                     context, details);
}

Error ErrorHandler::mediaError(ErrorCode code, const QString& context, const QString& details) {
  return createError(code, Error::localizedMessage(code), context, details);
}

Error ErrorHandler::validationError(const QString& context, const QString& details) {
  return createError(ErrorCode::InvalidArgument,
                     QCoreApplication::translate("ErrorHandler", "Validation error"),
                     context, details);
}

}  // namespace blueplayer::core












