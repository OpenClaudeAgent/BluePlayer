/**
 * tst_ClipCard.qml
 * 
 * Functional UI tests for the ClipCard component.
 * Tests clip cards displaying clip title, broadcaster name,
 * view count, duration badge, and thumbnail with hover effects.
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
    // Component Under Test (Mock of ClipCard with BaseCard behavior)
    // =========================================================================
    
    Item {
        id: clipCard
        objectName: "clipCard"
        anchors.centerIn: parent

        // ClipCard properties
        property string clipTitle: ""
        property string broadcasterName: ""
        property string viewCount: ""
        property string duration: ""
        property string thumbnailUrl: ""

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
            radius: clipCard.cardRadius
            color: clipCard.isPlaceholder ? mockTheme.surfaceSoft : mockTheme.surface
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
                anchors.margins: clipCard.contentMargins
                spacing: 12

                // Thumbnail zone
                Rectangle {
                    id: thumbnailZone
                    objectName: "thumbnailZone"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    radius: 8
                    color: clipCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                    border.color: mockTheme.divider
                    border.width: 1
                    clip: true

                    // Thumbnail image
                    Image {
                        id: thumbnailImage
                        objectName: "thumbnailImage"
                        anchors.fill: parent
                        source: clipCard.isPlaceholder ? "" : clipCard.thumbnailUrl
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        cache: true
                        visible: status === Image.Ready && !clipCard.isPlaceholder

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
                        color: clipCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                        visible: thumbnailImage.status !== Image.Ready || clipCard.isPlaceholder

                        Text {
                            id: placeholderIcon
                            objectName: "placeholderIcon"
                            anchors.centerIn: parent
                            text: clipCard.isPlaceholder ? "\u22EF" : (thumbnailImage.status === Image.Loading ? "\u23F3" : "\uD83C\uDFAC")
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
                        visible: !clipCard.isPlaceholder && clipCard.duration !== "" && thumbnailImage.status === Image.Ready

                        Text {
                            id: durationText
                            objectName: "durationText"
                            anchors.centerIn: parent
                            text: clipCard.duration
                            font.pixelSize: 10
                            font.bold: true
                            color: "#ffffff"
                        }
                    }
                }

                // Clip info
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        id: clipTitleText
                        objectName: "clipTitleText"
                        text: clipCard.clipTitle
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
                        id: broadcasterNameText
                        objectName: "broadcasterNameText"
                        text: clipCard.broadcasterName
                        font.family: mockTheme.fontFamily
                        font.pixelSize: 11
                        color: mockTheme.secondaryText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Text {
                        id: viewCountText
                        objectName: "viewCountText"
                        text: clipCard.viewCount !== "" ? clipCard.viewCount + " vues" : ""
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
                if (!clipCard.isPlaceholder) {
                    clipCard.cardClicked()
                }
            }
        }

        // Reset function for tests
        function reset() {
            clipTitle = ""
            broadcasterName = ""
            viewCount = ""
            duration = ""
            thumbnailUrl = ""
            isPlaceholder = false
            cardWidth = 180
            cardHeight = 220
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: cardClickedSpy; target: clipCard; signalName: "cardClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "ClipCardTests"
        when: windowShown

        function init() {
            clipCard.reset()
            cardClickedSpy.clear()
            // Move mouse away from component to reset hover state
            mouseMove(root, 1, 1)
            wait(200)
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultState_clipTitleEmpty() {
            compare(clipCard.clipTitle, "", "Default clipTitle should be empty")
        }
        
        function test_defaultState_broadcasterNameEmpty() {
            compare(clipCard.broadcasterName, "", "Default broadcasterName should be empty")
        }
        
        function test_defaultState_viewCountEmpty() {
            compare(clipCard.viewCount, "", "Default viewCount should be empty")
        }
        
        function test_defaultState_durationEmpty() {
            compare(clipCard.duration, "", "Default duration should be empty")
        }
        
        function test_defaultState_thumbnailUrlEmpty() {
            compare(clipCard.thumbnailUrl, "", "Default thumbnailUrl should be empty")
        }
        
        function test_defaultState_notPlaceholder() {
            compare(clipCard.isPlaceholder, false, "Should not be placeholder by default")
        }
        
        function test_defaultState_cardDimensions() {
            compare(clipCard.cardWidth, 180, "Default cardWidth should be 180")
            compare(clipCard.cardHeight, 220, "Default cardHeight should be 220")
        }
        
        function test_defaultState_cardRadius() {
            compare(clipCard.cardRadius, 12, "Default cardRadius should be 12")
        }
        
        function test_defaultState_contentMargins() {
            compare(clipCard.contentMargins, 12, "Default contentMargins should be 12")
        }

        // =====================================================================
        // TEST: Property Binding to UI Elements
        // =====================================================================
        
        function test_clipTitle_bindsToText() {
            // Arrange
            clipCard.clipTitle = "Epic 360 No Scope"
            wait(50)
            
            // Assert
            var titleText = findChild(clipCard, "clipTitleText")
            verify(titleText !== null, "Clip title text should exist")
            compare(titleText.text, "Epic 360 No Scope", "Clip title should be displayed")
        }
        
        function test_broadcasterName_bindsToText() {
            // Arrange
            clipCard.broadcasterName = "Shroud"
            wait(50)
            
            // Assert
            var broadcasterText = findChild(clipCard, "broadcasterNameText")
            verify(broadcasterText !== null, "Broadcaster name text should exist")
            compare(broadcasterText.text, "Shroud", "Broadcaster name should be displayed")
        }
        
        function test_viewCount_bindsToTextWithSuffix() {
            // Arrange
            clipCard.viewCount = "1.2M"
            wait(50)
            
            // Assert
            var viewText = findChild(clipCard, "viewCountText")
            verify(viewText !== null, "View count text should exist")
            compare(viewText.text, "1.2M vues", "View count should be displayed with 'vues' suffix")
        }
        
        function test_viewCount_emptyShowsNothing() {
            // Arrange
            clipCard.viewCount = ""
            wait(50)
            
            // Assert
            var viewText = findChild(clipCard, "viewCountText")
            compare(viewText.text, "", "Empty view count should show empty text")
        }
        
        function test_duration_bindsToText() {
            // Arrange
            clipCard.duration = "0:30"
            wait(50)
            
            // Assert
            var durationText = findChild(clipCard, "durationText")
            verify(durationText !== null, "Duration text should exist")
            compare(durationText.text, "0:30", "Duration should be displayed")
        }
        
        function test_thumbnailUrl_bindsToSource() {
            // Arrange
            clipCard.thumbnailUrl = "https://example.com/clip-thumb.jpg"
            wait(50)
            
            // Assert
            var thumbnail = findChild(clipCard, "thumbnailImage")
            verify(thumbnail !== null, "Thumbnail image should exist")
            compare(thumbnail.source.toString(), "https://example.com/clip-thumb.jpg", "Thumbnail source should be set")
        }

        // =====================================================================
        // TEST: Signal clicked
        // =====================================================================
        
        function test_click_emitsCardClicked() {
            // Arrange
            clipCard.clipTitle = "Amazing clip"
            wait(50)
            
            // Act
            var mouseArea = findChild(clipCard, "mouseArea")
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 1, "cardClicked signal should be emitted once")
        }
        
        function test_click_noSignalWhenPlaceholder() {
            // Arrange
            clipCard.isPlaceholder = true
            clipCard.clipTitle = "Some clip"
            wait(50)
            
            // Act
            var mouseArea = findChild(clipCard, "mouseArea")
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 0, "cardClicked should NOT be emitted for placeholder")
        }
        
        function test_multipleClicks_emitMultiple() {
            // Arrange
            clipCard.clipTitle = "Clip"
            wait(50)
            
            // Act
            var mouseArea = findChild(clipCard, "mouseArea")
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
            var mouseArea = findChild(clipCard, "mouseArea")
            verify(mouseArea !== null)
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert
            compare(clipCard.hovered, true, "hovered should be true when mouse is over")
        }
        
        function test_hover_increasesScale() {
            // Arrange
            var cardBackground = findChild(clipCard, "cardBackground")
            var mouseArea = findChild(clipCard, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250) // Wait for animation
            
            // Assert
            compare(cardBackground.scale, 1.02, "Scale should be 1.02 on hover")
        }
        
        function test_hoverExit_restoresScale() {
            // Arrange
            var cardBackground = findChild(clipCard, "cardBackground")
            var mouseArea = findChild(clipCard, "mouseArea")
            
            // Act - hover then leave
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250)
            mouseMove(root, 1, 1) // Move outside
            wait(250)
            
            // Assert
            compare(cardBackground.scale, 1.0, "Scale should return to 1.0 after hover exit")
            compare(clipCard.hovered, false, "hovered should be false after exit")
        }
        
        function test_hover_canBeDeactivated() {
            // Arrange - first hover the component
            var mouseArea = findChild(clipCard, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(100)
            verify(clipCard.hovered, "Should be hovered initially")
            
            // Act - move mouse away (to corner of root, clearly outside component)
            mouseMove(root, 0, 0)
            wait(150)
            
            // Assert
            compare(clipCard.hovered, false, "Should not be hovered after mouse moves away")
        }

        // =====================================================================
        // TEST: Placeholder Mode
        // =====================================================================
        
        function test_placeholder_hidesDurationBadge() {
            // Arrange
            clipCard.isPlaceholder = true
            clipCard.duration = "1:30"
            wait(50)
            
            // Assert
            var durationBadge = findChild(clipCard, "durationBadge")
            verify(durationBadge !== null, "Duration badge should exist")
            verify(!durationBadge.visible, "Duration badge should be hidden in placeholder mode")
        }
        
        function test_placeholder_showsPlaceholderIcon() {
            // Arrange
            clipCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var placeholderRect = findChild(clipCard, "placeholderRect")
            verify(placeholderRect !== null, "Placeholder rect should exist")
            verify(placeholderRect.visible, "Placeholder should be visible")
            
            var placeholderIcon = findChild(clipCard, "placeholderIcon")
            compare(placeholderIcon.text, "\u22EF", "Placeholder should show ellipsis icon")
        }
        
        function test_placeholder_noImageSource() {
            // Arrange
            clipCard.isPlaceholder = true
            clipCard.thumbnailUrl = "https://example.com/image.jpg"
            wait(50)
            
            // Assert
            var thumbnail = findChild(clipCard, "thumbnailImage")
            compare(thumbnail.source.toString(), "", "Image source should be empty in placeholder mode")
        }
        
        function test_placeholder_differentBackgroundColor() {
            // Arrange - ensure mouse is away to avoid hover color change
            mouseMove(root, root.width - 1, root.height - 1)
            wait(100)
            clipCard.isPlaceholder = true
            wait(300) // Wait for color animation to complete
            
            // Assert
            var cardBackground = findChild(clipCard, "cardBackground")
            compare(cardBackground.color.toString(), mockTheme.surfaceSoft.toString(), "Background should be surfaceSoft for placeholder")
        }

        // =====================================================================
        // TEST: Duration Badge
        // =====================================================================
        
        function test_durationBadge_hasCorrectColor() {
            // Assert
            var durationBadge = findChild(clipCard, "durationBadge")
            compare(durationBadge.color.toString(), "#00000080", "Badge should have semi-transparent black background")
        }
        
        function test_durationBadge_height() {
            // Assert
            var durationBadge = findChild(clipCard, "durationBadge")
            compare(durationBadge.height, 20, "Badge height should be 20")
        }
        
        function test_durationBadge_radius() {
            // Assert
            var durationBadge = findChild(clipCard, "durationBadge")
            compare(durationBadge.radius, 10, "Badge radius should be 10 (pill shape)")
        }
        
        function test_durationBadge_textColor() {
            // Assert
            var durationText = findChild(clipCard, "durationText")
            compare(durationText.color.toString(), "#ffffff", "Duration text should be white")
        }
        
        function test_durationBadge_textIsBold() {
            // Assert
            var durationText = findChild(clipCard, "durationText")
            compare(durationText.font.bold, true, "Duration text should be bold")
        }
        
        function test_durationBadge_textFontSize() {
            // Assert
            var durationText = findChild(clipCard, "durationText")
            compare(durationText.font.pixelSize, 10, "Duration text font size should be 10")
        }
        
        function test_durationBadge_hiddenWhenEmpty() {
            // Arrange
            clipCard.duration = ""
            wait(50)
            
            // Assert
            var durationBadge = findChild(clipCard, "durationBadge")
            verify(!durationBadge.visible, "Duration badge should be hidden when duration is empty")
        }

        // =====================================================================
        // TEST: Text Styling
        // =====================================================================
        
        function test_clipTitleText_isBold() {
            // Assert
            var titleText = findChild(clipCard, "clipTitleText")
            verify(titleText !== null)
            compare(titleText.font.bold, true, "Clip title should be bold")
        }
        
        function test_clipTitleText_fontSize() {
            // Assert
            var titleText = findChild(clipCard, "clipTitleText")
            compare(titleText.font.pixelSize, 12, "Clip title font size should be 12")
        }
        
        function test_clipTitleText_color() {
            // Assert
            var titleText = findChild(clipCard, "clipTitleText")
            compare(titleText.color.toString(), mockTheme.primaryText.toString(), "Clip title should use primary text color")
        }
        
        function test_clipTitleText_maxLines() {
            // Assert
            var titleText = findChild(clipCard, "clipTitleText")
            compare(titleText.maximumLineCount, 2, "Clip title should have max 2 lines")
        }
        
        function test_broadcasterNameText_fontSize() {
            // Assert
            var broadcasterText = findChild(clipCard, "broadcasterNameText")
            compare(broadcasterText.font.pixelSize, 11, "Broadcaster name font size should be 11")
        }
        
        function test_broadcasterNameText_color() {
            // Assert
            var broadcasterText = findChild(clipCard, "broadcasterNameText")
            compare(broadcasterText.color.toString(), mockTheme.secondaryText.toString(), "Broadcaster name should use secondary text color")
        }
        
        function test_viewCountText_fontSize() {
            // Assert
            var viewText = findChild(clipCard, "viewCountText")
            compare(viewText.font.pixelSize, 10, "View count font size should be 10")
        }
        
        function test_viewCountText_color() {
            // Assert
            var viewText = findChild(clipCard, "viewCountText")
            compare(viewText.color.toString(), mockTheme.accent.toString(), "View count should use accent color")
        }

        // =====================================================================
        // TEST: MouseArea Configuration
        // =====================================================================
        
        function test_mouseArea_hoverEnabled() {
            var mouseArea = findChild(clipCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.hoverEnabled, true, "Hover should be enabled")
        }
        
        function test_mouseArea_cursorShape() {
            var mouseArea = findChild(clipCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.cursorShape, Qt.PointingHandCursor, "Cursor should be pointing hand")
        }
        
        function test_mouseArea_fillsParent() {
            var mouseArea = findChild(clipCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.width, clipCard.width, "MouseArea should fill card width")
            compare(mouseArea.height, clipCard.height, "MouseArea should fill card height")
        }

        // =====================================================================
        // TEST: Card Dimensions
        // =====================================================================
        
        function test_card_appliesCustomWidth() {
            // Arrange
            clipCard.cardWidth = 200
            wait(50)
            
            // Assert
            compare(clipCard.width, 200, "Custom cardWidth should apply")
        }
        
        function test_card_appliesCustomHeight() {
            // Arrange
            clipCard.cardHeight = 250
            wait(50)
            
            // Assert
            compare(clipCard.height, 250, "Custom cardHeight should apply")
        }
        
        function test_cardBackground_appliesRadius() {
            // Assert
            var cardBackground = findChild(clipCard, "cardBackground")
            compare(cardBackground.radius, clipCard.cardRadius, "Background radius should match cardRadius")
        }

        // =====================================================================
        // TEST: Thumbnail Zone
        // =====================================================================
        
        function test_thumbnailZone_exists() {
            var thumbnailZone = findChild(clipCard, "thumbnailZone")
            verify(thumbnailZone !== null, "Thumbnail zone should exist")
        }
        
        function test_thumbnailZone_height() {
            var thumbnailZone = findChild(clipCard, "thumbnailZone")
            compare(thumbnailZone.Layout.preferredHeight, 120, "Thumbnail zone height should be 120")
        }
        
        function test_thumbnailZone_hasClipping() {
            var thumbnailZone = findChild(clipCard, "thumbnailZone")
            compare(thumbnailZone.clip, true, "Thumbnail zone should clip content")
        }
        
        function test_thumbnailZone_radius() {
            var thumbnailZone = findChild(clipCard, "thumbnailZone")
            compare(thumbnailZone.radius, 8, "Thumbnail zone radius should be 8")
        }

        // =====================================================================
        // TEST: Thumbnail Image
        // =====================================================================
        
        function test_thumbnailImage_isAsynchronous() {
            var thumbnail = findChild(clipCard, "thumbnailImage")
            compare(thumbnail.asynchronous, true, "Thumbnail should load asynchronously")
        }
        
        function test_thumbnailImage_isCached() {
            var thumbnail = findChild(clipCard, "thumbnailImage")
            compare(thumbnail.cache, true, "Thumbnail should be cached")
        }
        
        function test_thumbnailImage_fillMode() {
            var thumbnail = findChild(clipCard, "thumbnailImage")
            compare(thumbnail.fillMode, Image.PreserveAspectCrop, "Thumbnail should preserve aspect and crop")
        }
    }
}
