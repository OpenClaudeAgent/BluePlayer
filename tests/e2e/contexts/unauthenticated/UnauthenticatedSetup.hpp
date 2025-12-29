#ifndef BLUEPLAYER_TEST_E2E_UNAUTHENTICATEDSETUP_HPP
#define BLUEPLAYER_TEST_E2E_UNAUTHENTICATEDSETUP_HPP

#include "../BaseE2EContext.hpp"

namespace blueplayer::test::e2e {

/**
 * @brief Setup context for unauthenticated user tests.
 *
 * Inherits from BaseE2EContext and configures:
 * - NO credentials injected
 * - Application initialized as unauthenticated user
 *
 * Use this context for flows that don't require authentication:
 * - Login view, onboarding, etc.
 */
class UnauthenticatedSetup : public BaseE2EContext
{
    Q_OBJECT

public:
    explicit UnauthenticatedSetup(QObject* parent = nullptr);
    ~UnauthenticatedSetup() override = default;

protected:
    QString contextName() const override { return "Unauthenticated"; }
    bool shouldInjectCredentials() const override { return false; }
};

} // namespace blueplayer::test::e2e

#endif // BLUEPLAYER_TEST_E2E_UNAUTHENTICATEDSETUP_HPP
