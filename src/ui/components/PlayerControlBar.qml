import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15

import "../themes/AppleTheme.js" as AppleTheme

/**
 * PlayerControlBar - Modern streaming player controls
 */
Rectangle {
  id: controlBar
  
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
  property real playbackRate: 1.0
  property bool hardwareDecoding: true
  property bool cropVideo: false
  
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
    NumberAnimation { duration: 300; easing.type: Easing.InOutCubic }
  }
  
  // Main content
  RowLayout {
    anchors.fill: parent
    anchors.leftMargin: 24
    anchors.rightMargin: 24
    anchors.bottomMargin: 16
    anchors.topMargin: 24
    spacing: 16

    // Play/Pause Button
    Item {
      id: playPauseButton
      Layout.preferredWidth: 44
      Layout.preferredHeight: 44

      Rectangle {
        id: playPauseBg
        anchors.fill: parent
        radius: 22
        color: playPauseMouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF"
        border.color: "#4DFFFFFF"
        border.width: 1

        Behavior on color {
          ColorAnimation { duration: 150 }
        }

        scale: playPauseMouseArea.pressed ? 0.92 : (playPauseMouseArea.containsMouse ? 1.05 : 1.0)
        Behavior on scale {
          NumberAnimation { duration: 100; easing.type: Easing.OutQuad }
        }
      }

      // Play/Pause Icon container
      Item {
        anchors.centerIn: parent
        width: 24
        height: 24

        // Play triangle - using Unicode character
        Text {
          anchors.centerIn: parent
          visible: !buffering && (paused || !playing)
          text: "\u25B6"  // Unicode play triangle
          font.pixelSize: 18
          color: "#FFFFFF"
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter
        }

        // Pause bars
        Row {
          anchors.centerIn: parent
          spacing: 4
          visible: !buffering && playing && !paused

          Rectangle {
            width: 4
            height: 14
            radius: 1
            color: "#FFFFFF"
          }
          Rectangle {
            width: 4
            height: 14
            radius: 1
            color: "#FFFFFF"
          }
        }

        // Loading indicator
        Text {
          anchors.centerIn: parent
          visible: buffering
          text: "..."
          font.pixelSize: 14
          font.bold: true
          color: "#FFFFFF"
        }
      }

      MouseArea {
        id: playPauseMouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: controlBar.playPauseClicked()
      }

      ToolTip.visible: playPauseMouseArea.containsMouse
      ToolTip.text: buffering ? qsTr("Loading...") : (playing && !paused ? qsTr("Pause (Space)") : qsTr("Play (Space)"))
      ToolTip.delay: 800
    }

    // Spacer
    Item { Layout.fillWidth: true }

    // Seek area - YouTube-style DVR behavior
    ColumnLayout {
      Layout.fillWidth: true
      spacing: 6

      // Slider + Live button row
      RowLayout {
        Layout.fillWidth: true
        spacing: 12

        // Seek slider - Live mode: ALWAYS at 100%, VOD mode: follows position
        Slider {
          id: seekSlider
          Layout.fillWidth: true
          enabled: currentDuration > 0
          hoverEnabled: true
          from: 0
          to: currentDuration > 0 ? currentDuration : 1

          // Properties for easier access
          property bool isLiveMode: controlBar.liveMode
          property real currentDuration: controlBar.duration
          property bool userDragging: false
          property real seekTarget: 0

          // In live mode: always at live edge (UI only)
          readonly property bool atLiveEdge: isLiveMode || (currentDuration > 0 && (currentDuration - controlBar.position) <= 5)

          // SIMPLE LOGIC:
          // - Live mode: slider always at 100% (right edge)
          // - VOD mode: slider follows actual position
          Binding {
              target: seekSlider
              property: "value"
              value: seekSlider.isLiveMode ? seekSlider.to : controlBar.position
              when: !seekSlider.userDragging && !seekSlider.pressed
          }

          onPressedChanged: {
            if (pressed) {
              userDragging = true
              controlBar.seekDragStarted()
              // In live mode, start seek from current position
              seekTarget = isLiveMode ? currentDuration : controlBar.position
            } else if (userDragging) {
              userDragging = false
              controlBar.seekDragEnded(seekTarget)
              
              // If user seeked to a position (not at the very end), do the seek
              // The backend will handle switching to VOD mode if needed
              controlBar.seekRequested(seekTarget)
            }
          }

          onMoved: {
            if (userDragging || pressed) {
              seekTarget = value
              controlBar.seekPreviewed(value)
            }
          }

          background: Rectangle {
            x: seekSlider.leftPadding
            y: seekSlider.topPadding + seekSlider.availableHeight / 2 - height / 2
            width: seekSlider.availableWidth
            height: 6
            radius: 3
            color: "#26FFFFFF"

            // Buffered/available area (full width = all cached content)
            Rectangle {
              width: parent.width
              height: parent.height
              radius: parent.radius
              color: "#44FFFFFF"
            }

            // Played progress
            Rectangle {
              width: seekSlider.visualPosition * parent.width
              height: parent.height
              radius: parent.radius
              gradient: Gradient {
                GradientStop { position: 0.0; color: AppleTheme.accent }
                GradientStop { position: 1.0; color: AppleTheme.accentSubtle }
              }
              opacity: 0.9
            }

            // Hover highlight
            Rectangle {
              anchors.fill: parent
              radius: parent.radius
              color: AppleTheme.accent
              opacity: (seekSlider.pressed || seekSlider.userDragging) ? 0.10 : (seekSlider.hovered ? 0.06 : 0.0)
              Behavior on opacity { NumberAnimation { duration: 100 } }
            }
          }

          handle: Item {
            x: seekSlider.leftPadding + seekSlider.visualPosition * (seekSlider.availableWidth - width)
            y: seekSlider.topPadding + seekSlider.availableHeight / 2 - height / 2
            width: 18
            height: 18

            // Hover/drag halo
            Rectangle {
              anchors.centerIn: parent
              width: parent.width + 8
              height: parent.height + 8
              radius: width / 2
              color: AppleTheme.accent
              opacity: (seekSlider.pressed || seekSlider.userDragging) ? 0.16 : (seekSlider.hovered ? 0.10 : 0.0)
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
              border.color: seekSlider.hovered || seekSlider.pressed || seekSlider.userDragging ? AppleTheme.accent : "#B3FFFFFF"
              border.width: 1
              opacity: seekSlider.enabled ? 1.0 : 0.6
              scale: seekSlider.pressed ? 1.15 : (seekSlider.hovered ? 1.08 : 1.0)
              Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutCubic } }
              Behavior on border.color { ColorAnimation { duration: 120 } }
            }
          }
        }
        
        // LIVE/VOD indicator - Fixed size, only color changes
        Rectangle {
          id: livePill
          width: 56
          height: 24
          radius: 12
          color: seekSlider.atLiveEdge ? "#FF3B30" : "#2C2C2E"
          border.color: seekSlider.atLiveEdge ? "#FF6961" : "#48484A"
          border.width: 1
          
          Behavior on color { ColorAnimation { duration: 200 } }
          Behavior on border.color { ColorAnimation { duration: 200 } }
          
          Row {
            anchors.centerIn: parent
            spacing: 6
            
            // Pulsing dot
            Rectangle {
              width: 8; height: 8; radius: 4
              anchors.verticalCenter: parent.verticalCenter
              color: seekSlider.atLiveEdge ? "#FFFFFF" : "#8E8E93"
              
              SequentialAnimation on opacity {
                running: seekSlider.atLiveEdge
                loops: Animation.Infinite
                NumberAnimation { to: 0.5; duration: 600 }
                NumberAnimation { to: 1.0; duration: 600 }
              }
            }
            
            Text {
              text: seekSlider.atLiveEdge ? qsTr("LIVE") : qsTr("VOD")
              font.pixelSize: 11
              font.weight: Font.DemiBold
              color: "#FFFFFF"
              anchors.verticalCenter: parent.verticalCenter
            }
          }
          
          MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: seekSlider.atLiveEdge ? Qt.ArrowCursor : Qt.PointingHandCursor
            onClicked: {
              if (!seekSlider.atLiveEdge) {
                controlBar.liveClicked()
              }
            }
            
            ToolTip.visible: containsMouse && !seekSlider.atLiveEdge
            ToolTip.text: qsTr("Retour au direct")
            ToolTip.delay: 400
          }
        }
      }

      // Time labels - YouTube style
      RowLayout {
        Layout.fillWidth: true
        spacing: 8

        // Current position
        Text {
          text: controlBar._formatTime(position)
          color: "#FFFFFF"
          font.pixelSize: 12
          font.family: "SF Mono, Menlo, monospace"
        }
        
        Item { Layout.fillWidth: true }
        
        // Live offset or LIVE badge - Show offset when not at live edge in live mode
        Text {
          visible: controlBar.liveMode && !seekSlider.atLiveEdge && liveOffset > 0
          text: qsTr("-%1").arg(controlBar._formatTime(liveOffset))
          color: "#FFEB3B"
          font.pixelSize: 12
          font.family: "SF Mono, Menlo, monospace"
        }
        
        // Duration (DVR buffer end)
        Text {
          text: "/ " + (duration > 0 ? controlBar._formatTime(duration) : "--:--")
          color: "#99FFFFFF"
          font.pixelSize: 12
          font.family: "SF Mono, Menlo, monospace"
        }
      }
    }

    // Spacer
    Item { Layout.fillWidth: true }

    // Playback / video toggles cluster
    RowLayout {
      spacing: 6
      Layout.alignment: Qt.AlignVCenter

      Rectangle {
        id: rateDown
        width: 32; height: 32; radius: 16
        color: "#1AFFFFFF"
        Text { anchors.centerIn: parent; text: "\u2212"; color: "#FFFFFF"; font.pixelSize: 14 }
        MouseArea {
          anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
          onClicked: controlBar.playbackRateRequested(Math.max(0.25, playbackRate - 0.1))
        }
      }

      Rectangle {
        id: rateChip
        width: 70; height: 32; radius: 16
        color: "#26FFFFFF"
        border.color: "#4DFFFFFF"; border.width: 1
        Text {
          anchors.centerIn: parent
          text: playbackRate.toFixed(2) + "x"
          color: "#FFFFFF"
          font.pixelSize: 12
          font.bold: true
        }
        MouseArea {
          anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
          onClicked: controlBar.playbackRateRequested(1.0)
          ToolTip.visible: containsMouse
          ToolTip.text: qsTr("Réinitialiser la vitesse (R)")
        }
      }

      Rectangle {
        id: rateUp
        width: 32; height: 32; radius: 16
        color: "#1AFFFFFF"
        Text { anchors.centerIn: parent; text: "+"; color: "#FFFFFF"; font.pixelSize: 14 }
        MouseArea {
          anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
          onClicked: controlBar.playbackRateRequested(Math.min(3.0, playbackRate + 0.1))
        }
      }

      Rectangle {
        id: hwToggle
        width: 52; height: 32; radius: 16
        color: hardwareDecoding ? "#3349c7ff" : "#1AFFFFFF"
        border.color: hardwareDecoding ? "#49c7ff" : "#4DFFFFFF"
        Text {
          anchors.centerIn: parent
          text: hardwareDecoding ? qsTr("HW") : qsTr("SW")
          color: "#FFFFFF"
          font.pixelSize: 12
          font.bold: true
        }
        MouseArea {
          anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
          onClicked: controlBar.hardwareToggleClicked()
          ToolTip.visible: containsMouse
          ToolTip.text: hardwareDecoding ? qsTr("Décodage matériel") : qsTr("Décodage logiciel")
        }
      }

      Rectangle {
        id: cropToggle
        width: 68; height: 32; radius: 16
        color: cropVideo ? "#339C27B0" : "#1AFFFFFF"
        border.color: cropVideo ? "#9C27B0" : "#4DFFFFFF"
        Text {
          anchors.centerIn: parent
          text: cropVideo ? qsTr("Crop") : qsTr("Fit")
          color: "#FFFFFF"
          font.pixelSize: 12
          font.bold: true
        }
        MouseArea {
          anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
          onClicked: controlBar.cropToggleClicked()
          ToolTip.visible: containsMouse
          ToolTip.text: cropVideo ? qsTr("Rognage (panscan)") : qsTr("Adapter")
        }
      }
    }

    // Volume Control - Button with vertical popup slider
    Item {
      id: volumeControl
      Layout.preferredWidth: 40
      Layout.preferredHeight: 40

      // Volume Icon Button
      Item {
        id: volumeButton
        anchors.fill: parent

        Rectangle {
          anchors.fill: parent
          radius: 20
          color: volumeMouseArea.containsMouse ? "#26FFFFFF" : "transparent"

          Behavior on color {
            ColorAnimation { duration: 150 }
          }
        }

        // Volume Icon
        Canvas {
          id: volumeIcon
          anchors.centerIn: parent
          width: 22
          height: 22

          property real vol: muted ? 0 : volume
          onVolChanged: requestPaint()
          Component.onCompleted: requestPaint()

          onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.fillStyle = "#FFFFFF"
            ctx.strokeStyle = "#FFFFFF"
            ctx.lineWidth = 1.5
            ctx.lineCap = "round"

            // Speaker body
            ctx.beginPath()
            ctx.moveTo(3, 8)
            ctx.lineTo(7, 8)
            ctx.lineTo(12, 4)
            ctx.lineTo(12, 18)
            ctx.lineTo(7, 14)
            ctx.lineTo(3, 14)
            ctx.closePath()
            ctx.fill()

            if (vol === 0 || muted) {
              // X for muted
              ctx.beginPath()
              ctx.moveTo(15, 8)
              ctx.lineTo(20, 14)
              ctx.stroke()
              ctx.beginPath()
              ctx.moveTo(20, 8)
              ctx.lineTo(15, 14)
              ctx.stroke()
            } else {
              // Sound waves
              if (vol > 0) {
                ctx.beginPath()
                ctx.arc(12, 11, 4, -Math.PI/3, Math.PI/3, false)
                ctx.stroke()
              }
              if (vol > 0.5) {
                ctx.beginPath()
                ctx.arc(12, 11, 7, -Math.PI/3, Math.PI/3, false)
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
            controlBar.muteClicked()
            hideVolumeTimer.restart()
          }
          
          onEntered: {
            showVolumeSlider = true
            hideVolumeTimer.stop()
          }
        }

        ToolTip.visible: volumeMouseArea.containsMouse && !showVolumeSlider
        ToolTip.text: showVolumeSlider ? (muted ? qsTr("Unmute (M)") : qsTr("Mute (M)")) : qsTr("Volume")
        ToolTip.delay: 800
      }

      // Vertical Volume Slider Popup (appears above the button)
      Rectangle {
        id: volumeSliderPopup
        width: 40
        height: showVolumeSlider ? 120 : 0
        anchors.bottom: volumeButton.top
        anchors.bottomMargin: 8
        anchors.horizontalCenter: volumeButton.horizontalCenter
        radius: 20
        color: "#CC1C1C1E"
        border.color: "#4DFFFFFF"
        border.width: 1
        clip: true
        opacity: showVolumeSlider ? 1.0 : 0.0
        visible: height > 0

        Behavior on height {
          NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
        }
        Behavior on opacity {
          NumberAnimation { duration: 150 }
        }

        Slider {
          id: volumeSlider
          anchors.centerIn: parent
          orientation: Qt.Vertical
          height: 90
          width: 30
          from: 0.0
          to: 1.0
          value: muted ? 0 : volume

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
            width: 16
            height: 16
            radius: 8
            color: "#FFFFFF"

            scale: volumeSlider.pressed ? 1.2 : 1.0
            Behavior on scale {
              NumberAnimation { duration: 100 }
            }
          }

          onMoved: {
            if (muted && value > 0) {
              controlBar.muteClicked()
            }
            controlBar.volumeRequested(value)
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
    }

    // Spacer
    Item { Layout.fillWidth: true }

    // Fullscreen Button
    Item {
      id: fullscreenButton
      Layout.preferredWidth: 40
      Layout.preferredHeight: 40

      Rectangle {
          anchors.fill: parent
          radius: 20
          color: fsMouseArea.containsMouse ? "#26FFFFFF" : "transparent"
          Behavior on color { ColorAnimation { duration: 150 } }
      }

      Canvas {
          anchors.centerIn: parent
          width: 20
          height: 20
          onPaint: {
              var ctx = getContext("2d")
              ctx.reset()
              ctx.strokeStyle = "#FFFFFF"
              ctx.lineWidth = 2
              ctx.lineCap = "round"

              // Top Left
              ctx.beginPath(); ctx.moveTo(0, 6); ctx.lineTo(0,0); ctx.lineTo(6,0); ctx.stroke();
              // Top Right
              ctx.beginPath(); ctx.moveTo(14, 0); ctx.lineTo(20,0); ctx.lineTo(20,6); ctx.stroke();
              // Bottom Left
              ctx.beginPath(); ctx.moveTo(0, 14); ctx.lineTo(0,20); ctx.lineTo(6,20); ctx.stroke();
              // Bottom Right
              ctx.beginPath(); ctx.moveTo(14, 20); ctx.lineTo(20,20); ctx.lineTo(20,14); ctx.stroke();
          }
      }

      MouseArea {
          id: fsMouseArea
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: Qt.PointingHandCursor
          onClicked: controlBar.fullscreenClicked()
      }
    }
  }

  // Timer to hide volume slider
  Timer {
    id: hideVolumeTimer
    interval: 1500
    onTriggered: showVolumeSlider = false
  }
  
  // Hover area for volume section
  MouseArea {
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    width: 80
    hoverEnabled: true
    propagateComposedEvents: true
    onEntered: {
      showVolumeSlider = true
      hideVolumeTimer.stop()
    }
    onExited: hideVolumeTimer.restart()
    onPressed: function(mouse) { mouse.accepted = false }
    onReleased: function(mouse) { mouse.accepted = false }
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
