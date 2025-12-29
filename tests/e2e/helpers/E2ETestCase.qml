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
    // Signals for async operations
    // =========================================================================
    
    /** Emitted when a screenshot operation completes */
    signal screenshotComplete()

    // =========================================================================
    // Internal: SignalSpy for waiting on screenshot completion
    // =========================================================================
    
    SignalSpy {
        id: screenshotSpy
        target: e2eTestCase
        signalName: "screenshotComplete"
    }

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
     * Uses SignalSpy to wait for the async grabToImage to complete.
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
        
        // Clear previous signals
        screenshotSpy.clear()
        
        // Start async grab
        var success = captureTarget.grabToImage(function(image) {
            var path = screenshotDir + "/" + testName + ".png"
            if (image.saveToFile(path)) {
                console.log("Screenshot saved: " + path)
            } else {
                console.warn("Failed to save screenshot: " + path)
            }
            // Emit signal to notify completion
            e2eTestCase.screenshotComplete()
        })
        
        if (!success) {
            console.warn("E2ETestCase: grabToImage failed to start")
            return
        }
        
        // Wait for the signal (max 2 seconds) - don't fail test if timeout
        if (!screenshotSpy.wait(2000)) {
            console.warn("E2ETestCase: Screenshot timeout (non-fatal)")
        }
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
}
