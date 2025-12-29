/**
 * tst_StreamCard.qml
 * 
 * Functional UI tests for the StreamCard component.
 * Tests stream data display, click signal, and LIVE badge.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
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
        readonly property color mutedText: "#6e7681"
        readonly property color accent: "#3b82f6"
        readonly property color surface: "#0d1117"
        readonly property color surfaceSoft: "#161b22"
        readonly property color divider: "#30363d"
        readonly property color statusNegative: "#f85149"
        readonly property color cardHighlight: "#1c2128"
        readonly property int animCardDuration: 1
        readonly property int animContentFadeDuration: 1
        readonly property string fontFamily: "Inter"
    }

    // =========================================================================
    // Component Under Test (Mock of StreamCard extending BaseCard mock)
    // =========================================================================
    
    Component {
        id: streamCardComponent
        
        Item {
            id: cardRoot
            objectName: "streamCard"

            // BaseCard properties
            property bool isPlaceholder: false
            property int cardWidth: 200
            property int cardHeight: 220
            readonly property alias hovered: mouseArea.containsMouse

            // StreamCard specific properties
            property string streamerName: ""
            property string streamTitle: ""
            property string viewerCount: ""
            property string previewImage: ""
            property string streamerLogin: ""

            // Signals
            signal cardClicked()
            signal clicked(string streamerLogin, string streamerName, string streamTitle, string thumbnailUrl)

            implicitWidth: cardWidth
            implicitHeight: cardHeight
            width: cardWidth
            height: cardHeight

            Rectangle {
                id: cardBackground
                objectName: "cardBackground"
                anchors.fill: parent
                radius: 12
                color: cardRoot.isPlaceholder ? mockTheme.surfaceSoft : mockTheme.surface
                border.color: mockTheme.divider
                border.width: 1

                states: [
                    State {
                        name: "hovered"
                        when: mouseArea.containsMouse
                        PropertyChanges {
                            target: cardBackground
                            scale: 1.02
                        }
                    }
                ]

                transitions: Transition {
                    NumberAnimation {
                        properties: "scale"
                        duration: mockTheme.animCardDuration
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    // Preview image area
                    Rectangle {
                        id: previewArea
                        objectName: "previewArea"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120
                        radius: 8
                        color: cardRoot.isPlaceholder ? mockTheme.cardHighlight : mockTheme.surfaceSoft
                        border.color: mockTheme.divider
                        border.width: 1
                        clip: true

                        // Placeholder text
                        Text {
                            id: placeholderText
                            objectName: "placeholderText"
                            anchors.centerIn: parent
                            text: cardRoot.isPlaceholder ? "⋯" : "📺"
                            font.pixelSize: 32
                            color: mockTheme.mutedText
                            opacity: 0.5
                            visible: cardRoot.isPlaceholder || cardRoot.previewImage === ""
                        }

                        // LIVE badge
                        Rectangle {
                            id: liveBadge
                            objectName: "liveBadge"
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.margins: 6
                            width: 40
                            height: 20
                            radius: 10
                            color: mockTheme.statusNegative
                            visible: !cardRoot.isPlaceholder

                            Text {
                                anchors.centerIn: parent
                                text: "LIVE"
                                font.pixelSize: 10
                                font.bold: true
                                color: "#fff"
                            }
                        }
                    }

                    // Stream info
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            id: streamerNameText
                            objectName: "streamerNameText"
                            text: cardRoot.streamerName
                            font.family: mockTheme.fontFamily
                            font.pixelSize: 14
                            font.bold: true
                            color: mockTheme.primaryText
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Text {
                            id: streamTitleText
                            objectName: "streamTitleText"
                            text: cardRoot.streamTitle
                            font.family: mockTheme.fontFamily
                            font.pixelSize: 12
                            color: mockTheme.secondaryText
                            elide: Text.ElideRight
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            Layout.fillWidth: true
                        }

                        Text {
                            id: viewerCountText
                            objectName: "viewerCountText"
                            text: cardRoot.viewerCount
                            font.family: mockTheme.fontFamily
                            font.pixelSize: 11
                            color: mockTheme.accent
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            MouseArea {
                id: mouseArea
                objectName: "mouseArea"
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (!cardRoot.isPlaceholder && cardRoot.streamerLogin) {
                        cardRoot.cardClicked()
                        cardRoot.clicked(cardRoot.streamerLogin, cardRoot.streamerName, 
                                        cardRoot.streamTitle, cardRoot.previewImage)
                    }
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var card: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: cardClickedSpy; signalName: "cardClicked" }
    SignalSpy { id: clickedSpy; signalName: "clicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "StreamCardTests"
        when: windowShown

        function init() {
            card = createTemporaryObject(streamCardComponent, root)
            verify(card !== null, "StreamCard should be created")
            card.anchors.centerIn = root
            cardClickedSpy.target = card
            clickedSpy.target = card
            cardClickedSpy.clear()
            clickedSpy.clear()
            mouseMove(root, 1, 1)
        }

        function cleanup() {
            card = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_notPlaceholder() {
            compare(card.isPlaceholder, false, "Default isPlaceholder should be false")
        }
        
        function test_defaultState_emptyData() {
            compare(card.streamerName, "", "Default streamerName should be empty")
            compare(card.streamTitle, "", "Default streamTitle should be empty")
            compare(card.viewerCount, "", "Default viewerCount should be empty")
            compare(card.streamerLogin, "", "Default streamerLogin should be empty")
        }

        // =====================================================================
        // TEST: Data Display
        // =====================================================================
        
        function test_streamerName_displayed() {
            var nameText = findChild(card, "streamerNameText")
            
            card.streamerName = "Ninja"
            
            compare(nameText.text, "Ninja", "Streamer name should be displayed")
        }
        
        function test_streamTitle_displayed() {
            var titleText = findChild(card, "streamTitleText")
            
            card.streamTitle = "Playing Fortnite with viewers!"
            
            compare(titleText.text, "Playing Fortnite with viewers!", "Stream title should be displayed")
        }
        
        function test_viewerCount_displayed() {
            var viewerText = findChild(card, "viewerCountText")
            
            card.viewerCount = "45.2K viewers"
            
            compare(viewerText.text, "45.2K viewers", "Viewer count should be displayed")
        }

        // =====================================================================
        // TEST: LIVE Badge
        // =====================================================================
        
        function test_liveBadge_visibleWhenNotPlaceholder() {
            var liveBadge = findChild(card, "liveBadge")
            
            compare(liveBadge.visible, true, "LIVE badge should be visible for non-placeholder")
        }
        
        function test_liveBadge_hiddenWhenPlaceholder() {
            var liveBadge = findChild(card, "liveBadge")
            
            card.isPlaceholder = true
            
            compare(liveBadge.visible, false, "LIVE badge should be hidden for placeholder")
        }

        // =====================================================================
        // TEST: Click Signal
        // =====================================================================
        
        function test_click_emitsSignals() {
            var mouseArea = findChild(card, "mouseArea")
            card.streamerLogin = "ninja"
            card.streamerName = "Ninja"
            card.streamTitle = "Test Stream"
            card.previewImage = "http://example.com/thumb.jpg"
            
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 1, "cardClicked should be emitted")
            compare(clickedSpy.count, 1, "clicked should be emitted")
        }
        
        function test_click_passesCorrectArguments() {
            var mouseArea = findChild(card, "mouseArea")
            card.streamerLogin = "shroud"
            card.streamerName = "Shroud"
            card.streamTitle = "CS2 Ranked"
            card.previewImage = "http://example.com/preview.jpg"
            
            mouseClick(mouseArea)
            
            var args = clickedSpy.signalArguments[0]
            compare(args[0], "shroud", "streamerLogin should be passed")
            compare(args[1], "Shroud", "streamerName should be passed")
            compare(args[2], "CS2 Ranked", "streamTitle should be passed")
            compare(args[3], "http://example.com/preview.jpg", "thumbnailUrl should be passed")
        }
        
        function test_click_noSignalWithoutLogin() {
            var mouseArea = findChild(card, "mouseArea")
            card.streamerName = "Test"
            // streamerLogin is empty
            
            mouseClick(mouseArea)
            
            compare(clickedSpy.count, 0, "Should not emit clicked without streamerLogin")
        }
        
        function test_click_noSignalWhenPlaceholder() {
            var mouseArea = findChild(card, "mouseArea")
            card.isPlaceholder = true
            card.streamerLogin = "test"
            
            mouseClick(mouseArea)
            
            compare(clickedSpy.count, 0, "Should not emit clicked for placeholder")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_setsHoveredTrue() {
            var mouseArea = findChild(card, "mouseArea")
            
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(card, "hovered", true, 100, "hovered should be true")
        }

        // =====================================================================
        // TEST: Placeholder Mode
        // =====================================================================
        
        function test_placeholder_showsPlaceholderText() {
            var placeholderText = findChild(card, "placeholderText")
            
            card.isPlaceholder = true
            
            compare(placeholderText.visible, true, "Placeholder text should be visible")
            compare(placeholderText.text, "⋯", "Should show loading indicator")
        }
    }
}
