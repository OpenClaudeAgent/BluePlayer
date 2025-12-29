# tests/e2e/CMakeLists.txt
# E2E Testing Infrastructure - Main entry point

cmake_minimum_required(VERSION 3.24)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "E2E Testing Infrastructure")
message(STATUS "========================================")

# ============================================================================
# Dependencies
# ============================================================================

find_package(Qt6 6.5 COMPONENTS Core Network Quick QuickTest REQUIRED)

# ============================================================================
# Sub-modules
# ============================================================================

# Mock HTTP servers (Twitch API, HLS)
add_subdirectory(servers)

# QML test helpers (E2ETestCase base class)
add_subdirectory(helpers)

# Reusable contexts (setups)
add_subdirectory(contexts)

# E2E test scenarios
add_subdirectory(scenarios)

# Standalone launcher for manual testing
add_subdirectory(launcher)

# ============================================================================
# Usage
# ============================================================================

message(STATUS "")
message(STATUS "Targets:")
message(STATUS "  e2e_open_stream   - Open stream flow (authenticated)")
message(STATUS "  e2e_login_view    - Login flow (unauthenticated)")
message(STATUS "  e2e_launcher      - Interactive app with mock servers")
message(STATUS "")
message(STATUS "Run tests:")
message(STATUS "  ctest -L e2e                    # All E2E tests")
message(STATUS "  ctest -L authenticated          # Authenticated tests only")
message(STATUS "  ctest -L unauthenticated        # Unauthenticated tests only")
message(STATUS "")
