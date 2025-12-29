# tests/e2e/scenarios/audio-only-mode/CMakeLists.txt
# E2E Scenario: Audio Only Mode (A.5)

add_e2e_scenario(
    NAME e2e_audio_only_mode
    CONTEXT authenticated
    QML_FILE tst_audio_only_mode.qml
    LABELS "e2e;authenticated;audio"
)
