#ifndef BLUEPLAYER_TEST_E2E_AUTHENTICATEDSETUP_HPP
#define BLUEPLAYER_TEST_E2E_AUTHENTICATEDSETUP_HPP

#include "../BaseE2EContext.hpp"

namespace blueplayer::test::e2e {

/**
 * @brief Setup context for authenticated user tests.
 *
 * Inherits from BaseE2EContext and configures:
 * - Credentials injected into MockSecureStorage
 * - Application initialized as authenticated user
 *
 * Use this context for flows that require authentication:
 * - Open stream, search, home view, etc.
 */
class AuthenticatedSetup : public BaseE2EContext
{
    Q_OBJECT

public:
    explicit AuthenticatedSetup(QObject* parent = nullptr);
    ~AuthenticatedSetup() override = default;

protected:
    QString contextName() const override { return "Authenticated"; }
    bool shouldInjectCredentials() const override { return true; }
};

} // namespace blueplayer::test::e2e

#endif // BLUEPLAYER_TEST_E2E_AUTHENTICATEDSETUP_HPP
