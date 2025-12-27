import QtQuick 2.15
import QtQuick.Controls 6.5

import "../themes/BlueTheme.js" as BlueTheme

/**
 * SeekBar - Video position slider with live/VOD/replay support
 * 
 * Handles seeking in both live streams (always at edge) and 
 * VOD content (follows actual position).
 */
Slider {
    id: seekBar

    // Required properties
    property real duration: 0.0
    property real currentPosition: 0.0
    property bool liveMode: true
    property bool isReplayMode: false  // Local file mode (never live)

    // Signals
    signal seekRequested(real seconds)
    signal seekDragStarted()
    signal seekDragEnded(real seconds)
    signal seekPreviewed(real seconds)

    // Internal state
    property bool userDragging: false
    property real seekTarget: 0

    // Properties for easier access
    readonly property bool isLiveMode: liveMode && !isReplayMode  // Replay mode = never live
    readonly property real currentDuration: duration

    // In live mode: always at live edge (UI only) - but NOT in replay mode
    readonly property bool atLiveEdge: isLiveMode || (!isReplayMode && currentDuration > 0 && (currentDuration - currentPosition) <= 5)

    // Slider configuration
    enabled: currentDuration > 0
    hoverEnabled: true
    from: 0
    to: currentDuration > 0 ? currentDuration : 1

    // SIMPLE LOGIC:
    // - Live mode (and not replay): slider always at 100% (right edge)
    // - VOD/Replay mode: slider follows actual position from 0
    Binding {
        target: seekBar
        property: "value"
        value: seekBar.isLiveMode ? seekBar.to : seekBar.currentPosition
        when: !seekBar.userDragging && !seekBar.pressed
    }

    onPressedChanged: {
        if (pressed) {
            userDragging = true
            seekDragStarted()
            seekTarget = isLiveMode ? currentDuration : currentPosition
        } else if (userDragging) {
            userDragging = false
            seekDragEnded(seekTarget)
            seekRequested(seekTarget)
        }
    }

    onMoved: {
        if (userDragging || pressed) {
            seekTarget = value
            seekPreviewed(value)
        }
    }

    background: Rectangle {
        x: seekBar.leftPadding
        y: seekBar.topPadding + seekBar.availableHeight / 2 - height / 2
        width: seekBar.availableWidth
        height: 6
        radius: 3
        color: "#26FFFFFF"

        // Buffered/available area
        Rectangle {
            width: parent.width
            height: parent.height
            radius: parent.radius
            color: "#44FFFFFF"
        }

        // Played progress
        Rectangle {
            width: seekBar.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            gradient: Gradient {
                GradientStop { position: 0.0; color: BlueTheme.accent }
                GradientStop { position: 1.0; color: BlueTheme.accentSubtle }
            }
            opacity: 0.9
        }

        // Hover highlight
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: BlueTheme.accent
            opacity: (seekBar.pressed || seekBar.userDragging) ? 0.10 : (seekBar.hovered ? 0.06 : 0.0)
            Behavior on opacity { NumberAnimation { duration: BlueTheme.animPressDuration; easing.type: Easing.OutCubic } }
        }
    }

    handle: Item {
        x: seekBar.leftPadding + seekBar.visualPosition * (seekBar.availableWidth - width)
        y: seekBar.topPadding + seekBar.availableHeight / 2 - height / 2
        width: 16
        height: 16

        // Hover/drag halo
        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 6
            height: parent.height + 6
            radius: width / 2
            color: BlueTheme.accent
            opacity: (seekBar.pressed || seekBar.userDragging) ? 0.16 : (seekBar.hovered ? 0.10 : 0.0)
            visible: opacity > 0
            antialiasing: true
        }

        // Handle
        Rectangle {
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
            radius: width / 2
            color: "#FFFFFF"
            border.color: seekBar.hovered || seekBar.pressed || seekBar.userDragging ? BlueTheme.accent : "#B3FFFFFF"
            border.width: 1
            opacity: seekBar.enabled ? 1.0 : 0.6
            scale: seekBar.pressed ? 1.12 : (seekBar.hovered ? 1.06 : 1.0)
            Behavior on scale { NumberAnimation { duration: BlueTheme.animPressDuration; easing.type: Easing.OutCubic } }
            Behavior on border.color { ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic } }
        }
    }
}
