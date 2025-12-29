# tests/e2e/scenarios/open-stream/CMakeLists.txt
# E2E Scenario: Open Stream from Home

add_e2e_scenario(
    NAME e2e_open_stream
    CONTEXT authenticated
    QML_FILE tst_open_stream.qml
    LABELS "e2e;authenticated"
)
