import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 6.5
import QtMultimedia 6.5

import "themes/BlueTheme.js" as BlueTheme

Item {
  id: playerRoot
  width: 640
  height: 420

  property var mediaService: ffmpegService
  property bool playing: false
  property string mediaFilePath: ""
  property string statusText: qsTr("Sélectionnez une vidéo locale pour démarrer.")
  property string friendlyFileName: mediaFilePath.length > 0
      ? mediaFilePath.substr(mediaFilePath.lastIndexOf("/") + 1)
      : qsTr("Aucun fichier sélectionné")
  signal statusChanged(string message)

  function updateStatus(message) {
    statusText = message
    statusChanged(message)
  }

  function requestPlay() {
    if (!mediaFilePath) {
      fileDialog.open()
      return
    }
    if (!mediaService) {
      updateStatus(qsTr("Service média indisponible."))
      return
    }
    mediaService.playFile(mediaFilePath)
  }

  function requestStop() {
    if (mediaService) {
      mediaService.stop()
    }
  }

  Rectangle {
    anchors.fill: parent
    radius: BlueTheme.cornerRadius
    color: BlueTheme.surfaceSoft
    border.color: BlueTheme.divider
    border.width: BlueTheme.borderWidth
    gradient: Gradient {
      GradientStop { position: 0; color: BlueTheme.overlayTint }
      GradientStop { position: 1; color: BlueTheme.surfaceSoft }
    }

    ColumnLayout {
      anchors.fill: parent
      anchors.margins: 18
      spacing: BlueTheme.spacing

      Rectangle {
        id: videoStage
        Layout.fillWidth: true
        Layout.preferredHeight: 240
        radius: BlueTheme.cornerRadius
        clip: true
        color: "#05070f"

        VideoOutput {
          id: videoOutput
          anchors.fill: parent
          fillMode: VideoOutput.PreserveAspectCrop
        }

        Rectangle {
          anchors.fill: parent
          gradient: Gradient {
            GradientStop { position: 0; color: "#00000000" }
            GradientStop { position: 1; color: "#040b1a" }
          }
        }
      }

      RowLayout {
        Layout.fillWidth: true
        spacing: 12

        ColumnLayout {
          Layout.fillWidth: true

          Label {
            text: friendlyFileName
            color: BlueTheme.primaryText
            font.pixelSize: 16
            font.bold: true
            elide: Text.ElideRight
          }

          Label {
            text: mediaFilePath
                  ? mediaFilePath
                  : qsTr("Parcourez vos dossiers pour sélectionner une vidéo.")
            color: BlueTheme.mutedText
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            elide: Text.ElideLeft
          }
        }

        Button {
          text: qsTr("Parcourir")
          onClicked: fileDialog.open()
          background: Rectangle {
            radius: 12
            border.color: BlueTheme.divider
            border.width: 1
            color: BlueTheme.surface
          }
        }
      }

      RowLayout {
        Layout.fillWidth: true
        spacing: 12

        Button {
          id: playButton
          text: qsTr("Lecture locale")
          enabled: mediaFilePath.length > 0
          Layout.preferredWidth: 160
          onClicked: requestPlay()
          background: Rectangle {
            radius: 12
            border.color: BlueTheme.accentSubtle
            border.width: 1
            color: playButton.enabled ? BlueTheme.accent : BlueTheme.overlayTint
          }
          contentItem: Label {
            text: playButton.text
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
          }
        }

        Button {
          text: qsTr("Stop")
          Layout.preferredWidth: 120
          onClicked: requestStop()
          background: Rectangle {
            radius: 12
            border.color: BlueTheme.divider
            border.width: 1
            color: BlueTheme.surface
          }
        }

        Label {
          id: statusLabel
          text: statusText
          color: BlueTheme.secondaryText
          font.pixelSize: 12
          elide: Text.ElideMiddle
          Layout.alignment: Qt.AlignVCenter
          Layout.fillWidth: true
        }
      }

      Slider {
        Layout.fillWidth: true
        from: 0
        to: 100
        value: playing ? 72 : 0
        interactive: false
        background: Rectangle {
          radius: 10
          color: BlueTheme.divider
        }
      }
    }
  }

  FileDialog {
    id: fileDialog
    title: qsTr("Choisir une vidéo locale")
    fileMode: FileDialog.OpenFile
    nameFilters: [qsTr("Vidéos (*.mp4 *.mkv *.mov *.avi *.m4v)"), qsTr("Tous les fichiers (*)")]
    onAccepted: {
      mediaFilePath = selectedFile
      updateStatus(qsTr("Prêt à lancer %1").arg(friendlyFileName))
    }
  }

  Component.onCompleted: {
    if (mediaService && videoOutput) {
      mediaService.videoSink = videoOutput
    }
  }

  Connections {
    target: mediaService
    function onPlayingChanged(isPlaying) {
      playing = isPlaying
      if (isPlaying) {
        updateStatus(qsTr("Lecture en cours…"))
      } else {
        updateStatus(qsTr("Lecture arrêtée."))
      }
    }
    function onErrorOccurred(message) {
      updateStatus(message)
    }
  }
}

