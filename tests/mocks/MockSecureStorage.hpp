#pragma once

#include "core/ISecureStorage.hpp"

#include <QHash>
#include <QString>

namespace blueplayer::test {

/**
 * @brief Mock du stockage sécurisé pour les tests
 * 
 * Stocke les données en mémoire sans utiliser le Keychain macOS,
 * ce qui évite les popups d'autorisation interactives pendant les tests.
 */
class MockSecureStorage : public blueplayer::core::ISecureStorage {
public:
  MockSecureStorage() = default;
  ~MockSecureStorage() override = default;

  bool store(const QString& key, const QString& value) override {
    if (key.isEmpty()) {
      return false;
    }
    m_storage[key] = value;
    return true;
  }

  QString retrieve(const QString& key, const QString& defaultValue = QString()) const override {
    if (key.isEmpty()) {
      return defaultValue;
    }
    return m_storage.value(key, defaultValue);
  }

  void remove(const QString& key) override {
    m_storage.remove(key);
  }

  bool contains(const QString& key) const override {
    return m_storage.contains(key);
  }

  void clear() override {
    m_storage.clear();
  }

  // Méthodes utilitaires pour les tests
  int count() const { return m_storage.size(); }
  QStringList keys() const { return m_storage.keys(); }

private:
  QHash<QString, QString> m_storage;
};

}  // namespace blueplayer::test
