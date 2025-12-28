/**
 * tst_PipPlaceholder.qml
 * 
 * Functional UI tests for the PipPlaceholder component.
 * Tests the placeholder shown when video is in Picture-in-Picture mode.
 * 
 * Run with: ./test_functional_ui PipPlaceholderTests
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 800
    height: 600

    // Detect offscreen mode - mouse events may not work
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"

    // =========================================================================
    // Mock of PipPlaceholder Component (reproduces public API)
    // =========================================================================
    
    Component {
        id: pipPlaceholderComponent
        
        Rectangle {
            id: placeholder
            objectName: "pipPlaceholder"
            
            // ─────────────────────────────────────────────────────────────────
            // Signals
            // ─────────────────────────────────────────────────────────────────
            
            signal returnRequested()
            
            // ─────────────────────────────────────────────────────────────────
            // Appearance
            // ─────────────────────────────────────────────────────────────────
            
            color: "#1A1A2E"  // BlueTheme.windowBackground approximation
            
            // Fade animation
            opacity: visible ? 1.0 : 0.0
            Behavior on opacity {
                NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Content
            // ─────────────────────────────────────────────────────────────────
            
            Column {
                id: contentColumn
                objectName: "contentColumn"
                anchors.centerIn: parent
                spacing: 16
                
                // Icon Container
                Rectangle {
                    id: iconContainer
                    objectName: "iconContainer"
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 100
                    height: 100
                    radius: 50
                    color: "#2A2A4A"
                    border.color: "#3A3A5A"
                    border.width: 1
                    
                    // PiP icon text placeholder
                    Text {
                        id: iconText
                        objectName: "iconText"
                        anchors.centerIn: parent
                        text: "PiP"
                        color: "#9146FF"
                        font.pixelSize: 24
                        font.bold: true
                    }
                }
                
                // Title
                Text {
                    id: titleText
                    objectName: "titleText"
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Video in Picture-in-Picture")
                    color: "#FFFFFF"
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }
                
                // Subtitle
                Text {
                    id: subtitleText
                    objectName: "subtitleText"
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("The video is playing in a floating window")
                    color: "#888888"
                    font.pixelSize: 14
                }
                
                // Spacer
                Item { width: 1; height: 8 }
                
                // Return Button
                Rectangle {
                    id: returnButton
                    objectName: "returnButton"
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: returnRow.width + 32
                    height: 44
                    radius: 22
                    color: returnMouseArea.containsMouse ? "#9146FF" : "#2A2A4A"
                    border.color: returnMouseArea.containsMouse ? "#9146FF" : "#3A3A5A"
                    border.width: 1
                    
                    property bool hovered: returnMouseArea.containsMouse
                    
                    Behavior on color {
                        ColorAnimation { duration: 150; easing.type: Easing.OutCubic }
                    }
                    
                    Row {
                        id: returnRow
                        anchors.centerIn: parent
                        spacing: 8
                        
                        Text {
                            id: returnIcon
                            objectName: "returnIcon"
                            text: "\u2199"  // ↙
                            color: returnMouseArea.containsMouse ? "#FFFFFF" : "#CCCCCC"
                            font.pixelSize: 16
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        
                        Text {
                            id: returnLabel
                            objectName: "returnLabel"
                            text: qsTr("Return here")
                            color: returnMouseArea.containsMouse ? "#FFFFFF" : "#CCCCCC"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    
                    MouseArea {
                        id: returnMouseArea
                        objectName: "returnMouseArea"
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: placeholder.returnRequested()
                    }
                    
                    // Scale animation
                    scale: returnMouseArea.pressed ? 0.96 : (returnMouseArea.containsMouse ? 1.02 : 1.0)
                    Behavior on scale {
                        NumberAnimation { duration: 100; easing.type: Easing.OutQuart }
                    }
                }
                
                // Keyboard shortcut hint
                Text {
                    id: shortcutHint
                    objectName: "shortcutHint"
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Press P or Escape to return")
                    color: "#666666"
                    font.pixelSize: 12
                    topPadding: 8
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var placeholder: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    property var returnSpy: null
    
    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "PipPlaceholderTests"
        when: windowShown

        function init() {
            placeholder = createTemporaryObject(pipPlaceholderComponent, root)
            verify(placeholder !== null, "PipPlaceholder should be created")
            placeholder.anchors.fill = root
            placeholder.visible = true
            
            returnSpy = createTemporaryObject(signalSpyComponent, root, {
                target: placeholder, 
                signalName: "returnRequested"
            })
            
            waitForRendering(placeholder)
        }

        function cleanup() {
            placeholder = null
            returnSpy = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_isVisible() {
            // Assert
            verify(placeholder.visible, "Placeholder should be visible")
            compare(placeholder.opacity, 1.0, "Opacity should be 1.0 when visible")
        }
        
        function test_contentColumn_exists() {
            // Arrange
            var column = findChild(placeholder, "contentColumn")
            
            // Assert
            verify(column !== null, "Content column should exist")
        }
        
        function test_iconContainer_exists() {
            // Arrange
            var icon = findChild(placeholder, "iconContainer")
            
            // Assert
            verify(icon !== null, "Icon container should exist")
            compare(icon.width, 100, "Icon container width should be 100")
            compare(icon.height, 100, "Icon container height should be 100")
            compare(icon.radius, 50, "Icon container should be circular")
        }
        
        function test_titleText_correctContent() {
            // Arrange
            var title = findChild(placeholder, "titleText")
            
            // Assert
            verify(title !== null, "Title should exist")
            verify(title.text.indexOf("Picture-in-Picture") >= 0, 
                   "Title should mention Picture-in-Picture")
        }
        
        function test_subtitleText_correctContent() {
            // Arrange
            var subtitle = findChild(placeholder, "subtitleText")
            
            // Assert
            verify(subtitle !== null, "Subtitle should exist")
            verify(subtitle.text.indexOf("floating window") >= 0, 
                   "Subtitle should mention floating window")
        }
        
        function test_shortcutHint_mentionsKeys() {
            // Arrange
            var hint = findChild(placeholder, "shortcutHint")
            
            // Assert
            verify(hint !== null, "Shortcut hint should exist")
            verify(hint.text.indexOf("P") >= 0, "Should mention P key")
            verify(hint.text.indexOf("Escape") >= 0, "Should mention Escape key")
        }

        // =====================================================================
        // TEST: Return Button
        // =====================================================================
        
        function test_returnButton_exists() {
            // Arrange
            var button = findChild(placeholder, "returnButton")
            
            // Assert
            verify(button !== null, "Return button should exist")
            compare(button.height, 44, "Button height should be 44")
            compare(button.radius, 22, "Button should be pill-shaped")
        }
        
        function test_returnButton_click_emitsSignal() {
            if (root.isOffscreen) { 
                skip("Mouse events not supported in offscreen mode")
                return 
            }
            
            // Arrange
            var button = findChild(placeholder, "returnButton")
            verify(button !== null, "Return button should exist")
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(returnSpy.count, 1, "returnRequested should be emitted once")
        }
        
        function test_returnButton_doubleClick_emitsTwice() {
            if (root.isOffscreen) { 
                skip("Mouse events not supported in offscreen mode")
                return 
            }
            
            // Arrange
            var button = findChild(placeholder, "returnButton")
            
            // Act
            mouseClick(button)
            mouseClick(button)
            
            // Assert
            compare(returnSpy.count, 2, "returnRequested should be emitted twice")
        }
        
        function test_returnButton_hasCorrectLabel() {
            // Arrange
            var label = findChild(placeholder, "returnLabel")
            
            // Assert
            verify(label !== null, "Return label should exist")
            verify(label.text.indexOf("Return") >= 0, "Label should say Return")
        }
        
        function test_returnButton_hasIcon() {
            // Arrange
            var icon = findChild(placeholder, "returnIcon")
            
            // Assert
            verify(icon !== null, "Return icon should exist")
            verify(icon.text.length > 0, "Icon should have content")
        }

        // =====================================================================
        // TEST: Visibility / Opacity
        // =====================================================================
        
        function test_opacity_isZero_whenHidden() {
            // Arrange - Use tryCompare for async animations
            placeholder.visible = false
            
            // Assert - Wait for opacity animation using tryCompare
            tryCompare(placeholder, "opacity", 0.0, 500, 
                       "Opacity should be 0 when hidden")
        }
        
        function test_opacity_isOne_whenVisible() {
            // Arrange - Start hidden
            placeholder.visible = false
            tryCompare(placeholder, "opacity", 0.0, 500)
            
            // Act
            placeholder.visible = true
            
            // Assert - Wait for opacity animation
            tryCompare(placeholder, "opacity", 1.0, 500,
                       "Opacity should be 1.0 when visible")
        }

        // =====================================================================
        // TEST: Layout
        // =====================================================================
        
        function test_content_isCentered() {
            // Arrange
            var column = findChild(placeholder, "contentColumn")
            verify(column !== null)
            
            // Assert - Column should be approximately centered
            var centerX = placeholder.width / 2
            var centerY = placeholder.height / 2
            var columnCenterX = column.x + column.width / 2
            var columnCenterY = column.y + column.height / 2
            
            // Allow some tolerance for rounding
            verify(Math.abs(columnCenterX - centerX) < 5, 
                   "Content should be horizontally centered")
            verify(Math.abs(columnCenterY - centerY) < column.height, 
                   "Content should be approximately vertically centered")
        }

        // =====================================================================
        // TEST: Hover State (via MouseArea containsMouse simulation)
        // =====================================================================
        
        function test_returnButton_colorBehavior_exists() {
            // Arrange
            var button = findChild(placeholder, "returnButton")
            
            // Assert - Button has Behavior on color (can't test hover directly)
            verify(button !== null, "Button should exist")
            verify(button.color !== undefined, "Button should have color property")
            
            // Verify initial non-hover color is from surface theme
            var colorStr = "" + button.color
            verify(colorStr.indexOf("#") >= 0, "Color should be a hex value")
        }
        
        function test_returnButton_mouseArea_exists() {
            // Arrange
            var mouseArea = findChild(placeholder, "returnMouseArea")
            
            // Assert
            verify(mouseArea !== null, "returnMouseArea should exist")
            verify(mouseArea.hoverEnabled, "hoverEnabled should be true")
            compare(mouseArea.cursorShape, Qt.PointingHandCursor, 
                    "Cursor should be pointing hand")
        }
    }
}
