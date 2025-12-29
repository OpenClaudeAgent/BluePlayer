# tests/e2e/scenarios/open-stream/CMakeLists.txt
# E2E Scenario: Open Stream from Home

set(CMAKE_AUTOMOC ON)

add_executable(e2e_open_stream
    main.cpp
)

target_link_libraries(e2e_open_stream PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::QuickTest
    e2e_context_authenticated
)

target_include_directories(e2e_open_stream PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..  # tests/e2e/ for contexts/
    ${CMAKE_SOURCE_DIR}/src
)

# QML test files
set(QML_TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tst_open_stream.qml
)

# Copy QML test files to build directory
foreach(qml_file ${QML_TEST_FILES})
    get_filename_component(qml_name ${qml_file} NAME)
    configure_file(${qml_file} ${CMAKE_CURRENT_BINARY_DIR}/${qml_name} COPYONLY)
endforeach()

# Copy E2E helpers
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../helpers/E2ETestCase.qml
    ${CMAKE_CURRENT_BINARY_DIR}/E2ETestCase.qml
    COPYONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../helpers/E2EScenarioTemplate.qml
    ${CMAKE_CURRENT_BINARY_DIR}/E2EScenarioTemplate.qml
    COPYONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../helpers/E2EConstants.qml
    ${CMAKE_CURRENT_BINARY_DIR}/E2EConstants.qml
    COPYONLY
)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../helpers/qmldir
    ${CMAKE_CURRENT_BINARY_DIR}/qmldir
    COPYONLY
)

# Register as CTest
add_test(
    NAME e2e_open_stream
    COMMAND ${CMAKE_COMMAND} -E env
        QML_IMPORT_PATH=${CMAKE_SOURCE_DIR}/src/ui
        $<TARGET_FILE:e2e_open_stream>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

set_tests_properties(e2e_open_stream PROPERTIES
    LABELS "e2e;authenticated"
    TIMEOUT 60
)
