import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Dialogs 6.5
import QtQuick.Layouts 1.15
import QtMultimedia 6.5

import "../themes/BlueTheme.js" as BlueTheme

BlueCard {
  id: root
  property var ffmpegService
  property string selectedPath: ""
  property string statusText: qsTr("Select a video to start.")
  signal statusChanged(string message)

  function updateStatus(message) {
    statusText = message
    statusChanged(message)
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: BlueTheme.spacingMedium

    Rectangle {
      Layout.fillWidth: true
      Layout.preferredHeight: 200
      radius: BlueTheme.heroCornerRadius
      color: BlueTheme.surface
      border.color: BlueTheme.divider
      border.width: 1

      VideoOutput {
        id: videoOutput
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
      }

      Component.onCompleted: {
        if (ffmpegService && videoOutput) {
          ffmpegService.videoSink = videoOutput
        }
      }
    }

    RowLayout {
      Layout.fillWidth: true
      spacing: BlueTheme.spacingSmall

      TextField {
        id: pathField
        Layout.fillWidth: true
        placeholderText: qsTr("file:///Users/.../video.mp4")
        text: selectedPath
        readOnly: true
        font.family: BlueTheme.fontFamily
        font.pixelSize: 12
      }

      Button {
        text: qsTr("Browse")
        onClicked: fileDialog.open()
        background: Rectangle {
          radius: 12
          border.color: BlueTheme.buttonBorder
          border.width: 1
          color: BlueTheme.buttonSurface
        }
      }
    }

    RowLayout {
      Layout.fillWidth: true
      spacing: BlueTheme.spacingMedium

      Button {
        text: qsTr("Local playback")
        enabled: selectedPath.length > 0
        Layout.preferredWidth: 160
        onClicked: {
          if (ffmpegService) {
            ffmpegService.playFile(selectedPath)
          }
        }
        background: Rectangle {
          radius: 12
          border.color: BlueTheme.buttonBorder
          border.width: 1
          color: enabled ? BlueTheme.accent : BlueTheme.overlayTint
        }
      }

      Button {
        text: qsTr("Stop")
        Layout.preferredWidth: 120
        onClicked: {
          if (ffmpegService) {
            ffmpegService.stop()
          }
        }
        background: Rectangle {
          radius: 12
          border.color: BlueTheme.buttonBorder
          border.width: 1
          color: BlueTheme.surface
        }
      }

      Label {
        text: statusText
        color: BlueTheme.secondaryText
        font.pixelSize: 12
        horizontalAlignment: Text.AlignLeft
        Layout.fillWidth: true
      }
    }
  }

  FileDialog {
    id: fileDialog
    title: qsTr("Choose a local video")
    fileMode: FileDialog.OpenFile
    nameFilters: [qsTr("Videos (*.mp4 *.mkv *.mov *.avi *.m4v)"), qsTr("All files (*)")]
    onAccepted: {
      selectedPath = selectedFile
      updateStatus(qsTr("Ready to play %1").arg(selectedPath.split("/").pop()))
    }
  }

  QtObject {
    id: placeholderService
  }

  Connections {
    target: ffmpegService !== undefined ? ffmpegService : placeholderService
    ignoreUnknownSignals: true
    function onPlayingChanged(playing) {
      updateStatus(playing ? qsTr("Playing...") : qsTr("Playback stopped."))
    }
    function onErrorOccurred(message) {
      updateStatus(message)
    }
  }
}

