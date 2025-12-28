# tests/e2e/CMakeLists.txt
# E2E Testing Infrastructure using Qt Quick Test

cmake_minimum_required(VERSION 3.24)

message(STATUS "E2E Testing Infrastructure - Qt Quick Test")

# ============================================================================
# Dependencies
# ============================================================================

find_package(Qt6 6.5 COMPONENTS Core Network Quick QuickTest REQUIRED)

# ============================================================================
# Mock Servers Library
# ============================================================================

# Static library containing mock HTTP servers for Twitch API and HLS
add_library(e2e_mock_servers STATIC
    servers/MockTwitchServer.cpp
    servers/MockHlsServer.cpp
)

target_link_libraries(e2e_mock_servers PUBLIC
    Qt6::Core
    Qt6::Network
)

target_include_directories(e2e_mock_servers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/servers
)

set_target_properties(e2e_mock_servers PROPERTIES AUTOMOC ON)

# ============================================================================
# E2E Test Runner
# ============================================================================

# The E2E test executable
add_executable(e2e_tests
    main.cpp
    Setup.cpp
    Setup.hpp
)

target_link_libraries(e2e_tests PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickTest
    e2e_mock_servers
    blueplayer_core    # For Config and other core classes
    blueplayer_media   # Needed for transitive dependencies
)

# Add Homebrew lib path for libmpv
target_link_directories(e2e_tests PRIVATE ${HOMEBREW_PREFIX}/lib)

target_include_directories(e2e_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(e2e_tests PROPERTIES AUTOMOC ON)

# ============================================================================
# Test Resources (QML test files)
# ============================================================================

# Define the source directory for QML tests
# Qt Quick Test looks for tst_*.qml files in this directory
set(E2E_QML_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Pass the QML test directory to the executable
target_compile_definitions(e2e_tests PRIVATE
    QUICK_TEST_SOURCE_DIR="${E2E_QML_SOURCE_DIR}"
)

# ============================================================================
# CTest Integration
# ============================================================================

add_test(
    NAME E2E_OpenLive
    COMMAND e2e_tests -input "${E2E_QML_SOURCE_DIR}"
)

# Set test properties
set_tests_properties(E2E_OpenLive PROPERTIES
    LABELS "e2e"
    TIMEOUT 60
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"  # Run headless in CI
)

# ============================================================================
# Usage Instructions
# ============================================================================

message(STATUS "")
message(STATUS "E2E Test Infrastructure configured:")
message(STATUS "  - Mock servers: MockTwitchServer, MockHlsServer")
message(STATUS "  - Test runner: e2e_tests")
message(STATUS "  - QML tests: ${E2E_QML_SOURCE_DIR}/tst_*.qml")
message(STATUS "")
message(STATUS "To run E2E tests:")
message(STATUS "  make e2e_tests && ./tests/e2e/e2e_tests")
message(STATUS "")
message(STATUS "To run with visible UI (not headless):")
message(STATUS "  QT_QPA_PLATFORM= ./tests/e2e/e2e_tests")
message(STATUS "")
