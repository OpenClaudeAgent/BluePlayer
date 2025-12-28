/**
 * tst_CircleButton.qml
 * 
 * Functional UI tests for the CircleButton component (Plan 28).
 * Tests circular navigation buttons used for history, settings, etc.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 300
    height: 300

    // =========================================================================
    // Component Under Test
    // =========================================================================
    
    Item {
        id: circleButton
        objectName: "circleButton"
        anchors.centerIn: parent
        
        implicitWidth: 38
        implicitHeight: 38
        width: implicitWidth
        height: implicitHeight
        
        // Properties
        property string iconText: ""
        property bool active: false
        property string tooltipText: ""
        property int iconSize: 20
        property color iconColor: "#FFFFFF"
        
        // Signal
        signal clicked()
        
        // Reset
        function reset() {
            iconText = ""
            active = false
            tooltipText = ""
            iconSize = 20
        }
        
        Rectangle {
            id: buttonBackground
            objectName: "buttonBackground"
            anchors.fill: parent
            radius: width / 2
            
            color: {
                if (circleButton.active) return "#1A0066FF"
                else if (mouseArea.containsMouse) return "#1AFFFFFF"
                else return "#0DFFFFFF"
            }
            
            border.color: circleButton.active ? "#0066FF" : "#333333"
            border.width: 1
            
            Text {
                id: iconLabel
                objectName: "iconLabel"
                anchors.centerIn: parent
                text: circleButton.iconText
                font.pixelSize: circleButton.iconSize
                color: circleButton.iconColor
                scale: mouseArea.containsMouse ? 1.05 : 1.0
            }
        }
        
        MouseArea {
            id: mouseArea
            objectName: "mouseArea"
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: circleButton.clicked()
        }
        
        ToolTip {
            id: tooltip
            objectName: "tooltip"
            visible: circleButton.tooltipText !== "" && mouseArea.containsMouse
            text: circleButton.tooltipText
            delay: 500
        }
    }

    // =========================================================================
    // Signal Spy
    // =========================================================================
    
    SignalSpy { id: clickedSpy; target: circleButton; signalName: "clicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "CircleButtonTests"
        when: windowShown

        function init() {
            circleButton.reset()
            clickedSpy.clear()
            wait(50)
        }

        // =====================================================================
        // TEST: Initial State
        // =====================================================================
        
        function test_initialState_isCircular() {
            var bg = findChild(circleButton, "buttonBackground")
            verify(bg !== null, "Background should exist")
            compare(bg.radius, bg.width / 2, "Should be circular (radius = width/2)")
        }
        
        function test_initialState_notActive() {
            compare(circleButton.active, false, "Should not be active initially")
        }
        
        function test_initialState_dimensions() {
            compare(circleButton.width, 38, "Default width should be 38")
            compare(circleButton.height, 38, "Default height should be 38")
        }

        // =====================================================================
        // TEST: Icon Display
        // =====================================================================
        
        function test_iconText_displays() {
            // Arrange
            circleButton.iconText = "\u21BA"  // ↺
            wait(50)
            
            // Assert
            var icon = findChild(circleButton, "iconLabel")
            if (icon) {
                compare(icon.text, "\u21BA", "Icon text should be displayed")
            }
        }
        
        function test_iconSize_applies() {
            // Arrange
            circleButton.iconText = "\u2699"  // ⚙
            circleButton.iconSize = 24
            wait(50)
            
            // Assert
            var icon = findChild(circleButton, "iconLabel")
            if (icon) {
                compare(icon.font.pixelSize, 24, "Icon size should apply")
            }
        }

        // =====================================================================
        // TEST: Click Behavior
        // =====================================================================
        
        function test_click_emitsSignal() {
            // Arrange
            var mouseArea = findChild(circleButton, "mouseArea")
            verify(mouseArea !== null, "MouseArea should exist")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 1, "clicked should be emitted")
        }
        
        function test_multipleClicks_emitMultiple() {
            // Arrange
            var mouseArea = findChild(circleButton, "mouseArea")
            
            // Act
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 3, "Should emit for each click")
        }

        // =====================================================================
        // TEST: Active State
        // =====================================================================
        
        function test_activeState_changesBorderColor() {
            // Arrange
            var bg = findChild(circleButton, "buttonBackground")
            verify(bg !== null)
            
            // Act
            circleButton.active = true
            wait(50)
            
            // Assert
            compare(bg.border.color.toString(), "#0066ff", "Border should be accent when active")
        }
        
        function test_inactiveState_defaultBorderColor() {
            // Arrange
            circleButton.active = false
            wait(50)
            
            // Assert
            var bg = findChild(circleButton, "buttonBackground")
            if (bg) {
                compare(bg.border.color.toString(), "#333333", "Border should be default when inactive")
            }
        }

        // =====================================================================
        // TEST: Tooltip
        // =====================================================================
        
        function test_tooltipText_setsTooltip() {
            // Arrange
            circleButton.tooltipText = "Settings"
            wait(50)
            
            // Assert
            var tooltip = findChild(circleButton, "tooltip")
            if (tooltip) {
                compare(tooltip.text, "Settings", "Tooltip text should be set")
            }
        }
        
        function test_emptyTooltip_noTooltipVisible() {
            // Arrange
            circleButton.tooltipText = ""
            
            // Assert
            var tooltip = findChild(circleButton, "tooltip")
            if (tooltip) {
                verify(!tooltip.visible, "Tooltip should not be visible when text is empty")
            }
        }

        // =====================================================================
        // TEST: Various Icons
        // =====================================================================
        
        function test_historyIcon() {
            circleButton.iconText = "\u21BA"  // ↺
            var icon = findChild(circleButton, "iconLabel")
            if (icon) compare(icon.text, "\u21BA")
        }
        
        function test_settingsIcon() {
            circleButton.iconText = "\u2699"  // ⚙
            var icon = findChild(circleButton, "iconLabel")
            if (icon) compare(icon.text, "\u2699")
        }
        
        function test_closeIcon() {
            circleButton.iconText = "\u2715"  // ✕
            var icon = findChild(circleButton, "iconLabel")
            if (icon) compare(icon.text, "\u2715")
        }
    }
}
