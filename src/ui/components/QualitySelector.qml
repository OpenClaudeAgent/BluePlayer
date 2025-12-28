import QtQuick 2.15
import QtQuick.Controls 6.5

import "../themes/BlueTheme.js" as BlueTheme

/**
 * QualitySelector - Popup for video quality selection
 * 
 * Opens above the trigger button with a smooth animation.
 * Displays available qualities as radio-style options.
 */
Item {
    id: root

    // Model: array of {name: "1080p60", url: "...", value: "chunked"}
    property var qualities: []
    property string currentQuality: ""
    property bool expanded: false

    // Signals
    signal qualitySelected(string quality)
    signal closeRequested()

    // Auto-sizing based on content
    width: popup.width
    height: popup.height

    // Background overlay to catch clicks outside
    Rectangle {
        id: overlay
        anchors.fill: parent
        anchors.margins: -2000
        color: "transparent"
        visible: root.expanded

        MouseArea {
            anchors.fill: parent
            onClicked: root.closeRequested()
        }
    }

    // Main popup container
    Rectangle {
        id: popup
        width: 180
        height: root.expanded ? contentColumn.height + 24 : 0
        radius: 16
        color: "#E6141c2a"
        border.color: "#4DFFFFFF"
        border.width: 1
        clip: true
        opacity: root.expanded ? 1.0 : 0.0
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
                            text: modelData.name
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
                            root.closeRequested()
                        }
                    }
                }
            }
        }

        // Keep popup open when hovering
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true
            onPressed: function(mouse) { mouse.accepted = false }
            onReleased: function(mouse) { mouse.accepted = false }
        }
    }

    // Keyboard handling
    Keys.onEscapePressed: root.closeRequested()

    // Focus management
    onExpandedChanged: {
        if (expanded) {
            root.forceActiveFocus()
        }
    }
}
