/**
 * tst_BlueButton.qml
 * 
 * Functional UI tests for the BlueButton component.
 * Tests styled button with customizable tone, border, and filled mode.
 * Verifies text display, enabled/disabled states, and click behavior.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
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
    
    Button {
        id: blueButton
        objectName: "blueButton"
        anchors.centerIn: parent
        
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
        
        // Reset function for tests
        function reset() {
            text = ""
            tone = mockTheme.buttonSurface
            borderTone = mockTheme.buttonBorder
            filled = true
            enabled = true
        }
    }

    // =========================================================================
    // Signal Spy
    // =========================================================================
    
    SignalSpy { id: clickedSpy; target: blueButton; signalName: "clicked" }
    SignalSpy { id: pressedSpy; target: blueButton; signalName: "pressed" }
    SignalSpy { id: releasedSpy; target: blueButton; signalName: "released" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "BlueButtonTests"
        when: windowShown

        function init() {
            blueButton.reset()
            clickedSpy.clear()
            pressedSpy.clear()
            releasedSpy.clear()
            // Move mouse away from component to reset hover state
            mouseMove(root, 1, 1)
            wait(100)
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultState_toneIsButtonSurface() {
            compare(blueButton.tone.toString(), mockTheme.buttonSurface.toString(), 
                "Default tone should be buttonSurface color")
        }
        
        function test_defaultState_borderToneIsButtonBorder() {
            compare(blueButton.borderTone.toString(), mockTheme.buttonBorder.toString(), 
                "Default borderTone should be buttonBorder color")
        }
        
        function test_defaultState_filledIsTrue() {
            compare(blueButton.filled, true, "Default filled should be true")
        }
        
        function test_defaultState_isEnabled() {
            compare(blueButton.enabled, true, "Button should be enabled by default")
        }
        
        function test_defaultState_fontFamily() {
            compare(blueButton.font.family, mockTheme.fontFamily, 
                "Font family should match theme")
        }
        
        function test_defaultState_fontSize() {
            compare(blueButton.font.pixelSize, 13, "Font size should be 13px")
        }
        
        function test_defaultState_padding() {
            compare(blueButton.padding, mockTheme.spacingMedium, 
                "Padding should match theme spacingMedium")
        }

        // =====================================================================
        // TEST: Signal clicked
        // =====================================================================
        
        function test_click_emitsSignal() {
            // Act
            mouseClick(blueButton)
            
            // Assert
            compare(clickedSpy.count, 1, "clicked signal should be emitted once")
        }
        
        function test_multipleClicks_emitMultipleSignals() {
            // Act
            mouseClick(blueButton)
            mouseClick(blueButton)
            mouseClick(blueButton)
            
            // Assert
            compare(clickedSpy.count, 3, "clicked signal should be emitted 3 times")
        }
        
        function test_pressAndRelease_emitsBothSignals() {
            // Act
            mousePress(blueButton)
            wait(50)
            mouseRelease(blueButton)
            
            // Assert
            compare(pressedSpy.count, 1, "pressed signal should be emitted")
            compare(releasedSpy.count, 1, "released signal should be emitted")
        }
        
        function test_disabledButton_noClickSignal() {
            // Arrange
            blueButton.enabled = false
            wait(50)
            
            // Act
            mouseClick(blueButton)
            
            // Assert
            compare(clickedSpy.count, 0, "Disabled button should not emit clicked")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_changesHoveredProperty() {
            // Act
            mouseMove(blueButton, blueButton.width / 2, blueButton.height / 2)
            wait(50)
            
            // Assert
            compare(blueButton.hovered, true, "hovered should be true when mouse is over")
        }
        
        function test_mouseLeave_clearsHoveredProperty() {
            // Arrange - hover first
            mouseMove(blueButton, blueButton.width / 2, blueButton.height / 2)
            wait(50)
            verify(blueButton.hovered, "Should be hovered first")
            
            // Act - move mouse away
            mouseMove(root, 1, 1)
            wait(100)
            
            // Assert
            compare(blueButton.hovered, false, "hovered should be false after mouse leaves")
        }

        // =====================================================================
        // TEST: Pressed State
        // =====================================================================
        
        function test_pressed_changesPressedProperty() {
            // Act
            mousePress(blueButton, blueButton.width / 2, blueButton.height / 2)
            wait(50)
            
            // Assert
            compare(blueButton.pressed, true, "pressed should be true while mouse is down")
            
            // Cleanup
            mouseRelease(blueButton)
        }
        
        function test_release_clearsPressedProperty() {
            // Arrange
            mousePress(blueButton, blueButton.width / 2, blueButton.height / 2)
            wait(50)
            
            // Act
            mouseRelease(blueButton, blueButton.width / 2, blueButton.height / 2)
            wait(50)
            
            // Assert
            compare(blueButton.pressed, false, "pressed should be false after release")
        }

        // =====================================================================
        // TEST: Text Display
        // =====================================================================
        
        function test_text_displaysCorrectly() {
            // Arrange
            blueButton.text = "Click Me"
            wait(50)
            
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
            wait(50)
            
            // Act
            blueButton.text = "Second"
            wait(50)
            
            // Assert
            var label = findChild(blueButton, "buttonLabel")
            compare(label.text, "Second", "Label should update when text changes")
        }
        
        function test_textAlignment_isCentered() {
            // Arrange
            blueButton.text = "Centered"
            wait(50)
            
            // Assert
            var label = findChild(blueButton, "buttonLabel")
            compare(label.horizontalAlignment, Text.AlignHCenter, 
                "Text should be horizontally centered")
            compare(label.verticalAlignment, Text.AlignVCenter, 
                "Text should be vertically centered")
        }

        // =====================================================================
        // TEST: Background Styles - Filled Mode
        // =====================================================================
        
        function test_filled_backgroundUsesTone() {
            // Arrange
            blueButton.filled = true
            blueButton.tone = "#FF0000"
            wait(50)
            
            // Assert
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.color.toString(), "#ff0000", 
                "Filled button background should use tone color")
        }
        
        function test_notFilled_backgroundUsesSurface() {
            // Arrange
            blueButton.filled = false
            wait(50)
            
            // Assert
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.color.toString(), mockTheme.surface.toString().toLowerCase(), 
                "Non-filled button background should use surface color")
        }
        
        function test_background_hasRadius14() {
            // Assert
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.radius, 14, "Background radius should be 14")
        }
        
        function test_background_hasBorderWidth1() {
            // Assert
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.border.width, 1, "Border width should be 1")
        }
        
        function test_background_usesBorderTone() {
            // Arrange
            blueButton.borderTone = "#00FF00"
            wait(50)
            
            // Assert
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.border.color.toString(), "#00ff00", 
                "Border should use borderTone color")
        }

        // =====================================================================
        // TEST: Disabled State Styles
        // =====================================================================
        
        function test_disabled_backgroundUsesOverlayTint() {
            // Arrange
            blueButton.enabled = false
            wait(50)
            
            // Assert
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.color.toString(), mockTheme.overlayTint.toString().toLowerCase(), 
                "Disabled button should use overlayTint color")
        }
        
        function test_disabled_textUsesMutedColor() {
            // Arrange
            blueButton.text = "Disabled"
            blueButton.enabled = false
            wait(50)
            
            // Assert
            var label = findChild(blueButton, "buttonLabel")
            compare(label.color.toString(), mockTheme.mutedText.toString().toLowerCase(), 
                "Disabled button text should use muted color")
        }
        
        function test_enabled_textUsesPrimaryColor() {
            // Arrange
            blueButton.text = "Enabled"
            blueButton.enabled = true
            wait(50)
            
            // Assert
            var label = findChild(blueButton, "buttonLabel")
            compare(label.color.toString(), mockTheme.primaryText.toString().toLowerCase(), 
                "Enabled button text should use primary color")
        }

        // =====================================================================
        // TEST: Font Bold Based on Filled
        // =====================================================================
        
        function test_filled_textIsBold() {
            // Arrange
            blueButton.text = "Bold"
            blueButton.filled = true
            wait(50)
            
            // Assert
            var label = findChild(blueButton, "buttonLabel")
            compare(label.font.bold, true, "Filled button text should be bold")
        }
        
        function test_notFilled_textIsNotBold() {
            // Arrange
            blueButton.text = "Normal"
            blueButton.filled = false
            wait(50)
            
            // Assert
            var label = findChild(blueButton, "buttonLabel")
            compare(label.font.bold, false, "Non-filled button text should not be bold")
        }

        // =====================================================================
        // TEST: Custom Tone Colors
        // =====================================================================
        
        function test_customTone_appliesCorrectly() {
            // Arrange
            blueButton.filled = true
            blueButton.tone = "#9B59B6"
            wait(50)
            
            // Assert
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.color.toString(), "#9b59b6", 
                "Custom tone should be applied to background")
        }
        
        function test_customBorderTone_appliesCorrectly() {
            // Arrange
            blueButton.borderTone = "#E74C3C"
            wait(50)
            
            // Assert
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.border.color.toString(), "#e74c3c", 
                "Custom borderTone should be applied to border")
        }

        // =====================================================================
        // TEST: Toggle Filled Property
        // =====================================================================
        
        function test_toggleFilled_changesBackground() {
            // Arrange - start filled
            blueButton.filled = true
            wait(50)
            var bg = findChild(blueButton, "buttonBackground")
            var filledColor = bg.color.toString()
            
            // Act - toggle to not filled
            blueButton.filled = false
            wait(50)
            var notFilledColor = bg.color.toString()
            
            // Assert
            verify(filledColor !== notFilledColor, 
                "Background color should change when filled is toggled")
            compare(notFilledColor, mockTheme.surface.toString().toLowerCase(),
                "Not filled should use surface color")
        }
        
        function test_toggleFilled_changesFontWeight() {
            // Arrange
            blueButton.text = "Test"
            blueButton.filled = true
            wait(50)
            var label = findChild(blueButton, "buttonLabel")
            verify(label.font.bold, "Should be bold when filled")
            
            // Act
            blueButton.filled = false
            wait(50)
            
            // Assert
            verify(!label.font.bold, "Should not be bold when not filled")
        }

        // =====================================================================
        // TEST: Enable/Disable Toggle
        // =====================================================================
        
        function test_toggleEnabled_changesBackground() {
            // Arrange - start enabled
            blueButton.enabled = true
            blueButton.filled = true
            wait(50)
            var bg = findChild(blueButton, "buttonBackground")
            var enabledColor = bg.color.toString()
            
            // Act - disable
            blueButton.enabled = false
            wait(50)
            var disabledColor = bg.color.toString()
            
            // Assert
            verify(enabledColor !== disabledColor, 
                "Background should change when enabled is toggled")
        }
        
        function test_toggleEnabled_changesTextColor() {
            // Arrange
            blueButton.text = "Test"
            blueButton.enabled = true
            wait(50)
            var label = findChild(blueButton, "buttonLabel")
            var enabledTextColor = label.color.toString()
            
            // Act
            blueButton.enabled = false
            wait(50)
            var disabledTextColor = label.color.toString()
            
            // Assert
            verify(enabledTextColor !== disabledTextColor, 
                "Text color should change when enabled is toggled")
        }

        // =====================================================================
        // TEST: Interaction Combinations
        // =====================================================================
        
        function test_disabledFilled_usesOverlayTint() {
            // Arrange
            blueButton.filled = true
            blueButton.enabled = false
            wait(50)
            
            // Assert - disabled takes precedence
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.color.toString(), mockTheme.overlayTint.toString().toLowerCase(),
                "Disabled filled button should use overlayTint")
        }
        
        function test_disabledNotFilled_usesOverlayTint() {
            // Arrange
            blueButton.filled = false
            blueButton.enabled = false
            wait(50)
            
            // Assert - disabled takes precedence
            var bg = findChild(blueButton, "buttonBackground")
            compare(bg.color.toString(), mockTheme.overlayTint.toString().toLowerCase(),
                "Disabled not-filled button should use overlayTint")
        }
    }
}
