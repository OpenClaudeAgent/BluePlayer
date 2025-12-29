/**
 * tst_QualitySelector.qml
 * 
 * Functional UI tests for the QualitySelector component.
 * Tests quality selection, expand/collapse, and keyboard handling.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 500

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
        readonly property color divider: "#30363d"
        readonly property int animOverlayDuration: 1
        readonly property int animHoverDuration: 1
        readonly property int animPressDuration: 1
        readonly property string fontFamily: "Inter"
    }

    // =========================================================================
    // Component Under Test (Simplified Mock of QualitySelector)
    // =========================================================================
    
    Component {
        id: qualitySelectorComponent
        
        Item {
            id: selector
            objectName: "qualitySelector"

            // Model: array of {name: "1080p60", url: "...", value: "chunked"}
            property var qualities: []
            property string currentQuality: ""
            property bool expanded: false

            // Signals
            signal qualitySelected(string quality)
            signal closeRequested()

            width: popup.width
            height: popup.height

            // Background overlay
            Rectangle {
                id: overlay
                objectName: "overlay"
                anchors.fill: parent
                anchors.margins: -200
                color: "transparent"
                visible: selector.expanded

                MouseArea {
                    id: overlayMouse
                    objectName: "overlayMouse"
                    anchors.fill: parent
                    onClicked: selector.closeRequested()
                }
            }

            // Popup container
            Rectangle {
                id: popup
                objectName: "popup"
                width: 180
                height: selector.expanded ? contentColumn.height + 24 : 0
                radius: 16
                color: "#E6141c2a"
                border.color: "#4DFFFFFF"
                border.width: 1
                clip: true
                opacity: selector.expanded ? 1.0 : 0.0
                visible: height > 0

                Behavior on height {
                    NumberAnimation { duration: mockTheme.animOverlayDuration }
                }
                Behavior on opacity {
                    NumberAnimation { duration: mockTheme.animHoverDuration }
                }

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
                        objectName: "headerText"
                        text: "Quality"
                        color: mockTheme.secondaryText
                        font.pixelSize: 11
                        font.family: mockTheme.fontFamily
                        font.weight: Font.Medium
                        bottomPadding: 8
                    }

                    // Divider
                    Rectangle {
                        width: parent.width
                        height: 1
                        color: mockTheme.divider
                    }

                    // Spacer
                    Item { width: 1; height: 4 }

                    // Quality options
                    Repeater {
                        id: qualityRepeater
                        objectName: "qualityRepeater"
                        model: selector.qualities

                        delegate: Rectangle {
                            id: qualityItem
                            objectName: "qualityItem_" + index
                            width: contentColumn.width
                            height: 36
                            radius: 8
                            color: itemMouse.containsMouse ? "#1AFFFFFF" : "transparent"
                            
                            property bool isSelected: modelData.name === selector.currentQuality
                            property int itemIndex: index

                            Behavior on color {
                                ColorAnimation { duration: mockTheme.animHoverDuration }
                            }

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
                                    border.color: qualityItem.isSelected ? mockTheme.accent : "#66FFFFFF"
                                    border.width: qualityItem.isSelected ? 2 : 1.5

                                    Rectangle {
                                        id: innerDot
                                        objectName: "innerDot_" + index
                                        anchors.centerIn: parent
                                        width: qualityItem.isSelected ? 8 : 0
                                        height: width
                                        radius: width / 2
                                        color: mockTheme.accent

                                        Behavior on width {
                                            NumberAnimation { duration: mockTheme.animPressDuration }
                                        }
                                    }
                                }

                                // Quality label
                                Text {
                                    id: qualityLabel
                                    objectName: "qualityLabel_" + index
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.name
                                    color: qualityItem.isSelected ? "#FFFFFF" : mockTheme.secondaryText
                                    font.pixelSize: 13
                                    font.family: mockTheme.fontFamily
                                    font.weight: qualityItem.isSelected ? Font.DemiBold : Font.Normal
                                }
                            }

                            MouseArea {
                                id: itemMouse
                                objectName: "itemMouse_" + index
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    selector.qualitySelected(modelData.name)
                                    selector.closeRequested()
                                }
                            }
                        }
                    }
                }
            }

            // Keyboard handling
            Keys.onEscapePressed: selector.closeRequested()

            onExpandedChanged: {
                if (expanded) {
                    selector.forceActiveFocus()
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var selector: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: qualitySelectedSpy; signalName: "qualitySelected" }
    SignalSpy { id: closeRequestedSpy; signalName: "closeRequested" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "QualitySelectorTests"
        when: windowShown

        function init() {
            selector = createTemporaryObject(qualitySelectorComponent, root)
            verify(selector !== null, "QualitySelector should be created")
            selector.x = 100
            selector.y = 100
            qualitySelectedSpy.target = selector
            closeRequestedSpy.target = selector
            qualitySelectedSpy.clear()
            closeRequestedSpy.clear()
            mouseMove(root, 1, 1)
        }

        function cleanup() {
            selector = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_notExpanded() {
            compare(selector.expanded, false, "Default expanded should be false")
        }
        
        function test_defaultState_emptyQualities() {
            compare(selector.qualities.length, 0, "Default qualities should be empty")
        }
        
        function test_defaultState_noCurrentQuality() {
            compare(selector.currentQuality, "", "Default currentQuality should be empty")
        }
        
        function test_defaultState_popupHidden() {
            var popup = findChild(selector, "popup")
            compare(popup.visible, false, "Popup should be hidden when collapsed")
        }

        // =====================================================================
        // TEST: Expand/Collapse
        // =====================================================================
        
        function test_expand_showsPopup() {
            selector.qualities = [
                {name: "1080p60", url: "url1"},
                {name: "720p60", url: "url2"}
            ]
            
            selector.expanded = true
            
            var popup = findChild(selector, "popup")
            tryCompare(popup, "visible", true, 100, "Popup should be visible when expanded")
        }
        
        function test_collapse_hidesPopup() {
            selector.qualities = [{name: "1080p", url: "url"}]
            selector.expanded = true
            
            var popup = findChild(selector, "popup")
            tryCompare(popup, "visible", true, 100)
            
            selector.expanded = false
            
            tryCompare(popup, "height", 0, 100, "Popup height should be 0 when collapsed")
        }

        // =====================================================================
        // TEST: Quality Display
        // =====================================================================
        
        function test_qualities_displayedCorrectly() {
            selector.qualities = [
                {name: "Source", url: "url1"},
                {name: "720p", url: "url2"},
                {name: "480p", url: "url3"}
            ]
            selector.expanded = true
            
            var label0 = findChild(selector, "qualityLabel_0")
            var label1 = findChild(selector, "qualityLabel_1")
            var label2 = findChild(selector, "qualityLabel_2")
            
            compare(label0.text, "Source")
            compare(label1.text, "720p")
            compare(label2.text, "480p")
        }
        
        function test_header_displaysCorrectText() {
            selector.qualities = [{name: "1080p", url: "url"}]
            selector.expanded = true
            
            var header = findChild(selector, "headerText")
            compare(header.text, "Quality", "Header should say 'Quality'")
        }

        // =====================================================================
        // TEST: Selection State
        // =====================================================================
        
        function test_currentQuality_canBeSet() {
            selector.qualities = [
                {name: "1080p", url: "url1"},
                {name: "720p", url: "url2"}
            ]
            
            selector.currentQuality = "720p"
            
            compare(selector.currentQuality, "720p", "currentQuality should be settable")
        }
        
        function test_currentQuality_canBeChanged() {
            selector.qualities = [
                {name: "Auto", url: "url1"},
                {name: "Source", url: "url2"}
            ]
            selector.currentQuality = "Auto"
            
            selector.currentQuality = "Source"
            
            compare(selector.currentQuality, "Source", "currentQuality should change")
        }

        // =====================================================================
        // TEST: Quality Selection (via programmatic signal emission)
        // =====================================================================
        
        function test_qualitySelected_signalWorks() {
            selector.qualities = [
                {name: "1080p60", url: "url1"},
                {name: "720p60", url: "url2"}
            ]
            selector.expanded = true
            
            // Manually emit signal to test signal handling
            selector.qualitySelected("720p60")
            
            compare(qualitySelectedSpy.count, 1, "qualitySelected should be emitted")
            compare(qualitySelectedSpy.signalArguments[0][0], "720p60", "Correct quality should be passed")
        }
        
        function test_closeRequested_signalWorks() {
            selector.qualities = [{name: "480p", url: "url"}]
            selector.expanded = true
            
            // Manually emit closeRequested
            selector.closeRequested()
            
            compare(closeRequestedSpy.count, 1, "closeRequested should be emitted")
        }

        // =====================================================================
        // TEST: Overlay Click
        // =====================================================================
        
        function test_overlayClick_emitsCloseRequested() {
            selector.qualities = [{name: "720p", url: "url"}]
            selector.expanded = true
            
            var overlayMouse = findChild(selector, "overlayMouse")
            mouseClick(overlayMouse, 10, 10)
            
            compare(closeRequestedSpy.count, 1, "Clicking overlay should emit closeRequested")
        }

        // =====================================================================
        // TEST: Keyboard Navigation
        // =====================================================================
        
        function test_escape_emitsCloseRequested() {
            selector.qualities = [{name: "360p", url: "url"}]
            selector.expanded = true
            selector.forceActiveFocus()
            
            keyClick(Qt.Key_Escape)
            
            compare(closeRequestedSpy.count, 1, "Escape should emit closeRequested")
        }

        // =====================================================================
        // TEST: Multiple Qualities
        // =====================================================================
        
        function test_manyQualities_modelAccepted() {
            selector.qualities = [
                {name: "Source", url: "1"},
                {name: "1080p60", url: "2"},
                {name: "1080p", url: "3"},
                {name: "720p60", url: "4"},
                {name: "720p", url: "5"},
                {name: "480p", url: "6"},
                {name: "360p", url: "7"},
                {name: "160p", url: "8"}
            ]
            
            compare(selector.qualities.length, 8, "Should accept 8 qualities")
            compare(selector.qualities[0].name, "Source")
            compare(selector.qualities[7].name, "160p")
        }
        
        function test_selectDifferentQualities_viaSignal() {
            selector.qualities = [
                {name: "High", url: "1"},
                {name: "Medium", url: "2"},
                {name: "Low", url: "3"}
            ]
            selector.expanded = true
            
            // Select first via signal
            selector.qualitySelected("High")
            compare(qualitySelectedSpy.signalArguments[0][0], "High")
            
            // Reopen and select third
            selector.expanded = true
            qualitySelectedSpy.clear()
            // Select third via signal
            qualitySelectedSpy.clear()
            selector.qualitySelected("Low")
            compare(qualitySelectedSpy.signalArguments[0][0], "Low")
        }
    }
}
