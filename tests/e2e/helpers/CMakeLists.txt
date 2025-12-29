# E2E Test Helpers
# QML components shared across all E2E scenarios

set(E2E_HELPER_QML_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/E2ETestCase.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/E2EConstants.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/E2EScenarioTemplate.qml
)

# Copy qmldir for singleton registration
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/qmldir
    ${CMAKE_CURRENT_BINARY_DIR}/qmldir
    COPYONLY
)

# Make helpers available to parent scope
set(E2E_HELPER_QML_FILES ${E2E_HELPER_QML_FILES} PARENT_SCOPE)
set(E2E_HELPERS_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
