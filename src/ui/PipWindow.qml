import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Qt.labs.settings 1.1

import "themes/BlueTheme.js" as BlueTheme
import "components"

/**
 * PipWindow.qml - Picture-in-Picture floating window
 * 
 * A frameless, always-on-top window that displays the video player
 * in a compact floating format. Supports drag, resize with 16:9 ratio,
 * and mini controls on hover.
 */
Window {
    id: pipWindow

    // ─────────────────────────────────────────────────────────────────────────
    // Public Properties
    // ─────────────────────────────────────────────────────────────────────────
    
    /** The video item to be reparented into this window */
    property Item videoItem: null
    
    /** Current playback state */
    property bool playing: false
    property bool paused: false
    
    /** Streamer info for display */
    property string streamerName: ""

    // ─────────────────────────────────────────────────────────────────────────
    // Signals
    // ─────────────────────────────────────────────────────────────────────────
    
    /** Emitted when user clicks close button - stops playback */
    signal closeRequested()
    
    /** Emitted when user clicks "return to app" - keeps playback */
    signal returnToAppRequested()
    
    /** Emitted when user toggles play/pause */
    signal playPauseRequested()

    // ─────────────────────────────────────────────────────────────────────────
    // Window Configuration
    // ─────────────────────────────────────────────────────────────────────────
    
    flags: Qt.Window | Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint
    color: "transparent"
    
    // Default size (16:9 ratio)
    width: 400
    height: 225
    
    // Size constraints
    minimumWidth: 320
    minimumHeight: 180
    maximumWidth: 640
    maximumHeight: 360

    // ─────────────────────────────────────────────────────────────────────────
    // Settings Persistence
    // ─────────────────────────────────────────────────────────────────────────
    
    Settings {
        id: pipSettings
        category: "pip"
        property int windowX: -1
        property int windowY: -1
        property int windowWidth: 400
        property int windowHeight: 225
    }
    
    // Restore position/size on show
    onVisibleChanged: {
        if (visible) {
            if (pipSettings.windowX >= 0 && pipSettings.windowY >= 0) {
                x = pipSettings.windowX
                y = pipSettings.windowY
            } else {
                // Default: bottom-right corner with margin
                x = Screen.width - width - 24
                y = Screen.height - height - 80
            }
            width = pipSettings.windowWidth
            height = pipSettings.windowHeight
        } else {
            // Save on hide
            pipSettings.windowX = x
            pipSettings.windowY = y
            pipSettings.windowWidth = width
            pipSettings.windowHeight = height
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Main Container
    // ─────────────────────────────────────────────────────────────────────────
    
    Rectangle {
        id: container
        anchors.fill: parent
        radius: 16
        color: "#000000"
        clip: true
        
        // Subtle border for visibility against dark backgrounds
        border.color: "#40FFFFFF"
        border.width: 1

        // ─────────────────────────────────────────────────────────────────────
        // Video Container - Video item will be reparented here
        // ─────────────────────────────────────────────────────────────────────
        
        Item {
            id: videoContainer
            anchors.fill: parent
            anchors.margins: 1  // Account for border
            
            // Video item is reparented here dynamically
        }

        // ─────────────────────────────────────────────────────────────────────
        // Drag Area (entire window)
        // ─────────────────────────────────────────────────────────────────────
        
        MouseArea {
            id: dragArea
            anchors.fill: parent
            hoverEnabled: true
            
            property point clickPos: Qt.point(0, 0)
            property bool dragging: false
            
            onEntered: {
                container.controlsVisible = true
                hideControlsTimer.restart()
            }
            
            onPressed: function(mouse) {
                clickPos = Qt.point(mouse.x, mouse.y)
                dragging = true
                container.controlsVisible = true
                hideControlsTimer.stop()
            }
            
            onReleased: {
                dragging = false
                hideControlsTimer.restart()
            }
            
            onPositionChanged: function(mouse) {
                if (dragging) {
                    var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                    pipWindow.x += delta.x
                    pipWindow.y += delta.y
                }
                container.controlsVisible = true
                hideControlsTimer.restart()
            }
            
            onDoubleClicked: {
                console.info("[PiP] Returned to main window (double-click)")
                pipWindow.returnToAppRequested()
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        // Resize Handle (bottom-right corner)
        // ─────────────────────────────────────────────────────────────────────
        
        MouseArea {
            id: resizeHandle
            width: 20
            height: 20
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            cursorShape: Qt.SizeFDiagCursor
            
            property point clickPos: Qt.point(0, 0)
            property size startSize: Qt.size(0, 0)
            
            onPressed: function(mouse) {
                clickPos = mapToGlobal(mouse.x, mouse.y)
                startSize = Qt.size(pipWindow.width, pipWindow.height)
            }
            
            onPositionChanged: function(mouse) {
                if (pressed) {
                    var currentPos = mapToGlobal(mouse.x, mouse.y)
                    var deltaX = currentPos.x - clickPos.x
                    
                    // Calculate new width
                    var newWidth = Math.max(pipWindow.minimumWidth, 
                                           Math.min(pipWindow.maximumWidth, startSize.width + deltaX))
                    
                    // Force 16:9 ratio
                    var newHeight = Math.round(newWidth * 9 / 16)
                    
                    // Clamp height to constraints
                    newHeight = Math.max(pipWindow.minimumHeight, 
                                        Math.min(pipWindow.maximumHeight, newHeight))
                    
                    // Recalculate width to maintain ratio if height was clamped
                    newWidth = Math.round(newHeight * 16 / 9)
                    
                    pipWindow.width = newWidth
                    pipWindow.height = newHeight
                }
            }
            
            // Visual indicator
            Rectangle {
                anchors.fill: parent
                color: "transparent"
                
                Canvas {
                    anchors.fill: parent
                    anchors.margins: 4
                    opacity: resizeHandle.containsMouse || resizeHandle.pressed ? 0.8 : 0.3
                    
                    Behavior on opacity {
                        NumberAnimation { duration: BlueTheme.animHoverDuration }
                    }
                    
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset()
                        ctx.strokeStyle = "#FFFFFF"
                        ctx.lineWidth = 1.5
                        ctx.lineCap = "round"
                        
                        // Diagonal resize lines
                        ctx.beginPath()
                        ctx.moveTo(width, 0)
                        ctx.lineTo(0, height)
                        ctx.stroke()
                        
                        ctx.beginPath()
                        ctx.moveTo(width, height * 0.4)
                        ctx.lineTo(width * 0.4, height)
                        ctx.stroke()
                    }
                }
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        // Controls Visibility
        // ─────────────────────────────────────────────────────────────────────
        
        property bool controlsVisible: true
        
        Timer {
            id: hideControlsTimer
            interval: 2000
            onTriggered: {
                if (!dragArea.containsMouse && !closeButton.hovered && 
                    !playPauseButton.hovered && !returnButton.hovered) {
                    container.controlsVisible = false
                }
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        // Close Button (top-right)
        // ─────────────────────────────────────────────────────────────────────
        
        Rectangle {
            id: closeButton
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            width: 26
            height: 26
            radius: 13
            color: hovered ? "#E0000000" : "#99000000"
            opacity: container.controlsVisible ? 1.0 : 0.0
            visible: opacity > 0
            
            property bool hovered: closeMouseArea.containsMouse
            
            Behavior on color {
                ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
            }
            
            Behavior on opacity {
                NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
            }
            
            // Scale animation on hover
            scale: closeMouseArea.pressed ? 0.9 : (hovered ? 1.1 : 1.0)
            Behavior on scale {
                NumberAnimation { duration: BlueTheme.animPressDuration; easing.type: Easing.OutQuart }
            }
            
            Text {
                anchors.centerIn: parent
                text: "\u2715"  // ✕
                color: closeMouseArea.containsMouse ? "#FF6961" : "#FFFFFF"
                font.pixelSize: 12
                font.weight: Font.DemiBold
                
                Behavior on color {
                    ColorAnimation { duration: BlueTheme.animHoverDuration }
                }
            }
            
            MouseArea {
                id: closeMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    console.info("[PiP] Closed")
                    pipWindow.closeRequested()
                }
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        // Bottom Controls (center)
        // ─────────────────────────────────────────────────────────────────────
        
        Rectangle {
            id: bottomControls
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 14
            width: controlsRow.width + 24
            height: 44
            radius: 22
            color: "#99000000"
            opacity: container.controlsVisible ? 1.0 : 0.0
            visible: opacity > 0
            
            Behavior on opacity {
                NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
            }
            
            Row {
                id: controlsRow
                anchors.centerIn: parent
                spacing: 14
                
                // Play/Pause Button
                Rectangle {
                    id: playPauseButton
                    width: 36
                    height: 36
                    radius: 18
                    color: hovered ? "#40FFFFFF" : "transparent"
                    
                    property bool hovered: playPauseMouseArea.containsMouse
                    property bool isPlaying: pipWindow.playing && !pipWindow.paused
                    
                    Behavior on color {
                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                    }
                    
                    // Scale animation
                    scale: playPauseMouseArea.pressed ? 0.9 : (hovered ? 1.05 : 1.0)
                    Behavior on scale {
                        NumberAnimation { duration: BlueTheme.animPressDuration; easing.type: Easing.OutQuart }
                    }
                    
                    // Play/Pause icon using Canvas
                    Canvas {
                        anchors.centerIn: parent
                        width: 16
                        height: 16
                        
                        property bool isPlaying: playPauseButton.isPlaying
                        onIsPlayingChanged: requestPaint()
                        
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.reset()
                            ctx.fillStyle = "#FFFFFF"
                            
                            if (isPlaying) {
                                // Pause icon (two bars)
                                ctx.fillRect(1, 0, 4, 14)
                                ctx.fillRect(9, 0, 4, 14)
                            } else {
                                // Play icon (triangle)
                                ctx.beginPath()
                                ctx.moveTo(2, 0)
                                ctx.lineTo(14, 7)
                                ctx.lineTo(2, 14)
                                ctx.closePath()
                                ctx.fill()
                            }
                        }
                    }
                    
                    MouseArea {
                        id: playPauseMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: pipWindow.playPauseRequested()
                    }
                    
                    ToolTip.visible: hovered
                    ToolTip.text: isPlaying ? qsTr("Pause") : qsTr("Play")
                    ToolTip.delay: 500
                }
                
                // Separator
                Rectangle {
                    width: 1
                    height: 22
                    color: "#4DFFFFFF"
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                // Return to App Button
                Rectangle {
                    id: returnButton
                    width: 36
                    height: 36
                    radius: 18
                    color: hovered ? "#40FFFFFF" : "transparent"
                    
                    property bool hovered: returnMouseArea.containsMouse
                    
                    Behavior on color {
                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                    }
                    
                    // Scale animation
                    scale: returnMouseArea.pressed ? 0.9 : (hovered ? 1.05 : 1.0)
                    Behavior on scale {
                        NumberAnimation { duration: BlueTheme.animPressDuration; easing.type: Easing.OutQuart }
                    }
                    
                    // Return to app icon (PiP exit - arrow pointing to larger window)
                    Canvas {
                        anchors.centerIn: parent
                        width: 18
                        height: 14
                        
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.reset()
                            ctx.strokeStyle = "#FFFFFF"
                            ctx.fillStyle = "#FFFFFF"
                            ctx.lineWidth = 1.5
                            ctx.lineCap = "round"
                            ctx.lineJoin = "round"
                            
                            // Large window outline using moveTo/lineTo
                            ctx.beginPath()
                            ctx.moveTo(1, 1)
                            ctx.lineTo(17, 1)
                            ctx.lineTo(17, 13)
                            ctx.lineTo(1, 13)
                            ctx.closePath()
                            ctx.stroke()
                            
                            // Arrow pointing from bottom-right to center
                            ctx.beginPath()
                            ctx.moveTo(14, 10)
                            ctx.lineTo(7, 5)
                            ctx.stroke()
                            
                            // Arrow head
                            ctx.beginPath()
                            ctx.moveTo(7, 5)
                            ctx.lineTo(11, 5)
                            ctx.lineTo(7, 9)
                            ctx.closePath()
                            ctx.fill()
                        }
                    }
                    
                    MouseArea {
                        id: returnMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.info("[PiP] Returned to main window")
                            pipWindow.returnToAppRequested()
                        }
                    }
                    
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Return to app")
                    ToolTip.delay: 500
                }
            }
        }

        // ─────────────────────────────────────────────────────────────────────
        // Streamer Name Badge (bottom-left, subtle)
        // ─────────────────────────────────────────────────────────────────────
        
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: 10
            width: streamerLabel.width + 16
            height: 26
            radius: 13
            color: "#80000000"
            opacity: container.controlsVisible && pipWindow.streamerName.length > 0 ? 1.0 : 0.0
            visible: opacity > 0
            
            Behavior on opacity {
                NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
            }
            
            Text {
                id: streamerLabel
                anchors.centerIn: parent
                text: pipWindow.streamerName
                color: "#FFFFFF"
                font.pixelSize: 12
                font.family: BlueTheme.fontFamily
                font.weight: Font.Medium
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Video Reparenting Functions
    // ─────────────────────────────────────────────────────────────────────────
    
    /** Reparent the video item into this PiP window */
    function attachVideo(item) {
        if (item) {
            videoItem = item
            item.parent = videoContainer
            item.anchors.fill = videoContainer
        }
    }
    
    /** Detach the video item (caller should reparent it elsewhere) */
    function detachVideo() {
        if (videoItem) {
            videoItem.anchors.fill = undefined
            var item = videoItem
            videoItem = null
            return item
        }
        return null
    }
}
