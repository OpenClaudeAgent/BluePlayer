/**
 * tst_ChannelCard.qml
 * 
 * Functional UI tests for the ChannelCard component.
 * Tests channel data display, click signal, LIVE badge, and game status.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 400

    // =========================================================================
    // Component Under Test (Mock of ChannelCard extending BaseCard mock)
    // =========================================================================
    
    Component {
        id: channelCardComponent
        
        Item {
            id: cardRoot
            objectName: "channelCard"

            // Theme constants (inline)
            readonly property color _primaryText: "#e6edf3"
            readonly property color _secondaryText: "#8b949e"
            readonly property color _mutedText: "#6e7681"
            readonly property color _accent: "#3b82f6"
            readonly property color _surface: "#0d1117"
            readonly property color _surfaceSoft: "#161b22"
            readonly property color _divider: "#30363d"
            readonly property color _statusNegative: "#f85149"
            readonly property color _cardHighlight: "#1a2230"
            readonly property int _animCardDuration: 1
            readonly property int _animContentFadeDuration: 1
            readonly property string _fontFamily: "Inter"

            // BaseCard properties
            property bool isPlaceholder: false
            property int cardWidth: 200
            property int cardHeight: 260
            readonly property alias hovered: mouseArea.containsMouse

            // ChannelCard specific properties
            property string channelName: ""
            property string displayName: ""
            property string thumbnailUrl: ""
            property bool isLive: false
            property string gameName: ""

            // Signals
            signal cardClicked()

            implicitWidth: cardWidth
            implicitHeight: cardHeight
            width: cardWidth
            height: cardHeight

            Rectangle {
                id: cardBackground
                objectName: "cardBackground"
                anchors.fill: parent
                radius: 12
                color: cardRoot.isPlaceholder ? cardRoot._surfaceSoft : cardRoot._surface
                border.color: cardRoot._divider
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
                        duration: cardRoot._animCardDuration
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    // Preview image area
                    Rectangle {
                        id: previewArea
                        objectName: "previewArea"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 200
                        radius: 8
                        color: cardRoot.isPlaceholder ? cardRoot._cardHighlight : cardRoot._surfaceSoft
                        border.color: cardRoot._divider
                        border.width: 1
                        clip: true

                        // Avatar image (mock - always show placeholder in tests)
                        Image {
                            id: avatarImage
                            objectName: "avatarImage"
                            anchors.fill: parent
                            source: cardRoot.isPlaceholder ? "" : cardRoot.thumbnailUrl
                            fillMode: Image.PreserveAspectFit
                            asynchronous: true
                            cache: true
                            visible: status === Image.Ready && !cardRoot.isPlaceholder
                        }

                        // Placeholder/fallback display
                        Rectangle {
                            id: fallbackDisplay
                            objectName: "fallbackDisplay"
                            anchors.fill: parent
                            color: cardRoot.isPlaceholder ? cardRoot._cardHighlight : cardRoot._surfaceSoft
                            visible: avatarImage.status !== Image.Ready || cardRoot.isPlaceholder

                            Text {
                                id: placeholderText
                                objectName: "placeholderText"
                                anchors.centerIn: parent
                                text: cardRoot.isPlaceholder ? "..." : (avatarImage.status === Image.Loading ? "..." : "?")
                                font.pixelSize: 32
                                color: cardRoot._mutedText
                                opacity: 0.5
                            }
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
                            color: cardRoot._statusNegative
                            visible: cardRoot.isLive && !cardRoot.isPlaceholder && avatarImage.status === Image.Ready

                            Text {
                                anchors.centerIn: parent
                                text: "LIVE"
                                font.pixelSize: 10
                                font.bold: true
                                color: "#fff"
                            }
                        }
                    }

                    // Channel name (centered)
                    Text {
                        id: channelNameText
                        objectName: "channelNameText"
                        text: cardRoot.displayName !== "" ? cardRoot.displayName : cardRoot.channelName
                        font.family: cardRoot._fontFamily
                        font.pixelSize: 14
                        font.bold: true
                        color: cardRoot._primaryText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    // Game status (only visible when live with game name)
                    Text {
                        id: gameStatusText
                        objectName: "gameStatusText"
                        text: cardRoot.isLive && cardRoot.gameName !== "" ? cardRoot.gameName : ""
                        font.family: cardRoot._fontFamily
                        font.pixelSize: 11
                        color: cardRoot._accent
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        visible: text !== ""
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
                    if (!cardRoot.isPlaceholder) {
                        cardRoot.cardClicked()
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

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "ChannelCardTests"
        when: windowShown

        function init() {
            card = createTemporaryObject(channelCardComponent, root)
            verify(card !== null, "ChannelCard should be created")
            card.anchors.centerIn = root
            cardClickedSpy.target = card
            cardClickedSpy.clear()
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
            compare(card.channelName, "", "Default channelName should be empty")
            compare(card.displayName, "", "Default displayName should be empty")
            compare(card.thumbnailUrl, "", "Default thumbnailUrl should be empty")
            compare(card.gameName, "", "Default gameName should be empty")
        }

        function test_defaultState_notLive() {
            compare(card.isLive, false, "Default isLive should be false")
        }

        function test_defaultState_dimensions() {
            compare(card.cardHeight, 260, "Default cardHeight should be 260")
        }

        // =====================================================================
        // TEST: Data Display - Channel Name
        // =====================================================================
        
        function test_channelName_displayedWhenNoDisplayName() {
            var nameText = findChild(card, "channelNameText")
            
            card.channelName = "pokimane"
            card.displayName = ""
            
            compare(nameText.text, "pokimane", "channelName should be displayed when displayName is empty")
        }
        
        function test_displayName_preferredOverChannelName() {
            var nameText = findChild(card, "channelNameText")
            
            card.channelName = "pokimane"
            card.displayName = "Pokimane"
            
            compare(nameText.text, "Pokimane", "displayName should be displayed when available")
        }

        function test_displayName_withDifferentChannelName() {
            var nameText = findChild(card, "channelNameText")
            
            card.channelName = "xqc"
            card.displayName = "xQc"
            
            compare(nameText.text, "xQc", "displayName should show proper casing")
        }

        // =====================================================================
        // TEST: Data Display - Game Status
        // =====================================================================
        
        function test_gameStatus_hiddenWhenNotLive() {
            var gameText = findChild(card, "gameStatusText")
            
            card.isLive = false
            card.gameName = "Just Chatting"
            
            compare(gameText.visible, false, "Game status should be hidden when not live")
        }

        function test_gameStatus_hiddenWhenNoGameName() {
            var gameText = findChild(card, "gameStatusText")
            
            card.isLive = true
            card.gameName = ""
            
            compare(gameText.visible, false, "Game status should be hidden when game name is empty")
        }

        function test_gameStatus_visibleWhenLiveWithGame() {
            var gameText = findChild(card, "gameStatusText")
            
            card.isLive = true
            card.gameName = "Valorant"
            
            compare(gameText.visible, true, "Game status should be visible when live with game")
            compare(gameText.text, "Valorant", "Game name should be displayed")
        }

        // =====================================================================
        // TEST: LIVE Badge
        // =====================================================================
        
        function test_liveBadge_hiddenWhenNotLive() {
            var liveBadge = findChild(card, "liveBadge")
            
            card.isLive = false
            
            compare(liveBadge.visible, false, "LIVE badge should be hidden when not live")
        }
        
        function test_liveBadge_hiddenWhenPlaceholder() {
            var liveBadge = findChild(card, "liveBadge")
            
            card.isLive = true
            card.isPlaceholder = true
            
            compare(liveBadge.visible, false, "LIVE badge should be hidden for placeholder")
        }

        // =====================================================================
        // TEST: Click Signal
        // =====================================================================
        
        function test_click_emitsCardClicked() {
            var mouseArea = findChild(card, "mouseArea")
            card.channelName = "testchannel"
            
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 1, "cardClicked should be emitted once")
        }
        
        function test_click_noSignalWhenPlaceholder() {
            var mouseArea = findChild(card, "mouseArea")
            card.isPlaceholder = true
            card.channelName = "testchannel"
            
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 0, "Should not emit cardClicked for placeholder")
        }

        function test_click_multipleClicks() {
            var mouseArea = findChild(card, "mouseArea")
            card.channelName = "testchannel"
            
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 3, "cardClicked should be emitted for each click")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_defaultNotHovered() {
            compare(card.hovered, false, "Default hovered should be false")
        }

        function test_hover_setsHoveredTrue() {
            var mouseArea = findChild(card, "mouseArea")
            
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(card, "hovered", true, 100, "hovered should be true after mouseMove")
        }

        // =====================================================================
        // TEST: Placeholder Mode
        // =====================================================================
        
        function test_placeholder_showsFallbackDisplay() {
            var fallbackDisplay = findChild(card, "fallbackDisplay")
            
            card.isPlaceholder = true
            
            compare(fallbackDisplay.visible, true, "Fallback display should be visible for placeholder")
        }


        function test_placeholder_noInteraction() {
            var mouseArea = findChild(card, "mouseArea")
            card.isPlaceholder = true
            
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 0, "Placeholder should not respond to clicks")
        }

        // =====================================================================
        // TEST: Data-Driven - Various Channel Names
        // =====================================================================

        function test_channelNameFormats_data() {
            return [
                { tag: "lowercase", channelName: "shroud", displayName: "Shroud", expected: "Shroud" },
                { tag: "uppercase", channelName: "LIRIK", displayName: "LIRIK", expected: "LIRIK" },
                { tag: "mixed", channelName: "xqc", displayName: "xQc", expected: "xQc" },
                { tag: "numbers", channelName: "a]", displayName: "A_Seagull", expected: "A_Seagull" },
                { tag: "onlyChannelName", channelName: "teststreamer", displayName: "", expected: "teststreamer" }
            ]
        }

        function test_channelNameFormats(data) {
            var nameText = findChild(card, "channelNameText")
            
            card.channelName = data.channelName
            card.displayName = data.displayName
            
            compare(nameText.text, data.expected, "Name display for " + data.tag)
        }

        // =====================================================================
        // TEST: Data-Driven - Game Status Visibility
        // =====================================================================

        function test_gameStatusVisibility_data() {
            return [
                { tag: "live with game", isLive: true, gameName: "Minecraft", expectedVisible: true },
                { tag: "live no game", isLive: true, gameName: "", expectedVisible: false },
                { tag: "offline with game", isLive: false, gameName: "Fortnite", expectedVisible: false },
                { tag: "offline no game", isLive: false, gameName: "", expectedVisible: false }
            ]
        }

        function test_gameStatusVisibility(data) {
            var gameText = findChild(card, "gameStatusText")
            
            card.isLive = data.isLive
            card.gameName = data.gameName
            
            compare(gameText.visible, data.expectedVisible, "Game status visibility for: " + data.tag)
        }
    }
}
