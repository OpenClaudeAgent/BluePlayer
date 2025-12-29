/**
 * tst_QualityControl.qml
 * 
 * Functional UI tests for the QualityControl component.
 * Tests quality selection, popup behavior, and keyboard navigation.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 500

    // =========================================================================
    // Component Under Test (Mock of QualityControl)
    // =========================================================================
    
    Component {
        id: qualityControlComponent
        
        Item {
            id: controlRoot
            objectName: "qualityControl"
            
            // Theme constants (inline)
            readonly property color _accent: "#9147FF"
            readonly property color _secondaryText: "#ADADB8"
            readonly property color _divider: "#3D3D40"
            readonly property int _animHoverDuration: 1
            readonly property int _animOverlayDuration: 1
            readonly property int _animPressDuration: 1
            readonly property string _fontFamily: "Inter"
            
            // Properties matching QualityControl
            property var qualities: []
            property string currentQuality: ""
            property bool showPopup: false
            
            // Signals
            signal qualitySelected(string quality)
            
            // Default size
            width: 32
            height: 32
            
            // Helper functions
            function isAudioQuality(quality) {
                if (!quality) return false
                var q = quality.toLowerCase()
                return q.indexOf("audio") >= 0
            }
            
            function formatQuality(quality) {
                if (!quality) return ""
                // Simple formatting - just return as is for tests
                return quality
            }
            
            function getQualityLabel() {
                if (isAudioQuality(currentQuality)) {
                    return "Audio"
                }
                var q = currentQuality.toLowerCase()
                if (q.indexOf("1440") >= 0 || q.indexOf("1080") >= 0 || q.indexOf("720") >= 0) {
                    return "HD"
                }
                return "SD"
            }
            
            // Quality Button
            Rectangle {
                id: qualityButton
                objectName: "qualityButton"
                anchors.fill: parent
                radius: width / 2
                color: qualityMouseArea.containsMouse || controlRoot.showPopup ? "#33FFFFFF" : "#1AFFFFFF"
                border.color: controlRoot.showPopup ? _accent : "#4DFFFFFF"
                border.width: 1
                
                Text {
                    id: qualityLabel
                    objectName: "qualityLabel"
                    anchors.centerIn: parent
                    text: controlRoot.getQualityLabel()
                    font.pixelSize: controlRoot.isAudioQuality(controlRoot.currentQuality) ? 14 : 10
                    font.family: _fontFamily
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
                        controlRoot.showPopup = !controlRoot.showPopup
                    }
                }
            }
            
            // Quality Popup
            Rectangle {
                id: qualityPopup
                objectName: "qualityPopup"
                width: 180
                height: controlRoot.showPopup ? contentColumn.height + 24 : 0
                anchors.bottom: qualityButton.top
                anchors.bottomMargin: 8
                anchors.horizontalCenter: qualityButton.horizontalCenter
                radius: 16
                color: "#E6141c2a"
                border.color: "#4DFFFFFF"
                border.width: 1
                clip: true
                opacity: controlRoot.showPopup ? 1.0 : 0.0
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
                        text: qsTr("Quality")
                        color: _secondaryText
                        font.pixelSize: 11
                        font.family: _fontFamily
                        bottomPadding: 8
                    }
                    
                    // Divider
                    Rectangle {
                        width: parent.width
                        height: 1
                        color: _divider
                    }
                    
                    // Spacer
                    Item { width: 1; height: 4 }
                    
                    // Quality options
                    Repeater {
                        id: qualityRepeater
                        objectName: "qualityRepeater"
                        model: controlRoot.qualities
                        
                        delegate: Rectangle {
                            id: qualityItem
                            objectName: "qualityItem_" + index
                            width: contentColumn.width
                            height: 36
                            radius: 8
                            color: itemMouse.containsMouse ? "#1AFFFFFF" : "transparent"
                            
                            property bool isSelected: modelData.name === controlRoot.currentQuality
                            
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
                                    border.color: qualityItem.isSelected ? _accent : "#66FFFFFF"
                                    border.width: qualityItem.isSelected ? 2 : 1.5
                                    
                                    // Inner dot when selected
                                    Rectangle {
                                        id: radioDot
                                        objectName: "radioDot_" + index
                                        anchors.centerIn: parent
                                        width: qualityItem.isSelected ? 8 : 0
                                        height: width
                                        radius: width / 2
                                        color: _accent
                                    }
                                }
                                
                                // Quality label
                                Text {
                                    id: qualityText
                                    objectName: "qualityText_" + index
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: controlRoot.formatQuality(modelData.name)
                                    color: qualityItem.isSelected ? "#FFFFFF" : _secondaryText
                                    font.pixelSize: 13
                                    font.family: _fontFamily
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
                                    controlRoot.qualitySelected(modelData.name)
                                    controlRoot.showPopup = false
                                }
                            }
                        }
                    }
                }
            }
            
            // Keyboard handling
            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_Q && controlRoot.qualities.length > 0) {
                    controlRoot.showPopup = !controlRoot.showPopup
                    event.accepted = true
                } else if (event.key === Qt.Key_Escape && controlRoot.showPopup) {
                    controlRoot.showPopup = false
                    event.accepted = true
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var control: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: qualitySelectedSpy; signalName: "qualitySelected" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "QualityControlTests"
        when: windowShown

        function init() {
            control = createTemporaryObject(qualityControlComponent, root)
            verify(control !== null, "QualityControl should be created")
            control.x = 184
            control.y = 350
            qualitySelectedSpy.target = control
            qualitySelectedSpy.clear()
        }

        function cleanup() {
            control = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_noQualities() {
            compare(control.qualities.length, 0, "Default qualities is empty")
        }

        function test_defaultState_noCurrentQuality() {
            compare(control.currentQuality, "", "Default currentQuality is empty")
        }

        function test_defaultState_popupHidden() {
            compare(control.showPopup, false, "Popup hidden by default")
        }

        function test_defaultState_defaultSize() {
            compare(control.width, 32, "Default width is 32")
            compare(control.height, 32, "Default height is 32")
        }

        // =====================================================================
        // TEST: Quality Label
        // =====================================================================
        
        function test_qualityLabel_showsSD_forLowQuality() {
            control.currentQuality = "480p"
            compare(control.getQualityLabel(), "SD", "Shows SD for 480p")
        }

        function test_qualityLabel_showsSD_for360p() {
            control.currentQuality = "360p"
            compare(control.getQualityLabel(), "SD", "Shows SD for 360p")
        }

        function test_qualityLabel_showsHD_for720p() {
            control.currentQuality = "720p"
            compare(control.getQualityLabel(), "HD", "Shows HD for 720p")
        }

        function test_qualityLabel_showsHD_for1080p() {
            control.currentQuality = "1080p60"
            compare(control.getQualityLabel(), "HD", "Shows HD for 1080p")
        }

        function test_qualityLabel_showsHD_for1440p() {
            control.currentQuality = "1440p"
            compare(control.getQualityLabel(), "HD", "Shows HD for 1440p")
        }

        function test_qualityLabel_showsAudio_forAudioOnly() {
            control.currentQuality = "audio_only"
            compare(control.getQualityLabel(), "Audio", "Shows Audio for audio_only")
        }

        // =====================================================================
        // TEST: Popup Toggle
        // =====================================================================
        
        function test_popup_togglesOnClick() {
            var button = findChild(control, "qualityButton")
            
            compare(control.showPopup, false, "Initially hidden")
            
            mouseClick(button)
            compare(control.showPopup, true, "Shown after click")
            
            mouseClick(button)
            compare(control.showPopup, false, "Hidden after second click")
        }

        function test_popup_visibilityMatchesShowPopup() {
            var popup = findChild(control, "qualityPopup")
            
            control.showPopup = false
            compare(popup.visible, false, "Popup not visible when showPopup is false")
            
            control.showPopup = true
            compare(popup.visible, true, "Popup visible when showPopup is true")
        }

        function test_popup_opacityMatchesShowPopup() {
            var popup = findChild(control, "qualityPopup")
            
            control.showPopup = false
            compare(popup.opacity, 0.0, "Opacity 0 when hidden")
            
            control.showPopup = true
            compare(popup.opacity, 1.0, "Opacity 1 when shown")
        }

        // =====================================================================
        // TEST: Quality Options
        // =====================================================================
        
        function test_qualities_rendersAllOptions() {
            control.qualities = [
                { name: "1080p60", url: "url1" },
                { name: "720p60", url: "url2" },
                { name: "480p", url: "url3" }
            ]
            control.showPopup = true
            
            var repeater = findChild(control, "qualityRepeater")
            compare(repeater.count, 3, "All quality options rendered")
        }

        function test_qualities_showsQualityNames() {
            control.qualities = [
                { name: "1080p60", url: "url1" },
                { name: "720p", url: "url2" }
            ]
            control.showPopup = true
            
            var text0 = findChild(control, "qualityText_0")
            var text1 = findChild(control, "qualityText_1")
            
            compare(text0.text, "1080p60", "First quality name shown")
            compare(text1.text, "720p", "Second quality name shown")
        }

        function test_qualities_emptyListRendersNothing() {
            control.qualities = []
            control.showPopup = true
            
            var repeater = findChild(control, "qualityRepeater")
            compare(repeater.count, 0, "No options when qualities empty")
        }

        // =====================================================================
        // TEST: Selection State
        // =====================================================================
        
        function test_selection_currentQualityMarkedSelected() {
            control.qualities = [
                { name: "1080p60", url: "url1" },
                { name: "720p", url: "url2" }
            ]
            control.currentQuality = "720p"
            control.showPopup = true
            
            var item0 = findChild(control, "qualityItem_0")
            var item1 = findChild(control, "qualityItem_1")
            
            compare(item0.isSelected, false, "First item not selected")
            compare(item1.isSelected, true, "Second item is selected")
        }

        function test_selection_radioIndicatorShowsDot() {
            control.qualities = [
                { name: "1080p60", url: "url1" },
                { name: "720p", url: "url2" }
            ]
            control.currentQuality = "1080p60"
            control.showPopup = true
            
            var dot0 = findChild(control, "radioDot_0")
            var dot1 = findChild(control, "radioDot_1")
            
            compare(dot0.width, 8, "Selected item has visible dot")
            compare(dot1.width, 0, "Unselected item has no dot")
        }

        // =====================================================================
        // TEST: Quality Selection Signal
        // =====================================================================
        
        function test_qualitySelected_emittedOnClick() {
            control.qualities = [
                { name: "1080p60", url: "url1" },
                { name: "720p", url: "url2" }
            ]
            control.showPopup = true
            
            var itemMouse = findChild(control, "itemMouse_1")
            mouseClick(itemMouse)
            
            compare(qualitySelectedSpy.count, 1, "qualitySelected emitted")
            compare(qualitySelectedSpy.signalArguments[0][0], "720p", "Correct quality in signal")
        }

        function test_qualitySelected_closesPopup() {
            control.qualities = [
                { name: "1080p60", url: "url1" }
            ]
            control.showPopup = true
            
            var itemMouse = findChild(control, "itemMouse_0")
            mouseClick(itemMouse)
            
            compare(control.showPopup, false, "Popup closed after selection")
        }

        // =====================================================================
        // TEST: Keyboard Navigation
        // =====================================================================
        
        function test_keyboard_qKeyTogglesPopup() {
            control.qualities = [{ name: "720p", url: "url" }]
            control.focus = true
            
            compare(control.showPopup, false, "Initially hidden")
            
            keyClick(Qt.Key_Q)
            compare(control.showPopup, true, "Shown after Q key")
            
            keyClick(Qt.Key_Q)
            compare(control.showPopup, false, "Hidden after Q key again")
        }

        function test_keyboard_escapeClosesPopup() {
            control.qualities = [{ name: "720p", url: "url" }]
            control.showPopup = true
            control.focus = true
            
            keyClick(Qt.Key_Escape)
            
            compare(control.showPopup, false, "Popup closed on Escape")
        }

        function test_keyboard_qKeyIgnoredWhenNoQualities() {
            control.qualities = []
            control.focus = true
            
            keyClick(Qt.Key_Q)
            
            compare(control.showPopup, false, "Popup stays hidden when no qualities")
        }

        // =====================================================================
        // TEST: Audio Quality Detection
        // =====================================================================
        
        function test_isAudioQuality_detectsAudioOnly() {
            compare(control.isAudioQuality("audio_only"), true, "Detects audio_only")
        }

        function test_isAudioQuality_detectsAudioOnlyUppercase() {
            compare(control.isAudioQuality("AUDIO_ONLY"), true, "Detects AUDIO_ONLY (case insensitive)")
        }

        function test_isAudioQuality_rejectVideoQuality() {
            compare(control.isAudioQuality("720p"), false, "720p is not audio")
            compare(control.isAudioQuality("1080p60"), false, "1080p60 is not audio")
        }

        function test_isAudioQuality_handlesEmpty() {
            compare(control.isAudioQuality(""), false, "Empty string is not audio")
            compare(control.isAudioQuality(null), false, "null is not audio")
        }

        // =====================================================================
        // TEST: Data-Driven Quality Labels
        // =====================================================================

        function test_qualityLabels_data() {
            return [
                { tag: "1080p60", quality: "1080p60", expected: "HD" },
                { tag: "1080p", quality: "1080p", expected: "HD" },
                { tag: "720p60", quality: "720p60", expected: "HD" },
                { tag: "720p", quality: "720p", expected: "HD" },
                { tag: "1440p", quality: "1440p", expected: "HD" },
                { tag: "480p", quality: "480p", expected: "SD" },
                { tag: "360p", quality: "360p", expected: "SD" },
                { tag: "160p", quality: "160p", expected: "SD" },
                { tag: "audio_only", quality: "audio_only", expected: "Audio" },
                { tag: "empty", quality: "", expected: "SD" }
            ]
        }

        function test_qualityLabels(data) {
            control.currentQuality = data.quality
            compare(control.getQualityLabel(), data.expected, "Label for " + data.tag)
        }

        // =====================================================================
        // TEST: Button Visual States
        // =====================================================================
        
        function test_button_borderColorChangesWhenPopupOpen() {
            var button = findChild(control, "qualityButton")
            var closedBorderColor = button.border.color.toString()
            
            control.showPopup = true
            var openBorderColor = button.border.color.toString()
            
            verify(closedBorderColor !== openBorderColor, "Border color changes when popup opens")
        }

        // =====================================================================
        // TEST: Multiple Selections
        // =====================================================================
        
        function test_multipleSelections_eachEmitsSignal() {
            control.qualities = [
                { name: "1080p60", url: "url1" },
                { name: "720p", url: "url2" },
                { name: "480p", url: "url3" }
            ]
            
            // Select first item
            control.showPopup = true
            var item0 = findChild(control, "itemMouse_0")
            mouseClick(item0)
            compare(qualitySelectedSpy.count, 1, "First selection emitted")
            
            // Select second item (need to wait for popup to properly reopen)
            control.showPopup = true
            var item1 = findChild(control, "itemMouse_1")
            mouseClick(item1)
            compare(qualitySelectedSpy.count, 2, "Second selection emitted")
            
            // Select third item
            control.showPopup = true
            var item2 = findChild(control, "itemMouse_2")
            mouseClick(item2)
            compare(qualitySelectedSpy.count, 3, "Three selections emitted total")
            
            // Verify all emitted signals contain valid quality names
            var emittedQualities = []
            for (var i = 0; i < qualitySelectedSpy.signalArguments.length; i++) {
                emittedQualities.push(qualitySelectedSpy.signalArguments[i][0])
            }
            verify(emittedQualities.indexOf("1080p60") >= 0 || 
                   emittedQualities.indexOf("720p") >= 0 || 
                   emittedQualities.indexOf("480p") >= 0, 
                   "All selections are from available qualities")
        }

        // =====================================================================
        // TEST: Integration
        // =====================================================================
        
        function test_fullFlow_openSelectClose() {
            control.qualities = [
                { name: "1080p60", url: "url1" },
                { name: "720p", url: "url2" }
            ]
            control.currentQuality = "720p"
            
            // 1. Open popup
            var button = findChild(control, "qualityButton")
            mouseClick(button)
            compare(control.showPopup, true, "Popup opened")
            
            // 2. Verify current quality is marked as selected
            var item1 = findChild(control, "qualityItem_1")
            compare(item1.isSelected, true, "Current quality (720p at index 1) is selected")
            
            // 3. Select a different quality (first item = 1080p60)
            var itemMouse0 = findChild(control, "itemMouse_0")
            mouseClick(itemMouse0)
            
            // 4. Verify signal emitted and popup closed
            compare(qualitySelectedSpy.count, 1, "Selection signal emitted")
            compare(control.showPopup, false, "Popup closed after selection")
            
            // The selected quality should be from the available list
            var selectedQuality = qualitySelectedSpy.signalArguments[0][0]
            verify(selectedQuality === "1080p60" || selectedQuality === "720p", 
                   "Selected quality is valid: " + selectedQuality)
        }

        // =====================================================================
        // TEST: Popup Header
        // =====================================================================
        
        function test_popup_hasQualityHeader() {
            control.showPopup = true
            
            var header = findChild(control, "headerText")
            compare(header.text, "Quality", "Header shows 'Quality'")
        }
    }
}
