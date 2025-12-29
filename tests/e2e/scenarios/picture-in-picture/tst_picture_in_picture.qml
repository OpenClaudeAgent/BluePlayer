import QtQuick
import QtTest
import "." // Import helpers from current directory

/**
 * E2E Scenario: Picture-in-Picture (C.4)
 *
 * Tests the PiP button and state:
 * 1. Navigate to player view
 * 2. Verify PiP button exists and is enabled
 * 3. Verify PiP state properties
 *
 * Note: Actual PiP window behavior may be limited in offscreen mode.
 *
 * Context: AuthenticatedSetup
 */
E2EScenarioTemplate {
    id: root

    E2ETestCase {
        id: testCase
        name: "E2E_PictureInPicture"
        when: windowShown && root.appReady

        function test_01_navigate_to_player() {
            console.log("Testing: Navigate to player")
            
            // Wait for home to fully initialize (match audio-only-mode pattern)
            wait(E2EConstants.timeoutMedium)
            
            verify(mainWindow.currentView === "home", "Should be on home view")
            
            // Find a stream card
            var streamCard = findChildByPrefix(mainWindow, E2EConstants.streamCardPrefix)
            verify(streamCard !== null, "Stream card should exist")
            
            console.log("  Clicking stream card: " + streamCard.objectName)
            
            var navigated = clickAndWait(streamCard, function() {
                return mainWindow.currentView === "player"
            }, E2EConstants.timeoutLong)
            
            verify(navigated, "Should navigate to player view")
            console.log("OK Navigated to player")
        }

        function test_02_pip_button_exists() {
            console.log("Testing: PiP button exists")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            // Wait for PiP button to appear
            tryVerify(function() {
                var btn = findChild(mainWindow, E2EConstants.pipButton)
                return btn !== null && btn.visible
            }, 3000, "PiP button should appear")
            
            var pipButton = findChild(mainWindow, E2EConstants.pipButton)
            verify(pipButton !== null, "PiP button should exist")
            
            console.log("OK PiP button exists")
        }

        function test_03_pip_button_enabled() {
            console.log("Testing: PiP button is enabled")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var pipButton = findChild(mainWindow, E2EConstants.pipButton)
            if (pipButton === null) {
                skip("PiP button not found")
                return
            }
            
            console.log("  pipButton.enabled: " + pipButton.enabled)
            verify(pipButton.enabled, "PiP button should be enabled")
            
            console.log("OK PiP button enabled")
        }

        function test_04_pip_not_active() {
            console.log("Testing: PiP not active initially")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var pipButton = findChild(mainWindow, E2EConstants.pipButton)
            if (pipButton === null) {
                skip("PiP button not found")
                return
            }
            
            console.log("  pipButton.active: " + pipButton.active)
            verify(!pipButton.active, "PiP should not be active initially")
            
            console.log("OK PiP not active")
        }

        function test_05_pip_button_click() {
            console.log("Testing: PiP button click")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var pipButton = findChild(mainWindow, E2EConstants.pipButton)
            if (pipButton === null || !pipButton.enabled) {
                skip("PiP button not available")
                return
            }
            
            console.log("  Clicking PiP button")
            mouseClick(pipButton)
            
            // In offscreen mode, PiP window may not open
            // Just verify the click doesn't cause errors
            wait(300)
            
            console.log("  pipButton.active after click: " + pipButton.active)
            
            console.log("OK PiP button clickable")
        }

        function test_06_player_pip_properties() {
            console.log("Testing: Player PiP properties")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            if (playerView === null) {
                skip("Player view not found")
                return
            }
            
            console.log("  pipActive: " + playerView.pipActive)
            console.log("  pipEnabled: " + playerView.pipEnabled)
            
            verify(playerView.pipEnabled !== undefined, "pipEnabled should exist")
            
            console.log("OK Player PiP properties accessible")
        }
    }
}
