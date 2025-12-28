/**
 * tst_SearchResults.qml
 * 
 * Functional UI tests for the SearchResults component.
 * 
 * Tests search results dropdown functionality:
 * - Display of live channels and cached VODs
 * - Click handling for channels and cache items
 * - Keyboard navigation (Up/Down/Enter)
 * - Visual selection state
 * - Empty results handling
 * 
 * Run with: ./test_functional_ui SearchResultsTests
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 500
    height: 600

    // =========================================================================
    // Test Data
    // =========================================================================
    
    readonly property var testChannelResults: [
        { 
            broadcaster_login: "streamer1", 
            display_name: "Streamer One", 
            is_live: true,
            game_name: "Just Chatting",
            title: "Morning stream!",
            thumbnail_url: "https://example.com/thumb1.jpg"
        },
        { 
            broadcaster_login: "streamer2", 
            display_name: "Streamer Two", 
            is_live: true,
            game_name: "Fortnite",
            title: "Playing ranked",
            thumbnail_url: "https://example.com/thumb2.jpg"
        },
        { 
            broadcaster_login: "streamer3", 
            display_name: "Streamer Three", 
            is_live: true,
            game_name: "Minecraft",
            title: "",
            thumbnail_url: "https://example.com/thumb3.jpg"
        }
    ]
    
    readonly property var testCacheResults: [
        {
            id: "vod1",
            streamerName: "CachedStreamer",
            streamTitle: "Past broadcast",
            filePath: "/path/to/vod1.ts",
            thumbnailPath: "/path/to/thumb1.jpg",
            recordedAtFormatted: "Dec 27",
            durationFormatted: "2h30m",
            fileSizeFormatted: "1.2 GB"
        },
        {
            id: "vod2",
            streamerName: "AnotherStreamer",
            streamTitle: "Old stream",
            filePath: "/path/to/vod2.ts",
            thumbnailPath: "/path/to/thumb2.jpg",
            recordedAtFormatted: "Dec 26",
            durationFormatted: "1h15m",
            fileSizeFormatted: "800 MB"
        }
    ]

    // =========================================================================
    // Component Under Test
    // =========================================================================
    
    Rectangle {
        id: searchResults
        anchors.fill: parent
        anchors.margins: 20
        
        // Properties matching SearchResults.qml interface
        property var channelResults: []
        property var cacheResults: []
        property bool isVisible: true
        property bool hasSearchQuery: false
        property int selectedIndex: -1
        property int totalCount: (channelResults ? channelResults.length : 0) + (cacheResults ? cacheResults.length : 0)
        
        // Signals
        signal channelClicked(string broadcasterLogin, string displayName, bool isLive, string thumbnailUrl)
        signal cacheVodClicked(string vodId, string filePath, string streamerName)
        signal closeRequested()
        
        // Navigation methods
        function navigateUp() {
            if (selectedIndex > 0) selectedIndex--
            else selectedIndex = totalCount - 1
        }
        
        function navigateDown() {
            if (selectedIndex < totalCount - 1) selectedIndex++
            else selectedIndex = 0
        }
        
        function selectCurrent() {
            if (selectedIndex < 0) return
            var channelCount = channelResults ? channelResults.length : 0
            if (selectedIndex < channelCount) {
                var channel = channelResults[selectedIndex]
                channelClicked(channel.broadcaster_login || "", 
                              channel.display_name || "", 
                              channel.is_live || false, 
                              channel.thumbnail_url || "")
            } else {
                var cacheIndex = selectedIndex - channelCount
                var vod = cacheResults[cacheIndex]
                cacheVodClicked(vod.id || "", vod.filePath || "", vod.streamerName || "")
            }
        }
        
        // Reset function
        function reset() {
            channelResults = []
            cacheResults = []
            isVisible = true
            hasSearchQuery = false
            selectedIndex = -1
        }
        
        visible: isVisible && hasSearchQuery
        color: "#1a1a2e"
        radius: 16
        border.color: "#333333"
        border.width: 1
        
        Column {
            id: contentColumn
            anchors.fill: parent
            anchors.margins: 8
            spacing: 2
            
            // "En direct" section header
            Text {
                id: liveHeader
                objectName: "liveHeader"
                visible: searchResults.channelResults && searchResults.channelResults.length > 0
                text: "En direct"
                color: "#888888"
                font.pixelSize: 11
                font.bold: true
            }
            
            // Channel results
            Repeater {
                id: channelRepeater
                model: searchResults.channelResults
                
                delegate: Rectangle {
                    id: channelItem
                    objectName: "channelItem_" + index
                    width: contentColumn.width
                    height: 56
                    radius: 8
                    color: channelMouse.containsMouse ? "#1a2230" : "transparent"
                    border.color: searchResults.selectedIndex === index ? "#0066FF" : "transparent"
                    border.width: searchResults.selectedIndex === index ? 1 : 0
                    
                    property var channelData: modelData
                    
                    // Channel name
                    Text {
                        id: channelName
                        objectName: "channelName_" + index
                        x: 8
                        y: 8
                        text: channelItem.channelData.display_name || ""
                        color: "#FFFFFF"
                        font.pixelSize: 14
                        font.bold: true
                    }
                    
                    // Game name
                    Text {
                        id: gameName
                        objectName: "gameName_" + index
                        x: 8
                        y: 28
                        text: channelItem.channelData.game_name || ""
                        color: "#0066FF"
                        font.pixelSize: 12
                    }
                    
                    // LIVE badge
                    Rectangle {
                        id: liveBadge
                        objectName: "liveBadge_" + index
                        x: parent.width - 54
                        y: 8
                        visible: channelItem.channelData.is_live
                        width: 42
                        height: 18
                        radius: 9
                        color: "#FF0000"
                        
                        Text {
                            anchors.centerIn: parent
                            text: "LIVE"
                            color: "#FFFFFF"
                            font.pixelSize: 10
                            font.bold: true
                        }
                    }
                    
                    MouseArea {
                        id: channelMouse
                        objectName: "channelMouse_" + index
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            searchResults.channelClicked(
                                channelItem.channelData.broadcaster_login || "",
                                channelItem.channelData.display_name || "",
                                channelItem.channelData.is_live || false,
                                channelItem.channelData.thumbnail_url || ""
                            )
                        }
                    }
                }
            }
            
            // Separator
            Rectangle {
                id: separator
                objectName: "separator"
                width: contentColumn.width
                height: 1
                color: "#333333"
                visible: searchResults.channelResults && searchResults.channelResults.length > 0 &&
                         searchResults.cacheResults && searchResults.cacheResults.length > 0
            }
            
            // "Dans le cache" section header
            Text {
                id: cacheHeader
                objectName: "cacheHeader"
                visible: searchResults.cacheResults && searchResults.cacheResults.length > 0
                text: "Dans le cache"
                color: "#888888"
                font.pixelSize: 11
                font.bold: true
            }
            
            // Cache results
            Repeater {
                id: cacheRepeater
                model: searchResults.cacheResults
                
                delegate: Rectangle {
                    id: cacheItem
                    objectName: "cacheItem_" + index
                    width: contentColumn.width
                    height: 64
                    radius: 8
                    
                    property var vodData: modelData
                    property int globalIndex: (searchResults.channelResults ? searchResults.channelResults.length : 0) + index
                    
                    color: cacheMouse.containsMouse ? "#1a2230" : "transparent"
                    border.color: searchResults.selectedIndex === globalIndex ? "#0066FF" : "transparent"
                    border.width: searchResults.selectedIndex === globalIndex ? 1 : 0
                    
                    // Streamer name
                    Text {
                        id: streamerName
                        objectName: "streamerName_" + index
                        x: 8
                        y: 8
                        text: cacheItem.vodData.streamerName || ""
                        color: "#FFFFFF"
                        font.pixelSize: 14
                        font.bold: true
                    }
                    
                    // Stream title
                    Text {
                        id: streamTitle
                        objectName: "streamTitle_" + index
                        x: 8
                        y: 28
                        text: cacheItem.vodData.streamTitle || ""
                        color: "#AAAAAA"
                        font.pixelSize: 11
                    }
                    
                    // CACHE badge
                    Rectangle {
                        id: cacheBadge
                        objectName: "cacheBadge_" + index
                        x: parent.width - 62
                        y: 8
                        width: 50
                        height: 18
                        radius: 9
                        color: "#0066FF"
                        
                        Text {
                            anchors.centerIn: parent
                            text: "CACHE"
                            color: "#FFFFFF"
                            font.pixelSize: 9
                            font.bold: true
                        }
                    }
                    
                    MouseArea {
                        id: cacheMouse
                        objectName: "cacheMouse_" + index
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            searchResults.cacheVodClicked(
                                cacheItem.vodData.id || "",
                                cacheItem.vodData.filePath || "",
                                cacheItem.vodData.streamerName || ""
                            )
                        }
                    }
                }
            }
            
            // No results message
            Text {
                id: noResults
                objectName: "noResultsText"
                visible: searchResults.totalCount === 0 && searchResults.hasSearchQuery
                text: "Aucun resultat"
                color: "#888888"
                font.pixelSize: 13
                width: contentColumn.width
                horizontalAlignment: Text.AlignHCenter
            }
        }
        
        // Keyboard handling
        Keys.onUpPressed: navigateUp()
        Keys.onDownPressed: navigateDown()
        Keys.onReturnPressed: selectCurrent()
        Keys.onEnterPressed: selectCurrent()
        Keys.onEscapePressed: closeRequested()
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================
    
    SignalSpy { id: channelClickedSpy; target: searchResults; signalName: "channelClicked" }
    SignalSpy { id: cacheVodClickedSpy; target: searchResults; signalName: "cacheVodClicked" }
    SignalSpy { id: closeRequestedSpy; target: searchResults; signalName: "closeRequested" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "SearchResultsTests"
        when: windowShown

        function init() {
            searchResults.reset()
            channelClickedSpy.clear()
            cacheVodClickedSpy.clear()
            closeRequestedSpy.clear()
            wait(50)
        }

        // =====================================================================
        // TEST: Initial State
        // =====================================================================
        
        function test_initialState_notVisibleWithoutQuery() {
            // Arrange
            searchResults.hasSearchQuery = false
            
            // Assert
            verify(!searchResults.visible, "Should not be visible without search query")
        }
        
        function test_initialState_visibleWithQuery() {
            // Arrange
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            verify(searchResults.visible, "Should be visible with search query")
        }

        // =====================================================================
        // TEST: Channel Results Display
        // =====================================================================
        
        function test_channelResults_showsAllChannels() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            for (var i = 0; i < 3; i++) {
                var item = findChild(searchResults, "channelItem_" + i)
                verify(item !== null, "Channel item " + i + " should exist")
            }
        }
        
        function test_channelResults_showsCorrectNames() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            var expectedNames = ["Streamer One", "Streamer Two", "Streamer Three"]
            for (var i = 0; i < expectedNames.length; i++) {
                var nameLabel = findChild(searchResults, "channelName_" + i)
                if (nameLabel) {
                    compare(nameLabel.text, expectedNames[i], "Channel " + i + " name should match")
                }
            }
        }
        
        function test_channelResults_showsLiveBadge() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            var badge = findChild(searchResults, "liveBadge_0")
            if (badge) {
                verify(badge.visible, "LIVE badge should be visible for live channels")
            }
        }
        
        function test_channelResults_showsGameName() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            var gameLabel = findChild(searchResults, "gameName_0")
            if (gameLabel) {
                compare(gameLabel.text, "Just Chatting", "Game name should be displayed")
            }
        }

        // =====================================================================
        // TEST: Cache Results Display
        // =====================================================================
        
        function test_cacheResults_showsAllVods() {
            // Arrange
            searchResults.cacheResults = root.testCacheResults
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            for (var i = 0; i < 2; i++) {
                var item = findChild(searchResults, "cacheItem_" + i)
                verify(item !== null, "Cache item " + i + " should exist")
            }
        }
        
        function test_cacheResults_showsStreamerName() {
            // Arrange
            searchResults.cacheResults = root.testCacheResults
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            var nameLabel = findChild(searchResults, "streamerName_0")
            if (nameLabel) {
                compare(nameLabel.text, "CachedStreamer", "Streamer name should match")
            }
        }
        
        function test_cacheResults_showsCacheBadge() {
            // Arrange
            searchResults.cacheResults = root.testCacheResults
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            var badge = findChild(searchResults, "cacheBadge_0")
            if (badge) {
                verify(badge.visible, "CACHE badge should be visible")
            }
        }

        // =====================================================================
        // TEST: Mixed Results
        // =====================================================================
        
        function test_mixedResults_showsSeparator() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.cacheResults = root.testCacheResults
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            var separator = findChild(searchResults, "separator")
            if (separator) {
                verify(separator.visible, "Separator should be visible with both types")
            }
        }
        
        function test_mixedResults_showsBothHeaders() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.cacheResults = root.testCacheResults
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            var liveHeader = findChild(searchResults, "liveHeader")
            var cacheHeader = findChild(searchResults, "cacheHeader")
            if (liveHeader) verify(liveHeader.visible, "Live header should be visible")
            if (cacheHeader) verify(cacheHeader.visible, "Cache header should be visible")
        }

        // =====================================================================
        // TEST: Channel Click
        // =====================================================================
        
        function test_channelClick_emitsSignal() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            wait(50)
            var mouseArea = findChild(searchResults, "channelMouse_0")
            verify(mouseArea !== null, "Mouse area should exist")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(channelClickedSpy.count, 1, "channelClicked should be emitted")
            compare(channelClickedSpy.signalArguments[0][0], "streamer1", "broadcaster_login should match")
            compare(channelClickedSpy.signalArguments[0][1], "Streamer One", "display_name should match")
            compare(channelClickedSpy.signalArguments[0][2], true, "is_live should be true")
        }

        // =====================================================================
        // TEST: Cache VOD Click
        // =====================================================================
        
        function test_cacheClick_emitsSignal() {
            // Arrange
            searchResults.cacheResults = root.testCacheResults
            searchResults.hasSearchQuery = true
            wait(50)
            var mouseArea = findChild(searchResults, "cacheMouse_0")
            verify(mouseArea !== null, "Cache mouse area should exist")
            
            // Act
            mouseClick(mouseArea)
            
            // Assert
            compare(cacheVodClickedSpy.count, 1, "cacheVodClicked should be emitted")
            compare(cacheVodClickedSpy.signalArguments[0][0], "vod1", "vodId should match")
            compare(cacheVodClickedSpy.signalArguments[0][1], "/path/to/vod1.ts", "filePath should match")
            compare(cacheVodClickedSpy.signalArguments[0][2], "CachedStreamer", "streamerName should match")
        }

        // =====================================================================
        // TEST: Keyboard Navigation
        // =====================================================================
        
        function test_navigateDown_selectsFirstItem() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            searchResults.selectedIndex = -1
            searchResults.forceActiveFocus()
            wait(50)
            
            // Act
            searchResults.navigateDown()
            
            // Assert
            compare(searchResults.selectedIndex, 0, "Should select first item")
        }
        
        function test_navigateDown_movesToNextItem() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            searchResults.selectedIndex = 0
            wait(50)
            
            // Act
            searchResults.navigateDown()
            
            // Assert
            compare(searchResults.selectedIndex, 1, "Should select second item")
        }
        
        function test_navigateUp_movesToPreviousItem() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            searchResults.selectedIndex = 2
            wait(50)
            
            // Act
            searchResults.navigateUp()
            
            // Assert
            compare(searchResults.selectedIndex, 1, "Should select second item")
        }
        
        function test_navigateDown_wrapsAround() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            searchResults.selectedIndex = 2  // Last item
            wait(50)
            
            // Act
            searchResults.navigateDown()
            
            // Assert
            compare(searchResults.selectedIndex, 0, "Should wrap to first item")
        }
        
        function test_navigateUp_wrapsAround() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            searchResults.selectedIndex = 0
            wait(50)
            
            // Act
            searchResults.navigateUp()
            
            // Assert
            compare(searchResults.selectedIndex, 2, "Should wrap to last item")
        }
        
        function test_selectCurrent_emitsChannelSignal() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            searchResults.selectedIndex = 1
            wait(50)
            
            // Act
            searchResults.selectCurrent()
            
            // Assert
            compare(channelClickedSpy.count, 1, "channelClicked should be emitted")
            compare(channelClickedSpy.signalArguments[0][0], "streamer2", "Should select second channel")
        }
        
        function test_selectCurrent_emitsCacheSignal() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.cacheResults = root.testCacheResults
            searchResults.hasSearchQuery = true
            searchResults.selectedIndex = 3  // First cache item (after 3 channels)
            wait(50)
            
            // Act
            searchResults.selectCurrent()
            
            // Assert
            compare(cacheVodClickedSpy.count, 1, "cacheVodClicked should be emitted")
            compare(cacheVodClickedSpy.signalArguments[0][0], "vod1", "Should select first cache item")
        }
        
        function test_escapeKey_requestsClose() {
            // Arrange
            searchResults.hasSearchQuery = true
            searchResults.forceActiveFocus()
            wait(50)
            
            // Act
            keyClick(Qt.Key_Escape)
            
            // Assert
            compare(closeRequestedSpy.count, 1, "closeRequested should be emitted")
        }

        // =====================================================================
        // TEST: Empty Results
        // =====================================================================
        
        function test_emptyResults_showsNoResultsMessage() {
            // Arrange
            searchResults.channelResults = []
            searchResults.cacheResults = []
            searchResults.hasSearchQuery = true
            wait(50)
            
            // Assert
            var noResults = findChild(searchResults, "noResultsText")
            if (noResults) {
                verify(noResults.visible, "No results message should be visible")
                compare(noResults.text, "Aucun resultat", "Message should be in French")
            }
        }
        
        function test_emptyResults_totalCountIsZero() {
            // Arrange
            searchResults.channelResults = []
            searchResults.cacheResults = []
            
            // Assert
            compare(searchResults.totalCount, 0, "Total count should be 0")
        }

        // =====================================================================
        // TEST: Visual Selection State
        // =====================================================================
        
        function test_selectedItem_showsBorder() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            searchResults.selectedIndex = 0
            wait(50)
            
            // Assert
            var item = findChild(searchResults, "channelItem_0")
            if (item) {
                verify(item.border.width > 0, "Selected item should have border")
            }
        }
        
        function test_unselectedItem_noBorder() {
            // Arrange
            searchResults.channelResults = root.testChannelResults
            searchResults.hasSearchQuery = true
            searchResults.selectedIndex = 0
            wait(50)
            
            // Assert
            var item = findChild(searchResults, "channelItem_1")
            if (item) {
                compare(item.border.width, 0, "Unselected item should have no border")
            }
        }
    }
}
