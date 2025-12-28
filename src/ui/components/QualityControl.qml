import QtQuick 2.15
import QtQuick.Controls 6.5

import "../themes/BlueTheme.js" as BlueTheme

/**
 * QualityControl - Quality selector button with popup
 * 
 * Shows a quality button that reveals a popup with available
 * quality options on click. Uses anchors for reliable positioning.
 */
Item {
    id: root

    // Properties
    property var qualities: []           // List of {name: "1080p60", url: "..."}
    property string currentQuality: ""   // Currently selected quality

    // Signals
    signal qualitySelected(string quality)

    // Internal state for popup visibility
    property bool showPopup: false

    // Default size (matches chipHeight)
    width: 32
    height: 32

    // Quality Button (HD icon)
    Rectangle {
        id: qualityButton
        anchors.fill: parent
        radius: width / 2
        color: qualityMouseArea.containsMouse || root.showPopup ? "#33FFFFFF" : "#1AFFFFFF"
        border.color: root.showPopup ? BlueTheme.accent : "#4DFFFFFF"
        border.width: 1

        Behavior on color {
            ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
        }
        Behavior on border.color {
            ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
        }

        // Quality Icon - HD/SD or Audio indicator
        Text {
            anchors.centerIn: parent
            text: {
                if (BlueTheme.isAudioQuality(root.currentQuality)) {
                    return "\uD83D\uDD0A"  // 🔊
                }
                var q = root.currentQuality.toLowerCase()
                if (q.indexOf("1440") >= 0 || q.indexOf("1080") >= 0 || q.indexOf("720") >= 0) {
                    return "HD"
                }
                return "SD"
            }
            font.pixelSize: BlueTheme.isAudioQuality(root.currentQuality) ? 14 : 10
            font.family: BlueTheme.fontFamily
            font.bold: true
            font.letterSpacing: -0.5
            color: "#FFFFFF"
        }

        MouseArea {
            id: qualityMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor

            onClicked: {
                root.showPopup = !root.showPopup
            }
        }

        ToolTip.visible: qualityMouseArea.containsMouse && !root.showPopup
        ToolTip.text: BlueTheme.isAudioQuality(root.currentQuality) 
            ? qsTr("Audio only mode (Q)") 
            : qsTr("Quality: %1 (Q)").arg(BlueTheme.formatQuality(root.currentQuality))
        ToolTip.delay: 800
    }

    // Quality Selector Popup - positioned above the button
    Rectangle {
        id: qualityPopup
        width: 180
        height: root.showPopup ? contentColumn.height + 24 : 0
        anchors.bottom: qualityButton.top
        anchors.bottomMargin: 8
        anchors.horizontalCenter: qualityButton.horizontalCenter
        radius: 16
        color: "#E6141c2a"
        border.color: "#4DFFFFFF"
        border.width: 1
        clip: true
        opacity: root.showPopup ? 1.0 : 0.0
        visible: height > 0

        Behavior on height {
            NumberAnimation { 
                duration: BlueTheme.animOverlayDuration
                easing.type: Easing.OutCubic 
            }
        }
        Behavior on opacity {
            NumberAnimation { 
                duration: BlueTheme.animHoverDuration
                easing.type: Easing.OutCubic 
            }
        }

        Column {
            id: contentColumn
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 12
            spacing: 4

            // Header
            Text {
                text: qsTr("Quality")
                color: BlueTheme.secondaryText
                font.pixelSize: 11
                font.family: BlueTheme.fontFamily
                font.weight: Font.Medium
                bottomPadding: 8
            }

            // Divider
            Rectangle {
                width: parent.width
                height: 1
                color: BlueTheme.divider
            }

            // Spacer
            Item { width: 1; height: 4 }

            // Quality options
            Repeater {
                model: root.qualities

                delegate: Rectangle {
                    id: qualityItem
                    width: contentColumn.width
                    height: 36
                    radius: 8
                    color: itemMouse.containsMouse ? "#1AFFFFFF" : "transparent"
                    
                    property bool isSelected: modelData.name === root.currentQuality

                    Behavior on color {
                        ColorAnimation { 
                            duration: BlueTheme.animHoverDuration
                            easing.type: Easing.OutCubic 
                        }
                    }

                    Row {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        spacing: 12

                        // Radio indicator
                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 18
                            height: 18
                            radius: 9
                            color: "transparent"
                            border.color: qualityItem.isSelected ? BlueTheme.accent : "#66FFFFFF"
                            border.width: qualityItem.isSelected ? 2 : 1.5

                            Behavior on border.color {
                                ColorAnimation { 
                                    duration: BlueTheme.animHoverDuration 
                                }
                            }

                            // Inner dot when selected
                            Rectangle {
                                anchors.centerIn: parent
                                width: qualityItem.isSelected ? 8 : 0
                                height: width
                                radius: width / 2
                                color: BlueTheme.accent

                                Behavior on width {
                                    NumberAnimation { 
                                        duration: BlueTheme.animPressDuration
                                        easing.type: Easing.OutQuart 
                                    }
                                }
                            }
                        }

                        // Quality label
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: BlueTheme.formatQuality(modelData.name)
                            color: qualityItem.isSelected ? "#FFFFFF" : BlueTheme.secondaryText
                            font.pixelSize: 13
                            font.family: BlueTheme.fontFamily
                            font.weight: qualityItem.isSelected ? Font.DemiBold : Font.Normal

                            Behavior on color {
                                ColorAnimation { 
                                    duration: BlueTheme.animHoverDuration 
                                }
                            }
                        }
                    }

                    MouseArea {
                        id: itemMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.qualitySelected(modelData.name)
                            root.showPopup = false
                        }
                    }
                }
            }
        }

        // Keep popup open when interacting
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true
            onPressed: function(mouse) { mouse.accepted = false }
            onReleased: function(mouse) { mouse.accepted = false }
        }
    }

    // Click outside to close
    Connections {
        target: root.parent
        function onPressed() {
            if (root.showPopup) {
                root.showPopup = false
            }
        }
    }

    // Close popup when component loses focus or Q key is pressed
    Keys.onPressed: function(event) {
        if (event.key === Qt.Key_Q && root.qualities.length > 0) {
            root.showPopup = !root.showPopup
            event.accepted = true
        } else if (event.key === Qt.Key_Escape && root.showPopup) {
            root.showPopup = false
            event.accepted = true
        }
    }
}
