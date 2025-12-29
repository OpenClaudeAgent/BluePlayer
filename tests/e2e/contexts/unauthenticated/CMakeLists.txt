# E2E Context: Unauthenticated
# Provides setup for tests with no authentication
# Inherits from BaseE2EContext for common setup logic

set(CMAKE_AUTOMOC ON)

add_library(e2e_context_unauthenticated STATIC
    UnauthenticatedSetup.cpp
    UnauthenticatedSetup.hpp
)

target_include_directories(e2e_context_unauthenticated PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..      # For BaseE2EContext
    ${CMAKE_CURRENT_SOURCE_DIR}/../..   # For mocks/, servers/
    ${CMAKE_SOURCE_DIR}/src
)

# Link to base context - it brings all common dependencies
target_link_libraries(e2e_context_unauthenticated PUBLIC
    e2e_context_base
)

target_link_directories(e2e_context_unauthenticated PUBLIC ${HOMEBREW_PREFIX}/lib)
