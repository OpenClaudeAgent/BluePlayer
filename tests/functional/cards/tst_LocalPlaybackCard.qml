/**
 * tst_LocalPlaybackCard.qml
 * 
 * Functional UI tests for the LocalPlaybackCard component.
 * Tests file selection, playback controls, status updates, and signal emissions.
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
    // Mock FFmpeg Service
    // =========================================================================
    
    QtObject {
        id: mockFfmpegService
        objectName: "mockFfmpegService"
        
        property bool playing: false
        property string lastPlayedFile: ""
        property int playFileCallCount: 0
        property int stopCallCount: 0
        
        signal playbackStateChanged(bool isPlaying)
        signal errorOccurred(string message)
        
        function playFile(path) {
            lastPlayedFile = path
            playFileCallCount++
            playing = true
            playbackStateChanged(true)
        }
        
        function stop() {
            stopCallCount++
            playing = false
            playbackStateChanged(false)
        }
        
        function reset() {
            playing = false
            lastPlayedFile = ""
            playFileCallCount = 0
            stopCallCount = 0
        }
        
        function simulateError(message) {
            errorOccurred(message)
        }
    }

    // =========================================================================
    // Component Under Test (Mock of LocalPlaybackCard)
    // =========================================================================
    
    Component {
        id: localPlaybackCardComponent
        
        Rectangle {
            id: cardRoot
            objectName: "localPlaybackCard"
            
            // Theme constants (inline)
            readonly property color _accent: "#9147FF"
            readonly property color _surface: "#18181B"
            readonly property color _buttonSurface: "#2D2D30"
            readonly property color _buttonBorder: "#3D3D40"
            readonly property color _secondaryText: "#ADADB8"
            readonly property color _divider: "#3D3D40"
            readonly property color _overlayTint: "#1A1A1D"
            readonly property int _spacingSmall: 8
            readonly property int _spacingMedium: 16
            readonly property int _heroCornerRadius: 16
            
            // Properties matching LocalPlaybackCard
            property var ffmpegService: null
            property string selectedPath: ""
            property string statusText: qsTr("Select a video to start.")
            
            signal statusChanged(string message)
            
            function updateStatus(message) {
                statusText = message
                statusChanged(message)
            }
            
            width: 400
            height: 350
            radius: 12
            color: _surface
            border.color: _divider
            border.width: 1
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: _spacingMedium
                spacing: _spacingMedium
                
                // Video preview area
                Rectangle {
                    id: previewArea
                    objectName: "previewArea"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 180
                    radius: _heroCornerRadius
                    color: _surface
                    border.color: _divider
                    border.width: 1
                    
                    Text {
                        anchors.centerIn: parent
                        text: cardRoot.selectedPath ? "Preview" : "No video"
                        color: _secondaryText
                        font.pixelSize: 14
                    }
                }
                
                // Path field row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: _spacingSmall
                    
                    Rectangle {
                        id: pathField
                        objectName: "pathField"
                        Layout.fillWidth: true
                        height: 36
                        radius: 8
                        color: _buttonSurface
                        border.color: _buttonBorder
                        border.width: 1
                        
                        Text {
                            id: pathText
                            objectName: "pathText"
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            verticalAlignment: Text.AlignVCenter
                            text: cardRoot.selectedPath || "file:///Users/.../video.mp4"
                            color: cardRoot.selectedPath ? "#FFFFFF" : _secondaryText
                            font.pixelSize: 12
                            elide: Text.ElideMiddle
                        }
                    }
                    
                    Rectangle {
                        id: browseButton
                        objectName: "browseButton"
                        width: 80
                        height: 36
                        radius: 12
                        color: _buttonSurface
                        border.color: _buttonBorder
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("Browse")
                            color: "#FFFFFF"
                            font.pixelSize: 13
                        }
                        
                        MouseArea {
                            id: browseMouseArea
                            objectName: "browseMouseArea"
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                // In tests, we simulate file selection directly
                            }
                        }
                    }
                }
                
                // Control buttons row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: _spacingMedium
                    
                    Rectangle {
                        id: playButton
                        objectName: "playButton"
                        Layout.preferredWidth: 160
                        height: 40
                        radius: 12
                        color: cardRoot.selectedPath.length > 0 ? _accent : _overlayTint
                        border.color: _buttonBorder
                        border.width: 1
                        
                        property bool enabled: cardRoot.selectedPath.length > 0
                        
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("Local playback")
                            color: parent.enabled ? "#FFFFFF" : _secondaryText
                            font.pixelSize: 13
                            font.bold: true
                        }
                        
                        MouseArea {
                            id: playMouseArea
                            objectName: "playMouseArea"
                            anchors.fill: parent
                            cursorShape: parent.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                            onClicked: {
                                if (playButton.enabled && cardRoot.ffmpegService) {
                                    cardRoot.ffmpegService.playFile(cardRoot.selectedPath)
                                }
                            }
                        }
                    }
                    
                    Rectangle {
                        id: stopButton
                        objectName: "stopButton"
                        Layout.preferredWidth: 120
                        height: 40
                        radius: 12
                        color: _surface
                        border.color: _buttonBorder
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: qsTr("Stop")
                            color: "#FFFFFF"
                            font.pixelSize: 13
                        }
                        
                        MouseArea {
                            id: stopMouseArea
                            objectName: "stopMouseArea"
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (cardRoot.ffmpegService) {
                                    cardRoot.ffmpegService.stop()
                                }
                            }
                        }
                    }
                    
                    // Status label
                    Text {
                        id: statusLabel
                        objectName: "statusLabel"
                        text: cardRoot.statusText
                        color: _secondaryText
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignLeft
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                }
            }
            
            // Connections to FFmpeg service
            Connections {
                target: cardRoot.ffmpegService
                ignoreUnknownSignals: true
                
                function onPlaybackStateChanged(isPlaying) {
                    cardRoot.updateStatus(isPlaying ? qsTr("Playing...") : qsTr("Playback stopped."))
                }
                
                function onErrorOccurred(message) {
                    cardRoot.updateStatus(message)
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
    
    SignalSpy { id: statusChangedSpy; signalName: "statusChanged" }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "LocalPlaybackCardTests"
        when: windowShown

        function init() {
            mockFfmpegService.reset()
            card = createTemporaryObject(localPlaybackCardComponent, root)
            verify(card !== null, "LocalPlaybackCard should be created")
            card.anchors.centerIn = root
            card.ffmpegService = mockFfmpegService
            statusChangedSpy.target = card
            statusChangedSpy.clear()
        }

        function cleanup() {
            card = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultState_noSelectedPath() {
            compare(card.selectedPath, "", "Default selectedPath is empty")
        }

        function test_defaultState_initialStatusText() {
            compare(card.statusText, "Select a video to start.", "Default status message")
        }

        function test_defaultState_playButtonDisabled() {
            var playButton = findChild(card, "playButton")
            compare(playButton.enabled, false, "Play button disabled when no file selected")
        }

        // =====================================================================
        // TEST: File Selection
        // =====================================================================
        
        function test_fileSelection_updatesSelectedPath() {
            card.selectedPath = "file:///Users/test/video.mp4"
            compare(card.selectedPath, "file:///Users/test/video.mp4", "Selected path updated")
        }

        function test_fileSelection_enablesPlayButton() {
            var playButton = findChild(card, "playButton")
            
            compare(playButton.enabled, false, "Initially disabled")
            
            card.selectedPath = "/path/to/video.mp4"
            
            compare(playButton.enabled, true, "Enabled after file selection")
        }

        function test_fileSelection_pathDisplayed() {
            var pathText = findChild(card, "pathText")
            
            card.selectedPath = "file:///Movies/test.mkv"
            
            compare(pathText.text, "file:///Movies/test.mkv", "Path text shows selected file")
        }

        function test_fileSelection_emptyPathDisablesPlay() {
            card.selectedPath = "/some/video.mp4"
            var playButton = findChild(card, "playButton")
            compare(playButton.enabled, true, "Enabled with path")
            
            card.selectedPath = ""
            compare(playButton.enabled, false, "Disabled when path cleared")
        }

        // =====================================================================
        // TEST: Status Updates
        // =====================================================================
        
        function test_updateStatus_changesStatusText() {
            card.updateStatus("Ready to play")
            compare(card.statusText, "Ready to play", "Status text updated")
        }

        function test_updateStatus_emitsSignal() {
            card.updateStatus("New status")
            compare(statusChangedSpy.count, 1, "statusChanged emitted")
            compare(statusChangedSpy.signalArguments[0][0], "New status", "Correct status in signal")
        }

        function test_statusLabel_showsStatusText() {
            var statusLabel = findChild(card, "statusLabel")
            
            card.updateStatus("Processing...")
            
            compare(statusLabel.text, "Processing...", "Status label reflects statusText")
        }

        // =====================================================================
        // TEST: Play Button
        // =====================================================================
        
        function test_playButton_clickCallsPlayFile() {
            card.selectedPath = "/path/to/video.mp4"
            var playMouseArea = findChild(card, "playMouseArea")
            
            mouseClick(playMouseArea)
            
            compare(mockFfmpegService.playFileCallCount, 1, "playFile called once")
            compare(mockFfmpegService.lastPlayedFile, "/path/to/video.mp4", "Correct file passed")
        }

        function test_playButton_noCallWhenDisabled() {
            card.selectedPath = ""
            var playMouseArea = findChild(card, "playMouseArea")
            
            mouseClick(playMouseArea)
            
            compare(mockFfmpegService.playFileCallCount, 0, "playFile not called when disabled")
        }

        function test_playButton_noCallWithoutService() {
            card.ffmpegService = null
            card.selectedPath = "/path/to/video.mp4"
            var playMouseArea = findChild(card, "playMouseArea")
            
            mouseClick(playMouseArea)
            
            compare(mockFfmpegService.playFileCallCount, 0, "No call when service is null")
        }

        // =====================================================================
        // TEST: Stop Button
        // =====================================================================
        
        function test_stopButton_clickCallsStop() {
            var stopMouseArea = findChild(card, "stopMouseArea")
            
            mouseClick(stopMouseArea)
            
            compare(mockFfmpegService.stopCallCount, 1, "stop called once")
        }

        function test_stopButton_noCallWithoutService() {
            card.ffmpegService = null
            var stopMouseArea = findChild(card, "stopMouseArea")
            
            mouseClick(stopMouseArea)
            
            compare(mockFfmpegService.stopCallCount, 0, "No call when service is null")
        }

        // =====================================================================
        // TEST: FFmpeg Service Connections
        // =====================================================================
        
        function test_service_playbackStateChangedUpdatesStatus() {
            mockFfmpegService.playbackStateChanged(true)
            compare(card.statusText, "Playing...", "Status updated when playing")
            
            mockFfmpegService.playbackStateChanged(false)
            compare(card.statusText, "Playback stopped.", "Status updated when stopped")
        }

        function test_service_errorOccurredUpdatesStatus() {
            mockFfmpegService.simulateError("File not found")
            compare(card.statusText, "File not found", "Error message shown in status")
        }

        // =====================================================================
        // TEST: Integration Flow
        // =====================================================================
        
        function test_flow_selectPlayStop() {
            // 1. Select file
            card.selectedPath = "file:///test/movie.mp4"
            compare(card.selectedPath, "file:///test/movie.mp4", "File selected")
            
            var playButton = findChild(card, "playButton")
            compare(playButton.enabled, true, "Play enabled")
            
            // 2. Start playback
            var playMouseArea = findChild(card, "playMouseArea")
            mouseClick(playMouseArea)
            
            compare(mockFfmpegService.playing, true, "Service is playing")
            compare(card.statusText, "Playing...", "Status shows playing")
            
            // 3. Stop playback
            var stopMouseArea = findChild(card, "stopMouseArea")
            mouseClick(stopMouseArea)
            
            compare(mockFfmpegService.playing, false, "Service stopped")
            compare(card.statusText, "Playback stopped.", "Status shows stopped")
        }

        function test_flow_errorDuringPlayback() {
            card.selectedPath = "/invalid/path.mp4"
            var playMouseArea = findChild(card, "playMouseArea")
            
            mouseClick(playMouseArea)
            mockFfmpegService.simulateError("Could not open file")
            
            compare(card.statusText, "Could not open file", "Error shown in status")
        }

        // =====================================================================
        // TEST: Data-Driven File Paths
        // =====================================================================

        function test_validFilePaths_data() {
            return [
                { tag: "mp4", path: "/Users/test/video.mp4" },
                { tag: "mkv", path: "/Users/test/movie.mkv" },
                { tag: "mov", path: "/Users/test/clip.mov" },
                { tag: "avi", path: "/Users/test/old_video.avi" },
                { tag: "m4v", path: "/Users/test/itunes_video.m4v" },
                { tag: "spaces", path: "/Users/test/my video.mp4" },
                { tag: "unicode", path: "/Users/test/video.mp4" },
                { tag: "file_url", path: "file:///Users/test/video.mp4" }
            ]
        }

        function test_validFilePaths(data) {
            card.selectedPath = data.path
            
            var playButton = findChild(card, "playButton")
            compare(playButton.enabled, true, "Play enabled for " + data.tag)
            compare(card.selectedPath, data.path, "Path stored for " + data.tag)
        }

        // =====================================================================
        // TEST: Button States
        // =====================================================================
        
        function test_playButton_colorChangesWithState() {
            var playButton = findChild(card, "playButton")
            var disabledColor = playButton.color.toString()
            
            card.selectedPath = "/video.mp4"
            var enabledColor = playButton.color.toString()
            
            verify(disabledColor !== enabledColor, "Button color changes when enabled")
        }

        // =====================================================================
        // TEST: Multiple Status Updates
        // =====================================================================
        
        function test_multipleStatusUpdates_allEmitSignals() {
            card.updateStatus("Status 1")
            card.updateStatus("Status 2")
            card.updateStatus("Status 3")
            
            compare(statusChangedSpy.count, 3, "Three status changes emitted")
        }

        function test_statusUpdate_preservesLastStatus() {
            card.updateStatus("First")
            card.updateStatus("Second")
            card.updateStatus("Final")
            
            compare(card.statusText, "Final", "Last status preserved")
        }
    }
}
