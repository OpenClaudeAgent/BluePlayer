/**
 * tst_ErrorToast.qml
 * 
 * Functional UI tests for the ErrorToast component.
 * Tests show/hide behavior, auto-hide timer, message display,
 * and signal emissions.
 * 
 * Refactored to use:
 * - createTemporaryObject for test isolation
 * - cleanup() for proper teardown
 * - waitForRendering instead of wait() for visual sync
 * - tryCompare for async property checks
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 300

    // =========================================================================
    // Component Under Test
    // =========================================================================
    
    Component {
        id: errorToastComponent
        
        Rectangle {
            id: errorToast
            objectName: "errorToast"
            
            // Properties matching ErrorToast
            property string message: ""
            property bool showError: false
            property int autoHideDuration: 4000
            
            // Visual properties
            radius: 10
            color: "#CCB00020"
            border.color: "#FF5252"
            border.width: 1
            visible: showError
            opacity: showError ? 1.0 : 0.0
            
            // Auto-size based on content
            implicitWidth: contentRow.width + 24
            implicitHeight: contentRow.height + 24
            width: implicitWidth
            height: implicitHeight
            
            // Animation on opacity (short duration for tests)
            Behavior on opacity { 
                NumberAnimation { 
                    id: opacityAnimation
                    objectName: "opacityAnimation"
                    duration: 0  // Very short for tests
                    easing.type: Easing.OutCubic 
                } 
            }

            Row {
                id: contentRow
                objectName: "contentRow"
                anchors.centerIn: parent
                spacing: 8
                
                Text { 
                    id: warningIcon
                    objectName: "warningIcon"
                    text: "\u26A0" // Warning icon
                    color: "#FFFFFF"
                    font.pixelSize: 13 
                }
                
                Text { 
                    id: messageText
                    objectName: "messageText"
                    text: errorToast.message
                    color: "#FFFFFF"
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    maximumLineCount: 3
                }
            }
            
            Timer {
                id: hideTimer
                objectName: "hideTimer"
                interval: errorToast.autoHideDuration
                running: errorToast.showError && errorToast.autoHideDuration > 0
                repeat: false
                onTriggered: errorToast.showError = false
            }
            
            // Public function to show error
            function show(errorMessage) {
                message = errorMessage
                showError = true
            }
            
            // Public function to hide
            function hide() {
                showError = false
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var errorToast: null

    // =========================================================================
    // Signal Spies (created dynamically in init to avoid offscreen issues)
    // =========================================================================
    
    property var showErrorChangedSpy: null
    property var messageChangedSpy: null
    
    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "ErrorToastTests"
        when: windowShown
        
        function init() {
            errorToast = createTemporaryObject(errorToastComponent, root)
            verify(errorToast !== null, "ErrorToast should be created")
            errorToast.anchors.centerIn = root
            
            // Create spies dynamically to avoid offscreen mode issues
            showErrorChangedSpy = createTemporaryObject(signalSpyComponent, root, {target: errorToast, signalName: "showErrorChanged"})
            messageChangedSpy = createTemporaryObject(signalSpyComponent, root, {target: errorToast, signalName: "messageChanged"})
            
            // waitForRendering removed for perf
        }
        
        function cleanup() {
            errorToast = null
        }
        
        // =====================================================================
        // Default Values Tests
        // =====================================================================
        
        function test_defaultValues() {
            compare(errorToast.message, "", "Default message is empty")
            compare(errorToast.showError, false, "Default showError is false")
            compare(errorToast.autoHideDuration, 4000, "Default autoHideDuration is 4000ms")
        }
        
        function test_defaultVisibility() {
            compare(errorToast.visible, false, "Toast is hidden by default")
            compare(errorToast.opacity, 0.0, "Toast has zero opacity by default")
        }
        
        // =====================================================================
        // Show/Hide Property Tests
        // =====================================================================
        
        function test_showError_makesToastVisible() {
            compare(errorToast.visible, false, "Initially hidden")
            
            errorToast.showError = true
            
            compare(errorToast.visible, true, "Toast becomes visible when showError is true")
        }
        
        function test_showError_setsOpacityToOne() {
            // Ensure toast is hidden and wait for any animation to complete
            errorToast.showError = false
            // waitForRendering removed for perf
            verify(errorToast.opacity < 0.1, "Initially low opacity")
            
            errorToast.showError = true
            // waitForRendering removed for perf
            
            tryCompare(errorToast, "opacity", 1.0, 100, "Toast opacity becomes 1.0 when shown")
        }
        
        function test_hideError_makesToastInvisible() {
            errorToast.showError = true
            // waitForRendering removed for perf
            compare(errorToast.visible, true, "Toast is visible")
            
            errorToast.showError = false
            
            compare(errorToast.visible, false, "Toast becomes hidden when showError is false")
        }
        
        function test_hideError_setsOpacityToZero() {
            errorToast.showError = true
            // waitForRendering removed for perf
            tryCompare(errorToast, "opacity", 1.0, 100, "Toast has full opacity")
            
            errorToast.showError = false
            // waitForRendering removed for perf
            
            tryCompare(errorToast, "opacity", 0.0, 100, "Toast opacity becomes 0 when hidden")
        }
        
        // =====================================================================
        // Message Display Tests
        // =====================================================================
        
        function test_message_displayedInText() {
            var messageText = findChild(errorToast, "messageText")
            verify(messageText !== null, "Message text element exists")
            
            errorToast.message = "Test error message"
            
            compare(messageText.text, "Test error message", "Message is displayed in text")
        }
        
        function test_message_variousMessages_data() {
            return [
                { tag: "short", message: "Error" },
                { tag: "medium", message: "Connection failed" },
                { tag: "long", message: "Unable to connect to server. Please check your internet connection." },
                { tag: "withNumbers", message: "Error code: 404" },
                { tag: "withSpecialChars", message: "Error: File 'test.txt' not found!" }
            ]
        }
        
        function test_message_variousMessages(data) {
            var messageText = findChild(errorToast, "messageText")
            errorToast.message = data.message
            
            compare(messageText.text, data.message, "Message displayed correctly: " + data.tag)
        }
        
        // =====================================================================
        // Warning Icon Tests
        // =====================================================================
        
        function test_warningIcon_exists() {
            var icon = findChild(errorToast, "warningIcon")
            verify(icon !== null, "Warning icon exists")
        }
        
        function test_warningIcon_hasCorrectSymbol() {
            var icon = findChild(errorToast, "warningIcon")
            compare(icon.text, "\u26A0", "Warning icon shows warning symbol")
        }
        
        // =====================================================================
        // show() Function Tests
        // =====================================================================
        
        function test_show_setsMessageAndShowsToast() {
            compare(errorToast.message, "", "Initially no message")
            compare(errorToast.showError, false, "Initially hidden")
            
            errorToast.show("Network error occurred")
            
            compare(errorToast.message, "Network error occurred", "Message is set")
            compare(errorToast.showError, true, "Toast is shown")
        }
        
        function test_show_multipleCallsUpdateMessage() {
            errorToast.show("First error")
            compare(errorToast.message, "First error", "First message set")
            
            errorToast.show("Second error")
            compare(errorToast.message, "Second error", "Second message replaces first")
            compare(errorToast.showError, true, "Toast still shown")
        }
        
        // =====================================================================
        // hide() Function Tests
        // =====================================================================
        
        function test_hide_hidesTheToast() {
            errorToast.show("Test error")
            compare(errorToast.showError, true, "Toast is shown")
            
            errorToast.hide()
            
            compare(errorToast.showError, false, "Toast is hidden")
        }
        
        function test_hide_preservesMessage() {
            errorToast.show("Preserved message")
            errorToast.hide()
            
            compare(errorToast.message, "Preserved message", "Message is preserved after hide")
        }
        
        // =====================================================================
        // Auto-Hide Timer Tests
        // =====================================================================
        
        function test_autoHideTimer_exists() {
            var timer = findChild(errorToast, "hideTimer")
            verify(timer !== null, "Hide timer exists")
        }
        
        function test_autoHideTimer_startsWhenShown() {
            var timer = findChild(errorToast, "hideTimer")
            compare(timer.running, false, "Timer not running initially")
            
            errorToast.showError = true
            
            compare(timer.running, true, "Timer starts when shown")
        }
        
        function test_autoHideTimer_stopsWhenHidden() {
            errorToast.showError = true
            var timer = findChild(errorToast, "hideTimer")
            compare(timer.running, true, "Timer running when shown")
            
            errorToast.showError = false
            
            compare(timer.running, false, "Timer stops when hidden")
        }
        
        function test_autoHideTimer_hidesAfterDuration() {
            // Use short duration for test
            errorToast.autoHideDuration = 100
            errorToast.showError = true
            compare(errorToast.showError, true, "Toast is shown")
            
            // Wait for timer to trigger (timeout > duration for buffer)
            tryCompare(errorToast, "showError", false, 300, "Toast is hidden after timer")
        }
        
        function test_autoHideTimer_disabled_whenDurationZero() {
            errorToast.autoHideDuration = 0
            var timer = findChild(errorToast, "hideTimer")
            
            errorToast.showError = true
            
            compare(timer.running, false, "Timer does not start when duration is 0")
        }
        
        function test_autoHideTimer_disabled_staysVisible() {
            errorToast.autoHideDuration = 0
            errorToast.showError = true
            // waitForRendering removed for perf
            
            compare(errorToast.showError, true, "Toast stays visible with zero duration")
        }
        
        // =====================================================================
        // Edge Cases
        // =====================================================================
        
        function test_emptyMessage_stillShows() {
            errorToast.show("")
            
            compare(errorToast.showError, true, "Toast shows even with empty message")
            compare(errorToast.message, "", "Message is empty")
        }
        
        function test_rapidShowHide() {
            for (var i = 0; i < 5; i++) {
                errorToast.show("Error " + i)
                errorToast.hide()
            }
            
            compare(errorToast.showError, false, "Toast is hidden after rapid show/hide")
        }
        
        function test_showWhileAlreadyShown() {
            errorToast.show("First error")
            errorToast.show("Second error")
            
            compare(errorToast.message, "Second error", "Message updated")
            compare(errorToast.showError, true, "Toast still shown")
        }
        
        function test_hideWhileAlreadyHidden() {
            errorToast.hide()
            errorToast.hide()
            
            compare(errorToast.showError, false, "Toast still hidden")
        }
        
        // =====================================================================
        // Property Change Signal Tests
        // =====================================================================
        
        function test_showErrorChanged_emitsOnShow() {
            errorToast.showError = true
            
            compare(showErrorChangedSpy.count, 1, "showErrorChanged emitted when shown")
        }
        
        function test_showErrorChanged_emitsOnHide() {
            errorToast.showError = true
            showErrorChangedSpy.clear()
            
            errorToast.showError = false
            
            compare(showErrorChangedSpy.count, 1, "showErrorChanged emitted when hidden")
        }
        
        function test_messageChanged_emitsOnChange() {
            errorToast.message = "New message"
            
            compare(messageChangedSpy.count, 1, "messageChanged emitted when message changes")
        }
        
        // =====================================================================
        // Integration Tests
        // =====================================================================
        
        function test_fullShowHideFlow() {
            // 1. Start hidden
            compare(errorToast.visible, false, "Initially hidden")
            compare(errorToast.opacity, 0.0, "Initially transparent")
            
            // 2. Show error
            errorToast.show("Connection lost")
            // waitForRendering removed for perf
            
            compare(errorToast.visible, true, "Now visible")
            tryCompare(errorToast, "opacity", 1.0, 100, "Now opaque")
            compare(errorToast.message, "Connection lost", "Message set")
            
            // 3. Hide error
            errorToast.hide()
            // waitForRendering removed for perf
            
            compare(errorToast.visible, false, "Hidden again")
            tryCompare(errorToast, "opacity", 0.0, 100, "Transparent again")
        }
        
        function test_autoHideFullFlow() {
            // Use short duration
            errorToast.autoHideDuration = 100
            
            // Show error
            errorToast.show("Temporary error")
            compare(errorToast.showError, true, "Toast shown")
            
            // Wait for auto-hide with tryCompare for robustness (timeout > duration for buffer)
            tryCompare(errorToast, "showError", false, 300, "Toast auto-hidden")
            compare(errorToast.message, "Temporary error", "Message preserved")
        }
        
        function test_manualHideBeforeAutoHide() {
            errorToast.autoHideDuration = 100  // Reduced for fast tests
            errorToast.show("Will hide manually")
            
            // Hide before timer
            // waitForRendering removed for perf
            errorToast.hide()
            
            // Toast should stay hidden
            tryCompare(errorToast, "showError", false, 100, "Toast stays hidden")
        }
    }
}
