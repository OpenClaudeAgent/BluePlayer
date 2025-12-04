#include "core/InputValidator.hpp"

#include <QFileInfo>
#include <QJsonDocument>
#include <QRegularExpression>

namespace blueplayer::core {

// Expressions régulières pour la validation
const QRegularExpression InputValidator::s_twitchUserIdRegex(
    QRegularExpression::anchoredPattern(R"(^\d+$)"));

const QRegularExpression InputValidator::s_twitchUsernameRegex(
    QRegularExpression::anchoredPattern(R"(^[a-zA-Z0-9_]{4,25}$)"));

const QRegularExpression InputValidator::s_oauthTokenRegex(
    QRegularExpression::anchoredPattern(R"(^[a-zA-Z0-9\-_\.]+$)"));

bool InputValidator::isValidUrl(const QString& url) {
  if (url.isEmpty()) {
    return false;
  }

  QUrl qurl(url);
  return qurl.isValid() && 
         (qurl.scheme() == QStringLiteral("http") || 
          qurl.scheme() == QStringLiteral("https")) &&
         !qurl.host().isEmpty();
}

bool InputValidator::isValidFilePath(const QString& filePath) {
  if (filePath.isEmpty()) {
    return false;
  }

  // Vérifier que le chemin n'est pas vide et ne contient pas de caractères dangereux
  if (filePath.contains(QStringLiteral("..")) || 
      filePath.contains(QStringLiteral("\0"))) {
    return false;
  }

  QFileInfo fileInfo(filePath);
  return fileInfo.exists() && fileInfo.isFile();
}

bool InputValidator::isValidTwitchUserId(const QString& userId) {
  if (userId.isEmpty()) {
    return false;
  }

  QRegularExpressionMatch match = s_twitchUserIdRegex.match(userId);
  return match.hasMatch();
}

bool InputValidator::isValidTwitchUsername(const QString& username) {
  if (username.isEmpty()) {
    return false;
  }

  QRegularExpressionMatch match = s_twitchUsernameRegex.match(username);
  return match.hasMatch();
}

QString InputValidator::sanitizeString(const QString& input) {
  QString sanitized = input;
  
  // Supprimer les caractères de contrôle
  sanitized.remove(QRegularExpression(R"([\x00-\x1F\x7F])"));
  
  // Limiter la longueur pour éviter les attaques par déni de service
  constexpr int maxLength = 10000;
  if (sanitized.length() > maxLength) {
    sanitized = sanitized.left(maxLength);
  }
  
  return sanitized;
}

bool InputValidator::isValidJson(const QString& json) {
  if (json.isEmpty()) {
    return false;
  }

  QJsonParseError error;
  QJsonDocument::fromJson(json.toUtf8(), &error);
  return error.error == QJsonParseError::NoError;
}

bool InputValidator::isValidOAuthToken(const QString& token) {
  if (token.isEmpty()) {
    return false;
  }

  // Un token OAuth doit avoir une longueur raisonnable (entre 20 et 2000 caractères)
  if (token.length() < 20 || token.length() > 2000) {
    return false;
  }

  QRegularExpressionMatch match = s_oauthTokenRegex.match(token);
  return match.hasMatch();
}

}  // namespace blueplayer::core





