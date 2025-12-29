/**
 * @file main.cpp
 * @brief E2E Scenario: Chat Open/Close
 * 
 * Tests opening and closing the chat panel in player view.
 * Uses the authenticated context.
 */

#include <QtQuickTest/quicktest.h>
#include "contexts/authenticated/AuthenticatedSetup.hpp"

QUICK_TEST_MAIN_WITH_SETUP(E2E_ChatOpenClose, blueplayer::test::e2e::AuthenticatedSetup)
