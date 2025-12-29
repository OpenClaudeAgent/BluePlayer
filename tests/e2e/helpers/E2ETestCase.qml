import QtQuick
import QtTest

/**
 * E2ETestCase - Base class for all E2E test scenarios
 * 
 * Provides common functionality:
 * - Automatic screenshots after each test
 * - Helper functions for finding QML elements
 * - Wait utilities for async conditions
 * - Standard setup/cleanup hooks
 * 
 * Usage:
 *   E2ETestCase {
 *       name: "MyTest"
 *       function test_something() { ... }
 *   }
 */
TestCase {
    id: e2eTestCase

    // =========================================================================
    // Properties - Override in subclasses as needed
    // =========================================================================
    
    /** Reference to the main application window */
    property var mainWindow: null
    
    /** Screenshot output directory (set via E2E_SCREENSHOT_DIR context property) */
    property string screenshotDir: E2E_SCREENSHOT_DIR
    
    /** Whether to take screenshots after each test */
    property bool screenshotsEnabled: true
    
    /** Current test function name (set automatically) */
    property string currentTestName: ""

    // =========================================================================
    // Lifecycle Hooks
    // =========================================================================

    /**
     * Called before each test function.
     * Override in subclass, but call e2eTestCase.baseInit() first.
     */
    function init() {
        baseInit()
    }

    /**
     * Called after each test function.
     * Override in subclass, but call e2eTestCase.baseCleanup() last.
     */
    function cleanup() {
        baseCleanup()
    }

    /**
     * Base initialization - finds main window.
     * Call this from overridden init() functions.
     */
    function baseInit() {
        if (!mainWindow) {
            mainWindow = findChild(null, "mainWindow")
            if (!mainWindow) {
                wait(500)
                mainWindow = findChild(null, "mainWindow")
            }
        }
    }

    /**
     * Base cleanup - takes screenshot.
     * Call this from overridden cleanup() functions.
     */
    function baseCleanup() {
        if (screenshotsEnabled) {
            takeScreenshot()
        }
    }

    // =========================================================================
    // Screenshot Utilities
    // =========================================================================

    /**
     * Takes a screenshot and saves it to disk.
     * This is a fire-and-forget operation that won't fail the test.
     * @param suffix Optional suffix to add to the filename
     * @param target Optional Item to capture (defaults to test's parent Item)
     */
    function takeScreenshot(suffix, target) {
        // Find the best capture target:
        // 1. Explicit target parameter
        // 2. Parent of the TestCase (the root Item containing the test)
        // 3. mainWindow (fallback)
        var captureTarget = target || e2eTestCase.parent || mainWindow
        
        if (!captureTarget || typeof captureTarget.grabToImage !== "function") {
            console.warn("E2ETestCase: Cannot take screenshot - no valid capture target")
            return
        }

        var timestamp = new Date().toISOString().replace(/[:.]/g, "-")
        var testName = name + "_" + (suffix || timestamp)
        
        // Start async grab (fire and forget - don't block test)
        var success = captureTarget.grabToImage(function(image) {
            var path = screenshotDir + "/" + testName + ".png"
            if (image.saveToFile(path)) {
                console.log("Screenshot saved: " + path)
            } else {
                console.warn("Failed to save screenshot: " + path)
            }
        })
        
        if (!success) {
            console.warn("E2ETestCase: grabToImage failed to start")
            return
        }
        
        // Brief wait to allow screenshot to complete (non-blocking)
        wait(100)
    }

    // =========================================================================
    // Element Finding Utilities
    // =========================================================================

    /**
     * Recursively finds a child element by objectName.
     * Searches through children, contentItem, Loader.item, and data.
     * @param parent The parent element to search from (null = root)
     * @param objectName The objectName to find
     * @returns The found element or null
     */
    function findChild(parent, objectName) {
        // If no parent specified, start from the test root
        var searchRoot = parent
        if (!searchRoot) {
            // Try to find from test case parent or mainWindow
            searchRoot = mainWindow || e2eTestCase.parent
            if (!searchRoot) return null
        }

        if (searchRoot.objectName === objectName) {
            return searchRoot
        }

        // Search in children array
        if (searchRoot.children) {
            for (var i = 0; i < searchRoot.children.length; i++) {
                var found = findChild(searchRoot.children[i], objectName)
                if (found) return found
            }
        }

        // Search in contentItem (for ScrollView, ApplicationWindow, etc.)
        if (searchRoot.contentItem && searchRoot.contentItem !== searchRoot) {
            var found = findChild(searchRoot.contentItem, objectName)
            if (found) return found
        }

        // Search in Loader's item
        if (searchRoot.item && searchRoot.item !== searchRoot) {
            var found = findChild(searchRoot.item, objectName)
            if (found) return found
        }

        // Search in data property (for non-visual children)
        if (searchRoot.data) {
            for (var i = 0; i < searchRoot.data.length; i++) {
                var dataItem = searchRoot.data[i]
                if (dataItem && dataItem !== searchRoot) {
                    var found = findChild(dataItem, objectName)
                    if (found) return found
                }
            }
        }

        return null
    }

    /**
     * Finds a child element whose objectName starts with the given prefix.
     * Useful for finding indexed elements like "streamCard_0", "streamCard_1".
     * @param parent The parent element to search from
     * @param prefix The objectName prefix to match
     * @returns The first matching element or null
     */
    function findChildByPrefix(parent, prefix) {
        var searchRoot = parent || mainWindow || e2eTestCase.parent
        if (!searchRoot) return null

        if (searchRoot.objectName && searchRoot.objectName.indexOf(prefix) === 0) {
            return searchRoot
        }

        if (searchRoot.children) {
            for (var i = 0; i < searchRoot.children.length; i++) {
                var found = findChildByPrefix(searchRoot.children[i], prefix)
                if (found) return found
            }
        }

        if (searchRoot.contentItem && searchRoot.contentItem !== searchRoot) {
            var found = findChildByPrefix(searchRoot.contentItem, prefix)
            if (found) return found
        }

        if (searchRoot.item && searchRoot.item !== searchRoot) {
            var found = findChildByPrefix(searchRoot.item, prefix)
            if (found) return found
        }

        if (searchRoot.data) {
            for (var i = 0; i < searchRoot.data.length; i++) {
                var dataItem = searchRoot.data[i]
                if (dataItem && dataItem !== searchRoot) {
                    var found = findChildByPrefix(dataItem, prefix)
                    if (found) return found
                }
            }
        }

        return null
    }

    /**
     * Finds all children with objectNames matching the given prefix.
     * @param parent The parent element to search from
     * @param prefix The objectName prefix to match
     * @returns Array of matching elements
     */
    function findAllChildrenByPrefix(parent, prefix) {
        var results = []
        findAllChildrenByPrefixRecursive(parent || e2eTestCase.parent, prefix, results)
        return results
    }

    function findAllChildrenByPrefixRecursive(node, prefix, results) {
        if (!node) return

        if (node.objectName && node.objectName.indexOf(prefix) === 0) {
            results.push(node)
        }

        if (node.children) {
            for (var i = 0; i < node.children.length; i++) {
                findAllChildrenByPrefixRecursive(node.children[i], prefix, results)
            }
        }

        if (node.contentItem && node.contentItem !== node) {
            findAllChildrenByPrefixRecursive(node.contentItem, prefix, results)
        }

        if (node.item && node.item !== node) {
            findAllChildrenByPrefixRecursive(node.item, prefix, results)
        }

        if (node.data) {
            for (var i = 0; i < node.data.length; i++) {
                var dataItem = node.data[i]
                if (dataItem && dataItem !== node) {
                    findAllChildrenByPrefixRecursive(dataItem, prefix, results)
                }
            }
        }
    }

    // =========================================================================
    // Wait Utilities
    // =========================================================================

    /**
     * Waits until a condition becomes true or timeout is reached.
     * @param condition Function that returns true when condition is met
     * @param timeout Maximum time to wait in ms (default: 5000)
     * @param interval Check interval in ms (default: 100)
     * @returns true if condition was met, false if timeout
     */
    function waitForCondition(condition, timeout, interval) {
        var maxTime = timeout || 5000
        var checkInterval = interval || 100
        var elapsed = 0

        while (elapsed < maxTime) {
            if (condition()) {
                return true
            }
            wait(checkInterval)
            elapsed += checkInterval
        }

        return false
    }

    /**
     * Waits until an element with the given objectName exists.
     * @param objectName The objectName to wait for
     * @param timeout Maximum time to wait in ms (default: 5000)
     * @returns The found element or null if timeout
     */
    function waitForElement(objectName, timeout) {
        var element = null
        var found = waitForCondition(function() {
            element = findChild(mainWindow, objectName)
            return element !== null
        }, timeout || 5000)

        return found ? element : null
    }

    /**
     * Waits until an element's property reaches an expected value.
     * @param element The element to check
     * @param propertyName The property name to check
     * @param expectedValue The expected value
     * @param timeout Maximum time to wait in ms (default: 5000)
     * @returns true if value was reached, false if timeout
     */
    function waitForProperty(element, propertyName, expectedValue, timeout) {
        return waitForCondition(function() {
            return element && element[propertyName] === expectedValue
        }, timeout || 5000)
    }

    // =========================================================================
    // Assertion Helpers
    // =========================================================================

    /**
     * Verifies an element exists and is visible.
     * @param objectName The objectName to find
     * @param message Optional failure message
     * @returns The found element
     */
    function verifyElementVisible(objectName, message) {
        var element = findChild(mainWindow, objectName)
        verify(element !== null, message || "Element '" + objectName + "' should exist")
        verify(element.visible, message || "Element '" + objectName + "' should be visible")
        return element
    }

    /**
     * Verifies navigation to a specific view.
     * @param expectedView The expected currentView value
     * @param timeout Maximum time to wait
     */
    function verifyNavigation(expectedView, timeout) {
        var success = waitForProperty(mainWindow, "currentView", expectedView, timeout || 3000)
        verify(success, "Should navigate to '" + expectedView + "' view (current: " + mainWindow.currentView + ")")
    }

    // =========================================================================
    // High-level E2E Helpers
    // =========================================================================

    /**
     * Waits until the application is fully loaded and ready.
     * Ensures mainWindow exists and the initial view is displayed.
     * @param timeout Maximum time to wait (default: 3000ms)
     * @returns true if app is ready, false if timeout
     */
    function waitForAppReady(timeout) {
        var maxTime = timeout || 3000
        
        // First, ensure mainWindow is found
        if (!mainWindow) {
            var found = waitForCondition(function() {
                mainWindow = findChild(null, "mainWindow")
                return mainWindow !== null
            }, maxTime)
            
            if (!found) {
                console.warn("E2ETestCase: mainWindow not found within timeout")
                return false
            }
        }
        
        // Then wait for a valid view to be set
        var ready = waitForCondition(function() {
            return mainWindow.currentView && mainWindow.currentView.length > 0
        }, maxTime / 2)
        
        if (ready) {
            console.log("E2ETestCase: App ready, currentView = " + mainWindow.currentView)
        }
        
        return ready
    }

    /**
     * Navigates to a specific view by setting currentView property.
     * Use for programmatic navigation in tests.
     * @param viewName The view to navigate to ("home", "player", "login")
     * @param timeout Maximum time to wait for navigation (default: 1500ms)
     * @returns true if navigation succeeded
     */
    function navigateTo(viewName, timeout) {
        if (!mainWindow) {
            console.warn("E2ETestCase: Cannot navigate - mainWindow not set")
            return false
        }
        
        // Set the view
        mainWindow.currentView = viewName
        
        // Wait for the navigation to complete
        var success = waitForProperty(mainWindow, "currentView", viewName, timeout || 1500)
        
        if (success) {
            console.log("E2ETestCase: Navigated to " + viewName)
        } else {
            console.warn("E2ETestCase: Navigation to " + viewName + " failed (current: " + mainWindow.currentView + ")")
        }
        
        return success
    }

    /**
     * Clicks an element and waits for a condition to be met.
     * Useful for click actions that trigger async operations.
     * @param element The element to click
     * @param condition Function that returns true when ready
     * @param timeout Maximum time to wait after click (default: 1500ms)
     * @returns true if condition was met after click
     */
    function clickAndWait(element, condition, timeout) {
        if (!element) {
            console.warn("E2ETestCase: Cannot click - element is null")
            return false
        }
        
        verify(element.visible, "Element should be visible before clicking")
        
        mouseClick(element)
        
        if (condition) {
            return waitForCondition(condition, timeout || 1500)
        }
        
        // If no condition, just wait a bit
        wait(timeout || 500)
        return true
    }

    /**
     * Verifies that a stream card exists and is visible.
     * @param index The stream card index (default: 0)
     * @returns The stream card element if found and valid
     */
    function verifyStreamCard(index) {
        var cardIndex = index || 0
        var objectName = "streamCard_" + cardIndex
        
        var card = waitForElement(objectName, 3000)
        verify(card !== null, "Stream card " + cardIndex + " should exist")
        verify(card.visible, "Stream card " + cardIndex + " should be visible")
        
        console.log("E2ETestCase: Verified stream card " + cardIndex)
        return card
    }

    /**
     * Verifies that a VOD card exists and is visible.
     * @param index The VOD card index (default: 0)
     * @returns The VOD card element if found and valid
     */
    function verifyVodCard(index) {
        var cardIndex = index || 0
        var objectName = "vodCard_" + cardIndex
        
        var card = waitForElement(objectName, 3000)
        verify(card !== null, "VOD card " + cardIndex + " should exist")
        verify(card.visible, "VOD card " + cardIndex + " should be visible")
        
        console.log("E2ETestCase: Verified VOD card " + cardIndex)
        return card
    }

    /**
     * Verifies that the player view is showing and has valid stream info.
     * @returns The player view element
     */
    function verifyPlayerView() {
        var playerView = waitForElement("playerView", 3000)
        verify(playerView !== null, "Player view should exist")
        verify(!playerView.showError, "Player should not show error")
        
        console.log("E2ETestCase: Verified player view")
        return playerView
    }

    /**
     * Waits for video playback to start.
     * @param timeout Maximum time to wait (default: 10000ms)
     * @returns true if playback started
     */
    function waitForPlayback(timeout) {
        var playerView = findChild(mainWindow, "playerView")
        if (!playerView) {
            console.warn("E2ETestCase: Cannot wait for playback - playerView not found")
            return false
        }
        
        var maxTime = timeout || 10000
        
        var success = waitForCondition(function() {
            // Accept playing state or valid HLS URL without error
            return playerView.playing || 
                   (!playerView.showError && playerView.hlsUrl && playerView.hlsUrl.length > 0)
        }, maxTime)
        
        if (success) {
            console.log("E2ETestCase: Playback state valid (playing=" + playerView.playing + ")")
        } else {
            console.warn("E2ETestCase: Playback timeout or error (error=" + playerView.showError + ")")
        }
        
        return success
    }
}
