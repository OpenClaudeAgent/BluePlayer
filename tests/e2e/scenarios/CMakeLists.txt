# tests/e2e/scenarios/CMakeLists.txt
# E2E Test Scenarios - Individual test flows

# Each scenario is an independent executable that uses a context

# Authenticated scenarios (require login)
add_subdirectory(open-stream)

# Unauthenticated scenarios (no login required)
add_subdirectory(login-view)
