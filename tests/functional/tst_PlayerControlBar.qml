/**
 * tst_PlayerControlBar.qml
 * 
 * Functional UI tests for the PlayerControlBar component.
 * 
 * This is a POC (Proof of Concept) that demonstrates:
 * 1. How to structure QML functional tests
 * 2. How to use SignalSpy to capture signals
 * 3. How to simulate user interactions
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

    // =========================================================================
    // Component Under Test - Embedded for proper scene attachment
    // =========================================================================
    
    Item {
        id: controlBar
        anchors.fill: parent
        anchors.margins: 20
        
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
        
        // Reset function for tests
        function reset() {
            playing = false
            paused = false
            volume = 1.0
            muted = false
            playbackRate = 1.0
            chatVisible = false
        }
        
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
                text: controlBar.playing && !controlBar.paused ? "⏸ Pause" : "▶ Play"
                onClicked: controlBar.playPauseClicked()
            }
            
            // Volume Button
            Button {
                id: volumeBtn
                objectName: "volumeButton"
                width: 60
                height: 48
                text: controlBar.muted ? "🔇" : "🔊"
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
                text: "−"
                font.pixelSize: 20
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
                font.pixelSize: 20
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
                text: "⛶"
                onClicked: controlBar.fullscreenClicked()
            }
            
            // Chat Toggle
            Button {
                id: chatBtn
                objectName: "chatButton"
                width: 48
                height: 48
                text: "💬"
                checkable: true
                checked: controlBar.chatVisible
                onClicked: controlBar.chatToggleClicked()
            }
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: playPauseSpy; target: controlBar; signalName: "playPauseClicked" }
    SignalSpy { id: muteSpy; target: controlBar; signalName: "muteClicked" }
    SignalSpy { id: volumeSpy; target: controlBar; signalName: "volumeRequested" }
    SignalSpy { id: fullscreenSpy; target: controlBar; signalName: "fullscreenClicked" }
    SignalSpy { id: rateSpy; target: controlBar; signalName: "playbackRateRequested" }
    SignalSpy { id: chatSpy; target: controlBar; signalName: "chatToggleClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "PlayerControlBarTests"
        when: windowShown

        function init() {
            // Reset component state before each test
            controlBar.reset()
            
            // Clear all spies
            playPauseSpy.clear()
            muteSpy.clear()
            volumeSpy.clear()
            fullscreenSpy.clear()
            rateSpy.clear()
            chatSpy.clear()
            
            // Wait for rendering
            wait(50)
        }

        // =====================================================================
        // TEST: Play/Pause Button
        // =====================================================================
        
        function test_playPauseButton_click_emitsSignal() {
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
            // Arrange
            var button = findChild(controlBar, "playPauseButton")
            
            // Act
            mouseClick(button)
            mouseClick(button)
            
            // Assert
            compare(playPauseSpy.count, 2, "playPauseClicked should be emitted twice")
        }
        
        function test_playPauseButton_showsPlayIcon_initially() {
            // Arrange
            var button = findChild(controlBar, "playPauseButton")
            
            // Assert
            verify(button.text.indexOf("Play") >= 0, "Should show Play text initially")
        }
        
        function test_playPauseButton_showsPauseIcon_whenPlaying() {
            // Arrange
            var button = findChild(controlBar, "playPauseButton")
            
            // Act
            controlBar.playing = true
            controlBar.paused = false
            wait(50)
            
            // Assert
            verify(button.text.indexOf("Pause") >= 0, "Should show Pause text when playing")
        }

        // =====================================================================
        // TEST: Volume Controls
        // =====================================================================
        
        function test_volumeButton_click_emitsMuteSignal() {
            // Arrange
            var button = findChild(controlBar, "volumeButton")
            verify(button !== null, "volumeButton should exist")
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(muteSpy.count, 1, "muteClicked should be emitted")
        }
        
        function test_volumeIcon_showsVolumeIcon_initially() {
            // Arrange
            var button = findChild(controlBar, "volumeButton")
            
            // Assert
            compare(button.text, "🔊", "Should show volume icon initially")
        }
        
        function test_volumeIcon_showsMutedIcon_whenMuted() {
            // Arrange
            var button = findChild(controlBar, "volumeButton")
            
            // Act
            controlBar.muted = true
            wait(50)
            
            // Assert
            compare(button.text, "🔇", "Should show muted icon")
        }

        // =====================================================================
        // TEST: Playback Speed Controls
        // =====================================================================
        
        function test_speedUpButton_click_emitsSignal() {
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
            // Arrange
            var button = findChild(controlBar, "speedUpButton")
            controlBar.playbackRate = 2.75
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(rateSpy.signalArguments[0][0], 3.0, "Rate should cap at 3.0")
        }
        
        function test_speedDown_respectsMinimum_025x() {
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
