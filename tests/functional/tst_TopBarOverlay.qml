/**
 * tst_TopBarOverlay.qml
 * 
 * Functional UI tests for the TopBarOverlay component.
 * Tests the top bar with gradient background, back button, stream info and status badge.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 600
    height: 200

    // =========================================================================
    // Component Under Test (Mock)
    // =========================================================================
    
    Rectangle {
        id: topBarOverlay
        objectName: "topBarOverlay"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        
        // Properties
        property string streamerName: ""
        property string streamerLogin: ""
        property string streamTitle: ""
        property string statusText: ""
        property bool controlsVisible: true
        
        // Signal
        signal backClicked()
        
        // Reset function for tests
        function reset() {
            streamerName = ""
            streamerLogin = ""
            streamTitle = ""
            statusText = ""
            controlsVisible = true
            // Note: opacity is bound to controlsVisible, don't overwrite
        }
        
        height: 80
        
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#CC000000" }
            GradientStop { position: 1.0; color: "transparent" }
        }
        
        // Visibility (no animation in mock for deterministic tests)
        opacity: controlsVisible ? 1.0 : 0.0
        visible: opacity > 0
        
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
                    text: "\u2190" // left arrow
                    font.pixelSize: 22
                    color: "#FFFFFF"
                }
                
                MouseArea {
                    id: backButtonMouseArea
                    objectName: "backButtonMouseArea"
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: topBarOverlay.backClicked()
                }
            }
            
            // Stream Info
            ColumnLayout {
                id: streamInfo
                objectName: "streamInfo"
                Layout.fillWidth: true
                spacing: 2
                
                Text {
                    id: streamerNameText
                    objectName: "streamerNameText"
                    text: topBarOverlay.streamerName || topBarOverlay.streamerLogin
                    font.pixelSize: 18
                    font.bold: true
                    color: "#FFFFFF"
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                
                Text {
                    id: streamTitleText
                    objectName: "streamTitleText"
                    text: topBarOverlay.streamTitle || qsTr("Stream en direct")
                    font.pixelSize: 13
                    color: "#DDFFFFFF"
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
                visible: topBarOverlay.statusText.length > 0
                
                Text {
                    id: statusLabel
                    objectName: "statusLabel"
                    anchors.centerIn: parent
                    text: topBarOverlay.statusText
                    font.pixelSize: 11
                    color: "#FFFFFF"
                }
            }
        }
    }

    // =========================================================================
    // Signal Spy
    // =========================================================================
    
    SignalSpy { id: backClickedSpy; target: topBarOverlay; signalName: "backClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "TopBarOverlayTests"
        when: windowShown

        function init() {
            topBarOverlay.reset()
            backClickedSpy.clear()
            wait(50)
        }

        // =====================================================================
        // TEST: Initial State / Default Properties
        // =====================================================================
        
        function test_initialState_height() {
            compare(topBarOverlay.height, 80, "Default height should be 80")
        }
        
        function test_initialState_controlsVisible() {
            verify(topBarOverlay.controlsVisible, "Controls should be visible by default")
        }
        
        function test_initialState_opacity() {
            compare(topBarOverlay.opacity, 1.0, "Opacity should be 1.0 by default")
        }
        
        function test_initialState_emptyStreamerName() {
            compare(topBarOverlay.streamerName, "", "Streamer name should be empty by default")
        }
        
        function test_initialState_emptyStreamerLogin() {
            compare(topBarOverlay.streamerLogin, "", "Streamer login should be empty by default")
        }
        
        function test_initialState_emptyStreamTitle() {
            compare(topBarOverlay.streamTitle, "", "Stream title should be empty by default")
        }
        
        function test_initialState_emptyStatusText() {
            compare(topBarOverlay.statusText, "", "Status text should be empty by default")
        }

        // =====================================================================
        // TEST: Back Button
        // =====================================================================
        
        function test_backButton_exists() {
            var btn = findChild(topBarOverlay, "backButton")
            verify(btn !== null, "Back button should exist")
        }
        
        function test_backButton_dimensions() {
            var btn = findChild(topBarOverlay, "backButton")
            if (btn) {
                compare(btn.Layout.preferredWidth, 40, "Back button width should be 40")
                compare(btn.Layout.preferredHeight, 40, "Back button height should be 40")
            }
        }
        
        function test_backButton_isCircular() {
            var btn = findChild(topBarOverlay, "backButton")
            if (btn) {
                compare(btn.radius, 20, "Back button should be circular (radius = 20)")
            }
        }
        
        function test_backButton_showsArrow() {
            var icon = findChild(topBarOverlay, "backIcon")
            if (icon) {
                compare(icon.text, "\u2190", "Back icon should be left arrow")
            }
        }
        
        function test_backButton_click_emitsSignal() {
            var mouseArea = findChild(topBarOverlay, "backButtonMouseArea")
            verify(mouseArea !== null, "Back button mouse area should exist")
            
            mouseClick(mouseArea)
            
            compare(backClickedSpy.count, 1, "backClicked should be emitted once")
        }
        
        function test_backButton_multipleClicks_emitMultiple() {
            var mouseArea = findChild(topBarOverlay, "backButtonMouseArea")
            
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            compare(backClickedSpy.count, 3, "Should emit for each click")
        }

        // =====================================================================
        // TEST: Streamer Name Display
        // =====================================================================
        
        function test_streamerName_displays() {
            topBarOverlay.streamerName = "Ninja"
            wait(50)
            
            var nameText = findChild(topBarOverlay, "streamerNameText")
            if (nameText) {
                compare(nameText.text, "Ninja", "Streamer name should be displayed")
            }
        }
        
        function test_streamerName_fallbackToLogin() {
            // When streamerName is empty, should use streamerLogin
            topBarOverlay.streamerName = ""
            topBarOverlay.streamerLogin = "ninja_official"
            wait(50)
            
            var nameText = findChild(topBarOverlay, "streamerNameText")
            if (nameText) {
                compare(nameText.text, "ninja_official", "Should fallback to streamerLogin")
            }
        }
        
        function test_streamerName_prefersNameOverLogin() {
            topBarOverlay.streamerName = "Ninja"
            topBarOverlay.streamerLogin = "ninja_official"
            wait(50)
            
            var nameText = findChild(topBarOverlay, "streamerNameText")
            if (nameText) {
                compare(nameText.text, "Ninja", "Should prefer streamerName over streamerLogin")
            }
        }
        
        function test_streamerName_updates() {
            topBarOverlay.streamerName = "Streamer1"
            wait(50)
            topBarOverlay.streamerName = "Streamer2"
            wait(50)
            
            var nameText = findChild(topBarOverlay, "streamerNameText")
            if (nameText) {
                compare(nameText.text, "Streamer2", "Streamer name should update")
            }
        }

        // =====================================================================
        // TEST: Stream Title Display
        // =====================================================================
        
        function test_streamTitle_displays() {
            topBarOverlay.streamTitle = "Playing Fortnite!"
            wait(50)
            
            var titleText = findChild(topBarOverlay, "streamTitleText")
            if (titleText) {
                compare(titleText.text, "Playing Fortnite!", "Stream title should be displayed")
            }
        }
        
        function test_streamTitle_fallbackToDefault() {
            topBarOverlay.streamTitle = ""
            wait(50)
            
            var titleText = findChild(topBarOverlay, "streamTitleText")
            if (titleText) {
                compare(titleText.text, "Stream en direct", "Should show default title when empty")
            }
        }
        
        function test_streamTitle_updates() {
            topBarOverlay.streamTitle = "Morning Stream"
            wait(50)
            topBarOverlay.streamTitle = "Evening Stream"
            wait(50)
            
            var titleText = findChild(topBarOverlay, "streamTitleText")
            if (titleText) {
                compare(titleText.text, "Evening Stream", "Stream title should update")
            }
        }

        // =====================================================================
        // TEST: Status Badge
        // =====================================================================
        
        function test_statusBadge_hiddenWhenEmpty() {
            topBarOverlay.statusText = ""
            wait(50)
            
            var badge = findChild(topBarOverlay, "statusBadge")
            if (badge) {
                verify(!badge.visible, "Status badge should be hidden when text is empty")
            }
        }
        
        function test_statusBadge_visibleWhenHasText() {
            topBarOverlay.statusText = "LIVE"
            wait(50)
            
            var badge = findChild(topBarOverlay, "statusBadge")
            if (badge) {
                verify(badge.visible, "Status badge should be visible when has text")
            }
        }
        
        function test_statusBadge_displaysText() {
            topBarOverlay.statusText = "LIVE"
            wait(50)
            
            var label = findChild(topBarOverlay, "statusLabel")
            if (label) {
                compare(label.text, "LIVE", "Status label should display the text")
            }
        }
        
        function test_statusBadge_updates() {
            topBarOverlay.statusText = "LIVE"
            wait(50)
            topBarOverlay.statusText = "1080p60"
            wait(50)
            
            var label = findChild(topBarOverlay, "statusLabel")
            if (label) {
                compare(label.text, "1080p60", "Status text should update")
            }
        }
        
        function test_statusBadge_isRounded() {
            topBarOverlay.statusText = "LIVE"
            wait(50)
            
            var badge = findChild(topBarOverlay, "statusBadge")
            if (badge) {
                compare(badge.radius, 12, "Status badge should have rounded corners")
            }
        }

        // =====================================================================
        // TEST: Controls Visibility
        // =====================================================================
        
        function test_controlsVisible_true_isVisible() {
            topBarOverlay.controlsVisible = true
            wait(50)
            
            verify(topBarOverlay.visible, "TopBar should be visible when controlsVisible is true")
            compare(topBarOverlay.opacity, 1.0, "Opacity should be 1.0")
        }
        
        function test_controlsVisible_false_isHidden() {
            topBarOverlay.controlsVisible = false
            wait(50)
            
            compare(topBarOverlay.opacity, 0.0, "Opacity should be 0.0 when controlsVisible is false")
        }
        
        function test_controlsVisible_toggle() {
            // Start visible
            topBarOverlay.controlsVisible = true
            wait(50)
            compare(topBarOverlay.opacity, 1.0, "Should start visible")
            
            // Hide
            topBarOverlay.controlsVisible = false
            wait(50)
            compare(topBarOverlay.opacity, 0.0, "Should be hidden after toggle")
            
            // Show again
            topBarOverlay.controlsVisible = true
            wait(50)
            compare(topBarOverlay.opacity, 1.0, "Should be visible again")
        }

        // =====================================================================
        // TEST: Layout Structure
        // =====================================================================
        
        function test_layout_hasRowLayout() {
            var layout = findChild(topBarOverlay, "contentLayout")
            verify(layout !== null, "Content layout should exist")
        }
        
        function test_layout_margins() {
            var layout = findChild(topBarOverlay, "contentLayout")
            if (layout) {
                compare(layout.anchors.leftMargin, 24, "Left margin should be 24")
                compare(layout.anchors.rightMargin, 24, "Right margin should be 24")
            }
        }
        
        function test_layout_spacing() {
            var layout = findChild(topBarOverlay, "contentLayout")
            if (layout) {
                compare(layout.spacing, 16, "Spacing should be 16")
            }
        }

        // =====================================================================
        // TEST: Text Styling
        // =====================================================================
        
        function test_streamerName_isBold() {
            var nameText = findChild(topBarOverlay, "streamerNameText")
            if (nameText) {
                verify(nameText.font.bold, "Streamer name should be bold")
            }
        }
        
        function test_streamerName_fontSize() {
            var nameText = findChild(topBarOverlay, "streamerNameText")
            if (nameText) {
                compare(nameText.font.pixelSize, 18, "Streamer name font size should be 18")
            }
        }
        
        function test_streamTitle_fontSize() {
            var titleText = findChild(topBarOverlay, "streamTitleText")
            if (titleText) {
                compare(titleText.font.pixelSize, 13, "Stream title font size should be 13")
            }
        }
        
        function test_backIcon_fontSize() {
            var icon = findChild(topBarOverlay, "backIcon")
            if (icon) {
                compare(icon.font.pixelSize, 22, "Back icon font size should be 22")
            }
        }

        // =====================================================================
        // TEST: Real-world Scenarios
        // =====================================================================
        
        function test_scenario_liveStream() {
            topBarOverlay.streamerName = "Pokimane"
            topBarOverlay.streamerLogin = "pokimane"
            topBarOverlay.streamTitle = "Just Chatting with viewers!"
            topBarOverlay.statusText = "LIVE"
            wait(50)
            
            var nameText = findChild(topBarOverlay, "streamerNameText")
            var titleText = findChild(topBarOverlay, "streamTitleText")
            var badge = findChild(topBarOverlay, "statusBadge")
            
            if (nameText) compare(nameText.text, "Pokimane")
            if (titleText) compare(titleText.text, "Just Chatting with viewers!")
            if (badge) verify(badge.visible, "Live badge should be visible")
        }
        
        function test_scenario_vodPlayback() {
            topBarOverlay.streamerName = "xQc"
            topBarOverlay.streamerLogin = "xqcow"
            topBarOverlay.streamTitle = "PAST BROADCAST - Epic Gaming Session"
            topBarOverlay.statusText = "VOD"
            wait(50)
            
            var nameText = findChild(topBarOverlay, "streamerNameText")
            var titleText = findChild(topBarOverlay, "streamTitleText")
            var label = findChild(topBarOverlay, "statusLabel")
            
            if (nameText) compare(nameText.text, "xQc")
            if (titleText) compare(titleText.text, "PAST BROADCAST - Epic Gaming Session")
            if (label) compare(label.text, "VOD")
        }
        
        function test_scenario_minimalInfo() {
            // Only login available, no title, no status
            topBarOverlay.streamerName = ""
            topBarOverlay.streamerLogin = "unknownstreamer123"
            topBarOverlay.streamTitle = ""
            topBarOverlay.statusText = ""
            wait(50)
            
            var nameText = findChild(topBarOverlay, "streamerNameText")
            var titleText = findChild(topBarOverlay, "streamTitleText")
            var badge = findChild(topBarOverlay, "statusBadge")
            
            if (nameText) compare(nameText.text, "unknownstreamer123", "Should show login as fallback")
            if (titleText) compare(titleText.text, "Stream en direct", "Should show default title")
            if (badge) verify(!badge.visible, "Badge should be hidden with no status")
        }
    }
}
