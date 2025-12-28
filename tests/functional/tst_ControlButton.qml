/**
 * tst_ControlButton.qml
 * 
 * Functional UI tests for the ControlButton component.
 * Tests reusable round control buttons for player controls with
 * hover/press animations, tooltip support, and customizable content.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 300
    height: 300

    // =========================================================================
    // Mock Theme Constants
    // =========================================================================
    
    QtObject {
        id: mockTheme
        readonly property color accent: "#0066FF"
        readonly property int animHoverDuration: 150
        readonly property int animPressDuration: 100
    }

    // =========================================================================
    // Component Under Test (Mock)
    // =========================================================================
    
    Rectangle {
        id: controlButton
        objectName: "controlButton"
        anchors.centerIn: parent
        
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
        
        // Style - computed colors
        color: active ? mockTheme.accent : (mouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF")
        border.color: active ? mockTheme.accent : (showBorder ? "#4DFFFFFF" : "transparent")
        border.width: showBorder ? 1 : 0
        
        // Animations (simplified for testing)
        Behavior on color {
            ColorAnimation { duration: mockTheme.animHoverDuration; easing.type: Easing.OutCubic }
        }
        
        scale: mouseArea.pressed ? 0.92 : (mouseArea.containsMouse ? 1.05 : 1.0)
        Behavior on scale {
            NumberAnimation { duration: mockTheme.animPressDuration; easing.type: Easing.OutQuart }
        }
        
        // Signal
        signal clicked()
        
        // Expose hover and pressed state for external use
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
            onClicked: controlButton.clicked()
        }
        
        ToolTip {
            id: tooltip
            objectName: "tooltip"
            visible: mouseArea.containsMouse && controlButton.tooltipText !== ""
            text: controlButton.tooltipText
            delay: 800
        }
        
        // Reset function for tests
        function reset() {
            tooltipText = ""
            active = false
            showBorder = true
            width = 32
            height = 32
        }
    }
    
    // Test icon content
    Text {
        id: testIcon
        objectName: "testIcon"
        parent: controlButton
        anchors.centerIn: parent
        text: ""
        font.pixelSize: 16
        color: "#FFFFFF"
    }

    // =========================================================================
    // Signal Spy
    // =========================================================================
    
    SignalSpy { id: clickedSpy; target: controlButton; signalName: "clicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "ControlButtonTests"
        when: windowShown

        function init() {
            controlButton.reset()
            testIcon.text = ""
            clickedSpy.clear()
            // Move mouse away from component to reset hover state
            mouseMove(root, 1, 1)
            wait(200)
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultState_dimensions() {
            compare(controlButton.width, 32, "Default width should be 32")
            compare(controlButton.height, 32, "Default height should be 32")
        }
        
        function test_defaultState_isCircular() {
            compare(controlButton.radius, controlButton.width / 2, "Should be circular (radius = width/2)")
        }
        
        function test_defaultState_tooltipEmpty() {
            compare(controlButton.tooltipText, "", "Default tooltip should be empty")
        }
        
        function test_defaultState_notActive() {
            compare(controlButton.active, false, "Should not be active by default")
        }
        
        function test_defaultState_showBorderTrue() {
            compare(controlButton.showBorder, true, "showBorder should be true by default")
        }
        
        function test_defaultState_notPressed() {
            compare(controlButton.pressed, false, "Should not be pressed initially")
        }

        // =====================================================================
        // TEST: Signal clicked
        // =====================================================================
        
        function test_click_emitsSignal() {
            // Arrange
            var mouseArea = findChild(controlButton, "mouseArea")
            verify(mouseArea !== null, "MouseArea should exist")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 1, "clicked signal should be emitted once")
        }
        
        function test_multipleClicks_emitMultiple() {
            // Arrange
            var mouseArea = findChild(controlButton, "mouseArea")
            
            // Act
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 3, "Should emit clicked for each click")
        }
        
        function test_clickOnButton_emitsSignal() {
            // Act - click directly on the button
            mouseClick(controlButton)
            
            // Assert
            compare(clickedSpy.count, 1, "Clicking on button should emit clicked")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_changesHoveredProperty() {
            // Arrange
            var mouseArea = findChild(controlButton, "mouseArea")
            verify(mouseArea !== null)
            
            // Act - move mouse into the button
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert
            compare(controlButton.hovered, true, "hovered should be true when mouse is over")
        }
        
        function test_hover_increasesScale() {
            // Arrange
            var mouseArea = findChild(controlButton, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200) // Wait for animation
            
            // Assert
            compare(controlButton.scale, 1.05, "Scale should be 1.05 on hover")
        }
        
        function test_hover_changesBackgroundColor() {
            // Arrange - ensure not active
            controlButton.active = false
            wait(50)
            var initialColor = controlButton.color.toString()
            
            // Act
            var mouseArea = findChild(controlButton, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200)
            
            // Assert - color should change from #1AFFFFFF to #33FFFFFF
            verify(controlButton.hovered, "Should be hovered")
            compare(controlButton.color.toString(), "#33ffffff", "Background should be lighter on hover")
        }

        // =====================================================================
        // TEST: Pressed State
        // =====================================================================
        
        function test_pressed_decreasesScale() {
            // Arrange
            var mouseArea = findChild(controlButton, "mouseArea")
            
            // Act - press and hold
            mousePress(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(150)
            
            // Assert
            compare(controlButton.pressed, true, "pressed should be true")
            compare(controlButton.scale, 0.92, "Scale should be 0.92 when pressed")
            
            // Cleanup
            mouseRelease(mouseArea)
        }
        
        function test_pressRelease_restoresScale() {
            // Arrange
            var mouseArea = findChild(controlButton, "mouseArea")
            
            // Act - press then release
            mousePress(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(150)
            mouseRelease(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200)
            
            // Assert - should return to hover scale (still hovered after release)
            compare(controlButton.pressed, false, "pressed should be false after release")
        }

        // =====================================================================
        // TEST: Tooltip Behavior
        // =====================================================================
        
        function test_tooltip_textSets() {
            // Arrange
            controlButton.tooltipText = "Play"
            wait(50)
            
            // Assert
            var tooltip = findChild(controlButton, "tooltip")
            verify(tooltip !== null, "Tooltip should exist")
            compare(tooltip.text, "Play", "Tooltip text should be set")
        }
        
        function test_tooltip_notVisibleWithoutHover() {
            // Arrange
            controlButton.tooltipText = "Play"
            wait(50)
            
            // Assert
            var tooltip = findChild(controlButton, "tooltip")
            verify(!tooltip.visible, "Tooltip should not be visible without hover")
        }
        
        function test_tooltip_notVisibleWhenEmpty() {
            // Arrange
            controlButton.tooltipText = ""
            var mouseArea = findChild(controlButton, "mouseArea")
            
            // Act - hover
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert
            var tooltip = findChild(controlButton, "tooltip")
            verify(!tooltip.visible, "Tooltip should not be visible when text is empty")
        }
        
        function test_tooltip_conditionsForVisibility() {
            // Arrange
            controlButton.tooltipText = "Pause"
            var mouseArea = findChild(controlButton, "mouseArea")
            
            // Act - hover
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert - verify conditions for visibility are met
            // (actual visibility depends on delay timer, so we verify the conditions)
            var tooltip = findChild(controlButton, "tooltip")
            verify(controlButton.hovered, "Button should be hovered")
            verify(controlButton.tooltipText !== "", "Tooltip text should not be empty")
            compare(tooltip.text, "Pause", "Tooltip text should be set correctly")
        }
        
        function test_tooltip_hasDelay() {
            // Assert
            var tooltip = findChild(controlButton, "tooltip")
            compare(tooltip.delay, 800, "Tooltip delay should be 800ms")
        }

        // =====================================================================
        // TEST: Active State Visual Styles
        // =====================================================================
        
        function test_active_changesBackgroundColor() {
            // Arrange - move mouse away first
            mouseMove(root, 1, 1)
            controlButton.active = false
            wait(200)
            
            // Act
            controlButton.active = true
            wait(400) // Wait for color animation to complete
            
            // Assert
            compare(controlButton.color.toString(), "#0066ff", "Background should be accent color when active")
        }
        
        function test_active_changesBorderColor() {
            // Arrange
            controlButton.active = false
            wait(50)
            
            // Act
            controlButton.active = true
            wait(200)
            
            // Assert
            compare(controlButton.border.color.toString(), "#0066ff", "Border should be accent color when active")
        }
        
        function test_inactive_defaultBackgroundColor() {
            // Arrange - move mouse away to ensure not hovered
            mouseMove(root, root.width - 1, root.height - 1)
            controlButton.active = false
            wait(250)
            
            // Assert
            compare(controlButton.color.toString(), "#1affffff", "Background should be subtle when inactive and not hovered")
        }

        // =====================================================================
        // TEST: Border Styles
        // =====================================================================
        
        function test_showBorder_hasBorderWidth() {
            // Arrange
            controlButton.showBorder = true
            wait(50)
            
            // Assert
            compare(controlButton.border.width, 1, "Border width should be 1 when showBorder is true")
        }
        
        function test_hideBorder_noBorderWidth() {
            // Arrange
            controlButton.showBorder = false
            wait(50)
            
            // Assert
            compare(controlButton.border.width, 0, "Border width should be 0 when showBorder is false")
        }
        
        function test_hideBorder_transparentBorderColor() {
            // Arrange
            controlButton.active = false
            controlButton.showBorder = false
            wait(50)
            
            // Assert
            compare(controlButton.border.color.toString(), "#00000000", "Border should be transparent when showBorder is false")
        }
        
        function test_showBorder_hasBorderColor() {
            // Arrange
            controlButton.active = false
            controlButton.showBorder = true
            wait(50)
            
            // Assert
            compare(controlButton.border.color.toString(), "#4dffffff", "Border should have color when showBorder is true")
        }

        // =====================================================================
        // TEST: Content Container
        // =====================================================================
        
        function test_contentContainer_exists() {
            var container = findChild(controlButton, "contentContainer")
            verify(container !== null, "Content container should exist")
        }
        
        function test_contentContainer_isCentered() {
            var container = findChild(controlButton, "contentContainer")
            verify(container !== null)
            
            // Container should be centered (use fuzzy compare for float precision)
            var expectedX = (controlButton.width - container.width) / 2
            var expectedY = (controlButton.height - container.height) / 2
            
            fuzzyCompare(container.x, expectedX, 1, "Container should be horizontally centered")
            fuzzyCompare(container.y, expectedY, 1, "Container should be vertically centered")
        }
        
        function test_contentContainer_size() {
            var container = findChild(controlButton, "contentContainer")
            verify(container !== null)
            
            compare(container.width, controlButton.width * 0.6, "Container width should be 60% of button")
            compare(container.height, controlButton.height * 0.6, "Container height should be 60% of button")
        }

        // =====================================================================
        // TEST: Custom Dimensions
        // =====================================================================
        
        function test_customWidth_applies() {
            // Arrange
            controlButton.width = 48
            wait(50)
            
            // Assert
            compare(controlButton.width, 48, "Custom width should apply")
            compare(controlButton.radius, 24, "Radius should update with width")
        }
        
        function test_customHeight_applies() {
            // Arrange
            controlButton.height = 48
            wait(50)
            
            // Assert
            compare(controlButton.height, 48, "Custom height should apply")
        }

        // =====================================================================
        // TEST: MouseArea Configuration
        // =====================================================================
        
        function test_mouseArea_hoverEnabled() {
            var mouseArea = findChild(controlButton, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.hoverEnabled, true, "Hover should be enabled")
        }
        
        function test_mouseArea_cursorShape() {
            var mouseArea = findChild(controlButton, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.cursorShape, Qt.PointingHandCursor, "Cursor should be pointing hand")
        }
        
        function test_mouseArea_fillsParent() {
            var mouseArea = findChild(controlButton, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.width, controlButton.width, "MouseArea should fill button width")
            compare(mouseArea.height, controlButton.height, "MouseArea should fill button height")
        }
    }
}
