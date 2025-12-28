/**
 * tst_BaseCard.qml
 * 
 * Functional UI tests for the BaseCard component.
 * Tests hover state, click signal, and placeholder mode behavior.
 * 
 * Refactored to use:
 * - createTemporaryObject for test isolation
 * - cleanup() for proper teardown
 * - waitForRendering instead of wait() for visual sync
 * - tryCompare for async property checks
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root

    // Detect offscreen mode - mouse events crash in offscreen
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"
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
        // Reduced for faster test execution (real value is 200)
        readonly property int animCardDuration: 1
    }

    // =========================================================================
    // Component Under Test (Mock of BaseCard)
    // =========================================================================
    
    Component {
        id: baseCardComponent
        
        Item {
            id: baseCard
            objectName: "baseCard"

            // Props communes
            property bool isPlaceholder: false
            property int cardWidth: 180
            property int cardHeight: 220
            property int cardRadius: 12
            property int contentMargins: 12

            // Alias pour permettre aux enfants d'acceder au hover state
            readonly property alias hovered: mouseArea.containsMouse

            // Signal generique
            signal cardClicked()

            // Alias pour le contenu
            default property alias content: contentContainer.data

            implicitWidth: cardWidth
            implicitHeight: cardHeight
            width: cardWidth
            height: cardHeight

            Rectangle {
                id: cardBackground
                objectName: "cardBackground"
                anchors.fill: parent
                radius: baseCard.cardRadius
                color: baseCard.isPlaceholder ? mockTheme.surfaceSoft : mockTheme.surface
                border.color: mockTheme.divider
                border.width: 1

                // Hover effect
                states: [
                    State {
                        name: "hovered"
                        when: mouseArea.containsMouse
                        PropertyChanges {
                            target: cardBackground
                            color: baseCard.isPlaceholder ? "#252d3d" : "#1a2330"
                            scale: 1.02
                        }
                        PropertyChanges {
                            target: cardShadow
                            opacity: 0.3
                        }
                    }
                ]

                transitions: Transition {
                    NumberAnimation {
                        properties: "scale, opacity"
                        duration: mockTheme.animCardDuration
                        easing.type: Easing.OutCubic
                    }
                    ColorAnimation {
                        duration: mockTheme.animCardDuration
                        easing.type: Easing.OutCubic
                    }
                }

                // Ombre subtile
                Rectangle {
                    id: cardShadow
                    objectName: "cardShadow"
                    anchors.fill: parent
                    anchors.margins: -2
                    radius: parent.radius + 2
                    color: "transparent"
                    border.color: "#00000020"
                    border.width: 1
                    opacity: 0
                    z: -1
                }

                Item {
                    id: contentContainer
                    objectName: "contentContainer"
                    anchors.fill: parent
                    anchors.margins: baseCard.contentMargins
                }
            }

            MouseArea {
                id: mouseArea
                objectName: "mouseArea"
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (!baseCard.isPlaceholder) {
                        baseCard.cardClicked()
                    }
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var baseCard: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: cardClickedSpy; signalName: "cardClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "BaseCardTests"
        when: windowShown

        function init() {
            baseCard = createTemporaryObject(baseCardComponent, root)
            verify(baseCard !== null, "BaseCard should be created")
            baseCard.anchors.centerIn = root
            cardClickedSpy.target = baseCard
            cardClickedSpy.clear()
            // Move mouse away from component to reset hover state
            mouseMove(root, 1, 1)
            waitForRendering(baseCard)
        }

        function cleanup() {
            baseCard = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_isPlaceholderFalse() {
            compare(baseCard.isPlaceholder, false, "Default isPlaceholder should be false")
        }
        
        function test_defaultState_hoveredFalse() {
            compare(baseCard.hovered, false, "Default hovered should be false")
        }

        // =====================================================================
        // TEST: Signal cardClicked
        // =====================================================================
        
        function test_click_emitsCardClicked() {
            // Arrange
            var mouseArea = findChild(baseCard, "mouseArea")
            verify(mouseArea !== null, "MouseArea should exist")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 1, "cardClicked should be emitted once")
        }
        
        function test_click_noSignalWhenPlaceholder() {
            // Arrange
            baseCard.isPlaceholder = true
            var mouseArea = findChild(baseCard, "mouseArea")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 0, "cardClicked should NOT be emitted for placeholder")
        }
        
        function test_multipleClicks_emitMultipleSignals() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            var mouseArea = findChild(baseCard, "mouseArea")
            
            // Act
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 3, "Should emit cardClicked for each click")
        }
        
        function test_click_worksAfterPlaceholderDisabled() {
            if (root.isOffscreen) { skip("Mouse events not supported in offscreen mode"); return }
            // Arrange
            baseCard.isPlaceholder = true
            var mouseArea = findChild(baseCard, "mouseArea")
            mouseClick(mouseArea)
            compare(cardClickedSpy.count, 0, "No signal while placeholder")
            
            // Act - disable placeholder mode
            baseCard.isPlaceholder = false
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 1, "Should emit after placeholder disabled")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_setsHoveredTrue() {
            // Arrange
            var mouseArea = findChild(baseCard, "mouseArea")
            verify(mouseArea !== null)
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            waitForRendering(baseCard)
            
            // Assert
            tryCompare(baseCard, "hovered", true, 100, "hovered should be true when mouse is over")
        }
        
        function test_hoverExit_restoresHoveredFalse() {
            // Arrange
            var mouseArea = findChild(baseCard, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            waitForRendering(baseCard)
            tryCompare(baseCard, "hovered", true, 100, "Should be hovered initially")
            
            // Act
            mouseMove(root, 1, 1)
            waitForRendering(baseCard)
            
            // Assert
            tryCompare(baseCard, "hovered", false, 100, "hovered should be false after mouse leaves")
        }

        // =====================================================================
        // TEST: Placeholder Mode
        // =====================================================================
        
        function test_placeholder_stillHovers() {
            // Arrange
            baseCard.isPlaceholder = true
            var mouseArea = findChild(baseCard, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            waitForRendering(baseCard)
            
            // Assert - hover state works but click doesn't
            tryCompare(baseCard, "hovered", true, 100, "Placeholder should still show hover state")
        }
        
        function test_placeholder_noClickButStillHoverable() {
            // Arrange
            baseCard.isPlaceholder = true
            var mouseArea = findChild(baseCard, "mouseArea")
            var cardBackground = findChild(baseCard, "cardBackground")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            tryCompare(baseCard, "hovered", true, 100, "Should be hovered")
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 0, "No click signal for placeholder")
        }

        // =====================================================================
        // TEST: State Transitions
        // =====================================================================
        
        function test_hoverState_isNamedCorrectly() {
            // Arrange
            var cardBackground = findChild(baseCard, "cardBackground")
            var mouseArea = findChild(baseCard, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            waitForRendering(baseCard)
            
            // Assert
            tryCompare(cardBackground, "state", "hovered", 100, "State should be 'hovered' when mouse over")
        }
        
        function test_defaultState_isEmpty() {
            // Arrange - ensure no hover
            mouseMove(root, 1, 1)
            waitForRendering(baseCard)
            
            // Assert
            var cardBackground = findChild(baseCard, "cardBackground")
            compare(cardBackground.state, "", "Default state should be empty string")
        }
    }
}
