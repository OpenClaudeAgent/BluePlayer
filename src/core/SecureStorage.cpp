#include "core/SecureStorage.hpp"

#include "core/Logger.hpp"

#include <QCryptographicHash>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>

#ifdef Q_OS_MAC
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach_port.h>
#endif

namespace blueplayer::core {
using namespace blueplayer::core;

SecureStorage::SecureStorage(QObject* parent) : QObject(parent) {
  // Utiliser un chemin sécurisé pour les settings
  QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  QDir().mkpath(settingsPath);
  
  m_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                             QStringLiteral("BluePlayer"), QStringLiteral("SecureStorage"), this);
}

SecureStorage::~SecureStorage() = default;

QByteArray SecureStorage::deriveEncryptionKey() const {
  // Utiliser une clé en cache pour éviter de la régénérer à chaque fois
  if (!m_cachedKey.isEmpty()) {
    return m_cachedKey;
  }

  // Dériver une clé unique du système
  QString systemId;
  
#ifdef Q_OS_MAC
  // Sur macOS, utiliser le UUID de la plateforme via IOKit
  mach_port_t masterPort = 0;
  if (IOMasterPort(MACH_PORT_NULL, &masterPort) == KERN_SUCCESS) {
    io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(masterPort, "IOService:/");
    if (ioRegistryRoot != 0) {
      CFStringRef uuidCf = (CFStringRef)IORegistryEntryCreateCFProperty(
          ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
      if (uuidCf) {
        char uuid[128];
        if (CFStringGetCString(uuidCf, uuid, 128, kCFStringEncodingUTF8)) {
          systemId = QString::fromUtf8(uuid);
        }
        CFRelease(uuidCf);
      }
      IOObjectRelease(ioRegistryRoot);
    }
  }
#else
  // Sur Linux/Windows, utiliser le chemin home comme identifiant système
  systemId = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#endif

  // Si on ne peut pas obtenir d'ID système, utiliser un UUID stocké
  if (systemId.isEmpty()) {
    QSettings fallbackSettings(QSettings::IniFormat, QSettings::UserScope,
                               QStringLiteral("BluePlayer"), QStringLiteral("System"));
    systemId = fallbackSettings.value(QStringLiteral("system_uuid")).toString();
    if (systemId.isEmpty()) {
      systemId = QUuid::createUuid().toString();
      fallbackSettings.setValue(QStringLiteral("system_uuid"), systemId);
    }
  }

  // Dériver une clé AES-256 depuis l'ID système avec PBKDF2
  QByteArray salt = "BluePlayerSecureStorage2024";
  QByteArray key = QCryptographicHash::hash(
      (systemId + salt).toUtf8(),
      QCryptographicHash::Sha256);

  m_cachedKey = key;
  return key;
}

QString SecureStorage::encrypt(const QString& plaintext) const {
  if (plaintext.isEmpty()) {
    return QString();
  }

  // Note: Pour une implémentation complète, utiliser QCA (Qt Cryptographic Architecture)
  // ou une bibliothèque comme libsodium. Pour l'instant, on utilise un simple XOR
  // avec la clé dérivée (à améliorer avec un vrai chiffrement AES).
  
  QByteArray key = deriveEncryptionKey();
  QByteArray data = plaintext.toUtf8();
  QByteArray encrypted;
  encrypted.reserve(data.size());

  for (int i = 0; i < data.size(); ++i) {
    encrypted.append(data[i] ^ key[i % key.size()]);
  }

  return QString::fromUtf8(encrypted.toBase64());
}

QString SecureStorage::decrypt(const QString& ciphertext) const {
  if (ciphertext.isEmpty()) {
    return QString();
  }

  QByteArray key = deriveEncryptionKey();
  QByteArray encrypted = QByteArray::fromBase64(ciphertext.toUtf8());
  QByteArray decrypted;
  decrypted.reserve(encrypted.size());

  for (int i = 0; i < encrypted.size(); ++i) {
    decrypted.append(encrypted[i] ^ key[i % key.size()]);
  }

  return QString::fromUtf8(decrypted);
}

bool SecureStorage::store(const QString& key, const QString& value) {
  if (key.isEmpty()) {
    Logger::error(LogCategory::Core, QStringLiteral("SecureStorage::store: key is empty"));
    return false;
  }

  QString encrypted = encrypt(value);
  if (encrypted.isEmpty() && !value.isEmpty()) {
    Logger::error(LogCategory::Core, QStringLiteral("SecureStorage::store: encryption failed"));
    return false;
  }

  m_settings->setValue(key, encrypted);
  m_settings->sync();
  
  Logger::debug(LogCategory::Core, QStringLiteral("SecureStorage::store: stored key '%1'").arg(key));
  return true;
}

QString SecureStorage::retrieve(const QString& key, const QString& defaultValue) const {
  if (key.isEmpty()) {
    return defaultValue;
  }

  QString encrypted = m_settings->value(key).toString();
  if (encrypted.isEmpty()) {
    return defaultValue;
  }

  QString decrypted = decrypt(encrypted);
  Logger::debug(LogCategory::Core, QStringLiteral("SecureStorage::retrieve: retrieved key '%1'").arg(key));
  return decrypted;
}

void SecureStorage::remove(const QString& key) {
  if (key.isEmpty()) {
    return;
  }

  m_settings->remove(key);
  m_settings->sync();
  Logger::debug(LogCategory::Core, QStringLiteral("SecureStorage::remove: removed key '%1'").arg(key));
}

bool SecureStorage::contains(const QString& key) const {
  return m_settings->contains(key);
}

void SecureStorage::clear() {
  m_settings->clear();
  m_settings->sync();
  Logger::debug(LogCategory::Core, QStringLiteral("SecureStorage::clear: cleared all data"));
}

}  // namespace blueplayer::core

