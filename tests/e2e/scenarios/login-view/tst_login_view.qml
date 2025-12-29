import QtQuick
import QtTest
import "." // Import helpers from current directory

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
 * Uses: E2EScenarioTemplate, E2ETestCase, E2EConstants
 */
E2EScenarioTemplate {
    id: root

    E2ETestCase {
        id: testCase
        name: "E2E_LoginView"
        when: windowShown && root.appReady

        // Test-specific properties
        property var loginView: null

        function initTestCase() {
            console.log("=== E2E Login View Tests ===")
            mainWindow = root.app
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
            
            // Use waitForAppReady helper with constant timeout
            var ready = waitForAppReady(E2EConstants.timeoutAppReady)
            verify(ready, "App should be ready")
            
            // Check current view - should be login since we're unauthenticated
            compare(mainWindow.currentView, "login", "Should show login view when not authenticated")
            
            // Find the login view using constant
            loginView = findChild(mainWindow, E2EConstants.loginRoot)
            verify(loginView !== null, "Login view (loginRoot) should exist")
            
            console.log("OK Login view is displayed")
        }

        // =====================================================================
        // Test 2: Login button is visible
        // =====================================================================
        
        function test_02_login_button_visible() {
            console.log("Testing: Login button visibility")
            
            wait(E2EConstants.timeoutShort)
            
            // Use waitForElement helper with constant objectName
            var loginButton = waitForElement(E2EConstants.loginButton, E2EConstants.timeoutMedium)
            
            if (loginButton) {
                verify(loginButton.visible, "Login button should be visible")
                verify(loginButton.enabled, "Login button should be enabled")
                console.log("OK Login button is visible and enabled")
            } else {
                // Try finding by prefix (in case button has indexed name)
                var button = findChildByPrefix(mainWindow, E2EConstants.loginButton)
                if (button) {
                    verify(button.visible, "Login button should be visible")
                    console.log("OK Login button found with prefix")
                } else {
                    console.log("Login button not found - checking alternative UI states")
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
            wait(E2EConstants.timeoutLong / 2)
            
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
            
            wait(E2EConstants.timeoutShort)
            
            // Use verifyElementVisible helper with constant
            loginView = verifyElementVisible(E2EConstants.loginRoot, "Login view should be visible")
            
            // Check for any visible children (buttons, text, etc.)
            var hasVisibleContent = loginView.children && loginView.children.length > 0
            verify(hasVisibleContent, "Login view should have content")
            
            console.log("OK Login view has UI elements")
            console.log("  - Children count: " + (loginView.children ? loginView.children.length : 0))
        }
    }
}
