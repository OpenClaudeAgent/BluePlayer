/**
 * tst_SeekBar.qml
 * 
 * Functional UI tests for the SeekBar component.
 * Tests video position slider with live/VOD/replay support,
 * drag behavior, seek signals, and mode transitions.
 * 
 * Refactored to use:
 * - createTemporaryObject for test isolation
 * - waitForRendering instead of wait() for visual sync
 * - tryCompare for async property changes
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 600
    height: 200

    // Detect offscreen mode - mouse events crash in offscreen
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"

    // =========================================================================
    // Component Under Test (Mock matching SeekBar.qml API)
    // =========================================================================
    
    Component {
        id: seekBarComponent
        
        Slider {
            id: seekBar
            objectName: "seekBar"
            width: 500
            height: 40
            
            // Required properties
            property real duration: 0.0
            property real currentPosition: 0.0
            property bool liveMode: true
            property bool isReplayMode: false
            
            // Signals
            signal seekRequested(real seconds)
            signal seekDragStarted()
            signal seekDragEnded(real seconds)
            signal seekPreviewed(real seconds)
            
            // Internal state
            property bool userDragging: false
            property real seekTarget: 0
            
            // Computed properties
            readonly property bool isLiveMode: liveMode && !isReplayMode
            readonly property real currentDuration: duration
            readonly property bool atLiveEdge: isLiveMode || (!isReplayMode && currentDuration > 0 && (currentDuration - currentPosition) <= 5)
            
            // Slider configuration
            enabled: currentDuration > 0
            hoverEnabled: true
            from: 0
            to: currentDuration > 0 ? currentDuration : 1
            
            // Value binding
            Binding {
                target: seekBar
                property: "value"
                value: seekBar.isLiveMode ? seekBar.to : seekBar.currentPosition
                when: !seekBar.userDragging && !seekBar.pressed
            }
            
            onPressedChanged: {
                if (pressed) {
                    userDragging = true
                    seekDragStarted()
                    seekTarget = isLiveMode ? currentDuration : currentPosition
                } else if (userDragging) {
                    userDragging = false
                    seekDragEnded(seekTarget)
                    seekRequested(seekTarget)
                }
            }
            
            onMoved: {
                if (userDragging || pressed) {
                    seekTarget = value
                    seekPreviewed(value)
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var seekBar: null

    // =========================================================================
    // Signal Spies (created dynamically in init to avoid offscreen issues)
    // =========================================================================
    
    property var seekRequestedSpy: null
    property var seekDragStartedSpy: null
    property var seekDragEndedSpy: null
    property var seekPreviewedSpy: null
    
    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "SeekBarTests"
        when: windowShown
        
        // Helper to skip tests when Slider crashes in offscreen mode
        function requiresSlider() {
            if (root.isOffscreen) {
                skip("Slider-based tests crash in offscreen mode on Qt 6.9")
                return false
            }
            return true
        }
        
        function init() {
            // Skip in offscreen mode (Slider crashes)
            if (root.isOffscreen) return
            
            // Create fresh instance for each test
            seekBar = createTemporaryObject(seekBarComponent, root)
            verify(seekBar !== null, "SeekBar should be created")
            seekBar.anchors.centerIn = root
            
            // Create spies dynamically to avoid offscreen mode issues
            seekRequestedSpy = createTemporaryObject(signalSpyComponent, root, {target: seekBar, signalName: "seekRequested"})
            seekDragStartedSpy = createTemporaryObject(signalSpyComponent, root, {target: seekBar, signalName: "seekDragStarted"})
            seekDragEndedSpy = createTemporaryObject(signalSpyComponent, root, {target: seekBar, signalName: "seekDragEnded"})
            seekPreviewedSpy = createTemporaryObject(signalSpyComponent, root, {target: seekBar, signalName: "seekPreviewed"})
            
            waitForRendering(seekBar)
        }
        
        function cleanup() {
            seekBar = null
        }
        
        // =====================================================================
        // Default Values Tests
        // =====================================================================
        
        function test_defaultValues() {
            if (!requiresSlider()) return
            compare(seekBar.duration, 0.0, "Default duration is 0")
            compare(seekBar.currentPosition, 0.0, "Default position is 0")
            compare(seekBar.liveMode, true, "Default liveMode is true")
            compare(seekBar.isReplayMode, false, "Default isReplayMode is false")
            compare(seekBar.userDragging, false, "Default userDragging is false")
        }
        
        function test_computedProperties() {
            if (!requiresSlider()) return
            // Initially live mode
            compare(seekBar.isLiveMode, true, "isLiveMode computed correctly")
            compare(seekBar.currentDuration, 0.0, "currentDuration mirrors duration")
            
            // With duration
            seekBar.duration = 100
            compare(seekBar.currentDuration, 100, "currentDuration updates")
        }
        
        function test_sliderDisabledWhenNoDuration() {
            if (!requiresSlider()) return
            seekBar.duration = 0
            compare(seekBar.enabled, false, "Slider disabled when duration is 0")
            
            seekBar.duration = 100
            compare(seekBar.enabled, true, "Slider enabled when duration > 0")
        }
        
        // =====================================================================
        // Live Mode Tests
        // =====================================================================
        
        function test_liveMode_sliderAtEdge() {
            if (!requiresSlider()) return
            seekBar.liveMode = true
            seekBar.isReplayMode = false
            seekBar.duration = 100
            waitForRendering(seekBar)
            
            compare(seekBar.isLiveMode, true, "isLiveMode is true")
            compare(seekBar.value, seekBar.to, "Slider at live edge in live mode")
        }
        
        function test_liveMode_atLiveEdge() {
            if (!requiresSlider()) return
            seekBar.liveMode = true
            seekBar.isReplayMode = false
            seekBar.duration = 100
            
            compare(seekBar.atLiveEdge, true, "atLiveEdge is true in live mode")
        }
        
        // =====================================================================
        // VOD Mode Tests
        // =====================================================================
        
        function test_vodMode_followsPosition() {
            if (!requiresSlider()) return
            seekBar.liveMode = false
            seekBar.isReplayMode = false
            seekBar.duration = 100
            seekBar.currentPosition = 30
            
            compare(seekBar.isLiveMode, false, "isLiveMode is false")
            tryCompare(seekBar, "value", 30, 100, "Slider follows currentPosition in VOD mode")
        }
        
        function test_vodMode_positionUpdates() {
            if (!requiresSlider()) return
            seekBar.liveMode = false
            seekBar.duration = 100
            
            seekBar.currentPosition = 10
            tryCompare(seekBar, "value", 10, 100, "Position at 10")
            
            seekBar.currentPosition = 50
            tryCompare(seekBar, "value", 50, 100, "Position at 50")
            
            seekBar.currentPosition = 90
            tryCompare(seekBar, "value", 90, 100, "Position at 90")
        }
        
        function test_vodMode_atLiveEdge_nearEnd() {
            if (!requiresSlider()) return
            seekBar.liveMode = false
            seekBar.isReplayMode = false
            seekBar.duration = 100
            
            seekBar.currentPosition = 90
            compare(seekBar.atLiveEdge, false, "Not at edge at 90/100")
            
            seekBar.currentPosition = 96
            compare(seekBar.atLiveEdge, true, "At edge when within 5s of end")
        }
        
        // =====================================================================
        // Replay Mode Tests
        // =====================================================================
        
        function test_replayMode_neverLive() {
            if (!requiresSlider()) return
            seekBar.liveMode = true  // Would be live...
            seekBar.isReplayMode = true  // ...but replay mode overrides
            seekBar.duration = 100
            seekBar.currentPosition = 30
            
            compare(seekBar.isLiveMode, false, "isLiveMode is false when isReplayMode is true")
            tryCompare(seekBar, "value", 30, 100, "Replay mode follows position like VOD")
        }
        
        // =====================================================================
        // Drag Behavior Tests
        // =====================================================================
        
        function test_dragStart_setsUserDragging() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            seekBar.duration = 100
            seekBar.liveMode = false
            seekBar.currentPosition = 50
            waitForRendering(seekBar)
            
            compare(seekBar.userDragging, false, "Initially not dragging")
            
            // Simulate press
            mousePress(seekBar, seekBar.width / 2, seekBar.height / 2)
            
            tryCompare(seekBar, "userDragging", true, 100, "userDragging is true when pressed")
            compare(seekDragStartedSpy.count, 1, "seekDragStarted emitted")
            
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
        }
        
        function test_dragEnd_emitsSignals() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            seekBar.duration = 100
            seekBar.liveMode = false
            seekBar.currentPosition = 50
            waitForRendering(seekBar)
            
            // Press and release
            mousePress(seekBar, seekBar.width / 2, seekBar.height / 2)
            tryCompare(seekBar, "userDragging", true, 100)
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
            
            tryCompare(seekBar, "userDragging", false, 100, "userDragging is false after release")
            compare(seekDragEndedSpy.count, 1, "seekDragEnded emitted")
            compare(seekRequestedSpy.count, 1, "seekRequested emitted")
        }
        
        function test_dragPreview_emitsSeekPreviewed() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            seekBar.duration = 100
            seekBar.liveMode = false
            waitForRendering(seekBar)
            
            // Simulate drag
            mousePress(seekBar, 10, seekBar.height / 2)
            tryCompare(seekBar, "pressed", true, 100)
            mouseMove(seekBar, seekBar.width / 2, seekBar.height / 2)
            waitForRendering(seekBar)
            
            verify(seekPreviewedSpy.count >= 1, "seekPreviewed emitted during drag")
            
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
        }
        
        // =====================================================================
        // Mode Transitions
        // =====================================================================
        
        function test_modeTransition_liveToVod() {
            if (!requiresSlider()) return
            seekBar.liveMode = true
            seekBar.duration = 100
            waitForRendering(seekBar)
            compare(seekBar.value, 100, "At edge in live mode")
            
            seekBar.liveMode = false
            seekBar.currentPosition = 30
            
            tryCompare(seekBar, "value", 30, 100, "Follows position after switch to VOD")
        }
        
        function test_modeTransition_vodToLive() {
            if (!requiresSlider()) return
            seekBar.liveMode = false
            seekBar.duration = 100
            seekBar.currentPosition = 30
            tryCompare(seekBar, "value", 30, 100, "At position in VOD mode")
            
            seekBar.liveMode = true
            waitForRendering(seekBar)
            
            compare(seekBar.value, 100, "At edge after switch to live")
        }
        
        // =====================================================================
        // Signal Verification
        // =====================================================================
        
        function test_noSignalsWithoutInteraction() {
            if (!requiresSlider()) return
            seekBar.duration = 100
            seekBar.liveMode = false
            seekBar.currentPosition = 50
            waitForRendering(seekBar)
            
            compare(seekRequestedSpy.count, 0, "No seekRequested without interaction")
            compare(seekDragStartedSpy.count, 0, "No seekDragStarted without interaction")
            compare(seekDragEndedSpy.count, 0, "No seekDragEnded without interaction")
            compare(seekPreviewedSpy.count, 0, "No seekPreviewed without interaction")
        }
        
        function test_signalOrder() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            seekBar.duration = 100
            seekBar.liveMode = false
            waitForRendering(seekBar)
            
            // Press
            mousePress(seekBar, seekBar.width / 2, seekBar.height / 2)
            tryCompare(seekBar, "pressed", true, 100)
            compare(seekDragStartedSpy.count, 1, "seekDragStarted emitted on press")
            compare(seekDragEndedSpy.count, 0, "No seekDragEnded yet")
            
            // Release
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
            tryCompare(seekBar, "pressed", false, 100)
            compare(seekDragEndedSpy.count, 1, "seekDragEnded emitted on release")
            compare(seekRequestedSpy.count, 1, "seekRequested emitted on release")
        }
        
        // =====================================================================
        // Binding Stability Tests
        // =====================================================================
        
        function test_bindingNotBrokenByDrag() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            seekBar.liveMode = false
            seekBar.duration = 100
            seekBar.currentPosition = 25
            
            tryCompare(seekBar, "value", 25, 100, "Value at 25")
            
            // Drag and release
            mousePress(seekBar, seekBar.width / 2, seekBar.height / 2)
            tryCompare(seekBar, "pressed", true, 100)
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
            tryCompare(seekBar, "pressed", false, 100)
            
            // Now update position - binding should still work
            seekBar.currentPosition = 75
            
            tryCompare(seekBar, "value", 75, 100, "Value follows position after drag")
        }
        
        // =====================================================================
        // Data-Driven Tests
        // =====================================================================
        
        function test_allModes_data() {
            return [
                { tag: "live", live: true, replay: false, expectedLive: true },
                { tag: "vod", live: false, replay: false, expectedLive: false },
                { tag: "replay", live: true, replay: true, expectedLive: false },
                { tag: "replay-vod", live: false, replay: true, expectedLive: false }
            ]
        }
        
        function test_allModes(data) {
            if (!requiresSlider()) return
            seekBar.liveMode = data.live
            seekBar.isReplayMode = data.replay
            
            compare(seekBar.isLiveMode, data.expectedLive, "isLiveMode is " + data.expectedLive)
        }
    }
}
