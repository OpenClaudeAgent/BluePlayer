/**
 * tst_ChannelCard.qml
 * 
 * Functional UI tests for the ChannelCard component.
 * Tests channel display cards showing channel info, live status badge,
 * profile image, hover effects, and click interaction.
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
    // Mock Theme Constants
    // =========================================================================
    
    QtObject {
        id: mockTheme
        readonly property color surface: "#0f1419"
        readonly property color surfaceSoft: "#1a2230"
        readonly property color primaryText: "#FFFFFF"
        readonly property color mutedText: "#8899A6"
        readonly property color accent: "#0066FF"
        readonly property color divider: "#2F3336"
        readonly property color statusNegative: "#E91916"
        readonly property string fontFamily: "Inter"
        readonly property int animCardDuration: 150
        readonly property int animContentFadeDuration: 200
    }

    // =========================================================================
    // Component Under Test (Mock) - Combines BaseCard + ChannelCard
    // =========================================================================
    
    Item {
        id: channelCard
        objectName: "channelCard"
        anchors.centerIn: parent
        
        // BaseCard properties
        property bool isPlaceholder: false
        property int cardWidth: 180
        property int cardHeight: 260
        property int cardRadius: 12
        property int contentMargins: 12
        
        // ChannelCard specific properties
        property string channelName: ""
        property string displayName: ""
        property string thumbnailUrl: ""
        property bool isLive: false
        property string gameName: ""
        
        // Exposed state
        readonly property bool hovered: mouseArea.containsMouse
        
        // Signal
        signal cardClicked()
        
        implicitWidth: cardWidth
        implicitHeight: cardHeight
        width: cardWidth
        height: cardHeight
        
        Rectangle {
            id: cardBackground
            objectName: "cardBackground"
            anchors.fill: parent
            radius: channelCard.cardRadius
            color: channelCard.isPlaceholder ? mockTheme.surfaceSoft : mockTheme.surface
            border.color: mockTheme.divider
            border.width: 1
            
            scale: mouseArea.containsMouse ? 1.02 : 1.0
            Behavior on scale {
                NumberAnimation { duration: mockTheme.animCardDuration; easing.type: Easing.OutCubic }
            }
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: channelCard.contentMargins
                spacing: 12
                
                // Image preview zone
                Rectangle {
                    id: imageContainer
                    objectName: "imageContainer"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    radius: 8
                    color: channelCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                    border.color: mockTheme.divider
                    border.width: 1
                    clip: true
                    
                    Image {
                        id: avatarImage
                        objectName: "avatarImage"
                        anchors.fill: parent
                        source: channelCard.isPlaceholder ? "" : channelCard.thumbnailUrl
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        cache: true
                        visible: status === Image.Ready && !channelCard.isPlaceholder
                        
                        opacity: status === Image.Ready ? 1 : 0
                        Behavior on opacity {
                            NumberAnimation { duration: mockTheme.animContentFadeDuration; easing.type: Easing.OutCubic }
                        }
                    }
                    
                    // Placeholder/Loading indicator
                    Rectangle {
                        id: placeholderRect
                        objectName: "placeholderRect"
                        anchors.fill: parent
                        color: channelCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                        visible: avatarImage.status !== Image.Ready || channelCard.isPlaceholder
                        
                        Text {
                            id: placeholderIcon
                            objectName: "placeholderIcon"
                            anchors.centerIn: parent
                            text: channelCard.isPlaceholder ? "\u22EF" : (avatarImage.status === Image.Loading ? "\u23F3" : "\uD83D\uDC64")
                            font.pixelSize: 32
                            color: mockTheme.mutedText
                            opacity: 0.5
                        }
                    }
                    
                    // LIVE Badge
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
                        visible: channelCard.isLive && !channelCard.isPlaceholder && avatarImage.status === Image.Ready
                        
                        Text {
                            id: liveBadgeText
                            objectName: "liveBadgeText"
                            anchors.centerIn: parent
                            text: "LIVE"
                            font.pixelSize: 10
                            font.bold: true
                            color: "#FFFFFF"
                        }
                    }
                }
                
                // Channel name (centered)
                Text {
                    id: channelNameText
                    objectName: "channelNameText"
                    text: channelCard.displayName !== "" ? channelCard.displayName : channelCard.channelName
                    font.family: mockTheme.fontFamily
                    font.pixelSize: 14
                    font.bold: true
                    color: mockTheme.primaryText
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }
                
                // Game name (only visible when live with game)
                Text {
                    id: gameNameText
                    objectName: "gameNameText"
                    text: channelCard.isLive && channelCard.gameName !== "" ? channelCard.gameName : ""
                    font.family: mockTheme.fontFamily
                    font.pixelSize: 11
                    color: mockTheme.accent
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
                if (!channelCard.isPlaceholder) {
                    channelCard.cardClicked()
                }
            }
        }
        
        // Reset function for tests
        function reset() {
            isPlaceholder = false
            channelName = ""
            displayName = ""
            thumbnailUrl = ""
            isLive = false
            gameName = ""
            cardWidth = 180
            cardHeight = 260
        }
    }

    // =========================================================================
    // Signal Spy
    // =========================================================================
    
    SignalSpy { id: cardClickedSpy; target: channelCard; signalName: "cardClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "ChannelCardTests"
        when: windowShown

        function init() {
            channelCard.reset()
            cardClickedSpy.clear()
            // Move mouse away from component to reset hover state
            mouseMove(root, 1, 1)
            wait(200)
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultState_dimensions() {
            compare(channelCard.cardWidth, 180, "Default cardWidth should be 180")
            compare(channelCard.cardHeight, 260, "Default cardHeight should be 260")
        }
        
        function test_defaultState_width() {
            compare(channelCard.width, 180, "Width should match cardWidth")
        }
        
        function test_defaultState_height() {
            compare(channelCard.height, 260, "Height should match cardHeight")
        }
        
        function test_defaultState_channelNameEmpty() {
            compare(channelCard.channelName, "", "Default channelName should be empty")
        }
        
        function test_defaultState_displayNameEmpty() {
            compare(channelCard.displayName, "", "Default displayName should be empty")
        }
        
        function test_defaultState_thumbnailUrlEmpty() {
            compare(channelCard.thumbnailUrl, "", "Default thumbnailUrl should be empty")
        }
        
        function test_defaultState_notLive() {
            compare(channelCard.isLive, false, "Should not be live by default")
        }
        
        function test_defaultState_gameNameEmpty() {
            compare(channelCard.gameName, "", "Default gameName should be empty")
        }
        
        function test_defaultState_notPlaceholder() {
            compare(channelCard.isPlaceholder, false, "Should not be placeholder by default")
        }
        
        function test_defaultState_cardRadius() {
            compare(channelCard.cardRadius, 12, "Default cardRadius should be 12")
        }

        // =====================================================================
        // TEST: Signal cardClicked
        // =====================================================================
        
        function test_click_emitsSignal() {
            // Arrange
            var mouseArea = findChild(channelCard, "mouseArea")
            verify(mouseArea !== null, "MouseArea should exist")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 1, "cardClicked signal should be emitted once")
        }
        
        function test_multipleClicks_emitMultiple() {
            // Arrange
            var mouseArea = findChild(channelCard, "mouseArea")
            
            // Act
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 3, "Should emit cardClicked for each click")
        }
        
        function test_clickOnCard_emitsSignal() {
            // Act - click directly on the card
            mouseClick(channelCard)
            
            // Assert
            compare(cardClickedSpy.count, 1, "Clicking on card should emit cardClicked")
        }

        // =====================================================================
        // TEST: Placeholder State
        // =====================================================================
        
        function test_placeholder_noClickSignal() {
            // Arrange
            channelCard.isPlaceholder = true
            wait(50)
            
            // Act
            mouseClick(channelCard)
            
            // Assert
            compare(cardClickedSpy.count, 0, "Placeholder card should not emit click signal")
        }
        
        function test_placeholder_showsPlaceholderRect() {
            // Arrange
            channelCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var placeholderRect = findChild(channelCard, "placeholderRect")
            verify(placeholderRect !== null, "Placeholder rect should exist")
            verify(placeholderRect.visible, "Placeholder rect should be visible")
        }
        
        function test_placeholder_hidesLiveBadge() {
            // Arrange
            channelCard.isLive = true
            channelCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var liveBadge = findChild(channelCard, "liveBadge")
            verify(!liveBadge.visible, "Live badge should be hidden for placeholder")
        }
        
        function test_placeholder_showsEllipsisIcon() {
            // Arrange
            channelCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var icon = findChild(channelCard, "placeholderIcon")
            verify(icon !== null, "Placeholder icon should exist")
            compare(icon.text, "\u22EF", "Should show ellipsis icon for placeholder")
        }

        // =====================================================================
        // TEST: Live Badge
        // =====================================================================
        
        function test_liveBadge_hiddenWhenNotLive() {
            // Arrange
            channelCard.isLive = false
            wait(50)
            
            // Assert
            var liveBadge = findChild(channelCard, "liveBadge")
            verify(!liveBadge.visible, "Live badge should be hidden when not live")
        }
        
        function test_liveBadge_hasCorrectText() {
            // Assert
            var liveBadgeText = findChild(channelCard, "liveBadgeText")
            verify(liveBadgeText !== null, "Live badge text should exist")
            compare(liveBadgeText.text, "LIVE", "Badge should display 'LIVE'")
        }
        
        function test_liveBadge_hasCorrectColor() {
            // Assert
            var liveBadge = findChild(channelCard, "liveBadge")
            compare(liveBadge.color.toString(), mockTheme.statusNegative.toString(), "Badge should use statusNegative color")
        }
        
        function test_liveBadge_dimensions() {
            // Assert
            var liveBadge = findChild(channelCard, "liveBadge")
            compare(liveBadge.width, 40, "Badge width should be 40")
            compare(liveBadge.height, 20, "Badge height should be 20")
            compare(liveBadge.radius, 10, "Badge should be pill-shaped (radius 10)")
        }
        
        function test_liveBadge_positioning() {
            // Assert
            var liveBadge = findChild(channelCard, "liveBadge")
            var imageContainer = findChild(channelCard, "imageContainer")
            
            // Badge should be positioned at top-right with margins
            compare(liveBadge.anchors.margins, 6, "Badge margins should be 6")
        }

        // =====================================================================
        // TEST: Channel Name Display
        // =====================================================================
        
        function test_channelName_displayed() {
            // Arrange
            channelCard.channelName = "testchannel"
            wait(50)
            
            // Assert
            var nameText = findChild(channelCard, "channelNameText")
            compare(nameText.text, "testchannel", "Channel name should be displayed")
        }
        
        function test_displayName_priorityOverChannelName() {
            // Arrange
            channelCard.channelName = "testchannel"
            channelCard.displayName = "Test Channel Display"
            wait(50)
            
            // Assert
            var nameText = findChild(channelCard, "channelNameText")
            compare(nameText.text, "Test Channel Display", "Display name should take priority")
        }
        
        function test_channelName_usedWhenDisplayNameEmpty() {
            // Arrange
            channelCard.channelName = "fallback_name"
            channelCard.displayName = ""
            wait(50)
            
            // Assert
            var nameText = findChild(channelCard, "channelNameText")
            compare(nameText.text, "fallback_name", "Should fall back to channelName")
        }
        
        function test_channelName_isBold() {
            // Assert
            var nameText = findChild(channelCard, "channelNameText")
            compare(nameText.font.bold, true, "Channel name should be bold")
        }
        
        function test_channelName_isCentered() {
            // Assert
            var nameText = findChild(channelCard, "channelNameText")
            compare(nameText.horizontalAlignment, Text.AlignHCenter, "Channel name should be centered")
        }
        
        function test_channelName_elidesLongText() {
            // Assert
            var nameText = findChild(channelCard, "channelNameText")
            compare(nameText.elide, Text.ElideRight, "Long names should elide on the right")
        }

        // =====================================================================
        // TEST: Game Name Display
        // =====================================================================
        
        function test_gameName_hiddenWhenNotLive() {
            // Arrange
            channelCard.isLive = false
            channelCard.gameName = "Some Game"
            wait(50)
            
            // Assert
            var gameText = findChild(channelCard, "gameNameText")
            verify(!gameText.visible, "Game name should be hidden when not live")
        }
        
        function test_gameName_hiddenWhenEmpty() {
            // Arrange
            channelCard.isLive = true
            channelCard.gameName = ""
            wait(50)
            
            // Assert
            var gameText = findChild(channelCard, "gameNameText")
            verify(!gameText.visible, "Game name should be hidden when empty")
        }
        
        function test_gameName_visibleWhenLiveWithGame() {
            // Arrange
            channelCard.isLive = true
            channelCard.gameName = "Just Chatting"
            wait(50)
            
            // Assert
            var gameText = findChild(channelCard, "gameNameText")
            verify(gameText.visible, "Game name should be visible when live with game")
            compare(gameText.text, "Just Chatting", "Game name should be displayed")
        }
        
        function test_gameName_usesAccentColor() {
            // Assert
            var gameText = findChild(channelCard, "gameNameText")
            compare(gameText.color.toString(), mockTheme.accent.toString(), "Game name should use accent color")
        }
        
        function test_gameName_isCentered() {
            // Assert
            var gameText = findChild(channelCard, "gameNameText")
            compare(gameText.horizontalAlignment, Text.AlignHCenter, "Game name should be centered")
        }
        
        function test_gameName_smallerFontSize() {
            // Assert
            var nameText = findChild(channelCard, "channelNameText")
            var gameText = findChild(channelCard, "gameNameText")
            verify(gameText.font.pixelSize < nameText.font.pixelSize, "Game name font should be smaller than channel name")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_changesHoveredProperty() {
            // Arrange
            var mouseArea = findChild(channelCard, "mouseArea")
            verify(mouseArea !== null)
            
            // Act - move mouse into the card
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert
            compare(channelCard.hovered, true, "hovered should be true when mouse is over")
        }
        
        function test_hover_increasesScale() {
            // Arrange
            var mouseArea = findChild(channelCard, "mouseArea")
            var cardBackground = findChild(channelCard, "cardBackground")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200) // Wait for animation
            
            // Assert
            compare(cardBackground.scale, 1.02, "Scale should be 1.02 on hover")
        }
        
        function test_hover_restoresOnExit() {
            // Arrange
            var mouseArea = findChild(channelCard, "mouseArea")
            var cardBackground = findChild(channelCard, "cardBackground")
            
            // Act - hover then leave
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200)
            mouseMove(root, 1, 1)
            wait(200)
            
            // Assert
            compare(channelCard.hovered, false, "hovered should be false after mouse leaves")
            compare(cardBackground.scale, 1.0, "Scale should return to 1.0")
        }

        // =====================================================================
        // TEST: MouseArea Configuration
        // =====================================================================
        
        function test_mouseArea_hoverEnabled() {
            var mouseArea = findChild(channelCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.hoverEnabled, true, "Hover should be enabled")
        }
        
        function test_mouseArea_cursorShape() {
            var mouseArea = findChild(channelCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.cursorShape, Qt.PointingHandCursor, "Cursor should be pointing hand")
        }
        
        function test_mouseArea_fillsParent() {
            var mouseArea = findChild(channelCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.width, channelCard.width, "MouseArea should fill card width")
            compare(mouseArea.height, channelCard.height, "MouseArea should fill card height")
        }

        // =====================================================================
        // TEST: Image Container
        // =====================================================================
        
        function test_imageContainer_exists() {
            var container = findChild(channelCard, "imageContainer")
            verify(container !== null, "Image container should exist")
        }
        
        function test_imageContainer_height() {
            var container = findChild(channelCard, "imageContainer")
            compare(container.Layout.preferredHeight, 200, "Image container height should be 200")
        }
        
        function test_imageContainer_hasRoundedCorners() {
            var container = findChild(channelCard, "imageContainer")
            compare(container.radius, 8, "Image container should have radius 8")
        }
        
        function test_imageContainer_clipsContent() {
            var container = findChild(channelCard, "imageContainer")
            compare(container.clip, true, "Image container should clip content")
        }

        // =====================================================================
        // TEST: Card Background
        // =====================================================================
        
        function test_cardBackground_usesCardRadius() {
            var bg = findChild(channelCard, "cardBackground")
            compare(bg.radius, channelCard.cardRadius, "Background should use cardRadius")
        }
        
        function test_cardBackground_hasBorder() {
            var bg = findChild(channelCard, "cardBackground")
            compare(bg.border.width, 1, "Background should have 1px border")
        }
        
        function test_cardBackground_normalColor() {
            // Arrange
            channelCard.isPlaceholder = false
            wait(50)
            
            var bg = findChild(channelCard, "cardBackground")
            compare(bg.color.toString(), mockTheme.surface.toString(), "Normal card should use surface color")
        }
        
        function test_cardBackground_placeholderColor() {
            // Arrange
            channelCard.isPlaceholder = true
            wait(50)
            
            var bg = findChild(channelCard, "cardBackground")
            compare(bg.color.toString(), mockTheme.surfaceSoft.toString(), "Placeholder should use surfaceSoft color")
        }

        // =====================================================================
        // TEST: Avatar Image
        // =====================================================================
        
        function test_avatarImage_asynchronous() {
            var image = findChild(channelCard, "avatarImage")
            compare(image.asynchronous, true, "Image should load asynchronously")
        }
        
        function test_avatarImage_cached() {
            var image = findChild(channelCard, "avatarImage")
            compare(image.cache, true, "Image should be cached")
        }
        
        function test_avatarImage_fillMode() {
            var image = findChild(channelCard, "avatarImage")
            compare(image.fillMode, Image.PreserveAspectFit, "Image should preserve aspect ratio")
        }
        
        function test_avatarImage_emptyForPlaceholder() {
            // Arrange
            channelCard.isPlaceholder = true
            channelCard.thumbnailUrl = "http://example.com/image.jpg"
            wait(50)
            
            // Assert
            var image = findChild(channelCard, "avatarImage")
            compare(image.source, "", "Placeholder should have empty image source")
        }

        // =====================================================================
        // TEST: Custom Dimensions
        // =====================================================================
        
        function test_customWidth_applies() {
            // Arrange
            channelCard.cardWidth = 220
            wait(50)
            
            // Assert
            compare(channelCard.width, 220, "Custom width should apply")
        }
        
        function test_customHeight_applies() {
            // Arrange
            channelCard.cardHeight = 300
            wait(50)
            
            // Assert
            compare(channelCard.height, 300, "Custom height should apply")
        }
    }
}
