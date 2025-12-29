# E2E Context: Unauthenticated
# Provides setup for tests with no authentication

set(CMAKE_AUTOMOC ON)

add_library(e2e_context_unauthenticated STATIC
    UnauthenticatedSetup.cpp
    UnauthenticatedSetup.hpp
)

target_include_directories(e2e_context_unauthenticated PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..  # For mocks/, servers/
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(e2e_context_unauthenticated PUBLIC
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickTest
    blueplayer_core
    blueplayer_media
    e2e_mock_servers
)

target_link_directories(e2e_context_unauthenticated PUBLIC ${HOMEBREW_PREFIX}/lib)
