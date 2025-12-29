import QtQuick
import QtTest
import "." // Import helpers from current directory

/**
 * E2E Scenario: Audio Only Mode (A.5)
 * 
 * Tests the audio-only quality selection:
 * 1. Navigate to player from home
 * 2. Open quality selector
 * 3. Select "Audio" quality
 * 4. Verify audio-only placeholder appears
 * 5. Verify player continues working
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
            
            // Find audio quality option (should be index 3 - after chunked, 720p, 480p)
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
            
            // Wait for quality option to be fully visible and sized
            var optionName = E2EConstants.qualityOptionName(audioOptionIndex)
            tryVerify(function() {
                var opt = findChild(mainWindow, optionName)
                return opt !== null && opt.visible && opt.width > 0 && opt.height > 0
            }, 2000, "Audio quality option should be visible")
            
            // Small delay to ensure the option is fully interactive
            wait(200)
            
            var audioOption = findChild(mainWindow, optionName)
            console.log("  Clicking audio quality option at index " + audioOptionIndex)
            console.log("  Option position: " + audioOption.x + "," + audioOption.y + " size: " + audioOption.width + "x" + audioOption.height)
            mouseClick(audioOption)
            
            // Wait for popup to close and quality to change
            // Note: quality name contains "audio" (e.g., "audio_only" or "Audio")
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

        function test_05_audio_placeholder_visible() {
            console.log("Testing: Audio-only placeholder appears")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            verify(playerView !== null, "Player view should exist")
            
            // Check isAudioOnly property
            console.log("  isAudioOnly: " + playerView.isAudioOnly)
            verify(playerView.isAudioOnly, "Player should be in audio-only mode")
            
            // Wait for audio placeholder to become visible
            tryVerify(function() {
                var placeholder = findChild(mainWindow, E2EConstants.audioOnlyPlaceholder)
                return placeholder !== null && placeholder.visible
            }, 3000, "Audio placeholder should appear")
            
            var placeholder = findChild(mainWindow, E2EConstants.audioOnlyPlaceholder)
            verify(placeholder !== null, "Audio placeholder should exist")
            verify(placeholder.visible, "Audio placeholder should be visible")
            
            console.log("OK Audio-only placeholder is visible")
        }

        function test_06_player_still_works() {
            console.log("Testing: Player still works in audio mode")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            verify(playerView !== null, "Player view should exist")
            verify(!playerView.showError, "Player should not show error in audio mode")
            
            // Verify toast appeared for quality change
            var qualityToast = findChild(mainWindow, E2EConstants.qualityToast)
            if (qualityToast) {
                console.log("  Toast text: " + qualityToast.text)
            }
            
            console.log("OK Player works in audio-only mode")
        }
    }
}
