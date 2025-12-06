// Quick utility to extract OAuth token from SecureStorage for testing
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QUuid>
#include <QDebug>
#include <mach/mach_port.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>

QByteArray deriveEncryptionKey() {
    QString systemId;
    mach_port_t masterPort = 0;
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (IOMasterPort(MACH_PORT_NULL, &masterPort) == KERN_SUCCESS) {
    #pragma clang diagnostic pop
        io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(masterPort, "IOService:/");
        if (ioRegistryRoot != 0) {
            CFStringRef uuidCf = static_cast<CFStringRef>(IORegistryEntryCreateCFProperty(
                ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0));
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
    
    if (systemId.isEmpty()) {
        QSettings fallbackSettings(QSettings::IniFormat, QSettings::UserScope,
                                   QStringLiteral("BluePlayer"), QStringLiteral("System"));
        systemId = fallbackSettings.value(QStringLiteral("system_uuid")).toString();
        if (systemId.isEmpty()) {
            systemId = QUuid::createUuid().toString();
        }
    }
    
    QByteArray salt = "BluePlayerSecureStorage2024";
    return QCryptographicHash::hash(
        (systemId + salt).toUtf8(),
        QCryptographicHash::Sha256);
}

QString decrypt(const QString& ciphertext, const QByteArray& key) {
    if (ciphertext.isEmpty()) return QString();
    
    QByteArray encrypted = QByteArray::fromBase64(ciphertext.toUtf8());
    QByteArray decrypted;
    decrypted.reserve(encrypted.size());
    
    for (int i = 0; i < encrypted.size(); ++i) {
        decrypted.append(encrypted[i] ^ key[i % key.size()]);
    }
    
    return QString::fromUtf8(decrypted);
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QStringLiteral("BluePlayer"), QStringLiteral("SecureStorage"));
    
    QString encryptedToken = settings.value(QStringLiteral("access_token")).toString();
    if (encryptedToken.isEmpty()) {
        qDebug() << "No token found in secure storage";
        return 1;
    }
    
    QByteArray key = deriveEncryptionKey();
    QString token = decrypt(encryptedToken, key);
    
    if (token.isEmpty()) {
        qDebug() << "Failed to decrypt token";
        return 1;
    }
    
    qDebug() << token;
    return 0;
}





