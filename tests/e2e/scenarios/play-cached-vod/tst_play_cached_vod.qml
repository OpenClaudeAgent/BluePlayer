import QtQuick
import QtTest
import "." // Import helpers from current directory

/**
 * E2E Scenario: Play Cached VOD (A.2)
 *
 * Tests the cached VOD / Replays navigation flow:
 * - Replays button visibility on home
 * - Navigation to cache view
 * - Navigation buttons hide when panel is open
 * - Return to home functionality
 *
 * Note: VOD card interaction is tested separately when VODs exist in cache.
 *
 * Context: AuthenticatedSetup
 */
E2EScenarioTemplate {
    id: root

    E2ETestCase {
        id: testCase
        name: "E2E_PlayCachedVod"
        when: windowShown && root.appReady

        function test_01_replays_button_on_home() {
            console.log("Testing: Replays button on home")
            
            tryVerify(function() {
                return mainWindow.currentView === "home"
            }, 2000, "Should be on home view")
            
            var replaysBtn = findChild(mainWindow, E2EConstants.replaysButton)
            verify(replaysBtn !== null, "Replays button should exist")
            verify(replaysBtn.visible, "Replays button should be visible")
            
            console.log("OK Replays button visible")
        }

        function test_02_navigate_to_cache() {
            console.log("Testing: Navigate to cache view")
            
            var replaysBtn = findChild(mainWindow, E2EConstants.replaysButton)
            verify(replaysBtn !== null, "Replays button should exist")
            
            mouseClick(replaysBtn)
            
            tryVerify(function() {
                return mainWindow.currentView === "cache"
            }, 2000, "Should navigate to cache view")
            
            console.log("OK Navigated to cache")
        }

        function test_03_cache_view_active() {
            console.log("Testing: Cache view is active")
            
            verify(mainWindow.currentView === "cache", "Should be on cache view")
            console.log("  currentView = " + mainWindow.currentView)
            
            console.log("OK Cache view active")
        }

        function test_04_nav_buttons_hidden() {
            console.log("Testing: Nav buttons hidden in cache")
            
            verify(mainWindow.currentView === "cache", "Should be on cache view")
            
            // The replays button should be hidden per Plan 28
            var replaysBtn = findChild(mainWindow, E2EConstants.replaysButton)
            if (replaysBtn !== null) {
                console.log("  Replays visible: " + replaysBtn.visible)
                verify(!replaysBtn.visible, "Replays button should be hidden")
            }
            
            console.log("OK Nav buttons hidden")
        }

        function test_05_return_to_home() {
            console.log("Testing: Return to home")
            
            mainWindow.currentView = "home"
            
            tryVerify(function() {
                return mainWindow.currentView === "home"
            }, 2000, "Should return to home")
            
            var replaysBtn = findChild(mainWindow, E2EConstants.replaysButton)
            verify(replaysBtn !== null, "Replays button should exist")
            verify(replaysBtn.visible, "Replays button should be visible")
            
            console.log("OK Returned to home")
        }

        function test_06_preferences_button() {
            console.log("Testing: Preferences button")
            
            var prefsBtn = findChild(mainWindow, E2EConstants.preferencesButton)
            verify(prefsBtn !== null, "Preferences button should exist")
            verify(prefsBtn.visible, "Preferences button should be visible")
            
            console.log("OK Preferences button exists")
        }

        function test_07_navigate_to_preferences() {
            console.log("Testing: Navigate to preferences")
            
            var prefsBtn = findChild(mainWindow, E2EConstants.preferencesButton)
            verify(prefsBtn !== null, "Preferences button should exist")
            
            mouseClick(prefsBtn)
            
            tryVerify(function() {
                return mainWindow.currentView === "preferences"
            }, 2000, "Should navigate to preferences")
            
            // Nav buttons should be hidden
            var prefsBtn2 = findChild(mainWindow, E2EConstants.preferencesButton)
            if (prefsBtn2 !== null) {
                verify(!prefsBtn2.visible, "Preferences button should be hidden in preferences view")
            }
            
            // Return to home
            mainWindow.currentView = "home"
            tryVerify(function() {
                return mainWindow.currentView === "home"
            }, 2000, "Should return to home")
            
            console.log("OK Preferences navigation works")
        }
    }
}
