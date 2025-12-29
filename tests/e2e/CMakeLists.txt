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

find_package(Qt6 6.5 COMPONENTS Core Network Quick QuickTest WebSockets REQUIRED)

# ============================================================================
# Helper Function for E2E Scenarios
# ============================================================================
#
# Usage:
#   add_e2e_scenario(
#       NAME e2e_my_scenario
#       CONTEXT authenticated|unauthenticated
#       QML_FILE tst_my_scenario.qml
#       LABELS "e2e;authenticated;myfeature"
#   )
#
function(add_e2e_scenario)
    cmake_parse_arguments(E2E "" "NAME;CONTEXT;QML_FILE" "LABELS" ${ARGN})
    
    if(NOT E2E_NAME)
        message(FATAL_ERROR "add_e2e_scenario: NAME is required")
    endif()
    if(NOT E2E_CONTEXT)
        set(E2E_CONTEXT "authenticated")
    endif()
    if(NOT E2E_QML_FILE)
        message(FATAL_ERROR "add_e2e_scenario: QML_FILE is required")
    endif()
    
    # Create executable
    add_executable(${E2E_NAME} main.cpp)
    set_target_properties(${E2E_NAME} PROPERTIES AUTOMOC ON)
    
    # Link libraries
    target_link_libraries(${E2E_NAME} PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::QuickTest
        e2e_context_${E2E_CONTEXT}
    )
    
    # Include directories
    target_include_directories(${E2E_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${CMAKE_SOURCE_DIR}/src
    )
    
    # Copy QML test file
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${E2E_QML_FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/${E2E_QML_FILE}
        COPYONLY
    )
    
    # Copy E2E helpers
    set(HELPERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../helpers)
    configure_file(${HELPERS_DIR}/E2ETestCase.qml ${CMAKE_CURRENT_BINARY_DIR}/E2ETestCase.qml COPYONLY)
    configure_file(${HELPERS_DIR}/E2EScenarioTemplate.qml ${CMAKE_CURRENT_BINARY_DIR}/E2EScenarioTemplate.qml COPYONLY)
    configure_file(${HELPERS_DIR}/E2EConstants.qml ${CMAKE_CURRENT_BINARY_DIR}/E2EConstants.qml COPYONLY)
    configure_file(${HELPERS_DIR}/qmldir ${CMAKE_CURRENT_BINARY_DIR}/qmldir COPYONLY)
    
    # Register as CTest
    add_test(
        NAME ${E2E_NAME}
        COMMAND ${CMAKE_COMMAND} -E env
            QML_IMPORT_PATH=${CMAKE_SOURCE_DIR}/src/ui
            $<TARGET_FILE:${E2E_NAME}>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    # Set labels and timeout
    if(E2E_LABELS)
        set_tests_properties(${E2E_NAME} PROPERTIES
            LABELS "${E2E_LABELS}"
            TIMEOUT 60
        )
    else()
        set_tests_properties(${E2E_NAME} PROPERTIES
            LABELS "e2e;${E2E_CONTEXT}"
            TIMEOUT 60
        )
    endif()
endfunction()

# ============================================================================
# Sub-modules
# ============================================================================

# Fixture loading utilities
add_subdirectory(fixtures)

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
