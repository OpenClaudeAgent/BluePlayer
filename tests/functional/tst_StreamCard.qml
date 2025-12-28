/**
 * tst_StreamCard.qml
 * 
 * Functional UI tests for the StreamCard component.
 * Tests stream cards displaying streamer info, preview image,
 * viewer count, and LIVE badge with hover effects.
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
        readonly property color surface: "#0d1117"
        readonly property color surfaceSoft: "#161b22"
        readonly property color divider: "#30363d"
        readonly property color primaryText: "#f0f6fc"
        readonly property color secondaryText: "#8b949e"
        readonly property color mutedText: "#484f58"
        readonly property color accent: "#58a6ff"
        readonly property color statusNegative: "#f85149"
        readonly property string fontFamily: "Inter"
        readonly property int animCardDuration: 200
        readonly property int animContentFadeDuration: 300
    }

    // =========================================================================
    // Component Under Test (Mock of StreamCard with BaseCard behavior)
    // =========================================================================
    
    Item {
        id: streamCard
        objectName: "streamCard"
        anchors.centerIn: parent

        // StreamCard properties
        property string streamerName: ""
        property string streamTitle: ""
        property string viewerCount: ""
        property string previewImage: ""
        property string streamerLogin: ""

        // BaseCard properties
        property bool isPlaceholder: false
        property int cardWidth: 180
        property int cardHeight: 220
        property int cardRadius: 12
        property int contentMargins: 12

        // Expose hover state
        readonly property bool hovered: mouseArea.containsMouse

        // Signals
        signal clicked(string streamerLogin, string streamerName, string streamTitle, string thumbnailUrl)
        signal cardClicked()

        implicitWidth: cardWidth
        implicitHeight: cardHeight
        width: cardWidth
        height: cardHeight

        // Card background (from BaseCard)
        Rectangle {
            id: cardBackground
            objectName: "cardBackground"
            anchors.fill: parent
            radius: streamCard.cardRadius
            color: streamCard.isPlaceholder ? mockTheme.surfaceSoft : mockTheme.surface
            border.color: mockTheme.divider
            border.width: 1
            scale: mouseArea.containsMouse ? 1.02 : 1.0

            Behavior on scale {
                NumberAnimation { duration: mockTheme.animCardDuration; easing.type: Easing.OutCubic }
            }
            Behavior on color {
                ColorAnimation { duration: mockTheme.animCardDuration; easing.type: Easing.OutCubic }
            }

            // Card content
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: streamCard.contentMargins
                spacing: 12

                // Preview image zone
                Rectangle {
                    id: previewZone
                    objectName: "previewZone"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    radius: 8
                    color: streamCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                    border.color: mockTheme.divider
                    border.width: 1
                    clip: true

                    // Thumbnail image
                    Image {
                        id: thumbnailImage
                        objectName: "thumbnailImage"
                        anchors.fill: parent
                        source: streamCard.isPlaceholder ? "" : streamCard.previewImage
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        cache: true
                        visible: status === Image.Ready && !streamCard.isPlaceholder

                        opacity: status === Image.Ready ? 1 : 0
                        Behavior on opacity {
                            NumberAnimation { duration: mockTheme.animContentFadeDuration; easing.type: Easing.OutCubic }
                        }
                    }

                    // Placeholder while loading
                    Rectangle {
                        id: placeholderRect
                        objectName: "placeholderRect"
                        anchors.fill: parent
                        color: streamCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                        visible: thumbnailImage.status !== Image.Ready || streamCard.isPlaceholder

                        Text {
                            id: placeholderIcon
                            objectName: "placeholderIcon"
                            anchors.centerIn: parent
                            text: streamCard.isPlaceholder ? "\u22EF" : (thumbnailImage.status === Image.Loading ? "\u23F3" : "\uD83D\uDCFA")
                            font.pixelSize: 32
                            color: mockTheme.mutedText
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
                        color: mockTheme.statusNegative
                        visible: !streamCard.isPlaceholder && thumbnailImage.status === Image.Ready

                        Text {
                            id: liveBadgeText
                            objectName: "liveBadgeText"
                            anchors.centerIn: parent
                            text: "LIVE"
                            font.pixelSize: 10
                            font.bold: true
                            color: "#ffffff"
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
                        text: streamCard.streamerName
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
                        text: streamCard.streamTitle
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
                        text: streamCard.viewerCount
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
                if (!streamCard.isPlaceholder) {
                    streamCard.cardClicked()
                    if (streamCard.streamerLogin) {
                        streamCard.clicked(streamCard.streamerLogin, streamCard.streamerName, streamCard.streamTitle, streamCard.previewImage)
                    }
                }
            }
        }

        // Reset function for tests
        function reset() {
            streamerName = ""
            streamTitle = ""
            viewerCount = ""
            previewImage = ""
            streamerLogin = ""
            isPlaceholder = false
            cardWidth = 180
            cardHeight = 220
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: clickedSpy; target: streamCard; signalName: "clicked" }
    SignalSpy { id: cardClickedSpy; target: streamCard; signalName: "cardClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "StreamCardTests"
        when: windowShown

        function init() {
            streamCard.reset()
            clickedSpy.clear()
            cardClickedSpy.clear()
            // Move mouse away from component to reset hover state
            mouseMove(root, 1, 1)
            wait(200)
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultState_streamerNameEmpty() {
            compare(streamCard.streamerName, "", "Default streamerName should be empty")
        }
        
        function test_defaultState_streamTitleEmpty() {
            compare(streamCard.streamTitle, "", "Default streamTitle should be empty")
        }
        
        function test_defaultState_viewerCountEmpty() {
            compare(streamCard.viewerCount, "", "Default viewerCount should be empty")
        }
        
        function test_defaultState_previewImageEmpty() {
            compare(streamCard.previewImage, "", "Default previewImage should be empty")
        }
        
        function test_defaultState_streamerLoginEmpty() {
            compare(streamCard.streamerLogin, "", "Default streamerLogin should be empty")
        }
        
        function test_defaultState_notPlaceholder() {
            compare(streamCard.isPlaceholder, false, "Should not be placeholder by default")
        }
        
        function test_defaultState_cardDimensions() {
            compare(streamCard.cardWidth, 180, "Default cardWidth should be 180")
            compare(streamCard.cardHeight, 220, "Default cardHeight should be 220")
        }
        
        function test_defaultState_cardRadius() {
            compare(streamCard.cardRadius, 12, "Default cardRadius should be 12")
        }
        
        function test_defaultState_contentMargins() {
            compare(streamCard.contentMargins, 12, "Default contentMargins should be 12")
        }
        
        function test_hover_canBeDeactivated() {
            // Arrange - first hover the component
            var mouseArea = findChild(streamCard, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(100)
            verify(streamCard.hovered, "Should be hovered initially")
            
            // Act - move mouse away (to corner of root, clearly outside component)
            mouseMove(root, 0, 0)
            wait(150)
            
            // Assert
            compare(streamCard.hovered, false, "Should not be hovered after mouse moves away")
        }

        // =====================================================================
        // TEST: Property Binding to UI Elements
        // =====================================================================
        
        function test_streamerName_bindsToText() {
            // Arrange
            streamCard.streamerName = "Ninja"
            wait(50)
            
            // Assert
            var streamerText = findChild(streamCard, "streamerNameText")
            verify(streamerText !== null, "Streamer name text should exist")
            compare(streamerText.text, "Ninja", "Streamer name should be displayed")
        }
        
        function test_streamTitle_bindsToText() {
            // Arrange
            streamCard.streamTitle = "Playing Fortnite with fans!"
            wait(50)
            
            // Assert
            var titleText = findChild(streamCard, "streamTitleText")
            verify(titleText !== null, "Stream title text should exist")
            compare(titleText.text, "Playing Fortnite with fans!", "Stream title should be displayed")
        }
        
        function test_viewerCount_bindsToText() {
            // Arrange
            streamCard.viewerCount = "45.2K viewers"
            wait(50)
            
            // Assert
            var viewerText = findChild(streamCard, "viewerCountText")
            verify(viewerText !== null, "Viewer count text should exist")
            compare(viewerText.text, "45.2K viewers", "Viewer count should be displayed")
        }
        
        function test_previewImage_bindsToSource() {
            // Arrange
            streamCard.previewImage = "https://example.com/preview.jpg"
            wait(50)
            
            // Assert
            var thumbnail = findChild(streamCard, "thumbnailImage")
            verify(thumbnail !== null, "Thumbnail image should exist")
            compare(thumbnail.source.toString(), "https://example.com/preview.jpg", "Preview image source should be set")
        }

        // =====================================================================
        // TEST: Signal clicked
        // =====================================================================
        
        function test_click_emitsClickedWithData() {
            // Arrange
            streamCard.streamerLogin = "ninja"
            streamCard.streamerName = "Ninja"
            streamCard.streamTitle = "Epic stream"
            streamCard.previewImage = "https://example.com/thumb.jpg"
            wait(50)
            
            // Act
            var mouseArea = findChild(streamCard, "mouseArea")
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 1, "clicked signal should be emitted once")
            compare(clickedSpy.signalArguments[0][0], "ninja", "First arg should be streamerLogin")
            compare(clickedSpy.signalArguments[0][1], "Ninja", "Second arg should be streamerName")
            compare(clickedSpy.signalArguments[0][2], "Epic stream", "Third arg should be streamTitle")
            compare(clickedSpy.signalArguments[0][3], "https://example.com/thumb.jpg", "Fourth arg should be thumbnailUrl")
        }
        
        function test_click_emitsCardClicked() {
            // Arrange
            streamCard.streamerLogin = "xqc"
            wait(50)
            
            // Act
            var mouseArea = findChild(streamCard, "mouseArea")
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 1, "cardClicked signal should be emitted")
        }
        
        function test_click_noClickedSignalWithoutLogin() {
            // Arrange - no streamerLogin set
            streamCard.streamerLogin = ""
            streamCard.streamerName = "SomeStreamer"
            wait(50)
            
            // Act
            var mouseArea = findChild(streamCard, "mouseArea")
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 0, "clicked signal should NOT be emitted without streamerLogin")
            compare(cardClickedSpy.count, 1, "cardClicked should still be emitted")
        }
        
        function test_click_noSignalWhenPlaceholder() {
            // Arrange
            streamCard.isPlaceholder = true
            streamCard.streamerLogin = "streamer"
            wait(50)
            
            // Act
            var mouseArea = findChild(streamCard, "mouseArea")
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 0, "clicked signal should NOT be emitted for placeholder")
            compare(cardClickedSpy.count, 0, "cardClicked should NOT be emitted for placeholder")
        }
        
        function test_multipleClicks_emitMultiple() {
            // Arrange
            streamCard.streamerLogin = "pokimane"
            streamCard.streamerName = "Pokimane"
            wait(50)
            
            // Act
            var mouseArea = findChild(streamCard, "mouseArea")
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 3, "Should emit clicked for each click")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_changesHoveredProperty() {
            // Arrange
            var mouseArea = findChild(streamCard, "mouseArea")
            verify(mouseArea !== null)
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert
            compare(streamCard.hovered, true, "hovered should be true when mouse is over")
        }
        
        function test_hover_increasesScale() {
            // Arrange
            var cardBackground = findChild(streamCard, "cardBackground")
            var mouseArea = findChild(streamCard, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250) // Wait for animation
            
            // Assert
            compare(cardBackground.scale, 1.02, "Scale should be 1.02 on hover")
        }
        
        function test_hoverExit_restoresScale() {
            // Arrange
            var cardBackground = findChild(streamCard, "cardBackground")
            var mouseArea = findChild(streamCard, "mouseArea")
            
            // Act - hover then leave
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250)
            mouseMove(root, 1, 1) // Move outside
            wait(250)
            
            // Assert
            compare(cardBackground.scale, 1.0, "Scale should return to 1.0 after hover exit")
            compare(streamCard.hovered, false, "hovered should be false after exit")
        }

        // =====================================================================
        // TEST: Placeholder Mode
        // =====================================================================
        
        function test_placeholder_hidesLiveBadge() {
            // Arrange
            streamCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var liveBadge = findChild(streamCard, "liveBadge")
            verify(liveBadge !== null, "LIVE badge should exist")
            verify(!liveBadge.visible, "LIVE badge should be hidden in placeholder mode")
        }
        
        function test_placeholder_showsPlaceholderIcon() {
            // Arrange
            streamCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var placeholderRect = findChild(streamCard, "placeholderRect")
            verify(placeholderRect !== null, "Placeholder rect should exist")
            verify(placeholderRect.visible, "Placeholder should be visible")
            
            var placeholderIcon = findChild(streamCard, "placeholderIcon")
            compare(placeholderIcon.text, "\u22EF", "Placeholder should show ellipsis icon")
        }
        
        function test_placeholder_noImageSource() {
            // Arrange
            streamCard.isPlaceholder = true
            streamCard.previewImage = "https://example.com/image.jpg"
            wait(50)
            
            // Assert
            var thumbnail = findChild(streamCard, "thumbnailImage")
            compare(thumbnail.source.toString(), "", "Image source should be empty in placeholder mode")
        }
        
        function test_placeholder_differentBackgroundColor() {
            // Arrange - ensure mouse is away to avoid hover color change
            mouseMove(root, root.width - 1, root.height - 1)
            wait(100)
            streamCard.isPlaceholder = true
            wait(300) // Wait for color animation to complete
            
            // Assert
            var cardBackground = findChild(streamCard, "cardBackground")
            compare(cardBackground.color.toString(), mockTheme.surfaceSoft.toString(), "Background should be surfaceSoft for placeholder")
        }

        // =====================================================================
        // TEST: LIVE Badge
        // =====================================================================
        
        function test_liveBadge_hasCorrectText() {
            // Assert
            var liveBadgeText = findChild(streamCard, "liveBadgeText")
            verify(liveBadgeText !== null, "LIVE badge text should exist")
            compare(liveBadgeText.text, "LIVE", "Badge should display 'LIVE'")
        }
        
        function test_liveBadge_hasCorrectColor() {
            // Assert
            var liveBadge = findChild(streamCard, "liveBadge")
            compare(liveBadge.color.toString(), mockTheme.statusNegative.toString(), "Badge should be red (statusNegative)")
        }
        
        function test_liveBadge_dimensions() {
            // Assert
            var liveBadge = findChild(streamCard, "liveBadge")
            compare(liveBadge.width, 40, "Badge width should be 40")
            compare(liveBadge.height, 20, "Badge height should be 20")
            compare(liveBadge.radius, 10, "Badge radius should be 10 (pill shape)")
        }

        // =====================================================================
        // TEST: Text Styling
        // =====================================================================
        
        function test_streamerNameText_isBold() {
            // Assert
            var streamerText = findChild(streamCard, "streamerNameText")
            verify(streamerText !== null)
            compare(streamerText.font.bold, true, "Streamer name should be bold")
        }
        
        function test_streamerNameText_fontSize() {
            // Assert
            var streamerText = findChild(streamCard, "streamerNameText")
            compare(streamerText.font.pixelSize, 14, "Streamer name font size should be 14")
        }
        
        function test_streamerNameText_color() {
            // Assert
            var streamerText = findChild(streamCard, "streamerNameText")
            compare(streamerText.color.toString(), mockTheme.primaryText.toString(), "Streamer name should use primary text color")
        }
        
        function test_streamTitleText_fontSize() {
            // Assert
            var titleText = findChild(streamCard, "streamTitleText")
            compare(titleText.font.pixelSize, 12, "Stream title font size should be 12")
        }
        
        function test_streamTitleText_color() {
            // Assert
            var titleText = findChild(streamCard, "streamTitleText")
            compare(titleText.color.toString(), mockTheme.secondaryText.toString(), "Stream title should use secondary text color")
        }
        
        function test_streamTitleText_maxLines() {
            // Assert
            var titleText = findChild(streamCard, "streamTitleText")
            compare(titleText.maximumLineCount, 2, "Stream title should have max 2 lines")
        }
        
        function test_viewerCountText_fontSize() {
            // Assert
            var viewerText = findChild(streamCard, "viewerCountText")
            compare(viewerText.font.pixelSize, 11, "Viewer count font size should be 11")
        }
        
        function test_viewerCountText_color() {
            // Assert
            var viewerText = findChild(streamCard, "viewerCountText")
            compare(viewerText.color.toString(), mockTheme.accent.toString(), "Viewer count should use accent color")
        }

        // =====================================================================
        // TEST: MouseArea Configuration
        // =====================================================================
        
        function test_mouseArea_hoverEnabled() {
            var mouseArea = findChild(streamCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.hoverEnabled, true, "Hover should be enabled")
        }
        
        function test_mouseArea_cursorShape() {
            var mouseArea = findChild(streamCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.cursorShape, Qt.PointingHandCursor, "Cursor should be pointing hand")
        }
        
        function test_mouseArea_fillsParent() {
            var mouseArea = findChild(streamCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.width, streamCard.width, "MouseArea should fill card width")
            compare(mouseArea.height, streamCard.height, "MouseArea should fill card height")
        }

        // =====================================================================
        // TEST: Card Dimensions
        // =====================================================================
        
        function test_card_appliesCustomWidth() {
            // Arrange
            streamCard.cardWidth = 200
            wait(50)
            
            // Assert
            compare(streamCard.width, 200, "Custom cardWidth should apply")
        }
        
        function test_card_appliesCustomHeight() {
            // Arrange
            streamCard.cardHeight = 250
            wait(50)
            
            // Assert
            compare(streamCard.height, 250, "Custom cardHeight should apply")
        }
        
        function test_cardBackground_appliesRadius() {
            // Assert
            var cardBackground = findChild(streamCard, "cardBackground")
            compare(cardBackground.radius, streamCard.cardRadius, "Background radius should match cardRadius")
        }

        // =====================================================================
        // TEST: Preview Zone
        // =====================================================================
        
        function test_previewZone_exists() {
            var previewZone = findChild(streamCard, "previewZone")
            verify(previewZone !== null, "Preview zone should exist")
        }
        
        function test_previewZone_height() {
            var previewZone = findChild(streamCard, "previewZone")
            compare(previewZone.Layout.preferredHeight, 120, "Preview zone height should be 120")
        }
        
        function test_previewZone_hasClipping() {
            var previewZone = findChild(streamCard, "previewZone")
            compare(previewZone.clip, true, "Preview zone should clip content")
        }
        
        function test_previewZone_radius() {
            var previewZone = findChild(streamCard, "previewZone")
            compare(previewZone.radius, 8, "Preview zone radius should be 8")
        }

        // =====================================================================
        // TEST: Thumbnail Image
        // =====================================================================
        
        function test_thumbnailImage_isAsynchronous() {
            var thumbnail = findChild(streamCard, "thumbnailImage")
            compare(thumbnail.asynchronous, true, "Thumbnail should load asynchronously")
        }
        
        function test_thumbnailImage_isCached() {
            var thumbnail = findChild(streamCard, "thumbnailImage")
            compare(thumbnail.cache, true, "Thumbnail should be cached")
        }
        
        function test_thumbnailImage_fillMode() {
            var thumbnail = findChild(streamCard, "thumbnailImage")
            compare(thumbnail.fillMode, Image.PreserveAspectCrop, "Thumbnail should preserve aspect and crop")
        }
    }
}
