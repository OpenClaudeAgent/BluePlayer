/**
 * @file main.cpp
 * @brief E2E Scenario: Open Stream
 * 
 * Tests the flow of opening a live stream from the home view.
 * Uses the authenticated context.
 */

#include <QtQuickTest/quicktest.h>
#include "contexts/authenticated/AuthenticatedSetup.hpp"

QUICK_TEST_MAIN_WITH_SETUP(E2E_OpenStream, blueplayer::test::e2e::AuthenticatedSetup)
