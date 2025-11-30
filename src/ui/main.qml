import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts 1.3
import QtMultimedia 6.5

ApplicationWindow {
  id: root
  visible: true
  width: 960
  height: 540
  title: qsTr("BluePlayer – Lecture locale")
  color: "#0f1113"

  property string statusText: qsTr("Sélectionnez une vidéo pour démarrer.")

  VideoOutput {
    id: videoOutput
    anchors.fill: parent
    Component.onCompleted: ffmpegService.videoSink = videoSink
  }

  Rectangle {
    id: overlay
    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      margins: 16
    }
    height: 140
    radius: 12
    color: "#000000a0"

    Column {
      anchors.fill: parent
      anchors.margins: 12
      spacing: 8

      RowLayout {
        spacing: 6

        TextField {
          id: pathField
          placeholderText: qsTr("file:///Users/.../video.mp4")
          cursorPosition: text.length
          Layout.fillWidth: true
          font.pointSize: 12
          color: "white"
        }

        Button {
          text: qsTr("Ouvrir")
          onClicked: fileDialog.open()
        }
      }

      Row {
        spacing: 6

        Button {
          text: qsTr("Lecture")
          enabled: pathField.text.length > 0
          onClicked: ffmpegService.play(pathField.text)
        }

        Button {
          text: qsTr("Stop")
          onClicked: ffmpegService.stop()
        }
      }

      Label {
        text: statusText
        color: "#f0f0f0"
        font.pixelSize: 12
      }
    }
  }

  FileDialog {
    id: fileDialog
    title: qsTr("Choisir une vidéo locale")
    fileMode: FileDialog.OpenFile
    nameFilters: [qsTr("Vidéos (*.mp4 *.mkv *.mov *.avi)"), qsTr("Tous les fichiers (*)")]
    onAccepted: pathField.text = selectedFile
  }

  Connections {
    target: ffmpegService
    function onErrorOccurred(message) {
      statusText = message
    }
    function onPlayingChanged(playing) {
      statusText = playing ? qsTr("Lecture en cours…") : qsTr("Lecture arrêtée.")
    }
  }
}


