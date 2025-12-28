/**
 * tst_SeekBar.qml
 * 
 * Functional UI tests for the SeekBar component.
 * Tests video position slider with live/VOD/replay support.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 600
    height: 200

    // =========================================================================
    // Component Under Test (Mock matching SeekBar.qml API)
    // =========================================================================
    
    Item {
        id: seekBarContainer
        anchors.centerIn: parent
        width: 500
        height: 40
        
        Slider {
            id: seekBar
            objectName: "seekBar"
            anchors.fill: parent
            
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
            
            // Reset function for tests
            function reset() {
                duration = 0.0
                currentPosition = 0.0
                liveMode = true
                isReplayMode = false
                userDragging = false
                seekTarget = 0
            }
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: seekRequestedSpy; target: seekBar; signalName: "seekRequested" }
    SignalSpy { id: seekDragStartedSpy; target: seekBar; signalName: "seekDragStarted" }
    SignalSpy { id: seekDragEndedSpy; target: seekBar; signalName: "seekDragEnded" }
    SignalSpy { id: seekPreviewedSpy; target: seekBar; signalName: "seekPreviewed" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "SeekBarTests"
        when: windowShown
        
        function init() {
            seekBar.reset()
            seekRequestedSpy.clear()
            seekDragStartedSpy.clear()
            seekDragEndedSpy.clear()
            seekPreviewedSpy.clear()
            wait(50)
        }
        
        // =====================================================================
        // Default Values Tests
        // =====================================================================
        
        function test_defaultValues() {
            compare(seekBar.duration, 0.0, "Default duration is 0")
            compare(seekBar.currentPosition, 0.0, "Default position is 0")
            compare(seekBar.liveMode, true, "Default liveMode is true")
            compare(seekBar.isReplayMode, false, "Default isReplayMode is false")
            compare(seekBar.userDragging, false, "Default userDragging is false")
            compare(seekBar.seekTarget, 0, "Default seekTarget is 0")
        }
        
        function test_computedProperties() {
            // Initially live mode
            compare(seekBar.isLiveMode, true, "isLiveMode computed correctly")
            compare(seekBar.currentDuration, 0.0, "currentDuration mirrors duration")
            
            // With duration
            seekBar.duration = 100
            compare(seekBar.currentDuration, 100, "currentDuration updates")
        }
        
        function test_sliderDisabledWhenNoDuration() {
            seekBar.duration = 0
            compare(seekBar.enabled, false, "Slider disabled when duration is 0")
            
            seekBar.duration = 100
            compare(seekBar.enabled, true, "Slider enabled when duration > 0")
        }
        
        function test_sliderRange() {
            seekBar.duration = 3600 // 1 hour
            compare(seekBar.from, 0, "Slider starts from 0")
            compare(seekBar.to, 3600, "Slider ends at duration")
        }
        
        // =====================================================================
        // Live Mode Tests
        // =====================================================================
        
        function test_liveMode_sliderAtEdge() {
            seekBar.liveMode = true
            seekBar.isReplayMode = false
            seekBar.duration = 100
            wait(50)
            
            compare(seekBar.isLiveMode, true, "isLiveMode is true")
            // In live mode, value should be at 'to' (max)
            compare(seekBar.value, seekBar.to, "Slider at live edge in live mode")
        }
        
        function test_liveMode_atLiveEdge() {
            seekBar.liveMode = true
            seekBar.isReplayMode = false
            seekBar.duration = 100
            
            compare(seekBar.atLiveEdge, true, "atLiveEdge is true in live mode")
        }
        
        function test_liveMode_ignoresCurrentPosition() {
            seekBar.liveMode = true
            seekBar.isReplayMode = false
            seekBar.duration = 100
            seekBar.currentPosition = 50
            wait(50)
            
            // Live mode always at edge, ignores currentPosition
            compare(seekBar.value, seekBar.to, "Live mode ignores currentPosition")
        }
        
        // =====================================================================
        // VOD Mode Tests
        // =====================================================================
        
        function test_vodMode_followsPosition() {
            seekBar.liveMode = false
            seekBar.isReplayMode = false
            seekBar.duration = 100
            seekBar.currentPosition = 30
            wait(50)
            
            compare(seekBar.isLiveMode, false, "isLiveMode is false")
            tryCompare(seekBar, "value", 30, 200, "Slider follows currentPosition in VOD mode")
        }
        
        function test_vodMode_positionUpdates() {
            seekBar.liveMode = false
            seekBar.duration = 100
            
            seekBar.currentPosition = 10
            wait(50)
            tryCompare(seekBar, "value", 10, 200, "Position at 10")
            
            seekBar.currentPosition = 50
            wait(50)
            tryCompare(seekBar, "value", 50, 200, "Position at 50")
            
            seekBar.currentPosition = 90
            wait(50)
            tryCompare(seekBar, "value", 90, 200, "Position at 90")
        }
        
        function test_vodMode_atLiveEdge_nearEnd() {
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
            seekBar.liveMode = true  // Would be live...
            seekBar.isReplayMode = true  // ...but replay mode overrides
            seekBar.duration = 100
            seekBar.currentPosition = 30
            wait(50)
            
            compare(seekBar.isLiveMode, false, "isLiveMode is false when isReplayMode is true")
            tryCompare(seekBar, "value", 30, 200, "Replay mode follows position like VOD")
        }
        
        function test_replayMode_atLiveEdge() {
            seekBar.liveMode = true
            seekBar.isReplayMode = true
            seekBar.duration = 100
            seekBar.currentPosition = 50
            
            // In replay mode, atLiveEdge is always false (isLiveMode is false due to isReplayMode,
            // and the second condition requires !isReplayMode which is false)
            compare(seekBar.atLiveEdge, false, "Not at edge at 50/100 in replay")
            
            seekBar.currentPosition = 98
            compare(seekBar.atLiveEdge, false, "Still not 'atLiveEdge' in replay mode (by design)")
        }
        
        // =====================================================================
        // Drag Behavior Tests
        // =====================================================================
        
        function test_dragStart_setsUserDragging() {
            seekBar.duration = 100
            seekBar.liveMode = false
            seekBar.currentPosition = 50
            wait(50)
            
            compare(seekBar.userDragging, false, "Initially not dragging")
            
            // Simulate press
            mousePress(seekBar, seekBar.width / 2, seekBar.height / 2)
            wait(50)
            
            compare(seekBar.userDragging, true, "userDragging is true when pressed")
            compare(seekDragStartedSpy.count, 1, "seekDragStarted emitted")
            
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
        }
        
        function test_dragEnd_emitsSignals() {
            seekBar.duration = 100
            seekBar.liveMode = false
            seekBar.currentPosition = 50
            wait(50)
            
            // Press and release
            mousePress(seekBar, seekBar.width / 2, seekBar.height / 2)
            wait(50)
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
            wait(50)
            
            compare(seekBar.userDragging, false, "userDragging is false after release")
            compare(seekDragEndedSpy.count, 1, "seekDragEnded emitted")
            compare(seekRequestedSpy.count, 1, "seekRequested emitted")
        }
        
        function test_dragPreview_emitsSeekPreviewed() {
            seekBar.duration = 100
            seekBar.liveMode = false
            wait(50)
            
            // Simulate drag
            mousePress(seekBar, 10, seekBar.height / 2)
            wait(50)
            mouseMove(seekBar, seekBar.width / 2, seekBar.height / 2)
            wait(50)
            
            verify(seekPreviewedSpy.count >= 1, "seekPreviewed emitted during drag")
            
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
        }
        
        // =====================================================================
        // Seek Target Tests
        // =====================================================================
        
        function test_seekTarget_liveModeStartsAtDuration() {
            seekBar.duration = 100
            seekBar.liveMode = true
            seekBar.isReplayMode = false
            wait(50)
            
            mousePress(seekBar, seekBar.width - 10, seekBar.height / 2)
            wait(50)
            
            // In live mode, seekTarget starts at duration
            compare(seekBar.seekTarget, seekBar.duration, "seekTarget starts at duration in live mode")
            
            mouseRelease(seekBar, seekBar.width - 10, seekBar.height / 2)
        }
        
        function test_seekTarget_updatesOnClick() {
            seekBar.duration = 100
            seekBar.liveMode = false
            seekBar.currentPosition = 30
            wait(100)
            
            // Click at ~25% of slider (width/4)
            mousePress(seekBar, seekBar.width / 4, seekBar.height / 2)
            wait(50)
            
            // onPressedChanged sets seekTarget to currentPosition initially,
            // but onMoved immediately updates it to the clicked position
            // So seekTarget should be approximately 25 (25% of 100)
            verify(seekBar.seekTarget >= 20 && seekBar.seekTarget <= 30, 
                   "seekTarget is near clicked position (~25), actual: " + seekBar.seekTarget)
            
            mouseRelease(seekBar, seekBar.width / 4, seekBar.height / 2)
        }
        
        // =====================================================================
        // Mode Transitions
        // =====================================================================
        
        function test_modeTransition_liveToVod() {
            seekBar.liveMode = true
            seekBar.duration = 100
            wait(50)
            compare(seekBar.value, 100, "At edge in live mode")
            
            seekBar.liveMode = false
            seekBar.currentPosition = 30
            wait(100)
            
            tryCompare(seekBar, "value", 30, 200, "Follows position after switch to VOD")
        }
        
        function test_modeTransition_vodToLive() {
            seekBar.liveMode = false
            seekBar.duration = 100
            seekBar.currentPosition = 30
            wait(100)
            tryCompare(seekBar, "value", 30, 200, "At position in VOD mode")
            
            seekBar.liveMode = true
            wait(100)
            
            compare(seekBar.value, 100, "At edge after switch to live")
        }
        
        // =====================================================================
        // Duration Changes
        // =====================================================================
        
        function test_durationChange_updatesRange() {
            seekBar.duration = 60
            compare(seekBar.to, 60, "Range updated to 60")
            
            seekBar.duration = 120
            compare(seekBar.to, 120, "Range updated to 120")
        }
        
        function test_durationZero_disablesSlider() {
            seekBar.duration = 100
            compare(seekBar.enabled, true, "Enabled with duration")
            
            seekBar.duration = 0
            compare(seekBar.enabled, false, "Disabled when duration is 0")
        }
        
        // =====================================================================
        // Edge Cases
        // =====================================================================
        
        function test_positionExceedsDuration() {
            seekBar.liveMode = false
            seekBar.duration = 100
            seekBar.currentPosition = 150 // Exceeds duration
            wait(50)
            
            // Value should be clamped by slider's 'to'
            verify(seekBar.value <= seekBar.to, "Value clamped to duration")
        }
        
        function test_negativePosition() {
            seekBar.liveMode = false
            seekBar.duration = 100
            seekBar.currentPosition = -10
            wait(50)
            
            // Value should be at least 0
            verify(seekBar.value >= 0, "Value at least 0")
        }
        
        function test_rapidPositionUpdates() {
            seekBar.liveMode = false
            seekBar.duration = 100
            
            for (var i = 0; i <= 100; i += 10) {
                seekBar.currentPosition = i
            }
            wait(50)
            
            compare(seekBar.currentPosition, 100, "Position updated to 100")
        }
        
        // =====================================================================
        // Combined State Tests
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
            seekBar.liveMode = data.live
            seekBar.isReplayMode = data.replay
            
            compare(seekBar.isLiveMode, data.expectedLive, "isLiveMode is " + data.expectedLive)
        }
        
        // =====================================================================
        // Signal Verification
        // =====================================================================
        
        function test_noSignalsWithoutInteraction() {
            seekBar.duration = 100
            seekBar.liveMode = false
            seekBar.currentPosition = 50
            wait(100)
            
            compare(seekRequestedSpy.count, 0, "No seekRequested without interaction")
            compare(seekDragStartedSpy.count, 0, "No seekDragStarted without interaction")
            compare(seekDragEndedSpy.count, 0, "No seekDragEnded without interaction")
            compare(seekPreviewedSpy.count, 0, "No seekPreviewed without interaction")
        }
        
        function test_signalOrder() {
            seekBar.duration = 100
            seekBar.liveMode = false
            wait(50)
            
            // Press
            mousePress(seekBar, seekBar.width / 2, seekBar.height / 2)
            wait(50)
            compare(seekDragStartedSpy.count, 1, "seekDragStarted emitted on press")
            compare(seekDragEndedSpy.count, 0, "No seekDragEnded yet")
            
            // Release
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
            wait(50)
            compare(seekDragEndedSpy.count, 1, "seekDragEnded emitted on release")
            compare(seekRequestedSpy.count, 1, "seekRequested emitted on release")
        }
        
        // =====================================================================
        // Binding Stability Tests
        // =====================================================================
        
        function test_bindingNotBrokenByDrag() {
            seekBar.liveMode = false
            seekBar.duration = 100
            seekBar.currentPosition = 25
            wait(100)
            
            tryCompare(seekBar, "value", 25, 200, "Value at 25")
            
            // Drag and release
            mousePress(seekBar, seekBar.width / 2, seekBar.height / 2)
            wait(50)
            mouseRelease(seekBar, seekBar.width / 2, seekBar.height / 2)
            wait(100)
            
            // Now update position - binding should still work
            seekBar.currentPosition = 75
            wait(100)
            
            tryCompare(seekBar, "value", 75, 200, "Value follows position after drag")
        }
        
        // =====================================================================
        // Integration Tests
        // =====================================================================
        
        function test_fullVodPlayback() {
            seekBar.liveMode = false
            seekBar.duration = 600 // 10 minutes
            
            // Simulate playback
            for (var pos = 0; pos <= 600; pos += 60) {
                seekBar.currentPosition = pos
                wait(10)
            }
            
            compare(seekBar.currentPosition, 600, "Playback completed")
            compare(seekBar.atLiveEdge, true, "At edge at end of VOD")
        }
        
        function test_seekDuringPlayback() {
            seekBar.liveMode = false
            seekBar.duration = 100
            seekBar.currentPosition = 20
            wait(100)
            
            // User seeks
            mousePress(seekBar, seekBar.width * 0.8, seekBar.height / 2)
            wait(50)
            mouseRelease(seekBar, seekBar.width * 0.8, seekBar.height / 2)
            wait(50)
            
            compare(seekRequestedSpy.count, 1, "Seek requested")
            verify(seekRequestedSpy.signalArguments[0][0] > 50, "Seek is forward")
        }
    }
}
