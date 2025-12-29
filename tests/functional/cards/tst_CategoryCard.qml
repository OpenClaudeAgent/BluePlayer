/**
 * tst_CategoryCard.qml
 * 
 * Functional UI tests for the CategoryCard component.
 * Tests category data display, click signal, and box art display.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 400
    height: 400

    // Detect offscreen mode
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"

    // =========================================================================
    // Mock Theme Constants
    // =========================================================================
    
    QtObject {
        id: mockTheme
        readonly property color primaryText: "#e6edf3"
        readonly property color mutedText: "#6e7681"
        readonly property color surface: "#0d1117"
        readonly property color surfaceSoft: "#161b22"
        readonly property color divider: "#30363d"
        readonly property color cardHighlight: "#1c2128"
        readonly property int animCardDuration: 1
        readonly property int animContentFadeDuration: 1
        readonly property string fontFamily: "Inter"
    }

    // =========================================================================
    // Component Under Test (Mock of CategoryCard)
    // =========================================================================
    
    Component {
        id: categoryCardComponent
        
        Item {
            id: cardRoot
            objectName: "categoryCard"

            // BaseCard properties
            property bool isPlaceholder: false
            property int cardWidth: 180
            property int cardHeight: 260
            readonly property alias hovered: mouseArea.containsMouse

            // CategoryCard specific properties
            property string categoryName: ""
            property string categoryId: ""
            property string boxArtUrl: ""

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
                radius: 12
                color: cardRoot.isPlaceholder ? mockTheme.surfaceSoft : mockTheme.surface
                border.color: mockTheme.divider
                border.width: 1

                states: [
                    State {
                        name: "hovered"
                        when: mouseArea.containsMouse
                        PropertyChanges {
                            target: cardBackground
                            scale: 1.02
                        }
                    }
                ]

                transitions: Transition {
                    NumberAnimation {
                        properties: "scale"
                        duration: mockTheme.animCardDuration
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    // Box art area (taller for game covers)
                    Rectangle {
                        id: boxArtArea
                        objectName: "boxArtArea"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 200
                        radius: 8
                        color: cardRoot.isPlaceholder ? mockTheme.cardHighlight : mockTheme.surfaceSoft
                        border.color: mockTheme.divider
                        border.width: 1
                        clip: true

                        // Placeholder text
                        Text {
                            id: placeholderText
                            objectName: "placeholderText"
                            anchors.centerIn: parent
                            text: cardRoot.isPlaceholder ? "⋯" : "🎮"
                            font.pixelSize: 32
                            color: mockTheme.mutedText
                            opacity: 0.5
                            visible: cardRoot.isPlaceholder || cardRoot.boxArtUrl === ""
                        }
                    }

                    // Category name
                    Text {
                        id: categoryNameText
                        objectName: "categoryNameText"
                        text: cardRoot.categoryName
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
                    if (!cardRoot.isPlaceholder) {
                        cardRoot.cardClicked()
                        cardRoot.categoryClicked(cardRoot.categoryId, cardRoot.categoryName)
                    }
                }
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var card: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: cardClickedSpy; signalName: "cardClicked" }
    SignalSpy { id: categoryClickedSpy; signalName: "categoryClicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "CategoryCardTests"
        when: windowShown

        function init() {
            card = createTemporaryObject(categoryCardComponent, root)
            verify(card !== null, "CategoryCard should be created")
            card.anchors.centerIn = root
            cardClickedSpy.target = card
            categoryClickedSpy.target = card
            cardClickedSpy.clear()
            categoryClickedSpy.clear()
            mouseMove(root, 1, 1)
        }

        function cleanup() {
            card = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_notPlaceholder() {
            compare(card.isPlaceholder, false, "Default isPlaceholder should be false")
        }
        
        function test_defaultState_emptyData() {
            compare(card.categoryName, "", "Default categoryName should be empty")
            compare(card.categoryId, "", "Default categoryId should be empty")
            compare(card.boxArtUrl, "", "Default boxArtUrl should be empty")
        }
        
        function test_defaultState_dimensions() {
            compare(card.cardWidth, 180, "Default width should be 180")
            compare(card.cardHeight, 260, "Default height should be 260 (taller for box art)")
        }

        // =====================================================================
        // TEST: Data Display
        // =====================================================================
        
        function test_categoryName_displayed() {
            var nameText = findChild(card, "categoryNameText")
            
            card.categoryName = "Just Chatting"
            
            compare(nameText.text, "Just Chatting", "Category name should be displayed")
        }
        
        function test_categoryName_centered() {
            var nameText = findChild(card, "categoryNameText")
            
            compare(nameText.horizontalAlignment, Text.AlignHCenter, "Category name should be centered")
        }
        
        function test_categoryName_longText_elided() {
            var nameText = findChild(card, "categoryNameText")
            card.categoryName = "This Is A Very Long Category Name That Should Be Elided"
            
            compare(nameText.elide, Text.ElideRight, "Long names should be elided")
        }

        // =====================================================================
        // TEST: Box Art Area
        // =====================================================================
        
        function test_boxArtArea_tallerHeight() {
            var boxArtArea = findChild(card, "boxArtArea")
            
            compare(boxArtArea.Layout.preferredHeight, 200, "Box art area should be 200px tall")
        }
        
        function test_boxArtArea_showsPlaceholderIcon() {
            var placeholderText = findChild(card, "placeholderText")
            
            card.boxArtUrl = ""
            
            compare(placeholderText.text, "🎮", "Should show game controller emoji as placeholder")
            compare(placeholderText.visible, true, "Placeholder should be visible without image")
        }
        
        function test_boxArtArea_loadingPlaceholder() {
            var placeholderText = findChild(card, "placeholderText")
            
            card.isPlaceholder = true
            
            compare(placeholderText.text, "⋯", "Should show loading indicator for placeholder")
        }

        // =====================================================================
        // TEST: Click Signal
        // =====================================================================
        
        function test_click_emitsCardClicked() {
            var mouseArea = findChild(card, "mouseArea")
            
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 1, "cardClicked should be emitted")
        }
        
        function test_click_emitsCategoryClicked() {
            var mouseArea = findChild(card, "mouseArea")
            card.categoryId = "509658"
            card.categoryName = "Just Chatting"
            
            mouseClick(mouseArea)
            
            compare(categoryClickedSpy.count, 1, "categoryClicked should be emitted")
        }
        
        function test_click_passesCorrectArguments() {
            var mouseArea = findChild(card, "mouseArea")
            card.categoryId = "21779"
            card.categoryName = "League of Legends"
            
            mouseClick(mouseArea)
            
            var args = categoryClickedSpy.signalArguments[0]
            compare(args[0], "21779", "categoryId should be passed")
            compare(args[1], "League of Legends", "categoryName should be passed")
        }
        
        function test_click_noSignalWhenPlaceholder() {
            var mouseArea = findChild(card, "mouseArea")
            card.isPlaceholder = true
            card.categoryId = "123"
            
            mouseClick(mouseArea)
            
            compare(categoryClickedSpy.count, 0, "Should not emit when placeholder")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_setsHoveredTrue() {
            var mouseArea = findChild(card, "mouseArea")
            
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(card, "hovered", true, 100, "hovered should be true")
        }
        
        function test_hoverExit_restoresHoveredFalse() {
            var mouseArea = findChild(card, "mouseArea")
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            tryCompare(card, "hovered", true, 100)
            
            mouseMove(root, 1, 1)
            
            tryCompare(card, "hovered", false, 100, "hovered should be false after exit")
        }

        // =====================================================================
        // TEST: Popular Categories
        // =====================================================================
        
        function test_popularCategory_justChatting() {
            card.categoryId = "509658"
            card.categoryName = "Just Chatting"
            
            var nameText = findChild(card, "categoryNameText")
            compare(nameText.text, "Just Chatting")
        }
        
        function test_popularCategory_valorant() {
            card.categoryId = "516575"
            card.categoryName = "VALORANT"
            
            var nameText = findChild(card, "categoryNameText")
            compare(nameText.text, "VALORANT")
        }
        
        function test_popularCategory_minecraft() {
            card.categoryId = "27471"
            card.categoryName = "Minecraft"
            
            var nameText = findChild(card, "categoryNameText")
            compare(nameText.text, "Minecraft")
        }
    }
}
