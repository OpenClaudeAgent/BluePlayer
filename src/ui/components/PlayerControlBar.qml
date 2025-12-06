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
  property bool stickToLive: true
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

    // Live/VOD toggle harmonized with other chips
    Rectangle {
      id: liveToggle
      Layout.preferredWidth: 70
      Layout.preferredHeight: 32
      radius: 16
      color: "#26FFFFFF"
      border.color: controlBar.liveMode ? "#FF7061" : "#4DFFFFFF"
      border.width: 1

      Row {
        anchors.centerIn: parent
        spacing: 8

        Rectangle {
          width: 10; height: 10; radius: 5
          color: controlBar.liveMode ? "#FF3B30" : "#8BC34A"
          anchors.verticalCenter: parent.verticalCenter
        }

        Text {
          anchors.verticalCenter: parent.verticalCenter
          text: controlBar.liveMode ? qsTr("LIVE") : qsTr("VOD")
          font.pixelSize: 12
          font.bold: true
          color: "#FFFFFF"
        }
      }

      MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: controlBar.liveClicked()
      }
    }

    // Spacer
    Item { Layout.fillWidth: true }

    // Seek area
    ColumnLayout {
      Layout.fillWidth: true
      spacing: 6

      // Slider + Live button row
      RowLayout {
        Layout.fillWidth: true
        spacing: 12

        // Seek slider
        Slider {
          id: seekSlider
          Layout.fillWidth: true
          enabled: (duration > 0) || (position > 0)
          from: 0
          to: duration > 0 ? duration : Math.max(position, 1)
          // Coller à droite tant qu'on n'a pas volontairement reculé, ou si on est quasi live
          readonly property bool atLiveEdge: liveOffset <= 3
          property bool userDragging: false
          property real seekTarget: 0

          // Mettre à jour la valeur affichée seulement si l'utilisateur ne drag pas
          value: userDragging ? value : ((stickToLive || atLiveEdge)
                   ? to
                   : (duration > 0
                        ? Math.max(0, duration - liveOffset)
                        : (position > 0 ? position : 0)))

          onPressedChanged: {
            if (pressed) {
              userDragging = true
              controlBar.stickToLive = false  // l'utilisateur prend la main
              console.log("[Seekbar] User started dragging, current value:", value, "to:", to)
            } else if (userDragging) {
              seekTarget = value
              userDragging = false
              console.log("[Seekbar] User released at value:", seekTarget, "duration:", duration, "position:", position)
              controlBar.seekRequested(seekTarget)
            }
          }
          onMoved: {
            if (userDragging) {
              console.log("[Seekbar] Slider moved to:", value)
            }
          }

          background: Rectangle {
            x: seekSlider.leftPadding
            y: seekSlider.topPadding + seekSlider.availableHeight / 2 - height / 2
            width: seekSlider.availableWidth
            height: 4
            radius: 2
            color: "#33FFFFFF"

            Rectangle {
              width: (seekSlider.visualPosition * parent.width)
              height: parent.height
              radius: 2
              color: "#FFFFFF"
            }
          }

          handle: Rectangle {
            x: seekSlider.leftPadding + seekSlider.visualPosition * (seekSlider.availableWidth - width)
            y: seekSlider.topPadding + seekSlider.availableHeight / 2 - height / 2
            width: 14
            height: 14
            radius: 7
            color: "#FFFFFF"
            scale: seekSlider.pressed ? 1.2 : 1.0
            Behavior on scale { NumberAnimation { duration: 100 } }
          }
        }
      }

      // Time labels
      RowLayout {
        Layout.fillWidth: true
        spacing: 8

        Text {
          text: controlBar._formatTime(position)
          color: "#FFFFFF"
          font.pixelSize: 12
        }
        Item { Layout.fillWidth: true }
        Text {
          text: liveOffset > 3 ? qsTr("−%1").arg(controlBar._formatTime(liveOffset)) : qsTr("LIVE")
          color: liveOffset > 3 ? "#FFEB3B" : "#FF5252"
          font.pixelSize: 12
          font.bold: liveOffset <= 3
        }
        Text {
          text: "/ " + (duration > 0 ? controlBar._formatTime(duration) : "--:--")
          color: "#DDFFFFFF"
          font.pixelSize: 12
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

    // Volume Control Group
    RowLayout {
      spacing: 4

      // Volume Icon Button
      Item {
        id: volumeButton
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40

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
          onClicked: controlBar.muteClicked()
          onEntered: showVolumeSlider = true
        }

        ToolTip.visible: volumeMouseArea.containsMouse && !showVolumeSlider
        ToolTip.text: muted ? qsTr("Unmute (M)") : qsTr("Mute (M)")
        ToolTip.delay: 800
      }

      // Volume Slider Container
      Item {
        id: volumeSliderContainer
        Layout.preferredWidth: showVolumeSlider ? 110 : 0
        Layout.preferredHeight: 40
        clip: true

        Behavior on Layout.preferredWidth {
          NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
        }

        // Slider background pill
        Rectangle {
          anchors.verticalCenter: parent.verticalCenter
          anchors.left: parent.left
          anchors.leftMargin: 4
          width: 100
          height: 32
          radius: 16
          color: "#1AFFFFFF"

          Slider {
            id: volumeSlider
            anchors.centerIn: parent
            width: 80
            from: 0.0
            to: 1.0
            value: muted ? 0 : volume

            background: Rectangle {
              x: volumeSlider.leftPadding
              y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
              width: volumeSlider.availableWidth
              height: 4
              radius: 2
              color: "#33FFFFFF"

              Rectangle {
                width: volumeSlider.visualPosition * parent.width
                height: parent.height
                radius: 2
                color: "#FFFFFF"
              }
            }

            handle: Rectangle {
              x: volumeSlider.leftPadding + volumeSlider.visualPosition * (volumeSlider.availableWidth - width)
              y: volumeSlider.topPadding + volumeSlider.availableHeight / 2 - height / 2
              width: 14
              height: 14
              radius: 7
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
    width: 180
    hoverEnabled: true
    propagateComposedEvents: true
    onEntered: hideVolumeTimer.stop()
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
