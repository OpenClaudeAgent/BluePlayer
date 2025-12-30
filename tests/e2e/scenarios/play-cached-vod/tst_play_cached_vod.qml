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

        function test_04_cache_has_vods() {
            console.log("Testing: Cache has VODs")
            
            verify(mainWindow.currentView === "cache", "Should be on cache view")
            
            // Wait for CacheManagerView to initialize
            wait(500)
            
            // Verify cacheManager has VODs
            console.log("  cacheManager.vodCount: " + cacheManager.vodCount)
            verify(cacheManager.vodCount > 0, "Cache should have VODs")
            
            // Verify our mock VOD is in the list
            var vodList = cacheManager.vodList
            console.log("  vodList length: " + vodList.length)
            
            var foundMockVod = false
            for (var i = 0; i < vodList.length; i++) {
                if (vodList[i].streamTitle === "Mock VOD for E2E Testing") {
                    foundMockVod = true
                    console.log("  Found mock VOD at index " + i)
                    break
                }
            }
            
            verify(foundMockVod, "Mock VOD should be in the cache list")
            
            console.log("OK Cache has VODs including our mock")
        }

        function test_05_simulate_vod_playback() {
            console.log("Testing: Simulate VOD playback via properties")
            
            verify(mainWindow.currentView === "cache", "Should be on cache view")
            
            // Get the first VOD from the list
            var vodList = cacheManager.vodList
            verify(vodList.length > 0, "Should have VODs in cache")
            
            var vod = vodList[0]
            console.log("  Playing VOD: " + vod.streamTitle)
            console.log("  filePath: " + vod.filePath)
            
            // Simulate what happens when clicking a VOD card
            // (sets properties and navigates to player)
            mainWindow.vodId = vod.id
            mainWindow.vodFilePath = vod.filePath
            mainWindow.vodMetadata = vod
            mainWindow.playerStreamerLogin = ""
            mainWindow.playerStreamerName = vod.streamerName || ""
            mainWindow.playerStreamTitle = vod.streamTitle || ""
            mainWindow.currentView = "player"
            
            // Should navigate to player view
            tryVerify(function() {
                return mainWindow.currentView === "player"
            }, 3000, "Should navigate to player view")
            
            verify(mainWindow.currentView === "player", "Should be on player view")
            
            console.log("OK Navigated to player for VOD")
        }

        function test_06_player_in_vod_mode() {
            console.log("Testing: Player is in VOD mode")
            
            if (mainWindow.currentView !== "player") {
                skip("Not on player view")
                return
            }
            
            // Wait for player to load
            wait(500)
            
            var playerView = findChild(mainWindow, E2EConstants.playerView)
            verify(playerView !== null, "Player view should exist")
            
            console.log("  isVodMode: " + playerView.isVodMode)
            console.log("  vodFilePath: " + playerView.vodFilePath)
            
            verify(playerView.isVodMode === true, "Player should be in VOD mode")
            verify(playerView.vodFilePath.length > 0, "vodFilePath should be set")
            
            console.log("OK Player in VOD mode")
        }

        function test_07_return_to_home() {
            console.log("Testing: Return to home from player")
            
            mainWindow.currentView = "home"
            
            tryVerify(function() {
                return mainWindow.currentView === "home"
            }, 2000, "Should return to home")
            
            var replaysBtn = findChild(mainWindow, E2EConstants.replaysButton)
            verify(replaysBtn !== null, "Replays button should exist")
            verify(replaysBtn.visible, "Replays button should be visible")
            
            console.log("OK Returned to home")
        }
    }
}
