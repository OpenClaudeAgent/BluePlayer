# tests/e2e/scenarios/search-stream/CMakeLists.txt
# E2E Scenario: Search Stream (A.1)

add_e2e_scenario(
    NAME e2e_search_stream
    CONTEXT authenticated
    QML_FILE tst_search_stream.qml
    LABELS "e2e;authenticated;search"
)
