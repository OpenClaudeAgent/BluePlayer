# tests/e2e/scenarios/quality-switch/CMakeLists.txt
# E2E Scenario: Quality Switch (A.3)

add_e2e_scenario(
    NAME e2e_quality_switch
    CONTEXT authenticated
    QML_FILE tst_quality_switch.qml
    LABELS "e2e;authenticated;quality"
)
