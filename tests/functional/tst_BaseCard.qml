/**
 * tst_BaseCard.qml
 * 
 * Functional UI tests for the BaseCard component.
 * Tests base card properties, hover state, click signal,
 * placeholder mode, and visual styles.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
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
        readonly property int animCardDuration: 200
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
            wait(100)
            waitForRendering(baseCard)
        }

        function cleanup() {
            baseCard = null
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================
        
        function test_defaultProperty_isPlaceholderFalse() {
            // Assert
            compare(baseCard.isPlaceholder, false, "Default isPlaceholder should be false")
        }
        
        function test_defaultProperty_cardWidth() {
            // Assert
            compare(baseCard.cardWidth, 180, "Default cardWidth should be 180")
        }
        
        function test_defaultProperty_cardHeight() {
            // Assert
            compare(baseCard.cardHeight, 220, "Default cardHeight should be 220")
        }
        
        function test_defaultProperty_cardRadius() {
            // Assert
            compare(baseCard.cardRadius, 12, "Default cardRadius should be 12")
        }
        
        function test_defaultProperty_contentMargins() {
            // Assert
            compare(baseCard.contentMargins, 12, "Default contentMargins should be 12")
        }
        
        function test_defaultProperty_implicitWidth() {
            // Assert
            compare(baseCard.implicitWidth, 180, "implicitWidth should equal cardWidth")
        }
        
        function test_defaultProperty_implicitHeight() {
            // Assert
            compare(baseCard.implicitHeight, 220, "implicitHeight should equal cardHeight")
        }
        
        function test_defaultProperty_hoveredFalse() {
            // Assert
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
            wait(50)
            
            // Assert
            compare(baseCard.hovered, true, "hovered should be true when mouse is over")
        }
        
        function test_hover_increasesScale() {
            // Arrange
            var cardBackground = findChild(baseCard, "cardBackground")
            var mouseArea = findChild(baseCard, "mouseArea")
            compare(cardBackground.scale, 1.0, "Initial scale should be 1.0")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250) // Wait for animation
            
            // Assert
            compare(cardBackground.scale, 1.02, "Scale should be 1.02 on hover")
        }
        
        function test_hover_showsShadow() {
            // Arrange
            var cardShadow = findChild(baseCard, "cardShadow")
            var mouseArea = findChild(baseCard, "mouseArea")
            compare(cardShadow.opacity, 0, "Initial shadow opacity should be 0")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250) // Wait for animation
            
            // Assert
            compare(cardShadow.opacity, 0.3, "Shadow opacity should be 0.3 on hover")
        }
        
        function test_hover_changesBackgroundColor() {
            // Arrange
            var cardBackground = findChild(baseCard, "cardBackground")
            var mouseArea = findChild(baseCard, "mouseArea")
            var initialColor = cardBackground.color.toString()
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250) // Wait for animation
            
            // Assert
            verify(cardBackground.color.toString() !== initialColor, "Background color should change on hover")
            compare(cardBackground.color.toString(), "#1a2330", "Hovered color should be #1a2330")
        }
        
        function test_hoverExit_restoresHoveredFalse() {
            // Arrange
            var mouseArea = findChild(baseCard, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(100)
            verify(baseCard.hovered, "Should be hovered initially")
            
            // Act
            mouseMove(root, 1, 1)
            wait(150)
            
            // Assert
            compare(baseCard.hovered, false, "hovered should be false after mouse leaves")
        }
        
        function test_hoverExit_restoresScale() {
            // Arrange
            var cardBackground = findChild(baseCard, "cardBackground")
            var mouseArea = findChild(baseCard, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250)
            compare(cardBackground.scale, 1.02, "Should be scaled on hover")
            
            // Act
            mouseMove(root, 1, 1)
            wait(250)
            
            // Assert
            compare(cardBackground.scale, 1.0, "Scale should return to 1.0")
        }
        
        function test_hoverExit_hidesShadow() {
            // Arrange
            var cardShadow = findChild(baseCard, "cardShadow")
            var mouseArea = findChild(baseCard, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250)
            compare(cardShadow.opacity, 0.3, "Shadow should be visible on hover")
            
            // Act
            mouseMove(root, 1, 1)
            wait(250)
            
            // Assert
            compare(cardShadow.opacity, 0, "Shadow opacity should return to 0")
        }

        // =====================================================================
        // TEST: Placeholder Mode
        // =====================================================================
        
        function test_placeholder_changesBackgroundColor() {
            // Arrange - ensure mouse is away
            mouseMove(root, root.width - 1, root.height - 1)
            wait(100)
            
            // Act
            baseCard.isPlaceholder = true
            wait(250) // Wait for color animation
            
            // Assert
            var cardBackground = findChild(baseCard, "cardBackground")
            compare(cardBackground.color.toString(), mockTheme.surfaceSoft.toString(), 
                    "Placeholder background should be surfaceSoft")
        }
        
        function test_placeholder_hoverChangesColor() {
            // Arrange
            baseCard.isPlaceholder = true
            var cardBackground = findChild(baseCard, "cardBackground")
            var mouseArea = findChild(baseCard, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250)
            
            // Assert
            compare(cardBackground.color.toString(), "#252d3d", 
                    "Placeholder hover color should be #252d3d")
        }
        
        function test_placeholder_stillHovers() {
            // Arrange
            baseCard.isPlaceholder = true
            var mouseArea = findChild(baseCard, "mouseArea")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(50)
            
            // Assert - hover state works but click doesn't
            compare(baseCard.hovered, true, "Placeholder should still show hover state")
        }
        
        function test_placeholder_noClickButStillHoverable() {
            // Arrange
            baseCard.isPlaceholder = true
            var mouseArea = findChild(baseCard, "mouseArea")
            var cardBackground = findChild(baseCard, "cardBackground")
            
            // Act
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            wait(250)
            mouseClick(mouseArea)
            
            // Assert
            compare(cardClickedSpy.count, 0, "No click signal for placeholder")
            compare(cardBackground.scale, 1.02, "But scale should still change on hover")
        }

        // =====================================================================
        // TEST: Visual Styles
        // =====================================================================
        
        function test_background_initialColor() {
            // Arrange - ensure no hover
            mouseMove(root, 1, 1)
            wait(150)
            
            // Assert
            var cardBackground = findChild(baseCard, "cardBackground")
            compare(cardBackground.color.toString(), mockTheme.surface.toString(), 
                    "Initial background should be surface color")
        }
        
        function test_background_hasRadius() {
            // Assert
            var cardBackground = findChild(baseCard, "cardBackground")
            compare(cardBackground.radius, 12, "Background radius should match cardRadius")
        }
        
        function test_background_hasBorder() {
            // Assert
            var cardBackground = findChild(baseCard, "cardBackground")
            compare(cardBackground.border.width, 1, "Border width should be 1")
            compare(cardBackground.border.color.toString(), mockTheme.divider.toString(), 
                    "Border color should be divider color")
        }
        
        function test_shadow_initiallyHidden() {
            // Assert
            var cardShadow = findChild(baseCard, "cardShadow")
            compare(cardShadow.opacity, 0, "Shadow should be hidden initially")
        }
        
        function test_shadow_hasCorrectRadius() {
            // Assert
            var cardShadow = findChild(baseCard, "cardShadow")
            var cardBackground = findChild(baseCard, "cardBackground")
            compare(cardShadow.radius, cardBackground.radius + 2, 
                    "Shadow radius should be background radius + 2")
        }
        
        function test_shadow_hasCorrectMargins() {
            // Assert
            var cardShadow = findChild(baseCard, "cardShadow")
            compare(cardShadow.anchors.margins, -2, "Shadow margins should be -2")
        }

        // =====================================================================
        // TEST: Content Container
        // =====================================================================
        
        function test_contentContainer_exists() {
            // Assert
            var contentContainer = findChild(baseCard, "contentContainer")
            verify(contentContainer !== null, "Content container should exist")
        }
        
        function test_contentContainer_hasCorrectMargins() {
            // Assert
            var contentContainer = findChild(baseCard, "contentContainer")
            compare(contentContainer.anchors.margins, 12, 
                    "Content container margins should match contentMargins")
        }
        
        function test_contentContainer_updatesWithCustomMargins() {
            // Arrange
            baseCard.contentMargins = 20
            waitForRendering(baseCard)
            
            // Assert
            var contentContainer = findChild(baseCard, "contentContainer")
            compare(contentContainer.anchors.margins, 20, 
                    "Content container margins should update")
        }

        // =====================================================================
        // TEST: MouseArea Configuration
        // =====================================================================
        
        function test_mouseArea_exists() {
            // Assert
            var mouseArea = findChild(baseCard, "mouseArea")
            verify(mouseArea !== null, "MouseArea should exist")
        }
        
        function test_mouseArea_hoverEnabled() {
            // Assert
            var mouseArea = findChild(baseCard, "mouseArea")
            compare(mouseArea.hoverEnabled, true, "Hover should be enabled")
        }
        
        function test_mouseArea_cursorShape() {
            // Assert
            var mouseArea = findChild(baseCard, "mouseArea")
            compare(mouseArea.cursorShape, Qt.PointingHandCursor, 
                    "Cursor should be pointing hand")
        }
        
        function test_mouseArea_fillsParent() {
            // Assert
            var mouseArea = findChild(baseCard, "mouseArea")
            compare(mouseArea.width, baseCard.width, "MouseArea should fill card width")
            compare(mouseArea.height, baseCard.height, "MouseArea should fill card height")
        }

        // =====================================================================
        // TEST: Custom Dimensions
        // =====================================================================
        
        function test_customWidth_appliedToCard() {
            // Arrange
            baseCard.cardWidth = 250
            waitForRendering(baseCard)
            
            // Assert
            compare(baseCard.width, 250, "Custom width should apply")
            compare(baseCard.implicitWidth, 250, "implicitWidth should update")
        }
        
        function test_customHeight_appliedToCard() {
            // Arrange
            baseCard.cardHeight = 300
            waitForRendering(baseCard)
            
            // Assert
            compare(baseCard.height, 300, "Custom height should apply")
            compare(baseCard.implicitHeight, 300, "implicitHeight should update")
        }
        
        function test_customRadius_appliedToBackground() {
            // Arrange
            baseCard.cardRadius = 20
            waitForRendering(baseCard)
            
            // Assert
            var cardBackground = findChild(baseCard, "cardBackground")
            compare(cardBackground.radius, 20, "Custom radius should apply to background")
        }
        
        function test_customRadius_updatesShadowRadius() {
            // Arrange
            baseCard.cardRadius = 16
            waitForRendering(baseCard)
            
            // Assert
            var cardShadow = findChild(baseCard, "cardShadow")
            compare(cardShadow.radius, 18, "Shadow radius should be cardRadius + 2")
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
            wait(50)
            
            // Assert
            compare(cardBackground.state, "hovered", "State should be 'hovered' when mouse over")
        }
        
        function test_defaultState_isEmpty() {
            // Arrange - ensure no hover
            mouseMove(root, 1, 1)
            wait(150)
            
            // Assert
            var cardBackground = findChild(baseCard, "cardBackground")
            compare(cardBackground.state, "", "Default state should be empty string")
        }
    }
}
