# tests/e2e/scenarios/picture-in-picture/CMakeLists.txt
# E2E Scenario: Picture-in-Picture (C.4)

add_e2e_scenario(
    NAME e2e_picture_in_picture
    CONTEXT authenticated
    QML_FILE tst_picture_in_picture.qml
    LABELS "e2e;authenticated;pip"
)
