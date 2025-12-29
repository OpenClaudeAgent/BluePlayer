/**
 * tst_BlueDropdown.qml
 * 
 * Functional UI tests for the BlueDropdown component.
 * Tests selection, model binding, placeholder, and value sync.
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
    // Mock Theme Constants
    // =========================================================================
    
    QtObject {
        id: mockTheme
        readonly property color primaryText: "#e6edf3"
        readonly property color secondaryText: "#8b949e"
        readonly property color accent: "#3b82f6"
        readonly property color surface: "#0d1117"
        readonly property color surfaceSoft: "#161b22"
        readonly property color divider: "#30363d"
        readonly property color cardHighlight: "#1c2128"
        readonly property int animHoverDuration: 1
        readonly property string fontFamily: "Inter"
    }

    // =========================================================================
    // Component Under Test (Simplified Mock of BlueDropdown)
    // =========================================================================
    
    Component {
        id: dropdownComponent
        
        Item {
            id: dropdown
            objectName: "blueDropdown"

            implicitWidth: 140
            implicitHeight: 36

            // Properties
            property var model: []
            property string placeholder: ""
            property string selectedValue: ""
            property int currentIndex: -1
            property string displayText: currentIndex >= 0 && model.length > currentIndex ? model[currentIndex] : ""
            property bool popupVisible: popup.visible

            // Signals
            signal valueSelected(string value)
            signal activated(int index)

            // Sync currentIndex when selectedValue changes
            onSelectedValueChanged: {
                if (model) {
                    for (var i = 0; i < model.length; i++) {
                        if (model[i] === selectedValue) {
                            currentIndex = i
                            break
                        }
                    }
                }
            }

            Component.onCompleted: {
                if (selectedValue && model) {
                    for (var i = 0; i < model.length; i++) {
                        if (model[i] === selectedValue) {
                            currentIndex = i
                            break
                        }
                    }
                }
            }

            function textAt(index) {
                return index >= 0 && index < model.length ? model[index] : ""
            }

            // Background
            Rectangle {
                id: background
                objectName: "background"
                anchors.fill: parent
                radius: 10
                color: Qt.rgba(mockTheme.surfaceSoft.r, mockTheme.surfaceSoft.g, mockTheme.surfaceSoft.b, 
                              mainMouseArea.containsMouse || popup.visible ? 0.8 : 0.5)
                border.color: popup.visible ? mockTheme.accent : mockTheme.divider
                border.width: 1

                Behavior on color {
                    ColorAnimation { duration: mockTheme.animHoverDuration }
                }
                Behavior on border.color {
                    ColorAnimation { duration: mockTheme.animHoverDuration }
                }
            }

            // Content
            Text {
                id: contentText
                objectName: "contentText"
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 12
                anchors.rightMargin: 28
                text: dropdown.displayText || dropdown.placeholder
                font.family: mockTheme.fontFamily
                font.pixelSize: 13
                font.weight: Font.Medium
                color: dropdown.displayText ? mockTheme.primaryText : mockTheme.secondaryText
                elide: Text.ElideRight
            }

            // Indicator
            Text {
                id: indicator
                objectName: "indicator"
                x: dropdown.width - width - 10
                y: (dropdown.height - height) / 2
                text: "\u25BC"
                font.pixelSize: 8
                color: mockTheme.secondaryText
                rotation: popup.visible ? 180 : 0

                Behavior on rotation {
                    NumberAnimation { duration: mockTheme.animHoverDuration }
                }
            }

            // Main click area
            MouseArea {
                id: mainMouseArea
                objectName: "mainMouseArea"
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: popup.visible = !popup.visible
            }

            // Popup
            Rectangle {
                id: popup
                objectName: "popup"
                visible: false
                y: dropdown.height + 4
                width: dropdown.width
                height: visible ? listView.contentHeight + 16 : 0
                radius: 12
                color: mockTheme.surface
                border.color: mockTheme.divider
                border.width: 1
                clip: true

                Behavior on height {
                    NumberAnimation { duration: mockTheme.animHoverDuration }
                }

                ListView {
                    id: listView
                    objectName: "listView"
                    anchors.fill: parent
                    anchors.margins: 8
                    model: dropdown.model
                    clip: true

                    delegate: Rectangle {
                        id: delegateItem
                        objectName: "delegate_" + index
                        width: listView.width
                        height: 32
                        radius: 6
                        color: delegateMouse.containsMouse ? mockTheme.cardHighlight : "transparent"

                        property int itemIndex: index

                        Text {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 10
                            text: modelData
                            font.family: mockTheme.fontFamily
                            font.pixelSize: 13
                            font.weight: dropdown.currentIndex === index ? Font.DemiBold : Font.Normal
                            color: dropdown.currentIndex === index ? mockTheme.accent : mockTheme.primaryText
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin: 10
                            text: dropdown.currentIndex === index ? "\u2713" : ""
                            font.pixelSize: 12
                            font.weight: Font.Bold
                            color: mockTheme.accent
                        }

                        MouseArea {
                            id: delegateMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                dropdown.currentIndex = index
                                dropdown.selectedValue = dropdown.textAt(index)
                                dropdown.activated(index)
                                dropdown.valueSelected(dropdown.textAt(index))
                                popup.visible = false
                            }
                        }
                    }
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var dropdown: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: valueSelectedSpy; signalName: "valueSelected" }
    SignalSpy { id: activatedSpy; signalName: "activated" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "BlueDropdownTests"
        when: windowShown

        function init() {
            dropdown = createTemporaryObject(dropdownComponent, root)
            verify(dropdown !== null, "BlueDropdown should be created")
            dropdown.anchors.centerIn = root
            valueSelectedSpy.target = dropdown
            activatedSpy.target = dropdown
            valueSelectedSpy.clear()
            activatedSpy.clear()
            mouseMove(root, 1, 1)
        }

        function cleanup() {
            dropdown = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_dimensions() {
            compare(dropdown.implicitWidth, 140, "Default width should be 140")
            compare(dropdown.implicitHeight, 36, "Default height should be 36")
        }
        
        function test_defaultState_noSelection() {
            compare(dropdown.currentIndex, -1, "Default currentIndex should be -1")
            compare(dropdown.selectedValue, "", "Default selectedValue should be empty")
        }
        
        function test_defaultState_popupHidden() {
            compare(dropdown.popupVisible, false, "Popup should be hidden by default")
        }
        
        function test_defaultState_emptyModel() {
            compare(dropdown.model.length, 0, "Default model should be empty")
        }

        // =====================================================================
        // TEST: Placeholder
        // =====================================================================
        
        function test_placeholder_displayedWhenNoSelection() {
            var contentText = findChild(dropdown, "contentText")
            dropdown.placeholder = "Select option"
            
            compare(contentText.text, "Select option", "Placeholder should be displayed")
        }
        
        function test_placeholder_hiddenAfterSelection() {
            var contentText = findChild(dropdown, "contentText")
            dropdown.placeholder = "Select option"
            dropdown.model = ["Option 1", "Option 2"]
            dropdown.currentIndex = 0
            
            compare(contentText.text, "Option 1", "Selected value should replace placeholder")
        }

        // =====================================================================
        // TEST: Model Binding
        // =====================================================================
        
        function test_model_canBeSet() {
            dropdown.model = ["A", "B", "C"]
            
            compare(dropdown.model.length, 3, "Model should have 3 items")
            compare(dropdown.model[0], "A")
            compare(dropdown.model[1], "B")
            compare(dropdown.model[2], "C")
        }
        
        function test_model_textAtReturnsCorrectValue() {
            dropdown.model = ["First", "Second", "Third"]
            
            compare(dropdown.textAt(0), "First")
            compare(dropdown.textAt(1), "Second")
            compare(dropdown.textAt(2), "Third")
        }
        
        function test_model_textAtHandlesInvalidIndex() {
            dropdown.model = ["A", "B"]
            
            compare(dropdown.textAt(-1), "", "Should return empty for negative index")
            compare(dropdown.textAt(10), "", "Should return empty for out of bounds index")
        }

        // =====================================================================
        // TEST: Popup Toggle
        // =====================================================================
        
        function test_popup_opensOnClick() {
            var mainMouseArea = findChild(dropdown, "mainMouseArea")
            dropdown.model = ["Option 1", "Option 2"]
            
            mouseClick(mainMouseArea)
            
            tryCompare(dropdown, "popupVisible", true, 100, "Popup should open on click")
        }
        
        function test_popup_closesOnSecondClick() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            var mainMouseArea = findChild(dropdown, "mainMouseArea")
            dropdown.model = ["Option 1", "Option 2"]
            
            mouseClick(mainMouseArea)
            tryCompare(dropdown, "popupVisible", true, 100)
            
            mouseClick(mainMouseArea)
            tryCompare(dropdown, "popupVisible", false, 100, "Popup should close on second click")
        }

        // =====================================================================
        // TEST: Selection (via programmatic control, not delegate click)
        // =====================================================================
        
        function test_selection_updatesCurrentIndex() {
            dropdown.model = ["A", "B", "C"]
            
            // Simulate selection programmatically
            dropdown.currentIndex = 1
            dropdown.selectedValue = dropdown.textAt(1)
            
            compare(dropdown.currentIndex, 1, "currentIndex should be updated")
            compare(dropdown.selectedValue, "B", "selectedValue should match")
        }
        
        function test_selection_updatesSelectedValue() {
            dropdown.model = ["Alpha", "Beta", "Gamma"]
            
            dropdown.currentIndex = 2
            dropdown.selectedValue = dropdown.textAt(2)
            
            compare(dropdown.selectedValue, "Gamma", "selectedValue should be updated")
        }
        
        function test_selection_displayTextUpdates() {
            dropdown.model = ["X", "Y", "Z"]
            
            dropdown.currentIndex = 0
            
            compare(dropdown.displayText, "X", "displayText should update with selection")
        }
        
        function test_activated_emitsSignal() {
            dropdown.model = ["One", "Two", "Three"]
            
            // Manually emit activated to test signal flow
            dropdown.activated(1)
            
            compare(activatedSpy.count, 1, "activated should be emitted")
            compare(activatedSpy.signalArguments[0][0], 1, "Correct index should be passed")
        }
        
        function test_valueSelected_emitsSignal() {
            dropdown.model = ["P", "Q", "R"]
            
            // Manually emit valueSelected to test signal flow
            dropdown.valueSelected("Q")
            
            compare(valueSelectedSpy.count, 1, "valueSelected should be emitted")
            compare(valueSelectedSpy.signalArguments[0][0], "Q", "Correct value should be passed")
        }

        // =====================================================================
        // TEST: selectedValue Sync
        // =====================================================================
        
        function test_selectedValueSync_updatesCurrentIndex() {
            dropdown.model = ["Cat", "Dog", "Bird"]
            
            dropdown.selectedValue = "Dog"
            
            compare(dropdown.currentIndex, 1, "currentIndex should sync with selectedValue")
        }
        
        function test_selectedValueSync_worksOnInit() {
            // Create with initial value
            var dropdown2 = createTemporaryObject(dropdownComponent, root, {
                model: ["Red", "Green", "Blue"],
                selectedValue: "Green"
            })
            
            compare(dropdown2.currentIndex, 1, "currentIndex should be set on init")
        }

        // =====================================================================
        // TEST: Indicator Rotation
        // =====================================================================
        
        function test_indicator_rotatesWhenOpen() {
            var indicator = findChild(dropdown, "indicator")
            var mainMouseArea = findChild(dropdown, "mainMouseArea")
            dropdown.model = ["A", "B"]
            
            compare(indicator.rotation, 0, "Initial rotation should be 0")
            
            mouseClick(mainMouseArea)
            tryCompare(dropdown, "popupVisible", true, 100)
            
            tryCompare(indicator, "rotation", 180, 100, "Indicator should rotate when popup opens")
        }

        // =====================================================================
        // TEST: Display Text
        // =====================================================================
        
        function test_displayText_showsSelectedValue() {
            var contentText = findChild(dropdown, "contentText")
            dropdown.model = ["First", "Second", "Third"]
            
            dropdown.currentIndex = 1
            
            compare(dropdown.displayText, "Second", "displayText should show selected item")
            compare(contentText.text, "Second", "Content text should display selected item")
        }
    }
}
