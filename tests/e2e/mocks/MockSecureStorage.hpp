#pragma once

#include "core/ISecureStorage.hpp"

#include <QDateTime>
#include <QHash>
#include <QString>

namespace blueplayer::test::e2e {

/**
 * @brief Mock implementation of ISecureStorage for E2E tests
 * 
 * Stores credentials in memory instead of the system keychain.
 * Pre-populated with test fixtures to simulate an authenticated user.
 */
class MockSecureStorage : public blueplayer::core::ISecureStorage {
public:
    MockSecureStorage() = default;
    ~MockSecureStorage() override = default;

    /**
     * @brief Pre-populate with authentication credentials
     * @param accessToken The access token to use
     * @param refreshToken The refresh token to use
     * @param clientId The client ID that generated the token
     * @param expiresInSeconds Token expiration time in seconds from now
     */
    void setCredentials(const QString& accessToken,
                        const QString& refreshToken,
                        const QString& clientId,
                        int expiresInSeconds = 14400) {
        m_storage[QStringLiteral("access_token")] = accessToken;
        m_storage[QStringLiteral("refresh_token")] = refreshToken;
        m_storage[QStringLiteral("token_client_id")] = clientId;
        
        // Calculate expiration time (ISO format)
        QDateTime expiration = QDateTime::currentDateTimeUtc().addSecs(expiresInSeconds);
        m_storage[QStringLiteral("token_expiration")] = expiration.toString(Qt::ISODate);
    }

    // ISecureStorage interface
    bool store(const QString& key, const QString& value) override {
        m_storage[key] = value;
        return true;
    }

    QString retrieve(const QString& key, const QString& defaultValue = QString()) const override {
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

private:
    QHash<QString, QString> m_storage;
};

}  // namespace blueplayer::test::e2e
