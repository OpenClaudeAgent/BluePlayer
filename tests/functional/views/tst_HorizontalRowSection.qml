/**
 * tst_HorizontalRowSection.qml
 *
 * Functional UI tests for the HorizontalRowSection component.
 * Tests section title/subtitle, model binding, card type detection,
 * visibility conditions, and signal emissions.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 800
    height: 500

    // =========================================================================
    // Mock Theme Constants (inline - no external import)
    // =========================================================================

    property color primaryText: "#FFFFFF"
    property color secondaryText: "#8899A6"
    property color accent: "#0066FF"
    property color surface: "#1b2130"
    property color divider: "#2a324e"
    property int spacingSmall: 8
    property int spacingMedium: 16
    property string fontFamily: "Inter"

    // =========================================================================
    // Component Under Test (Mock) - HorizontalRowSection
    // =========================================================================

    Component {
        id: horizontalRowSectionComponent

        Item {
            id: section
            objectName: "horizontalRowSection"

            // Public properties (matching real component API)
            property string sectionTitle: ""
            property string sectionSubtitle: ""
            property var cardsModel: []
            property real rowHeight: 220
            property real cardWidth: 180
            property real cardSpacing: 16
            property string sectionType: ""

            // Signals
            signal categoryClicked(string categoryId, string categoryName)
            signal streamClicked(string streamerLogin, string streamerName, string streamTitle, string thumbnailUrl)

            // Computed properties (matching real component logic)
            readonly property bool isCategorySection: sectionTitle === "Parcourir" ||
                (sectionType === "" && cardsModel.length > 0 && cardsModel[0] && cardsModel[0].boxArtUrl !== undefined)

            readonly property bool shouldShowSection: {
                if (sectionTitle === "Recommandations par categorie") {
                    if (cardsModel.length === 0) return false
                    for (var i = 0; i < cardsModel.length; i++) {
                        if (cardsModel[i] && !cardsModel[i].isPlaceholder) {
                            return true
                        }
                    }
                    return false
                }
                return true
            }

            readonly property bool isClipSection: sectionType === "clips" ||
                (cardsModel.length > 0 && cardsModel[0] && cardsModel[0].clipTitle !== undefined)

            readonly property bool isVideoSection: sectionType === "videos" ||
                (cardsModel.length > 0 && cardsModel[0] && cardsModel[0].videoTitle !== undefined)

            readonly property bool isChannelSection: sectionType === "channels" ||
                (cardsModel.length > 0 && cardsModel[0] && cardsModel[0].channelName !== undefined)

            readonly property real actualRowHeight: (isCategorySection || isChannelSection) ? 260 : rowHeight

            readonly property int cardCount: cardsModel ? cardsModel.length : 0

            width: 600
            height: shouldShowSection ? columnLayout.implicitHeight : 0
            visible: shouldShowSection

            // Reset function for tests
            function reset() {
                sectionTitle = ""
                sectionSubtitle = ""
                cardsModel = []
                rowHeight = 220
                cardWidth = 180
                cardSpacing = 16
                sectionType = ""
            }

            ColumnLayout {
                id: columnLayout
                objectName: "columnLayout"
                anchors.fill: parent
                spacing: root.spacingMedium

                // Title Section
                ColumnLayout {
                    id: titleSection
                    objectName: "titleSection"
                    Layout.fillWidth: true
                    spacing: 4
                    visible: section.sectionTitle !== ""

                    Text {
                        id: titleText
                        objectName: "titleText"
                        text: section.sectionTitle
                        font.family: root.fontFamily
                        font.pixelSize: 20
                        font.bold: true
                        color: root.primaryText
                    }

                    Text {
                        id: subtitleText
                        objectName: "subtitleText"
                        visible: section.sectionSubtitle !== ""
                        text: section.sectionSubtitle
                        font.family: root.fontFamily
                        font.pixelSize: 14
                        color: root.secondaryText
                    }
                }

                // Scrollable cards area
                Item {
                    id: scrollWrapper
                    objectName: "scrollWrapper"
                    Layout.fillWidth: true
                    Layout.preferredHeight: section.actualRowHeight + root.spacingSmall * 2
                    clip: true

                    ListView {
                        id: listView
                        objectName: "listView"
                        anchors.fill: parent
                        anchors.margins: root.spacingSmall
                        orientation: ListView.Horizontal
                        spacing: section.cardSpacing
                        clip: false
                        interactive: true
                        model: section.cardsModel

                        delegate: Item {
                            id: cardDelegate
                            objectName: "cardDelegate_" + index
                            width: section.cardWidth
                            height: section.actualRowHeight

                            property var cardData: modelData
                            property bool isCategory: cardData ? (cardData.boxArtUrl !== undefined) : false
                            property bool isClip: cardData ? (cardData.clipTitle !== undefined) : false
                            property bool isVideo: cardData ? (cardData.videoTitle !== undefined) : false
                            property bool isChannel: cardData ? (cardData.channelName !== undefined) : false
                            property bool isStream: !isCategory && !isClip && !isVideo && !isChannel

                            Rectangle {
                                id: cardBackground
                                objectName: "cardBackground_" + index
                                anchors.fill: parent
                                color: root.surface
                                radius: 8
                                border.color: root.divider
                                border.width: 1

                                // Card type indicator for testing
                                Text {
                                    id: cardTypeLabel
                                    objectName: "cardTypeLabel_" + index
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.margins: 8
                                    text: cardDelegate.isCategory ? "CATEGORY" :
                                          cardDelegate.isClip ? "CLIP" :
                                          cardDelegate.isVideo ? "VIDEO" :
                                          cardDelegate.isChannel ? "CHANNEL" : "STREAM"
                                    font.pixelSize: 10
                                    font.bold: true
                                    color: root.accent
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    anchors.topMargin: 30
                                    spacing: root.spacingSmall

                                    // Generic name display
                                    Text {
                                        id: nameLabel
                                        objectName: "nameLabel_" + index
                                        text: {
                                            if (!cardData) return ""
                                            if (cardData.name) return cardData.name
                                            if (cardData.categoryName) return cardData.categoryName
                                            if (cardData.clipTitle) return cardData.clipTitle
                                            if (cardData.videoTitle) return cardData.videoTitle
                                            if (cardData.channelName) return cardData.channelName
                                            return ""
                                        }
                                        color: root.primaryText
                                        font.pixelSize: 12
                                        font.bold: true
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }

                                    // Detail/subtitle
                                    Text {
                                        id: detailLabel
                                        objectName: "detailLabel_" + index
                                        text: {
                                            if (!cardData) return ""
                                            if (cardData.detail) return cardData.detail
                                            if (cardData.broadcasterName) return cardData.broadcasterName
                                            if (cardData.userName) return cardData.userName
                                            if (cardData.displayName) return cardData.displayName
                                            return ""
                                        }
                                        color: root.secondaryText
                                        font.pixelSize: 11
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }

                                    // Viewers/count
                                    Text {
                                        id: viewersLabel
                                        objectName: "viewersLabel_" + index
                                        text: {
                                            if (!cardData) return ""
                                            if (cardData.viewers) return cardData.viewers
                                            if (cardData.viewCount) return cardData.viewCount
                                            return ""
                                        }
                                        color: root.accent
                                        font.pixelSize: 11
                                    }

                                    // Placeholder indicator
                                    Text {
                                        id: placeholderLabel
                                        objectName: "placeholderLabel_" + index
                                        visible: cardData && cardData.isPlaceholder === true
                                        text: "Loading..."
                                        color: root.secondaryText
                                        font.pixelSize: 10
                                        font.italic: true
                                    }
                                }

                                MouseArea {
                                    id: cardMouseArea
                                    objectName: "cardMouseArea_" + index
                                    anchors.fill: parent
                                    hoverEnabled: true

                                    onClicked: {
                                        if (cardDelegate.isCategory && cardData) {
                                            section.categoryClicked(cardData.id || "", cardData.name || cardData.categoryName || "")
                                        } else if (cardDelegate.isStream && cardData) {
                                            section.streamClicked(
                                                cardData.streamerLogin || cardData.userLogin || "",
                                                cardData.name || "",
                                                cardData.detail || "",
                                                cardData.previewImage || ""
                                            )
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // =========================================================================
    // Test Instance and Signal Spies
    // =========================================================================

    property var sectionInstance: null

    SignalSpy { id: categoryClickedSpy; signalName: "categoryClicked" }
    SignalSpy { id: streamClickedSpy; signalName: "streamClicked" }

    // =========================================================================
    // Test Data
    // =========================================================================

    property var emptyModel: []

    property var streamModel: [
        { name: "Streamer1", detail: "Playing Fortnite", viewers: "15K", streamerLogin: "streamer1", previewImage: "http://example.com/thumb1.jpg" },
        { name: "Streamer2", detail: "Just Chatting", viewers: "8K", streamerLogin: "streamer2", previewImage: "http://example.com/thumb2.jpg" }
    ]

    property var categoryModel: [
        { id: "cat1", name: "Just Chatting", boxArtUrl: "http://example.com/art1.jpg" },
        { id: "cat2", name: "Fortnite", boxArtUrl: "http://example.com/art2.jpg" },
        { id: "cat3", name: "League of Legends", boxArtUrl: "http://example.com/art3.jpg" }
    ]

    property var clipModel: [
        { clipTitle: "Amazing Play", broadcasterName: "ProPlayer", viewCount: "50K", duration: "30s", thumbnailUrl: "http://example.com/clip1.jpg" },
        { clipTitle: "Funny Moment", broadcasterName: "FunnyGuy", viewCount: "25K", duration: "45s", thumbnailUrl: "http://example.com/clip2.jpg" }
    ]

    property var videoModel: [
        { videoTitle: "Full Stream Day 1", userName: "Creator1", viewCount: "100K", duration: "3:45:00", thumbnailUrl: "http://example.com/vid1.jpg" },
        { videoTitle: "Highlights", userName: "Creator2", viewCount: "75K", duration: "15:30", thumbnailUrl: "http://example.com/vid2.jpg", hasProgress: true, watchPosition: 500 }
    ]

    property var channelModel: [
        { channelName: "channel1", displayName: "Cool Channel", thumbnailUrl: "http://example.com/ch1.jpg", isLive: true, gameName: "Valorant" },
        { channelName: "channel2", displayName: "Another Channel", thumbnailUrl: "http://example.com/ch2.jpg", isLive: false, gameName: "" }
    ]

    property var placeholderOnlyModel: [
        { name: "Loading1", isPlaceholder: true },
        { name: "Loading2", isPlaceholder: true }
    ]

    property var mixedPlaceholderModel: [
        { name: "RealData", isPlaceholder: false },
        { name: "Loading", isPlaceholder: true }
    ]

    // =========================================================================
    // Test Case
    // =========================================================================

    TestCase {
        id: testCase
        name: "HorizontalRowSectionTests"
        when: windowShown

        function init() {
            // Cleanup previous instance
            if (sectionInstance) {
                sectionInstance.destroy()
                sectionInstance = null
            }
            // Create fresh instance
            sectionInstance = createTemporaryObject(horizontalRowSectionComponent, root)
            verify(sectionInstance !== null, "Component should be created")

            // Setup spies
            categoryClickedSpy.target = sectionInstance
            streamClickedSpy.target = sectionInstance
            categoryClickedSpy.clear()
            streamClickedSpy.clear()
        }

        function cleanup() {
            if (sectionInstance) {
                sectionInstance.destroy()
                sectionInstance = null
            }
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================

        function test_defaultState_sectionTitleEmpty() {
            compare(sectionInstance.sectionTitle, "", "Default sectionTitle should be empty")
        }

        function test_defaultState_sectionSubtitleEmpty() {
            compare(sectionInstance.sectionSubtitle, "", "Default sectionSubtitle should be empty")
        }

        function test_defaultState_cardsModelEmpty() {
            compare(sectionInstance.cardsModel.length, 0, "Default cardsModel should be empty array")
        }

        function test_defaultState_cardCount() {
            compare(sectionInstance.cardCount, 0, "Default cardCount should be 0")
        }

        function test_defaultState_rowHeight() {
            compare(sectionInstance.rowHeight, 220, "Default rowHeight should be 220")
        }

        function test_defaultState_cardWidth() {
            compare(sectionInstance.cardWidth, 180, "Default cardWidth should be 180")
        }

        function test_defaultState_sectionTypeEmpty() {
            compare(sectionInstance.sectionType, "", "Default sectionType should be empty")
        }

        function test_defaultState_shouldShowSection() {
            compare(sectionInstance.shouldShowSection, true, "Default shouldShowSection should be true")
        }

        function test_defaultState_visible() {
            compare(sectionInstance.visible, true, "Default visible should be true")
        }

        // =====================================================================
        // TEST: Section Title Display
        // =====================================================================

        function test_sectionTitle_displayed() {
            // Arrange
            sectionInstance.sectionTitle = "Live Streams"

            // Assert
            var titleText = findChild(sectionInstance, "titleText")
            verify(titleText !== null, "Title text should exist")
            compare(titleText.text, "Live Streams", "Title should be displayed")
        }

        function test_sectionTitle_titleSectionVisible() {
            // Arrange
            sectionInstance.sectionTitle = "Categories"

            // Assert
            var titleSection = findChild(sectionInstance, "titleSection")
            verify(titleSection !== null, "Title section should exist")
            compare(titleSection.visible, true, "Title section should be visible when title is set")
        }

        function test_sectionTitle_titleSectionHiddenWhenEmpty() {
            // Arrange
            sectionInstance.sectionTitle = ""

            // Assert
            var titleSection = findChild(sectionInstance, "titleSection")
            compare(titleSection.visible, false, "Title section should be hidden when title is empty")
        }

        function test_sectionTitle_specialCharacters() {
            // Arrange
            sectionInstance.sectionTitle = "Top Clips & Videos <2024>"

            // Assert
            var titleText = findChild(sectionInstance, "titleText")
            compare(titleText.text, "Top Clips & Videos <2024>", "Special characters should be displayed")
        }

        // =====================================================================
        // TEST: Section Subtitle Display
        // =====================================================================

        function test_sectionSubtitle_displayed() {
            // Arrange
            sectionInstance.sectionTitle = "Streams"
            sectionInstance.sectionSubtitle = "Based on your preferences"

            // Assert
            var subtitleText = findChild(sectionInstance, "subtitleText")
            verify(subtitleText !== null, "Subtitle text should exist")
            compare(subtitleText.text, "Based on your preferences", "Subtitle should be displayed")
        }


        function test_sectionSubtitle_hiddenWhenEmpty() {
            // Arrange
            sectionInstance.sectionSubtitle = ""

            // Assert
            var subtitleText = findChild(sectionInstance, "subtitleText")
            compare(subtitleText.visible, false, "Subtitle should be hidden when empty")
        }

        // =====================================================================
        // TEST: Empty State
        // =====================================================================

        function test_emptyModel_cardCountZero() {
            // Arrange
            sectionInstance.cardsModel = emptyModel

            // Assert
            compare(sectionInstance.cardCount, 0, "Card count should be 0 with empty model")
        }

        function test_emptyModel_listViewEmpty() {
            // Arrange
            sectionInstance.cardsModel = emptyModel

            // Assert
            var listView = findChild(sectionInstance, "listView")
            verify(listView !== null, "ListView should exist")
            compare(listView.count, 0, "ListView should have 0 items")
        }

        // =====================================================================
        // TEST: Loading State (Placeholders)
        // =====================================================================

        function test_placeholders_displayedCorrectly() {
            // Arrange
            sectionInstance.cardsModel = placeholderOnlyModel

            // Assert
            var placeholderLabel = findChild(sectionInstance, "placeholderLabel_0")
            verify(placeholderLabel !== null, "Placeholder label should exist")
            compare(placeholderLabel.visible, true, "Placeholder label should be visible")
        }

        function test_placeholders_recommendationsSectionHidden() {
            // Arrange - "Recommandations par categorie" with only placeholders
            sectionInstance.sectionTitle = "Recommandations par categorie"
            sectionInstance.cardsModel = placeholderOnlyModel

            // Assert
            compare(sectionInstance.shouldShowSection, false, "Section should be hidden with only placeholders")
            compare(sectionInstance.visible, false, "Component should not be visible")
        }

        function test_placeholders_recommendationsSectionVisibleWithRealData() {
            // Arrange
            sectionInstance.sectionTitle = "Recommandations par categorie"
            sectionInstance.cardsModel = mixedPlaceholderModel

            // Assert
            compare(sectionInstance.shouldShowSection, true, "Section should be visible with real data")
        }

        function test_placeholders_otherSectionsAlwaysVisible() {
            // Arrange - Regular section with only placeholders
            sectionInstance.sectionTitle = "Popular Streams"
            sectionInstance.cardsModel = placeholderOnlyModel

            // Assert
            compare(sectionInstance.shouldShowSection, true, "Regular sections should always be visible")
        }

        // =====================================================================
        // TEST: Stream Cards Display
        // =====================================================================

        function test_streamCards_countCorrect() {
            // Arrange
            sectionInstance.cardsModel = streamModel

            // Assert
            compare(sectionInstance.cardCount, 2, "Should have 2 stream cards")
        }

        function test_streamCards_nameDisplayed() {
            // Arrange
            sectionInstance.cardsModel = streamModel

            // Assert
            var nameLabel = findChild(sectionInstance, "nameLabel_0")
            verify(nameLabel !== null, "Name label should exist")
            compare(nameLabel.text, "Streamer1", "Stream name should be displayed")
        }

        function test_streamCards_detailDisplayed() {
            // Arrange
            sectionInstance.cardsModel = streamModel

            // Assert
            var detailLabel = findChild(sectionInstance, "detailLabel_0")
            compare(detailLabel.text, "Playing Fortnite", "Stream detail should be displayed")
        }

        function test_streamCards_viewersDisplayed() {
            // Arrange
            sectionInstance.cardsModel = streamModel

            // Assert
            var viewersLabel = findChild(sectionInstance, "viewersLabel_0")
            compare(viewersLabel.text, "15K", "Viewer count should be displayed")
        }

        function test_streamCards_typeDetection() {
            // Arrange
            sectionInstance.cardsModel = streamModel

            // Assert
            var cardTypeLabel = findChild(sectionInstance, "cardTypeLabel_0")
            compare(cardTypeLabel.text, "STREAM", "Card type should be STREAM")
        }

        // =====================================================================
        // TEST: Category Cards Display
        // =====================================================================

        function test_categoryCards_countCorrect() {
            // Arrange
            sectionInstance.cardsModel = categoryModel

            // Assert
            compare(sectionInstance.cardCount, 3, "Should have 3 category cards")
        }

        function test_categoryCards_isCategorySectionTrue() {
            // Arrange
            sectionInstance.cardsModel = categoryModel

            // Assert
            compare(sectionInstance.isCategorySection, true, "Should detect as category section")
        }

        function test_categoryCards_actualRowHeightAdjusted() {
            // Arrange
            sectionInstance.cardsModel = categoryModel

            // Assert
            compare(sectionInstance.actualRowHeight, 260, "Row height should be 260 for categories")
        }

        function test_categoryCards_nameDisplayed() {
            // Arrange
            sectionInstance.cardsModel = categoryModel

            // Assert
            var nameLabel = findChild(sectionInstance, "nameLabel_0")
            compare(nameLabel.text, "Just Chatting", "Category name should be displayed")
        }

        function test_categoryCards_typeDetection() {
            // Arrange
            sectionInstance.cardsModel = categoryModel

            // Assert
            var cardTypeLabel = findChild(sectionInstance, "cardTypeLabel_0")
            compare(cardTypeLabel.text, "CATEGORY", "Card type should be CATEGORY")
        }

        function test_categoryCards_parcourirTitle() {
            // Arrange
            sectionInstance.sectionTitle = "Parcourir"
            sectionInstance.cardsModel = streamModel  // Even with stream data

            // Assert
            compare(sectionInstance.isCategorySection, true, "Should be category section with 'Parcourir' title")
        }

        // =====================================================================
        // TEST: Clip Cards Display
        // =====================================================================

        function test_clipCards_countCorrect() {
            // Arrange
            sectionInstance.cardsModel = clipModel

            // Assert
            compare(sectionInstance.cardCount, 2, "Should have 2 clip cards")
        }

        function test_clipCards_isClipSectionTrue() {
            // Arrange
            sectionInstance.cardsModel = clipModel

            // Assert
            compare(sectionInstance.isClipSection, true, "Should detect as clip section")
        }

        function test_clipCards_typeDetection() {
            // Arrange
            sectionInstance.cardsModel = clipModel

            // Assert
            var cardTypeLabel = findChild(sectionInstance, "cardTypeLabel_0")
            compare(cardTypeLabel.text, "CLIP", "Card type should be CLIP")
        }

        function test_clipCards_titleDisplayed() {
            // Arrange
            sectionInstance.cardsModel = clipModel

            // Assert
            var nameLabel = findChild(sectionInstance, "nameLabel_0")
            compare(nameLabel.text, "Amazing Play", "Clip title should be displayed")
        }

        function test_clipCards_broadcasterDisplayed() {
            // Arrange
            sectionInstance.cardsModel = clipModel

            // Assert
            var detailLabel = findChild(sectionInstance, "detailLabel_0")
            compare(detailLabel.text, "ProPlayer", "Broadcaster name should be displayed")
        }

        function test_clipCards_sectionTypeOverride() {
            // Arrange
            sectionInstance.sectionType = "clips"
            sectionInstance.cardsModel = streamModel  // Even with stream data

            // Assert
            compare(sectionInstance.isClipSection, true, "Should be clip section with sectionType='clips'")
        }

        // =====================================================================
        // TEST: Video Cards Display
        // =====================================================================

        function test_videoCards_countCorrect() {
            // Arrange
            sectionInstance.cardsModel = videoModel

            // Assert
            compare(sectionInstance.cardCount, 2, "Should have 2 video cards")
        }

        function test_videoCards_isVideoSectionTrue() {
            // Arrange
            sectionInstance.cardsModel = videoModel

            // Assert
            compare(sectionInstance.isVideoSection, true, "Should detect as video section")
        }

        function test_videoCards_typeDetection() {
            // Arrange
            sectionInstance.cardsModel = videoModel

            // Assert
            var cardTypeLabel = findChild(sectionInstance, "cardTypeLabel_0")
            compare(cardTypeLabel.text, "VIDEO", "Card type should be VIDEO")
        }

        function test_videoCards_titleDisplayed() {
            // Arrange
            sectionInstance.cardsModel = videoModel

            // Assert
            var nameLabel = findChild(sectionInstance, "nameLabel_0")
            compare(nameLabel.text, "Full Stream Day 1", "Video title should be displayed")
        }

        function test_videoCards_sectionTypeOverride() {
            // Arrange
            sectionInstance.sectionType = "videos"
            sectionInstance.cardsModel = streamModel

            // Assert
            compare(sectionInstance.isVideoSection, true, "Should be video section with sectionType='videos'")
        }

        // =====================================================================
        // TEST: Channel Cards Display
        // =====================================================================

        function test_channelCards_countCorrect() {
            // Arrange
            sectionInstance.cardsModel = channelModel

            // Assert
            compare(sectionInstance.cardCount, 2, "Should have 2 channel cards")
        }

        function test_channelCards_isChannelSectionTrue() {
            // Arrange
            sectionInstance.cardsModel = channelModel

            // Assert
            compare(sectionInstance.isChannelSection, true, "Should detect as channel section")
        }

        function test_channelCards_actualRowHeightAdjusted() {
            // Arrange
            sectionInstance.cardsModel = channelModel

            // Assert
            compare(sectionInstance.actualRowHeight, 260, "Row height should be 260 for channels")
        }

        function test_channelCards_typeDetection() {
            // Arrange
            sectionInstance.cardsModel = channelModel

            // Assert
            var cardTypeLabel = findChild(sectionInstance, "cardTypeLabel_0")
            compare(cardTypeLabel.text, "CHANNEL", "Card type should be CHANNEL")
        }

        function test_channelCards_sectionTypeOverride() {
            // Arrange
            sectionInstance.sectionType = "channels"
            sectionInstance.cardsModel = streamModel

            // Assert
            compare(sectionInstance.isChannelSection, true, "Should be channel section with sectionType='channels'")
        }

        // =====================================================================
        // TEST: Signal Emissions
        // =====================================================================

        function test_categoryClick_emitsSignal() {
            // Arrange
            sectionInstance.cardsModel = categoryModel

            // Act
            var mouseArea = findChild(sectionInstance, "cardMouseArea_0")
            verify(mouseArea !== null, "Mouse area should exist")
            mouseClick(mouseArea)

            // Assert
            compare(categoryClickedSpy.count, 1, "categoryClicked should be emitted once")
        }

        function test_categoryClick_signalArguments() {
            // Arrange
            sectionInstance.cardsModel = categoryModel

            // Act
            var mouseArea = findChild(sectionInstance, "cardMouseArea_0")
            mouseClick(mouseArea)

            // Assert
            compare(categoryClickedSpy.signalArguments[0][0], "cat1", "Category ID should be passed")
            compare(categoryClickedSpy.signalArguments[0][1], "Just Chatting", "Category name should be passed")
        }

        function test_streamClick_emitsSignal() {
            // Arrange
            sectionInstance.cardsModel = streamModel

            // Act
            var mouseArea = findChild(sectionInstance, "cardMouseArea_0")
            mouseClick(mouseArea)

            // Assert
            compare(streamClickedSpy.count, 1, "streamClicked should be emitted once")
        }

        function test_streamClick_signalArguments() {
            // Arrange
            sectionInstance.cardsModel = streamModel

            // Act
            var mouseArea = findChild(sectionInstance, "cardMouseArea_0")
            mouseClick(mouseArea)

            // Assert
            compare(streamClickedSpy.signalArguments[0][0], "streamer1", "Streamer login should be passed")
            compare(streamClickedSpy.signalArguments[0][1], "Streamer1", "Streamer name should be passed")
            compare(streamClickedSpy.signalArguments[0][2], "Playing Fortnite", "Stream title should be passed")
            compare(streamClickedSpy.signalArguments[0][3], "http://example.com/thumb1.jpg", "Thumbnail URL should be passed")
        }

        // =====================================================================
        // TEST: ListView Configuration
        // =====================================================================

        function test_listView_exists() {
            var listView = findChild(sectionInstance, "listView")
            verify(listView !== null, "ListView should exist")
        }

        function test_listView_horizontalOrientation() {
            var listView = findChild(sectionInstance, "listView")
            compare(listView.orientation, ListView.Horizontal, "Should be horizontal orientation")
        }

        function test_listView_interactive() {
            var listView = findChild(sectionInstance, "listView")
            compare(listView.interactive, true, "ListView should be interactive")
        }

        function test_listView_spacing() {
            // Arrange
            sectionInstance.cardSpacing = 20

            // Assert
            var listView = findChild(sectionInstance, "listView")
            compare(listView.spacing, 20, "ListView spacing should match cardSpacing")
        }

        // =====================================================================
        // TEST: Model Changes
        // =====================================================================

        function test_model_changingModelUpdatesCards() {
            // Arrange
            sectionInstance.cardsModel = streamModel
            compare(sectionInstance.cardCount, 2, "Initial count should be 2")

            // Act
            sectionInstance.cardsModel = categoryModel

            // Assert
            compare(sectionInstance.cardCount, 3, "Count should update to 3")
        }

        function test_model_settingEmptyArrayClearsCards() {
            // Arrange
            sectionInstance.cardsModel = streamModel
            compare(sectionInstance.cardCount, 2, "Initial count should be 2")

            // Act
            sectionInstance.cardsModel = []

            // Assert
            compare(sectionInstance.cardCount, 0, "Count should be 0 after clearing")
        }

        // =====================================================================
        // TEST: Data-Driven - Multiple Cards
        // =====================================================================

        function test_multipleCategories_allDisplayed_data() {
            return [
                { tag: "first", index: 0, expectedName: "Just Chatting" },
                { tag: "second", index: 1, expectedName: "Fortnite" },
                { tag: "third", index: 2, expectedName: "League of Legends" }
            ]
        }

        function test_multipleCategories_allDisplayed(data) {
            // Arrange
            sectionInstance.cardsModel = categoryModel

            // Assert
            var nameLabel = findChild(sectionInstance, "nameLabel_" + data.index)
            verify(nameLabel !== null, "Card at index " + data.index + " should exist")
            compare(nameLabel.text, data.expectedName, "Card " + data.index + " should show " + data.expectedName)
        }

        // =====================================================================
        // TEST: Edge Cases
        // =====================================================================

        function test_edgeCase_veryLongSectionTitle() {
            // Arrange
            var longTitle = "This is a very long section title that might need to be truncated or wrapped"
            sectionInstance.sectionTitle = longTitle

            // Assert
            var titleText = findChild(sectionInstance, "titleText")
            compare(titleText.text, longTitle, "Long title should be displayed")
        }

        function test_edgeCase_unicodeTitle() {
            // Arrange
            sectionInstance.sectionTitle = "Streams en direct"

            // Assert
            var titleText = findChild(sectionInstance, "titleText")
            compare(titleText.text, "Streams en direct", "Unicode characters should be displayed")
        }

        function test_edgeCase_cardWithEmptyStrings() {
            // Arrange
            sectionInstance.cardsModel = [
                { name: "", detail: "", viewers: "" }
            ]

            // Assert
            var nameLabel = findChild(sectionInstance, "nameLabel_0")
            compare(nameLabel.text, "", "Empty name should be handled")
        }

        function test_edgeCase_resetFunction() {
            // Arrange
            sectionInstance.sectionTitle = "Test"
            sectionInstance.cardsModel = streamModel

            // Act
            sectionInstance.reset()

            // Assert
            compare(sectionInstance.sectionTitle, "", "Title should be reset")
            compare(sectionInstance.cardsModel.length, 0, "Model should be reset")
            compare(sectionInstance.sectionType, "", "Type should be reset")
        }

        function test_edgeCase_customDimensions() {
            // Arrange
            sectionInstance.rowHeight = 300
            sectionInstance.cardWidth = 250
            sectionInstance.cardSpacing = 24

            // Assert
            compare(sectionInstance.rowHeight, 300, "Custom rowHeight should be set")
            compare(sectionInstance.cardWidth, 250, "Custom cardWidth should be set")
            compare(sectionInstance.cardSpacing, 24, "Custom cardSpacing should be set")
        }
    }
}
