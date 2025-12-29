# tests/e2e/scenarios/chat-open-close/CMakeLists.txt
# E2E Scenario: Chat Open/Close (B.1)

add_e2e_scenario(
    NAME e2e_chat_open_close
    CONTEXT authenticated
    QML_FILE tst_chat_open_close.qml
    LABELS "e2e;authenticated;chat"
)
