/**
 * tst_LocalPlaybackCard.qml
 *
 * Functional UI tests for the LocalPlaybackCard component.
 * Tests local video playback card with file selection,
 * play/stop controls, and status display.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtTest 1.15

Item {
    id: root
    width: 500
    height: 400

    // =========================================================================
    // Mock Theme Constants
    // =========================================================================

    QtObject {
        id: mockTheme
        readonly property color surface: "#141c2a"
        readonly property color surfaceSoft: "#1d2533"
        readonly property color divider: "#222b37"
        readonly property color primaryText: "#f6f7fa"
        readonly property color secondaryText: "#aeb9c9"
        readonly property color accent: "#5bc0ff"
        readonly property color overlayTint: "#0c111b"
        readonly property color buttonSurface: "#1c2232"
        readonly property color buttonBorder: "#2b3450"
        readonly property color mutedText: "#7d89a4"
        readonly property string fontFamily: "Inter"
        readonly property int spacingSmall: 8
        readonly property int spacingMedium: 16
        readonly property int heroCornerRadius: 30
    }

    // =========================================================================
    // Mock FFmpeg Service
    // =========================================================================

    QtObject {
        id: mockFfmpegService
        objectName: "mockFfmpegService"

        property var videoSink: null
        property bool isPlaying: false
        property string lastPlayedFile: ""
        property bool stopCalled: false

        signal playingChanged(bool playing)
        signal errorOccurred(string message)

        function playFile(filePath) {
            lastPlayedFile = filePath
            isPlaying = true
            playingChanged(true)
        }

        function stop() {
            stopCalled = true
            isPlaying = false
            playingChanged(false)
        }

        function reset() {
            videoSink = null
            isPlaying = false
            lastPlayedFile = ""
            stopCalled = false
        }

        function simulateError(message) {
            errorOccurred(message)
        }
    }

    // =========================================================================
    // Component Under Test (Mock of LocalPlaybackCard)
    // =========================================================================

    Item {
        id: localPlaybackCard
        objectName: "localPlaybackCard"
        anchors.centerIn: parent
        width: 400
        height: 320

        // LocalPlaybackCard properties
        property var ffmpegService: null
        property string selectedPath: ""
        property string statusText: qsTr("Select a video to start.")

        // Signal
        signal statusChanged(string message)

        // Function to update status
        function updateStatus(message) {
            statusText = message
            statusChanged(message)
        }

        // Reset function for tests
        function reset() {
            ffmpegService = null
            selectedPath = ""
            statusText = qsTr("Select a video to start.")
        }

        // Card background (from BlueCard)
        Rectangle {
            id: cardBackground
            objectName: "cardBackground"
            anchors.fill: parent
            color: "#1b2130"
            radius: 8
            border.color: "#2a324e"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: mockTheme.spacingMedium

                // Video preview zone
                Rectangle {
                    id: videoPreviewZone
                    objectName: "videoPreviewZone"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                    radius: mockTheme.heroCornerRadius
                    color: mockTheme.surface
                    border.color: mockTheme.divider
                    border.width: 1
                }

                // Path row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: mockTheme.spacingSmall

                    // TextField mock as Rectangle + Text
                    Rectangle {
                        id: pathField
                        objectName: "pathField"
                        Layout.fillWidth: true
                        height: 32
                        radius: 6
                        color: mockTheme.surfaceSoft
                        border.color: mockTheme.divider
                        border.width: 1

                        property string text: localPlaybackCard.selectedPath
                        property string placeholderText: "file:///Users/.../video.mp4"
                        property bool readOnly: true
                        property int fontPixelSize: 12

                        Text {
                            anchors.fill: parent
                            anchors.margins: 8
                            text: pathField.text || pathField.placeholderText
                            color: pathField.text ? mockTheme.primaryText : mockTheme.mutedText
                            font.pixelSize: pathField.fontPixelSize
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideMiddle
                        }
                    }

                    // Browse button mock
                    Rectangle {
                        id: browseButton
                        objectName: "browseButton"
                        width: 80
                        height: 32
                        radius: 12
                        color: mockTheme.buttonSurface
                        border.color: mockTheme.buttonBorder
                        border.width: 1

                        property string text: qsTr("Browse")
                        signal clicked()

                        Text {
                            anchors.centerIn: parent
                            text: browseButton.text
                            color: mockTheme.primaryText
                            font.pixelSize: 12
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: browseButton.clicked()
                        }
                    }
                }

                // Controls row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: mockTheme.spacingMedium

                    // Play local button mock
                    Rectangle {
                        id: playLocalButton
                        objectName: "playLocalButton"
                        Layout.preferredWidth: 160
                        height: 32
                        radius: 12
                        color: playLocalButton.enabled ? mockTheme.accent : mockTheme.overlayTint
                        border.color: mockTheme.buttonBorder
                        border.width: 1

                        property string text: qsTr("Local Playback")
                        property bool enabled: localPlaybackCard.selectedPath.length > 0
                        signal clicked()

                        Text {
                            anchors.centerIn: parent
                            text: playLocalButton.text
                            color: mockTheme.primaryText
                            font.pixelSize: 12
                            opacity: playLocalButton.enabled ? 1.0 : 0.5
                        }

                        MouseArea {
                            id: playLocalMouseArea
                            objectName: "playLocalMouseArea"
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            enabled: playLocalButton.enabled
                            onClicked: {
                                playLocalButton.clicked()
                                if (localPlaybackCard.ffmpegService) {
                                    localPlaybackCard.ffmpegService.playFile(localPlaybackCard.selectedPath)
                                }
                            }
                        }
                    }

                    // Stop button mock
                    Rectangle {
                        id: stopButton
                        objectName: "stopButton"
                        Layout.preferredWidth: 120
                        height: 32
                        radius: 12
                        color: mockTheme.surface
                        border.color: mockTheme.buttonBorder
                        border.width: 1

                        property string text: qsTr("Stop")
                        signal clicked()

                        Text {
                            anchors.centerIn: parent
                            text: stopButton.text
                            color: mockTheme.primaryText
                            font.pixelSize: 12
                        }

                        MouseArea {
                            id: stopMouseArea
                            objectName: "stopMouseArea"
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                stopButton.clicked()
                                if (localPlaybackCard.ffmpegService) {
                                    localPlaybackCard.ffmpegService.stop()
                                }
                            }
                        }
                    }

                    // Status label
                    Text {
                        id: statusLabel
                        objectName: "statusLabel"
                        text: localPlaybackCard.statusText
                        color: mockTheme.secondaryText
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignLeft
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // Connections to ffmpegService
        QtObject {
            id: placeholderService
        }

        Connections {
            target: localPlaybackCard.ffmpegService !== null ? localPlaybackCard.ffmpegService : placeholderService
            ignoreUnknownSignals: true
            function onPlayingChanged(playing) {
                localPlaybackCard.updateStatus(playing ? qsTr("Playback in progress...") : qsTr("Playback stopped."))
            }
            function onErrorOccurred(message) {
                localPlaybackCard.updateStatus(message)
            }
        }
    }

    // =========================================================================
    // Signal Spies
    // =========================================================================

    SignalSpy { id: statusChangedSpy; target: localPlaybackCard; signalName: "statusChanged" }
    SignalSpy { id: playLocalClickedSpy; signalName: "clicked" }
    SignalSpy { id: stopClickedSpy; signalName: "clicked" }

    // =========================================================================
    // Test Case
    // =========================================================================

    TestCase {
        id: testCase
        name: "LocalPlaybackCardTests"
        when: windowShown

        function init() {
            localPlaybackCard.reset()
            mockFfmpegService.reset()
            statusChangedSpy.clear()
            // Configure button spies with targets found via findChild
            playLocalClickedSpy.target = findChild(localPlaybackCard, "playLocalButton")
            stopClickedSpy.target = findChild(localPlaybackCard, "stopButton")
            playLocalClickedSpy.clear()
            stopClickedSpy.clear()
            wait(50)
        }

        // =====================================================================
        // TEST: Default Properties
        // =====================================================================

        function test_defaultState_selectedPathEmpty() {
            compare(localPlaybackCard.selectedPath, "", "Default selectedPath should be empty")
        }

        function test_defaultState_statusText() {
            compare(localPlaybackCard.statusText, "Select a video to start.", "Default statusText should be set")
        }

        function test_defaultState_ffmpegServiceNull() {
            compare(localPlaybackCard.ffmpegService, null, "Default ffmpegService should be null")
        }

        // =====================================================================
        // TEST: Property Binding to UI Elements
        // =====================================================================

        function test_selectedPath_bindsToTextField() {
            // Arrange
            localPlaybackCard.selectedPath = "file:///path/to/video.mp4"
            wait(50)

            // Assert
            var pathField = findChild(localPlaybackCard, "pathField")
            verify(pathField !== null, "Path field should exist")
            compare(pathField.text, "file:///path/to/video.mp4", "Path field should display selectedPath")
        }

        function test_statusText_bindsToLabel() {
            // Arrange
            localPlaybackCard.statusText = "Custom status message"
            wait(50)

            // Assert
            var statusLabel = findChild(localPlaybackCard, "statusLabel")
            verify(statusLabel !== null, "Status label should exist")
            compare(statusLabel.text, "Custom status message", "Status label should display statusText")
        }

        function test_pathField_isReadOnly() {
            // Assert
            var pathField = findChild(localPlaybackCard, "pathField")
            verify(pathField !== null, "Path field should exist")
            compare(pathField.readOnly, true, "Path field should be read-only")
        }

        // =====================================================================
        // TEST: updateStatus Function and Signal
        // =====================================================================

        function test_updateStatus_changesStatusText() {
            // Arrange
            var newStatus = "Ready to play video.mp4"

            // Act
            localPlaybackCard.updateStatus(newStatus)
            wait(50)

            // Assert
            compare(localPlaybackCard.statusText, newStatus, "statusText should be updated")
        }

        function test_updateStatus_emitsStatusChangedSignal() {
            // Arrange
            var newStatus = "Loading video..."

            // Act
            localPlaybackCard.updateStatus(newStatus)

            // Assert
            compare(statusChangedSpy.count, 1, "statusChanged should be emitted once")
            compare(statusChangedSpy.signalArguments[0][0], newStatus, "Signal should contain the message")
        }

        function test_updateStatus_multipleCallsEmitMultipleSignals() {
            // Act
            localPlaybackCard.updateStatus("Status 1")
            localPlaybackCard.updateStatus("Status 2")
            localPlaybackCard.updateStatus("Status 3")

            // Assert
            compare(statusChangedSpy.count, 3, "statusChanged should be emitted 3 times")
        }

        // =====================================================================
        // TEST: Play Local Button State
        // =====================================================================

        function test_playLocalButton_disabledWhenNoPath() {
            // Arrange
            localPlaybackCard.selectedPath = ""
            wait(50)

            // Assert
            var playButton = findChild(localPlaybackCard, "playLocalButton")
            verify(playButton !== null, "Play local button should exist")
            compare(playButton.enabled, false, "Play button should be disabled when path is empty")
        }

        function test_playLocalButton_enabledWhenPathSet() {
            // Arrange
            localPlaybackCard.selectedPath = "file:///video.mp4"
            wait(50)

            // Assert
            var playButton = findChild(localPlaybackCard, "playLocalButton")
            compare(playButton.enabled, true, "Play button should be enabled when path is set")
        }

        function test_playLocalButton_colorWhenDisabled() {
            // Arrange
            localPlaybackCard.selectedPath = ""
            wait(50)

            // Assert
            var playButton = findChild(localPlaybackCard, "playLocalButton")
            verify(playButton !== null, "Play button should exist")
            compare(playButton.color.toString(), mockTheme.overlayTint.toString(), "Disabled button should use overlayTint color")
        }

        function test_playLocalButton_colorWhenEnabled() {
            // Arrange
            localPlaybackCard.selectedPath = "file:///video.mp4"
            wait(50)

            // Assert
            var playButton = findChild(localPlaybackCard, "playLocalButton")
            compare(playButton.color.toString(), mockTheme.accent.toString(), "Enabled button should use accent color")
        }

        // =====================================================================
        // TEST: FFmpeg Service Integration
        // =====================================================================

        function test_playLocalButton_click_callsPlayFile() {
            // Arrange
            localPlaybackCard.ffmpegService = mockFfmpegService
            localPlaybackCard.selectedPath = "file:///path/to/movie.mkv"
            wait(50)

            // Act
            var playMouseArea = findChild(localPlaybackCard, "playLocalMouseArea")
            mouseClick(playMouseArea)
            wait(50)

            // Assert
            compare(mockFfmpegService.lastPlayedFile, "file:///path/to/movie.mkv", "playFile should be called with selectedPath")
        }

        function test_stopButton_click_callsStop() {
            // Arrange
            localPlaybackCard.ffmpegService = mockFfmpegService
            wait(50)

            // Act
            var stopMouseArea = findChild(localPlaybackCard, "stopMouseArea")
            mouseClick(stopMouseArea)
            wait(50)

            // Assert
            compare(mockFfmpegService.stopCalled, true, "stop() should be called on ffmpegService")
        }

        function test_playLocalButton_click_withoutService_noError() {
            // Arrange - no ffmpegService set
            localPlaybackCard.ffmpegService = null
            localPlaybackCard.selectedPath = "file:///video.mp4"
            wait(50)

            // Act - should not throw
            var playMouseArea = findChild(localPlaybackCard, "playLocalMouseArea")
            mouseClick(playMouseArea)
            wait(50)

            // Assert - no crash, test passes
            verify(true, "Clicking play without service should not crash")
        }

        function test_stopButton_click_withoutService_noError() {
            // Arrange - no ffmpegService set
            localPlaybackCard.ffmpegService = null
            wait(50)

            // Act - should not throw
            var stopMouseArea = findChild(localPlaybackCard, "stopMouseArea")
            mouseClick(stopMouseArea)
            wait(50)

            // Assert - no crash
            verify(true, "Clicking stop without service should not crash")
        }

        // =====================================================================
        // TEST: FFmpeg Service Signal Handling
        // =====================================================================

        function test_ffmpegService_playingChanged_updatesStatus() {
            // Arrange
            localPlaybackCard.ffmpegService = mockFfmpegService
            wait(50)
            statusChangedSpy.clear()

            // Act - simulate playing
            mockFfmpegService.playingChanged(true)
            wait(50)

            // Assert
            compare(localPlaybackCard.statusText, "Playback in progress...", "Status should update when playing starts")
            compare(statusChangedSpy.count, 1, "statusChanged should be emitted")
        }

        function test_ffmpegService_playingStopped_updatesStatus() {
            // Arrange
            localPlaybackCard.ffmpegService = mockFfmpegService
            wait(50)
            statusChangedSpy.clear()

            // Act - simulate stop
            mockFfmpegService.playingChanged(false)
            wait(50)

            // Assert
            compare(localPlaybackCard.statusText, "Playback stopped.", "Status should update when playing stops")
        }

        function test_ffmpegService_errorOccurred_updatesStatus() {
            // Arrange
            localPlaybackCard.ffmpegService = mockFfmpegService
            wait(50)
            statusChangedSpy.clear()

            // Act
            mockFfmpegService.simulateError("Failed to open video file")
            wait(50)

            // Assert
            compare(localPlaybackCard.statusText, "Failed to open video file", "Status should show error message")
            compare(statusChangedSpy.count, 1, "statusChanged should be emitted for error")
        }

        // =====================================================================
        // TEST: UI Elements Existence
        // =====================================================================

        function test_browseButton_exists() {
            var browseButton = findChild(localPlaybackCard, "browseButton")
            verify(browseButton !== null, "Browse button should exist")
            compare(browseButton.text, "Browse", "Browse button should have correct text")
        }

        function test_playLocalButton_exists() {
            var playButton = findChild(localPlaybackCard, "playLocalButton")
            verify(playButton !== null, "Play local button should exist")
            compare(playButton.text, "Local Playback", "Play button should have correct text")
        }

        function test_stopButton_exists() {
            var stopButton = findChild(localPlaybackCard, "stopButton")
            verify(stopButton !== null, "Stop button should exist")
            compare(stopButton.text, "Stop", "Stop button should have correct text")
        }

        function test_videoPreviewZone_exists() {
            var previewZone = findChild(localPlaybackCard, "videoPreviewZone")
            verify(previewZone !== null, "Video preview zone should exist")
        }

        // =====================================================================
        // TEST: Button Dimensions
        // =====================================================================

        function test_playLocalButton_width() {
            var playButton = findChild(localPlaybackCard, "playLocalButton")
            compare(playButton.Layout.preferredWidth, 160, "Play button preferred width should be 160")
        }

        function test_stopButton_width() {
            var stopButton = findChild(localPlaybackCard, "stopButton")
            compare(stopButton.Layout.preferredWidth, 120, "Stop button preferred width should be 120")
        }

        // =====================================================================
        // TEST: Button Styles
        // =====================================================================

        function test_playLocalButton_hasCorrectRadius() {
            var playButton = findChild(localPlaybackCard, "playLocalButton")
            verify(playButton !== null)
            compare(playButton.radius, 12, "Play button radius should be 12")
        }

        function test_stopButton_hasCorrectRadius() {
            var stopButton = findChild(localPlaybackCard, "stopButton")
            verify(stopButton !== null)
            compare(stopButton.radius, 12, "Stop button radius should be 12")
        }

        function test_stopButton_color() {
            var stopButton = findChild(localPlaybackCard, "stopButton")
            compare(stopButton.color.toString(), mockTheme.surface.toString(), "Stop button should use surface color")
        }

        function test_browseButton_hasCorrectRadius() {
            var browseButton = findChild(localPlaybackCard, "browseButton")
            verify(browseButton !== null)
            compare(browseButton.radius, 12, "Browse button radius should be 12")
        }

        // =====================================================================
        // TEST: Card Background
        // =====================================================================

        function test_cardBackground_exists() {
            var cardBackground = findChild(localPlaybackCard, "cardBackground")
            verify(cardBackground !== null, "Card background should exist")
        }

        function test_cardBackground_color() {
            var cardBackground = findChild(localPlaybackCard, "cardBackground")
            compare(cardBackground.color.toString(), "#1b2130", "Card background color should match BlueCard")
        }

        function test_cardBackground_radius() {
            var cardBackground = findChild(localPlaybackCard, "cardBackground")
            compare(cardBackground.radius, 8, "Card background radius should be 8")
        }

        function test_cardBackground_border() {
            var cardBackground = findChild(localPlaybackCard, "cardBackground")
            compare(cardBackground.border.width, 1, "Card border width should be 1")
            compare(cardBackground.border.color.toString(), "#2a324e", "Card border color should match BlueCard")
        }

        // =====================================================================
        // TEST: Video Preview Zone
        // =====================================================================

        function test_videoPreviewZone_height() {
            var previewZone = findChild(localPlaybackCard, "videoPreviewZone")
            compare(previewZone.Layout.preferredHeight, 150, "Preview zone height should be 150")
        }

        function test_videoPreviewZone_radius() {
            var previewZone = findChild(localPlaybackCard, "videoPreviewZone")
            compare(previewZone.radius, mockTheme.heroCornerRadius, "Preview zone radius should match heroCornerRadius")
        }

        function test_videoPreviewZone_border() {
            var previewZone = findChild(localPlaybackCard, "videoPreviewZone")
            compare(previewZone.border.width, 1, "Preview zone border width should be 1")
            compare(previewZone.border.color.toString(), mockTheme.divider.toString(), "Preview zone border color should be divider")
        }

        // =====================================================================
        // TEST: Status Label Styling
        // =====================================================================

        function test_statusLabel_fontSize() {
            var statusLabel = findChild(localPlaybackCard, "statusLabel")
            compare(statusLabel.font.pixelSize, 12, "Status label font size should be 12")
        }

        function test_statusLabel_color() {
            var statusLabel = findChild(localPlaybackCard, "statusLabel")
            compare(statusLabel.color.toString(), mockTheme.secondaryText.toString(), "Status label should use secondaryText color")
        }

        function test_statusLabel_alignment() {
            var statusLabel = findChild(localPlaybackCard, "statusLabel")
            compare(statusLabel.horizontalAlignment, Text.AlignLeft, "Status label should align left")
        }

        // =====================================================================
        // TEST: Path Field Styling
        // =====================================================================

        function test_pathField_fontSize() {
            var pathField = findChild(localPlaybackCard, "pathField")
            compare(pathField.fontPixelSize, 12, "Path field font size should be 12")
        }

        function test_pathField_placeholderText() {
            var pathField = findChild(localPlaybackCard, "pathField")
            compare(pathField.placeholderText, "file:///Users/.../video.mp4", "Path field should have correct placeholder")
        }

        // =====================================================================
        // TEST: Button Click Signals
        // =====================================================================

        function test_playLocalButton_emitsClickedSignal() {
            // Arrange
            localPlaybackCard.selectedPath = "file:///video.mp4"
            wait(50)
            playLocalClickedSpy.clear()

            // Act
            var playMouseArea = findChild(localPlaybackCard, "playLocalMouseArea")
            mouseClick(playMouseArea)
            wait(50)

            // Assert
            compare(playLocalClickedSpy.count, 1, "Play button clicked signal should be emitted")
        }

        function test_stopButton_emitsClickedSignal() {
            // Arrange
            stopClickedSpy.clear()

            // Act
            var stopMouseArea = findChild(localPlaybackCard, "stopMouseArea")
            mouseClick(stopMouseArea)
            wait(50)

            // Assert
            compare(stopClickedSpy.count, 1, "Stop button clicked signal should be emitted")
        }

        function test_playLocalButton_noClickWhenDisabled() {
            // Arrange
            localPlaybackCard.selectedPath = ""  // Disabled
            wait(50)
            playLocalClickedSpy.clear()

            // Act
            var playMouseArea = findChild(localPlaybackCard, "playLocalMouseArea")
            mouseClick(playMouseArea)
            wait(50)

            // Assert
            compare(playLocalClickedSpy.count, 0, "Disabled play button should not emit clicked")
        }
    }
}
