import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtMultimedia 6.5

import "themes/AppleTheme.js" as AppleTheme

Item {
  id: playerRoot
  
  property var twitchService: null
  property var mediaService: ffmpegService
  property string streamerLogin: ""
  property string streamerName: ""
  property string streamTitle: ""
  property bool playing: false
  property string statusText: qsTr("Chargement du flux...")
  property string hlsUrl: ""
  property bool adsActive: false
  property int adSegments: 0
  
  signal backRequested()
  
  function updateStatus(message) {
    statusText = message
  }
  
  function loadStream() {
    console.log("[PlayerView] loadStream() called")
    console.log("[PlayerView] streamerLogin:", streamerLogin)
    console.log("[PlayerView] streamerName:", streamerName)
    console.log("[PlayerView] streamTitle:", streamTitle)
    console.log("[PlayerView] twitchService:", twitchService ? "exists" : "null")
    
    if (!streamerLogin) {
      console.log("[PlayerView] ERROR: No streamerLogin provided")
      updateStatus(qsTr("Aucun stream sélectionné"))
      return
    }
    
    updateStatus(qsTr("Récupération de l'URL du flux..."))
    console.log("[PlayerView] Calling twitchService.getStreamHlsUrl() with login:", streamerLogin)
    
    // Obtenir l'URL HLS via le service Twitch
    if (twitchService) {
      twitchService.getStreamHlsUrl(streamerLogin)
    } else {
      console.log("[PlayerView] ERROR: twitchService is null")
      updateStatus(qsTr("Service Twitch indisponible"))
    }
  }
  
  function requestStop() {
    if (mediaService) {
      mediaService.stop()
    }
  }
  
  Rectangle {
    anchors.fill: parent
    color: "#000000"
    
    ColumnLayout {
      anchors.fill: parent
      spacing: 0
      
      // Barre de contrôle en haut avec bouton retour
      Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 60
        color: "#1a1a1a"
        z: 10
        
        RowLayout {
          anchors.fill: parent
          anchors.leftMargin: 16
          anchors.rightMargin: 16
          spacing: 16
          
          // Bouton retour
          Rectangle {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
            radius: 20
            color: backButtonMouseArea.containsMouse ? "#2a2a2a" : "#1a1a1a"
            border.color: AppleTheme.divider
            border.width: 1
            
            Text {
              anchors.centerIn: parent
              text: "←"
              font.pixelSize: 20
              color: AppleTheme.primaryText
            }
            
            MouseArea {
              id: backButtonMouseArea
              anchors.fill: parent
              hoverEnabled: true
              cursorShape: Qt.PointingHandCursor
              onClicked: {
                requestStop()
                playerRoot.backRequested()
              }
            }
          }
          
          // Informations du stream
          ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            
            Text {
              text: streamerName || streamerLogin
              font.family: AppleTheme.fontFamily
              font.pixelSize: 16
              font.bold: true
              color: AppleTheme.primaryText
              elide: Text.ElideRight
              Layout.fillWidth: true
            }
            
            Text {
              text: streamTitle || qsTr("Stream en direct")
              font.family: AppleTheme.fontFamily
              font.pixelSize: 12
              color: AppleTheme.secondaryText
              elide: Text.ElideRight
              Layout.fillWidth: true
            }
          }
          
          // Statut
          Text {
            text: statusText
            font.family: AppleTheme.fontFamily
            font.pixelSize: 12
            color: AppleTheme.mutedText
            Layout.alignment: Qt.AlignVCenter
          }
        }
      }
      
      // Zone vidéo
      Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "#000000"
        
        VideoOutput {
          id: videoOutput
          anchors.fill: parent
          fillMode: VideoOutput.PreserveAspectFit
        }
        
        // Overlay de chargement (seulement avant le début de la lecture)
        Rectangle {
          anchors.fill: parent
          color: "#000000"
          visible: !playing && (statusText.indexOf("Chargement") >= 0 || statusText.indexOf("Connexion") >= 0 || statusText.indexOf("Récupération") >= 0)
          
          ColumnLayout {
            anchors.centerIn: parent
            spacing: 16
            
            Text {
              text: "⏳"
              font.pixelSize: 48
              Layout.alignment: Qt.AlignHCenter
            }
            
            Text {
              text: statusText
              font.family: AppleTheme.fontFamily
              font.pixelSize: 14
              color: AppleTheme.primaryText
              Layout.alignment: Qt.AlignHCenter
            }
          }
        }
        
        // Badge pub discret dans le coin (visible pendant les pubs)
        Rectangle {
          visible: adsActive
          anchors.top: parent.top
          anchors.left: parent.left
          anchors.margins: 12
          width: adBadgeRow.width + 16
          height: 28
          radius: 6
          color: "#DD000000"
          border.color: "#FF9500"
          border.width: 1
          
          Row {
            id: adBadgeRow
            anchors.centerIn: parent
            spacing: 6
            
            Text {
              text: "📺"
              font.pixelSize: 12
            }
            
            Text {
              text: qsTr("Pub en cours...")
              font.family: AppleTheme.fontFamily
              font.pixelSize: 11
              font.bold: true
              color: "#FF9500"
            }
          }
        }
      }
    }
  }
  
  Component.onCompleted: {
    console.log("[PlayerView] Component.onCompleted called")
    if (mediaService && videoOutput) {
      mediaService.videoSink = videoOutput.videoSink
    }
    // Ne pas appeler loadStream() ici car les propriétés ne sont pas encore définies
    // loadStream() sera appelé quand streamerLogin sera défini
  }
  
  // Appeler loadStream() quand streamerLogin devient disponible
  onStreamerLoginChanged: {
    console.log("[PlayerView] onStreamerLoginChanged called, streamerLogin:", streamerLogin)
    if (streamerLogin && streamerLogin.length > 0) {
      loadStream()
    }
  }
  
  Connections {
    target: mediaService
    function onPlayingChanged(isPlaying) {
      playing = isPlaying
      if (isPlaying) {
        updateStatus(qsTr("Lecture en cours"))
      } else {
        if (hlsUrl.length > 0) {
          updateStatus(qsTr("Lecture arrêtée"))
        }
      }
    }
    function onErrorOccurred(message) {
      updateStatus(qsTr("Erreur: %1").arg(message))
    }
  }
  
  Connections {
    target: twitchService
    enabled: twitchService !== null
    function onHlsUrlReady(url) {
      console.log("[PlayerView] onHlsUrlReady() called with url length:", url ? url.length : 0)
      if (url && url.length > 0) {
        hlsUrl = url
        console.log("[PlayerView] HLS URL received:", url.substring(0, 100) + "...")
        updateStatus(adsActive ? qsTr("⚠️ Pubs détectées - Connexion...") : qsTr("Connexion au flux..."))
        if (mediaService) {
          console.log("[PlayerView] Calling mediaService.play() with URL")
          mediaService.play(Qt.resolvedUrl(url))
        } else {
          console.log("[PlayerView] ERROR: mediaService is null")
        }
      } else {
        console.log("[PlayerView] ERROR: Empty or invalid HLS URL")
        updateStatus(qsTr("Impossible de récupérer l'URL du flux"))
      }
    }
    function onErrorOccurred(message) {
      console.log("[PlayerView] Twitch error:", message)
      updateStatus(qsTr("Erreur Twitch: %1").arg(message))
    }
    function onAdsDetected(count) {
      console.log("[PlayerView] Ads detected:", count, "markers")
      adsActive = true
      adSegments = count
      // Ne pas changer le status - on continue la lecture normalement
    }
    function onAdsFinished() {
      console.log("[PlayerView] Ads finished")
      adsActive = false
      adSegments = 0
      // Le status sera mis à jour automatiquement par la lecture
    }
    function onAdFilterLog(message) {
      console.log("[PlayerView AdFilter]", message)
    }
  }
}

