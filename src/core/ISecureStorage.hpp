#pragma once

#include <QString>

namespace blueplayer::core {

/**
 * @brief Interface pour le stockage sécurisé de données sensibles
 * 
 * Permet de mocker le stockage sécurisé dans les tests sans dépendre
 * du Keychain macOS qui demande des autorisations interactives.
 */
class ISecureStorage {
public:
  virtual ~ISecureStorage() = default;

  /**
   * @brief Stocke une valeur de manière sécurisée
   * @param key La clé d'identification
   * @param value La valeur à stocker
   * @return true si le stockage a réussi
   */
  virtual bool store(const QString& key, const QString& value) = 0;

  /**
   * @brief Récupère une valeur stockée
   * @param key La clé d'identification
   * @param defaultValue La valeur par défaut si la clé n'existe pas
   * @return La valeur récupérée ou defaultValue
   */
  virtual QString retrieve(const QString& key, const QString& defaultValue = QString()) const = 0;

  /**
   * @brief Supprime une valeur du stockage
   * @param key La clé d'identification
   */
  virtual void remove(const QString& key) = 0;

  /**
   * @brief Vérifie si une clé existe dans le stockage
   * @param key La clé d'identification
   * @return true si la clé existe
   */
  virtual bool contains(const QString& key) const = 0;

  /**
   * @brief Efface toutes les données du stockage
   */
  virtual void clear() = 0;
};

}  // namespace blueplayer::core
