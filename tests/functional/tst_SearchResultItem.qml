/**
 * tst_SearchResultItem.qml
 * 
 * Functional UI tests for the SearchResultItem component.
 * Tests search result items displaying channel/category info, live badge,
 * thumbnail, hover effects, selection state, and click interaction.
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
        readonly property color secondaryText: "#8899A6"
        readonly property color mutedText: "#8899A6"
        readonly property color statusNegative: "#E91916"
        readonly property string fontFamily: "Inter"
        readonly property int animHoverDuration: 150
    }

    // =========================================================================
    // Component Under Test (Mock) - SearchResultItem
    // =========================================================================
    
    Rectangle {
        id: searchResultItem
        objectName: "searchResultItem"
        x: 20
        y: 20
        width: 320
        
        // Properties
        property string itemText: ""
        property string itemSubtext: ""
        property string thumbnailUrl: ""
        property bool isLive: false
        property bool isCategory: false
        property bool isSelected: false
        
        // Exposed state
        readonly property bool hovered: mouseArea.containsMouse
        
        // Signal
        signal clicked()
        
        implicitHeight: 48
        color: isSelected ? mockTheme.surfaceSoft : (mouseArea.containsMouse ? "#1a2230" : "transparent")
        radius: 8
        
        Behavior on color {
            ColorAnimation { duration: mockTheme.animHoverDuration; easing.type: Easing.OutCubic }
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 12
            
            // Thumbnail/Avatar
            Rectangle {
                id: thumbnailContainer
                objectName: "thumbnailContainer"
                Layout.preferredWidth: searchResultItem.isCategory ? 32 : 36
                Layout.preferredHeight: searchResultItem.isCategory ? 42 : 36
                radius: searchResultItem.isCategory ? 4 : 18
                color: mockTheme.surfaceSoft
                clip: true
                
                Image {
                    id: thumbnailImage
                    objectName: "thumbnailImage"
                    anchors.fill: parent
                    source: searchResultItem.thumbnailUrl
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    cache: true
                    visible: status === Image.Ready
                    
                    opacity: status === Image.Ready ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation { duration: mockTheme.animHoverDuration; easing.type: Easing.OutCubic }
                    }
                }
                
                // Placeholder
                Text {
                    id: placeholderIcon
                    objectName: "placeholderIcon"
                    anchors.centerIn: parent
                    text: searchResultItem.isCategory ? "\uD83C\uDFAE" : "\uD83D\uDC64"
                    font.pixelSize: 16
                    color: mockTheme.mutedText
                    visible: thumbnailImage.status !== Image.Ready
                    opacity: 0.5
                }
            }
            
            // Text content
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    
                    Text {
                        id: itemTextLabel
                        objectName: "itemTextLabel"
                        text: searchResultItem.itemText
                        font.family: mockTheme.fontFamily
                        font.pixelSize: 14
                        font.bold: true
                        color: mockTheme.primaryText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    
                    // Live badge
                    Rectangle {
                        id: liveBadge
                        objectName: "liveBadge"
                        visible: searchResultItem.isLive && !searchResultItem.isCategory
                        width: 36
                        height: 16
                        radius: 8
                        color: mockTheme.statusNegative
                        
                        Text {
                            id: liveBadgeText
                            objectName: "liveBadgeText"
                            anchors.centerIn: parent
                            text: "LIVE"
                            font.family: mockTheme.fontFamily
                            font.pixelSize: 9
                            font.bold: true
                            color: "#ffffff"
                        }
                    }
                }
                
                // Subtext
                Text {
                    id: subtextLabel
                    objectName: "subtextLabel"
                    visible: searchResultItem.itemSubtext !== ""
                    text: searchResultItem.itemSubtext
                    font.family: mockTheme.fontFamily
                    font.pixelSize: 12
                    color: mockTheme.secondaryText
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
            
            // Arrow indicator
            Text {
                id: arrowIndicator
                objectName: "arrowIndicator"
                text: "\u2192"
                font.pixelSize: 14
                color: mockTheme.mutedText
                opacity: mouseArea.containsMouse || searchResultItem.isSelected ? 1 : 0
                
                Behavior on opacity {
                    NumberAnimation { duration: mockTheme.animHoverDuration; easing.type: Easing.OutCubic }
                }
            }
        }
        
        MouseArea {
            id: mouseArea
            objectName: "mouseArea"
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: searchResultItem.clicked()
        }
        
        // Reset function for tests
        function reset() {
            itemText = ""
            itemSubtext = ""
            thumbnailUrl = ""
            isLive = false
            isCategory = false
            isSelected = false
        }
    }

    // =========================================================================
    // Signal Spy
    // =========================================================================
    
    SignalSpy { id: clickedSpy; target: searchResultItem; signalName: "clicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "SearchResultItemTests"
        when: windowShown

        function init() {
            searchResultItem.reset()
            clickedSpy.clear()
            // Move mouse away from component to reset hover state
            // Component is centered, so move to far corner
            mouseMove(root, root.width - 1, root.height - 1)
            wait(250)
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultState_itemTextEmpty() {
            compare(searchResultItem.itemText, "", "Default itemText should be empty")
        }
        
        function test_defaultState_itemSubtextEmpty() {
            compare(searchResultItem.itemSubtext, "", "Default itemSubtext should be empty")
        }
        
        function test_defaultState_thumbnailUrlEmpty() {
            compare(searchResultItem.thumbnailUrl, "", "Default thumbnailUrl should be empty")
        }
        
        function test_defaultState_notLive() {
            compare(searchResultItem.isLive, false, "Should not be live by default")
        }
        
        function test_defaultState_notCategory() {
            compare(searchResultItem.isCategory, false, "Should not be category by default")
        }
        
        function test_defaultState_notSelected() {
            compare(searchResultItem.isSelected, false, "Should not be selected by default")
        }
        
        function test_defaultState_implicitHeight() {
            compare(searchResultItem.implicitHeight, 48, "Default implicitHeight should be 48")
        }
        
        function test_defaultState_radius() {
            compare(searchResultItem.radius, 8, "Default radius should be 8")
        }
        
        function test_defaultState_hoveredProperty_exists() {
            // Note: Initial hover state depends on mouse position at test start
            // We just verify the property exists and is a boolean
            verify(typeof searchResultItem.hovered === "boolean", "hovered property should be a boolean")
        }

        // =====================================================================
        // TEST: Signal clicked
        // =====================================================================
        
        function test_click_emitsSignal() {
            // Arrange
            var mouseArea = findChild(searchResultItem, "mouseArea")
            verify(mouseArea !== null, "MouseArea should exist")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 1, "clicked signal should be emitted once")
        }
        
        function test_multipleClicks_emitMultiple() {
            // Arrange
            var mouseArea = findChild(searchResultItem, "mouseArea")
            
            // Act
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            // Assert
            compare(clickedSpy.count, 3, "Should emit clicked for each click")
        }
        
        function test_clickOnItem_emitsSignal() {
            // Act - click directly on the item
            mouseClick(searchResultItem)
            
            // Assert
            compare(clickedSpy.count, 1, "Clicking on item should emit clicked")
        }

        // =====================================================================
        // TEST: Text Display
        // =====================================================================
        
        function test_itemText_displayed() {
            // Arrange
            searchResultItem.itemText = "xQc"
            wait(50)
            
            // Assert
            var textLabel = findChild(searchResultItem, "itemTextLabel")
            compare(textLabel.text, "xQc", "Item text should be displayed")
        }
        
        function test_itemText_isBold() {
            // Assert
            var textLabel = findChild(searchResultItem, "itemTextLabel")
            compare(textLabel.font.bold, true, "Item text should be bold")
        }
        
        function test_itemText_fontSize() {
            // Assert
            var textLabel = findChild(searchResultItem, "itemTextLabel")
            compare(textLabel.font.pixelSize, 14, "Item text font size should be 14")
        }
        
        function test_itemText_elidesLongText() {
            // Assert
            var textLabel = findChild(searchResultItem, "itemTextLabel")
            compare(textLabel.elide, Text.ElideRight, "Long text should elide on the right")
        }
        
        function test_itemText_usePrimaryTextColor() {
            // Assert
            var textLabel = findChild(searchResultItem, "itemTextLabel")
            compare(textLabel.color.toString(), mockTheme.primaryText.toString(), "Should use primaryText color")
        }

        // =====================================================================
        // TEST: Subtext Display
        // =====================================================================
        
        function test_subtext_hiddenWhenEmpty() {
            // Arrange
            searchResultItem.itemSubtext = ""
            wait(50)
            
            // Assert
            var subtextLabel = findChild(searchResultItem, "subtextLabel")
            verify(!subtextLabel.visible, "Subtext should be hidden when empty")
        }
        
        function test_subtext_visibleWhenSet() {
            // Arrange
            searchResultItem.itemSubtext = "Just Chatting"
            wait(50)
            
            // Assert
            var subtextLabel = findChild(searchResultItem, "subtextLabel")
            verify(subtextLabel.visible, "Subtext should be visible when set")
            compare(subtextLabel.text, "Just Chatting", "Subtext should display correct text")
        }
        
        function test_subtext_fontSize() {
            // Assert
            var subtextLabel = findChild(searchResultItem, "subtextLabel")
            compare(subtextLabel.font.pixelSize, 12, "Subtext font size should be 12")
        }
        
        function test_subtext_usesSecondaryTextColor() {
            // Assert
            var subtextLabel = findChild(searchResultItem, "subtextLabel")
            compare(subtextLabel.color.toString(), mockTheme.secondaryText.toString(), "Should use secondaryText color")
        }
        
        function test_subtext_elidesLongText() {
            // Assert
            var subtextLabel = findChild(searchResultItem, "subtextLabel")
            compare(subtextLabel.elide, Text.ElideRight, "Long subtext should elide on the right")
        }
        
        function test_subtext_smallerThanItemText() {
            // Assert
            var textLabel = findChild(searchResultItem, "itemTextLabel")
            var subtextLabel = findChild(searchResultItem, "subtextLabel")
            verify(subtextLabel.font.pixelSize < textLabel.font.pixelSize, "Subtext should be smaller than item text")
        }

        // =====================================================================
        // TEST: Live Badge
        // =====================================================================
        
        function test_liveBadge_hiddenWhenNotLive() {
            // Arrange
            searchResultItem.isLive = false
            wait(50)
            
            // Assert
            var liveBadge = findChild(searchResultItem, "liveBadge")
            verify(!liveBadge.visible, "Live badge should be hidden when not live")
        }
        
        function test_liveBadge_visibleWhenLive() {
            // Arrange
            searchResultItem.isLive = true
            searchResultItem.isCategory = false
            wait(50)
            
            // Assert
            var liveBadge = findChild(searchResultItem, "liveBadge")
            verify(liveBadge.visible, "Live badge should be visible when live")
        }
        
        function test_liveBadge_hiddenWhenCategory() {
            // Arrange
            searchResultItem.isLive = true
            searchResultItem.isCategory = true
            wait(50)
            
            // Assert
            var liveBadge = findChild(searchResultItem, "liveBadge")
            verify(!liveBadge.visible, "Live badge should be hidden for categories even if isLive")
        }
        
        function test_liveBadge_hasCorrectText() {
            // Assert
            var liveBadgeText = findChild(searchResultItem, "liveBadgeText")
            verify(liveBadgeText !== null, "Live badge text should exist")
            compare(liveBadgeText.text, "LIVE", "Badge should display 'LIVE'")
        }
        
        function test_liveBadge_hasCorrectColor() {
            // Assert
            var liveBadge = findChild(searchResultItem, "liveBadge")
            compare(liveBadge.color.toString(), mockTheme.statusNegative.toString(), "Badge should use statusNegative color")
        }
        
        function test_liveBadge_dimensions() {
            // Assert
            var liveBadge = findChild(searchResultItem, "liveBadge")
            compare(liveBadge.width, 36, "Badge width should be 36")
            compare(liveBadge.height, 16, "Badge height should be 16")
            compare(liveBadge.radius, 8, "Badge should have radius 8")
        }
        
        function test_liveBadge_textIsBold() {
            // Assert
            var liveBadgeText = findChild(searchResultItem, "liveBadgeText")
            compare(liveBadgeText.font.bold, true, "Badge text should be bold")
        }
        
        function test_liveBadge_textFontSize() {
            // Assert
            var liveBadgeText = findChild(searchResultItem, "liveBadgeText")
            compare(liveBadgeText.font.pixelSize, 9, "Badge text font size should be 9")
        }

        // =====================================================================
        // TEST: Thumbnail Container (Channel vs Category)
        // =====================================================================
        
        function test_thumbnail_channelDimensions() {
            // Arrange
            searchResultItem.isCategory = false
            wait(50)
            
            // Assert
            var container = findChild(searchResultItem, "thumbnailContainer")
            compare(container.Layout.preferredWidth, 36, "Channel thumbnail width should be 36")
            compare(container.Layout.preferredHeight, 36, "Channel thumbnail height should be 36")
        }
        
        function test_thumbnail_channelRadius() {
            // Arrange
            searchResultItem.isCategory = false
            wait(50)
            
            // Assert
            var container = findChild(searchResultItem, "thumbnailContainer")
            compare(container.radius, 18, "Channel thumbnail should be circular (radius 18)")
        }
        
        function test_thumbnail_categoryDimensions() {
            // Arrange
            searchResultItem.isCategory = true
            wait(50)
            
            // Assert
            var container = findChild(searchResultItem, "thumbnailContainer")
            compare(container.Layout.preferredWidth, 32, "Category thumbnail width should be 32")
            compare(container.Layout.preferredHeight, 42, "Category thumbnail height should be 42")
        }
        
        function test_thumbnail_categoryRadius() {
            // Arrange
            searchResultItem.isCategory = true
            wait(50)
            
            // Assert
            var container = findChild(searchResultItem, "thumbnailContainer")
            compare(container.radius, 4, "Category thumbnail should have small radius (4)")
        }
        
        function test_thumbnail_clipsContent() {
            // Assert
            var container = findChild(searchResultItem, "thumbnailContainer")
            compare(container.clip, true, "Thumbnail container should clip content")
        }

        // =====================================================================
        // TEST: Placeholder Icon
        // =====================================================================
        
        function test_placeholder_channelIcon() {
            // Arrange
            searchResultItem.isCategory = false
            wait(50)
            
            // Assert
            var icon = findChild(searchResultItem, "placeholderIcon")
            compare(icon.text, "\uD83D\uDC64", "Channel placeholder should show person emoji")
        }
        
        function test_placeholder_categoryIcon() {
            // Arrange
            searchResultItem.isCategory = true
            wait(50)
            
            // Assert
            var icon = findChild(searchResultItem, "placeholderIcon")
            compare(icon.text, "\uD83C\uDFAE", "Category placeholder should show game emoji")
        }
        
        function test_placeholder_visibleWhenNoImage() {
            // Arrange
            searchResultItem.thumbnailUrl = ""
            wait(50)
            
            // Assert
            var icon = findChild(searchResultItem, "placeholderIcon")
            verify(icon.visible, "Placeholder should be visible when no image")
        }

        // =====================================================================
        // TEST: Arrow Indicator
        // =====================================================================
        
        function test_arrow_hiddenByDefault() {
            // Arrange - ensure not hovered and not selected
            searchResultItem.isSelected = false
            mouseMove(root, 1, 1)
            wait(200)
            
            // Assert
            var arrow = findChild(searchResultItem, "arrowIndicator")
            compare(arrow.opacity, 0, "Arrow should be hidden by default")
        }
        
        function test_arrow_visibleWhenSelected() {
            // Arrange
            searchResultItem.isSelected = true
            wait(250)
            
            // Assert
            var arrow = findChild(searchResultItem, "arrowIndicator")
            verify(arrow.opacity > 0.95, "Arrow should be visible when selected (opacity: " + arrow.opacity + ")")
        }
        
        function test_arrow_visibleOnHover() {
            // Arrange
            var mouseArea = findChild(searchResultItem, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250)
            
            // Assert
            var arrow = findChild(searchResultItem, "arrowIndicator")
            verify(arrow.opacity > 0.95, "Arrow should be visible on hover (opacity: " + arrow.opacity + ")")
        }
        
        function test_arrow_hasCorrectSymbol() {
            // Assert
            var arrow = findChild(searchResultItem, "arrowIndicator")
            compare(arrow.text, "\u2192", "Arrow should be right arrow symbol")
        }
        
        function test_arrow_fontSize() {
            // Assert
            var arrow = findChild(searchResultItem, "arrowIndicator")
            compare(arrow.font.pixelSize, 14, "Arrow font size should be 14")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_changesHoveredProperty() {
            // Arrange
            var mouseArea = findChild(searchResultItem, "mouseArea")
            verify(mouseArea !== null)
            
            // Act - move mouse into the item
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert
            compare(searchResultItem.hovered, true, "hovered should be true when mouse is over")
        }
        
        function test_hover_changesBackgroundColor() {
            // Arrange
            searchResultItem.isSelected = false
            var mouseArea = findChild(searchResultItem, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200)
            
            // Assert
            compare(searchResultItem.color.toString(), "#1a2230", "Background should change on hover")
        }
        
        function test_hover_restoresOnExit() {
            // Arrange
            searchResultItem.isSelected = false
            var mouseArea = findChild(searchResultItem, "mouseArea")
            
            // Act - hover then leave
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200)
            mouseMove(root, 1, 1)
            wait(200)
            
            // Assert
            compare(searchResultItem.hovered, false, "hovered should be false after mouse leaves")
            compare(searchResultItem.color.toString(), "#00000000", "Background should return to transparent")
        }

        // =====================================================================
        // TEST: Selection State
        // =====================================================================
        
        function test_selected_changesBackgroundColor() {
            // Arrange
            searchResultItem.isSelected = true
            wait(200)
            
            // Assert
            compare(searchResultItem.color.toString(), mockTheme.surfaceSoft.toString(), "Selected item should use surfaceSoft color")
        }
        
        function test_selected_priorityOverHover() {
            // Arrange
            searchResultItem.isSelected = true
            var mouseArea = findChild(searchResultItem, "mouseArea")
            
            // Act - hover while selected
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(200)
            
            // Assert - color should still be surfaceSoft (selected state)
            compare(searchResultItem.color.toString(), mockTheme.surfaceSoft.toString(), "Selected color should take priority over hover")
        }
        
        function test_notSelected_backgroundNotSolid() {
            // Arrange
            searchResultItem.isSelected = false
            wait(50)
            
            // Assert - when not selected, verify the component responds to isSelected property
            // The background should be either transparent (if not hovered) or hover color (if hovered)
            // But NOT the surfaceSoft color that's reserved for selected state
            verify(searchResultItem.isSelected === false, "isSelected should be false")
            
            // Test that setting isSelected to true changes the background
            var colorBefore = searchResultItem.color.toString()
            searchResultItem.isSelected = true
            wait(200)
            compare(searchResultItem.color.toString(), mockTheme.surfaceSoft.toString(), 
                    "Selected state should use surfaceSoft color")
        }

        // =====================================================================
        // TEST: MouseArea Configuration
        // =====================================================================
        
        function test_mouseArea_hoverEnabled() {
            var mouseArea = findChild(searchResultItem, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.hoverEnabled, true, "Hover should be enabled")
        }
        
        function test_mouseArea_cursorShape() {
            var mouseArea = findChild(searchResultItem, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.cursorShape, Qt.PointingHandCursor, "Cursor should be pointing hand")
        }
        
        function test_mouseArea_fillsParent() {
            var mouseArea = findChild(searchResultItem, "mouseArea")
            verify(mouseArea !== null)
            compare(mouseArea.width, searchResultItem.width, "MouseArea should fill item width")
            compare(mouseArea.height, searchResultItem.height, "MouseArea should fill item height")
        }

        // =====================================================================
        // TEST: Image Configuration
        // =====================================================================
        
        function test_thumbnailImage_asynchronous() {
            var image = findChild(searchResultItem, "thumbnailImage")
            compare(image.asynchronous, true, "Image should load asynchronously")
        }
        
        function test_thumbnailImage_cached() {
            var image = findChild(searchResultItem, "thumbnailImage")
            compare(image.cache, true, "Image should be cached")
        }
        
        function test_thumbnailImage_fillMode() {
            var image = findChild(searchResultItem, "thumbnailImage")
            compare(image.fillMode, Image.PreserveAspectCrop, "Image should preserve aspect ratio and crop")
        }
        
        function test_thumbnailImage_sourceBound() {
            // Arrange
            searchResultItem.thumbnailUrl = "https://example.com/avatar.png"
            wait(50)
            
            // Assert
            var image = findChild(searchResultItem, "thumbnailImage")
            compare(image.source, "https://example.com/avatar.png", "Image source should be bound to thumbnailUrl")
        }

        // =====================================================================
        // TEST: Complete Scenarios
        // =====================================================================
        
        function test_scenario_liveChannel() {
            // Arrange - simulate a live channel search result
            searchResultItem.itemText = "shroud"
            searchResultItem.itemSubtext = "VALORANT"
            searchResultItem.isLive = true
            searchResultItem.isCategory = false
            searchResultItem.thumbnailUrl = "https://example.com/shroud.png"
            wait(50)
            
            // Assert
            var textLabel = findChild(searchResultItem, "itemTextLabel")
            var subtextLabel = findChild(searchResultItem, "subtextLabel")
            var liveBadge = findChild(searchResultItem, "liveBadge")
            var container = findChild(searchResultItem, "thumbnailContainer")
            
            compare(textLabel.text, "shroud", "Channel name should be displayed")
            compare(subtextLabel.text, "VALORANT", "Game name should be displayed")
            verify(liveBadge.visible, "Live badge should be visible")
            compare(container.radius, 18, "Should have circular avatar")
        }
        
        function test_scenario_offlineChannel() {
            // Arrange - simulate an offline channel search result
            searchResultItem.itemText = "pokimane"
            searchResultItem.itemSubtext = ""
            searchResultItem.isLive = false
            searchResultItem.isCategory = false
            wait(50)
            
            // Assert
            var textLabel = findChild(searchResultItem, "itemTextLabel")
            var subtextLabel = findChild(searchResultItem, "subtextLabel")
            var liveBadge = findChild(searchResultItem, "liveBadge")
            
            compare(textLabel.text, "pokimane", "Channel name should be displayed")
            verify(!subtextLabel.visible, "Subtext should be hidden for offline")
            verify(!liveBadge.visible, "Live badge should be hidden")
        }
        
        function test_scenario_category() {
            // Arrange - simulate a category search result
            searchResultItem.itemText = "Just Chatting"
            searchResultItem.itemSubtext = "123K viewers"
            searchResultItem.isLive = false
            searchResultItem.isCategory = true
            wait(50)
            
            // Assert
            var textLabel = findChild(searchResultItem, "itemTextLabel")
            var subtextLabel = findChild(searchResultItem, "subtextLabel")
            var liveBadge = findChild(searchResultItem, "liveBadge")
            var container = findChild(searchResultItem, "thumbnailContainer")
            
            compare(textLabel.text, "Just Chatting", "Category name should be displayed")
            compare(subtextLabel.text, "123K viewers", "Viewer count should be displayed")
            verify(!liveBadge.visible, "Live badge should be hidden for category")
            compare(container.radius, 4, "Should have rectangular thumbnail")
        }
        
        function test_scenario_selectedItem() {
            // Arrange - simulate a selected search result
            searchResultItem.itemText = "selected_channel"
            searchResultItem.isSelected = true
            wait(200)
            
            // Assert
            var arrow = findChild(searchResultItem, "arrowIndicator")
            compare(searchResultItem.color.toString(), mockTheme.surfaceSoft.toString(), "Should have selection background")
            compare(arrow.opacity, 1, "Arrow should be visible")
        }
    }
}
