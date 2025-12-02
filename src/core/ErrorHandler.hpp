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
  static Error networkError(const QString& context, const QString& details = {});

  /**
   * @brief Crée une erreur Twitch API standardisée
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  static Error twitchApiError(const QString& context, const QString& details = {});

  /**
   * @brief Crée une erreur Twitch authentification standardisée
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  static Error twitchAuthError(const QString& context, const QString& details = {});

  /**
   * @brief Crée une erreur média standardisée
   * @param code Le code d'erreur média spécifique
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  static Error mediaError(ErrorCode code, const QString& context, const QString& details = {});

  /**
   * @brief Crée une erreur de validation standardisée
   * @param context Le contexte de l'erreur
   * @param details Détails additionnels optionnels
   * @return L'erreur créée
   */
  static Error validationError(const QString& context, const QString& details = {});

  /**
   * @brief Convertit une Error en QString pour compatibilité avec les signaux QML existants
   * @param error L'erreur à convertir
   * @return Le message d'erreur sous forme de QString
   */
  static QString toString(const Error& error) { return error.toString(); }
};

}  // namespace blueplayer::core




