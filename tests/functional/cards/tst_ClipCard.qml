/**
 * tst_ClipCard.qml
 * 
 * Functional UI tests for the ClipCard component.
 * Tests clip data display, duration badge, and placeholder mode.
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
    // Mock Theme Constants (inline)
    // =========================================================================
    
    QtObject {
        id: mockTheme
        readonly property color primaryText: "#e6edf3"
        readonly property color secondaryText: "#8b949e"
        readonly property color mutedText: "#6e7681"
        readonly property color accent: "#3b82f6"
        readonly property color surface: "#0d1117"
        readonly property color surfaceSoft: "#161b22"
        readonly property color divider: "#30363d"
        readonly property color cardHighlight: "#1a2230"
        readonly property int animCardDuration: 1
        readonly property int animContentFadeDuration: 1
        readonly property string fontFamily: "Inter"
    }

    // =========================================================================
    // Component Under Test (Mock of ClipCard)
    // =========================================================================
    
    Component {
        id: clipCardComponent
        
        Item {
            id: cardRoot
            objectName: "clipCard"

            // BaseCard properties
            property bool isPlaceholder: false
            property int cardWidth: 200
            property int cardHeight: 220
            readonly property alias hovered: mouseArea.containsMouse

            // ClipCard specific properties
            property string clipTitle: ""
            property string broadcasterName: ""
            property string viewCount: ""
            property string duration: ""
            property string thumbnailUrl: ""

            // Signal
            signal cardClicked()

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

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    // Thumbnail area
                    Rectangle {
                        id: thumbnailArea
                        objectName: "thumbnailArea"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 120
                        radius: 8
                        color: cardRoot.isPlaceholder ? mockTheme.cardHighlight : mockTheme.surfaceSoft
                        border.color: mockTheme.divider
                        border.width: 1
                        clip: true

                        // Simulated thumbnail image status
                        property bool thumbnailReady: cardRoot.thumbnailUrl !== "" && !cardRoot.isPlaceholder

                        // Placeholder text
                        Text {
                            id: placeholderText
                            objectName: "placeholderText"
                            anchors.centerIn: parent
                            text: cardRoot.isPlaceholder ? "..." : "🎬"
                            font.pixelSize: 32
                            color: mockTheme.mutedText
                            opacity: 0.5
                            visible: cardRoot.isPlaceholder || cardRoot.thumbnailUrl === ""
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
                            visible: !cardRoot.isPlaceholder && cardRoot.duration !== "" && thumbnailArea.thumbnailReady

                            Text {
                                id: durationText
                                objectName: "durationText"
                                anchors.centerIn: parent
                                text: cardRoot.duration
                                font.pixelSize: 10
                                font.bold: true
                                color: "#fff"
                            }
                        }
                    }

                    // Clip info
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            id: titleText
                            objectName: "titleText"
                            text: cardRoot.clipTitle
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
                            id: broadcasterText
                            objectName: "broadcasterText"
                            text: cardRoot.broadcasterName
                            font.family: mockTheme.fontFamily
                            font.pixelSize: 11
                            color: mockTheme.secondaryText
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        Text {
                            id: viewCountText
                            objectName: "viewCountText"
                            text: cardRoot.viewCount !== "" ? cardRoot.viewCount + " vues" : ""
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
                    if (!cardRoot.isPlaceholder) {
                        cardRoot.cardClicked()
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

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "ClipCardTests"
        when: windowShown

        function init() {
            card = createTemporaryObject(clipCardComponent, root)
            verify(card !== null, "ClipCard should be created")
            card.anchors.centerIn = root
            cardClickedSpy.target = card
            cardClickedSpy.clear()
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
            compare(card.clipTitle, "", "Default clipTitle should be empty")
            compare(card.broadcasterName, "", "Default broadcasterName should be empty")
            compare(card.viewCount, "", "Default viewCount should be empty")
            compare(card.duration, "", "Default duration should be empty")
            compare(card.thumbnailUrl, "", "Default thumbnailUrl should be empty")
        }
        
        function test_defaultState_dimensions() {
            compare(card.cardWidth, 200, "Default cardWidth should be 200")
            compare(card.cardHeight, 220, "Default cardHeight should be 220")
        }
        
        function test_defaultState_notHovered() {
            compare(card.hovered, false, "Default hovered should be false")
        }

        // =====================================================================
        // TEST: Data Display - Clip Title
        // =====================================================================
        
        function test_clipTitle_displayed() {
            var titleText = findChild(card, "titleText")
            
            card.clipTitle = "Amazing Play of the Day"
            
            compare(titleText.text, "Amazing Play of the Day", "Clip title should be displayed")
        }
        
        function test_clipTitle_emptyWhenNotSet() {
            var titleText = findChild(card, "titleText")
            
            compare(titleText.text, "", "Clip title should be empty when not set")
        }

        // =====================================================================
        // TEST: Data Display - Broadcaster Name
        // =====================================================================
        
        function test_broadcasterName_displayed() {
            var broadcasterText = findChild(card, "broadcasterText")
            
            card.broadcasterName = "TopStreamer"
            
            compare(broadcasterText.text, "TopStreamer", "Broadcaster name should be displayed")
        }
        
        function test_broadcasterName_emptyWhenNotSet() {
            var broadcasterText = findChild(card, "broadcasterText")
            
            compare(broadcasterText.text, "", "Broadcaster name should be empty when not set")
        }

        // =====================================================================
        // TEST: Data Display - View Count
        // =====================================================================
        
        function test_viewCount_displayedWithSuffix() {
            var viewText = findChild(card, "viewCountText")
            
            card.viewCount = "50K"
            
            compare(viewText.text, "50K vues", "View count should be displayed with 'vues' suffix")
        }
        
        function test_viewCount_emptyWhenNoCount() {
            var viewText = findChild(card, "viewCountText")
            
            card.viewCount = ""
            
            compare(viewText.text, "", "View count text should be empty when no count")
        }
        
        function test_viewCount_differentFormats_data() {
            return [
                { tag: "thousands", input: "1.2K", expected: "1.2K vues" },
                { tag: "millions", input: "2M", expected: "2M vues" },
                { tag: "exact", input: "456", expected: "456 vues" }
            ]
        }
        
        function test_viewCount_differentFormats(data) {
            var viewText = findChild(card, "viewCountText")
            
            card.viewCount = data.input
            
            compare(viewText.text, data.expected, "View count should format correctly: " + data.tag)
        }

        // =====================================================================
        // TEST: Duration Badge
        // =====================================================================
        
        function test_durationBadge_visibleWithDurationAndThumbnail() {
            var durationBadge = findChild(card, "durationBadge")
            
            card.duration = "0:30"
            card.thumbnailUrl = "https://example.com/thumb.jpg"
            
            compare(durationBadge.visible, true, "Duration badge should be visible with duration and thumbnail")
        }
        
        function test_durationBadge_hiddenWithoutDuration() {
            var durationBadge = findChild(card, "durationBadge")
            
            card.duration = ""
            card.thumbnailUrl = "https://example.com/thumb.jpg"
            
            compare(durationBadge.visible, false, "Duration badge should be hidden without duration")
        }
        
        function test_durationBadge_hiddenWithoutThumbnail() {
            var durationBadge = findChild(card, "durationBadge")
            
            card.duration = "0:45"
            card.thumbnailUrl = ""
            
            compare(durationBadge.visible, false, "Duration badge should be hidden without thumbnail")
        }
        
        function test_durationBadge_hiddenWhenPlaceholder() {
            var durationBadge = findChild(card, "durationBadge")
            
            card.duration = "1:00"
            card.thumbnailUrl = "https://example.com/thumb.jpg"
            card.isPlaceholder = true
            
            compare(durationBadge.visible, false, "Duration badge should be hidden for placeholder")
        }
        
        function test_durationBadge_displaysCorrectText() {
            var durationText = findChild(card, "durationText")
            
            card.duration = "0:59"
            
            compare(durationText.text, "0:59", "Duration should display correctly")
        }
        
        function test_durationBadge_variousFormats_data() {
            return [
                { tag: "seconds", duration: "0:15" },
                { tag: "minutes", duration: "2:30" },
                { tag: "long", duration: "59:59" }
            ]
        }
        
        function test_durationBadge_variousFormats(data) {
            var durationText = findChild(card, "durationText")
            
            card.duration = data.duration
            
            compare(durationText.text, data.duration, "Duration format " + data.tag + " should display correctly")
        }

        // =====================================================================
        // TEST: Placeholder Mode
        // =====================================================================
        
        function test_placeholder_showsPlaceholderText() {
            var placeholderText = findChild(card, "placeholderText")
            
            card.isPlaceholder = true
            
            compare(placeholderText.visible, true, "Placeholder text should be visible")
            compare(placeholderText.text, "...", "Placeholder should show ellipsis")
        }
        
        function test_placeholder_showsEmojiWhenNoThumbnail() {
            var placeholderText = findChild(card, "placeholderText")
            
            card.isPlaceholder = false
            card.thumbnailUrl = ""
            
            compare(placeholderText.visible, true, "Placeholder emoji should be visible without thumbnail")
            compare(placeholderText.text, "🎬", "Should show clip emoji")
        }
        
        function test_placeholder_hidesTextWithThumbnail() {
            var placeholderText = findChild(card, "placeholderText")
            
            card.isPlaceholder = false
            card.thumbnailUrl = "https://example.com/thumb.jpg"
            
            compare(placeholderText.visible, false, "Placeholder text should be hidden with thumbnail")
        }
        

        // =====================================================================
        // TEST: Click Signal
        // =====================================================================
        
        function test_click_emitsCardClicked() {
            var mouseArea = findChild(card, "mouseArea")
            
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 1, "cardClicked should be emitted on click")
        }
        
        function test_click_noSignalWhenPlaceholder() {
            var mouseArea = findChild(card, "mouseArea")
            card.isPlaceholder = true
            
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 0, "Should not emit cardClicked when placeholder")
        }
        
        function test_click_multipleClicks() {
            var mouseArea = findChild(card, "mouseArea")
            
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 3, "Should emit cardClicked for each click")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_setsHoveredTrue() {
            var mouseArea = findChild(card, "mouseArea")
            
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(card, "hovered", true, 100, "hovered should be true on mouse enter")
        }
        
        function test_hover_setsHoveredFalseOnLeave() {
            var mouseArea = findChild(card, "mouseArea")
            
            // First hover
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            tryCompare(card, "hovered", true, 100)
            
            // Then leave
            mouseMove(root, 1, 1)
            
            tryCompare(card, "hovered", false, 100, "hovered should be false on mouse leave")
        }

        // =====================================================================
        // TEST: Card Dimensions
        // =====================================================================
        
        function test_dimensions_canBeCustomized() {
            card.cardWidth = 250
            card.cardHeight = 300
            
            compare(card.width, 250, "Card width should be customizable")
            compare(card.height, 300, "Card height should be customizable")
        }

        // =====================================================================
        // TEST: Full Data Population
        // =====================================================================
        
        function test_fullData_allFieldsDisplayed() {
            // Arrange
            card.clipTitle = "Insane Clutch Moment"
            card.broadcasterName = "ProGamer123"
            card.viewCount = "100K"
            card.duration = "0:28"
            card.thumbnailUrl = "https://clips.twitch.tv/thumb.jpg"
            
            // Assert
            var titleText = findChild(card, "titleText")
            var broadcasterText = findChild(card, "broadcasterText")
            var viewText = findChild(card, "viewCountText")
            var durationText = findChild(card, "durationText")
            var durationBadge = findChild(card, "durationBadge")
            
            compare(titleText.text, "Insane Clutch Moment", "Title displayed")
            compare(broadcasterText.text, "ProGamer123", "Broadcaster displayed")
            compare(viewText.text, "100K vues", "View count displayed with suffix")
            compare(durationText.text, "0:28", "Duration displayed")
            compare(durationBadge.visible, true, "Duration badge visible")
        }
    }
}
