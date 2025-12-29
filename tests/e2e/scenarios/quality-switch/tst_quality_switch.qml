import QtQuick
import QtTest
import "." // Import helpers from current directory

/**
 * E2E Scenario: Quality Switch (A.3)
 * 
 * Tests the quality selector in player view:
 * 1. Navigate to player from home
 * 2. Click on quality button
 * 3. Verify popup opens with quality options
 * 4. Click on a different quality
 * 5. Verify quality changed
 * 
 * Context: AuthenticatedSetup
 */
E2EScenarioTemplate {
    id: root

    E2ETestCase {
        id: testCase
        name: "E2E_QualitySwitch"
        when: windowShown && root.appReady

        // Store initial quality for comparison
        property string initialQuality: ""

        // Uses default initTestCase() and cleanupTestCase() from E2ETestCase base

        // =====================================================================
        // Test 1: Navigate to player view
        // =====================================================================
        
        function test_01_navigate_to_player() {
            console.log("Testing: Navigate to player")
            
            // Wait for home to fully initialize (API calls, rendering, etc.)
            wait(E2EConstants.timeoutMedium)
            
            var streamCard = findChildByPrefix(mainWindow, E2EConstants.streamCardPrefix)
            verify(streamCard !== null, "Stream card should exist on home")
            
            console.log("  Clicking stream card: " + streamCard.objectName)
            
            // Use clickAndWait helper with navigation condition
            var navigated = clickAndWait(streamCard, function() {
                return mainWindow.currentView === "player"
            }, E2EConstants.timeoutLong)
            
            verify(navigated, "Should navigate to player view")
            console.log("OK Navigated to player view")
        }

        // =====================================================================
        // Test 2: Quality button exists and is clickable
        // =====================================================================
        
        function test_02_quality_button_exists() {
            console.log("Testing: Quality button exists")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            // Wait for quality button to appear (player fully loaded)
            tryVerify(function() {
                var btn = findChild(mainWindow, E2EConstants.qualityButton)
                return btn !== null && btn.visible
            }, 3000, "Quality button should appear")
            
            var qualityButton = findChild(mainWindow, E2EConstants.qualityButton)
            verify(qualityButton !== null, "Quality button should exist")
            
            console.log("  Quality button found, visible: " + qualityButton.visible)
            console.log("OK Quality button exists")
        }

        // =====================================================================
        // Test 3: Click quality button opens popup
        // =====================================================================
        
        function test_03_click_quality_button_opens_popup() {
            console.log("Testing: Click quality button opens popup")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var qualityButton = findChild(mainWindow, E2EConstants.qualityButton)
            if (!qualityButton) {
                skip("Quality button not found")
                return
            }
            
            // Get quality control to check popup state
            var qualityControl = findChild(mainWindow, E2EConstants.qualityControl)
            verify(qualityControl !== null, "Quality control should exist")
            
            console.log("  Initial popup state: " + qualityControl.showPopup)
            
            // Click on quality button
            console.log("  Clicking quality button...")
            mouseClick(qualityButton)
            
            // Wait for popup to open
            tryVerify(function() {
                return qualityControl.showPopup === true
            }, 1000, "Quality popup should open")
            
            // Verify popup is now shown
            console.log("  Popup state after click: " + qualityControl.showPopup)
            verify(qualityControl.showPopup, "Quality popup should be open after click")
            
            // Check that quality options exist
            var qualityOption0 = findChild(mainWindow, E2EConstants.qualityOptionName(0))
            console.log("  Quality option 0 found: " + (qualityOption0 !== null))
            verify(qualityOption0 !== null, "Should have at least one quality option")
            
            console.log("OK Quality popup opened with options")
        }

        // =====================================================================
        // Test 4: Click on different quality changes selection
        // =====================================================================
        
        function test_04_select_different_quality() {
            console.log("Testing: Select different quality")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var qualityControl = findChild(mainWindow, E2EConstants.qualityControl)
            if (!qualityControl) {
                skip("Quality control not found")
                return
            }
            
            // Make sure popup is open
            if (!qualityControl.showPopup) {
                var qualityButton = findChild(mainWindow, E2EConstants.qualityButton)
                if (qualityButton) {
                    console.log("  Re-opening popup...")
                    mouseClick(qualityButton)
                    tryVerify(function() {
                        return qualityControl.showPopup === true
                    }, 2000, "Quality popup should open")
                }
            }
            
            // Store initial quality
            initialQuality = qualityControl.currentQuality
            console.log("  Initial quality: " + initialQuality)
            console.log("  Available qualities: " + qualityControl.qualities.length)
            
            // Log each quality for debugging
            for (var i = 0; i < qualityControl.qualities.length; i++) {
                var q = qualityControl.qualities[i]
                console.log("    Quality " + i + ": " + JSON.stringify(q))
            }
            
            // Wait for quality option to be fully visible and ready
            tryVerify(function() {
                var opt = findChild(mainWindow, E2EConstants.qualityOptionName(2))
                return opt !== null && opt.visible && opt.width > 0 && opt.height > 0
            }, 2000, "Quality option 2 should be visible and sized")
            
            // Click on 480p (index 2) to get SD button
            var qualityOption2 = findChild(mainWindow, E2EConstants.qualityOptionName(2))
            if (!qualityOption2) {
                console.log("  qualityOption_2 not found, trying option 1")
                qualityOption2 = findChild(mainWindow, E2EConstants.qualityOptionName(1))
            }
            
            if (!qualityOption2) {
                skip("No quality option found to click")
                return
            }
            
            console.log("  Clicking 480p quality option...")
            mouseClick(qualityOption2)
            
            // Wait for popup to close first
            tryVerify(function() {
                return qualityControl.showPopup === false
            }, 2000, "Popup should close after selection")
            
            // Then wait for quality to change
            tryVerify(function() {
                return qualityControl.currentQuality === "480p"
            }, 2000, "Quality should change to 480p")
            
            // Verify popup closed
            console.log("  Popup state after selection: " + qualityControl.showPopup)
            verify(!qualityControl.showPopup, "Popup should close after selection")
            
            // Verify quality changed to 480p
            console.log("  New quality: " + qualityControl.currentQuality)
            verify(qualityControl.currentQuality === "480p", "Quality should be 480p")
            
            // Verify button shows SD (480p is SD quality)
            var qualityButtonText = findChild(mainWindow, E2EConstants.qualityButtonText)
            verify(qualityButtonText !== null, "Quality button text should exist")
            console.log("  Button text: " + qualityButtonText.text)
            verify(qualityButtonText.text === "SD", "Button should show SD for 480p")
            
            console.log("OK Quality switched to 480p (SD)")
        }

        // =====================================================================
        // Test 5: Player still works after quality change
        // =====================================================================
        
        function test_05_player_works_after_quality_change() {
            console.log("Testing: Player works after quality change")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var playerView = findChild(mainWindow, "playerView")
            verify(playerView !== null, "Player view should exist")
            verify(!playerView.showError, "Player should not show error after quality change")
            
            console.log("OK Player still works after quality change")
        }
    }
}
