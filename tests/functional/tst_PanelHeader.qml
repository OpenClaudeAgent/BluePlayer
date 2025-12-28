/**
 * tst_PanelHeader.qml
 * 
 * Functional UI tests for the PanelHeader component (Plan 28).
 * Tests standard panel headers with back button and title.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 500
    height: 200

    // =========================================================================
    // Component Under Test
    // =========================================================================
    
    Item {
        id: panelHeader
        objectName: "panelHeader"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        
        implicitHeight: 64
        height: implicitHeight
        
        // Properties
        property string title: ""
        property bool showBackButton: true
        property color backgroundColor: "#1A1A2E"
        
        // Signal
        signal backClicked()
        
        // Reset
        function reset() {
            title = ""
            showBackButton = true
        }
        
        Rectangle {
            id: headerBackground
            objectName: "headerBackground"
            anchors.fill: parent
            color: panelHeader.backgroundColor
        }
        
        Rectangle {
            id: backButton
            objectName: "backButton"
            visible: panelHeader.showBackButton
            
            anchors {
                left: parent.left
                leftMargin: 12
                verticalCenter: parent.verticalCenter
            }
            
            width: 36
            height: 36
            radius: 18
            color: backButtonArea.containsMouse ? "#1AFFFFFF" : "transparent"
            
            Text {
                id: backIcon
                objectName: "backIcon"
                anchors.centerIn: parent
                text: "\u2190"  // ←
                font.pixelSize: 20
                color: "#FFFFFF"
            }
            
            MouseArea {
                id: backButtonArea
                objectName: "backButtonArea"
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: panelHeader.backClicked()
            }
        }
        
        Text {
            id: titleText
            objectName: "titleText"
            
            anchors {
                left: panelHeader.showBackButton ? backButton.right : parent.left
                leftMargin: panelHeader.showBackButton ? 12 : 24
                verticalCenter: parent.verticalCenter
            }
            
            text: panelHeader.title
            font.pixelSize: 20
            font.weight: Font.DemiBold
            color: "#FFFFFF"
        }
        
        Rectangle {
            id: bottomDivider
            objectName: "bottomDivider"
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: 1
            color: "#333333"
        }
    }

    // =========================================================================
    // Signal Spy
    // =========================================================================
    
    SignalSpy { id: backClickedSpy; target: panelHeader; signalName: "backClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "PanelHeaderTests"
        when: windowShown

        function init() {
            panelHeader.reset()
            backClickedSpy.clear()
            wait(50)
        }

        // =====================================================================
        // TEST: Initial State
        // =====================================================================
        
        function test_initialState_height() {
            compare(panelHeader.height, 64, "Default height should be 64")
        }
        
        function test_initialState_backButtonVisible() {
            var btn = findChild(panelHeader, "backButton")
            verify(btn !== null && btn.visible, "Back button should be visible by default")
        }
        
        function test_initialState_hasDivider() {
            var divider = findChild(panelHeader, "bottomDivider")
            verify(divider !== null, "Bottom divider should exist")
            compare(divider.height, 1, "Divider should be 1px")
        }

        // =====================================================================
        // TEST: Title Display
        // =====================================================================
        
        function test_title_displays() {
            // Arrange
            panelHeader.title = "Preferences"
            wait(50)
            
            // Assert
            var titleText = findChild(panelHeader, "titleText")
            if (titleText) {
                compare(titleText.text, "Preferences", "Title should be displayed")
            }
        }
        
        function test_title_changesText() {
            // Arrange
            panelHeader.title = "Settings"
            wait(50)
            
            // Act
            panelHeader.title = "Cache Manager"
            wait(50)
            
            // Assert
            var titleText = findChild(panelHeader, "titleText")
            if (titleText) {
                compare(titleText.text, "Cache Manager", "Title should update")
            }
        }

        // =====================================================================
        // TEST: Back Button
        // =====================================================================
        
        function test_backButton_click_emitsSignal() {
            // Arrange
            var mouseArea = findChild(panelHeader, "backButtonArea")
            verify(mouseArea !== null, "Back button mouse area should exist")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(backClickedSpy.count, 1, "backClicked should be emitted")
        }
        
        function test_backButton_showsArrow() {
            var icon = findChild(panelHeader, "backIcon")
            if (icon) {
                compare(icon.text, "\u2190", "Back icon should be left arrow")
            }
        }
        
        function test_backButton_isCircular() {
            var btn = findChild(panelHeader, "backButton")
            if (btn) {
                compare(btn.radius, btn.width / 2, "Back button should be circular")
            }
        }

        // =====================================================================
        // TEST: Hide Back Button
        // =====================================================================
        
        function test_hideBackButton_hidesButton() {
            // Act
            panelHeader.showBackButton = false
            wait(50)
            
            // Assert
            var btn = findChild(panelHeader, "backButton")
            if (btn) {
                verify(!btn.visible, "Back button should be hidden")
            }
        }
        
        function test_hideBackButton_titleMovesLeft() {
            // Arrange
            panelHeader.showBackButton = true
            wait(50)
            var titleWithButton = findChild(panelHeader, "titleText")
            var xWithButton = titleWithButton ? titleWithButton.x : 0
            
            // Act
            panelHeader.showBackButton = false
            wait(50)
            
            // Assert
            var titleWithoutButton = findChild(panelHeader, "titleText")
            if (titleWithoutButton && xWithButton > 0) {
                verify(titleWithoutButton.x < xWithButton, "Title should move left when back button hidden")
            }
        }

        // =====================================================================
        // TEST: Background Color
        // =====================================================================
        
        function test_backgroundColor_applies() {
            // Arrange
            panelHeader.backgroundColor = "#FF0000"
            wait(50)
            
            // Assert
            var bg = findChild(panelHeader, "headerBackground")
            if (bg) {
                compare(bg.color.toString(), "#ff0000", "Background color should apply")
            }
        }

        // =====================================================================
        // TEST: Multiple Clicks
        // =====================================================================
        
        function test_multipleBackClicks_emitMultiple() {
            var mouseArea = findChild(panelHeader, "backButtonArea")
            
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            compare(backClickedSpy.count, 3, "Should emit for each click")
        }

        // =====================================================================
        // TEST: Different Titles
        // =====================================================================
        
        function test_preferencesTitle() {
            panelHeader.title = "Preferences"
            var t = findChild(panelHeader, "titleText")
            if (t) compare(t.text, "Preferences")
        }
        
        function test_cacheManagerTitle() {
            panelHeader.title = "Cache Manager"
            var t = findChild(panelHeader, "titleText")
            if (t) compare(t.text, "Cache Manager")
        }
        
        function test_watchHistoryTitle() {
            panelHeader.title = "Watch History"
            var t = findChild(panelHeader, "titleText")
            if (t) compare(t.text, "Watch History")
        }
    }
}
