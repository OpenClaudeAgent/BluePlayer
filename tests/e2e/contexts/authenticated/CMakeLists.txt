# E2E Context: Authenticated
# Provides setup for tests requiring authenticated user

set(CMAKE_AUTOMOC ON)

add_library(e2e_context_authenticated STATIC
    AuthenticatedSetup.cpp
    AuthenticatedSetup.hpp
)

target_include_directories(e2e_context_authenticated PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..  # For mocks/, servers/
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(e2e_context_authenticated PUBLIC
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickTest
    blueplayer_core
    blueplayer_media
    e2e_mock_servers
)

target_link_directories(e2e_context_authenticated PUBLIC ${HOMEBREW_PREFIX}/lib)
