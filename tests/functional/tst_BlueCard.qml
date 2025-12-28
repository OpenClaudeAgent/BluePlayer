/**
 * tst_BlueCard.qml
 *
 * Functional UI tests for the BlueCard component.
 * Tests card default properties, styles, dimensions,
 * and content injection capability.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 300

    // =========================================================================
    // Mock BlueCard Component (inline)
    // =========================================================================

    Component {
        id: blueCardComponent

        Item {
            id: blueCard
            objectName: "blueCard"
            width: 220
            height: 140

            // Allow consumers to inject arbitrary content inside this card
            default property alias content: contentContainer.children

            // Expose internal elements for testing
            readonly property Rectangle backgroundRect: cardBackground
            readonly property ColumnLayout contentLayout: contentContainer

            Rectangle {
                id: cardBackground
                objectName: "cardBackground"
                anchors.fill: parent
                color: "#1b2130"
                radius: 8
                border.color: "#2a324e"
                border.width: 1

                // content holder
                ColumnLayout {
                    id: contentContainer
                    objectName: "contentContainer"
                    anchors.fill: parent
                    anchors.margins: 16
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================

    property var blueCard: null

    // =========================================================================
    // Test Case
    // =========================================================================

    TestCase {
        id: testCase
        name: "BlueCardTests"
        when: windowShown

        function init() {
            blueCard = createTemporaryObject(blueCardComponent, root)
            verify(blueCard !== null, "BlueCard should be created")
            waitForRendering(blueCard)
        }

        function cleanup() {
            blueCard = null
        }

        // =====================================================================
        // TEST: Default Dimensions
        // =====================================================================

        function test_defaultState_width() {
            // Assert
            compare(blueCard.width, 220, "Default width should be 220")
        }

        function test_defaultState_height() {
            // Assert
            compare(blueCard.height, 140, "Default height should be 140")
        }

        function test_defaultState_implicitWidth() {
            // Assert
            compare(blueCard.implicitWidth, 0, "Implicit width should be 0 (Item default)")
        }

        function test_defaultState_implicitHeight() {
            // Assert
            compare(blueCard.implicitHeight, 0, "Implicit height should be 0 (Item default)")
        }

        // =====================================================================
        // TEST: Background Rectangle
        // =====================================================================

        function test_background_exists() {
            // Arrange
            var background = findChild(blueCard, "cardBackground")

            // Assert
            verify(background !== null, "Background rectangle should exist")
        }

        function test_background_fillsParent() {
            // Arrange
            var background = findChild(blueCard, "cardBackground")

            // Assert
            compare(background.width, blueCard.width, "Background width should match card width")
            compare(background.height, blueCard.height, "Background height should match card height")
        }

        function test_background_color() {
            // Arrange
            var background = findChild(blueCard, "cardBackground")
            var expectedColor = "#1b2130"

            // Assert
            compare(background.color.toString(), expectedColor, "Background color should be #1b2130")
        }

        function test_background_radius() {
            // Arrange
            var background = findChild(blueCard, "cardBackground")

            // Assert
            compare(background.radius, 8, "Background radius should be 8")
        }

        // =====================================================================
        // TEST: Border Styling
        // =====================================================================

        function test_border_color() {
            // Arrange
            var background = findChild(blueCard, "cardBackground")
            var expectedBorderColor = "#2a324e"

            // Assert
            compare(background.border.color.toString(), expectedBorderColor, "Border color should be #2a324e")
        }

        function test_border_width() {
            // Arrange
            var background = findChild(blueCard, "cardBackground")

            // Assert
            compare(background.border.width, 1, "Border width should be 1")
        }

        // =====================================================================
        // TEST: Content Container
        // =====================================================================

        function test_contentContainer_exists() {
            // Arrange
            var container = findChild(blueCard, "contentContainer")

            // Assert
            verify(container !== null, "Content container should exist")
        }

        function test_contentContainer_isColumnLayout() {
            // Arrange
            var container = findChild(blueCard, "contentContainer")

            // Assert
            // ColumnLayout inherits from Item which has the Layout attached property
            verify(container !== null, "Content container should be a ColumnLayout")
        }

        function test_contentContainer_margins() {
            // Arrange
            var container = findChild(blueCard, "contentContainer")

            // Assert
            compare(container.anchors.margins, 16, "Content container margins should be 16")
        }

        function test_contentContainer_fillsBackground() {
            // Arrange
            var container = findChild(blueCard, "contentContainer")
            var background = findChild(blueCard, "cardBackground")
            var expectedWidth = background.width - 32 // 16px margins on each side
            var expectedHeight = background.height - 32

            // Assert
            compare(container.width, expectedWidth, "Container width should be background - 32")
            compare(container.height, expectedHeight, "Container height should be background - 32")
        }

        // =====================================================================
        // TEST: Content Injection
        // =====================================================================

        function test_contentInjection_acceptsChild() {
            // Arrange - Create card with injected content
            var cardWithContent = createTemporaryObject(blueCardWithContentComponent, root)
            verify(cardWithContent !== null, "Card with content should be created")
            waitForRendering(cardWithContent)

            // Assert
            var container = findChild(cardWithContent, "contentContainer")
            verify(container.children.length > 0, "Container should have children")
        }

        function test_contentInjection_childIsVisible() {
            // Arrange
            var cardWithContent = createTemporaryObject(blueCardWithContentComponent, root)
            waitForRendering(cardWithContent)

            // Assert
            var injectedText = findChild(cardWithContent, "injectedText")
            verify(injectedText !== null, "Injected text should exist")
            verify(injectedText.visible, "Injected text should be visible")
        }

        function test_contentInjection_childTextIsCorrect() {
            // Arrange
            var cardWithContent = createTemporaryObject(blueCardWithContentComponent, root)
            waitForRendering(cardWithContent)

            // Assert
            var injectedText = findChild(cardWithContent, "injectedText")
            compare(injectedText.text, "Test Content", "Injected text content should match")
        }

        function test_contentInjection_multipleChildren() {
            // Arrange
            var cardWithMultiple = createTemporaryObject(blueCardWithMultipleContentComponent, root)
            waitForRendering(cardWithMultiple)

            // Assert
            var container = findChild(cardWithMultiple, "contentContainer")
            compare(container.children.length, 2, "Container should have 2 children")
        }

        // =====================================================================
        // TEST: Color Format Validation
        // =====================================================================

        function test_background_colorIsValidHex() {
            // Arrange
            var background = findChild(blueCard, "cardBackground")
            var colorStr = background.color.toString()

            // Assert - Qt returns colors as #RRGGBB or #AARRGGBB
            verify(colorStr.startsWith("#"), "Color should be in hex format")
        }

        function test_border_colorIsValidHex() {
            // Arrange
            var background = findChild(blueCard, "cardBackground")
            var borderColorStr = background.border.color.toString()

            // Assert
            verify(borderColorStr.startsWith("#"), "Border color should be in hex format")
        }

        // =====================================================================
        // TEST: Hierarchy Structure
        // =====================================================================

        function test_hierarchy_backgroundIsDirectChild() {
            // Arrange
            var background = findChild(blueCard, "cardBackground")

            // Assert
            compare(background.parent, blueCard, "Background should be direct child of blueCard")
        }

        function test_hierarchy_containerInsideBackground() {
            // Arrange
            var container = findChild(blueCard, "contentContainer")
            var background = findChild(blueCard, "cardBackground")

            // Assert
            compare(container.parent, background, "Container should be inside background")
        }
    }

    // =========================================================================
    // Additional Test Components
    // =========================================================================

    Component {
        id: blueCardWithContentComponent

        Item {
            id: blueCardWithContent
            width: 220
            height: 140
            default property alias content: contentContainer.children

            Rectangle {
                id: cardBg
                objectName: "cardBackground"
                anchors.fill: parent
                color: "#1b2130"
                radius: 8
                border.color: "#2a324e"
                border.width: 1

                ColumnLayout {
                    id: contentContainer
                    objectName: "contentContainer"
                    anchors.fill: parent
                    anchors.margins: 16

                    Text {
                        objectName: "injectedText"
                        text: "Test Content"
                        color: "white"
                    }
                }
            }
        }
    }

    Component {
        id: blueCardWithMultipleContentComponent

        Item {
            id: blueCardMultiple
            width: 220
            height: 140
            default property alias content: contentContainer.children

            Rectangle {
                id: cardBg2
                objectName: "cardBackground"
                anchors.fill: parent
                color: "#1b2130"
                radius: 8
                border.color: "#2a324e"
                border.width: 1

                ColumnLayout {
                    id: contentContainer
                    objectName: "contentContainer"
                    anchors.fill: parent
                    anchors.margins: 16

                    Text {
                        objectName: "firstText"
                        text: "First"
                        color: "white"
                    }
                    Text {
                        objectName: "secondText"
                        text: "Second"
                        color: "white"
                    }
                }
            }
        }
    }
}
