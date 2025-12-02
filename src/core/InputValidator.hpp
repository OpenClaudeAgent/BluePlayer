#pragma once

#include <QString>
#include <QUrl>
#include <QRegularExpression>

namespace blueplayer::core {

/**
 * @brief Classe utilitaire pour la validation d'entrées utilisateur
 * 
 * Fournit des méthodes statiques pour valider différents types d'entrées :
 * - URLs
 * - Chemins de fichiers
 * - Identifiants utilisateur
 * - Données JSON
 */
class InputValidator {
public:
  /**
   * @brief Valide une URL
   * @param url L'URL à valider
   * @return true si l'URL est valide
   */
  static bool isValidUrl(const QString& url);

  /**
   * @brief Valide un chemin de fichier
   * @param filePath Le chemin à valider
   * @return true si le chemin est valide et le fichier existe
   */
  static bool isValidFilePath(const QString& filePath);

  /**
   * @brief Valide un identifiant utilisateur Twitch
   * @param userId L'identifiant à valider
   * @return true si l'identifiant est valide
   */
  static bool isValidTwitchUserId(const QString& userId);

  /**
   * @brief Valide un nom d'utilisateur Twitch
   * @param username Le nom d'utilisateur à valider
   * @return true si le nom est valide
   */
  static bool isValidTwitchUsername(const QString& username);

  /**
   * @brief Sanitise une chaîne pour éviter les injections
   * @param input La chaîne à sanitiser
   * @return La chaîne sanitisée
   */
  static QString sanitizeString(const QString& input);

  /**
   * @brief Valide une chaîne JSON
   * @param json La chaîne JSON à valider
   * @return true si le JSON est valide
   */
  static bool isValidJson(const QString& json);

  /**
   * @brief Valide un token OAuth
   * @param token Le token à valider
   * @return true si le token a un format valide
   */
  static bool isValidOAuthToken(const QString& token);

private:
  // Expressions régulières pour la validation
  static const QRegularExpression s_twitchUserIdRegex;
  static const QRegularExpression s_twitchUsernameRegex;
  static const QRegularExpression s_oauthTokenRegex;
};

}  // namespace blueplayer::core




