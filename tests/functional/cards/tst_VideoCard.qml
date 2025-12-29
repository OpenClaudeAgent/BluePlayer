/**
 * tst_VideoCard.qml
 * 
 * Functional UI tests for the VideoCard component.
 * Tests VOD data display, duration badge, and resume badge.
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
        readonly property color secondaryText: "#8b949e"
        readonly property color mutedText: "#6e7681"
        readonly property color accent: "#3b82f6"
        readonly property color surface: "#0d1117"
        readonly property color surfaceSoft: "#161b22"
        readonly property color divider: "#30363d"
        readonly property color cardHighlight: "#1c2128"
        readonly property int animCardDuration: 1
        readonly property int animContentFadeDuration: 1
        readonly property string fontFamily: "Inter"
    }

    // =========================================================================
    // Component Under Test (Mock of VideoCard)
    // =========================================================================
    
    Component {
        id: videoCardComponent
        
        Item {
            id: cardRoot
            objectName: "videoCard"

            // BaseCard properties
            property bool isPlaceholder: false
            property int cardWidth: 200
            property int cardHeight: 220
            readonly property alias hovered: mouseArea.containsMouse

            // VideoCard specific properties
            property string videoTitle: ""
            property string userName: ""
            property string viewCount: ""
            property string duration: ""
            property string thumbnailUrl: ""
            property bool hasProgress: false
            property int watchPosition: 0

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

                        // Placeholder text
                        Text {
                            id: placeholderText
                            objectName: "placeholderText"
                            anchors.centerIn: parent
                            text: cardRoot.isPlaceholder ? "⋯" : "📹"
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
                            visible: !cardRoot.isPlaceholder && cardRoot.duration !== ""

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

                        // Resume badge
                        Rectangle {
                            id: resumeBadge
                            objectName: "resumeBadge"
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.margins: 6
                            width: resumeText.implicitWidth + 12
                            height: 24
                            radius: 12
                            color: mockTheme.accent
                            visible: cardRoot.hasProgress && !cardRoot.isPlaceholder

                            Text {
                                id: resumeText
                                objectName: "resumeText"
                                anchors.centerIn: parent
                                text: "▶ Reprendre"
                                font.pixelSize: 10
                                font.bold: true
                                color: "#fff"
                            }
                        }
                    }

                    // Video info
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            id: titleText
                            objectName: "titleText"
                            text: cardRoot.videoTitle
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
                            id: userNameText
                            objectName: "userNameText"
                            text: cardRoot.userName
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
        name: "VideoCardTests"
        when: windowShown

        function init() {
            card = createTemporaryObject(videoCardComponent, root)
            verify(card !== null, "VideoCard should be created")
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
            compare(card.videoTitle, "", "Default videoTitle should be empty")
            compare(card.userName, "", "Default userName should be empty")
            compare(card.viewCount, "", "Default viewCount should be empty")
            compare(card.duration, "", "Default duration should be empty")
        }
        
        function test_defaultState_noProgress() {
            compare(card.hasProgress, false, "Default hasProgress should be false")
            compare(card.watchPosition, 0, "Default watchPosition should be 0")
        }

        // =====================================================================
        // TEST: Data Display
        // =====================================================================
        
        function test_videoTitle_displayed() {
            var titleText = findChild(card, "titleText")
            
            card.videoTitle = "Epic Stream Highlights"
            
            compare(titleText.text, "Epic Stream Highlights", "Video title should be displayed")
        }
        
        function test_userName_displayed() {
            var userText = findChild(card, "userNameText")
            
            card.userName = "StreamerPro"
            
            compare(userText.text, "StreamerPro", "User name should be displayed")
        }
        
        function test_viewCount_displayedWithSuffix() {
            var viewText = findChild(card, "viewCountText")
            
            card.viewCount = "123K"
            
            compare(viewText.text, "123K vues", "View count should be displayed with 'vues' suffix")
        }
        
        function test_viewCount_emptyWhenNoCount() {
            var viewText = findChild(card, "viewCountText")
            
            card.viewCount = ""
            
            compare(viewText.text, "", "View count text should be empty when no count")
        }

        // =====================================================================
        // TEST: Duration Badge
        // =====================================================================
        
        function test_durationBadge_visibleWithDuration() {
            var durationBadge = findChild(card, "durationBadge")
            
            card.duration = "2:45:30"
            
            compare(durationBadge.visible, true, "Duration badge should be visible")
        }
        
        function test_durationBadge_hiddenWithoutDuration() {
            var durationBadge = findChild(card, "durationBadge")
            
            card.duration = ""
            
            compare(durationBadge.visible, false, "Duration badge should be hidden without duration")
        }
        
        function test_durationBadge_hiddenWhenPlaceholder() {
            var durationBadge = findChild(card, "durationBadge")
            
            card.duration = "1:00:00"
            card.isPlaceholder = true
            
            compare(durationBadge.visible, false, "Duration badge should be hidden for placeholder")
        }
        
        function test_durationBadge_displaysCorrectText() {
            var durationText = findChild(card, "durationText")
            
            card.duration = "45:30"
            
            compare(durationText.text, "45:30", "Duration should display correctly")
        }

        // =====================================================================
        // TEST: Resume Badge
        // =====================================================================
        
        function test_resumeBadge_visibleWithProgress() {
            var resumeBadge = findChild(card, "resumeBadge")
            
            card.hasProgress = true
            
            compare(resumeBadge.visible, true, "Resume badge should be visible with progress")
        }
        
        function test_resumeBadge_hiddenWithoutProgress() {
            var resumeBadge = findChild(card, "resumeBadge")
            
            card.hasProgress = false
            
            compare(resumeBadge.visible, false, "Resume badge should be hidden without progress")
        }
        
        function test_resumeBadge_hiddenWhenPlaceholder() {
            var resumeBadge = findChild(card, "resumeBadge")
            
            card.hasProgress = true
            card.isPlaceholder = true
            
            compare(resumeBadge.visible, false, "Resume badge should be hidden for placeholder")
        }
        
        function test_resumeBadge_correctText() {
            var resumeText = findChild(card, "resumeText")
            
            compare(resumeText.text, "▶ Reprendre", "Resume badge should say 'Reprendre'")
        }

        // =====================================================================
        // TEST: Click Signal
        // =====================================================================
        
        function test_click_emitsCardClicked() {
            var mouseArea = findChild(card, "mouseArea")
            
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 1, "cardClicked should be emitted")
        }
        
        function test_click_noSignalWhenPlaceholder() {
            var mouseArea = findChild(card, "mouseArea")
            card.isPlaceholder = true
            
            mouseClick(mouseArea)
            
            compare(cardClickedSpy.count, 0, "Should not emit when placeholder")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_setsHoveredTrue() {
            var mouseArea = findChild(card, "mouseArea")
            
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(card, "hovered", true, 100, "hovered should be true")
        }

        // =====================================================================
        // TEST: Watch Position
        // =====================================================================
        
        function test_watchPosition_canBeSet() {
            card.watchPosition = 3600  // 1 hour
            
            compare(card.watchPosition, 3600, "watchPosition should be settable")
        }
    }
}
