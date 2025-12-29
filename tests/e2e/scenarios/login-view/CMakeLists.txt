# tests/e2e/scenarios/login-view/CMakeLists.txt
# E2E Scenario: Login View for unauthenticated users

add_e2e_scenario(
    NAME e2e_login_view
    CONTEXT unauthenticated
    QML_FILE tst_login_view.qml
    LABELS "e2e;unauthenticated"
)
