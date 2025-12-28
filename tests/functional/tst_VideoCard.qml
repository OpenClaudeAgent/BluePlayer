/**
 * tst_VideoCard.qml
 * 
 * Functional UI tests for the VideoCard component.
 * Tests video cards displaying VOD info, thumbnail, duration badge,
 * view count, and resume badge with hover effects.
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
        readonly property string fontFamily: "Inter"
        readonly property int animCardDuration: 200
        readonly property int animContentFadeDuration: 300
    }

    // =========================================================================
    // Component Under Test (Mock of VideoCard with BaseCard behavior)
    // =========================================================================
    
    Item {
        id: videoCard
        objectName: "videoCard"
        anchors.centerIn: parent

        // VideoCard properties
        property string videoTitle: ""
        property string userName: ""
        property string viewCount: ""
        property string duration: ""
        property string thumbnailUrl: ""
        property bool hasProgress: false
        property int watchPosition: 0

        // BaseCard properties
        property bool isPlaceholder: false
        property int cardWidth: 180
        property int cardHeight: 220
        property int cardRadius: 12
        property int contentMargins: 12

        // Expose hover state
        readonly property bool hovered: mouseArea.containsMouse

        // Signals
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
            radius: videoCard.cardRadius
            color: videoCard.isPlaceholder ? mockTheme.surfaceSoft : mockTheme.surface
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
                anchors.margins: videoCard.contentMargins
                spacing: 12

                // Thumbnail zone
                Rectangle {
                    id: thumbnailZone
                    objectName: "thumbnailZone"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    radius: 8
                    color: videoCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                    border.color: mockTheme.divider
                    border.width: 1
                    clip: true

                    // Thumbnail image
                    Image {
                        id: thumbnailImage
                        objectName: "thumbnailImage"
                        anchors.fill: parent
                        source: videoCard.isPlaceholder ? "" : videoCard.thumbnailUrl
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        cache: true
                        visible: status === Image.Ready && !videoCard.isPlaceholder

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
                        color: videoCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                        visible: thumbnailImage.status !== Image.Ready || videoCard.isPlaceholder

                        Text {
                            id: placeholderIcon
                            objectName: "placeholderIcon"
                            anchors.centerIn: parent
                            text: videoCard.isPlaceholder ? "\u22EF" : (thumbnailImage.status === Image.Loading ? "\u23F3" : "\uD83D\uDCF9")
                            font.pixelSize: 32
                            color: mockTheme.mutedText
                            opacity: 0.5
                        }
                    }

                    // Duration badge
                    Rectangle {
                        id: durationBadge
                        objectName: "durationBadge"
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.margins: 6
                        width: durationText.implicitWidth + 8
                        height: 20
                        radius: 10
                        color: "#00000080"
                        visible: !videoCard.isPlaceholder && videoCard.duration !== "" && thumbnailImage.status === Image.Ready

                        Text {
                            id: durationText
                            objectName: "durationText"
                            anchors.centerIn: parent
                            text: videoCard.duration
                            font.pixelSize: 10
                            font.bold: true
                            color: "#ffffff"
                        }
                    }

                    // Resume badge
                    Rectangle {
                        id: resumeBadge
                        objectName: "resumeBadge"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.margins: 6
                        width: resumeBadgeText.implicitWidth + 12
                        height: 24
                        radius: 12
                        color: mockTheme.accent
                        visible: videoCard.hasProgress && !videoCard.isPlaceholder && thumbnailImage.status === Image.Ready

                        Text {
                            id: resumeBadgeText
                            objectName: "resumeBadgeText"
                            anchors.centerIn: parent
                            text: "\u25B6 Reprendre"
                            font.pixelSize: 10
                            font.bold: true
                            color: "#ffffff"
                        }
                    }
                }

                // Video info
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        id: videoTitleText
                        objectName: "videoTitleText"
                        text: videoCard.videoTitle
                        font.family: mockTheme.fontFamily
                        font.pixelSize: 12
                        font.bold: true
                        color: mockTheme.primaryText
                        elide: Text.ElideRight
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        Layout.fillWidth: true
                    }

                    Text {
                        id: userNameText
                        objectName: "userNameText"
                        text: videoCard.userName
                        font.family: mockTheme.fontFamily
                        font.pixelSize: 11
                        color: mockTheme.secondaryText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Text {
                        id: viewCountText
                        objectName: "viewCountText"
                        text: videoCard.viewCount !== "" ? videoCard.viewCount + " vues" : ""
                        font.family: mockTheme.fontFamily
                        font.pixelSize: 10
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
                if (!videoCard.isPlaceholder) {
                    videoCard.cardClicked()
                }
            }
        }

        // Reset function for tests
        function reset() {
            videoTitle = ""
            userName = ""
            viewCount = ""
            duration = ""
            thumbnailUrl = ""
            hasProgress = false
            watchPosition = 0
            isPlaceholder = false
            cardWidth = 180
            cardHeight = 220
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: cardClickedSpy; target: videoCard; signalName: "cardClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "VideoCardTests"
        when: windowShown

        function init() {
            videoCard.reset()
            cardClickedSpy.clear()
            // Move mouse away from component to reset hover state
            mouseMove(root, 1, 1)
            wait(200)
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultState_videoTitleEmpty() {
            compare(videoCard.videoTitle, "", "Default videoTitle should be empty")
        }
        
        function test_defaultState_userNameEmpty() {
            compare(videoCard.userName, "", "Default userName should be empty")
        }
        
        function test_defaultState_viewCountEmpty() {
            compare(videoCard.viewCount, "", "Default viewCount should be empty")
        }
        
        function test_defaultState_durationEmpty() {
            compare(videoCard.duration, "", "Default duration should be empty")
        }
        
        function test_defaultState_thumbnailUrlEmpty() {
            compare(videoCard.thumbnailUrl, "", "Default thumbnailUrl should be empty")
        }
        
        function test_defaultState_hasProgressFalse() {
            compare(videoCard.hasProgress, false, "Default hasProgress should be false")
        }
        
        function test_defaultState_watchPositionZero() {
            compare(videoCard.watchPosition, 0, "Default watchPosition should be 0")
        }
        
        function test_defaultState_notPlaceholder() {
            compare(videoCard.isPlaceholder, false, "Should not be placeholder by default")
        }
        
        function test_defaultState_cardDimensions() {
            compare(videoCard.cardWidth, 180, "Default cardWidth should be 180")
            compare(videoCard.cardHeight, 220, "Default cardHeight should be 220")
        }
        
        function test_defaultState_cardRadius() {
            compare(videoCard.cardRadius, 12, "Default cardRadius should be 12")
        }
        
        function test_defaultState_contentMargins() {
            compare(videoCard.contentMargins, 12, "Default contentMargins should be 12")
        }

        // =====================================================================
        // TEST: Property Binding to UI Elements
        // =====================================================================
        
        function test_videoTitle_bindsToText() {
            // Arrange
            videoCard.videoTitle = "Epic Gaming Moments - Best of 2024"
            wait(50)
            
            // Assert
            var titleText = findChild(videoCard, "videoTitleText")
            verify(titleText !== null, "Video title text should exist")
            compare(titleText.text, "Epic Gaming Moments - Best of 2024", "Video title should be displayed")
        }
        
        function test_userName_bindsToText() {
            // Arrange
            videoCard.userName = "Ninja"
            wait(50)
            
            // Assert
            var userText = findChild(videoCard, "userNameText")
            verify(userText !== null, "User name text should exist")
            compare(userText.text, "Ninja", "User name should be displayed")
        }
        
        function test_viewCount_bindsToTextWithSuffix() {
            // Arrange
            videoCard.viewCount = "1.2M"
            wait(50)
            
            // Assert
            var viewText = findChild(videoCard, "viewCountText")
            verify(viewText !== null, "View count text should exist")
            compare(viewText.text, "1.2M vues", "View count should be displayed with 'vues' suffix")
        }
        
        function test_viewCount_emptyShowsNothing() {
            // Arrange
            videoCard.viewCount = ""
            wait(50)
            
            // Assert
            var viewText = findChild(videoCard, "viewCountText")
            compare(viewText.text, "", "Empty viewCount should show empty text")
        }
        
        function test_duration_bindsToText() {
            // Arrange
            videoCard.duration = "2:34:56"
            wait(50)
            
            // Assert
            var durationText = findChild(videoCard, "durationText")
            verify(durationText !== null, "Duration text should exist")
            compare(durationText.text, "2:34:56", "Duration should be displayed")
        }
        
        function test_thumbnailUrl_bindsToSource() {
            // Arrange
            videoCard.thumbnailUrl = "https://example.com/thumb.jpg"
            wait(50)
            
            // Assert
            var thumbnail = findChild(videoCard, "thumbnailImage")
            verify(thumbnail !== null, "Thumbnail image should exist")
            compare(thumbnail.source.toString(), "https://example.com/thumb.jpg", "Thumbnail source should be set")
        }

        // =====================================================================
        // TEST: Signal clicked
        // =====================================================================
        
        function test_click_emitsCardClicked() {
            // Arrange
            videoCard.videoTitle = "Test Video"
            wait(50)
            
            // Act
            var mouseArea = findChild(videoCard, "mouseArea")
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 1, "cardClicked signal should be emitted")
        }
        
        function test_click_noSignalWhenPlaceholder() {
            // Arrange
            videoCard.isPlaceholder = true
            videoCard.videoTitle = "Test Video"
            wait(50)
            
            // Act
            var mouseArea = findChild(videoCard, "mouseArea")
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 0, "cardClicked should NOT be emitted for placeholder")
        }
        
        function test_multipleClicks_emitMultiple() {
            // Arrange
            videoCard.videoTitle = "Clickable Video"
            wait(50)
            
            // Act
            var mouseArea = findChild(videoCard, "mouseArea")
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 3, "Should emit cardClicked for each click")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_changesHoveredProperty() {
            // Arrange
            var mouseArea = findChild(videoCard, "mouseArea")
            verify(mouseArea !== null)
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert
            compare(videoCard.hovered, true, "hovered should be true when mouse is over")
        }
        
        function test_hover_increasesScale() {
            // Arrange
            var cardBackground = findChild(videoCard, "cardBackground")
            var mouseArea = findChild(videoCard, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250) // Wait for animation
            
            // Assert
            compare(cardBackground.scale, 1.02, "Scale should be 1.02 on hover")
        }
        
        function test_hoverExit_restoresScale() {
            // Arrange
            var cardBackground = findChild(videoCard, "cardBackground")
            var mouseArea = findChild(videoCard, "mouseArea")
            
            // Act - hover then leave
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250)
            mouseMove(root, 1, 1) // Move outside
            wait(250)
            
            // Assert
            compare(cardBackground.scale, 1.0, "Scale should return to 1.0 after hover exit")
            compare(videoCard.hovered, false, "hovered should be false after exit")
        }
        
        function test_hover_canBeDeactivated() {
            // Arrange - first hover the component
            var mouseArea = findChild(videoCard, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(100)
            verify(videoCard.hovered, "Should be hovered initially")
            
            // Act - move mouse away
            mouseMove(root, 0, 0)
            wait(150)
            
            // Assert
            compare(videoCard.hovered, false, "Should not be hovered after mouse moves away")
        }

        // =====================================================================
        // TEST: Placeholder Mode
        // =====================================================================
        
        function test_placeholder_hidesDurationBadge() {
            // Arrange
            videoCard.isPlaceholder = true
            videoCard.duration = "1:00:00"
            wait(50)
            
            // Assert
            var durationBadge = findChild(videoCard, "durationBadge")
            verify(durationBadge !== null, "Duration badge should exist")
            verify(!durationBadge.visible, "Duration badge should be hidden in placeholder mode")
        }
        
        function test_placeholder_hidesResumeBadge() {
            // Arrange
            videoCard.isPlaceholder = true
            videoCard.hasProgress = true
            wait(50)
            
            // Assert
            var resumeBadge = findChild(videoCard, "resumeBadge")
            verify(resumeBadge !== null, "Resume badge should exist")
            verify(!resumeBadge.visible, "Resume badge should be hidden in placeholder mode")
        }
        
        function test_placeholder_showsPlaceholderIcon() {
            // Arrange
            videoCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var placeholderRect = findChild(videoCard, "placeholderRect")
            verify(placeholderRect !== null, "Placeholder rect should exist")
            verify(placeholderRect.visible, "Placeholder should be visible")
            
            var placeholderIcon = findChild(videoCard, "placeholderIcon")
            compare(placeholderIcon.text, "\u22EF", "Placeholder should show ellipsis icon")
        }
        
        function test_placeholder_noImageSource() {
            // Arrange
            videoCard.isPlaceholder = true
            videoCard.thumbnailUrl = "https://example.com/image.jpg"
            wait(50)
            
            // Assert
            var thumbnail = findChild(videoCard, "thumbnailImage")
            compare(thumbnail.source.toString(), "", "Image source should be empty in placeholder mode")
        }
        
        function test_placeholder_differentBackgroundColor() {
            // Arrange - ensure mouse is away to avoid hover color change
            mouseMove(root, root.width - 1, root.height - 1)
            wait(100)
            videoCard.isPlaceholder = true
            wait(300) // Wait for color animation
            
            // Assert
            var cardBackground = findChild(videoCard, "cardBackground")
            compare(cardBackground.color.toString(), mockTheme.surfaceSoft.toString(), "Background should be surfaceSoft for placeholder")
        }

        // =====================================================================
        // TEST: Duration Badge
        // =====================================================================
        
        function test_durationBadge_hiddenWhenEmpty() {
            // Arrange
            videoCard.duration = ""
            wait(50)
            
            // Assert
            var durationBadge = findChild(videoCard, "durationBadge")
            verify(!durationBadge.visible, "Duration badge should be hidden when duration is empty")
        }
        
        function test_durationBadge_hasCorrectColor() {
            // Assert
            var durationBadge = findChild(videoCard, "durationBadge")
            compare(durationBadge.color.toString(), "#00000080", "Badge should be semi-transparent black")
        }
        
        function test_durationBadge_dimensions() {
            // Assert
            var durationBadge = findChild(videoCard, "durationBadge")
            compare(durationBadge.height, 20, "Badge height should be 20")
            compare(durationBadge.radius, 10, "Badge radius should be 10 (pill shape)")
        }
        
        function test_durationBadge_textIsBold() {
            // Assert
            var durationText = findChild(videoCard, "durationText")
            compare(durationText.font.bold, true, "Duration text should be bold")
        }
        
        function test_durationBadge_textIsWhite() {
            // Assert
            var durationText = findChild(videoCard, "durationText")
            compare(durationText.color.toString(), "#ffffff", "Duration text should be white")
        }
        
        function test_durationBadge_fontSize() {
            // Assert
            var durationText = findChild(videoCard, "durationText")
            compare(durationText.font.pixelSize, 10, "Duration font size should be 10")
        }

        // =====================================================================
        // TEST: Resume Badge
        // =====================================================================
        
        function test_resumeBadge_hiddenByDefault() {
            // Arrange - hasProgress is false by default
            wait(50)
            
            // Assert
            var resumeBadge = findChild(videoCard, "resumeBadge")
            verify(!resumeBadge.visible, "Resume badge should be hidden when hasProgress is false")
        }
        
        function test_resumeBadge_hasCorrectText() {
            // Assert
            var resumeBadgeText = findChild(videoCard, "resumeBadgeText")
            verify(resumeBadgeText !== null, "Resume badge text should exist")
            compare(resumeBadgeText.text, "\u25B6 Reprendre", "Badge should display play icon and 'Reprendre'")
        }
        
        function test_resumeBadge_hasAccentColor() {
            // Assert
            var resumeBadge = findChild(videoCard, "resumeBadge")
            compare(resumeBadge.color.toString(), mockTheme.accent.toString(), "Badge should use accent color")
        }
        
        function test_resumeBadge_dimensions() {
            // Assert
            var resumeBadge = findChild(videoCard, "resumeBadge")
            compare(resumeBadge.height, 24, "Badge height should be 24")
            compare(resumeBadge.radius, 12, "Badge radius should be 12 (pill shape)")
        }
        
        function test_resumeBadge_textIsBold() {
            // Assert
            var resumeBadgeText = findChild(videoCard, "resumeBadgeText")
            compare(resumeBadgeText.font.bold, true, "Resume text should be bold")
        }
        
        function test_resumeBadge_textIsWhite() {
            // Assert
            var resumeBadgeText = findChild(videoCard, "resumeBadgeText")
            compare(resumeBadgeText.color.toString(), "#ffffff", "Resume text should be white")
        }
        
        function test_resumeBadge_fontSize() {
            // Assert
            var resumeBadgeText = findChild(videoCard, "resumeBadgeText")
            compare(resumeBadgeText.font.pixelSize, 10, "Resume font size should be 10")
        }

        // =====================================================================
        // TEST: Text Styling
        // =====================================================================
        
        function test_videoTitleText_isBold() {
            // Assert
            var titleText = findChild(videoCard, "videoTitleText")
            verify(titleText !== null)
            compare(titleText.font.bold, true, "Video title should be bold")
        }
        
        function test_videoTitleText_fontSize() {
            // Assert
            var titleText = findChild(videoCard, "videoTitleText")
            compare(titleText.font.pixelSize, 12, "Video title font size should be 12")
        }
        
        function test_videoTitleText_color() {
            // Assert
            var titleText = findChild(videoCard, "videoTitleText")
            compare(titleText.color.toString(), mockTheme.primaryText.toString(), "Video title should use primary text color")
        }
        
        function test_videoTitleText_maxLines() {
            // Assert
            var titleText = findChild(videoCard, "videoTitleText")
            compare(titleText.maximumLineCount, 2, "Video title should have max 2 lines")
        }
        
        function test_userNameText_fontSize() {
            // Assert
            var userText = findChild(videoCard, "userNameText")
            compare(userText.font.pixelSize, 11, "User name font size should be 11")
        }
        
        function test_userNameText_color() {
            // Assert
            var userText = findChild(videoCard, "userNameText")
            compare(userText.color.toString(), mockTheme.secondaryText.toString(), "User name should use secondary text color")
        }
        
        function test_viewCountText_fontSize() {
            // Assert
            var viewText = findChild(videoCard, "viewCountText")
            compare(viewText.font.pixelSize, 10, "View count font size should be 10")
        }
        
        function test_viewCountText_color() {
            // Assert
            var viewText = findChild(videoCard, "viewCountText")
            compare(viewText.color.toString(), mockTheme.accent.toString(), "View count should use accent color")
        }

        // =====================================================================
        // TEST: MouseArea Configuration
        // =====================================================================
        
        function test_mouseArea_hoverEnabled() {
            var mouseArea = findChild(videoCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.hoverEnabled, true, "Hover should be enabled")
        }
        
        function test_mouseArea_cursorShape() {
            var mouseArea = findChild(videoCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.cursorShape, Qt.PointingHandCursor, "Cursor should be pointing hand")
        }
        
        function test_mouseArea_fillsParent() {
            var mouseArea = findChild(videoCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.width, videoCard.width, "MouseArea should fill card width")
            compare(mouseArea.height, videoCard.height, "MouseArea should fill card height")
        }

        // =====================================================================
        // TEST: Card Dimensions
        // =====================================================================
        
        function test_card_appliesCustomWidth() {
            // Arrange
            videoCard.cardWidth = 200
            wait(50)
            
            // Assert
            compare(videoCard.width, 200, "Custom cardWidth should apply")
        }
        
        function test_card_appliesCustomHeight() {
            // Arrange
            videoCard.cardHeight = 250
            wait(50)
            
            // Assert
            compare(videoCard.height, 250, "Custom cardHeight should apply")
        }
        
        function test_cardBackground_appliesRadius() {
            // Assert
            var cardBackground = findChild(videoCard, "cardBackground")
            compare(cardBackground.radius, videoCard.cardRadius, "Background radius should match cardRadius")
        }

        // =====================================================================
        // TEST: Thumbnail Zone
        // =====================================================================
        
        function test_thumbnailZone_exists() {
            var thumbnailZone = findChild(videoCard, "thumbnailZone")
            verify(thumbnailZone !== null, "Thumbnail zone should exist")
        }
        
        function test_thumbnailZone_height() {
            var thumbnailZone = findChild(videoCard, "thumbnailZone")
            compare(thumbnailZone.Layout.preferredHeight, 120, "Thumbnail zone height should be 120")
        }
        
        function test_thumbnailZone_hasClipping() {
            var thumbnailZone = findChild(videoCard, "thumbnailZone")
            compare(thumbnailZone.clip, true, "Thumbnail zone should clip content")
        }
        
        function test_thumbnailZone_radius() {
            var thumbnailZone = findChild(videoCard, "thumbnailZone")
            compare(thumbnailZone.radius, 8, "Thumbnail zone radius should be 8")
        }

        // =====================================================================
        // TEST: Thumbnail Image
        // =====================================================================
        
        function test_thumbnailImage_isAsynchronous() {
            var thumbnail = findChild(videoCard, "thumbnailImage")
            compare(thumbnail.asynchronous, true, "Thumbnail should load asynchronously")
        }
        
        function test_thumbnailImage_isCached() {
            var thumbnail = findChild(videoCard, "thumbnailImage")
            compare(thumbnail.cache, true, "Thumbnail should be cached")
        }
        
        function test_thumbnailImage_fillMode() {
            var thumbnail = findChild(videoCard, "thumbnailImage")
            compare(thumbnail.fillMode, Image.PreserveAspectCrop, "Thumbnail should preserve aspect and crop")
        }

        // =====================================================================
        // TEST: WatchPosition Property
        // =====================================================================
        
        function test_watchPosition_canBeSet() {
            // Arrange
            videoCard.watchPosition = 3600
            
            // Assert
            compare(videoCard.watchPosition, 3600, "watchPosition should be settable")
        }
        
        function test_watchPosition_independentOfHasProgress() {
            // Arrange - watchPosition set but hasProgress false
            videoCard.watchPosition = 1000
            videoCard.hasProgress = false
            wait(50)
            
            // Assert
            var resumeBadge = findChild(videoCard, "resumeBadge")
            verify(!resumeBadge.visible, "Resume badge visibility depends on hasProgress, not watchPosition")
        }
    }
}
