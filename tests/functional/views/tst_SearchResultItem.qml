/**
 * tst_SearchResultItem.qml
 * 
 * Functional UI tests for the SearchResultItem component.
 * Tests data display, click signal, hover state, and LIVE badge.
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
    // Mock Theme Constants (inline)
    // =========================================================================
    
    QtObject {
        id: mockTheme
        readonly property color primaryText: "#e6edf3"
        readonly property color secondaryText: "#8b949e"
        readonly property color mutedText: "#6e7681"
        readonly property color surfaceSoft: "#161b22"
        readonly property color statusNegative: "#f85149"
        readonly property color hoverColor: "#1a2230"
        readonly property int animHoverDuration: 1
        readonly property string fontFamily: "Inter"
    }

    // =========================================================================
    // Component Under Test (Mock of SearchResultItem)
    // =========================================================================
    
    Component {
        id: searchResultItemComponent
        
        Rectangle {
            id: itemRoot
            objectName: "searchResultItem"
            
            // Public properties
            property string itemText: ""
            property string itemSubtext: ""
            property string thumbnailUrl: ""
            property bool isLive: false
            property bool isCategory: false
            property bool isSelected: false
            
            // Internal state exposed for testing
            readonly property alias hovered: mouseArea.containsMouse
            
            // Signal
            signal clicked()
            
            implicitWidth: 300
            implicitHeight: 48
            width: 300
            height: 48
            color: isSelected ? mockTheme.surfaceSoft : (mouseArea.containsMouse ? mockTheme.hoverColor : "transparent")
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
                    Layout.preferredWidth: itemRoot.isCategory ? 32 : 36
                    Layout.preferredHeight: itemRoot.isCategory ? 42 : 36
                    radius: itemRoot.isCategory ? 4 : 18
                    color: mockTheme.surfaceSoft
                    clip: true
                    
                    Image {
                        id: thumbnailImage
                        objectName: "thumbnailImage"
                        anchors.fill: parent
                        source: itemRoot.thumbnailUrl
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
                        id: placeholderText
                        objectName: "placeholderText"
                        anchors.centerIn: parent
                        text: itemRoot.isCategory ? "\uD83C\uDFAE" : "\uD83D\uDC64"
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
                            id: mainText
                            objectName: "mainText"
                            text: itemRoot.itemText
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
                            visible: itemRoot.isLive && !itemRoot.isCategory
                            width: 36
                            height: 16
                            radius: 8
                            color: mockTheme.statusNegative
                            
                            Text {
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
                        id: subText
                        objectName: "subText"
                        visible: itemRoot.itemSubtext !== ""
                        text: itemRoot.itemSubtext
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
                    opacity: mouseArea.containsMouse || itemRoot.isSelected ? 1 : 0
                    
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
                onClicked: itemRoot.clicked()
            }
        }
    }

    // =========================================================================
    // Test Instance
    // =========================================================================
    
    property var item: null

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: clickedSpy; signalName: "clicked" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "SearchResultItemTests"
        when: windowShown

        function init() {
            item = createTemporaryObject(searchResultItemComponent, root)
            verify(item !== null, "SearchResultItem should be created")
            item.anchors.centerIn = root
            clickedSpy.target = item
            clickedSpy.clear()
            mouseMove(root, 1, 1)
        }

        function cleanup() {
            item = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_emptyText() {
            compare(item.itemText, "", "Default itemText should be empty")
        }
        
        function test_defaultState_emptySubtext() {
            compare(item.itemSubtext, "", "Default itemSubtext should be empty")
        }
        
        function test_defaultState_emptyThumbnailUrl() {
            compare(item.thumbnailUrl, "", "Default thumbnailUrl should be empty")
        }
        
        function test_defaultState_notLive() {
            compare(item.isLive, false, "Default isLive should be false")
        }
        
        function test_defaultState_notCategory() {
            compare(item.isCategory, false, "Default isCategory should be false")
        }
        
        function test_defaultState_notSelected() {
            compare(item.isSelected, false, "Default isSelected should be false")
        }
        
        function test_defaultState_notHovered() {
            compare(item.hovered, false, "Default hovered should be false")
        }

        // =====================================================================
        // TEST: Data Display
        // =====================================================================
        
        function test_itemText_displayed() {
            var mainText = findChild(item, "mainText")
            
            item.itemText = "xQc"
            
            compare(mainText.text, "xQc", "itemText should be displayed")
        }
        
        function test_itemSubtext_displayed() {
            var subText = findChild(item, "subText")
            
            item.itemSubtext = "Just Chatting"
            
            compare(subText.text, "Just Chatting", "itemSubtext should be displayed")
            compare(subText.visible, true, "subText should be visible when not empty")
        }
        
        function test_itemSubtext_hiddenWhenEmpty() {
            var subText = findChild(item, "subText")
            
            item.itemSubtext = ""
            
            compare(subText.visible, false, "subText should be hidden when empty")
        }

        // =====================================================================
        // TEST: LIVE Badge
        // =====================================================================
        
        function test_liveBadge_visibleWhenLive() {
            var liveBadge = findChild(item, "liveBadge")
            
            item.isLive = true
            item.isCategory = false
            
            compare(liveBadge.visible, true, "LIVE badge should be visible when isLive=true")
        }
        
        function test_liveBadge_hiddenWhenNotLive() {
            var liveBadge = findChild(item, "liveBadge")
            
            item.isLive = false
            
            compare(liveBadge.visible, false, "LIVE badge should be hidden when isLive=false")
        }
        
        function test_liveBadge_hiddenForCategory() {
            var liveBadge = findChild(item, "liveBadge")
            
            item.isLive = true
            item.isCategory = true
            
            compare(liveBadge.visible, false, "LIVE badge should be hidden for categories")
        }

        // =====================================================================
        // TEST: Thumbnail Container
        // =====================================================================
        
        function test_thumbnail_roundForChannel() {
            var thumbnailContainer = findChild(item, "thumbnailContainer")
            
            item.isCategory = false
            
            compare(thumbnailContainer.radius, 18, "Thumbnail should be circular for channels")
            compare(thumbnailContainer.Layout.preferredWidth, 36, "Channel thumbnail width should be 36")
            compare(thumbnailContainer.Layout.preferredHeight, 36, "Channel thumbnail height should be 36")
        }
        
        function test_thumbnail_squareForCategory() {
            var thumbnailContainer = findChild(item, "thumbnailContainer")
            
            item.isCategory = true
            
            compare(thumbnailContainer.radius, 4, "Thumbnail should be square for categories")
            compare(thumbnailContainer.Layout.preferredWidth, 32, "Category thumbnail width should be 32")
            compare(thumbnailContainer.Layout.preferredHeight, 42, "Category thumbnail height should be 42")
        }
        
        function test_placeholder_channelIcon() {
            var placeholderText = findChild(item, "placeholderText")
            
            item.isCategory = false
            
            compare(placeholderText.text, "\uD83D\uDC64", "Channel placeholder should show user icon")
        }
        
        function test_placeholder_categoryIcon() {
            var placeholderText = findChild(item, "placeholderText")
            
            item.isCategory = true
            
            compare(placeholderText.text, "\uD83C\uDFAE", "Category placeholder should show game icon")
        }

        // =====================================================================
        // TEST: Click Signal
        // =====================================================================
        
        function test_click_emitsSignal() {
            var mouseArea = findChild(item, "mouseArea")
            
            mouseClick(mouseArea)
            
            compare(clickedSpy.count, 1, "clicked should be emitted once")
        }
        
        function test_multipleClicks_emitMultipleSignals() {
            var mouseArea = findChild(item, "mouseArea")
            
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            mouseClick(mouseArea)
            
            compare(clickedSpy.count, 3, "clicked should be emitted 3 times")
        }

        // =====================================================================
        // TEST: Hover State
        // =====================================================================
        
        function test_hover_setsHoveredTrue() {
            var mouseArea = findChild(item, "mouseArea")
            
            mouseMove(mouseArea, mouseArea.width / 2, mouseArea.height / 2)
            
            tryCompare(item, "hovered", true, 100, "hovered should be true")
        }
        
        function test_arrowIndicator_hiddenByDefault() {
            var arrowIndicator = findChild(item, "arrowIndicator")
            
            compare(arrowIndicator.opacity, 0, "Arrow should be hidden by default")
        }
        
        function test_arrowIndicator_visibleWhenSelected() {
            var arrowIndicator = findChild(item, "arrowIndicator")
            
            item.isSelected = true
            
            tryCompare(arrowIndicator, "opacity", 1, 100, "Arrow should be visible when selected")
        }

        // =====================================================================
        // TEST: Selected State
        // =====================================================================
        
        

        // =====================================================================
        // TEST: Data-driven scenarios
        // =====================================================================
        
        function test_liveChannel_data() {
            return [
                { tag: "live channel", isLive: true, isCategory: false, badgeVisible: true },
                { tag: "offline channel", isLive: false, isCategory: false, badgeVisible: false },
                { tag: "live category", isLive: true, isCategory: true, badgeVisible: false },
                { tag: "normal category", isLive: false, isCategory: true, badgeVisible: false }
            ]
        }
        
        function test_liveChannel(data) {
            var liveBadge = findChild(item, "liveBadge")
            
            item.isLive = data.isLive
            item.isCategory = data.isCategory
            
            compare(liveBadge.visible, data.badgeVisible, 
                    data.tag + ": badge visibility should be " + data.badgeVisible)
        }
    }
}
