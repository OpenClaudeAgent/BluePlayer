# tests/e2e/fixtures/CMakeLists.txt
# Fixture loading utilities

# Define the fixtures path at compile time
set(E2E_FIXTURES_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Static library for fixture loading
add_library(e2e_fixtures STATIC
    FixtureLoader.cpp
    FixtureLoader.hpp
)

target_include_directories(e2e_fixtures PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(e2e_fixtures PUBLIC
    Qt6::Core
)

# Pass fixtures path as compile definition
target_compile_definitions(e2e_fixtures PUBLIC
    E2E_FIXTURES_PATH="${E2E_FIXTURES_DIR}"
)

message(STATUS "  e2e_fixtures      - Fixture loader (path: ${E2E_FIXTURES_DIR})")
