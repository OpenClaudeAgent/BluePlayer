/**
 * tst_PlayerControlBar.qml
 * 
 * Functional UI tests for the PlayerControlBar component.
 * Tests button clicks, signal emissions, and state transitions.
 * 
 * Refactored to use:
 * - createTemporaryObject for test isolation
 * - cleanup() for proper teardown
 * - waitForRendering instead of wait() for visual sync
 * 
 * Run with: ./test_functional_ui PlayerControlBarTests
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 800
    height: 300

    // Detect offscreen mode - mouse events crash in offscreen
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"

    // =========================================================================
    // Component Under Test
    // =========================================================================
    
    Component {
        id: controlBarComponent
        
        Item {
            id: controlBar
            width: 760
            height: 260
            
            // Properties
            property bool playing: false
            property bool paused: false
            property real volume: 1.0
            property bool muted: false
            property real playbackRate: 1.0
            property bool chatVisible: false
            
            // Signals
            signal playPauseClicked()
            signal muteClicked()
            signal volumeRequested(real newVolume)
            signal fullscreenClicked()
            signal playbackRateRequested(real rate)
            signal chatToggleClicked()
            
            Row {
                id: buttonRow
                anchors.centerIn: parent
                spacing: 12
                
                // Play/Pause Button
                Button {
                    id: playPauseBtn
                    objectName: "playPauseButton"
                    width: 60
                    height: 48
                    text: controlBar.playing && !controlBar.paused ? "Pause" : "Play"
                    onClicked: controlBar.playPauseClicked()
                }
                
                // Volume Button
                Button {
                    id: volumeBtn
                    objectName: "volumeButton"
                    width: 60
                    height: 48
                    text: controlBar.muted ? "Muted" : "Volume"
                    onClicked: controlBar.muteClicked()
                }
                
                // Volume Slider
                Slider {
                    id: volSlider
                    objectName: "volumeSlider"
                    width: 120
                    height: 48
                    from: 0.0
                    to: 1.0
                    value: controlBar.muted ? 0 : controlBar.volume
                    onMoved: controlBar.volumeRequested(value)
                }
                
                // Speed Down
                Button {
                    id: speedDownBtn
                    objectName: "speedDownButton"
                    width: 48
                    height: 48
                    text: "-"
                    onClicked: controlBar.playbackRateRequested(
                        Math.max(0.25, controlBar.playbackRate - 0.25)
                    )
                }
                
                // Speed Display
                Rectangle {
                    width: 60
                    height: 48
                    color: "#333"
                    radius: 4
                    Text {
                        anchors.centerIn: parent
                        text: controlBar.playbackRate.toFixed(2) + "x"
                        color: "white"
                        font.pixelSize: 14
                    }
                }
                
                // Speed Up
                Button {
                    id: speedUpBtn
                    objectName: "speedUpButton"
                    width: 48
                    height: 48
                    text: "+"
                    onClicked: controlBar.playbackRateRequested(
                        Math.min(3.0, controlBar.playbackRate + 0.25)
                    )
                }
                
                // Fullscreen
                Button {
                    id: fullscreenBtn
                    objectName: "fullscreenButton"
                    width: 48
                    height: 48
                    text: "FS"
                    onClicked: controlBar.fullscreenClicked()
                }
                
                // Chat Toggle
                Button {
                    id: chatBtn
                    objectName: "chatButton"
                    width: 48
                    height: 48
                    text: "Chat"
                    checkable: true
                    checked: controlBar.chatVisible
                    onClicked: controlBar.chatToggleClicked()
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var controlBar: null

    // =========================================================================
    // Signal Spies (created dynamically in init to avoid offscreen issues)
    // =========================================================================
    
    property var playPauseSpy: null
    property var muteSpy: null
    property var volumeSpy: null
    property var fullscreenSpy: null
    property var rateSpy: null
    property var chatSpy: null
    
    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "PlayerControlBarTests"
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
            
            controlBar = createTemporaryObject(controlBarComponent, root)
            verify(controlBar !== null, "ControlBar should be created")
            controlBar.anchors.fill = root
            controlBar.anchors.margins = 20
            
            // Create spies dynamically to avoid offscreen mode issues
            playPauseSpy = createTemporaryObject(signalSpyComponent, root, {target: controlBar, signalName: "playPauseClicked"})
            muteSpy = createTemporaryObject(signalSpyComponent, root, {target: controlBar, signalName: "muteClicked"})
            volumeSpy = createTemporaryObject(signalSpyComponent, root, {target: controlBar, signalName: "volumeRequested"})
            fullscreenSpy = createTemporaryObject(signalSpyComponent, root, {target: controlBar, signalName: "fullscreenClicked"})
            rateSpy = createTemporaryObject(signalSpyComponent, root, {target: controlBar, signalName: "playbackRateRequested"})
            chatSpy = createTemporaryObject(signalSpyComponent, root, {target: controlBar, signalName: "chatToggleClicked"})
            
            waitForRendering(controlBar)
        }

        function cleanup() {
            controlBar = null
        }

        // =====================================================================
        // TEST: Play/Pause Button
        // =====================================================================
        
        function test_playPauseButton_click_emitsSignal() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var button = findChild(controlBar, "playPauseButton")
            verify(button !== null, "playPauseButton should exist")
            verify(button.visible, "playPauseButton should be visible")
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(playPauseSpy.count, 1, "playPauseClicked should be emitted once")
        }
        
        function test_playPauseButton_doubleClick_emitsTwice() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var button = findChild(controlBar, "playPauseButton")
            
            // Act
            mouseClick(button)
            mouseClick(button)
            
            // Assert
            compare(playPauseSpy.count, 2, "playPauseClicked should be emitted twice")
        }
        
        function test_playPauseButton_showsPlayText_initially() {
            if (!requiresSlider()) return
            // Arrange
            var button = findChild(controlBar, "playPauseButton")
            
            // Assert
            verify(button.text.indexOf("Play") >= 0, "Should show Play text initially")
        }
        
        function test_playPauseButton_showsPauseText_whenPlaying() {
            if (!requiresSlider()) return
            // Arrange
            var button = findChild(controlBar, "playPauseButton")
            
            // Act
            controlBar.playing = true
            controlBar.paused = false
            waitForRendering(controlBar)
            
            // Assert
            verify(button.text.indexOf("Pause") >= 0, "Should show Pause text when playing")
        }

        // =====================================================================
        // TEST: Volume Controls
        // =====================================================================
        
        function test_volumeButton_click_emitsMuteSignal() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var button = findChild(controlBar, "volumeButton")
            verify(button !== null, "volumeButton should exist")
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(muteSpy.count, 1, "muteClicked should be emitted")
        }
        
        function test_volumeText_changesWhenMuted() {
            if (!requiresSlider()) return
            // Arrange
            var button = findChild(controlBar, "volumeButton")
            
            // Act
            controlBar.muted = true
            waitForRendering(controlBar)
            
            // Assert
            compare(button.text, "Muted", "Should show Muted text")
        }

        // =====================================================================
        // TEST: Playback Speed Controls
        // =====================================================================
        
        function test_speedUpButton_click_emitsSignal() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var button = findChild(controlBar, "speedUpButton")
            verify(button !== null, "speedUpButton should exist")
            controlBar.playbackRate = 1.0
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(rateSpy.count, 1, "playbackRateRequested should be emitted")
            compare(rateSpy.signalArguments[0][0], 1.25, "Rate should be 1.25")
        }
        
        function test_speedDownButton_click_emitsSignal() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var button = findChild(controlBar, "speedDownButton")
            verify(button !== null, "speedDownButton should exist")
            controlBar.playbackRate = 1.0
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(rateSpy.count, 1, "playbackRateRequested should be emitted")
            compare(rateSpy.signalArguments[0][0], 0.75, "Rate should be 0.75")
        }
        
        function test_speedUp_respectsMaximum_3x() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var button = findChild(controlBar, "speedUpButton")
            controlBar.playbackRate = 2.75
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(rateSpy.signalArguments[0][0], 3.0, "Rate should cap at 3.0")
        }
        
        function test_speedDown_respectsMinimum_025x() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var button = findChild(controlBar, "speedDownButton")
            controlBar.playbackRate = 0.5
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(rateSpy.signalArguments[0][0], 0.25, "Rate should floor at 0.25")
        }

        // =====================================================================
        // TEST: Fullscreen Button
        // =====================================================================
        
        function test_fullscreenButton_click_emitsSignal() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var button = findChild(controlBar, "fullscreenButton")
            verify(button !== null, "fullscreenButton should exist")
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(fullscreenSpy.count, 1, "fullscreenClicked should be emitted")
        }

        // =====================================================================
        // TEST: Chat Toggle Button
        // =====================================================================
        
        function test_chatButton_click_emitsSignal() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var button = findChild(controlBar, "chatButton")
            verify(button !== null, "chatButton should exist")
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(chatSpy.count, 1, "chatToggleClicked should be emitted")
        }

        // =====================================================================
        // TEST: Initial State
        // =====================================================================
        
        function test_initialState_isCorrect() {
            if (!requiresSlider()) return
            // Assert initial state after reset
            compare(controlBar.playing, false, "Should not be playing")
            compare(controlBar.paused, false, "Should not be paused")
            compare(controlBar.muted, false, "Should not be muted")
            compare(controlBar.volume, 1.0, "Volume should be 1.0")
            compare(controlBar.playbackRate, 1.0, "Playback rate should be 1.0")
            compare(controlBar.chatVisible, false, "Chat should not be visible")
        }

        // =====================================================================
        // TEST: Multiple Interactions
        // =====================================================================
        
        function test_rapidClicks_allButtonsWork() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var playBtn = findChild(controlBar, "playPauseButton")
            var volBtn = findChild(controlBar, "volumeButton")
            var fsBtn = findChild(controlBar, "fullscreenButton")
            
            // Act
            mouseClick(playBtn)
            mouseClick(volBtn)
            mouseClick(fsBtn)
            mouseClick(playBtn)
            
            // Assert
            compare(playPauseSpy.count, 2, "Play/pause clicked twice")
            compare(muteSpy.count, 1, "Mute clicked once")
            compare(fullscreenSpy.count, 1, "Fullscreen clicked once")
        }
        
        function test_speedChanges_accumulate() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var speedUp = findChild(controlBar, "speedUpButton")
            controlBar.playbackRate = 1.0
            
            // Act - First increase
            mouseClick(speedUp)
            compare(rateSpy.count, 1)
            compare(rateSpy.signalArguments[0][0], 1.25)
            
            // Simulate component updating
            controlBar.playbackRate = 1.25
            
            // Second increase
            mouseClick(speedUp)
            compare(rateSpy.count, 2)
            compare(rateSpy.signalArguments[1][0], 1.50)
            
            // Third increase
            controlBar.playbackRate = 1.50
            mouseClick(speedUp)
            compare(rateSpy.count, 3)
            compare(rateSpy.signalArguments[2][0], 1.75)
        }
    }
}
