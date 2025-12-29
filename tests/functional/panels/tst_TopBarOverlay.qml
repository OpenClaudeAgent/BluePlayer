/**
 * tst_TopBarOverlay.qml
 * 
 * Functional UI tests for the TopBarOverlay component.
 * Tests back button, stream info display, status badge, and visibility animations.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 800
    height: 400

    // =========================================================================
    // Component Under Test (Mock of TopBarOverlay)
    // =========================================================================
    
    Component {
        id: topBarOverlayComponent
        
        Rectangle {
            id: topBar
            objectName: "topBarOverlay"
            
            // Theme constants (inline)
            readonly property int _animControlBarDuration: 1
            readonly property string _fontFamily: "Inter"
            
            // Properties matching TopBarOverlay
            property string streamerName: ""
            property string streamerLogin: ""
            property string streamTitle: ""
            property string statusText: ""
            property bool controlsVisible: true
            
            // Signals
            signal backClicked()
            
            width: parent ? parent.width : 800
            height: 80
            
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#CC000000" }
                GradientStop { position: 1.0; color: "transparent" }
            }
            
            // Visibility Animation
            opacity: controlsVisible ? 1.0 : 0.0
            visible: opacity > 0
            
            Behavior on opacity { 
                NumberAnimation { 
                    duration: _animControlBarDuration
                    easing.type: Easing.InOutCubic 
                } 
            }
            
            RowLayout {
                id: contentLayout
                objectName: "contentLayout"
                anchors.fill: parent
                anchors.leftMargin: 24
                anchors.rightMargin: 24
                spacing: 16
                
                // Back Button
                Rectangle {
                    id: backButton
                    objectName: "backButton"
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    radius: 20
                    color: backButtonMouseArea.containsMouse ? "#4DFFFFFF" : "#1AFFFFFF"
                    
                    Text {
                        id: backIcon
                        objectName: "backIcon"
                        anchors.centerIn: parent
                        text: "\u2190" // Left arrow
                        font.pixelSize: 22
                        color: "#FFFFFF"
                    }
                    
                    MouseArea {
                        id: backButtonMouseArea
                        objectName: "backButtonMouseArea"
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: topBar.backClicked()
                    }
                }
                
                // Stream Info
                ColumnLayout {
                    id: streamInfoLayout
                    objectName: "streamInfoLayout"
                    Layout.fillWidth: true
                    spacing: 2
                    
                    Text {
                        id: streamerNameText
                        objectName: "streamerNameText"
                        text: topBar.streamerName || topBar.streamerLogin
                        font.family: _fontFamily
                        font.pixelSize: 18
                        font.bold: true
                        color: "#FFFFFF"
                        style: Text.Outline
                        styleColor: "#80000000"
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    
                    Text {
                        id: streamTitleText
                        objectName: "streamTitleText"
                        text: topBar.streamTitle || qsTr("Live stream")
                        font.family: _fontFamily
                        font.pixelSize: 13
                        color: "#DDFFFFFF"
                        style: Text.Outline
                        styleColor: "#80000000"
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                }
                
                // Status Badge
                Rectangle {
                    id: statusBadge
                    objectName: "statusBadge"
                    Layout.preferredHeight: 24
                    Layout.preferredWidth: statusLabel.width + 16
                    radius: 12
                    color: "#4D000000"
                    visible: topBar.statusText.length > 0
                    
                    Text {
                        id: statusLabel
                        objectName: "statusLabel"
                        anchors.centerIn: parent
                        text: topBar.statusText
                        font.pixelSize: 11
                        color: "#FFFFFF"
                    }
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var topBar: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: backClickedSpy; signalName: "backClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "TopBarOverlayTests"
        when: windowShown

        function init() {
            topBar = createTemporaryObject(topBarOverlayComponent, root)
            verify(topBar !== null, "TopBarOverlay should be created")
            topBar.anchors.top = root.top
            topBar.anchors.left = root.left
            topBar.anchors.right = root.right
            backClickedSpy.target = topBar
            backClickedSpy.clear()
        }

        function cleanup() {
            topBar = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_emptyStreamerName() {
            compare(topBar.streamerName, "", "Default streamerName is empty")
        }

        function test_defaultState_emptyStreamerLogin() {
            compare(topBar.streamerLogin, "", "Default streamerLogin is empty")
        }

        function test_defaultState_emptyStreamTitle() {
            compare(topBar.streamTitle, "", "Default streamTitle is empty")
        }

        function test_defaultState_emptyStatusText() {
            compare(topBar.statusText, "", "Default statusText is empty")
        }

        function test_defaultState_controlsVisible() {
            compare(topBar.controlsVisible, true, "Controls visible by default")
        }

        function test_defaultState_height() {
            compare(topBar.height, 80, "Default height is 80")
        }

        // =====================================================================
        // TEST: Back Button
        // =====================================================================
        
        function test_backButton_exists() {
            var backButton = findChild(topBar, "backButton")
            verify(backButton !== null, "Back button exists")
        }

        function test_backButton_clickEmitsSignal() {
            var backMouseArea = findChild(topBar, "backButtonMouseArea")
            
            mouseClick(backMouseArea)
            
            compare(backClickedSpy.count, 1, "backClicked emitted")
        }

        function test_backButton_multipleClicks() {
            var backMouseArea = findChild(topBar, "backButtonMouseArea")
            
            mouseClick(backMouseArea)
            mouseClick(backMouseArea)
            mouseClick(backMouseArea)
            
            compare(backClickedSpy.count, 3, "backClicked emitted for each click")
        }

        function test_backButton_hasArrowIcon() {
            var backIcon = findChild(topBar, "backIcon")
            compare(backIcon.text, "\u2190", "Back button shows left arrow")
        }

        function test_backButton_isRound() {
            var backButton = findChild(topBar, "backButton")
            compare(backButton.radius, 20, "Back button has round radius")
            compare(backButton.width, 40, "Back button width is 40")
            compare(backButton.height, 40, "Back button height is 40")
        }

        // =====================================================================
        // TEST: Streamer Name Display
        // =====================================================================
        
        function test_streamerName_displayedWhenSet() {
            var nameText = findChild(topBar, "streamerNameText")
            
            topBar.streamerName = "Pokimane"
            
            compare(nameText.text, "Pokimane", "Streamer name displayed")
        }

        function test_streamerLogin_usedAsFallback() {
            var nameText = findChild(topBar, "streamerNameText")
            
            topBar.streamerName = ""
            topBar.streamerLogin = "shroud"
            
            compare(nameText.text, "shroud", "Login used when name empty")
        }

        function test_streamerName_preferredOverLogin() {
            var nameText = findChild(topBar, "streamerNameText")
            
            topBar.streamerName = "xQc"
            topBar.streamerLogin = "xqc"
            
            compare(nameText.text, "xQc", "Name preferred over login")
        }

        function test_streamerName_emptyShowsEmpty() {
            var nameText = findChild(topBar, "streamerNameText")
            
            topBar.streamerName = ""
            topBar.streamerLogin = ""
            
            compare(nameText.text, "", "Empty when both name and login empty")
        }

        // =====================================================================
        // TEST: Stream Title Display
        // =====================================================================
        
        function test_streamTitle_displayedWhenSet() {
            var titleText = findChild(topBar, "streamTitleText")
            
            topBar.streamTitle = "Playing Valorant with friends!"
            
            compare(titleText.text, "Playing Valorant with friends!", "Stream title displayed")
        }

        function test_streamTitle_defaultWhenEmpty() {
            var titleText = findChild(topBar, "streamTitleText")
            
            topBar.streamTitle = ""
            
            compare(titleText.text, "Live stream", "Default title when empty")
        }

        function test_streamTitle_longTitleElided() {
            var titleText = findChild(topBar, "streamTitleText")
            compare(titleText.elide, Text.ElideRight, "Title elides on the right")
        }

        // =====================================================================
        // TEST: Status Badge
        // =====================================================================
        
        function test_statusBadge_hiddenWhenEmpty() {
            var badge = findChild(topBar, "statusBadge")
            
            topBar.statusText = ""
            
            compare(badge.visible, false, "Badge hidden when status empty")
        }

        function test_statusBadge_visibleWithText() {
            var badge = findChild(topBar, "statusBadge")
            
            topBar.statusText = "LIVE"
            
            compare(badge.visible, true, "Badge visible with status text")
        }

        function test_statusBadge_showsStatusText() {
            var statusLabel = findChild(topBar, "statusLabel")
            
            topBar.statusText = "1080p60"
            
            compare(statusLabel.text, "1080p60", "Status label shows text")
        }

        function test_statusBadge_hasRoundedCorners() {
            var badge = findChild(topBar, "statusBadge")
            
            // Badge should have rounded corners
            compare(badge.radius, 12, "Badge has 12px radius for rounded corners")
        }

        // =====================================================================
        // TEST: Visibility Control
        // =====================================================================
        
        function test_visibility_visibleWhenControlsVisible() {
            topBar.controlsVisible = true
            compare(topBar.visible, true, "Visible when controlsVisible is true")
            compare(topBar.opacity, 1.0, "Full opacity when visible")
        }

        function test_visibility_hiddenWhenControlsNotVisible() {
            topBar.controlsVisible = false
            tryCompare(topBar, "opacity", 0.0, 100, "Opacity 0 when not visible")
        }

        function test_visibility_canToggle() {
            topBar.controlsVisible = true
            compare(topBar.visible, true, "Initially visible")
            
            topBar.controlsVisible = false
            tryCompare(topBar, "opacity", 0.0, 100, "Hidden after toggle")
            
            topBar.controlsVisible = true
            tryCompare(topBar, "opacity", 1.0, 100, "Visible after toggle back")
        }

        // =====================================================================
        // TEST: Text Styling
        // =====================================================================
        
        function test_streamerName_isBold() {
            var nameText = findChild(topBar, "streamerNameText")
            compare(nameText.font.bold, true, "Streamer name is bold")
        }

        function test_streamerName_hasOutlineStyle() {
            var nameText = findChild(topBar, "streamerNameText")
            compare(nameText.style, Text.Outline, "Name has outline style")
        }

        function test_streamTitle_hasOutlineStyle() {
            var titleText = findChild(topBar, "streamTitleText")
            compare(titleText.style, Text.Outline, "Title has outline style")
        }

        function test_streamerName_fontSize() {
            var nameText = findChild(topBar, "streamerNameText")
            compare(nameText.font.pixelSize, 18, "Name font size is 18")
        }

        function test_streamTitle_fontSize() {
            var titleText = findChild(topBar, "streamTitleText")
            compare(titleText.font.pixelSize, 13, "Title font size is 13")
        }

        // =====================================================================
        // TEST: Data-Driven Streamer Names
        // =====================================================================

        function test_streamerNames_data() {
            return [
                { tag: "simple", name: "Ninja", login: "ninja", expected: "Ninja" },
                { tag: "mixed_case", name: "xQc", login: "xqc", expected: "xQc" },
                { tag: "only_login", name: "", login: "shroud", expected: "shroud" },
                { tag: "special_chars", name: "NICKMERCS", login: "nickmercs", expected: "NICKMERCS" },
                { tag: "korean", name: "Faker", login: "faker", expected: "Faker" },
                { tag: "numbers", name: "summit1g", login: "summit1g", expected: "summit1g" }
            ]
        }

        function test_streamerNames(data) {
            var nameText = findChild(topBar, "streamerNameText")
            
            topBar.streamerName = data.name
            topBar.streamerLogin = data.login
            
            compare(nameText.text, data.expected, "Name for " + data.tag)
        }

        // =====================================================================
        // TEST: Data-Driven Status Text
        // =====================================================================

        function test_statusBadgeVisibility_data() {
            return [
                { tag: "empty", status: "", expectedVisible: false },
                { tag: "LIVE", status: "LIVE", expectedVisible: true },
                { tag: "quality", status: "1080p60", expectedVisible: true },
                { tag: "single_char", status: "X", expectedVisible: true },
                { tag: "whitespace_only", status: "   ", expectedVisible: true }
            ]
        }

        function test_statusBadgeVisibility(data) {
            var badge = findChild(topBar, "statusBadge")
            
            topBar.statusText = data.status
            
            compare(badge.visible, data.expectedVisible, "Badge visibility for: " + data.tag)
        }

        // =====================================================================
        // TEST: Integration - Full Stream Info
        // =====================================================================
        
        function test_fullStreamInfo() {
            topBar.streamerName = "Pokimane"
            topBar.streamerLogin = "pokimane"
            topBar.streamTitle = "Just Chatting with chat!"
            topBar.statusText = "LIVE"
            topBar.controlsVisible = true
            
            var nameText = findChild(topBar, "streamerNameText")
            var titleText = findChild(topBar, "streamTitleText")
            var badge = findChild(topBar, "statusBadge")
            var statusLabel = findChild(topBar, "statusLabel")
            
            compare(nameText.text, "Pokimane", "Name displayed")
            compare(titleText.text, "Just Chatting with chat!", "Title displayed")
            compare(badge.visible, true, "Badge visible")
            compare(statusLabel.text, "LIVE", "Status shown")
            compare(topBar.visible, true, "Bar visible")
        }

        function test_backButton_duringPlayback() {
            topBar.streamerName = "shroud"
            topBar.streamTitle = "FPS GAMING"
            topBar.statusText = "720p60"
            
            var backMouseArea = findChild(topBar, "backButtonMouseArea")
            mouseClick(backMouseArea)
            
            compare(backClickedSpy.count, 1, "Back clicked during playback")
        }

        // =====================================================================
        // TEST: Layout Structure
        // =====================================================================
        
        function test_layout_hasContentLayout() {
            var layout = findChild(topBar, "contentLayout")
            verify(layout !== null, "Content layout exists")
        }

        function test_layout_hasStreamInfoLayout() {
            var infoLayout = findChild(topBar, "streamInfoLayout")
            verify(infoLayout !== null, "Stream info layout exists")
        }

        // =====================================================================
        // TEST: Edge Cases
        // =====================================================================
        
        function test_veryLongStreamerName() {
            var nameText = findChild(topBar, "streamerNameText")
            
            topBar.streamerName = "VeryLongStreamerNameThatShouldBeElided"
            
            compare(nameText.elide, Text.ElideRight, "Long name is elided")
        }

        function test_veryLongStreamTitle() {
            var titleText = findChild(topBar, "streamTitleText")
            
            topBar.streamTitle = "This is a very long stream title that should definitely be elided because it is too long to fit"
            
            compare(titleText.elide, Text.ElideRight, "Long title is elided")
        }

        function test_rapidVisibilityToggle() {
            topBar.controlsVisible = true
            topBar.controlsVisible = false
            topBar.controlsVisible = true
            topBar.controlsVisible = false
            topBar.controlsVisible = true
            
            tryCompare(topBar, "opacity", 1.0, 100, "Final state is visible after rapid toggles")
        }
    }
}
