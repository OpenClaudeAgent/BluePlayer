# tests/e2e/servers/CMakeLists.txt
# Mock HTTP Servers for E2E Testing

add_library(e2e_mock_servers STATIC
    MockTwitchServer.cpp
    MockHlsServer.cpp
)

target_link_libraries(e2e_mock_servers PUBLIC
    Qt6::Core
    Qt6::Network
)

target_include_directories(e2e_mock_servers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set_target_properties(e2e_mock_servers PROPERTIES AUTOMOC ON)
