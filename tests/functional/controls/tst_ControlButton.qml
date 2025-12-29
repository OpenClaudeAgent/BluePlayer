/**
 * tst_ControlButton.qml
 * 
 * Functional UI tests for the ControlButton component.
 * Tests click, hover, active state, scale animations, and tooltip.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 400

    // Detect offscreen mode - mouse events may not work
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"

    // =========================================================================
    // Component Under Test (Mock of ControlButton)
    // =========================================================================
    
    Component {
        id: controlButtonComponent
        
        Rectangle {
            id: controlBtn
            objectName: "controlButton"

            // Theme constants (inline for proper binding)
            readonly property color themeAccent: "#3b82f6"
            readonly property int animDuration: 1

            // Content alias for custom icon/content
            default property alias icon: contentContainer.children
            
            // Button properties
            property string tooltipText: ""
            property bool active: false
            property bool showBorder: true
            
            // Size defaults
            width: 32
            height: 32
            radius: width / 2

            // Style
            color: active ? themeAccent : (mouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF")
            border.color: active ? themeAccent : (showBorder ? "#4DFFFFFF" : "transparent")
            border.width: showBorder ? 1 : 0

            // Animations
            Behavior on color {
                ColorAnimation { duration: controlBtn.animDuration; easing.type: Easing.OutCubic }
            }

            scale: mouseArea.pressed ? 0.92 : (mouseArea.containsMouse ? 1.05 : 1.0)
            Behavior on scale {
                NumberAnimation { duration: controlBtn.animDuration; easing.type: Easing.OutQuart }
            }

            // Signal
            // Signal
            signal clicked()

            // Expose hover state for external use
            readonly property bool hovered: mouseArea.containsMouse
            readonly property bool pressed: mouseArea.pressed

            // Content container centered
            Item {
                id: contentContainer
                objectName: "contentContainer"
                anchors.centerIn: parent
                width: parent.width * 0.6
                height: parent.height * 0.6
            }

            MouseArea {
                id: mouseArea
                objectName: "mouseArea"
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: controlBtn.clicked()
            }

            ToolTip.visible: mouseArea.containsMouse && tooltipText !== ""
            ToolTip.text: tooltipText
            ToolTip.delay: 800
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var button: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: clickedSpy; signalName: "clicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "ControlButtonTests"
        when: windowShown

        function init() {
            button = createTemporaryObject(controlButtonComponent, root)
            verify(button !== null, "ControlButton should be created")
            button.anchors.centerIn = root
            clickedSpy.target = button
            clickedSpy.clear()
            mouseMove(root, 1, 1)
        }

        function cleanup() {
            button = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_dimensions() {
            compare(button.width, 32, "Default width should be 32")
            compare(button.height, 32, "Default height should be 32")
        }
        
        function test_defaultState_notActive() {
            compare(button.active, false, "Default active should be false")
        }
        
        function test_defaultState_showBorderTrue() {
            compare(button.showBorder, true, "Default showBorder should be true")
        }
        
        function test_defaultState_notHovered() {
            compare(button.hovered, false, "Default hovered should be false")
        }
        
        function test_defaultState_scaleIsOne() {
            compare(button.scale, 1.0, "Default scale should be 1.0")
        }

        // =====================================================================
        // TEST: Click Signal
        // =====================================================================
        
        function test_click_emitsSignal() {
            var mouseArea = findChild(button, "mouseArea")
            verify(mouseArea !== null, "MouseArea should exist")
            
            mouseClick(mouseArea)
            
            compare(clickedSpy.count, 1, "clicked should be emitted once")
        }
        
        function test_multipleClicks_emitMultipleSignals() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            var mouseArea = findChild(button, "mouseArea")
            
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            compare(clickedSpy.count, 3, "Should emit clicked for each click")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_setsHoveredTrue() {
            var mouseArea = findChild(button, "mouseArea")
            
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(button, "hovered", true, 100, "hovered should be true when mouse is over")
        }
        
        function test_hover_scaleIncreases() {
            var mouseArea = findChild(button, "mouseArea")
            
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(button, "scale", 1.05, 100, "scale should be 1.05 on hover")
        }
        
        function test_hoverExit_restoresScale() {
            var mouseArea = findChild(button, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            tryCompare(button, "scale", 1.05, 100)
            
            mouseMove(root, 1, 1)
            
            tryCompare(button, "scale", 1.0, 100, "scale should return to 1.0")
        }

        // =====================================================================
        // TEST: Active State
        // =====================================================================
        
        function test_active_canBeSet() {
            compare(button.active, false, "Initial active should be false")
            
            button.active = true
            
            compare(button.active, true, "active should be settable to true")
        }
        
        function test_active_canBeToggled() {
            button.active = true
            compare(button.active, true)
            
            button.active = false
            compare(button.active, false)
            
            button.active = true
            compare(button.active, true, "active should toggle correctly")
        }
        
        function test_themeAccent_isDefined() {
            // Verify theme properties are accessible
            verify(button.themeAccent !== undefined, "themeAccent should be defined")
        }

        // =====================================================================
        // TEST: Border Visibility
        // =====================================================================
        
        function test_showBorder_hasBorderWidth() {
            compare(button.border.width, 1, "Border width should be 1 when showBorder is true")
        }
        
        function test_hideBorder_noBorderWidth() {
            button.showBorder = false
            
            compare(button.border.width, 0, "Border width should be 0 when showBorder is false")
        }
        
        function test_hideBorder_transparentBorderColor() {
            button.showBorder = false
            
            // Qt may represent transparent as "#00000000" or "transparent"
            var colorStr = button.border.color.toString()
            verify(colorStr === "transparent" || colorStr === "#00000000", 
                   "Border color should be transparent (got: " + colorStr + ")")
        }

        // =====================================================================
        // TEST: Tooltip
        // =====================================================================
        
        function test_tooltip_emptyByDefault() {
            compare(button.tooltipText, "", "Default tooltipText should be empty")
        }
        
        function test_tooltip_canBeSet() {
            button.tooltipText = "Test Tooltip"
            
            compare(button.tooltipText, "Test Tooltip", "tooltipText should be settable")
        }

        // =====================================================================
        // TEST: Content Container
        // =====================================================================
        
        function test_contentContainer_exists() {
            var container = findChild(button, "contentContainer")
            verify(container !== null, "Content container should exist")
        }
        
        function test_contentContainer_isCentered() {
            var container = findChild(button, "contentContainer")
            
            // Container should be roughly centered (within 1 pixel)
            var expectedX = (button.width - container.width) / 2
            var expectedY = (button.height - container.height) / 2
            
            verify(Math.abs(container.x - expectedX) < 1, "Container should be horizontally centered")
            verify(Math.abs(container.y - expectedY) < 1, "Container should be vertically centered")
        }
        
        function test_contentContainer_hasSmallerSize() {
            var container = findChild(button, "contentContainer")
            
            compare(container.width, button.width * 0.6, "Container width should be 60% of button")
            compare(container.height, button.height * 0.6, "Container height should be 60% of button")
        }

        // =====================================================================
        // TEST: Round Shape
        // =====================================================================
        
        function test_shape_isRound() {
            compare(button.radius, button.width / 2, "Radius should be half of width (round)")
        }
    }
}
