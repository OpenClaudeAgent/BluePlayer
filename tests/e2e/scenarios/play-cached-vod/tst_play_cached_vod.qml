import QtQuick
import QtTest
import "." // Import helpers from current directory

/**
 * E2E Scenario: Play Cached VOD (A.2)
 *
 * Tests the cached VOD / Replays functionality:
 * 1. Replays button visibility on home
 * 2. Navigation to cache view
 * 3. Inject mock VOD into cache
 * 4. Verify VOD card appears
 * 5. Click VOD card -> navigate to player
 * 6. Verify VOD playback mode
 *
 * Context: AuthenticatedSetup
 */
E2EScenarioTemplate {
    id: root

    // Mock VOD data for testing - uses real test_segment.ts from fixtures
    readonly property string mockVodFilePath: {
        // E2E_FIXTURES_PATH is exposed by BaseE2EContext
        if (typeof E2E_FIXTURES_PATH !== "undefined" && E2E_FIXTURES_PATH) {
            return E2E_FIXTURES_PATH + "/test_segment.ts"
        }
        return "/tmp/e2e_mock_vod.ts" // Fallback
    }

    readonly property var mockVodData: ({
        "streamerLogin": "teststreamer",
        "streamerName": "TestStreamer",
        "streamTitle": "Mock VOD for E2E Testing",
        "filePath": mockVodFilePath,
        "duration": 3600,
        "gameCategory": "Just Chatting",
        "thumbnailPath": ""
    })

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

        function test_02_inject_mock_vod() {
            console.log("Testing: Inject mock VOD into cache")
            
            // Verify cacheManager is available
            verify(typeof cacheManager !== "undefined", "cacheManager should be defined")
            verify(cacheManager !== null, "cacheManager should not be null")
            
            var initialCount = cacheManager.vodCount
            console.log("  Initial VOD count: " + initialCount)
            
            // Inject mock VOD
            var result = cacheManager.addVodFromQml(root.mockVodData)
            console.log("  addVodFromQml result: " + result)
            
            // Verify VOD was added
            tryVerify(function() {
                return cacheManager.vodCount > initialCount
            }, 2000, "VOD count should increase")
            
            console.log("  New VOD count: " + cacheManager.vodCount)
            verify(cacheManager.vodCount > initialCount, "VOD should be added")
            
            console.log("OK Mock VOD injected")
        }

        function test_03_navigate_to_cache() {
            console.log("Testing: Navigate to cache view")
            
            var replaysBtn = findChild(mainWindow, E2EConstants.replaysButton)
            verify(replaysBtn !== null, "Replays button should exist")
            
            mouseClick(replaysBtn)
            
            tryVerify(function() {
                return mainWindow.currentView === "cache"
            }, 2000, "Should navigate to cache view")
            
            console.log("OK Navigated to cache")
        }

        function test_04_find_vod_card() {
            console.log("Testing: Find VOD card in cache view")
            
            verify(mainWindow.currentView === "cache", "Should be on cache view")
            
            // Verify cacheManager has VODs
            console.log("  cacheManager.vodCount: " + cacheManager.vodCount)
            verify(cacheManager.vodCount > 0, "Cache should have VODs")
            
            // Wait for VOD card to be rendered
            tryVerify(function() {
                var card = findChild(mainWindow, E2EConstants.vodCardPrefix + "0")
                return card !== null
            }, 5000, "VOD card should be rendered")
            
            var vodCard = findChild(mainWindow, E2EConstants.vodCardPrefix + "0")
            verify(vodCard !== null, "VOD card should exist")
            console.log("  Found VOD card: " + vodCard.objectName)
            
            console.log("OK VOD card found")
        }

        function test_05_click_vod_card() {
            console.log("Testing: Click VOD card to play")
            
            verify(mainWindow.currentView === "cache", "Should be on cache view")
            
            // Find VOD card
            var vodCard = findChild(mainWindow, E2EConstants.vodCardPrefix + "0")
            verify(vodCard !== null, "VOD card should exist")
            
            console.log("  Clicking VOD card...")
            mouseClick(vodCard)
            
            // Should navigate to player view
            tryVerify(function() {
                return mainWindow.currentView === "player"
            }, 3000, "Should navigate to player view")
            
            verify(mainWindow.currentView === "player", "Should be on player view")
            
            console.log("OK Navigated to player via VOD card click")
        }

        function test_06_player_in_vod_mode() {
            console.log("Testing: Player is in VOD mode")
            
            if (!requiresView("player")) return
            
            // Wait for player to initialize
            tryVerify(function() {
                var playerView = findChild(mainWindow, E2EConstants.playerView)
                return playerView !== null
            }, 3000, "Player view should load")
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            verify(playerView !== null, "Player view should exist")
            
            console.log("  isVodMode: " + playerView.isVodMode)
            console.log("  vodFilePath: " + playerView.vodFilePath)
            
            verify(playerView.isVodMode === true, "Player should be in VOD mode")
            verify(playerView.vodFilePath.length > 0, "vodFilePath should be set")
            
            console.log("OK Player in VOD mode")
        }

        function test_07_click_back_button() {
            console.log("Testing: Click back button to return to cache")
            
            if (!requiresView("player")) return
            
            // Make controls visible
            showPlayerControls()
            
            // Find back button (now visible)
            tryVerify(function() {
                var btn = findChild(mainWindow, E2EConstants.backButton)
                return btn !== null && btn.visible
            }, 3000, "Back button should be visible")
            
            var backButton = findChild(mainWindow, E2EConstants.backButton)
            verify(backButton !== null, "Back button should exist")
            
            console.log("  Clicking back button...")
            mouseClick(backButton)
            
            // Should return to cache view (since we came from VOD playback)
            tryVerify(function() {
                return mainWindow.currentView === "cache"
            }, 3000, "Should return to cache view")
            
            verify(mainWindow.currentView === "cache", "Should be on cache view")
            
            console.log("OK Returned to cache via back button")
        }
    }
}
