import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtMultimedia 6.5

ApplicationWindow {
  id: root
  visible: true
  width: 960
  height: 540
  title: qsTr("BluePlayer – Lecture locale")
  color: "#0f1113"

  property string statusText: qsTr("Sélectionnez une vidéo pour démarrer.")

  function openPreferencesDialog() {
    if (preferencesDialog) {
      preferencesDialog.open()
    }
  }

  Component {
    id: onboardingPage
    Rectangle {
      anchors.fill: parent
      color: "#050d17"

      ColumnLayout {
        anchors.centerIn: parent
        spacing: 18
        width: parent.width * 0.6

        Label {
          text: qsTr("Bienvenue sur BluePlayer")
          font.pixelSize: 28
          color: "white"
          horizontalAlignment: Text.AlignHCenter
          wrapMode: Text.WordWrap
          Layout.fillWidth: true
        }
        Label {
          text: qsTr("Connectez-vous avec Twitch pour accéder à vos streams, vos préférences et vos playlists.")
          font.pixelSize: 14
          color: "#8fa2c0"
          horizontalAlignment: Text.AlignHCenter
          wrapMode: Text.WordWrap
          Layout.fillWidth: true
        }
        Button {
          text: qsTr("Connexion Twitch")
          width: 220
          onClicked: twitchService.login()
          Layout.alignment: Qt.AlignHCenter
        }
        Label {
          text: statusText
          color: "#f0f0f0"
          font.pixelSize: 12
          horizontalAlignment: Text.AlignHCenter
          Layout.fillWidth: true
        }
      }
    }
  }

  Component {
    id: homePage
    Rectangle {
      anchors.fill: parent
      color: "#03070c"

      ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 18

        Rectangle {
          Layout.fillWidth: true
          height: 180
          radius: 12
          color: "#111722"

          ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            RowLayout {
              spacing: 8
              Layout.fillWidth: true

              TextField {
                id: pathField
                placeholderText: qsTr("file:///Users/.../video.mp4")
                Layout.fillWidth: true
                font.pointSize: 12
                color: "white"
              }
              Button {
                text: qsTr("Ouvrir")
                onClicked: fileDialog.open()
              }
            }

            RowLayout {
              spacing: 10

              Button {
                text: qsTr("Lecture locale")
                enabled: pathField.text.length > 0
                onClicked: ffmpegService.play(pathField.text)
              }
              Button {
                text: qsTr("Stop")
                onClicked: ffmpegService.stop()
              }
              Label {
                text: statusText
                color: "#c8d2e0"
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
        }

        Rectangle {
          Layout.fillWidth: true
          Layout.fillHeight: true
          radius: 10
          color: "#0c1624"

          ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            RowLayout {
              spacing: 12
              Layout.fillWidth: true

              Label {
                text: qsTr("Twitch")
                font.pixelSize: 18
                color: "white"
              }
              Label {
                text: twitchService.authenticated ? qsTr("Connecté") : qsTr("Déconnecté")
                font.pixelSize: 14
                color: twitchService.authenticated ? "#4ef57a" : "#f27a7a"
              }
              Button {
                text: qsTr("Préférences")
                onClicked: root.openPreferencesDialog()
              }
            }

            ListView {
              id: twitchList
              Layout.fillWidth: true
              Layout.fillHeight: true
              model: twitchService.streams
              clip: true
              spacing: 6
              boundsBehavior: Flickable.StopAtBounds
              ScrollBar.vertical: ScrollBar {}
              delegate: Rectangle {
                width: twitchList.width
                height: 60
                radius: 8
                color: index % 2 === 0 ? "#1f2534" : "#1b1f2a"

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
                    color: "#8fdfff"
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
  }

  Dialog {
    id: preferencesDialog
    title: qsTr("Préférences Twitch")
    modal: true
    standardButtons: Dialog.Ok
    width: 320

    ColumnLayout {
      anchors.fill: parent
      anchors.margins: 16
      spacing: 12

      Label {
        text: twitchService.authenticated ? qsTr("Connecté") : qsTr("Déconnecté")
        font.pixelSize: 16
        color: twitchService.authenticated ? "#4ef57a" : "#f27a7a"
      }

      Label {
        text: qsTr("Jetons stockés : %1")
                .arg(twitchService.authenticated ? qsTr("actifs") : qsTr("absents"))
        color: "#cfd6e0"
        wrapMode: Text.WordWrap
      }

      Button {
        text: qsTr("Déconnexion")
        enabled: twitchService.authenticated
        onClicked: twitchService.logout()
      }

      Button {
        text: qsTr("Actualiser les streams")
        enabled: twitchService.authenticated
        onClicked: twitchService.refreshStreams()
      }
    }
  }

  VideoOutput {
    id: videoOutput
    anchors.fill: parent
    z: 0
    Component.onCompleted: ffmpegService.videoSink = videoSink
  }

  Loader {
    id: pageLoader
    anchors.fill: parent
    z: 1
    sourceComponent: twitchService.authenticated ? homePage : onboardingPage
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
      pageLoader.sourceComponent = authenticated ? homePage : onboardingPage
    }
  }
}

