/**
 * @file main.cpp
 * @brief E2E Scenario: Audio Only Mode
 * 
 * Tests the audio-only quality selection in the player view.
 * Uses the authenticated context.
 */

#include <QtQuickTest/quicktest.h>
#include "contexts/authenticated/AuthenticatedSetup.hpp"

QUICK_TEST_MAIN_WITH_SETUP(E2E_AudioOnlyMode, blueplayer::test::e2e::AuthenticatedSetup)
