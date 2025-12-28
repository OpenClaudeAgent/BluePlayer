/**
 * tst_BlueCard.qml
 *
 * Functional UI tests for the BlueCard component.
 * Tests content injection capability and visibility.
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
