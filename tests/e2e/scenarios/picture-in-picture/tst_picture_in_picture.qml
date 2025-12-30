import QtQuick
import QtTest
import "." // Import helpers from current directory

/**
 * E2E Scenario: Picture-in-Picture (C.4)
 *
 * Tests the PiP button activation and deactivation:
 * 1. Navigate to player view
 * 2. Verify PiP button exists and is enabled
 * 3. Verify PiP is initially inactive
 * 4. Click PiP button -> verify pipActive becomes true
 * 5. Click PiP button again -> verify pipActive becomes false (return)
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
            
            // Wait for home to fully initialize
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
            
            // Wait for player to stabilize
            wait(500)
            
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
            verify(pipButton.visible, "PiP button should be visible")
            
            console.log("OK PiP button exists and visible")
        }

        function test_03_pip_initially_inactive() {
            console.log("Testing: PiP initially inactive")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            verify(playerView !== null, "Player view should exist")
            
            console.log("  pipActive: " + playerView.pipActive)
            verify(playerView.pipActive === false, "PiP should be inactive initially")
            
            console.log("OK PiP initially inactive")
        }

        function test_04_activate_pip() {
            console.log("Testing: Activate PiP")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var pipButton = findChild(mainWindow, E2EConstants.pipButton)
            if (pipButton === null) {
                skip("PiP button not found")
                return
            }
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            verify(playerView !== null, "Player view should exist")
            
            console.log("  pipActive before click: " + playerView.pipActive)
            console.log("  Clicking PiP button to activate...")
            
            mouseClick(pipButton)
            
            // Wait for pipActive to become true
            tryVerify(function() {
                return playerView.pipActive === true
            }, 2000, "PiP should become active after click")
            
            console.log("  pipActive after click: " + playerView.pipActive)
            verify(playerView.pipActive === true, "pipActive should be true")
            
            console.log("OK PiP activated")
        }

        function test_05_deactivate_pip() {
            console.log("Testing: Deactivate PiP (return to normal)")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var pipButton = findChild(mainWindow, E2EConstants.pipButton)
            if (pipButton === null) {
                skip("PiP button not found")
                return
            }
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            verify(playerView !== null, "Player view should exist")
            
            // Should be active from previous test
            console.log("  pipActive before click: " + playerView.pipActive)
            
            if (!playerView.pipActive) {
                console.log("  PiP not active, activating first...")
                mouseClick(pipButton)
                tryVerify(function() {
                    return playerView.pipActive === true
                }, 2000, "PiP should become active")
            }
            
            console.log("  Clicking PiP button to deactivate...")
            mouseClick(pipButton)
            
            // Wait for pipActive to become false
            tryVerify(function() {
                return playerView.pipActive === false
            }, 2000, "PiP should become inactive after second click")
            
            console.log("  pipActive after click: " + playerView.pipActive)
            verify(playerView.pipActive === false, "pipActive should be false")
            
            console.log("OK PiP deactivated (returned to normal)")
        }

        function test_06_pip_button_still_works() {
            console.log("Testing: PiP button still functional after toggle")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var pipButton = findChild(mainWindow, E2EConstants.pipButton)
            verify(pipButton !== null, "PiP button should still exist")
            verify(pipButton.visible, "PiP button should still be visible")
            verify(pipButton.enabled, "PiP button should still be enabled")
            
            console.log("OK PiP button still functional")
        }
    }
}
