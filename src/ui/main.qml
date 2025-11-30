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
    height: 360
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

      Rectangle {
        width: parent.width
        height: 220
        radius: 10
        color: "#1a1f28"
        Column {
          anchors.fill: parent
          anchors.margins: 8
          spacing: 6

          RowLayout {
            spacing: 6
            Layout.fillWidth: true
            Label {
              text: qsTr("Twitch")
              font.pixelSize: 16
              color: "white"
            }
            Label {
              text: twitchService.authenticated ? qsTr("Connecté") : qsTr("Déconnecté")
              font.pixelSize: 12
              color: twitchService.authenticated ? "#56f27a" : "#f27a7a"
            }
          }

          RowLayout {
            spacing: 6
            Button {
              text: qsTr("Connexion")
              onClicked: twitchService.login()
            }
            Button {
              text: qsTr("Déconnexion")
              enabled: twitchService.authenticated
              onClicked: twitchService.logout()
            }
            Button {
              text: qsTr("Actualiser")
              enabled: twitchService.authenticated
              onClicked: twitchService.refreshStreams()
            }
          }

          ListView {
            id: twitchList
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            model: twitchService.streams
            clip: true
            spacing: 6
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { }
            delegate: Rectangle {
              width: twitchList.width
              height: 60
              radius: 6
              color: index % 2 === 0 ? "#2a2f38" : "#22262f"
              RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8

                ColumnLayout {
                  Layout.fillWidth: true
                  Text {
                    text: model.user_name
                    font.bold: true
                    color: "white"
                  }
                  Text {
                    text: model.title
                    color: "#cfcfcf"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                  }
                }

                Text {
                  text: qsTr("%1 viewers").arg(model.viewer_count)
                  color: "#a0e2ff"
                  font.pixelSize: 12
                }
              }
              MouseArea {
                anchors.fill: parent
                onClicked: {
                  twitchService.playStream(index)
                  if (twitchService.selectedStreamUrl.length > 0) {
                    Qt.openUrlExternally(twitchService.selectedStreamUrl)
                  }
                }
              }
            }
          }
        }
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

  Connections {
    target: twitchService
    function onErrorOccurred(message) {
      statusText = message
    }
    function onAuthenticatedChanged(authenticated) {
      statusText = authenticated ? qsTr("Connecté à Twitch") : qsTr("Déconnecté de Twitch")
    }
  }
}


