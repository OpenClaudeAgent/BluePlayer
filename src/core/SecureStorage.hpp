#pragma once

#include <QObject>
#include <QString>

namespace blueplayer::core {

/**
 * @brief Classe pour le stockage sécurisé de données sensibles
 * 
 * Utilise macOS Keychain pour protéger les tokens et autres données sensibles.
 * Les données sont stockées de manière native et sécurisée par le système.
 */
class SecureStorage : public QObject {
  Q_OBJECT

public:
  explicit SecureStorage(QObject* parent = nullptr);
  ~SecureStorage() override;

  /**
   * @brief Stocke une valeur de manière sécurisée dans le Keychain
   * @param key La clé d'identification
   * @param value La valeur à stocker
   * @return true si le stockage a réussi
   */
  bool store(const QString& key, const QString& value);

  /**
   * @brief Récupère une valeur stockée dans le Keychain
   * @param key La clé d'identification
   * @param defaultValue La valeur par défaut si la clé n'existe pas
   * @return La valeur récupérée ou defaultValue
   */
  QString retrieve(const QString& key, const QString& defaultValue = QString()) const;

  /**
   * @brief Supprime une valeur du Keychain
   * @param key La clé d'identification
   */
  void remove(const QString& key);

  /**
   * @brief Vérifie si une clé existe dans le Keychain
   * @param key La clé d'identification
   * @return true si la clé existe
   */
  bool contains(const QString& key) const;

  /**
   * @brief Efface toutes les données BluePlayer du Keychain
   */
  void clear();
};

}  // namespace blueplayer::core
