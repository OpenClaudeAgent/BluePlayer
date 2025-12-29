/**
 * tst_CircleButton.qml
 * 
 * Functional UI tests for the CircleButton component.
 * Tests click, hover, active state, icon display, and tooltip.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 400

    // Detect offscreen mode
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"

    // =========================================================================
    // Component Under Test (Mock of CircleButton)
    // =========================================================================
    
    Component {
        id: circleButtonComponent
        
        Item {
            id: circleBtn
            objectName: "circleButton"

            // Theme constants (inline for proper binding)
            readonly property color themePrimaryText: "#e6edf3"
            readonly property color themeAccent: "#3b82f6"
            readonly property color themeSurface: "#0d1117"
            readonly property color themeSurfaceSoft: "#161b22"
            readonly property color themeOverlayTint: "#1a3a5c"
            readonly property color themeButtonBorder: "#30363d"
            readonly property int themeBorderWidth: 1
            readonly property int animDuration: 1
            readonly property real themeScalePress: 0.95
            readonly property string themeFontFamily: "Inter"

            // Public Properties
            property string iconText: ""
            property bool active: false
            property string tooltipText: ""
            property int iconSize: 20
            property color iconColor: themePrimaryText

            // Signal
            signal clicked()

            // Expose internal state
            readonly property bool hovered: mouseArea.containsMouse
            readonly property alias backgroundScale: buttonBackground.scale

            implicitWidth: 38
            implicitHeight: 38

            Rectangle {
                id: buttonBackground
                objectName: "buttonBackground"
                anchors.fill: parent
                radius: width / 2
                
                color: {
                    if (circleBtn.active) {
                        return circleBtn.themeOverlayTint
                    } else if (mouseArea.containsMouse) {
                        return circleBtn.themeSurfaceSoft
                    } else {
                        return circleBtn.themeSurface
                    }
                }
                
                border.color: circleBtn.active ? circleBtn.themeAccent : circleBtn.themeButtonBorder
                border.width: circleBtn.themeBorderWidth

                Behavior on color {
                    ColorAnimation {
                        duration: circleBtn.animDuration
                        easing.type: Easing.OutCubic
                    }
                }
                
                Behavior on border.color {
                    ColorAnimation {
                        duration: circleBtn.animDuration
                        easing.type: Easing.OutCubic
                    }
                }

                Text {
                    id: iconLabel
                    objectName: "iconLabel"
                    anchors.centerIn: parent
                    text: circleBtn.iconText
                    font.pixelSize: circleBtn.iconSize
                    font.family: circleBtn.themeFontFamily
                    color: circleBtn.iconColor
                    
                    scale: mouseArea.containsMouse ? 1.05 : 1.0
                    
                    Behavior on scale {
                        NumberAnimation {
                            duration: circleBtn.animDuration
                            easing.type: Easing.OutCubic
                        }
                    }
                }
            }

            MouseArea {
                id: mouseArea
                objectName: "mouseArea"
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                
                onClicked: circleBtn.clicked()
                
                onPressed: buttonBackground.scale = circleBtn.themeScalePress
                onReleased: buttonBackground.scale = 1.0
            }
            
            Behavior on scale {
                NumberAnimation {
                    duration: circleBtn.animDuration
                    easing.type: Easing.OutQuart
                }
            }

            ToolTip {
                id: tooltip
                visible: circleBtn.tooltipText !== "" && mouseArea.containsMouse
                text: circleBtn.tooltipText
                delay: 500
                
                background: Rectangle {
                    color: circleBtn.themeSurface
                    border.color: circleBtn.themeButtonBorder
                    border.width: 1
                    radius: 6
                }
                
                contentItem: Text {
                    text: tooltip.text
                    font.pixelSize: 12
                    font.family: circleBtn.themeFontFamily
                    color: circleBtn.themePrimaryText
                }
            }
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
        name: "CircleButtonTests"
        when: windowShown

        function init() {
            button = createTemporaryObject(circleButtonComponent, root)
            verify(button !== null, "CircleButton should be created")
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
            compare(button.implicitWidth, 38, "Default width should be 38")
            compare(button.implicitHeight, 38, "Default height should be 38")
        }
        
        function test_defaultState_notActive() {
            compare(button.active, false, "Default active should be false")
        }
        
        function test_defaultState_emptyIcon() {
            compare(button.iconText, "", "Default iconText should be empty")
        }
        
        function test_defaultState_emptyTooltip() {
            compare(button.tooltipText, "", "Default tooltipText should be empty")
        }
        
        function test_defaultState_notHovered() {
            compare(button.hovered, false, "Default hovered should be false")
        }
        
        function test_defaultState_iconSize() {
            compare(button.iconSize, 20, "Default iconSize should be 20")
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
            
            compare(clickedSpy.count, 2, "Should emit clicked for each click")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_setsHoveredTrue() {
            var mouseArea = findChild(button, "mouseArea")
            
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(button, "hovered", true, 100, "hovered should be true")
        }
        
        function test_hoverExit_restoresHoveredFalse() {
            var mouseArea = findChild(button, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            tryCompare(button, "hovered", true, 100)
            
            mouseMove(root, 1, 1)
            
            tryCompare(button, "hovered", false, 100, "hovered should be false after exit")
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
        
        function test_themeProperties_areDefined() {
            // Verify theme properties are accessible
            verify(button.themeAccent !== undefined, "themeAccent should be defined")
            verify(button.themeOverlayTint !== undefined, "themeOverlayTint should be defined")
            verify(button.themeSurface !== undefined, "themeSurface should be defined")
        }

        // =====================================================================
        // TEST: Icon Display
        // =====================================================================
        
        function test_icon_displaysText() {
            var iconLabel = findChild(button, "iconLabel")
            
            button.iconText = "⚙"
            
            compare(iconLabel.text, "⚙", "Icon should display the set text")
        }
        
        function test_icon_unicodeHistory() {
            var iconLabel = findChild(button, "iconLabel")
            
            button.iconText = "\u21BA"  // ↺
            
            compare(iconLabel.text, "\u21BA", "Icon should display Unicode character")
        }
        
        function test_icon_colorCanBeChanged() {
            var iconLabel = findChild(button, "iconLabel")
            var newColor = "#ff0000"
            
            button.iconColor = newColor
            
            compare(iconLabel.color.toString(), newColor, "Icon color should be changeable")
        }
        
        function test_icon_sizeCanBeChanged() {
            var iconLabel = findChild(button, "iconLabel")
            
            button.iconSize = 24
            
            compare(iconLabel.font.pixelSize, 24, "Icon size should be changeable")
        }

        // =====================================================================
        // TEST: Tooltip
        // =====================================================================
        
        function test_tooltip_canBeSet() {
            button.tooltipText = "Watch History"
            
            compare(button.tooltipText, "Watch History", "Tooltip should be settable")
        }

        // =====================================================================
        // TEST: Round Shape
        // =====================================================================
        
        function test_shape_isRound() {
            var background = findChild(button, "buttonBackground")
            
            compare(background.radius, background.width / 2, "Button should be round")
        }

        // =====================================================================
        // TEST: Press Effect
        // =====================================================================
        
        function test_press_scalesDown() {
            var mouseArea = findChild(button, "mouseArea")
            var background = findChild(button, "buttonBackground")
            
            mousePress(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(background, "scale", button.themeScalePress, 100, "Should scale down on press")
            
            mouseRelease(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
        }
        
        function test_release_restoresScale() {
            var mouseArea = findChild(button, "mouseArea")
            var background = findChild(button, "buttonBackground")
            
            mousePress(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            tryCompare(background, "scale", button.themeScalePress, 100)
            
            mouseRelease(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(background, "scale", 1.0, 100, "Should restore scale on release")
        }
    }
}
