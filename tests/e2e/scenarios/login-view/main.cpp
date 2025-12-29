/**
 * @file main.cpp
 * @brief E2E Scenario: Login View
 * 
 * Tests the login flow for unauthenticated users.
 * Uses the unauthenticated context.
 */

#include <QtQuickTest/quicktest.h>
#include "contexts/unauthenticated/UnauthenticatedSetup.hpp"

QUICK_TEST_MAIN_WITH_SETUP(E2E_LoginView, blueplayer::test::e2e::UnauthenticatedSetup)
