/**
 * @file main.cpp
 * @brief E2E Scenario: Quality Switch
 * 
 * Tests the quality selector in the player view.
 * Uses the authenticated context.
 */

#include <QtQuickTest/quicktest.h>
#include "contexts/authenticated/AuthenticatedSetup.hpp"

QUICK_TEST_MAIN_WITH_SETUP(E2E_QualitySwitch, blueplayer::test::e2e::AuthenticatedSetup)
