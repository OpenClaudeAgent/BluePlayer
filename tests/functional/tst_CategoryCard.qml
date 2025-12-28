/**
 * tst_CategoryCard.qml
 * 
 * Functional UI tests for the CategoryCard component.
 * Tests category display cards showing category name, box art image,
 * hover effects, and click interaction with categoryId/categoryName params.
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
        readonly property string fontFamily: "Inter"
        readonly property int animCardDuration: 150
        readonly property int animContentFadeDuration: 200
    }

    // =========================================================================
    // Component Under Test (Mock) - Combines BaseCard + CategoryCard
    // =========================================================================
    
    Item {
        id: categoryCard
        objectName: "categoryCard"
        anchors.centerIn: parent
        
        // BaseCard properties
        property bool isPlaceholder: false
        property int cardWidth: 180
        property int cardHeight: 260
        property int cardRadius: 12
        property int contentMargins: 12
        
        // CategoryCard specific properties
        property string categoryName: ""
        property string categoryId: ""
        property string boxArtUrl: ""
        
        // Exposed state
        readonly property bool hovered: mouseArea.containsMouse
        
        // Signals
        signal cardClicked()
        signal categoryClicked(string categoryId, string categoryName)
        
        implicitWidth: cardWidth
        implicitHeight: cardHeight
        width: cardWidth
        height: cardHeight
        
        Rectangle {
            id: cardBackground
            objectName: "cardBackground"
            anchors.fill: parent
            radius: categoryCard.cardRadius
            color: categoryCard.isPlaceholder ? mockTheme.surfaceSoft : mockTheme.surface
            border.color: mockTheme.divider
            border.width: 1
            
            scale: mouseArea.containsMouse ? 1.02 : 1.0
            Behavior on scale {
                NumberAnimation { duration: mockTheme.animCardDuration; easing.type: Easing.OutCubic }
            }
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: categoryCard.contentMargins
                spacing: 12
                
                // Box art image zone (ratio 285x380 ~ 0.75)
                Rectangle {
                    id: imageContainer
                    objectName: "imageContainer"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    radius: 8
                    color: categoryCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                    border.color: mockTheme.divider
                    border.width: 1
                    clip: true
                    
                    Image {
                        id: boxArtImage
                        objectName: "boxArtImage"
                        anchors.fill: parent
                        source: categoryCard.isPlaceholder ? "" : categoryCard.boxArtUrl
                        fillMode: Image.PreserveAspectFit
                        asynchronous: true
                        cache: true
                        visible: status === Image.Ready && !categoryCard.isPlaceholder
                        
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
                        color: categoryCard.isPlaceholder ? "#1a2230" : mockTheme.surfaceSoft
                        visible: boxArtImage.status !== Image.Ready || categoryCard.isPlaceholder
                        
                        Text {
                            id: placeholderIcon
                            objectName: "placeholderIcon"
                            anchors.centerIn: parent
                            text: categoryCard.isPlaceholder ? "\u22EF" : (boxArtImage.status === Image.Loading ? "\u23F3" : "\uD83C\uDFAE")
                            font.pixelSize: 32
                            color: mockTheme.mutedText
                            opacity: 0.5
                        }
                    }
                }
                
                // Category name (centered)
                Text {
                    id: categoryNameText
                    objectName: "categoryNameText"
                    text: categoryCard.categoryName
                    font.family: mockTheme.fontFamily
                    font.pixelSize: 14
                    font.bold: true
                    color: mockTheme.primaryText
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
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
                if (!categoryCard.isPlaceholder) {
                    categoryCard.cardClicked()
                    categoryCard.categoryClicked(categoryCard.categoryId, categoryCard.categoryName)
                }
            }
        }
        
        // Reset function for tests
        function reset() {
            isPlaceholder = false
            categoryName = ""
            categoryId = ""
            boxArtUrl = ""
            cardWidth = 180
            cardHeight = 260
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: cardClickedSpy; target: categoryCard; signalName: "cardClicked" }
    SignalSpy { id: categoryClickedSpy; target: categoryCard; signalName: "categoryClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "CategoryCardTests"
        when: windowShown

        function init() {
            categoryCard.reset()
            cardClickedSpy.clear()
            categoryClickedSpy.clear()
            // Move mouse away from component to reset hover state
            mouseMove(root, 1, 1)
            wait(200)
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultState_dimensions() {
            compare(categoryCard.cardWidth, 180, "Default cardWidth should be 180")
            compare(categoryCard.cardHeight, 260, "Default cardHeight should be 260")
        }
        
        function test_defaultState_width() {
            compare(categoryCard.width, 180, "Width should match cardWidth")
        }
        
        function test_defaultState_height() {
            compare(categoryCard.height, 260, "Height should match cardHeight")
        }
        
        function test_defaultState_categoryNameEmpty() {
            compare(categoryCard.categoryName, "", "Default categoryName should be empty")
        }
        
        function test_defaultState_categoryIdEmpty() {
            compare(categoryCard.categoryId, "", "Default categoryId should be empty")
        }
        
        function test_defaultState_boxArtUrlEmpty() {
            compare(categoryCard.boxArtUrl, "", "Default boxArtUrl should be empty")
        }
        
        function test_defaultState_notPlaceholder() {
            compare(categoryCard.isPlaceholder, false, "Should not be placeholder by default")
        }
        
        function test_defaultState_cardRadius() {
            compare(categoryCard.cardRadius, 12, "Default cardRadius should be 12")
        }
        
        function test_defaultState_contentMargins() {
            compare(categoryCard.contentMargins, 12, "Default contentMargins should be 12")
        }

        // =====================================================================
        // TEST: Signal cardClicked
        // =====================================================================
        
        function test_click_emitsCardClickedSignal() {
            // Arrange
            var mouseArea = findChild(categoryCard, "mouseArea")
            verify(mouseArea !== null, "MouseArea should exist")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 1, "cardClicked signal should be emitted once")
        }
        
        function test_click_emitsCategoryClickedSignal() {
            // Arrange
            var mouseArea = findChild(categoryCard, "mouseArea")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(categoryClickedSpy.count, 1, "categoryClicked signal should be emitted once")
        }
        
        function test_click_emitsCategoryClickedWithCorrectParams() {
            // Arrange
            categoryCard.categoryId = "12345"
            categoryCard.categoryName = "Just Chatting"
            wait(50)
            
            // Act
            mouseClick(categoryCard)
            
            // Assert
            compare(categoryClickedSpy.count, 1, "categoryClicked should be emitted")
            compare(categoryClickedSpy.signalArguments[0][0], "12345", "First param should be categoryId")
            compare(categoryClickedSpy.signalArguments[0][1], "Just Chatting", "Second param should be categoryName")
        }
        
        function test_multipleClicks_emitMultiple() {
            // Arrange
            var mouseArea = findChild(categoryCard, "mouseArea")
            
            // Act
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 3, "Should emit cardClicked for each click")
            compare(categoryClickedSpy.count, 3, "Should emit categoryClicked for each click")
        }
        
        function test_clickOnCard_emitsSignal() {
            // Act - click directly on the card
            mouseClick(categoryCard)
            
            // Assert
            compare(cardClickedSpy.count, 1, "Clicking on card should emit cardClicked")
        }

        // =====================================================================
        // TEST: Placeholder State
        // =====================================================================
        
        function test_placeholder_noClickSignal() {
            // Arrange
            categoryCard.isPlaceholder = true
            wait(50)
            
            // Act
            mouseClick(categoryCard)
            
            // Assert
            compare(cardClickedSpy.count, 0, "Placeholder card should not emit click signal")
            compare(categoryClickedSpy.count, 0, "Placeholder card should not emit categoryClicked signal")
        }
        
        function test_placeholder_showsPlaceholderRect() {
            // Arrange
            categoryCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var placeholderRect = findChild(categoryCard, "placeholderRect")
            verify(placeholderRect !== null, "Placeholder rect should exist")
            verify(placeholderRect.visible, "Placeholder rect should be visible")
        }
        
        function test_placeholder_showsEllipsisIcon() {
            // Arrange
            categoryCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var icon = findChild(categoryCard, "placeholderIcon")
            verify(icon !== null, "Placeholder icon should exist")
            compare(icon.text, "\u22EF", "Should show ellipsis icon for placeholder")
        }
        
        function test_placeholder_backgroundUsesPlaceholderColor() {
            // Arrange
            categoryCard.isPlaceholder = true
            wait(50)
            
            // Assert
            var bg = findChild(categoryCard, "cardBackground")
            compare(bg.color.toString(), mockTheme.surfaceSoft.toString(), "Placeholder should use surfaceSoft color")
        }

        // =====================================================================
        // TEST: Category Name Display
        // =====================================================================
        
        function test_categoryName_displayed() {
            // Arrange
            categoryCard.categoryName = "Art"
            wait(50)
            
            // Assert
            var nameText = findChild(categoryCard, "categoryNameText")
            compare(nameText.text, "Art", "Category name should be displayed")
        }
        
        function test_categoryName_isBold() {
            // Assert
            var nameText = findChild(categoryCard, "categoryNameText")
            compare(nameText.font.bold, true, "Category name should be bold")
        }
        
        function test_categoryName_isCentered() {
            // Assert
            var nameText = findChild(categoryCard, "categoryNameText")
            compare(nameText.horizontalAlignment, Text.AlignHCenter, "Category name should be centered")
        }
        
        function test_categoryName_fontSize() {
            // Assert
            var nameText = findChild(categoryCard, "categoryNameText")
            compare(nameText.font.pixelSize, 14, "Category name font size should be 14")
        }
        
        function test_categoryName_elidesLongText() {
            // Assert
            var nameText = findChild(categoryCard, "categoryNameText")
            compare(nameText.elide, Text.ElideRight, "Long names should elide on the right")
        }
        
        function test_categoryName_usesCorrectColor() {
            // Assert
            var nameText = findChild(categoryCard, "categoryNameText")
            compare(nameText.color.toString(), mockTheme.primaryText.toString(), "Should use primaryText color")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_changesHoveredProperty() {
            // Arrange
            var mouseArea = findChild(categoryCard, "mouseArea")
            verify(mouseArea !== null)
            
            // Act - move mouse into the card
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert
            compare(categoryCard.hovered, true, "hovered should be true when mouse is over")
        }
        
        function test_hover_increasesScale() {
            // Arrange
            var mouseArea = findChild(categoryCard, "mouseArea")
            var cardBackground = findChild(categoryCard, "cardBackground")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200) // Wait for animation
            
            // Assert
            compare(cardBackground.scale, 1.02, "Scale should be 1.02 on hover")
        }
        
        function test_hover_restoresOnExit() {
            // Arrange
            var mouseArea = findChild(categoryCard, "mouseArea")
            var cardBackground = findChild(categoryCard, "cardBackground")
            
            // Act - hover then leave
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200)
            mouseMove(root, 1, 1)
            wait(200)
            
            // Assert
            compare(categoryCard.hovered, false, "hovered should be false after mouse leaves")
            compare(cardBackground.scale, 1.0, "Scale should return to 1.0")
        }
        
        // =====================================================================
        // TEST: MouseArea Configuration
        // =====================================================================
        
        function test_mouseArea_hoverEnabled() {
            var mouseArea = findChild(categoryCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.hoverEnabled, true, "Hover should be enabled")
        }
        
        function test_mouseArea_cursorShape() {
            var mouseArea = findChild(categoryCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.cursorShape, Qt.PointingHandCursor, "Cursor should be pointing hand")
        }
        
        function test_mouseArea_fillsParent() {
            var mouseArea = findChild(categoryCard, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.width, categoryCard.width, "MouseArea should fill card width")
            compare(mouseArea.height, categoryCard.height, "MouseArea should fill card height")
        }

        // =====================================================================
        // TEST: Image Container
        // =====================================================================
        
        function test_imageContainer_exists() {
            var container = findChild(categoryCard, "imageContainer")
            verify(container !== null, "Image container should exist")
        }
        
        function test_imageContainer_height() {
            var container = findChild(categoryCard, "imageContainer")
            compare(container.Layout.preferredHeight, 200, "Image container height should be 200")
        }
        
        function test_imageContainer_hasRoundedCorners() {
            var container = findChild(categoryCard, "imageContainer")
            compare(container.radius, 8, "Image container should have radius 8")
        }
        
        function test_imageContainer_clipsContent() {
            var container = findChild(categoryCard, "imageContainer")
            compare(container.clip, true, "Image container should clip content")
        }
        
        function test_imageContainer_hasBorder() {
            var container = findChild(categoryCard, "imageContainer")
            compare(container.border.width, 1, "Image container should have 1px border")
        }

        // =====================================================================
        // TEST: Card Background
        // =====================================================================
        
        function test_cardBackground_usesCardRadius() {
            var bg = findChild(categoryCard, "cardBackground")
            compare(bg.radius, categoryCard.cardRadius, "Background should use cardRadius")
        }
        
        function test_cardBackground_hasBorder() {
            var bg = findChild(categoryCard, "cardBackground")
            compare(bg.border.width, 1, "Background should have 1px border")
        }
        
        function test_cardBackground_normalColor() {
            // Arrange
            categoryCard.isPlaceholder = false
            wait(50)
            
            var bg = findChild(categoryCard, "cardBackground")
            compare(bg.color.toString(), mockTheme.surface.toString(), "Normal card should use surface color")
        }
        
        function test_cardBackground_placeholderColor() {
            // Arrange
            categoryCard.isPlaceholder = true
            wait(50)
            
            var bg = findChild(categoryCard, "cardBackground")
            compare(bg.color.toString(), mockTheme.surfaceSoft.toString(), "Placeholder should use surfaceSoft color")
        }
        
        function test_cardBackground_borderColor() {
            var bg = findChild(categoryCard, "cardBackground")
            compare(bg.border.color.toString(), mockTheme.divider.toString(), "Border should use divider color")
        }

        // =====================================================================
        // TEST: Box Art Image
        // =====================================================================
        
        function test_boxArtImage_exists() {
            var image = findChild(categoryCard, "boxArtImage")
            verify(image !== null, "Box art image should exist")
        }
        
        function test_boxArtImage_asynchronous() {
            var image = findChild(categoryCard, "boxArtImage")
            compare(image.asynchronous, true, "Image should load asynchronously")
        }
        
        function test_boxArtImage_cached() {
            var image = findChild(categoryCard, "boxArtImage")
            compare(image.cache, true, "Image should be cached")
        }
        
        function test_boxArtImage_fillMode() {
            var image = findChild(categoryCard, "boxArtImage")
            compare(image.fillMode, Image.PreserveAspectFit, "Image should preserve aspect ratio")
        }
        
        function test_boxArtImage_emptyForPlaceholder() {
            // Arrange
            categoryCard.isPlaceholder = true
            categoryCard.boxArtUrl = "http://example.com/boxart.jpg"
            wait(50)
            
            // Assert
            var image = findChild(categoryCard, "boxArtImage")
            compare(image.source, "", "Placeholder should have empty image source")
        }
        
        function test_boxArtImage_sourceSetFromUrl() {
            // Arrange
            categoryCard.isPlaceholder = false
            categoryCard.boxArtUrl = "http://example.com/boxart.jpg"
            wait(50)
            
            // Assert
            var image = findChild(categoryCard, "boxArtImage")
            compare(image.source.toString(), "http://example.com/boxart.jpg", "Image source should match boxArtUrl")
        }

        // =====================================================================
        // TEST: Custom Dimensions
        // =====================================================================
        
        function test_customWidth_applies() {
            // Arrange
            categoryCard.cardWidth = 220
            wait(50)
            
            // Assert
            compare(categoryCard.width, 220, "Custom width should apply")
        }
        
        function test_customHeight_applies() {
            // Arrange
            categoryCard.cardHeight = 300
            wait(50)
            
            // Assert
            compare(categoryCard.height, 300, "Custom height should apply")
        }

        // =====================================================================
        // TEST: Placeholder Icon States
        // =====================================================================
        
        function test_placeholderIcon_showsGamepadWhenNoImage() {
            // Arrange - normal state, no image loaded
            categoryCard.isPlaceholder = false
            categoryCard.boxArtUrl = ""
            wait(50)
            
            // Assert
            var icon = findChild(categoryCard, "placeholderIcon")
            compare(icon.text, "\uD83C\uDFAE", "Should show gamepad icon when no image")
        }
        
        function test_placeholderIcon_fontSizeAndOpacity() {
            // Assert
            var icon = findChild(categoryCard, "placeholderIcon")
            compare(icon.font.pixelSize, 32, "Icon font size should be 32")
            compare(icon.opacity, 0.5, "Icon opacity should be 0.5")
        }
        
        function test_placeholderIcon_usesCorrectColor() {
            // Assert
            var icon = findChild(categoryCard, "placeholderIcon")
            compare(icon.color.toString(), mockTheme.mutedText.toString(), "Icon should use mutedText color")
        }
    }
}
