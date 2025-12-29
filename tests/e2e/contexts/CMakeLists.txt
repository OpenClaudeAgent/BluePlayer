# E2E Contexts
# Reusable setup configurations for E2E tests

# Base context class (shared by all contexts)
add_library(e2e_context_base STATIC
    BaseE2EContext.cpp
    BaseE2EContext.hpp
    E2EContextHelpers.hpp
)

target_link_libraries(e2e_context_base PUBLIC
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickTest
    e2e_mock_servers
    e2e_fixtures
    blueplayer_core
    blueplayer_media
)

target_include_directories(e2e_context_base PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${PROJECT_SOURCE_DIR}/src
)

# Ensure Homebrew libraries are found (mpv, etc.)
target_link_directories(e2e_context_base PUBLIC ${HOMEBREW_PREFIX}/lib)

set_target_properties(e2e_context_base PROPERTIES AUTOMOC ON)

# Derived contexts
add_subdirectory(authenticated)
add_subdirectory(unauthenticated)
