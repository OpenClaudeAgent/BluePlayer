import QtQuick 2.15
import QtQuick.Controls 6.5

import "../themes/BlueTheme.js" as BlueTheme

/**
 * ControlButton - Reusable round control button for player controls
 * 
 * A consistent button component with hover/press animations, 
 * tooltip support, and customizable content.
 */
Rectangle {
    id: root

    // Content alias for custom icon/content
    default property alias icon: contentContainer.children
    
    // Button properties
    property string tooltipText: ""
    property bool active: false
    property bool showBorder: true
    
    // Size defaults (can be overridden)
    width: 32
    height: 32
    radius: width / 2

    // Style
    color: active ? BlueTheme.accent : (mouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF")
    border.color: active ? BlueTheme.accent : (showBorder ? "#4DFFFFFF" : "transparent")
    border.width: showBorder ? 1 : 0

    // Animations
    Behavior on color {
        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
    }

    scale: mouseArea.pressed ? 0.92 : (mouseArea.containsMouse ? 1.05 : 1.0)
    Behavior on scale {
        NumberAnimation { duration: BlueTheme.animPressDuration; easing.type: Easing.OutQuart }
    }

    // Signal
    signal clicked()

    // Expose hover state for external use
    readonly property bool hovered: mouseArea.containsMouse
    readonly property bool pressed: mouseArea.pressed

    // Content container centered
    Item {
        id: contentContainer
        anchors.centerIn: parent
        width: parent.width * 0.6
        height: parent.height * 0.6
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    ToolTip.visible: mouseArea.containsMouse && tooltipText !== ""
    ToolTip.text: tooltipText
    ToolTip.delay: 800
}
