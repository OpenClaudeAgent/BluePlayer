/**
 * @file main.cpp
 * @brief E2E Scenario: Search Stream
 * 
 * Tests the search flow: typing in search bar, viewing results,
 * clicking a live streamer to open the player.
 * Uses the authenticated context.
 */

#include <QtQuickTest/quicktest.h>
#include "contexts/authenticated/AuthenticatedSetup.hpp"

QUICK_TEST_MAIN_WITH_SETUP(E2E_SearchStream, blueplayer::test::e2e::AuthenticatedSetup)
