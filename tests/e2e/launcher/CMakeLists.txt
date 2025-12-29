# tests/e2e/launcher/CMakeLists.txt
# Standalone E2E Launcher with mock servers

add_executable(e2e_launcher
    main.cpp
)

target_link_libraries(e2e_launcher PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::Widgets
    e2e_mock_servers
    blueplayer_core
    blueplayer_media
)

target_link_directories(e2e_launcher PRIVATE ${HOMEBREW_PREFIX}/lib)

target_include_directories(e2e_launcher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..       # tests/e2e/
    ${CMAKE_CURRENT_SOURCE_DIR}/../mocks # tests/e2e/mocks/
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(e2e_launcher PROPERTIES AUTOMOC ON)
