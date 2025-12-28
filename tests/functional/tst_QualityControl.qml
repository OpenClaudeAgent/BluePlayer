/**
 * tst_QualityControl.qml
 * 
 * Functional UI tests for the QualityControl component.
 * 
 * Tests quality control button with popup:
 * - Properties: qualities list, currentQuality
 * - Signals: qualitySelected
 * - Popup toggle on button click
 * - Quality selection from popup
 * - HD/SD icon based on quality
 * - Keyboard shortcuts (Q to toggle, Escape to close)
 * 
 * Run with: ./test_functional_ui QualityControlTests
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 500

    // =========================================================================
    // Test Data
    // =========================================================================
    
    readonly property var testQualities: [
        { name: "1080p60", url: "http://example.com/1080p60" },
        { name: "720p60", url: "http://example.com/720p60" },
        { name: "480p", url: "http://example.com/480p" },
        { name: "360p", url: "http://example.com/360p" },
        { name: "160p", url: "http://example.com/160p" }
    ]

    // =========================================================================
    // Mock: QualityControl Component
    // =========================================================================
    
    Item {
        id: qualityControl
        objectName: "qualityControl"
        anchors.centerIn: parent
        width: 32
        height: 32

        // Properties
        property var qualities: []
        property string currentQuality: ""
        property bool showPopup: false

        // Signal
        signal qualitySelected(string quality)

        // Reset function for tests
        function reset() {
            qualities = []
            currentQuality = ""
            showPopup = false
        }

        // Quality Button (HD icon)
        Rectangle {
            id: qualityButton
            objectName: "qualityButton"
            anchors.fill: parent
            radius: width / 2
            color: qualityMouseArea.containsMouse || qualityControl.showPopup ? "#33FFFFFF" : "#1AFFFFFF"
            border.color: qualityControl.showPopup ? "#0066FF" : "#4DFFFFFF"
            border.width: 1

            // HD/SD Icon - based on current quality
            Text {
                id: buttonText
                objectName: "buttonText"
                anchors.centerIn: parent
                text: {
                    var q = qualityControl.currentQuality.toLowerCase()
                    if (q.indexOf("1440") >= 0 || q.indexOf("1080") >= 0 || q.indexOf("720") >= 0) {
                        return "HD"
                    }
                    return "SD"
                }
                font.pixelSize: 10
                font.bold: true
                color: "#FFFFFF"
            }

            MouseArea {
                id: qualityMouseArea
                objectName: "qualityMouseArea"
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    qualityControl.showPopup = !qualityControl.showPopup
                }
            }
        }

        // Quality Selector Popup
        Rectangle {
            id: qualityPopup
            objectName: "qualityPopup"
            width: 180
            height: qualityControl.showPopup ? contentColumn.height + 24 : 0
            anchors.bottom: qualityButton.top
            anchors.bottomMargin: 8
            anchors.horizontalCenter: qualityButton.horizontalCenter
            radius: 16
            color: "#E6141c2a"
            border.color: "#4DFFFFFF"
            border.width: 1
            clip: true
            opacity: qualityControl.showPopup ? 1.0 : 0.0
            visible: height > 0

            Column {
                id: contentColumn
                objectName: "contentColumn"
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 12
                spacing: 4

                // Header
                Text {
                    id: headerText
                    objectName: "headerText"
                    text: qsTr("Qualite")
                    color: "#AAAAAA"
                    font.pixelSize: 11
                }

                // Divider
                Rectangle {
                    width: parent.width
                    height: 1
                    color: "#333333"
                }

                // Spacer
                Item { width: 1; height: 4 }

                // Quality options
                Repeater {
                    id: qualityRepeater
                    model: qualityControl.qualities

                    delegate: Rectangle {
                        id: qualityItem
                        objectName: "qualityOption_" + index
                        width: contentColumn.width
                        height: 36
                        radius: 8
                        color: itemMouse.containsMouse ? "#1AFFFFFF" : "transparent"

                        property bool isSelected: modelData.name === qualityControl.currentQuality
                        property string qualityName: modelData.name

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            spacing: 12

                            // Radio indicator
                            Rectangle {
                                id: radioIndicator
                                objectName: "radioIndicator_" + index
                                anchors.verticalCenter: parent.verticalCenter
                                width: 18
                                height: 18
                                radius: 9
                                color: "transparent"
                                border.color: qualityItem.isSelected ? "#0066FF" : "#66FFFFFF"
                                border.width: qualityItem.isSelected ? 2 : 1.5

                                // Inner dot when selected
                                Rectangle {
                                    id: innerDot
                                    objectName: "innerDot_" + index
                                    anchors.centerIn: parent
                                    width: qualityItem.isSelected ? 8 : 0
                                    height: width
                                    radius: width / 2
                                    color: "#0066FF"
                                }
                            }

                            // Quality label
                            Text {
                                id: qualityLabel
                                objectName: "qualityLabel_" + index
                                anchors.verticalCenter: parent.verticalCenter
                                text: modelData.name
                                color: qualityItem.isSelected ? "#FFFFFF" : "#AAAAAA"
                                font.pixelSize: 13
                            }
                        }

                        MouseArea {
                            id: itemMouse
                            objectName: "qualityMouse_" + index
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                qualityControl.qualitySelected(modelData.name)
                                qualityControl.showPopup = false
                            }
                        }
                    }
                }
            }
        }

        // Keyboard handling
        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Q && qualityControl.qualities.length > 0) {
                qualityControl.showPopup = !qualityControl.showPopup
                event.accepted = true
            } else if (event.key === Qt.Key_Escape && qualityControl.showPopup) {
                qualityControl.showPopup = false
                event.accepted = true
            }
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: qualitySelectedSpy; target: qualityControl; signalName: "qualitySelected" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "QualityControlTests"
        when: windowShown

        function init() {
            // Reset component state before each test
            qualityControl.reset()
            qualityControl.qualities = root.testQualities
            qualityControl.currentQuality = "720p60"
            
            // Clear spy
            qualitySelectedSpy.clear()
            
            // Wait for rendering
            wait(50)
        }

        // =====================================================================
        // TEST: Properties - Initial State
        // =====================================================================
        
        function test_properties_qualitiesListSet() {
            // Assert
            compare(qualityControl.qualities.length, 5, "Should have 5 quality options")
            compare(qualityControl.qualities[0].name, "1080p60", "First quality should be 1080p60")
        }
        
        function test_properties_currentQualitySet() {
            // Assert
            compare(qualityControl.currentQuality, "720p60", "Current quality should be 720p60")
        }
        
        function test_properties_popupClosedInitially() {
            // Assert
            compare(qualityControl.showPopup, false, "Popup should be closed initially")
        }
        
        function test_properties_emptyQualitiesDefault() {
            // Arrange
            qualityControl.reset()
            
            // Assert
            compare(qualityControl.qualities.length, 0, "Default qualities should be empty")
            compare(qualityControl.currentQuality, "", "Default currentQuality should be empty")
        }

        // =====================================================================
        // TEST: HD/SD Icon Display
        // =====================================================================
        
        function test_buttonText_showsHDfor1080p() {
            // Arrange
            qualityControl.currentQuality = "1080p60"
            wait(50)
            
            // Assert
            var buttonText = findChild(qualityControl, "buttonText")
            verify(buttonText !== null, "Button text should exist")
            compare(buttonText.text, "HD", "Should show HD for 1080p60")
        }
        
        function test_buttonText_showsHDfor720p() {
            // Arrange
            qualityControl.currentQuality = "720p60"
            wait(50)
            
            // Assert
            var buttonText = findChild(qualityControl, "buttonText")
            compare(buttonText.text, "HD", "Should show HD for 720p60")
        }
        
        function test_buttonText_showsHDfor1440p() {
            // Arrange
            qualityControl.currentQuality = "1440p"
            wait(50)
            
            // Assert
            var buttonText = findChild(qualityControl, "buttonText")
            compare(buttonText.text, "HD", "Should show HD for 1440p")
        }
        
        function test_buttonText_showsSDfor480p() {
            // Arrange
            qualityControl.currentQuality = "480p"
            wait(50)
            
            // Assert
            var buttonText = findChild(qualityControl, "buttonText")
            compare(buttonText.text, "SD", "Should show SD for 480p")
        }
        
        function test_buttonText_showsSDfor360p() {
            // Arrange
            qualityControl.currentQuality = "360p"
            wait(50)
            
            // Assert
            var buttonText = findChild(qualityControl, "buttonText")
            compare(buttonText.text, "SD", "Should show SD for 360p")
        }
        
        function test_buttonText_showsSDforEmptyQuality() {
            // Arrange
            qualityControl.currentQuality = ""
            wait(50)
            
            // Assert
            var buttonText = findChild(qualityControl, "buttonText")
            compare(buttonText.text, "SD", "Should show SD for empty quality")
        }

        // =====================================================================
        // TEST: Popup Toggle on Button Click
        // =====================================================================
        
        function test_buttonClick_opensPopup() {
            // Arrange
            qualityControl.showPopup = false
            var button = findChild(qualityControl, "qualityMouseArea")
            verify(button !== null, "Button mouse area should exist")
            
            // Act
            mouseClick(button)
            wait(50)
            
            // Assert
            compare(qualityControl.showPopup, true, "Popup should be open after click")
        }
        
        function test_buttonClick_closesPopupWhenOpen() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            var button = findChild(qualityControl, "qualityMouseArea")
            
            // Act
            mouseClick(button)
            wait(50)
            
            // Assert
            compare(qualityControl.showPopup, false, "Popup should be closed after click")
        }
        
        function test_buttonClick_togglesPopupTwice() {
            // Arrange
            var button = findChild(qualityControl, "qualityMouseArea")
            qualityControl.showPopup = false
            
            // Act & Assert - First click opens
            mouseClick(button)
            wait(50)
            compare(qualityControl.showPopup, true, "First click should open popup")
            
            // Act & Assert - Second click closes
            mouseClick(button)
            wait(50)
            compare(qualityControl.showPopup, false, "Second click should close popup")
        }

        // =====================================================================
        // TEST: Popup Visibility
        // =====================================================================
        
        function test_popup_visibleWhenOpen() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            
            // Assert
            var popup = findChild(qualityControl, "qualityPopup")
            verify(popup !== null, "Popup should exist")
            verify(popup.visible, "Popup should be visible when showPopup is true")
            compare(popup.opacity, 1.0, "Popup opacity should be 1.0")
        }
        
        function test_popup_invisibleWhenClosed() {
            // Arrange
            qualityControl.showPopup = false
            wait(50)
            
            // Assert
            var popup = findChild(qualityControl, "qualityPopup")
            compare(popup.height, 0, "Popup height should be 0 when closed")
        }

        // =====================================================================
        // TEST: Quality Options Display
        // =====================================================================
        
        function test_popup_showsAllQualityOptions() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            
            // Assert
            for (var i = 0; i < 5; i++) {
                var label = findChild(qualityControl, "qualityLabel_" + i)
                verify(label !== null, "Quality label " + i + " should exist")
            }
        }
        
        function test_popup_showsCorrectQualityNames() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            var expectedNames = ["1080p60", "720p60", "480p", "360p", "160p"]
            
            // Assert
            for (var i = 0; i < expectedNames.length; i++) {
                var label = findChild(qualityControl, "qualityLabel_" + i)
                if (label) {
                    compare(label.text, expectedNames[i], 
                            "Quality " + i + " should be " + expectedNames[i])
                }
            }
        }
        
        function test_popup_showsHeader() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            
            // Assert
            var header = findChild(qualityControl, "headerText")
            verify(header !== null, "Header should exist")
            compare(header.text, "Qualite", "Header text should be 'Qualite'")
        }

        // =====================================================================
        // TEST: Quality Selection - Signal Emission
        // =====================================================================
        
        function test_qualityOption_click_emitsSignal() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            var option = findChild(qualityControl, "qualityMouse_0")
            verify(option !== null, "First quality option should exist")
            
            // Act
            mouseClick(option)
            
            // Assert
            compare(qualitySelectedSpy.count, 1, "qualitySelected should be emitted once")
            compare(qualitySelectedSpy.signalArguments[0][0], "1080p60", 
                    "Should emit '1080p60' as selected quality")
        }
        
        function test_qualityOption_click_closesPopup() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            var option = findChild(qualityControl, "qualityMouse_1")
            verify(option !== null, "Quality option should exist")
            
            // Act
            mouseClick(option)
            wait(50)
            
            // Assert
            compare(qualityControl.showPopup, false, "Popup should close after selection")
        }
        
        function test_selectDifferentQuality_emitsCorrectValue() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            var option = findChild(qualityControl, "qualityMouse_2")
            verify(option !== null, "480p option should exist")
            
            // Act
            mouseClick(option)
            
            // Assert
            compare(qualitySelectedSpy.signalArguments[0][0], "480p", 
                    "Should emit '480p' as selected quality")
        }
        
        function test_selectLastQuality_emitsCorrectValue() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            var option = findChild(qualityControl, "qualityMouse_4")
            verify(option !== null, "160p option should exist")
            
            // Act
            mouseClick(option)
            
            // Assert
            compare(qualitySelectedSpy.signalArguments[0][0], "160p", 
                    "Should emit '160p' as selected quality")
        }

        // =====================================================================
        // TEST: Visual Selection Indicator
        // =====================================================================
        
        function test_selectedQuality_showsIndicatorDot() {
            // Arrange - 720p60 is at index 1
            qualityControl.currentQuality = "720p60"
            qualityControl.showPopup = true
            wait(50)
            
            // Assert
            var innerDot = findChild(qualityControl, "innerDot_1")
            verify(innerDot !== null, "Inner dot should exist")
            verify(innerDot.width > 0, "Selected quality should show inner dot")
        }
        
        function test_unselectedQuality_noIndicatorDot() {
            // Arrange - 720p60 is selected, checking index 0 (1080p60)
            qualityControl.currentQuality = "720p60"
            qualityControl.showPopup = true
            wait(50)
            
            // Assert
            var innerDot = findChild(qualityControl, "innerDot_0")
            verify(innerDot !== null, "Inner dot element should exist")
            compare(innerDot.width, 0, "Unselected quality should not show inner dot")
        }
        
        function test_selectedQuality_correctBorderColor() {
            // Arrange
            qualityControl.currentQuality = "480p"
            qualityControl.showPopup = true
            wait(50)
            
            // Assert - 480p is at index 2
            var radioIndicator = findChild(qualityControl, "radioIndicator_2")
            if (radioIndicator) {
                compare(radioIndicator.border.color.toString(), "#0066ff", 
                        "Selected quality radio should have accent border")
            }
        }

        // =====================================================================
        // TEST: Keyboard Navigation
        // =====================================================================
        
        function test_keyQ_opensPopup() {
            // Arrange
            qualityControl.showPopup = false
            qualityControl.forceActiveFocus()
            wait(50)
            
            // Act
            keyClick(Qt.Key_Q)
            wait(50)
            
            // Assert
            compare(qualityControl.showPopup, true, "Q key should open popup")
        }
        
        function test_keyQ_closesPopupWhenOpen() {
            // Arrange
            qualityControl.showPopup = true
            qualityControl.forceActiveFocus()
            wait(50)
            
            // Act
            keyClick(Qt.Key_Q)
            wait(50)
            
            // Assert
            compare(qualityControl.showPopup, false, "Q key should close popup when open")
        }
        
        function test_keyEscape_closesPopup() {
            // Arrange
            qualityControl.showPopup = true
            qualityControl.forceActiveFocus()
            wait(50)
            
            // Act
            keyClick(Qt.Key_Escape)
            wait(50)
            
            // Assert
            compare(qualityControl.showPopup, false, "Escape should close popup")
        }
        
        function test_keyEscape_doesNothingWhenClosed() {
            // Arrange
            qualityControl.showPopup = false
            qualityControl.forceActiveFocus()
            wait(50)
            
            // Act
            keyClick(Qt.Key_Escape)
            wait(50)
            
            // Assert
            compare(qualityControl.showPopup, false, "Popup should remain closed")
        }
        
        function test_keyQ_noEffectWithEmptyQualities() {
            // Arrange
            qualityControl.qualities = []
            qualityControl.showPopup = false
            qualityControl.forceActiveFocus()
            wait(50)
            
            // Act
            keyClick(Qt.Key_Q)
            wait(50)
            
            // Assert
            compare(qualityControl.showPopup, false, 
                    "Q key should not open popup with empty qualities")
        }

        // =====================================================================
        // TEST: Empty Qualities
        // =====================================================================
        
        function test_emptyQualities_noOptions() {
            // Arrange
            qualityControl.qualities = []
            qualityControl.showPopup = true
            wait(50)
            
            // Assert
            var option = findChild(qualityControl, "qualityOption_0")
            verify(option === null, "No options should exist with empty qualities")
        }

        // =====================================================================
        // TEST: Multiple Selections
        // =====================================================================
        
        function test_multipleSelections_emitMultipleSignals() {
            // Arrange
            qualityControl.showPopup = true
            wait(50)
            
            // Act - First selection
            var option1 = findChild(qualityControl, "qualityMouse_0")
            mouseClick(option1)
            
            // Re-open popup
            qualityControl.showPopup = true
            wait(50)
            
            // Act - Second selection
            var option2 = findChild(qualityControl, "qualityMouse_3")
            mouseClick(option2)
            
            // Assert
            compare(qualitySelectedSpy.count, 2, "Should emit twice for two selections")
            compare(qualitySelectedSpy.signalArguments[0][0], "1080p60", "First selection")
            compare(qualitySelectedSpy.signalArguments[1][0], "360p", "Second selection")
        }

        // =====================================================================
        // TEST: Button Dimensions
        // =====================================================================
        
        function test_button_defaultSize() {
            // Assert
            compare(qualityControl.width, 32, "Default width should be 32")
            compare(qualityControl.height, 32, "Default height should be 32")
        }
        
        function test_button_isCircular() {
            // Assert
            var button = findChild(qualityControl, "qualityButton")
            verify(button !== null, "Quality button should exist")
            compare(button.radius, button.width / 2, "Button should be circular")
        }
    }
}
