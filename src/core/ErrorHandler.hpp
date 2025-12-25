#pragma once

#include "core/Error.hpp"
#include <QString>

namespace blueplayer::core {

/**
 * @brief Classe utilitaire pour créer et gérer des erreurs standardisées
 * 
 * Fournit des méthodes statiques pour créer des erreurs typées
 * selon leur domaine (Twitch, Media, Network, etc.)
 */
class ErrorHandler {
public:
  /**
   * @brief Crée une erreur réseau standardisée
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  [[nodiscard]] static Error networkError(const QString& context, const QString& details = QString());

  /**
   * @brief Crée une erreur Twitch API standardisée
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  [[nodiscard]] static Error twitchApiError(const QString& context, const QString& details = QString());

  /**
   * @brief Crée une erreur Twitch authentification standardisée
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  [[nodiscard]] static Error twitchAuthError(const QString& context, const QString& details = QString());

  /**
   * @brief Crée une erreur média standardisée
   * @param code Le code d'erreur média spécifique
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  [[nodiscard]] static Error mediaError(ErrorCode code, const QString& context, const QString& details = QString());

  /**
   * @brief Crée une erreur de validation standardisée
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  [[nodiscard]] static Error validationError(const QString& context, const QString& details = QString());

private:
  /**
   * @brief Helper pour créer une erreur avec message optionnel
   * @param code Le code d'erreur
   * @param baseMessage Le message de base
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  [[nodiscard]] static Error createError(ErrorCode code, const QString& baseMessage,
                                         const QString& context, const QString& details);
};

}  // namespace blueplayer::core












