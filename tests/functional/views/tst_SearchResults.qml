/**
 * tst_SearchResults.qml
 *
 * Functional UI tests for the SearchResults component.
 * Tests visibility, results display, navigation, selection, and signals.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 600
    height: 500

    // =========================================================================
    // Theme Constants (inline, no external QtObject)
    // =========================================================================

    readonly property color themePrimaryText: "#FFFFFF"
    readonly property color themeSecondaryText: "#8899A6"
    readonly property color themeMutedText: "#657786"
    readonly property color themeAccent: "#0066FF"
    readonly property color themeSurface: "#1b2130"
    readonly property color themeSurfaceSoft: "#252d3d"
    readonly property color themeDivider: "#2a324e"
    readonly property color themeCardHighlight: "#2a324e"
    readonly property color themeStatusNegative: "#e91916"
    readonly property int themeSpacingSmall: 8
    readonly property string themeFontFamily: "Inter"
    readonly property int themeAnimHoverDuration: 100

    // =========================================================================
    // Component Under Test (Mock) - SearchResults
    // =========================================================================

    Rectangle {
        id: searchResults
        objectName: "searchResults"
        anchors.centerIn: parent
        width: 400
        height: implicitHeight

        // Public properties (mirroring SearchResults.qml API)
        property var channelResults: []
        property var cacheResults: []
        property bool isVisible: false
        property bool hasSearchQuery: false
        property int selectedIndex: -1
        readonly property int totalCount: (channelResults ? channelResults.length : 0) + (cacheResults ? cacheResults.length : 0)

        // Signals
        signal channelClicked(string broadcasterLogin, string displayName, bool isLive, string thumbnailUrl)
        signal cacheVodClicked(string vodId, string filePath, string streamerName)
        signal closeRequested()

        // Visibility logic
        visible: isVisible && hasSearchQuery

        // Styling
        color: root.themeSurface
        border.color: root.themeDivider
        border.width: 1
        radius: 16
        clip: true

        implicitHeight: totalCount > 0 ? Math.min(contentColumn.height + 16, 400) : 60

        // Navigation functions
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
                channelClicked(
                    channel.broadcaster_login || "",
                    channel.display_name || "",
                    channel.is_live || false,
                    channel.thumbnail_url || ""
                )
            } else {
                var cacheIndex = selectedIndex - channelCount
                var vod = cacheResults[cacheIndex]
                cacheVodClicked(
                    vod.id || "",
                    vod.filePath || "",
                    vod.streamerName || ""
                )
            }
        }

        // Reset selectedIndex when results change
        onChannelResultsChanged: selectedIndex = -1
        onCacheResultsChanged: selectedIndex = -1

        // Reset function for tests
        function reset() {
            channelResults = []
            cacheResults = []
            isVisible = false
            hasSearchQuery = false
            selectedIndex = -1
        }

        // Content
        Flickable {
            id: flickable
            objectName: "flickable"
            anchors.fill: parent
            anchors.margins: root.themeSpacingSmall
            contentHeight: contentColumn.height
            clip: true

            Column {
                id: contentColumn
                objectName: "contentColumn"
                width: parent.width
                spacing: root.themeSpacingSmall

                readonly property int sectionLabelHeight: 24
                readonly property int sectionSpacing: 8

                // ==================== LIVE CHANNELS HEADER ====================
                Item {
                    id: liveHeader
                    objectName: "liveHeader"
                    visible: searchResults.channelResults && searchResults.channelResults.length > 0
                    width: contentColumn.width
                    height: contentColumn.sectionLabelHeight

                    Text {
                        id: liveLabel
                        objectName: "liveLabel"
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 4
                        text: qsTr("Live")
                        font.family: root.themeFontFamily
                        font.pixelSize: 11
                        font.bold: true
                        color: root.themeMutedText
                    }
                }

                // ==================== LIVE CHANNELS REPEATER ====================
                Repeater {
                    id: channelRepeater
                    objectName: "channelRepeater"
                    model: searchResults.channelResults

                    delegate: Rectangle {
                        id: channelDelegate
                        objectName: "channelDelegate_" + index
                        width: contentColumn.width
                        height: 52
                        radius: root.themeSpacingSmall

                        property var channelInfo: modelData
                        property int itemIndex: index
                        property bool isSelected: searchResults.selectedIndex === itemIndex
                        property bool isLive: channelInfo && channelInfo.is_live ? true : false

                        color: channelMouse.containsMouse ? root.themeCardHighlight : "transparent"
                        border.color: isSelected ? root.themeAccent : "transparent"
                        border.width: isSelected ? 1 : 0

                        // Avatar placeholder
                        Rectangle {
                            id: channelAvatar
                            objectName: "channelAvatar_" + index
                            x: 8
                            y: (parent.height - 36) / 2
                            width: 36
                            height: 36
                            radius: 18
                            color: root.themeSurfaceSoft
                        }

                        // Channel name
                        Text {
                            id: channelNameText
                            objectName: "channelNameText_" + index
                            x: 56
                            y: (parent.height - height) / 2
                            width: channelDelegate.width - 120
                            text: channelDelegate.channelInfo ? (channelDelegate.channelInfo.display_name || channelDelegate.channelInfo.broadcaster_login || "Unknown") : "Unknown"
                            font.family: root.themeFontFamily
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            color: root.themePrimaryText
                            elide: Text.ElideRight
                        }

                        // Live badge
                        Rectangle {
                            id: liveBadge
                            objectName: "liveBadge_" + index
                            x: channelDelegate.width - 54
                            y: 8
                            visible: channelDelegate.isLive
                            width: 42
                            height: 18
                            radius: 9
                            color: root.themeStatusNegative

                            Text {
                                objectName: "liveBadgeText_" + index
                                anchors.centerIn: parent
                                text: "LIVE"
                                font.pixelSize: 10
                                font.bold: true
                                color: "#ffffff"
                            }
                        }

                        MouseArea {
                            id: channelMouse
                            objectName: "channelMouseArea_" + index
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (channelDelegate.channelInfo) {
                                    searchResults.channelClicked(
                                        channelDelegate.channelInfo.broadcaster_login || "",
                                        channelDelegate.channelInfo.display_name || "",
                                        channelDelegate.channelInfo.is_live || false,
                                        channelDelegate.channelInfo.thumbnail_url || ""
                                    )
                                }
                            }
                        }
                    }
                }

                // ==================== SEPARATOR ====================
                Item {
                    id: separator
                    objectName: "separator"
                    visible: searchResults.channelResults && searchResults.channelResults.length > 0 &&
                             searchResults.cacheResults && searchResults.cacheResults.length > 0
                    width: contentColumn.width
                    height: contentColumn.sectionSpacing * 2

                    Rectangle {
                        objectName: "separatorLine"
                        anchors.centerIn: parent
                        width: parent.width - 16
                        height: 1
                        color: root.themeDivider
                    }
                }

                // ==================== CACHED VODS HEADER ====================
                Item {
                    id: cacheHeader
                    objectName: "cacheHeader"
                    visible: searchResults.cacheResults && searchResults.cacheResults.length > 0
                    width: contentColumn.width
                    height: contentColumn.sectionLabelHeight

                    Text {
                        id: cacheLabel
                        objectName: "cacheLabel"
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 4
                        text: qsTr("In cache")
                        font.family: root.themeFontFamily
                        font.pixelSize: 11
                        font.bold: true
                        color: root.themeMutedText
                    }
                }

                // ==================== CACHED VODS REPEATER ====================
                Repeater {
                    id: cacheRepeater
                    objectName: "cacheRepeater"
                    model: searchResults.cacheResults

                    delegate: Rectangle {
                        id: cacheDelegate
                        objectName: "cacheDelegate_" + index
                        width: contentColumn.width
                        height: 64
                        radius: root.themeSpacingSmall

                        property var vodInfo: modelData
                        property int itemIndex: index
                        property int globalIndex: (searchResults.channelResults ? searchResults.channelResults.length : 0) + itemIndex
                        property bool isSelected: searchResults.selectedIndex === globalIndex

                        color: cacheMouse.containsMouse ? root.themeCardHighlight : "transparent"
                        border.color: isSelected ? root.themeAccent : "transparent"
                        border.width: isSelected ? 1 : 0

                        // Thumbnail placeholder
                        Rectangle {
                            id: vodThumb
                            objectName: "vodThumb_" + index
                            x: 8
                            y: 8
                            width: 48
                            height: 48
                            radius: 8
                            color: root.themeSurfaceSoft
                        }

                        // Streamer name
                        Text {
                            id: streamerNameText
                            objectName: "streamerNameText_" + index
                            x: 68
                            y: 10
                            width: cacheDelegate.width - 140
                            text: cacheDelegate.vodInfo ? (cacheDelegate.vodInfo.streamerName || "Unknown") : "Unknown"
                            font.family: root.themeFontFamily
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            color: root.themePrimaryText
                            elide: Text.ElideRight
                        }

                        // Stream title
                        Text {
                            id: streamTitleText
                            objectName: "streamTitleText_" + index
                            x: 68
                            y: 28
                            width: cacheDelegate.width - 140
                            text: cacheDelegate.vodInfo ? (cacheDelegate.vodInfo.streamTitle || "") : ""
                            font.family: root.themeFontFamily
                            font.pixelSize: 11
                            color: root.themeSecondaryText
                            elide: Text.ElideRight
                        }

                        // Cache badge
                        Rectangle {
                            id: cacheBadge
                            objectName: "cacheBadge_" + index
                            x: cacheDelegate.width - 62
                            y: 8
                            width: 50
                            height: 18
                            radius: 9
                            color: root.themeAccent
                            opacity: 0.8

                            Text {
                                objectName: "cacheBadgeText_" + index
                                anchors.centerIn: parent
                                text: "CACHE"
                                font.pixelSize: 9
                                font.bold: true
                                color: "#ffffff"
                            }
                        }

                        MouseArea {
                            id: cacheMouse
                            objectName: "cacheMouseArea_" + index
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (cacheDelegate.vodInfo) {
                                    searchResults.cacheVodClicked(
                                        cacheDelegate.vodInfo.id || "",
                                        cacheDelegate.vodInfo.filePath || "",
                                        cacheDelegate.vodInfo.streamerName || ""
                                    )
                                }
                            }
                        }
                    }
                }

                // ==================== NO RESULTS ====================
                Text {
                    id: noResultsText
                    objectName: "noResultsText"
                    visible: searchResults.totalCount === 0
                    text: qsTr("No results")
                    font.family: root.themeFontFamily
                    font.pixelSize: 13
                    color: root.themeMutedText
                    width: contentColumn.width
                    horizontalAlignment: Text.AlignHCenter
                    topPadding: 16
                    bottomPadding: 16
                }
            }
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================

    SignalSpy { id: channelClickedSpy; target: searchResults; signalName: "channelClicked" }
    SignalSpy { id: cacheVodClickedSpy; target: searchResults; signalName: "cacheVodClicked" }
    SignalSpy { id: closeRequestedSpy; target: searchResults; signalName: "closeRequested" }

    // =========================================================================
    // Test Data
    // =========================================================================

    property var emptyChannelResults: []
    property var emptyCacheResults: []

    property var singleChannelResult: [
        { broadcaster_login: "ninja", display_name: "Ninja", is_live: true, thumbnail_url: "https://example.com/ninja.jpg", game_name: "Fortnite", title: "Playing Fortnite!" }
    ]

    property var multipleChannelResults: [
        { broadcaster_login: "shroud", display_name: "shroud", is_live: true, thumbnail_url: "", game_name: "Valorant", title: "Ranked grind" },
        { broadcaster_login: "pokimane", display_name: "pokimane", is_live: true, thumbnail_url: "", game_name: "Just Chatting", title: "Chatting with chat" },
        { broadcaster_login: "xqc", display_name: "xQc", is_live: false, thumbnail_url: "", game_name: "", title: "" }
    ]

    property var singleCacheResult: [
        { id: "vod123", filePath: "/cache/vod123.mp4", streamerName: "DrLupo", streamTitle: "Charity stream", recordedAtFormatted: "Dec 25", durationFormatted: "3h 45m", fileSizeFormatted: "2.1 GB" }
    ]

    property var multipleCacheResults: [
        { id: "vod456", filePath: "/cache/vod456.mp4", streamerName: "TimTheTatman", streamTitle: "Warzone session", recordedAtFormatted: "Dec 24", durationFormatted: "2h 30m", fileSizeFormatted: "1.5 GB" },
        { id: "vod789", filePath: "/cache/vod789.mp4", streamerName: "Summit1g", streamTitle: "GTA RP", recordedAtFormatted: "Dec 23", durationFormatted: "5h 00m", fileSizeFormatted: "3.2 GB" }
    ]

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
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================

        function test_defaultState_notVisible() {
            compare(searchResults.visible, false, "Should be hidden by default")
        }

        function test_defaultState_channelResultsEmpty() {
            compare(searchResults.channelResults.length, 0, "channelResults should be empty by default")
        }

        function test_defaultState_cacheResultsEmpty() {
            compare(searchResults.cacheResults.length, 0, "cacheResults should be empty by default")
        }

        function test_defaultState_selectedIndexNegative() {
            compare(searchResults.selectedIndex, -1, "selectedIndex should be -1 by default")
        }

        function test_defaultState_totalCountZero() {
            compare(searchResults.totalCount, 0, "totalCount should be 0 by default")
        }

        function test_defaultState_hasSearchQueryFalse() {
            compare(searchResults.hasSearchQuery, false, "hasSearchQuery should be false by default")
        }

        function test_defaultState_isVisibleFalse() {
            compare(searchResults.isVisible, false, "isVisible should be false by default")
        }

        // =====================================================================
        // TEST: Visibility Logic
        // =====================================================================

        function test_visibility_hiddenWhenIsVisibleFalse() {
            // Arrange
            searchResults.hasSearchQuery = true
            searchResults.isVisible = false

            // Assert
            compare(searchResults.visible, false, "Should be hidden when isVisible is false")
        }

        function test_visibility_hiddenWhenHasSearchQueryFalse() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = false

            // Assert
            compare(searchResults.visible, false, "Should be hidden when hasSearchQuery is false")
        }

        function test_visibility_visibleWhenBothTrue() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true

            // Assert
            compare(searchResults.visible, true, "Should be visible when both isVisible and hasSearchQuery are true")
        }

        function test_visibility_showsNoResultsWhenEmpty() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = []
            searchResults.cacheResults = []

            // Assert
            var noResultsText = findChild(searchResults, "noResultsText")
            verify(noResultsText !== null, "No results text should exist")
            compare(noResultsText.visible, true, "No results text should be visible")
        }

        // =====================================================================
        // TEST: Channel Results Display
        // =====================================================================

        function test_channelResults_singleResult() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult

            // Assert
            var channelRepeater = findChild(searchResults, "channelRepeater")
            compare(channelRepeater.count, 1, "Should display 1 channel")
        }

        function test_channelResults_multipleResults() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = multipleChannelResults

            // Assert
            var channelRepeater = findChild(searchResults, "channelRepeater")
            compare(channelRepeater.count, 3, "Should display 3 channels")
        }

        function test_channelResults_displayName() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult

            // Assert
            var nameText = findChild(searchResults, "channelNameText_0")
            verify(nameText !== null, "Channel name text should exist")
            compare(nameText.text, "Ninja", "Should display channel display_name")
        }

        function test_channelResults_liveBadgeVisible() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult

            // Assert
            var liveBadge = findChild(searchResults, "liveBadge_0")
            verify(liveBadge !== null, "Live badge should exist")
            compare(liveBadge.visible, true, "Live badge should be visible for live channel")
        }

        function test_channelResults_liveBadgeHiddenWhenOffline() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = [{ broadcaster_login: "test", display_name: "Test", is_live: false }]

            // Assert
            var liveBadge = findChild(searchResults, "liveBadge_0")
            verify(liveBadge !== null, "Live badge should exist")
            compare(liveBadge.visible, false, "Live badge should be hidden for offline channel")
        }

        function test_channelResults_liveHeaderVisible() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult

            // Assert
            var liveHeader = findChild(searchResults, "liveHeader")
            verify(liveHeader !== null, "Live header should exist")
            compare(liveHeader.visible, true, "Live header should be visible when channels exist")
        }

        function test_channelResults_liveHeaderHiddenWhenEmpty() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = []

            // Assert
            var liveHeader = findChild(searchResults, "liveHeader")
            compare(liveHeader.visible, false, "Live header should be hidden when no channels")
        }

        // =====================================================================
        // TEST: Cache Results Display
        // =====================================================================

        function test_cacheResults_singleResult() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = singleCacheResult

            // Assert
            var cacheRepeater = findChild(searchResults, "cacheRepeater")
            compare(cacheRepeater.count, 1, "Should display 1 cached VOD")
        }

        function test_cacheResults_multipleResults() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = multipleCacheResults

            // Assert
            var cacheRepeater = findChild(searchResults, "cacheRepeater")
            compare(cacheRepeater.count, 2, "Should display 2 cached VODs")
        }

        function test_cacheResults_streamerName() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = singleCacheResult

            // Assert
            var streamerName = findChild(searchResults, "streamerNameText_0")
            verify(streamerName !== null, "Streamer name text should exist")
            compare(streamerName.text, "DrLupo", "Should display streamer name")
        }

        function test_cacheResults_streamTitle() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = singleCacheResult

            // Assert
            var streamTitle = findChild(searchResults, "streamTitleText_0")
            verify(streamTitle !== null, "Stream title text should exist")
            compare(streamTitle.text, "Charity stream", "Should display stream title")
        }

        function test_cacheResults_cacheBadgeVisible() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = singleCacheResult

            // Assert
            var cacheBadge = findChild(searchResults, "cacheBadge_0")
            verify(cacheBadge !== null, "Cache badge should exist")
            compare(cacheBadge.visible, true, "Cache badge should be visible")
        }

        function test_cacheResults_cacheHeaderVisible() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = singleCacheResult

            // Assert
            var cacheHeader = findChild(searchResults, "cacheHeader")
            verify(cacheHeader !== null, "Cache header should exist")
            compare(cacheHeader.visible, true, "Cache header should be visible when cache results exist")
        }

        function test_cacheResults_cacheHeaderHiddenWhenEmpty() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = []

            // Assert
            var cacheHeader = findChild(searchResults, "cacheHeader")
            compare(cacheHeader.visible, false, "Cache header should be hidden when no cache results")
        }

        // =====================================================================
        // TEST: Separator Display
        // =====================================================================

        function test_separator_visibleWhenBothSections() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult
            searchResults.cacheResults = singleCacheResult

            // Assert
            var separator = findChild(searchResults, "separator")
            verify(separator !== null, "Separator should exist")
            compare(separator.visible, true, "Separator should be visible when both sections have results")
        }

        function test_separator_hiddenWhenOnlyChannels() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult
            searchResults.cacheResults = []

            // Assert
            var separator = findChild(searchResults, "separator")
            compare(separator.visible, false, "Separator should be hidden when only channels")
        }

        function test_separator_hiddenWhenOnlyCache() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = []
            searchResults.cacheResults = singleCacheResult

            // Assert
            var separator = findChild(searchResults, "separator")
            compare(separator.visible, false, "Separator should be hidden when only cache")
        }

        // =====================================================================
        // TEST: Total Count Calculation
        // =====================================================================

        function test_totalCount_channelsOnly() {
            // Arrange
            searchResults.channelResults = multipleChannelResults
            searchResults.cacheResults = []

            // Assert
            compare(searchResults.totalCount, 3, "totalCount should be 3 with 3 channels")
        }

        function test_totalCount_cacheOnly() {
            // Arrange
            searchResults.channelResults = []
            searchResults.cacheResults = multipleCacheResults

            // Assert
            compare(searchResults.totalCount, 2, "totalCount should be 2 with 2 cache items")
        }

        function test_totalCount_combined() {
            // Arrange
            searchResults.channelResults = multipleChannelResults
            searchResults.cacheResults = multipleCacheResults

            // Assert
            compare(searchResults.totalCount, 5, "totalCount should be 5 (3 channels + 2 cache)")
        }

        // =====================================================================
        // TEST: Selection State
        // =====================================================================

        function test_selection_initiallyNone() {
            // Arrange
            searchResults.channelResults = singleChannelResult

            // Assert
            compare(searchResults.selectedIndex, -1, "Selection should reset to -1 when results change")
        }

        function test_selection_channelHighlighted() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult
            searchResults.selectedIndex = 0

            // Assert
            var channelDelegate = findChild(searchResults, "channelDelegate_0")
            verify(channelDelegate !== null, "Channel delegate should exist")
            compare(channelDelegate.border.width, 1, "Selected channel should have border")
            compare(channelDelegate.border.color, root.themeAccent, "Border should be accent color")
        }

        function test_selection_cacheHighlighted() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult
            searchResults.cacheResults = singleCacheResult
            searchResults.selectedIndex = 1 // First cache item (after 1 channel)

            // Assert
            var cacheDelegate = findChild(searchResults, "cacheDelegate_0")
            verify(cacheDelegate !== null, "Cache delegate should exist")
            compare(cacheDelegate.border.width, 1, "Selected cache item should have border")
            compare(cacheDelegate.border.color, root.themeAccent, "Border should be accent color")
        }

        function test_selection_resetOnChannelResultsChange() {
            // Arrange
            searchResults.channelResults = singleChannelResult
            searchResults.selectedIndex = 0

            // Act
            searchResults.channelResults = multipleChannelResults

            // Assert
            compare(searchResults.selectedIndex, -1, "Selection should reset when channelResults change")
        }

        function test_selection_resetOnCacheResultsChange() {
            // Arrange
            searchResults.cacheResults = singleCacheResult
            searchResults.selectedIndex = 0

            // Act
            searchResults.cacheResults = multipleCacheResults

            // Assert
            compare(searchResults.selectedIndex, -1, "Selection should reset when cacheResults change")
        }

        // =====================================================================
        // TEST: Navigation
        // =====================================================================

        function test_navigation_navigateDownFromStart() {
            // Arrange
            searchResults.channelResults = multipleChannelResults
            searchResults.selectedIndex = -1

            // Act
            searchResults.navigateDown()

            // Assert
            compare(searchResults.selectedIndex, 0, "navigateDown from -1 should select first item")
        }

        function test_navigation_navigateDownIncrement() {
            // Arrange
            searchResults.channelResults = multipleChannelResults
            searchResults.selectedIndex = 0

            // Act
            searchResults.navigateDown()

            // Assert
            compare(searchResults.selectedIndex, 1, "navigateDown should increment selection")
        }

        function test_navigation_navigateDownWrapsAround() {
            // Arrange
            searchResults.channelResults = multipleChannelResults // 3 items
            searchResults.selectedIndex = 2

            // Act
            searchResults.navigateDown()

            // Assert
            compare(searchResults.selectedIndex, 0, "navigateDown at end should wrap to 0")
        }

        function test_navigation_navigateUpDecrement() {
            // Arrange
            searchResults.channelResults = multipleChannelResults
            searchResults.selectedIndex = 2

            // Act
            searchResults.navigateUp()

            // Assert
            compare(searchResults.selectedIndex, 1, "navigateUp should decrement selection")
        }

        function test_navigation_navigateUpWrapsAround() {
            // Arrange
            searchResults.channelResults = multipleChannelResults // 3 items
            searchResults.selectedIndex = 0

            // Act
            searchResults.navigateUp()

            // Assert
            compare(searchResults.selectedIndex, 2, "navigateUp at 0 should wrap to last item")
        }

        function test_navigation_acrossSections() {
            // Arrange
            searchResults.channelResults = singleChannelResult // 1 channel
            searchResults.cacheResults = singleCacheResult // 1 cache
            searchResults.selectedIndex = 0 // On channel

            // Act
            searchResults.navigateDown()

            // Assert
            compare(searchResults.selectedIndex, 1, "navigateDown should move to cache section")
        }

        // =====================================================================
        // TEST: Signals - Channel Click
        // =====================================================================

        function test_signal_channelClicked_emitted() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult

            var mouseArea = findChild(searchResults, "channelMouseArea_0")
            verify(mouseArea !== null, "Channel mouse area should exist")

            // Act
            mouseClick(mouseArea)

            // Assert
            compare(channelClickedSpy.count, 1, "channelClicked signal should be emitted once")
        }

        function test_signal_channelClicked_arguments() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult

            var mouseArea = findChild(searchResults, "channelMouseArea_0")

            // Act
            mouseClick(mouseArea)

            // Assert
            compare(channelClickedSpy.signalArguments[0][0], "ninja", "broadcaster_login should be 'ninja'")
            compare(channelClickedSpy.signalArguments[0][1], "Ninja", "display_name should be 'Ninja'")
            compare(channelClickedSpy.signalArguments[0][2], true, "is_live should be true")
            compare(channelClickedSpy.signalArguments[0][3], "https://example.com/ninja.jpg", "thumbnail_url should match")
        }

        function test_signal_selectCurrentChannel() {
            // Arrange
            searchResults.channelResults = singleChannelResult
            searchResults.selectedIndex = 0

            // Act
            searchResults.selectCurrent()

            // Assert
            compare(channelClickedSpy.count, 1, "selectCurrent on channel should emit channelClicked")
        }

        // =====================================================================
        // TEST: Signals - Cache VOD Click
        // =====================================================================

        function test_signal_cacheVodClicked_emitted() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = singleCacheResult

            var mouseArea = findChild(searchResults, "cacheMouseArea_0")
            verify(mouseArea !== null, "Cache mouse area should exist")

            // Act
            mouseClick(mouseArea)

            // Assert
            compare(cacheVodClickedSpy.count, 1, "cacheVodClicked signal should be emitted once")
        }

        function test_signal_cacheVodClicked_arguments() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = singleCacheResult

            var mouseArea = findChild(searchResults, "cacheMouseArea_0")

            // Act
            mouseClick(mouseArea)

            // Assert
            compare(cacheVodClickedSpy.signalArguments[0][0], "vod123", "vodId should be 'vod123'")
            compare(cacheVodClickedSpy.signalArguments[0][1], "/cache/vod123.mp4", "filePath should match")
            compare(cacheVodClickedSpy.signalArguments[0][2], "DrLupo", "streamerName should be 'DrLupo'")
        }

        function test_signal_selectCurrentCache() {
            // Arrange
            searchResults.channelResults = singleChannelResult
            searchResults.cacheResults = singleCacheResult
            searchResults.selectedIndex = 1 // Cache item

            // Act
            searchResults.selectCurrent()

            // Assert
            compare(cacheVodClickedSpy.count, 1, "selectCurrent on cache should emit cacheVodClicked")
        }

        function test_signal_selectCurrentWithNoSelection() {
            // Arrange
            searchResults.channelResults = singleChannelResult
            searchResults.selectedIndex = -1

            // Act
            searchResults.selectCurrent()

            // Assert
            compare(channelClickedSpy.count, 0, "selectCurrent with -1 should not emit signal")
            compare(cacheVodClickedSpy.count, 0, "selectCurrent with -1 should not emit cacheVodClicked")
        }

        // =====================================================================
        // TEST: Data-Driven Tests - Multiple Channels
        // =====================================================================

        function test_channelNames_data() {
            return [
                { tag: "first", index: 0, expectedName: "shroud" },
                { tag: "second", index: 1, expectedName: "pokimane" },
                { tag: "third", index: 2, expectedName: "xQc" }
            ]
        }

        function test_channelNames(data) {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = multipleChannelResults

            // Assert
            var nameText = findChild(searchResults, "channelNameText_" + data.index)
            verify(nameText !== null, "Channel name text " + data.index + " should exist")
            compare(nameText.text, data.expectedName, "Channel " + data.index + " should show " + data.expectedName)
        }

        // =====================================================================
        // TEST: Edge Cases
        // =====================================================================

        function test_edgeCase_emptyDisplayName() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = [{ broadcaster_login: "testuser", display_name: "", is_live: true }]

            // Assert
            var nameText = findChild(searchResults, "channelNameText_0")
            compare(nameText.text, "testuser", "Should fallback to broadcaster_login when display_name is empty")
        }

        function test_edgeCase_nullChannelInfo() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = [null]

            // Assert - should not crash
            var channelRepeater = findChild(searchResults, "channelRepeater")
            compare(channelRepeater.count, 1, "Should handle null in model")
        }

        function test_edgeCase_emptyStreamerName() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.cacheResults = [{ id: "test", filePath: "/test", streamerName: "", streamTitle: "Test" }]

            // Assert
            var streamerName = findChild(searchResults, "streamerNameText_0")
            compare(streamerName.text, "Unknown", "Should show 'Unknown' for empty streamer name")
        }

        function test_edgeCase_rapidModelChanges() {
            // Arrange & Act - rapid changes
            searchResults.channelResults = singleChannelResult
            searchResults.channelResults = multipleChannelResults
            searchResults.channelResults = emptyChannelResults
            searchResults.channelResults = singleChannelResult

            // Assert
            compare(searchResults.totalCount, 1, "Should handle rapid model changes")
            compare(searchResults.selectedIndex, -1, "Selection should be reset")
        }

        // =====================================================================
        // TEST: No Results State
        // =====================================================================

        function test_noResults_textVisibleWhenEmpty() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = []
            searchResults.cacheResults = []

            // Assert
            var noResultsText = findChild(searchResults, "noResultsText")
            verify(noResultsText !== null, "No results text should exist")
            compare(noResultsText.visible, true, "No results text should be visible")
            compare(noResultsText.text, "No results", "Should show 'No results' message")
        }

        function test_noResults_textHiddenWhenResults() {
            // Arrange
            searchResults.isVisible = true
            searchResults.hasSearchQuery = true
            searchResults.channelResults = singleChannelResult

            // Assert
            var noResultsText = findChild(searchResults, "noResultsText")
            compare(noResultsText.visible, false, "No results text should be hidden when results exist")
        }
    }
}
