#include "core/SecureStorage.hpp"

#include "core/Logger.hpp"

#include <QStandardPaths>
#include <QDir>

#ifdef Q_OS_MAC
#include <Security/Security.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace blueplayer::core {
using namespace blueplayer::core;

namespace {
// Service name for Keychain items
constexpr const char* kKeychainService = "BluePlayer";

#ifdef Q_OS_MAC
/**
 * @brief Creates a CFString from a QString
 * @param str The QString to convert
 * @return A CFStringRef (caller must release)
 */
CFStringRef createCFString(const QString& str) {
  return CFStringCreateWithCString(kCFAllocatorDefault,
                                   str.toUtf8().constData(),
                                   kCFStringEncodingUTF8);
}

#endif
}  // namespace

SecureStorage::SecureStorage(QObject* parent) : QObject(parent) {
  // Keychain doesn't require initialization
}

SecureStorage::~SecureStorage() = default;

bool SecureStorage::store(const QString& key, const QString& value) {
  if (key.isEmpty()) {
    Logger::error(LogCategory::Core, QStringLiteral("SecureStorage::store: key is empty"));
    return false;
  }

#ifdef Q_OS_MAC
  // First, try to delete any existing item
  remove(key);

  // Create the keychain item attributes
  CFStringRef cfService = createCFString(QString::fromUtf8(kKeychainService));
  CFStringRef cfAccount = createCFString(key);
  CFDataRef cfData = CFDataCreate(kCFAllocatorDefault,
                                  reinterpret_cast<const UInt8*>(value.toUtf8().constData()),
                                  value.toUtf8().length());

  const void* keys[] = {
    kSecClass,
    kSecAttrService,
    kSecAttrAccount,
    kSecValueData,
    kSecAttrAccessible
  };

  const void* values[] = {
    kSecClassGenericPassword,
    cfService,
    cfAccount,
    cfData,
    kSecAttrAccessibleWhenUnlocked
  };

  CFDictionaryRef query = CFDictionaryCreate(kCFAllocatorDefault,
                                              keys, values, 5,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);

  OSStatus status = SecItemAdd(query, nullptr);

  CFRelease(query);
  CFRelease(cfData);
  CFRelease(cfAccount);
  CFRelease(cfService);

  if (status == errSecSuccess) {
    Logger::debug(LogCategory::Core, QStringLiteral("SecureStorage::store: stored key '%1' in Keychain").arg(key));
    return true;
  } else {
    Logger::error(LogCategory::Core, QStringLiteral("SecureStorage::store: failed to store key '%1' (error: %2)").arg(key).arg(status));
    return false;
  }
#else
  // Fallback for non-macOS platforms (should not happen in production)
  Logger::warning(LogCategory::Core, QStringLiteral("SecureStorage: Keychain not available on this platform"));
  return false;
#endif
}

QString SecureStorage::retrieve(const QString& key, const QString& defaultValue) const {
  if (key.isEmpty()) {
    return defaultValue;
  }

#ifdef Q_OS_MAC
  CFStringRef cfService = createCFString(QString::fromUtf8(kKeychainService));
  CFStringRef cfAccount = createCFString(key);

  const void* keys[] = {
    kSecClass,
    kSecAttrService,
    kSecAttrAccount,
    kSecReturnData,
    kSecMatchLimit
  };

  const void* values[] = {
    kSecClassGenericPassword,
    cfService,
    cfAccount,
    kCFBooleanTrue,
    kSecMatchLimitOne
  };

  CFDictionaryRef query = CFDictionaryCreate(kCFAllocatorDefault,
                                              keys, values, 5,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);

  CFTypeRef result = nullptr;
  OSStatus status = SecItemCopyMatching(query, &result);

  CFRelease(query);
  CFRelease(cfAccount);
  CFRelease(cfService);

  if (status == errSecSuccess && result != nullptr) {
    CFDataRef data = static_cast<CFDataRef>(result);
    QString value = QString::fromUtf8(
        reinterpret_cast<const char*>(CFDataGetBytePtr(data)),
        static_cast<int>(CFDataGetLength(data)));
    CFRelease(result);
    Logger::debug(LogCategory::Core, QStringLiteral("SecureStorage::retrieve: retrieved key '%1' from Keychain").arg(key));
    return value;
  } else if (status == errSecItemNotFound) {
    // Item not found - this is normal for first-time use
    return defaultValue;
  } else {
    Logger::warning(LogCategory::Core, QStringLiteral("SecureStorage::retrieve: failed to retrieve key '%1' (error: %2)").arg(key).arg(status));
    return defaultValue;
  }
#else
  Logger::warning(LogCategory::Core, QStringLiteral("SecureStorage: Keychain not available on this platform"));
  return defaultValue;
#endif
}

void SecureStorage::remove(const QString& key) {
  if (key.isEmpty()) {
    return;
  }

#ifdef Q_OS_MAC
  CFStringRef cfService = createCFString(QString::fromUtf8(kKeychainService));
  CFStringRef cfAccount = createCFString(key);

  const void* keys[] = {
    kSecClass,
    kSecAttrService,
    kSecAttrAccount
  };

  const void* values[] = {
    kSecClassGenericPassword,
    cfService,
    cfAccount
  };

  CFDictionaryRef query = CFDictionaryCreate(kCFAllocatorDefault,
                                              keys, values, 3,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);

  OSStatus status = SecItemDelete(query);

  CFRelease(query);
  CFRelease(cfAccount);
  CFRelease(cfService);

  if (status == errSecSuccess) {
    Logger::debug(LogCategory::Core, QStringLiteral("SecureStorage::remove: removed key '%1' from Keychain").arg(key));
  } else if (status != errSecItemNotFound) {
    // Only log warning if it's not "item not found" (which is expected)
    Logger::warning(LogCategory::Core, QStringLiteral("SecureStorage::remove: failed to remove key '%1' (error: %2)").arg(key).arg(status));
  }
#else
  Logger::warning(LogCategory::Core, QStringLiteral("SecureStorage: Keychain not available on this platform"));
#endif
}

bool SecureStorage::contains(const QString& key) const {
  if (key.isEmpty()) {
    return false;
  }

#ifdef Q_OS_MAC
  CFStringRef cfService = createCFString(QString::fromUtf8(kKeychainService));
  CFStringRef cfAccount = createCFString(key);

  const void* keys[] = {
    kSecClass,
    kSecAttrService,
    kSecAttrAccount,
    kSecReturnAttributes,
    kSecMatchLimit
  };

  const void* values[] = {
    kSecClassGenericPassword,
    cfService,
    cfAccount,
    kCFBooleanTrue,
    kSecMatchLimitOne
  };

  CFDictionaryRef query = CFDictionaryCreate(kCFAllocatorDefault,
                                              keys, values, 5,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);

  CFTypeRef result = nullptr;
  OSStatus status = SecItemCopyMatching(query, &result);

  CFRelease(query);
  CFRelease(cfAccount);
  CFRelease(cfService);

  if (result != nullptr) {
    CFRelease(result);
  }

  return status == errSecSuccess;
#else
  return false;
#endif
}

void SecureStorage::clear() {
#ifdef Q_OS_MAC
  CFStringRef cfService = createCFString(QString::fromUtf8(kKeychainService));

  const void* keys[] = {
    kSecClass,
    kSecAttrService,
    kSecMatchLimit
  };

  const void* values[] = {
    kSecClassGenericPassword,
    cfService,
    kSecMatchLimitAll
  };

  CFDictionaryRef query = CFDictionaryCreate(kCFAllocatorDefault,
                                              keys, values, 3,
                                              &kCFTypeDictionaryKeyCallBacks,
                                              &kCFTypeDictionaryValueCallBacks);

  // Keep deleting until no more items are found
  // This is necessary because SecItemDelete may not delete all items at once on macOS
  OSStatus status = errSecSuccess;
  int deletedCount = 0;
  while (status == errSecSuccess) {
    status = SecItemDelete(query);
    if (status == errSecSuccess) {
      deletedCount++;
    }
  }

  CFRelease(query);
  CFRelease(cfService);

  if (status == errSecItemNotFound) {
    Logger::debug(LogCategory::Core, QStringLiteral("SecureStorage::clear: cleared %1 Keychain items for BluePlayer").arg(deletedCount));
  } else if (deletedCount > 0) {
    Logger::debug(LogCategory::Core, QStringLiteral("SecureStorage::clear: cleared %1 Keychain items for BluePlayer").arg(deletedCount));
  } else {
    Logger::warning(LogCategory::Core, QStringLiteral("SecureStorage::clear: failed to clear Keychain (error: %1)").arg(status));
  }
#else
  Logger::warning(LogCategory::Core, QStringLiteral("SecureStorage: Keychain not available on this platform"));
#endif
}

}  // namespace blueplayer::core
