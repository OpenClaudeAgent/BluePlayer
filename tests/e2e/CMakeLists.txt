# tests/e2e/CMakeLists.txt
# End-to-End Testing Infrastructure for BluePlayer

cmake_minimum_required(VERSION 3.24)

# Find required Qt modules
find_package(Qt6 6.5 COMPONENTS Core Gui Network Test REQUIRED)

message(STATUS "E2E tests - configuring mock servers and scenarios")

# Common libraries for E2E tests
set(E2E_TEST_LIBS
  Qt6::Core
  Qt6::Network
  Qt6::Test
)

# E2E Server library (MockTwitchServer, MockHlsServer)
add_library(e2e_servers STATIC
  servers/MockTwitchServer.cpp
  servers/MockHlsServer.cpp
)

target_link_libraries(e2e_servers PUBLIC
  ${E2E_TEST_LIBS}
)

target_include_directories(e2e_servers PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/servers
  ${CMAKE_CURRENT_SOURCE_DIR}/helpers
  ${CMAKE_CURRENT_SOURCE_DIR}/../
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

set_target_properties(e2e_servers PROPERTIES AUTOMOC ON)

# E2E Helpers library
add_library(e2e_helpers STATIC
  helpers/E2ETestBase.cpp
)

target_link_libraries(e2e_helpers PUBLIC
  e2e_servers
  ${E2E_TEST_LIBS}
)

target_include_directories(e2e_helpers PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/helpers
  ${CMAKE_CURRENT_SOURCE_DIR}/servers
)

set_target_properties(e2e_helpers PROPERTIES AUTOMOC ON)

# E2E Infrastructure Test (validates mock servers work)
add_executable(test_e2e_infrastructure
  scenarios/main.cpp
)

target_link_libraries(test_e2e_infrastructure PRIVATE
  e2e_helpers
  e2e_servers
  ${E2E_TEST_LIBS}
  blueplayer_core  # For TwitchApiClient integration tests
)

target_compile_definitions(test_e2e_infrastructure PRIVATE
  QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/scenarios"
)

target_include_directories(test_e2e_infrastructure PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_directories(test_e2e_infrastructure PRIVATE /opt/homebrew/lib)

set_target_properties(test_e2e_infrastructure PROPERTIES AUTOMOC ON)

# Apply coverage flags if enabled
if(BLUEPLAYER_COVERAGE_ENABLED)
  target_compile_options(test_e2e_infrastructure PRIVATE ${BLUEPLAYER_COVERAGE_FLAGS})
  target_link_options(test_e2e_infrastructure PRIVATE ${BLUEPLAYER_COVERAGE_FLAGS})
endif()

add_test(NAME E2E_Infrastructure COMMAND test_e2e_infrastructure)

set_tests_properties(E2E_Infrastructure PROPERTIES
  LABELS "e2e"
  TIMEOUT 60
)

message(STATUS "E2E tests configured:")
message(STATUS "  - test_e2e_infrastructure (mock server validation)")
message(STATUS "Run with: ctest -L e2e")
