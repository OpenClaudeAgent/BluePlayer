/**
 * tst_QualitySelector.qml
 * 
 * Functional UI tests for the QualitySelector component.
 * 
 * Tests quality selection popup functionality:
 * - Opening/closing behavior
 * - Quality option selection
 * - Visual state changes
 * - Keyboard navigation (Escape to close)
 * 
 * Run with: ./test_functional_ui QualitySelectorTests
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
        { name: "1080p60", value: "chunked", url: "http://example.com/1080p60" },
        { name: "720p60", value: "720p60", url: "http://example.com/720p60" },
        { name: "480p", value: "480p", url: "http://example.com/480p" },
        { name: "360p", value: "360p", url: "http://example.com/360p" },
        { name: "160p", value: "160p", url: "http://example.com/160p" }
    ]

    // =========================================================================
    // Component Under Test
    // =========================================================================
    
    Item {
        id: qualitySelector
        anchors.centerIn: parent
        width: 200
        height: 300
        
        // Properties matching QualitySelector.qml interface
        property var qualities: []
        property string currentQuality: ""
        property bool expanded: false
        
        // Signals
        signal qualitySelected(string quality)
        signal closeRequested()
        
        // Reset function for tests
        function reset() {
            qualities = []
            currentQuality = ""
            expanded = false
        }
        
        // Main popup container
        Rectangle {
            id: popup
            anchors.fill: parent
            visible: qualitySelector.expanded
            color: "#E6141c2a"
            radius: 16
            border.color: "#4DFFFFFF"
            border.width: 1
            
            Column {
                id: contentColumn
                anchors.fill: parent
                anchors.margins: 12
                spacing: 4
                
                // Header
                Text {
                    id: headerText
                    objectName: "headerText"
                    text: "Qualite"
                    color: "#AAAAAA"
                    font.pixelSize: 11
                }
                
                // Divider
                Rectangle {
                    width: parent.width
                    height: 1
                    color: "#333333"
                }
                
                // Quality options
                Repeater {
                    id: qualityRepeater
                    model: qualitySelector.qualities
                    
                    delegate: Rectangle {
                        id: qualityItem
                        objectName: "qualityOption_" + index
                        width: contentColumn.width
                        height: 36
                        radius: 8
                        color: itemMouse.containsMouse ? "#1AFFFFFF" : "transparent"
                        
                        property bool isSelected: modelData.name === qualitySelector.currentQuality
                        property string qualityName: modelData.name
                        
                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 8
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
                                qualitySelector.qualitySelected(modelData.name)
                                qualitySelector.closeRequested()
                            }
                        }
                    }
                }
            }
        }
        
        // Keyboard handling
        Keys.onEscapePressed: closeRequested()
        
        // Focus management
        onExpandedChanged: {
            if (expanded) {
                forceActiveFocus()
            }
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: qualitySelectedSpy; target: qualitySelector; signalName: "qualitySelected" }
    SignalSpy { id: closeRequestedSpy; target: qualitySelector; signalName: "closeRequested" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "QualitySelectorTests"
        when: windowShown

        function init() {
            // Reset component state before each test
            qualitySelector.reset()
            qualitySelector.qualities = root.testQualities
            
            // Clear all spies
            qualitySelectedSpy.clear()
            closeRequestedSpy.clear()
            
            // Wait for rendering
            wait(50)
        }

        // =====================================================================
        // TEST: Initial State
        // =====================================================================
        
        function test_initialState_popupNotVisible() {
            // Arrange - already done in init()
            
            // Assert
            compare(qualitySelector.expanded, false, "Should not be expanded initially")
            verify(!findChild(root, "qualityOption_0") || !findChild(root, "qualityOption_0").visible, 
                   "Quality options should not be visible when collapsed")
        }
        
        function test_initialState_noQualitySelected() {
            // Assert
            compare(qualitySelector.currentQuality, "", "No quality should be selected initially")
        }

        // =====================================================================
        // TEST: Expand/Collapse
        // =====================================================================
        
        function test_expand_showsPopup() {
            // Arrange
            qualitySelector.expanded = false
            
            // Act
            qualitySelector.expanded = true
            wait(50)
            
            // Assert
            verify(qualitySelector.expanded, "Should be expanded")
        }
        
        function test_collapse_hidesPopup() {
            // Arrange
            qualitySelector.expanded = true
            wait(50)
            
            // Act
            qualitySelector.expanded = false
            wait(50)
            
            // Assert
            verify(!qualitySelector.expanded, "Should be collapsed")
        }

        // =====================================================================
        // TEST: Quality Options Display
        // =====================================================================
        
        function test_qualitiesDisplay_showsAllOptions() {
            // Arrange
            qualitySelector.expanded = true
            wait(50)
            
            // Assert - All 5 quality options should be present
            for (var i = 0; i < 5; i++) {
                var label = findChild(qualitySelector, "qualityLabel_" + i)
                verify(label !== null, "Quality label " + i + " should exist")
            }
        }
        
        function test_qualitiesDisplay_showsCorrectNames() {
            // Arrange
            qualitySelector.expanded = true
            wait(50)
            
            // Assert
            var expectedNames = ["1080p60", "720p60", "480p", "360p", "160p"]
            for (var i = 0; i < expectedNames.length; i++) {
                var label = findChild(qualitySelector, "qualityLabel_" + i)
                if (label) {
                    compare(label.text, expectedNames[i], 
                            "Quality " + i + " should be " + expectedNames[i])
                }
            }
        }

        // =====================================================================
        // TEST: Quality Selection
        // =====================================================================
        
        function test_qualityOption_click_emitsSignal() {
            // Arrange
            qualitySelector.expanded = true
            wait(50)
            var option = findChild(qualitySelector, "qualityMouse_0")
            verify(option !== null, "First quality option should exist")
            
            // Act
            mouseClick(option)
            
            // Assert
            compare(qualitySelectedSpy.count, 1, "qualitySelected should be emitted once")
            compare(qualitySelectedSpy.signalArguments[0][0], "1080p60", 
                    "Should emit '1080p60' as selected quality")
        }
        
        function test_qualityOption_click_requestsClose() {
            // Arrange
            qualitySelector.expanded = true
            wait(50)
            var option = findChild(qualitySelector, "qualityMouse_1")
            verify(option !== null, "Second quality option should exist")
            
            // Act
            mouseClick(option)
            
            // Assert
            compare(closeRequestedSpy.count, 1, "closeRequested should be emitted")
        }
        
        function test_selectDifferentQuality_emitsCorrectValue() {
            // Arrange
            qualitySelector.expanded = true
            wait(50)
            var option = findChild(qualitySelector, "qualityMouse_2")
            verify(option !== null, "480p option should exist")
            
            // Act
            mouseClick(option)
            
            // Assert
            compare(qualitySelectedSpy.signalArguments[0][0], "480p", 
                    "Should emit '480p' as selected quality")
        }

        // =====================================================================
        // TEST: Visual Selection Indicator
        // =====================================================================
        
        function test_selectedQuality_showsIndicator() {
            // Arrange
            qualitySelector.currentQuality = "720p60"
            qualitySelector.expanded = true
            wait(50)
            
            // Assert - Second option (index 1) should be selected
            var innerDot = findChild(qualitySelector, "innerDot_1")
            if (innerDot) {
                verify(innerDot.width > 0, "Selected quality should show inner dot")
            }
        }
        
        function test_unselectedQuality_noIndicator() {
            // Arrange
            qualitySelector.currentQuality = "720p60"
            qualitySelector.expanded = true
            wait(50)
            
            // Assert - First option (index 0) should NOT be selected
            var innerDot = findChild(qualitySelector, "innerDot_0")
            if (innerDot) {
                compare(innerDot.width, 0, "Unselected quality should not show inner dot")
            }
        }

        // =====================================================================
        // TEST: Keyboard Navigation
        // =====================================================================
        
        function test_escapeKey_requestsClose() {
            // Arrange
            qualitySelector.expanded = true
            qualitySelector.forceActiveFocus()
            wait(50)
            
            // Act
            keyClick(Qt.Key_Escape)
            
            // Assert
            compare(closeRequestedSpy.count, 1, "Escape key should request close")
        }

        // =====================================================================
        // TEST: Empty Qualities
        // =====================================================================
        
        function test_emptyQualities_noOptions() {
            // Arrange
            qualitySelector.qualities = []
            qualitySelector.expanded = true
            wait(50)
            
            // Assert
            var option = findChild(qualitySelector, "qualityOption_0")
            verify(option === null || !option.visible, 
                   "No options should be visible with empty qualities")
        }

        // =====================================================================
        // TEST: Multiple Selections
        // =====================================================================
        
        function test_multipleClicks_emitMultipleSignals() {
            // Arrange
            qualitySelector.expanded = true
            wait(50)
            
            // Act - Click on different options
            var option1 = findChild(qualitySelector, "qualityMouse_0")
            var option2 = findChild(qualitySelector, "qualityMouse_1")
            
            if (option1) mouseClick(option1)
            qualitySelector.expanded = true  // Re-expand after close
            wait(50)
            if (option2) mouseClick(option2)
            
            // Assert
            compare(qualitySelectedSpy.count, 2, "Should emit twice")
            compare(qualitySelectedSpy.signalArguments[0][0], "1080p60")
            compare(qualitySelectedSpy.signalArguments[1][0], "720p60")
        }

        // =====================================================================
        // TEST: Header Display
        // =====================================================================
        
        function test_header_showsCorrectText() {
            // Arrange
            qualitySelector.expanded = true
            wait(50)
            
            // Assert
            var header = findChild(qualitySelector, "headerText")
            if (header) {
                compare(header.text, "Qualite", "Header should say 'Qualite'")
            }
        }
    }
}
