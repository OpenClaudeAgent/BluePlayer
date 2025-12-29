# tests/e2e/scenarios/CMakeLists.txt
# E2E Test Scenarios - Individual test flows
#
# Each scenario is an independent executable that uses a context.
#
# =============================================================================
# LABEL SYSTEM FOR TEST PROFILES
# =============================================================================
#
# Labels are used to categorize tests for different run profiles:
#
#   e2e              - Fast tests (< 30 seconds), run in CI
#   e2e-extended     - Slow tests (cache, resume, etc.), run in nightly/release
#
# Make commands:
#   make e2e          - Run fast E2E tests only
#   make e2e-extended - Run ALL E2E tests (fast + slow)
#
# When creating a new scenario:
#   - Fast test:  LABELS "e2e;authenticated"
#   - Slow test:  LABELS "e2e-extended;authenticated"  (no "e2e" label!)
#
# =============================================================================

# Authenticated scenarios (require login)
add_subdirectory(open-stream)
add_subdirectory(search-stream)
add_subdirectory(quality-switch)
add_subdirectory(chat-open-close)
add_subdirectory(audio-only-mode)
add_subdirectory(play-cached-vod)
add_subdirectory(picture-in-picture)

# Unauthenticated scenarios (no login required)
add_subdirectory(login-view)
