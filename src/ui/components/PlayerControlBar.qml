import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15

import "../themes/BlueTheme.js" as BlueTheme

/**
 * PlayerControlBar - Modern streaming player controls
 */
Rectangle {
    id: controlBar
    clip: false  // Allow popups to render outside bounds

    // Properties
    property bool playing: false
    property bool paused: false
    property bool buffering: false
    property real volume: 1.0
    property bool muted: false
    property bool showVolumeSlider: false
    property real duration: 0.0
    property real position: 0.0
    property real liveOffset: 0.0
    property bool liveMode: true
    property bool isReplayMode: false  // Mode fichier local (pas de live)
    property real playbackRate: 1.0
    property bool hardwareDecoding: true
    property bool cropVideo: false
    property bool chatVisible: false
    property bool chatEnabled: true  // Disabled in VOD mode
    
    // Quality selector properties
    property var availableQualities: []      // List of {name: "1080p60", url: "..."}
    property string currentQuality: "Auto"   // Currently selected quality
    
    // Picture-in-Picture properties
    property bool pipActive: false           // True when PiP window is open
    property bool pipEnabled: true           // False when in fullscreen (mutually exclusive)

    // Chip button dimensions (for consistent sizing)
    readonly property int chipWidth: 64
    readonly property int chipHeight: 32
    readonly property int chipRadius: 16

    // Signals
    signal playPauseClicked()
    signal stopClicked()
    signal volumeRequested(real newVolume)
    signal muteClicked()
    signal seekRequested(real seconds)
    signal seekDragStarted()
    signal seekDragEnded(real seconds)
    signal seekPreviewed(real seconds)
    signal liveRequested()
    signal liveClicked()
    signal fullscreenClicked()
    signal playbackRateRequested(real rate)
    signal hardwareToggleClicked()
    signal cropToggleClicked()
    signal chatToggleClicked()
    signal qualitySelected(string quality)
    signal pipClicked()

    height: 80

    // Gradient background with transparency
    gradient: Gradient {
        GradientStop { position: 0.0; color: "transparent" }
        GradientStop { position: 0.3; color: "#80000000" }
        GradientStop { position: 1.0; color: "#E0000000" }
    }

    // Auto-hide animation
    property bool autoHide: true
    property bool controlsVisible: true

    opacity: controlsVisible ? 1.0 : 0.0
    Behavior on opacity {
        NumberAnimation { duration: BlueTheme.animControlBarDuration; easing.type: Easing.InOutCubic }
    }

    // Main content - Single row layout
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        anchors.bottomMargin: 20
        anchors.topMargin: 28
        spacing: 8

        // Play/Pause Button
        ControlButton {
            id: playPauseButton
            Layout.preferredWidth: chipHeight
            Layout.preferredHeight: chipHeight
            tooltipText: buffering ? qsTr("Loading...") : (playing && !paused ? qsTr("Pause (Space)") : qsTr("Play (Space)"))
            onClicked: controlBar.playPauseClicked()

            // Play/Pause Icon container
            Item {
                anchors.centerIn: parent
                width: 20
                height: 20

                // Play triangle
                Text {
                    anchors.centerIn: parent
                    visible: !buffering && (paused || !playing)
                    text: "\u25B6"
                    font.pixelSize: 14
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                // Pause bars
                Row {
                    anchors.centerIn: parent
                    spacing: 3
                    visible: !buffering && playing && !paused

                    Rectangle {
                        width: 3
                        height: 12
                        radius: 1
                        color: "#FFFFFF"
                    }
                    Rectangle {
                        width: 3
                        height: 12
                        radius: 1
                        color: "#FFFFFF"
                    }
                }

                // Loading indicator
                Text {
                    anchors.centerIn: parent
                    visible: buffering
                    text: "..."
                    font.pixelSize: 12
                    font.bold: true
                    color: "#FFFFFF"
                }
            }
        }

        // Seek slider - Takes all available space
        SeekBar {
            id: seekSlider
            Layout.fillWidth: true
            Layout.preferredHeight: chipHeight
            duration: controlBar.duration
            currentPosition: controlBar.position
            liveMode: controlBar.liveMode
            isReplayMode: controlBar.isReplayMode

            onSeekRequested: (seconds) => controlBar.seekRequested(seconds)
            onSeekDragStarted: controlBar.seekDragStarted()
            onSeekDragEnded: (seconds) => controlBar.seekDragEnded(seconds)
            onSeekPreviewed: (seconds) => controlBar.seekPreviewed(seconds)
        }

        // LIVE/VOD indicator - Small round indicator (hidden in replay mode)
        Rectangle {
            id: livePill
            visible: !controlBar.isReplayMode
            Layout.preferredWidth: 24
            Layout.preferredHeight: 24
            Layout.alignment: Qt.AlignVCenter
            radius: 12
            color: liveMouseArea.containsMouse ? (seekSlider.atLiveEdge ? "#FF4136" : "#3C3C3E") : (seekSlider.atLiveEdge ? "#FF3B30" : "#2C2C2E")
            border.color: seekSlider.atLiveEdge ? "#FF6961" : "#48484A"
            border.width: 1

            Behavior on color { ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic } }
            Behavior on border.color { ColorAnimation { duration: BlueTheme.animCardDuration; easing.type: Easing.OutCubic } }

            // Pulsing dot centered
            Rectangle {
                anchors.centerIn: parent
                width: 8; height: 8; radius: 4
                color: seekSlider.atLiveEdge ? "#FFFFFF" : "#8E8E93"

                SequentialAnimation on opacity {
                    running: seekSlider.atLiveEdge
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.5; duration: BlueTheme.animPulseDuration }
                    NumberAnimation { to: 1.0; duration: BlueTheme.animPulseDuration }
                }
            }

            MouseArea {
                id: liveMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: controlBar.liveClicked()
            }

            ToolTip.visible: liveMouseArea.containsMouse
            ToolTip.text: seekSlider.atLiveEdge ? qsTr("Re-sync to live") : qsTr("Go to live")
            ToolTip.delay: 800
        }

        // Time display - adapts to mode
        Column {
            Layout.alignment: Qt.AlignVCenter
            spacing: 2

            property bool isReplay: controlBar.isReplayMode
            property bool showTimers: isReplay || !seekSlider.atLiveEdge || controlBar.duration > 0

            // Current position (top)
            Text {
                text: controlBar._formatTime(position)
                color: "#FFFFFF"
                font.pixelSize: 10
                font.family: "Menlo"
                visible: parent.isReplay || !seekSlider.atLiveEdge
                opacity: parent.showTimers ? 1.0 : 0.0
            }

            // Total duration
            Text {
                text: controlBar._formatTime(duration)
                color: parent.isReplay ? "#88FFFFFF" : "#FFFFFF"
                font.pixelSize: 10
                font.family: "Menlo"
                opacity: (controlBar.duration > 0 || parent.isReplay) ? 1.0 : 0.3
            }
        }

        // Right controls group - all aligned
        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignVCenter

            // Speed down
            ControlButton {
                id: rateDown
                width: chipHeight; height: chipHeight
                tooltipText: qsTr("Slow down")
                onClicked: controlBar.playbackRateRequested(Math.max(0.25, playbackRate - 0.1))

                Text { anchors.centerIn: parent; text: "\u2212"; color: "#FFFFFF"; font.pixelSize: 14 }
            }

            // Speed display
            Rectangle {
                id: rateChip
                width: chipWidth; height: chipHeight; radius: chipRadius
                color: "#26FFFFFF"
                border.color: "#4DFFFFFF"; border.width: 1
                Text {
                    anchors.centerIn: parent
                    text: playbackRate.toFixed(2) + "x"
                    color: "#FFFFFF"
                    font.pixelSize: 11
                    font.bold: true
                }
                MouseArea {
                    id: rateChipMouse
                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: controlBar.playbackRateRequested(1.0)
                }
                ToolTip.visible: rateChipMouse.containsMouse
                ToolTip.text: qsTr("Reset speed (R)")
                ToolTip.delay: 800
            }

            // Speed up
            ControlButton {
                id: rateUp
                width: chipHeight; height: chipHeight
                tooltipText: qsTr("Speed up")
                onClicked: controlBar.playbackRateRequested(Math.min(3.0, playbackRate + 0.1))

                Text { anchors.centerIn: parent; text: "+"; color: "#FFFFFF"; font.pixelSize: 14 }
            }

            // HW toggle - compact
            Rectangle {
                id: hwToggle
                width: 44; height: chipHeight; radius: chipRadius
                color: hwMouse.containsMouse ? "#33FFFFFF" : "#26FFFFFF"
                border.color: "#4DFFFFFF"; border.width: 1
                Behavior on color { ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic } }
                Text {
                    anchors.centerIn: parent
                    text: hardwareDecoding ? qsTr("HW") : qsTr("SW")
                    color: "#FFFFFF"
                    font.pixelSize: 11
                    font.bold: true
                }
                MouseArea {
                    id: hwMouse
                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: controlBar.hardwareToggleClicked()
                }
                ToolTip.visible: hwMouse.containsMouse
                ToolTip.text: hardwareDecoding ? qsTr("Hardware decoding") : qsTr("Software decoding")
                ToolTip.delay: 800
            }

            // Fit/Crop toggle - compact
            Rectangle {
                id: cropToggle
                width: 44; height: chipHeight; radius: chipRadius
                color: cropMouse.containsMouse ? "#33FFFFFF" : "#26FFFFFF"
                border.color: "#4DFFFFFF"; border.width: 1
                Behavior on color { ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic } }
                Text {
                    anchors.centerIn: parent
                    text: cropVideo ? qsTr("Crop") : qsTr("Fit")
                    color: "#FFFFFF"
                    font.pixelSize: 11
                    font.bold: true
                }
                MouseArea {
                    id: cropMouse
                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                    onClicked: controlBar.cropToggleClicked()
                }
                ToolTip.visible: cropMouse.containsMouse
                ToolTip.text: cropVideo ? qsTr("Crop mode (V)") : qsTr("Fit mode (V)")
                ToolTip.delay: 800
            }

            // Chat Toggle Button (hidden in VOD mode)
            ControlButton {
                id: chatButton
                visible: chatEnabled
                width: chipHeight; height: chipHeight
                active: chatVisible
                tooltipText: chatVisible ? qsTr("Hide chat (C)") : qsTr("Show chat (C)")
                onClicked: controlBar.chatToggleClicked()

                // Chat bubble icon
                Canvas {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset()
                        ctx.strokeStyle = "#FFFFFF"
                        ctx.fillStyle = "transparent"
                        ctx.lineWidth = 1.5
                        ctx.lineCap = "round"
                        ctx.lineJoin = "round"

                        // Chat bubble shape
                        ctx.beginPath()
                        ctx.moveTo(2, 3)
                        ctx.lineTo(14, 3)
                        ctx.quadraticCurveTo(15, 3, 15, 4)
                        ctx.lineTo(15, 10)
                        ctx.quadraticCurveTo(15, 11, 14, 11)
                        ctx.lineTo(6, 11)
                        ctx.lineTo(3, 14)
                        ctx.lineTo(3, 11)
                        ctx.lineTo(2, 11)
                        ctx.quadraticCurveTo(1, 11, 1, 10)
                        ctx.lineTo(1, 4)
                        ctx.quadraticCurveTo(1, 3, 2, 3)
                        ctx.stroke()

                        // Chat lines
                        ctx.beginPath()
                        ctx.moveTo(4, 6)
                        ctx.lineTo(12, 6)
                        ctx.stroke()
                        ctx.beginPath()
                        ctx.moveTo(4, 9)
                        ctx.lineTo(9, 9)
                        ctx.stroke()
                    }
                }
            }

            // Quality Selector (live mode only, hidden when no qualities available)
            QualityControl {
                id: qualityControl
                visible: !controlBar.isReplayMode && controlBar.availableQualities.length > 0
                Layout.preferredWidth: chipHeight
                Layout.preferredHeight: chipHeight
                qualities: controlBar.availableQualities
                currentQuality: controlBar.currentQuality
                onQualitySelected: function(quality) {
                    controlBar.qualitySelected(quality)
                }
            }

            // Volume Control
            VolumeControl {
                id: volumeControl
                Layout.preferredWidth: chipHeight
                Layout.preferredHeight: chipHeight
                volume: controlBar.volume
                muted: controlBar.muted
                onVolumeRequested: (newVolume) => controlBar.volumeRequested(newVolume)
                onMuteClicked: controlBar.muteClicked()
            }

            // Picture-in-Picture Button
            ControlButton {
                id: pipButton
                width: chipHeight; height: chipHeight
                active: controlBar.pipActive
                enabled: controlBar.pipEnabled
                opacity: enabled ? 1.0 : 0.4
                tooltipText: controlBar.pipActive ? qsTr("Exit Picture-in-Picture (P)") : qsTr("Picture-in-Picture (P)")
                onClicked: controlBar.pipClicked()

                Behavior on opacity {
                    NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                }

                // PiP icon: two nested rectangles
                Canvas {
                    anchors.centerIn: parent
                    width: 16
                    height: 12
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset()
                        ctx.strokeStyle = "#FFFFFF"
                        ctx.fillStyle = "#FFFFFF"
                        ctx.lineWidth = 1.5
                        ctx.lineCap = "round"
                        ctx.lineJoin = "round"

                        // Main window outline using moveTo/lineTo
                        ctx.beginPath()
                        ctx.moveTo(1, 1)
                        ctx.lineTo(15, 1)
                        ctx.lineTo(15, 11)
                        ctx.lineTo(1, 11)
                        ctx.closePath()
                        ctx.stroke()

                        // Small PiP window (filled, bottom-right)
                        ctx.beginPath()
                        ctx.moveTo(9, 5)
                        ctx.lineTo(14, 5)
                        ctx.lineTo(14, 10)
                        ctx.lineTo(9, 10)
                        ctx.closePath()
                        ctx.fill()
                    }
                }
            }

            // Fullscreen Button
            ControlButton {
                id: fullscreenButton
                width: chipHeight; height: chipHeight
                tooltipText: qsTr("Fullscreen (F)")
                onClicked: controlBar.fullscreenClicked()

                Canvas {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset()
                        ctx.strokeStyle = "#FFFFFF"
                        ctx.lineWidth = 1.5
                        ctx.lineCap = "round"

                        // Top Left
                        ctx.beginPath(); ctx.moveTo(0, 5); ctx.lineTo(0,0); ctx.lineTo(5,0); ctx.stroke();
                        // Top Right
                        ctx.beginPath(); ctx.moveTo(11, 0); ctx.lineTo(16,0); ctx.lineTo(16,5); ctx.stroke();
                        // Bottom Left
                        ctx.beginPath(); ctx.moveTo(0, 11); ctx.lineTo(0,16); ctx.lineTo(5,16); ctx.stroke();
                        // Bottom Right
                        ctx.beginPath(); ctx.moveTo(11, 16); ctx.lineTo(16,16); ctx.lineTo(16,11); ctx.stroke();
                    }
                }
            }
        }
    }

    function _formatTime(sec) {
        if (sec <= 0 || sec !== sec) return "00:00";
        var total = Math.floor(sec);
        var m = Math.floor(total / 60);
        var s = total % 60;
        var mm = m < 10 ? "0" + m : m;
        var ss = s < 10 ? "0" + s : s;
        return mm + ":" + ss;
    }
}
