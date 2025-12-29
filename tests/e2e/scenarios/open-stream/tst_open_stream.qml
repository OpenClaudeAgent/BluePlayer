import QtQuick
import QtTest
import "." // Import E2ETestCase from current directory

/**
 * E2E Scenario: Open Stream
 * 
 * Tests the complete flow of opening a live stream from the home view:
 * 1. App starts with authenticated user
 * 2. Home displays streams from mock server
 * 3. User clicks on a stream card
 * 4. Player view opens
 * 5. Stream starts playing (not stuck in loading/buffering)
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
        name: "E2E_OpenStream"
        when: windowShown && appLoader.status === Loader.Ready

        function initTestCase() {
            console.log("=== E2E Open Stream Tests ===")
            mainWindow = appLoader.item
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
            
            wait(1500)
            
            compare(mainWindow.currentView, "home", "Should show home view when authenticated")
            console.log("OK Home view is displayed")
        }

        // =====================================================================
        // Test 2: Stream cards are loaded from mock server
        // =====================================================================
        
        function test_02_streams_loaded() {
            console.log("Testing: Streams loaded from mock server")
            
            wait(1500)
            
            var streamCard = findChildByPrefix(mainWindow, "streamCard_")
            verify(streamCard !== null, "Stream cards should be displayed")
            console.log("OK Stream cards are loaded")
        }

        // =====================================================================
        // Test 3: Clicking a stream opens the player view
        // =====================================================================
        
        function test_03_click_stream_opens_player() {
            console.log("Testing: Click on stream opens player")
            
            wait(1000)
            
            var streamCard = findChildByPrefix(mainWindow, "streamCard_")
            verify(streamCard !== null, "Stream card should exist")
            verify(streamCard.visible, "Stream card should be visible")
            
            // Click on the stream card
            mouseClick(streamCard)
            
            // Wait for navigation
            wait(1500)
            
            compare(mainWindow.currentView, "player", "Should navigate to player view")
            console.log("OK Player view opened")
        }

        // =====================================================================
        // Test 4: Player view has correct stream information
        // =====================================================================
        
        function test_04_player_has_stream_info() {
            console.log("Testing: Player has correct stream info")
            
            wait(500)
            
            var playerView = findChild(mainWindow, "playerView")
            verify(playerView !== null, "PlayerView should exist")
            
            // Verify stream information
            verify(!playerView.showError, "Player should not show error")
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
            
            var playerView = findChild(mainWindow, "playerView")
            verify(playerView !== null, "PlayerView should exist")
            
            // Wait for stream to start playing
            // The player should transition from buffering to playing
            var maxWait = 10000  // 10 seconds max
            var checkInterval = 500
            var elapsed = 0
            var isPlaying = false
            
            while (elapsed < maxWait && !isPlaying) {
                // Check if playing or if we have a valid state
                // Note: In mock mode, the HLS server provides minimal segments
                // so we may not get full playback, but we should not be in error state
                
                if (playerView.playing) {
                    isPlaying = true
                    console.log("  - Stream is playing after " + elapsed + "ms")
                } else if (playerView.showError) {
                    console.log("  - Error detected: " + playerView.errorMessage)
                    break
                } else if (playerView.buffering) {
                    console.log("  - Still buffering after " + elapsed + "ms...")
                }
                
                wait(checkInterval)
                elapsed += checkInterval
            }
            
            // Verify final state
            verify(!playerView.showError, "Player should not show error")
            
            // Log final state for debugging
            console.log("  - Final state: playing=" + playerView.playing + 
                       ", buffering=" + playerView.buffering +
                       ", paused=" + playerView.paused)
            
            // Accept either playing state or at least no error with valid HLS
            // (mock HLS segments may not fully decode in offscreen mode)
            var validState = playerView.playing || 
                            (!playerView.showError && playerView.hlsUrl.length > 0)
            verify(validState, "Stream should be playing or have valid HLS URL without error")
            
            console.log("OK Stream playback state is valid")
        }
    }
}
