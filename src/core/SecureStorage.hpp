#pragma once

#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace blueplayer::core {

/**
 * @brief Classe pour le stockage sécurisé de données sensibles
 * 
 * Utilise un chiffrement AES pour protéger les tokens et autres données sensibles.
 * La clé de chiffrement est dérivée du système pour éviter le stockage en clair.
 */
class SecureStorage : public QObject {
  Q_OBJECT

public:
  explicit SecureStorage(QObject* parent = nullptr);
  ~SecureStorage() override;

  /**
   * @brief Stocke une valeur de manière sécurisée
   * @param key La clé d'identification
   * @param value La valeur à stocker (sera chiffrée)
   * @return true si le stockage a réussi
   */
  bool store(const QString& key, const QString& value);

  /**
   * @brief Récupère une valeur stockée de manière sécurisée
   * @param key La clé d'identification
   * @param defaultValue La valeur par défaut si la clé n'existe pas
   * @return La valeur déchiffrée ou defaultValue
   */
  QString retrieve(const QString& key, const QString& defaultValue = QString()) const;

  /**
   * @brief Supprime une valeur stockée
   * @param key La clé d'identification
   */
  void remove(const QString& key);

  /**
   * @brief Vérifie si une clé existe
   * @param key La clé d'identification
   * @return true si la clé existe
   */
  bool contains(const QString& key) const;

  /**
   * @brief Efface toutes les données stockées
   */
  void clear();

private:
  /**
   * @brief Génère une clé de chiffrement dérivée du système
   * @return La clé de chiffrement
   */
  QByteArray deriveEncryptionKey() const;

  /**
   * @brief Chiffre une valeur avec AES
   * @param plaintext Le texte en clair
   * @return Le texte chiffré (base64)
   */
  QString encrypt(const QString& plaintext) const;

  /**
   * @brief Déchiffre une valeur avec AES
   * @param ciphertext Le texte chiffré (base64)
   * @return Le texte en clair
   */
  QString decrypt(const QString& ciphertext) const;

  QSettings* m_settings = nullptr;
  mutable QByteArray m_cachedKey;
};

}  // namespace blueplayer::core



