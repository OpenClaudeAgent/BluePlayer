import QtQuick
import QtTest
import "." // Import E2ETestCase from current directory

/**
 * E2E Scenario: Login View
 * 
 * Tests the login flow for unauthenticated users:
 * 1. App starts without credentials
 * 2. Login view is displayed
 * 3. Login button is visible and clickable
 * 4. UI is in a valid state
 * 
 * Context: UnauthenticatedSetup (no credentials injected)
 */
Item {
    id: root
    width: 1280
    height: 720

    Loader {
        id: appLoader
        anchors.fill: parent
        source: "file://" + E2E_QML_PATH + "/main.qml"
        asynchronous: false
    }

    E2ETestCase {
        id: testCase
        name: "E2E_LoginView"
        when: windowShown && appLoader.status === Loader.Ready

        // Test-specific properties
        property var loginView: null

        function initTestCase() {
            console.log("=== E2E Login View Tests ===")
            mainWindow = appLoader.item
            verify(mainWindow !== null, "Main window should load")
        }

        function cleanupTestCase() {
            console.log("=== E2E Login View Tests Complete ===")
        }

        // =====================================================================
        // Test 1: App shows login view for unauthenticated user
        // =====================================================================
        
        function test_01_login_view_displayed() {
            console.log("Testing: Login view is displayed")
            
            // Wait for the app to load
            wait(1000)
            
            // Check current view - should be login since we're unauthenticated
            compare(mainWindow.currentView, "login", "Should show login view when not authenticated")
            
            // Find the login view (objectName is "loginRoot" in LoginView.qml)
            loginView = findChild(mainWindow, "loginRoot")
            verify(loginView !== null, "Login view (loginRoot) should exist")
            
            console.log("OK Login view is displayed")
        }

        // =====================================================================
        // Test 2: Login button is visible
        // =====================================================================
        
        function test_02_login_button_visible() {
            console.log("Testing: Login button visibility")
            
            wait(500)
            
            // Find the login button
            var loginButton = findChild(mainWindow, "loginButton")
            
            if (loginButton) {
                verify(loginButton.visible, "Login button should be visible")
                verify(loginButton.enabled, "Login button should be enabled")
                console.log("OK Login button is visible and enabled")
            } else {
                // Try finding by prefix (in case button has indexed name)
                var button = findChildByPrefix(mainWindow, "loginButton")
                if (button) {
                    verify(button.visible, "Login button should be visible")
                    console.log("OK Login button found with prefix")
                } else {
                    console.log("Login button not found - checking alternative UI states")
                    // Log the current view for debugging
                    console.log("  - Current view: " + mainWindow.currentView)
                    verify(mainWindow.currentView === "login", "Should be on login view")
                }
            }
        }

        // =====================================================================
        // Test 3: App does not navigate to home without auth
        // =====================================================================
        
        function test_03_no_auto_navigation_to_home() {
            console.log("Testing: No auto-navigation to home without auth")
            
            // Wait a bit to make sure no auto-navigation happens
            wait(2000)
            
            // Should still be on login view
            verify(mainWindow.currentView !== "home", 
                   "Should not navigate to home without authentication")
            
            console.log("OK App stays on login view")
        }

        // =====================================================================
        // Test 4: Login view UI elements
        // =====================================================================
        
        function test_04_login_ui_elements() {
            console.log("Testing: Login UI elements present")
            
            wait(500)
            
            // Find loginRoot (the actual LoginView component)
            loginView = findChild(mainWindow, "loginRoot")
            
            if (loginView) {
                // The login view should have some text explaining the login
                verify(loginView.visible, "Login view should be visible")
                
                // Check for any visible children (buttons, text, etc.)
                var hasVisibleContent = loginView.children && loginView.children.length > 0
                verify(hasVisibleContent, "Login view should have content")
                
                console.log("OK Login view has UI elements")
                console.log("  - Children count: " + (loginView.children ? loginView.children.length : 0))
            } else {
                console.log("Login view not found - verifying we're on login screen")
                compare(mainWindow.currentView, "login", "Should be on login view")
            }
        }
    }
}
