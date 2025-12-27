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
  
  // Main content - Single row layout
  RowLayout {
    anchors.fill: parent
    anchors.leftMargin: 24
    anchors.rightMargin: 24
    anchors.bottomMargin: 20
    anchors.topMargin: 28
    spacing: 8

    // Play/Pause Button - Same height as other controls
    Rectangle {
      id: playPauseButton
      Layout.preferredWidth: chipHeight
      Layout.preferredHeight: chipHeight
      radius: chipRadius
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

    // Seek slider - Takes all available space
    Slider {
      id: seekSlider
      Layout.fillWidth: true
      Layout.preferredHeight: chipHeight
      enabled: currentDuration > 0
      hoverEnabled: true
      from: 0
      to: currentDuration > 0 ? currentDuration : 1

      // Properties for easier access
      property bool isLiveMode: controlBar.liveMode && !controlBar.isReplayMode  // Replay mode = never live
      property real currentDuration: controlBar.duration
      property bool userDragging: false
      property real seekTarget: 0

      // In live mode: always at live edge (UI only) - but NOT in replay mode
      readonly property bool atLiveEdge: isLiveMode || (!controlBar.isReplayMode && currentDuration > 0 && (currentDuration - controlBar.position) <= 5)

      // SIMPLE LOGIC:
      // - Live mode (and not replay): slider always at 100% (right edge)
      // - VOD/Replay mode: slider follows actual position from 0
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
          seekTarget = isLiveMode ? currentDuration : controlBar.position
        } else if (userDragging) {
          userDragging = false
          controlBar.seekDragEnded(seekTarget)
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

        // Buffered/available area
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
          opacity: (seekSlider.pressed || seekSlider.userDragging) ? 0.10 : (seekSlider.hovered ? 0.06 : 0.0)
          Behavior on opacity { NumberAnimation { duration: 100 } }
        }
      }

      handle: Item {
        x: seekSlider.leftPadding + seekSlider.visualPosition * (seekSlider.availableWidth - width)
        y: seekSlider.topPadding + seekSlider.availableHeight / 2 - height / 2
        width: 16
        height: 16

        // Hover/drag halo
        Rectangle {
          anchors.centerIn: parent
          width: parent.width + 6
          height: parent.height + 6
          radius: width / 2
          color: BlueTheme.accent
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
          border.color: seekSlider.hovered || seekSlider.pressed || seekSlider.userDragging ? BlueTheme.accent : "#B3FFFFFF"
          border.width: 1
          opacity: seekSlider.enabled ? 1.0 : 0.6
          scale: seekSlider.pressed ? 1.12 : (seekSlider.hovered ? 1.06 : 1.0)
          Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutCubic } }
          Behavior on border.color { ColorAnimation { duration: 120 } }
        }
      }
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
      
      Behavior on color { ColorAnimation { duration: 150 } }
      Behavior on border.color { ColorAnimation { duration: 200 } }
      
      // Pulsing dot centered
      Rectangle {
        anchors.centerIn: parent
        width: 8; height: 8; radius: 4
        color: seekSlider.atLiveEdge ? "#FFFFFF" : "#8E8E93"
        
        SequentialAnimation on opacity {
          running: seekSlider.atLiveEdge
          loops: Animation.Infinite
          NumberAnimation { to: 0.5; duration: 600 }
          NumberAnimation { to: 1.0; duration: 600 }
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
      ToolTip.text: seekSlider.atLiveEdge ? qsTr("Resynchroniser le direct") : qsTr("Retour au direct")
      ToolTip.delay: 800
    }

    // Time display - adapts to mode
    Column {
      Layout.alignment: Qt.AlignVCenter
      spacing: 2
      
      // En mode replay: toujours afficher position + durée
      // En mode live: afficher seulement la durée totale (temps de buffer)
      property bool isReplay: controlBar.isReplayMode
      property bool showTimers: isReplay || !seekSlider.atLiveEdge || controlBar.duration > 0
      
      // Current position (top) - seulement en mode replay ou quand pas au live edge
      Text {
        text: controlBar._formatTime(position)
        color: "#FFFFFF"
        font.pixelSize: 10
        font.family: "Menlo"
        visible: parent.isReplay || !seekSlider.atLiveEdge
        opacity: parent.showTimers ? 1.0 : 0.0
      }
      
      // Total duration - toujours visible, commence à "00:00"
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
      Rectangle {
        id: rateDown
        width: chipHeight; height: chipHeight; radius: chipRadius
        color: rateDownMouse.containsMouse ? "#33FFFFFF" : "#1AFFFFFF"
        Behavior on color { ColorAnimation { duration: 150 } }
        Text { anchors.centerIn: parent; text: "\u2212"; color: "#FFFFFF"; font.pixelSize: 14 }
        MouseArea {
          id: rateDownMouse
          anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
          onClicked: controlBar.playbackRateRequested(Math.max(0.25, playbackRate - 0.1))
        }
        ToolTip.visible: rateDownMouse.containsMouse
        ToolTip.text: qsTr("Ralentir")
        ToolTip.delay: 800
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
        ToolTip.text: qsTr("Réinitialiser la vitesse (R)")
        ToolTip.delay: 800
      }

      // Speed up
      Rectangle {
        id: rateUp
        width: chipHeight; height: chipHeight; radius: chipRadius
        color: rateUpMouse.containsMouse ? "#33FFFFFF" : "#1AFFFFFF"
        Behavior on color { ColorAnimation { duration: 150 } }
        Text { anchors.centerIn: parent; text: "+"; color: "#FFFFFF"; font.pixelSize: 14 }
        MouseArea {
          id: rateUpMouse
          anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
          onClicked: controlBar.playbackRateRequested(Math.min(3.0, playbackRate + 0.1))
        }
        ToolTip.visible: rateUpMouse.containsMouse
        ToolTip.text: qsTr("Accélérer")
        ToolTip.delay: 800
      }

      // HW toggle - compact
      Rectangle {
        id: hwToggle
        width: 44; height: chipHeight; radius: chipRadius
        color: hwMouse.containsMouse ? "#33FFFFFF" : "#26FFFFFF"
        border.color: "#4DFFFFFF"; border.width: 1
        Behavior on color { ColorAnimation { duration: 150 } }
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
        ToolTip.text: hardwareDecoding ? qsTr("Décodage matériel") : qsTr("Décodage logiciel")
        ToolTip.delay: 800
      }

      // Fit/Crop toggle - compact
      Rectangle {
        id: cropToggle
        width: 44; height: chipHeight; radius: chipRadius
        color: cropMouse.containsMouse ? "#33FFFFFF" : "#26FFFFFF"
        border.color: "#4DFFFFFF"; border.width: 1
        Behavior on color { ColorAnimation { duration: 150 } }
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
        ToolTip.text: cropVideo ? qsTr("Rognage (panscan) (V)") : qsTr("Adapter (V)")
        ToolTip.delay: 800
      }

      // Chat Toggle Button (hidden in VOD mode)
      Rectangle {
        id: chatButton
        visible: chatEnabled
        width: chipHeight; height: chipHeight; radius: chipRadius
        color: chatVisible ? BlueTheme.accent : (chatMouse.containsMouse ? "#33FFFFFF" : "#1AFFFFFF")
        border.color: chatVisible ? BlueTheme.accent : "#4DFFFFFF"
        border.width: 1
        Behavior on color { ColorAnimation { duration: 150 } }

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

        MouseArea {
          id: chatMouse
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: Qt.PointingHandCursor
          onClicked: controlBar.chatToggleClicked()
        }
        ToolTip.visible: chatMouse.containsMouse
        ToolTip.text: chatVisible ? qsTr("Hide chat (C)") : qsTr("Show chat (C)")
        ToolTip.delay: 800
      }

      // Volume Control - Button with vertical popup slider
      Item {
        id: volumeControl
        Layout.preferredWidth: chipHeight
        Layout.preferredHeight: chipHeight

        // Volume Icon Button
        Rectangle {
          id: volumeButton
          anchors.fill: parent
          radius: chipRadius
          color: volumeMouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF"
          border.color: "#4DFFFFFF"
          border.width: 1

          Behavior on color {
            ColorAnimation { duration: 150 }
          }

          // Volume Icon
          Canvas {
            id: volumeIcon
            anchors.centerIn: parent
            width: 18
            height: 18

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

              if (vol === 0 || muted) {
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
              controlBar.muteClicked()
              hideVolumeTimer.restart()
            }
            
            onEntered: {
              showVolumeSlider = true
              hideVolumeTimer.stop()
            }
          }

          ToolTip.visible: volumeMouseArea.containsMouse && !showVolumeSlider
          ToolTip.text: muted ? qsTr("Activer le son (M)") : qsTr("Couper le son (M)")
          ToolTip.delay: 800
        }

        // Vertical Volume Slider Popup
        Rectangle {
          id: volumeSliderPopup
          width: 36
          height: showVolumeSlider ? 110 : 0
          anchors.bottom: volumeButton.top
          anchors.bottomMargin: 8
          anchors.horizontalCenter: volumeButton.horizontalCenter
          radius: 18
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
            height: 85
            width: 26
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

      // Fullscreen Button
      Rectangle {
        id: fullscreenButton
        width: chipHeight; height: chipHeight; radius: chipRadius
        color: fsMouseArea.containsMouse ? "#33FFFFFF" : "#1AFFFFFF"
        border.color: "#4DFFFFFF"
        border.width: 1
        Behavior on color { ColorAnimation { duration: 150 } }

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

        MouseArea {
            id: fsMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: controlBar.fullscreenClicked()
        }

        ToolTip.visible: fsMouseArea.containsMouse
        ToolTip.text: qsTr("Plein écran (F)")
        ToolTip.delay: 800
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
