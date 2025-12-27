import QtQuick 2.15
import QtQuick.Controls 6.5

import "../themes/BlueTheme.js" as BlueTheme

/**
 * VolumeControl - Volume button with vertical popup slider
 * 
 * Shows a volume icon button that reveals a vertical slider 
 * popup on hover for volume adjustment.
 */
Item {
    id: root

    // Properties
    property real volume: 1.0
    property bool muted: false

    // Signals
    signal volumeRequested(real newVolume)
    signal muteClicked()

    // Internal state for popup visibility
    property bool showSlider: false

    // Default size
    width: 32
    height: 32

    // Volume Icon Button
    Rectangle {
        id: volumeButton
        anchors.fill: parent
        radius: width / 2
        color: volumeMouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF"
        border.color: "#4DFFFFFF"
        border.width: 1

        Behavior on color {
            ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
        }

        // Volume Icon (Canvas)
        Canvas {
            id: volumeIcon
            anchors.centerIn: parent
            width: 18
            height: 18

            property real vol: root.muted ? 0 : root.volume
            onVolChanged: requestPaint()
            Component.onCompleted: requestPaint()

            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()
                ctx.fillStyle = "#FFFFFF"
                ctx.strokeStyle = "#FFFFFF"
                ctx.lineWidth = 1.5
                ctx.lineCap = "round"

                // Speaker body (scaled down)
                ctx.beginPath()
                ctx.moveTo(2, 6)
                ctx.lineTo(5, 6)
                ctx.lineTo(9, 3)
                ctx.lineTo(9, 15)
                ctx.lineTo(5, 12)
                ctx.lineTo(2, 12)
                ctx.closePath()
                ctx.fill()

                if (vol === 0 || root.muted) {
                    // X for muted
                    ctx.beginPath()
                    ctx.moveTo(12, 6)
                    ctx.lineTo(16, 12)
                    ctx.stroke()
                    ctx.beginPath()
                    ctx.moveTo(16, 6)
                    ctx.lineTo(12, 12)
                    ctx.stroke()
                } else {
                    // Sound waves
                    if (vol > 0) {
                        ctx.beginPath()
                        ctx.arc(9, 9, 3, -Math.PI/3, Math.PI/3, false)
                        ctx.stroke()
                    }
                    if (vol > 0.5) {
                        ctx.beginPath()
                        ctx.arc(9, 9, 6, -Math.PI/3, Math.PI/3, false)
                        ctx.stroke()
                    }
                }
            }
        }

        MouseArea {
            id: volumeMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                root.muteClicked()
                hideVolumeTimer.restart()
            }

            onEntered: {
                root.showSlider = true
                hideVolumeTimer.stop()
            }
        }

        ToolTip.visible: volumeMouseArea.containsMouse && !root.showSlider
        ToolTip.text: root.muted ? qsTr("Activer le son (M)") : qsTr("Couper le son (M)")
        ToolTip.delay: 800
    }

    // Vertical Volume Slider Popup
    Rectangle {
        id: volumeSliderPopup
        width: 36
        height: root.showSlider ? 110 : 0
        anchors.bottom: volumeButton.top
        anchors.bottomMargin: 8
        anchors.horizontalCenter: volumeButton.horizontalCenter
        radius: 18
        color: "#CC1C1C1E"
        border.color: "#4DFFFFFF"
        border.width: 1
        clip: true
        opacity: root.showSlider ? 1.0 : 0.0
        visible: height > 0

        Behavior on height {
            NumberAnimation { duration: BlueTheme.animOverlayDuration; easing.type: Easing.OutCubic }
        }
        Behavior on opacity {
            NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
        }

        Slider {
            id: volumeSlider
            anchors.centerIn: parent
            orientation: Qt.Vertical
            height: 85
            width: 26
            from: 0.0
            to: 1.0
            value: root.muted ? 0 : root.volume

            background: Rectangle {
                x: volumeSlider.leftPadding + volumeSlider.availableWidth / 2 - width / 2
                y: volumeSlider.topPadding
                width: 4
                height: volumeSlider.availableHeight
                radius: 2
                color: "#33FFFFFF"

                Rectangle {
                    width: parent.width
                    height: (1 - volumeSlider.visualPosition) * parent.height
                    anchors.bottom: parent.bottom
                    radius: 2
                    color: "#FFFFFF"
                }
            }

            handle: Rectangle {
                x: volumeSlider.leftPadding + volumeSlider.availableWidth / 2 - width / 2
                y: volumeSlider.topPadding + volumeSlider.visualPosition * (volumeSlider.availableHeight - height)
                width: 14
                height: 14
                radius: 7
                color: "#FFFFFF"

                scale: volumeSlider.pressed ? 1.2 : 1.0
                Behavior on scale {
                    NumberAnimation { duration: BlueTheme.animPressDuration; easing.type: Easing.OutQuart }
                }
            }

            onMoved: {
                if (root.muted && value > 0) {
                    root.muteClicked()
                }
                root.volumeRequested(value)
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true
            onExited: hideVolumeTimer.restart()
            onEntered: hideVolumeTimer.stop()
            onPressed: function(mouse) { mouse.accepted = false }
            onReleased: function(mouse) { mouse.accepted = false }
        }
    }

    // Timer to hide volume slider
    Timer {
        id: hideVolumeTimer
        interval: 1500
        onTriggered: root.showSlider = false
    }
}
