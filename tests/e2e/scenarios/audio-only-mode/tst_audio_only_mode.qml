import QtQuick
import QtTest
import "." // Import helpers from current directory

/**
 * E2E Scenario: Audio Only Mode (A.5)
 * 
 * Tests the audio-only quality selection flow:
 * 1. Navigate to player from home
 * 2. Open quality selector
 * 3. Verify audio option is available
 * 4. Select "Audio" quality
 * 5. Verify player switches to audio-only mode
 * 
 * Note: In test environment, the audio segment may not be valid,
 * so we verify the UI state (isAudioOnly) rather than actual playback.
 * 
 * Context: AuthenticatedSetup
 */
E2EScenarioTemplate {
    id: root

    E2ETestCase {
        id: testCase
        name: "E2E_AudioOnlyMode"
        when: windowShown && root.appReady

        function test_01_navigate_to_player() {
            console.log("Testing: Navigate to player")
            
            // Wait for home to fully initialize
            wait(E2EConstants.timeoutMedium)
            
            var streamCard = findChildByPrefix(mainWindow, "streamCard_")
            verify(streamCard !== null, "Stream card should exist on home")
            
            console.log("  Clicking stream card: " + streamCard.objectName)
            
            var navigated = clickAndWait(streamCard, function() {
                return mainWindow.currentView === "player"
            }, E2EConstants.timeoutLong)
            
            verify(navigated, "Should navigate to player view")
            
            // Wait for player to stabilize
            wait(1000)
            
            console.log("OK Navigated to player view")
        }

        function test_02_quality_button_exists() {
            console.log("Testing: Quality button exists")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            tryVerify(function() {
                var btn = findChild(mainWindow, E2EConstants.qualityButton)
                return btn !== null && btn.visible
            }, 3000, "Quality button should appear")
            
            var qualityButton = findChild(mainWindow, E2EConstants.qualityButton)
            verify(qualityButton !== null, "Quality button should exist")
            
            console.log("OK Quality button exists")
        }

        function test_03_audio_quality_available() {
            console.log("Testing: Audio quality is available in selector")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var qualityControl = findChild(mainWindow, E2EConstants.qualityControl)
            verify(qualityControl !== null, "Quality control should exist")
            
            // Open the quality popup
            var qualityButton = findChild(mainWindow, E2EConstants.qualityButton)
            mouseClick(qualityButton)
            
            tryVerify(function() {
                return qualityControl.showPopup === true
            }, 2000, "Quality popup should open")
            
            // Check that we have audio_only quality in the list
            console.log("  Available qualities: " + qualityControl.qualities.length)
            
            var hasAudioOnly = false
            var audioIndex = -1
            for (var i = 0; i < qualityControl.qualities.length; i++) {
                var q = qualityControl.qualities[i]
                console.log("    Quality " + i + ": " + q.name + " (" + q.value + ")")
                if (q.value === "audio_only" || q.name.toLowerCase().indexOf("audio") !== -1) {
                    hasAudioOnly = true
                    audioIndex = i
                }
            }
            
            verify(hasAudioOnly, "Audio quality option should be available")
            console.log("OK Audio quality found at index " + audioIndex)
        }

        function test_04_select_audio_quality() {
            console.log("Testing: Select audio quality")
            
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
                mouseClick(qualityButton)
                tryVerify(function() {
                    return qualityControl.showPopup === true
                }, 2000, "Quality popup should open")
            }
            
            // Find audio quality option
            var audioOptionIndex = -1
            for (var i = 0; i < qualityControl.qualities.length; i++) {
                if (qualityControl.qualities[i].value === "audio_only") {
                    audioOptionIndex = i
                    break
                }
            }
            
            if (audioOptionIndex === -1) {
                skip("Audio quality option not found")
                return
            }
            
            // Wait for quality option to be fully visible
            var optionName = E2EConstants.qualityOptionName(audioOptionIndex)
            tryVerify(function() {
                var opt = findChild(mainWindow, optionName)
                return opt !== null && opt.visible && opt.width > 0 && opt.height > 0
            }, 2000, "Audio quality option should be visible")
            
            wait(200)
            
            var audioOption = findChild(mainWindow, optionName)
            console.log("  Clicking audio quality option at index " + audioOptionIndex)
            mouseClick(audioOption)
            
            // Wait for popup to close
            tryVerify(function() {
                return qualityControl.showPopup === false
            }, 3000, "Popup should close after selection")
            
            // Verify quality changed to audio
            tryVerify(function() {
                var q = qualityControl.currentQuality.toLowerCase()
                return q.indexOf("audio") !== -1
            }, 2000, "Quality should change to audio")
            
            console.log("  New quality: " + qualityControl.currentQuality)
            console.log("OK Audio quality selected")
        }

        function test_05_audio_mode_activated() {
            console.log("Testing: Audio mode is activated")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            verify(playerView !== null, "Player view should exist")
            
            // Wait for isAudioOnly to become true
            tryVerify(function() {
                return playerView.isAudioOnly === true
            }, 3000, "Player should be in audio-only mode")
            
            console.log("  isAudioOnly: " + playerView.isAudioOnly)
            verify(playerView.isAudioOnly, "isAudioOnly should be true")
            
            console.log("OK Audio mode is activated")
        }

        function test_06_quality_display_updated() {
            console.log("Testing: Quality display shows Audio")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var qualityControl = findChild(mainWindow, E2EConstants.qualityControl)
            if (!qualityControl) {
                skip("Quality control not found")
                return
            }
            
            // Verify the button shows the audio quality
            var q = qualityControl.currentQuality.toLowerCase()
            console.log("  Current quality: " + qualityControl.currentQuality)
            verify(q.indexOf("audio") !== -1, "Quality should show audio")
            
            // Check the button text
            var buttonText = findChild(mainWindow, E2EConstants.qualityButtonText)
            if (buttonText) {
                console.log("  Button text: " + buttonText.text)
            }
            
            console.log("OK Quality display updated")
        }
    }
}
