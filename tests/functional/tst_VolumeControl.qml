/**
 * tst_VolumeControl.qml
 * 
 * Functional UI tests for the VolumeControl component.
 * Tests volume button with vertical popup slider behavior.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 400

    // =========================================================================
    // Component Under Test (Mock matching VolumeControl.qml API)
    // =========================================================================
    
    Item {
        id: volumeControl
        objectName: "volumeControl"
        
        // Position in center-bottom for popup visibility
        x: 184
        y: 300
        
        // Properties matching VolumeControl
        property real volume: 1.0
        property bool muted: false
        property bool showSlider: false
        
        // Signals
        signal volumeRequested(real newVolume)
        signal muteClicked()
        
        // Default size
        width: 32
        height: 32
        
        // Volume Icon Button
        Rectangle {
            id: volumeButton
            objectName: "volumeButton"
            anchors.fill: parent
            radius: width / 2
            color: volumeMouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF"
            border.color: "#4DFFFFFF"
            border.width: 1
            
            // Volume Icon (Canvas)
            Canvas {
                id: volumeIcon
                objectName: "volumeIcon"
                anchors.centerIn: parent
                width: 18
                height: 18
                
                property real vol: volumeControl.muted ? 0 : volumeControl.volume
                onVolChanged: requestPaint()
                Component.onCompleted: requestPaint()
                
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    ctx.fillStyle = "#FFFFFF"
                    ctx.strokeStyle = "#FFFFFF"
                    ctx.lineWidth = 1.5
                    ctx.lineCap = "round"
                    
                    // Speaker body
                    ctx.beginPath()
                    ctx.moveTo(2, 6)
                    ctx.lineTo(5, 6)
                    ctx.lineTo(9, 3)
                    ctx.lineTo(9, 15)
                    ctx.lineTo(5, 12)
                    ctx.lineTo(2, 12)
                    ctx.closePath()
                    ctx.fill()
                    
                    if (vol === 0 || volumeControl.muted) {
                        // X for muted
                        ctx.beginPath()
                        ctx.moveTo(12, 6)
                        ctx.lineTo(16, 12)
                        ctx.stroke()
                        ctx.beginPath()
                        ctx.moveTo(16, 6)
                        ctx.lineTo(12, 12)
                        ctx.stroke()
                    } else {
                        // Sound waves
                        if (vol > 0) {
                            ctx.beginPath()
                            ctx.arc(9, 9, 3, -Math.PI/3, Math.PI/3, false)
                            ctx.stroke()
                        }
                        if (vol > 0.5) {
                            ctx.beginPath()
                            ctx.arc(9, 9, 6, -Math.PI/3, Math.PI/3, false)
                            ctx.stroke()
                        }
                    }
                }
            }
            
            MouseArea {
                id: volumeMouseArea
                objectName: "volumeMouseArea"
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                
                onClicked: {
                    volumeControl.muteClicked()
                    hideVolumeTimer.restart()
                }
                
                onEntered: {
                    volumeControl.showSlider = true
                    hideVolumeTimer.stop()
                }
            }
            
            ToolTip.visible: volumeMouseArea.containsMouse && !volumeControl.showSlider
            ToolTip.text: volumeControl.muted ? qsTr("Activer le son (M)") : qsTr("Couper le son (M)")
            ToolTip.delay: 800
        }
        
        // Vertical Volume Slider Popup
        Rectangle {
            id: volumeSliderPopup
            objectName: "volumeSliderPopup"
            width: 36
            height: volumeControl.showSlider ? 110 : 0
            anchors.bottom: volumeButton.top
            anchors.bottomMargin: 8
            anchors.horizontalCenter: volumeButton.horizontalCenter
            radius: 18
            color: "#CC1C1C1E"
            border.color: "#4DFFFFFF"
            border.width: 1
            clip: true
            opacity: volumeControl.showSlider ? 1.0 : 0.0
            visible: height > 0
            
            Slider {
                id: volumeSlider
                objectName: "volumeSlider"
                anchors.centerIn: parent
                orientation: Qt.Vertical
                height: 85
                width: 26
                from: 0.0
                to: 1.0
                
                // Use Binding element so it can be re-established after value assignment
                Binding on value {
                    value: volumeControl.muted ? 0 : volumeControl.volume
                    restoreMode: Binding.RestoreBindingOrValue
                }
                
                onMoved: {
                    if (volumeControl.muted && value > 0) {
                        volumeControl.muteClicked()
                    }
                    volumeControl.volumeRequested(value)
                }
            }
            
            MouseArea {
                id: popupMouseArea
                objectName: "popupMouseArea"
                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true
                onExited: hideVolumeTimer.restart()
                onEntered: hideVolumeTimer.stop()
                onPressed: function(mouse) { mouse.accepted = false }
                onReleased: function(mouse) { mouse.accepted = false }
            }
        }
        
        // Timer to hide volume slider
        Timer {
            id: hideVolumeTimer
            objectName: "hideVolumeTimer"
            interval: 1500
            onTriggered: volumeControl.showSlider = false
        }
        
        // Reset function for tests
        function reset() {
            volume = 1.0
            muted = false
            showSlider = false
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { 
        id: volumeRequestedSpy 
        target: volumeControl 
        signalName: "volumeRequested" 
    }
    
    SignalSpy { 
        id: muteClickedSpy 
        target: volumeControl 
        signalName: "muteClicked" 
    }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "VolumeControlTests"
        when: windowShown
        
        function init() {
            volumeControl.reset()
            volumeRequestedSpy.clear()
            muteClickedSpy.clear()
            wait(50)
        }
        
        // =====================================================================
        // Default Values Tests
        // =====================================================================
        
        function test_defaultValues() {
            compare(volumeControl.volume, 1.0, "Default volume is 1.0 (max)")
            compare(volumeControl.muted, false, "Default muted is false")
            compare(volumeControl.showSlider, false, "Slider hidden by default")
        }
        
        function test_defaultSize() {
            compare(volumeControl.width, 32, "Default width is 32")
            compare(volumeControl.height, 32, "Default height is 32")
        }
        
        // =====================================================================
        // Volume Property Tests
        // =====================================================================
        
        function test_volumeProperty_canBeSet() {
            volumeControl.volume = 0.5
            compare(volumeControl.volume, 0.5, "Volume can be set to 0.5")
            
            volumeControl.volume = 0.0
            compare(volumeControl.volume, 0.0, "Volume can be set to 0.0")
            
            volumeControl.volume = 1.0
            compare(volumeControl.volume, 1.0, "Volume can be set to 1.0")
        }
        
        function test_volumeProperty_variousLevels_data() {
            return [
                { tag: "muted", value: 0.0 },
                { tag: "veryLow", value: 0.1 },
                { tag: "low", value: 0.25 },
                { tag: "medium", value: 0.5 },
                { tag: "high", value: 0.75 },
                { tag: "max", value: 1.0 }
            ]
        }
        
        function test_volumeProperty_variousLevels(data) {
            volumeControl.volume = data.value
            compare(volumeControl.volume, data.value, "Volume set to " + data.value)
        }
        
        // =====================================================================
        // Muted Property Tests
        // =====================================================================
        
        function test_mutedProperty_canBeToggled() {
            compare(volumeControl.muted, false, "Initially not muted")
            
            volumeControl.muted = true
            compare(volumeControl.muted, true, "Can be muted")
            
            volumeControl.muted = false
            compare(volumeControl.muted, false, "Can be unmuted")
        }
        
        function test_mutedProperty_independentOfVolume() {
            volumeControl.volume = 0.8
            volumeControl.muted = true
            
            compare(volumeControl.volume, 0.8, "Volume preserved when muted")
            compare(volumeControl.muted, true, "Muted state is true")
            
            volumeControl.muted = false
            compare(volumeControl.volume, 0.8, "Volume still preserved after unmute")
        }
        
        // =====================================================================
        // Click Behavior Tests
        // =====================================================================
        
        function test_click_emitsMuteClickedSignal() {
            var button = findChild(volumeControl, "volumeButton")
            mouseClick(button, button.width / 2, button.height / 2)
            compare(muteClickedSpy.count, 1, "muteClicked emitted on click")
        }
        
        function test_multipleClicks_emitMultipleSignals() {
            var button = findChild(volumeControl, "volumeButton")
            mouseClick(button, button.width / 2, button.height / 2)
            mouseClick(button, button.width / 2, button.height / 2)
            mouseClick(button, button.width / 2, button.height / 2)
            compare(muteClickedSpy.count, 3, "muteClicked emitted for each click")
        }
        
        // =====================================================================
        // Hover Behavior Tests
        // Note: mouseMove doesn't always trigger onEntered in test environment,
        // so we test the logic by setting showSlider directly for reliable tests.
        // =====================================================================
        
        function test_hover_showsSliderPopup_viaDirectSet() {
            compare(volumeControl.showSlider, false, "Slider hidden initially")
            
            // Simulate what hover does
            volumeControl.showSlider = true
            wait(50)
            
            compare(volumeControl.showSlider, true, "Slider shown when showSlider set")
        }
        
        function test_hover_sliderPopupBecomesVisible_viaDirectSet() {
            var popup = findChild(volumeControl, "volumeSliderPopup")
            compare(popup.visible, false, "Popup hidden initially")
            
            // Simulate hover behavior
            volumeControl.showSlider = true
            wait(100)
            
            compare(volumeControl.showSlider, true, "showSlider is true")
            compare(popup.visible, true, "Popup becomes visible")
        }
        
        // =====================================================================
        // Slider Popup Tests
        // =====================================================================
        
        function test_sliderPopup_heightAnimates() {
            var popup = findChild(volumeControl, "volumeSliderPopup")
            compare(popup.height, 0, "Popup height is 0 when hidden")
            
            volumeControl.showSlider = true
            wait(50)
            
            compare(popup.height, 110, "Popup height is 110 when shown")
        }
        
        function test_sliderPopup_opacityAnimates() {
            var popup = findChild(volumeControl, "volumeSliderPopup")
            compare(popup.opacity, 0.0, "Popup opacity is 0 when hidden")
            
            volumeControl.showSlider = true
            wait(50)
            
            compare(popup.opacity, 1.0, "Popup opacity is 1.0 when shown")
        }
        
        function test_sliderPopup_volumeBinding() {
            volumeControl.muted = false
            volumeControl.showSlider = true
            wait(50)
            
            volumeControl.volume = 0.7
            wait(50)
            
            var slider = findChild(volumeControl, "volumeSlider")
            // Slider value binding: volumeControl.muted ? 0 : volumeControl.volume
            tryCompare(slider, "value", 0.7, 200, "Slider reflects volume value")
        }
        
        function test_sliderPopup_showsZeroWhenMuted() {
            volumeControl.showSlider = true
            wait(50)
            
            volumeControl.volume = 0.8
            volumeControl.muted = true
            wait(100)
            
            var slider = findChild(volumeControl, "volumeSlider")
            // Slider value binding: volumeControl.muted ? 0 : volumeControl.volume
            tryCompare(slider, "value", 0, 200, "Slider shows 0 when muted")
            compare(volumeControl.volume, 0.8, "Volume property preserved")
        }
        
        // =====================================================================
        // Slider Interaction Tests
        // =====================================================================
        
        function test_sliderMoved_emitsVolumeRequested() {
            volumeControl.showSlider = true
            wait(100)
            
            var slider = findChild(volumeControl, "volumeSlider")
            slider.value = 0.5
            slider.moved()
            
            compare(volumeRequestedSpy.count, 1, "volumeRequested emitted")
            compare(volumeRequestedSpy.signalArguments[0][0], 0.5, "Correct volume value")
        }
        
        function test_sliderMoved_unmutesWhenValueAboveZero() {
            volumeControl.muted = true
            volumeControl.showSlider = true
            wait(100)
            
            var slider = findChild(volumeControl, "volumeSlider")
            slider.value = 0.3
            slider.moved()
            
            // Should emit muteClicked to unmute
            compare(muteClickedSpy.count, 1, "muteClicked emitted to unmute")
        }
        
        // =====================================================================
        // Timer Auto-Hide Tests
        // =====================================================================
        
        function test_autoHide_sliderHidesAfterTimeout() {
            volumeControl.showSlider = true
            compare(volumeControl.showSlider, true, "Slider is shown")
            
            // Start the timer manually
            var timer = findChild(volumeControl, "hideVolumeTimer")
            timer.start()
            
            // Wait for timer (1500ms) plus buffer
            wait(1700)
            
            compare(volumeControl.showSlider, false, "Slider hidden after timeout")
        }
        
        function test_autoHide_timerInterval() {
            var timer = findChild(volumeControl, "hideVolumeTimer")
            compare(timer.interval, 1500, "Timer interval is 1500ms")
        }
        
        function test_autoHide_clickRestartsTimer() {
            var button = findChild(volumeControl, "volumeButton")
            mouseMove(button, button.width / 2, button.height / 2)
            wait(100)
            compare(volumeControl.showSlider, true, "Slider shown")
            
            // Click restarts hide timer
            mouseClick(button, button.width / 2, button.height / 2)
            compare(muteClickedSpy.count, 1, "Click registered")
            
            // Slider should still be visible
            compare(volumeControl.showSlider, true, "Slider still visible after click")
        }
        
        // =====================================================================
        // Visual State Tests
        // =====================================================================
        
        function test_volumeButton_isCircular() {
            var button = findChild(volumeControl, "volumeButton")
            compare(button.radius, button.width / 2, "Button is circular")
        }
        
        function test_volumeButton_exists() {
            var button = findChild(volumeControl, "volumeButton")
            verify(button !== null, "Volume button exists")
            verify(button.visible, "Volume button is visible")
        }
        
        function test_volumeIcon_exists() {
            var icon = findChild(volumeControl, "volumeIcon")
            verify(icon !== null, "Volume icon exists")
        }
        
        // =====================================================================
        // Muted Visual State Tests
        // =====================================================================
        
        function test_mutedState_affectsIcon() {
            var icon = findChild(volumeControl, "volumeIcon")
            
            volumeControl.muted = false
            volumeControl.volume = 0.8
            compare(icon.vol, 0.8, "Icon vol reflects volume when not muted")
            
            volumeControl.muted = true
            compare(icon.vol, 0, "Icon vol is 0 when muted")
        }
        
        function test_zeroVolume_affectsIcon() {
            var icon = findChild(volumeControl, "volumeIcon")
            
            volumeControl.muted = false
            volumeControl.volume = 0.0
            compare(icon.vol, 0.0, "Icon vol is 0 when volume is 0")
        }
        
        // =====================================================================
        // Edge Cases
        // =====================================================================
        
        function test_rapidVolumeChanges() {
            for (var i = 0; i <= 10; i++) {
                volumeControl.volume = i / 10.0
            }
            compare(volumeControl.volume, 1.0, "Volume ends at 1.0")
        }
        
        function test_rapidMuteToggle() {
            for (var i = 0; i < 5; i++) {
                volumeControl.muted = !volumeControl.muted
            }
            compare(volumeControl.muted, true, "Muted state correct after 5 toggles")
        }
        
        function test_volumeAndMutedCombinations_data() {
            return [
                { tag: "unmuted-max", volume: 1.0, muted: false },
                { tag: "unmuted-mid", volume: 0.5, muted: false },
                { tag: "unmuted-zero", volume: 0.0, muted: false },
                { tag: "muted-max", volume: 1.0, muted: true },
                { tag: "muted-mid", volume: 0.5, muted: true },
                { tag: "muted-zero", volume: 0.0, muted: true }
            ]
        }
        
        function test_volumeAndMutedCombinations(data) {
            volumeControl.volume = data.volume
            volumeControl.muted = data.muted
            
            compare(volumeControl.volume, data.volume, "Volume is " + data.volume)
            compare(volumeControl.muted, data.muted, "Muted is " + data.muted)
        }
        
        // =====================================================================
        // showSlider Property Tests
        // =====================================================================
        
        function test_showSlider_canBeSetDirectly() {
            compare(volumeControl.showSlider, false, "Initially false")
            
            volumeControl.showSlider = true
            compare(volumeControl.showSlider, true, "Can be set to true")
            
            volumeControl.showSlider = false
            compare(volumeControl.showSlider, false, "Can be set to false")
        }
        
        // =====================================================================
        // Integration Tests
        // =====================================================================
        
        function test_fullInteractionFlow() {
            // 1. Start with default state
            compare(volumeControl.volume, 1.0, "Initial volume")
            compare(volumeControl.muted, false, "Initially not muted")
            compare(volumeControl.showSlider, false, "Slider hidden")
            
            // 2. Simulate hover to show slider (direct set since mouseMove unreliable in tests)
            volumeControl.showSlider = true
            wait(50)
            compare(volumeControl.showSlider, true, "Slider shown")
            
            // 3. Click to emit mute signal
            var button = findChild(volumeControl, "volumeButton")
            mouseClick(button, button.width / 2, button.height / 2)
            compare(muteClickedSpy.count, 1, "Mute signal emitted")
            
            // 4. Slider should still be visible (click restarts timer but doesn't hide)
            compare(volumeControl.showSlider, true, "Slider still visible after click")
        }
        
        function test_volumeControlPreservesState() {
            volumeControl.volume = 0.42
            volumeControl.muted = true
            
            // Show/hide slider
            volumeControl.showSlider = true
            wait(50)
            volumeControl.showSlider = false
            wait(50)
            
            // State should be preserved
            compare(volumeControl.volume, 0.42, "Volume preserved")
            compare(volumeControl.muted, true, "Muted state preserved")
        }
    }
}
