# tests/e2e/scenarios/play-cached-vod/CMakeLists.txt
# E2E Scenario: Play Cached VOD (A.2)

add_e2e_scenario(
    NAME e2e_play_cached_vod
    CONTEXT authenticated
    QML_FILE tst_play_cached_vod.qml
    LABELS "e2e;authenticated;vod"
)
