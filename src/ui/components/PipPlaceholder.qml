import QtQuick 2.15
import QtQuick.Controls 2.15

import "../themes/BlueTheme.js" as BlueTheme

/**
 * PipPlaceholder.qml - Placeholder displayed when video is in PiP mode
 * 
 * Shows a visual indicator that the video is playing in a floating window,
 * with a button to bring it back to the main window.
 */
Rectangle {
    id: root

    // ─────────────────────────────────────────────────────────────────────────
    // Signals
    // ─────────────────────────────────────────────────────────────────────────
    
    /** Emitted when user clicks the "Return here" button */
    signal returnRequested()

    // ─────────────────────────────────────────────────────────────────────────
    // Appearance
    // ─────────────────────────────────────────────────────────────────────────
    
    color: BlueTheme.windowBackground
    
    // Fade in animation
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity {
        NumberAnimation { 
            duration: BlueTheme.animPanelDuration
            easing.type: Easing.OutCubic 
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Content
    // ─────────────────────────────────────────────────────────────────────────
    
    Column {
        anchors.centerIn: parent
        spacing: BlueTheme.spacingLarge
        
        // PiP Icon
        Rectangle {
            id: iconContainer
            anchors.horizontalCenter: parent.horizontalCenter
            width: 100
            height: 100
            radius: 50
            color: BlueTheme.surface
            border.color: BlueTheme.divider
            border.width: 1
            
            // PiP icon using Canvas
            Canvas {
                anchors.centerIn: parent
                width: 48
                height: 48
                
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.reset()
                    ctx.strokeStyle = BlueTheme.accent
                    ctx.fillStyle = BlueTheme.accent
                    ctx.lineWidth = 2
                    ctx.lineCap = "round"
                    ctx.lineJoin = "round"
                    
                    // Main window outline
                    ctx.beginPath()
                    ctx.rect(2, 2, 44, 28)
                    ctx.stroke()
                    
                    // Small PiP window (filled)
                    ctx.fillRect(24, 14, 20, 14)
                }
            }
            
            // Subtle pulse animation
            SequentialAnimation on scale {
                running: root.visible
                loops: Animation.Infinite
                NumberAnimation { to: 1.02; duration: 1500; easing.type: Easing.InOutSine }
                NumberAnimation { to: 1.0; duration: 1500; easing.type: Easing.InOutSine }
            }
        }
        
        // Title
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Video in Picture-in-Picture")
            color: BlueTheme.primaryText
            font.pixelSize: 20
            font.family: BlueTheme.fontFamily
            font.weight: Font.DemiBold
        }
        
        // Subtitle
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("The video is playing in a floating window")
            color: BlueTheme.secondaryText
            font.pixelSize: 14
            font.family: BlueTheme.fontFamily
        }
        
        // Spacer
        Item { width: 1; height: BlueTheme.spacingSmall }
        
        // Return Button
        Rectangle {
            id: returnButton
            anchors.horizontalCenter: parent.horizontalCenter
            width: returnRow.width + 32
            height: 44
            radius: 22
            color: returnMouseArea.containsMouse ? BlueTheme.accent : BlueTheme.surface
            border.color: returnMouseArea.containsMouse ? BlueTheme.accent : BlueTheme.buttonBorder
            border.width: 1
            
            Behavior on color {
                ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
            }
            
            Behavior on border.color {
                ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
            }
            
            Row {
                id: returnRow
                anchors.centerIn: parent
                spacing: 8
                
                Text {
                    text: "\u2199"  // ↙
                    color: returnMouseArea.containsMouse ? "#FFFFFF" : BlueTheme.primaryText
                    font.pixelSize: 16
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Behavior on color {
                        ColorAnimation { duration: BlueTheme.animHoverDuration }
                    }
                }
                
                Text {
                    text: qsTr("Return here")
                    color: returnMouseArea.containsMouse ? "#FFFFFF" : BlueTheme.primaryText
                    font.pixelSize: 14
                    font.family: BlueTheme.fontFamily
                    font.weight: Font.Medium
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Behavior on color {
                        ColorAnimation { duration: BlueTheme.animHoverDuration }
                    }
                }
            }
            
            MouseArea {
                id: returnMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: root.returnRequested()
            }
            
            // Scale animation on hover
            scale: returnMouseArea.pressed ? 0.96 : (returnMouseArea.containsMouse ? 1.02 : 1.0)
            Behavior on scale {
                NumberAnimation { duration: BlueTheme.animPressDuration; easing.type: Easing.OutQuart }
            }
        }
        
        // Keyboard shortcut hint
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Press P or Escape to return")
            color: BlueTheme.mutedText
            font.pixelSize: 12
            font.family: BlueTheme.fontFamily
            topPadding: BlueTheme.spacingSmall
        }
    }
}
