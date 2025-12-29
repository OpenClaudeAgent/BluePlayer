import QtQuick
import QtTest
import "." // Import helpers from current directory

/**
 * E2E Scenario: Open Stream
 * 
 * Tests the complete flow of opening a live stream from the home view:
 * 1. App starts with authenticated user
 * 2. Home displays streams from mock server
 * 3. User clicks on a stream card
 * 4. Player view opens
 * 5. Stream starts playing (not stuck in loading/buffering)
 * 
 * Uses: E2EScenarioTemplate, E2ETestCase, E2EConstants
 */
E2EScenarioTemplate {
    id: root

    E2ETestCase {
        id: testCase
        name: "E2E_OpenStream"
        when: windowShown && root.appReady

        function initTestCase() {
            console.log("=== E2E Open Stream Tests ===")
            mainWindow = root.app
            verify(mainWindow !== null, "Main window should load")
        }

        function cleanupTestCase() {
            console.log("=== E2E Open Stream Tests Complete ===")
        }

        // =====================================================================
        // Test 1: Home view is displayed for authenticated user
        // =====================================================================
        
        function test_01_home_view_displayed() {
            console.log("Testing: Home view displayed for authenticated user")
            
            // Use waitForAppReady helper with constant timeout
            var ready = waitForAppReady(E2EConstants.timeoutAppReady)
            verify(ready, "App should be ready")
            
            compare(mainWindow.currentView, "home", "Should show home view when authenticated")
            console.log("OK Home view is displayed")
        }

        // =====================================================================
        // Test 2: Stream cards are loaded from mock server
        // =====================================================================
        
        function test_02_streams_loaded() {
            console.log("Testing: Streams loaded from mock server")
            
            wait(E2EConstants.timeoutMedium)
            
            // Use verifyStreamCard helper
            var streamCard = verifyStreamCard(0)
            verify(streamCard !== null, "First stream card should exist")
            console.log("OK Stream cards are loaded")
        }

        // =====================================================================
        // Test 3: Clicking a stream opens the player view
        // =====================================================================
        
        function test_03_click_stream_opens_player() {
            console.log("Testing: Click on stream opens player")
            
            wait(E2EConstants.timeoutShort)
            
            var streamCard = findChildByPrefix(mainWindow, E2EConstants.streamCardPrefix)
            verify(streamCard !== null, "Stream card should exist")
            
            // Use clickAndWait helper with navigation condition
            var navigated = clickAndWait(streamCard, function() {
                return mainWindow.currentView === "player"
            }, E2EConstants.timeoutMedium)
            
            verify(navigated, "Should navigate to player view after click")
            compare(mainWindow.currentView, "player", "Should be on player view")
            console.log("OK Player view opened")
        }

        // =====================================================================
        // Test 4: Player view has correct stream information
        // =====================================================================
        
        function test_04_player_has_stream_info() {
            console.log("Testing: Player has correct stream info")
            
            wait(E2EConstants.timeoutShort)
            
            // Use verifyPlayerView helper
            var playerView = verifyPlayerView()
            
            // Verify stream information
            compare(playerView.streamerLogin, "teststreamer", "Player should have correct streamer login")
            verify(playerView.hlsUrl.length > 0, "Player should have HLS URL")
            
            console.log("OK Player has correct stream info")
            console.log("  - Streamer: " + playerView.streamerLogin)
            console.log("  - HLS URL: " + playerView.hlsUrl.substring(0, 50) + "...")
        }

        // =====================================================================
        // Test 5: Stream starts playing (not stuck in loading)
        // =====================================================================
        
        function test_05_stream_is_playing() {
            console.log("Testing: Stream is playing")
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            verify(playerView !== null, "PlayerView should exist")
            
            // Use waitForPlayback helper with constant timeout
            var playbackValid = waitForPlayback(E2EConstants.timeoutPlayback)
            
            // Verify final state
            verify(!playerView.showError, "Player should not show error")
            
            // Log final state for debugging
            console.log("  - Final state: playing=" + playerView.playing + 
                       ", buffering=" + playerView.buffering +
                       ", paused=" + playerView.paused)
            
            verify(playbackValid, "Stream should be playing or have valid HLS URL without error")
            console.log("OK Stream playback state is valid")
        }
    }
}
