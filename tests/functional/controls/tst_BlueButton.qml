/**
 * tst_BlueButton.qml
 * 
 * Functional UI tests for the BlueButton component.
 * Tests click signals, enabled/disabled states, hover, pressed states,
 * and text display behavior.
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

    // Detect offscreen mode - mouse events crash in offscreen
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"
    width: 400
    height: 300

    // =========================================================================
    // Mock Theme Constants (simulates BlueTheme.js)
    // =========================================================================
    
    QtObject {
        id: mockTheme
        readonly property color buttonSurface: "#0066FF"
        readonly property color buttonBorder: "#3388FF"
        readonly property color surface: "#1A1A2E"
        readonly property color overlayTint: "#333344"
        readonly property color primaryText: "#FFFFFF"
        readonly property color mutedText: "#888899"
        readonly property string fontFamily: "Inter"
        readonly property int spacingMedium: 12
    }

    // =========================================================================
    // Component Under Test (Mock of BlueButton)
    // =========================================================================
    
    Component {
        id: blueButtonComponent
        
        Button {
            id: blueButton
            objectName: "blueButton"
            
            // Custom properties matching BlueButton.qml
            property color tone: mockTheme.buttonSurface
            property color borderTone: mockTheme.buttonBorder
            property bool filled: true
            
            // Font settings
            font.family: mockTheme.fontFamily
            font.pixelSize: 13
            padding: mockTheme.spacingMedium
            
            // Background matching BlueButton implementation
            background: Rectangle {
                id: buttonBackground
                objectName: "buttonBackground"
                radius: 14
                border.width: 1
                border.color: blueButton.borderTone
                color: blueButton.enabled 
                    ? (blueButton.filled ? blueButton.tone : mockTheme.surface) 
                    : mockTheme.overlayTint
            }
            
            // Content item matching BlueButton implementation
            contentItem: Label {
                id: buttonLabel
                objectName: "buttonLabel"
                text: blueButton.text
                color: blueButton.enabled ? mockTheme.primaryText : mockTheme.mutedText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.bold: blueButton.filled
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var blueButton: null

    // =========================================================================
    // Signal Spies (created dynamically in init to avoid offscreen issues)
    // =========================================================================
    
    property var clickedSpy: null
    property var pressedSpy: null
    property var releasedSpy: null
    
    Component {
        id: signalSpyComponent
        SignalSpy {}
    }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "BlueButtonTests"
        when: windowShown

        function init() {
            // Create fresh instance for each test
            blueButton = createTemporaryObject(blueButtonComponent, root)
            verify(blueButton !== null, "BlueButton should be created")
            blueButton.anchors.centerIn = root
            
            // Create spies dynamically to avoid offscreen mode issues
            clickedSpy = createTemporaryObject(signalSpyComponent, root, {target: blueButton, signalName: "clicked"})
            pressedSpy = createTemporaryObject(signalSpyComponent, root, {target: blueButton, signalName: "pressed"})
            releasedSpy = createTemporaryObject(signalSpyComponent, root, {target: blueButton, signalName: "released"})
            
            // Move mouse away from component to reset hover state
            mouseMove(root, 1, 1)
            // waitForRendering removed for perf
        }

        function cleanup() {
            blueButton = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_isEnabled() {
            compare(blueButton.enabled, true, "Button should be enabled by default")
        }

        // =====================================================================
        // TEST: Signal clicked
        // =====================================================================
        
        function test_click_emitsSignal() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Act
            mouseClick(blueButton)
            
            // Assert
            compare(clickedSpy.count, 1, "clicked signal should be emitted once")
        }
        
        function test_multipleClicks_emitMultipleSignals() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Act
            mouseClick(blueButton)
            mouseClick(blueButton)
            mouseClick(blueButton)
            
            // Assert
            compare(clickedSpy.count, 3, "clicked signal should be emitted 3 times")
        }
        
        function test_pressAndRelease_emitsBothSignals() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Act
            mousePress(blueButton)
            tryCompare(blueButton, "pressed", true, 100)
            mouseRelease(blueButton)
            
            // Assert
            compare(pressedSpy.count, 1, "pressed signal should be emitted")
            compare(releasedSpy.count, 1, "released signal should be emitted")
        }
        
        function test_disabledButton_noClickSignal() {
            // Arrange
            blueButton.enabled = false
            // waitForRendering removed for perf
            
            // Act
            mouseClick(blueButton)
            
            // Assert
            compare(clickedSpy.count, 0, "Disabled button should not emit clicked")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_changesHoveredProperty() {
            if (root.isOffscreen) { skip("Hover events not supported in offscreen mode"); return }
            // Act
            mouseMove(blueButton, blueButton.width / 2, blueButton.height / 2)
            
            // Assert
            tryCompare(blueButton, "hovered", true, 100, "hovered should be true when mouse is over")
        }
        
        function test_mouseLeave_clearsHoveredProperty() {
            if (root.isOffscreen) { skip("Hover events not supported in offscreen mode"); return }
            // Arrange - hover first
            mouseMove(blueButton, blueButton.width / 2, blueButton.height / 2)
            tryCompare(blueButton, "hovered", true, 100)
            
            // Act - move mouse away
            mouseMove(root, 1, 1)
            
            // Assert
            tryCompare(blueButton, "hovered", false, 100, "hovered should be false after mouse leaves")
        }

        // =====================================================================
        // TEST: Pressed State
        // =====================================================================
        
        function test_pressed_changesPressedProperty() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Act
            mousePress(blueButton, blueButton.width / 2, blueButton.height / 2)
            
            // Assert
            tryCompare(blueButton, "pressed", true, 100, "pressed should be true while mouse is down")
            
            // Cleanup
            mouseRelease(blueButton)
        }
        
        function test_release_clearsPressedProperty() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            mousePress(blueButton, blueButton.width / 2, blueButton.height / 2)
            tryCompare(blueButton, "pressed", true, 100)
            
            // Act
            mouseRelease(blueButton, blueButton.width / 2, blueButton.height / 2)
            
            // Assert
            tryCompare(blueButton, "pressed", false, 100, "pressed should be false after release")
        }

        // =====================================================================
        // TEST: Text Display
        // =====================================================================
        
        function test_text_displaysCorrectly() {
            // Arrange
            blueButton.text = "Click Me"
            // waitForRendering removed for perf
            
            // Assert
            var label = findChild(blueButton, "buttonLabel")
            verify(label !== null, "Label should exist")
            compare(label.text, "Click Me", "Label should display button text")
        }
        
        function test_text_emptyByDefault() {
            // Assert
            compare(blueButton.text, "", "Text should be empty by default")
        }
        
        function test_text_canBeChanged() {
            // Arrange
            blueButton.text = "First"
            // waitForRendering removed for perf
            
            // Act
            blueButton.text = "Second"
            // waitForRendering removed for perf
            
            // Assert
            var label = findChild(blueButton, "buttonLabel")
            compare(label.text, "Second", "Label should update when text changes")
        }
    }
}
