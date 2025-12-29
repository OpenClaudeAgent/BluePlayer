/**
 * tst_CategoriesScroller.qml
 *
 * Functional UI tests for the CategoriesScroller component.
 * Tests model binding, scroll behavior, and delegate content display.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 800
    height: 400

    // =========================================================================
    // Mock Theme Constants
    // =========================================================================

    QtObject {
        id: mockTheme
        readonly property color primaryText: "#FFFFFF"
        readonly property color secondaryText: "#8899A6"
        readonly property color accent: "#0066FF"
        readonly property color surface: "#1b2130"
        readonly property color divider: "#2a324e"
        readonly property int spacingSmall: 8
    }

    // =========================================================================
    // Component Under Test (Mock) - CategoriesScroller
    // =========================================================================

    Item {
        id: categoriesScroller
        objectName: "categoriesScroller"
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20

        // Public properties
        property string sectionTitle: ""
        property string sectionSubtitle: ""
        property var cardsModel: null
        property real availableWidth: 600

        // Computed properties for testing
        readonly property int cardCount: cardsModel ? cardsModel.length : 0

        width: availableWidth
        height: columnLayout.implicitHeight

        ColumnLayout {
            id: columnLayout
            objectName: "columnLayout"
            width: categoriesScroller.availableWidth
            spacing: mockTheme.spacingSmall

            Text {
                id: titleText
                objectName: "titleText"
                text: categoriesScroller.sectionTitle
                color: mockTheme.primaryText
                font.bold: true
                font.pixelSize: 14
            }

            Text {
                id: subtitleText
                objectName: "subtitleText"
                text: categoriesScroller.sectionSubtitle
                color: mockTheme.secondaryText
                font.pixelSize: 12
            }

            Flickable {
                id: flickable
                objectName: "flickable"
                width: parent.width
                height: 150
                contentWidth: categoriesScroller.cardsModel ? categoriesScroller.cardsModel.length * 210 : 0
                clip: true
                flickableDirection: Flickable.HorizontalFlick

                Row {
                    id: cardsRow
                    objectName: "cardsRow"
                    spacing: 10

                    Repeater {
                        id: cardsRepeater
                        objectName: "cardsRepeater"
                        model: categoriesScroller.cardsModel

                        delegate: Item {
                            id: cardDelegate
                            objectName: "cardDelegate_" + index
                            width: 200
                            height: 140

                            Rectangle {
                                id: cardBackground
                                objectName: "cardBackground_" + index
                                anchors.fill: parent
                                color: mockTheme.surface
                                radius: 8
                                border.color: mockTheme.divider
                                border.width: 1

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: mockTheme.spacingSmall

                                    Text {
                                        id: nameLabel
                                        objectName: "nameLabel_" + index
                                        text: modelData.name
                                        color: mockTheme.primaryText
                                        font.pixelSize: 12
                                        font.bold: true
                                    }

                                    Text {
                                        id: detailLabel
                                        objectName: "detailLabel_" + index
                                        text: modelData.detail
                                        color: mockTheme.secondaryText
                                        font.pixelSize: 11
                                        wrapMode: Text.WordWrap
                                    }

                                    Text {
                                        id: viewersLabel
                                        objectName: "viewersLabel_" + index
                                        text: modelData.viewers
                                        color: mockTheme.accent
                                        font.pixelSize: 11
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Reset function for tests
        function reset() {
            sectionTitle = ""
            sectionSubtitle = ""
            cardsModel = null
            availableWidth = 600
        }
    }

    // =========================================================================
    // Test Data
    // =========================================================================

    property var emptyModel: []

    property var singleItemModel: [
        { name: "Just Chatting", detail: "Social category", viewers: "250K viewers" }
    ]

    property var multipleItemsModel: [
        { name: "Just Chatting", detail: "Social category", viewers: "250K viewers" },
        { name: "Fortnite", detail: "Battle Royale", viewers: "180K viewers" },
        { name: "League of Legends", detail: "MOBA game", viewers: "150K viewers" },
        { name: "Valorant", detail: "Tactical shooter", viewers: "120K viewers" },
        { name: "Minecraft", detail: "Sandbox game", viewers: "100K viewers" }
    ]

    // =========================================================================
    // Test Case
    // =========================================================================

    TestCase {
        id: testCase
        name: "CategoriesScrollerTests"
        when: windowShown

        function init() {
            categoriesScroller.reset()
            // Skip waitForRendering in init - each test will wait if needed
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================

        function test_defaultState_sectionTitleEmpty() {
            compare(categoriesScroller.sectionTitle, "", "Default sectionTitle should be empty")
        }

        function test_defaultState_sectionSubtitleEmpty() {
            compare(categoriesScroller.sectionSubtitle, "", "Default sectionSubtitle should be empty")
        }

        function test_defaultState_cardsModelNull() {
            compare(categoriesScroller.cardsModel, null, "Default cardsModel should be null")
        }

        function test_defaultState_cardCount() {
            compare(categoriesScroller.cardCount, 0, "Default cardCount should be 0")
        }

        // =====================================================================
        // TEST: Section Title Display
        // =====================================================================

        function test_sectionTitle_displayed() {
            // Arrange
            categoriesScroller.sectionTitle = "Top Categories"
            // waitForRendering removed for perf

            // Assert
            var titleText = findChild(categoriesScroller, "titleText")
            verify(titleText !== null, "Title text should exist")
            compare(titleText.text, "Top Categories", "Title should be displayed")
        }

        // =====================================================================
        // TEST: Section Subtitle Display
        // =====================================================================

        function test_sectionSubtitle_displayed() {
            // Arrange
            categoriesScroller.sectionSubtitle = "Browse by game"
            // waitForRendering removed for perf

            // Assert
            var subtitleText = findChild(categoriesScroller, "subtitleText")
            verify(subtitleText !== null, "Subtitle text should exist")
            compare(subtitleText.text, "Browse by game", "Subtitle should be displayed")
        }

        // =====================================================================
        // TEST: Flickable Configuration
        // =====================================================================

        function test_flickable_exists() {
            var flickable = findChild(categoriesScroller, "flickable")
            verify(flickable !== null, "Flickable should exist")
        }

        function test_flickable_horizontalDirection() {
            var flickable = findChild(categoriesScroller, "flickable")
            compare(flickable.flickableDirection, Flickable.HorizontalFlick, "Should only allow horizontal flicking")
        }

        function test_flickable_contentWidthWithEmptyModel() {
            // Arrange
            categoriesScroller.cardsModel = emptyModel
            // waitForRendering removed for perf

            // Assert
            var flickable = findChild(categoriesScroller, "flickable")
            compare(flickable.contentWidth, 0, "Content width should be 0 with empty model")
        }

        function test_flickable_contentWidthWithMultipleItems() {
            // Arrange
            categoriesScroller.cardsModel = multipleItemsModel
            // waitForRendering removed for perf

            // Assert
            var flickable = findChild(categoriesScroller, "flickable")
            var expectedWidth = 5 * 210  // 210 per card
            compare(flickable.contentWidth, expectedWidth, "Content width should be 1050 for 5 items")
        }

        // =====================================================================
        // TEST: Model Binding
        // =====================================================================

        function test_model_emptyModelShowsNoCards() {
            // Arrange
            categoriesScroller.cardsModel = emptyModel
            // waitForRendering removed for perf

            // Assert
            compare(categoriesScroller.cardCount, 0, "Card count should be 0")
            var repeater = findChild(categoriesScroller, "cardsRepeater")
            compare(repeater.count, 0, "Repeater count should be 0")
        }

        function test_model_singleItemCreatesOneCard() {
            // Arrange
            categoriesScroller.cardsModel = singleItemModel
            // waitForRendering removed for perf

            // Assert
            compare(categoriesScroller.cardCount, 1, "Card count should be 1")
            var repeater = findChild(categoriesScroller, "cardsRepeater")
            compare(repeater.count, 1, "Repeater should have 1 item")
        }

        function test_model_multipleItemsCreateMultipleCards() {
            // Arrange
            categoriesScroller.cardsModel = multipleItemsModel
            // waitForRendering removed for perf

            // Assert
            compare(categoriesScroller.cardCount, 5, "Card count should be 5")
            var repeater = findChild(categoriesScroller, "cardsRepeater")
            compare(repeater.count, 5, "Repeater should have 5 items")
        }

        function test_model_changingModelUpdatesCards() {
            // Arrange
            categoriesScroller.cardsModel = singleItemModel
            // waitForRendering removed for perf
            compare(categoriesScroller.cardCount, 1, "Initial count should be 1")

            // Act
            categoriesScroller.cardsModel = multipleItemsModel
            // waitForRendering removed for perf

            // Assert
            compare(categoriesScroller.cardCount, 5, "Count should update to 5")
        }

        function test_model_settingNullClearsCards() {
            // Arrange
            categoriesScroller.cardsModel = multipleItemsModel
            // waitForRendering removed for perf
            compare(categoriesScroller.cardCount, 5, "Initial count should be 5")

            // Act
            categoriesScroller.cardsModel = null
            // waitForRendering removed for perf

            // Assert
            compare(categoriesScroller.cardCount, 0, "Count should be 0 after null")
        }

        // =====================================================================
        // TEST: Delegate Content
        // =====================================================================

        function test_delegate_nameDisplayed() {
            // Arrange
            categoriesScroller.cardsModel = singleItemModel
            // waitForRendering removed for perf

            // Assert
            var nameLabel = findChild(categoriesScroller, "nameLabel_0")
            verify(nameLabel !== null, "Name label should exist")
            compare(nameLabel.text, "Just Chatting", "Name should be displayed")
        }

        function test_delegate_detailDisplayed() {
            // Arrange
            categoriesScroller.cardsModel = singleItemModel
            // waitForRendering removed for perf

            // Assert
            var detailLabel = findChild(categoriesScroller, "detailLabel_0")
            verify(detailLabel !== null, "Detail label should exist")
            compare(detailLabel.text, "Social category", "Detail should be displayed")
        }

        function test_delegate_viewersDisplayed() {
            // Arrange
            categoriesScroller.cardsModel = singleItemModel
            // waitForRendering removed for perf

            // Assert
            var viewersLabel = findChild(categoriesScroller, "viewersLabel_0")
            verify(viewersLabel !== null, "Viewers label should exist")
            compare(viewersLabel.text, "250K viewers", "Viewers should be displayed")
        }

        function test_delegate_secondItemHasCorrectData() {
            // Arrange
            categoriesScroller.cardsModel = multipleItemsModel
            // waitForRendering removed for perf

            // Assert
            var nameLabel = findChild(categoriesScroller, "nameLabel_1")
            compare(nameLabel.text, "Fortnite", "Second item name should be Fortnite")

            var detailLabel = findChild(categoriesScroller, "detailLabel_1")
            compare(detailLabel.text, "Battle Royale", "Second item detail should be Battle Royale")

            var viewersLabel = findChild(categoriesScroller, "viewersLabel_1")
            compare(viewersLabel.text, "180K viewers", "Second item viewers should be 180K")
        }

        // =====================================================================
        // TEST: Scroll Behavior
        // =====================================================================

        function test_scroll_initialPositionAtZero() {
            // Arrange
            categoriesScroller.cardsModel = multipleItemsModel
            // waitForRendering removed for perf

            // Assert
            var flickable = findChild(categoriesScroller, "flickable")
            compare(flickable.contentX, 0, "Initial scroll position should be 0")
        }

        function test_scroll_canScrollHorizontally() {
            // Arrange
            categoriesScroller.cardsModel = multipleItemsModel
            categoriesScroller.availableWidth = 400  // Less than content width
            // waitForRendering removed for perf

            var flickable = findChild(categoriesScroller, "flickable")
            var maxScroll = flickable.contentWidth - flickable.width

            // Act - simulate scroll by setting contentX
            flickable.contentX = 100
            // waitForRendering removed for perf

            // Assert
            compare(flickable.contentX, 100, "Should be able to scroll horizontally")
            verify(maxScroll > 0, "Content should be scrollable (wider than visible area)")
        }

        // =====================================================================
        // TEST: Data-Driven Tests
        // =====================================================================

        function test_multipleCards_allHaveCorrectIndex_data() {
            return [
                { tag: "first", index: 0, expectedName: "Just Chatting" },
                { tag: "second", index: 1, expectedName: "Fortnite" },
                { tag: "third", index: 2, expectedName: "League of Legends" },
                { tag: "fourth", index: 3, expectedName: "Valorant" },
                { tag: "fifth", index: 4, expectedName: "Minecraft" }
            ]
        }

        function test_multipleCards_allHaveCorrectIndex(data) {
            // Arrange
            categoriesScroller.cardsModel = multipleItemsModel
            // waitForRendering removed for perf

            // Assert
            var nameLabel = findChild(categoriesScroller, "nameLabel_" + data.index)
            verify(nameLabel !== null, "Card at index " + data.index + " should exist")
            compare(nameLabel.text, data.expectedName, "Card " + data.index + " should show " + data.expectedName)
        }

        // =====================================================================
        // TEST: Edge Cases
        // =====================================================================

        function test_edgeCase_veryLongTitle() {
            // Arrange
            categoriesScroller.sectionTitle = "This is a very long section title that might overflow"
            // waitForRendering removed for perf

            // Assert
            var titleText = findChild(categoriesScroller, "titleText")
            compare(titleText.text, "This is a very long section title that might overflow", "Long title should be displayed")
        }

        function test_edgeCase_specialCharactersInTitle() {
            // Arrange
            categoriesScroller.sectionTitle = "Categories & Games <Test>"
            // waitForRendering removed for perf

            // Assert
            var titleText = findChild(categoriesScroller, "titleText")
            compare(titleText.text, "Categories & Games <Test>", "Special characters should be displayed")
        }

        function test_edgeCase_emptyStringsInModel() {
            // Arrange
            categoriesScroller.cardsModel = [
                { name: "", detail: "", viewers: "" }
            ]
            // waitForRendering removed for perf

            // Assert
            var nameLabel = findChild(categoriesScroller, "nameLabel_0")
            compare(nameLabel.text, "", "Empty name should be handled")
        }
    }
}
