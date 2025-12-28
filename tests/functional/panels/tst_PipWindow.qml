/**
 * tst_PipWindow.qml
 * 
 * Functional UI tests for the PipWindow component logic.
 * Since Window type cannot be tested directly as Item, we test
 * the logic through a mock that reproduces the public API.
 * 
 * Run with: ./test_functional_ui PipWindowTests
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtTest 1.15

Item {
    id: root
    width: 640
    height: 400

    // Detect offscreen mode
    readonly property bool isOffscreen: Qt.platform.pluginName === "offscreen"

    // =========================================================================
    // Mock of PipWindow Component (reproduces public API without Window type)
    // =========================================================================
    
    Component {
        id: pipWindowComponent
        
        Rectangle {
            id: pipWindow
            objectName: "pipWindow"
            
            // Default size (16:9 ratio)
            width: 400
            height: 225
            
            // Size constraints
            readonly property int minimumWidth: 320
            readonly property int minimumHeight: 180
            readonly property int maximumWidth: 640
            readonly property int maximumHeight: 360
            
            // ─────────────────────────────────────────────────────────────────
            // Public Properties
            // ─────────────────────────────────────────────────────────────────
            
            property Item videoItem: null
            property bool playing: false
            property bool paused: false
            property string streamerName: ""
            
            // ─────────────────────────────────────────────────────────────────
            // Signals
            // ─────────────────────────────────────────────────────────────────
            
            signal closeRequested()
            signal returnToAppRequested()
            signal playPauseRequested()
            
            // ─────────────────────────────────────────────────────────────────
            // Appearance
            // ─────────────────────────────────────────────────────────────────
            
            radius: 16
            color: "#000000"
            border.color: "#40FFFFFF"
            border.width: 1
            clip: true
            
            // ─────────────────────────────────────────────────────────────────
            // Video Container
            // ─────────────────────────────────────────────────────────────────
            
            Item {
                id: videoContainer
                objectName: "videoContainer"
                anchors.fill: parent
                anchors.margins: 1
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Controls Visibility
            // ─────────────────────────────────────────────────────────────────
            
            property bool controlsVisible: true
            
            Timer {
                id: hideControlsTimer
                objectName: "hideControlsTimer"
                interval: 2000
                onTriggered: {
                    pipWindow.controlsVisible = false
                }
            }
            
            function showControls() {
                controlsVisible = true
                hideControlsTimer.restart()
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Close Button
            // ─────────────────────────────────────────────────────────────────
            
            Rectangle {
                id: closeButton
                objectName: "closeButton"
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 10
                width: 26
                height: 26
                radius: 13
                color: closeMouseArea.containsMouse ? "#E0000000" : "#99000000"
                opacity: pipWindow.controlsVisible ? 1.0 : 0.0
                visible: opacity > 0
                
                property bool hovered: closeMouseArea.containsMouse
                
                Text {
                    anchors.centerIn: parent
                    text: "\u2715"  // ✕
                    color: closeMouseArea.containsMouse ? "#FF6961" : "#FFFFFF"
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }
                
                MouseArea {
                    id: closeMouseArea
                    objectName: "closeMouseArea"
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: pipWindow.closeRequested()
                }
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Bottom Controls
            // ─────────────────────────────────────────────────────────────────
            
            Rectangle {
                id: bottomControls
                objectName: "bottomControls"
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 14
                width: controlsRow.width + 24
                height: 44
                radius: 22
                color: "#99000000"
                opacity: pipWindow.controlsVisible ? 1.0 : 0.0
                visible: opacity > 0
                
                Row {
                    id: controlsRow
                    objectName: "controlsRow"
                    anchors.centerIn: parent
                    spacing: 14
                    
                    // Play/Pause Button
                    Rectangle {
                        id: playPauseButton
                        objectName: "playPauseButton"
                        width: 36
                        height: 36
                        radius: 18
                        color: playPauseMouseArea.containsMouse ? "#40FFFFFF" : "transparent"
                        
                        property bool hovered: playPauseMouseArea.containsMouse
                        property bool isPlaying: pipWindow.playing && !pipWindow.paused
                        
                        Text {
                            anchors.centerIn: parent
                            text: playPauseButton.isPlaying ? "\u23F8" : "\u25B6"  // Pause or Play
                            color: "#FFFFFF"
                            font.pixelSize: 14
                        }
                        
                        MouseArea {
                            id: playPauseMouseArea
                            objectName: "playPauseMouseArea"
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: pipWindow.playPauseRequested()
                        }
                    }
                    
                    // Separator
                    Rectangle {
                        width: 1
                        height: 22
                        color: "#4DFFFFFF"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    // Return Button
                    Rectangle {
                        id: returnButton
                        objectName: "returnButton"
                        width: 36
                        height: 36
                        radius: 18
                        color: returnMouseArea.containsMouse ? "#40FFFFFF" : "transparent"
                        
                        property bool hovered: returnMouseArea.containsMouse
                        
                        Text {
                            anchors.centerIn: parent
                            text: "\u21A9"  // Return arrow
                            color: "#FFFFFF"
                            font.pixelSize: 14
                        }
                        
                        MouseArea {
                            id: returnMouseArea
                            objectName: "returnMouseArea"
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: pipWindow.returnToAppRequested()
                        }
                    }
                }
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Streamer Name Badge
            // ─────────────────────────────────────────────────────────────────
            
            Rectangle {
                id: streamerBadge
                objectName: "streamerBadge"
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.margins: 10
                width: streamerLabel.width + 16
                height: 26
                radius: 13
                color: "#80000000"
                opacity: pipWindow.controlsVisible && pipWindow.streamerName.length > 0 ? 1.0 : 0.0
                visible: opacity > 0
                
                Text {
                    id: streamerLabel
                    objectName: "streamerLabel"
                    anchors.centerIn: parent
                    text: pipWindow.streamerName
                    color: "#FFFFFF"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                }
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Video Reparenting Functions
            // ─────────────────────────────────────────────────────────────────
            
            function attachVideo(item) {
                if (item) {
                    videoItem = item
                    item.parent = videoContainer
                }
            }
            
            function detachVideo() {
                if (videoItem) {
                    var item = videoItem
                    videoItem = null
                    return item
                }
                return null
            }
            
            // ─────────────────────────────────────────────────────────────────
            // Size Constraint Helpers (for resize logic testing)
            // ─────────────────────────────────────────────────────────────────
            
            function constrainWidth(w) {
                return Math.max(minimumWidth, Math.min(maximumWidth, w))
            }
            
            function constrainHeight(h) {
                return Math.max(minimumHeight, Math.min(maximumHeight, h))
            }
            
            function resizeWithRatio(newWidth) {
                var w = constrainWidth(newWidth)
                var h = Math.round(w * 9 / 16)
                h = constrainHeight(h)
                w = Math.round(h * 16 / 9)  // Recalc to maintain ratio
                width = w
                height = h
            }
        }
    }

    // =========================================================================
    // Test Instance & Spies
    // =========================================================================
    
    property var pipWindow: null
    property var closeSpy: null
    property var returnSpy: null
    property var playPauseSpy: null
    
    Component {
        id: signalSpyComponent
        SignalSpy {}
    }
    
    // Mock video item for testing
    Component {
        id: mockVideoComponent
        Rectangle {
            objectName: "mockVideo"
            color: "blue"
            width: 320
            height: 180
        }
    }

    // =========================================================================
    // Test Case
    // =========================================================================
    
    TestCase {
        id: testCase
        name: "PipWindowTests"
        when: windowShown

        function init() {
            pipWindow = createTemporaryObject(pipWindowComponent, root)
            verify(pipWindow !== null, "PipWindow should be created")
            pipWindow.x = 20
            pipWindow.y = 20
            
            closeSpy = createTemporaryObject(signalSpyComponent, root, {
                target: pipWindow, signalName: "closeRequested"
            })
            returnSpy = createTemporaryObject(signalSpyComponent, root, {
                target: pipWindow, signalName: "returnToAppRequested"
            })
            playPauseSpy = createTemporaryObject(signalSpyComponent, root, {
                target: pipWindow, signalName: "playPauseRequested"
            })
            
            waitForRendering(pipWindow)
        }

        function cleanup() {
            pipWindow = null
        }

        // =====================================================================
        // TEST: Default State
        // =====================================================================
        
        function test_defaultSize_is16by9() {
            // Assert
            compare(pipWindow.width, 400, "Default width should be 400")
            compare(pipWindow.height, 225, "Default height should be 225")
            
            // Verify 16:9 ratio
            var ratio = pipWindow.width / pipWindow.height
            fuzzyCompare(ratio, 16/9, 0.01, "Ratio should be 16:9")
        }
        
        function test_defaultProperties() {
            // Assert
            compare(pipWindow.playing, false, "Should not be playing by default")
            compare(pipWindow.paused, false, "Should not be paused by default")
            compare(pipWindow.streamerName, "", "Streamer name should be empty")
            compare(pipWindow.videoItem, null, "Video item should be null")
            compare(pipWindow.controlsVisible, true, "Controls should be visible")
        }
        
        function test_sizeConstraints() {
            // Assert
            compare(pipWindow.minimumWidth, 320, "Minimum width should be 320")
            compare(pipWindow.minimumHeight, 180, "Minimum height should be 180")
            compare(pipWindow.maximumWidth, 640, "Maximum width should be 640")
            compare(pipWindow.maximumHeight, 360, "Maximum height should be 360")
        }

        // =====================================================================
        // TEST: Close Button
        // =====================================================================
        
        function test_closeButton_exists() {
            // Arrange
            var button = findChild(pipWindow, "closeButton")
            
            // Assert
            verify(button !== null, "Close button should exist")
            compare(button.width, 26, "Close button width should be 26")
            compare(button.radius, 13, "Close button should be circular")
        }
        
        function test_closeButton_click_emitsSignal() {
            if (root.isOffscreen) { 
                skip("Mouse events not supported in offscreen mode")
                return 
            }
            
            // Arrange
            var button = findChild(pipWindow, "closeButton")
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(closeSpy.count, 1, "closeRequested should be emitted")
        }
        
        function test_closeButton_hidden_whenControlsNotVisible() {
            // Arrange
            var button = findChild(pipWindow, "closeButton")
            
            // Act
            pipWindow.controlsVisible = false
            waitForRendering(pipWindow)
            
            // Assert
            compare(button.opacity, 0.0, "Close button should be hidden")
        }

        // =====================================================================
        // TEST: Play/Pause Button
        // =====================================================================
        
        function test_playPauseButton_exists() {
            // Arrange
            var button = findChild(pipWindow, "playPauseButton")
            
            // Assert
            verify(button !== null, "Play/pause button should exist")
            compare(button.width, 36, "Button width should be 36")
            compare(button.radius, 18, "Button should be circular")
        }
        
        function test_playPauseButton_click_emitsSignal() {
            if (root.isOffscreen) { 
                skip("Mouse events not supported in offscreen mode")
                return 
            }
            
            // Arrange
            var button = findChild(pipWindow, "playPauseButton")
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(playPauseSpy.count, 1, "playPauseRequested should be emitted")
        }
        
        function test_playPauseButton_isPlaying_property() {
            // Arrange
            var button = findChild(pipWindow, "playPauseButton")
            
            // Assert initial state
            compare(button.isPlaying, false, "Should not be playing initially")
            
            // Act - Set playing
            pipWindow.playing = true
            pipWindow.paused = false
            
            // Assert
            compare(button.isPlaying, true, "isPlaying should be true")
            
            // Act - Pause
            pipWindow.paused = true
            
            // Assert
            compare(button.isPlaying, false, "isPlaying should be false when paused")
        }

        // =====================================================================
        // TEST: Return Button
        // =====================================================================
        
        function test_returnButton_exists() {
            // Arrange
            var button = findChild(pipWindow, "returnButton")
            
            // Assert
            verify(button !== null, "Return button should exist")
        }
        
        function test_returnButton_click_emitsSignal() {
            if (root.isOffscreen) { 
                skip("Mouse events not supported in offscreen mode")
                return 
            }
            
            // Arrange
            var button = findChild(pipWindow, "returnButton")
            
            // Act
            mouseClick(button)
            
            // Assert
            compare(returnSpy.count, 1, "returnToAppRequested should be emitted")
        }

        // =====================================================================
        // TEST: Streamer Badge
        // =====================================================================
        
        function test_streamerBadge_hidden_whenNoName() {
            // Arrange
            var badge = findChild(pipWindow, "streamerBadge")
            pipWindow.streamerName = ""
            
            // Assert
            compare(badge.opacity, 0.0, "Badge should be hidden when no name")
        }
        
        function test_streamerBadge_visible_whenHasName() {
            // Arrange
            var badge = findChild(pipWindow, "streamerBadge")
            
            // Act
            pipWindow.streamerName = "TestStreamer"
            waitForRendering(pipWindow)
            
            // Assert
            compare(badge.opacity, 1.0, "Badge should be visible")
        }
        
        function test_streamerBadge_showsCorrectName() {
            // Arrange
            var label = findChild(pipWindow, "streamerLabel")
            
            // Act
            pipWindow.streamerName = "xQc"
            waitForRendering(pipWindow)
            
            // Assert
            compare(label.text, "xQc", "Label should show streamer name")
        }

        // =====================================================================
        // TEST: Video Reparenting
        // =====================================================================
        
        function test_attachVideo_setsVideoItem() {
            // Arrange
            var mockVideo = createTemporaryObject(mockVideoComponent, root)
            
            // Act
            pipWindow.attachVideo(mockVideo)
            
            // Assert
            compare(pipWindow.videoItem, mockVideo, "videoItem should be set")
        }
        
        function test_attachVideo_reparentsToContainer() {
            // Arrange
            var mockVideo = createTemporaryObject(mockVideoComponent, root)
            var container = findChild(pipWindow, "videoContainer")
            
            // Act
            pipWindow.attachVideo(mockVideo)
            
            // Assert
            compare(mockVideo.parent, container, "Video should be reparented")
        }
        
        function test_detachVideo_clearsVideoItem() {
            // Arrange
            var mockVideo = createTemporaryObject(mockVideoComponent, root)
            pipWindow.attachVideo(mockVideo)
            
            // Act
            var detached = pipWindow.detachVideo()
            
            // Assert
            compare(pipWindow.videoItem, null, "videoItem should be null")
            compare(detached, mockVideo, "Should return detached video")
        }
        
        function test_detachVideo_returnsNull_whenNoVideo() {
            // Act
            var result = pipWindow.detachVideo()
            
            // Assert
            compare(result, null, "Should return null when no video")
        }

        // =====================================================================
        // TEST: Size Constraints
        // =====================================================================
        
        function test_constrainWidth_respectsMinimum() {
            // Act
            var result = pipWindow.constrainWidth(100)
            
            // Assert
            compare(result, 320, "Width should be clamped to minimum")
        }
        
        function test_constrainWidth_respectsMaximum() {
            // Act
            var result = pipWindow.constrainWidth(1000)
            
            // Assert
            compare(result, 640, "Width should be clamped to maximum")
        }
        
        function test_constrainWidth_allowsValidValue() {
            // Act
            var result = pipWindow.constrainWidth(500)
            
            // Assert
            compare(result, 500, "Valid width should pass through")
        }
        
        function test_resizeWithRatio_maintains16by9() {
            // Act
            pipWindow.resizeWithRatio(480)
            
            // Assert
            var ratio = pipWindow.width / pipWindow.height
            fuzzyCompare(ratio, 16/9, 0.02, "Ratio should remain 16:9")
        }
        
        function test_resizeWithRatio_clampsToConstraints() {
            // Act
            pipWindow.resizeWithRatio(100)  // Below minimum
            
            // Assert
            verify(pipWindow.width >= pipWindow.minimumWidth, 
                   "Width should not go below minimum")
            verify(pipWindow.height >= pipWindow.minimumHeight, 
                   "Height should not go below minimum")
        }

        // =====================================================================
        // TEST: Controls Visibility
        // =====================================================================
        
        function test_showControls_makesVisible() {
            // Arrange
            pipWindow.controlsVisible = false
            
            // Act
            pipWindow.showControls()
            
            // Assert
            compare(pipWindow.controlsVisible, true, "Controls should be visible")
        }
        
        function test_bottomControls_hidden_whenControlsNotVisible() {
            // Arrange
            var controls = findChild(pipWindow, "bottomControls")
            
            // Act
            pipWindow.controlsVisible = false
            waitForRendering(pipWindow)
            
            // Assert
            compare(controls.opacity, 0.0, "Bottom controls should be hidden")
        }
        
        function test_bottomControls_visible_whenControlsVisible() {
            // Arrange
            var controls = findChild(pipWindow, "bottomControls")
            pipWindow.controlsVisible = true
            
            // Assert
            compare(controls.opacity, 1.0, "Bottom controls should be visible")
        }

        // =====================================================================
        // TEST: Appearance
        // =====================================================================
        
        function test_window_hasRoundedCorners() {
            // Assert
            compare(pipWindow.radius, 16, "Window should have 16px radius")
        }
        
        function test_window_hasBlackBackground() {
            // Assert
            compare(pipWindow.color, "#000000", "Background should be black")
        }
        
        function test_window_hasBorder() {
            // Assert
            compare(pipWindow.border.width, 1, "Should have 1px border")
        }
    }
}
