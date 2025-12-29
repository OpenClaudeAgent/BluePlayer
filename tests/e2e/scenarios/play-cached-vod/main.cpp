// tests/e2e/scenarios/play-cached-vod/main.cpp
// E2E Scenario: Play Cached VOD (A.2)

#include "contexts/authenticated/AuthenticatedSetup.hpp"

#include <QtQuickTest>

using blueplayer::test::e2e::AuthenticatedSetup;

QUICK_TEST_MAIN_WITH_SETUP(e2e_play_cached_vod, AuthenticatedSetup)
