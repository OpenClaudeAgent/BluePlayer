import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import BluePlayer.Media 1.0
import "themes/AppleTheme.js" as AppleTheme
import "components"

Item {
  id: playerRoot
  
  property var twitchService: null
  property string streamerLogin: ""
  property string streamerName: ""
  property string streamTitle: ""
  property bool playing: false
  property bool paused: false
  property bool buffering: false
  property real volume: 1.0
  property bool muted: false
  property real duration: 0.0
  property real position: 0.0
  property real liveOffset: 0.0
  property bool liveMode: true
  property string statusText: qsTr("Chargement du flux...")
  property string hlsUrl: ""
  property bool adsActive: false
  property int adSegments: 0
  property bool controlsVisible: true
  
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
      updateStatus(qsTr("Aucun stream selectionne"))
      return
    }
    
    updateStatus(qsTr("Recuperation de l'URL du flux..."))
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
    mpvPlayer.stop()
  }
  
  function togglePlayPause() {
    mpvPlayer.togglePause()
  }
  
  function setVolume(vol) {
    mpvPlayer.volume = vol
  }
  
  function toggleMute() {
    mpvPlayer.muted = !mpvPlayer.muted
  }

  function seekTo(seconds) {
    console.log("[PlayerView] seekTo called with seconds:", seconds, "duration:", duration, "position:", position)
    if (seconds >= 0) {
      mpvPlayer.seek(seconds)
    }
  }

  function goLive() {
    mpvPlayer.seekToLive()
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
      
      // Zone video avec MpvQuickItem (rendu GPU direct)
      Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "#000000"
        
        // MpvQuickItem - rendu OpenGL direct sans copie CPU
        MpvQuickItem {
          id: mpvPlayer
          anchors.fill: parent
          
          onPlayingChanged: function(isPlaying) {
            playerRoot.playing = isPlaying
            if (isPlaying) {
              playerRoot.buffering = false
              playerRoot.updateStatus(qsTr("Lecture en cours"))
              playerRoot.forceActiveFocus()
            } else {
              if (playerRoot.hlsUrl.length > 0) {
                playerRoot.updateStatus(qsTr("Lecture arretee"))
              }
            }
          }
          
          onPausedChanged: function(isPaused) {
            playerRoot.paused = isPaused
            if (isPaused) {
              playerRoot.updateStatus(qsTr("Pause"))
              playerRoot.controlsVisible = true
            } else if (playerRoot.playing) {
              playerRoot.updateStatus(qsTr("Lecture en cours"))
            }
          }
          
          onVolumeChanged: function(newVolume) {
            playerRoot.volume = newVolume
          }
          
          onMutedChanged: function(isMuted) {
            playerRoot.muted = isMuted
          }
          
          onDurationChanged: function(dur) {
            playerRoot.duration = dur
          }
          
          onPositionChanged: function(pos) {
            playerRoot.position = pos
          }
          
          onLiveOffsetChanged: function(offset) {
            playerRoot.liveOffset = offset
          }
          
          onIsLiveModeChanged: function(isLive) {
            playerRoot.liveMode = isLive
          }
          
          onBufferingChanged: function(isBuffering) {
            playerRoot.buffering = isBuffering
          }
          
          onErrorOccurred: function(message) {
            playerRoot.updateStatus(qsTr("Erreur: %1").arg(message))
          }
        }
        
        // Overlay de chargement (seulement avant le debut de la lecture)
        Rectangle {
          anchors.fill: parent
          color: "#000000"
          visible: !playing && (statusText.indexOf("Chargement") >= 0 || statusText.indexOf("Connexion") >= 0 || statusText.indexOf("Recuperation") >= 0)
          
          ColumnLayout {
            anchors.centerIn: parent
            spacing: 16
            
            BusyIndicator {
              Layout.alignment: Qt.AlignHCenter
              running: true
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
              text: "AD"
              font.pixelSize: 10
              font.bold: true
              color: "#FF9500"
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
        
        // Barre de controle en bas
        PlayerControlBar {
          id: playerControlBar
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          
          playing: playerRoot.playing
          paused: playerRoot.paused
          buffering: playerRoot.buffering
          volume: playerRoot.volume
          muted: playerRoot.muted
          duration: playerRoot.duration
          position: playerRoot.position
          liveOffset: playerRoot.liveOffset
          liveMode: playerRoot.liveMode
          controlsVisible: playerRoot.controlsVisible
          
          onPlayPauseClicked: togglePlayPause()
          onStopClicked: {
            requestStop()
            playerRoot.backRequested()
          }
          onVolumeRequested: function(newVolume) { setVolume(newVolume) }
          onMuteClicked: toggleMute()
          onSeekRequested: function(seconds) { seekTo(seconds) }
          onLiveRequested: goLive()
          onLiveClicked: goLive()
        }
        
        // Zone de detection de souris pour afficher/masquer les controles
        MouseArea {
          anchors.fill: parent
          hoverEnabled: true
          propagateComposedEvents: true
          
          onPositionChanged: {
            controlsVisible = true
            hideControlsTimer.restart()
          }
          
          onPressed: function(mouse) { mouse.accepted = false }
          onReleased: function(mouse) { mouse.accepted = false }
          onClicked: function(mouse) { mouse.accepted = false }
        }
        
        // Timer pour masquer les controles
        Timer {
          id: hideControlsTimer
          interval: 3000
          onTriggered: {
            if (playing && !paused) {
              controlsVisible = false
            }
          }
        }
      }
    }
  }
  
  // Gestion des raccourcis clavier
  Keys.onPressed: function(event) {
    switch (event.key) {
      case Qt.Key_Space:
        togglePlayPause()
        event.accepted = true
        break
      case Qt.Key_M:
        toggleMute()
        event.accepted = true
        break
      case Qt.Key_Up:
        setVolume(Math.min(1.0, volume + 0.1))
        event.accepted = true
        break
      case Qt.Key_Down:
        setVolume(Math.max(0.0, volume - 0.1))
        event.accepted = true
        break
      case Qt.Key_Escape:
        requestStop()
        playerRoot.backRequested()
        event.accepted = true
        break
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
        updateStatus(adsActive ? qsTr("Pubs detectees - Connexion...") : qsTr("Connexion au flux..."))
        buffering = true
        console.log("[PlayerView] Calling mpvPlayer.play() with URL")
        mpvPlayer.play(url)
      } else {
        console.log("[PlayerView] ERROR: Empty or invalid HLS URL")
        updateStatus(qsTr("Impossible de recuperer l'URL du flux"))
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
    }
    function onAdsFinished() {
      console.log("[PlayerView] Ads finished")
      adsActive = false
      adSegments = 0
    }
    function onAdFilterLog(message) {
      console.log("[PlayerView AdFilter]", message)
    }
  }
  
  // Appeler loadStream() quand streamerLogin devient disponible
  onStreamerLoginChanged: {
    console.log("[PlayerView] onStreamerLoginChanged called, streamerLogin:", streamerLogin)
    if (streamerLogin && streamerLogin.length > 0) {
      loadStream()
    }
  }
  
  Component.onCompleted: {
    console.log("[PlayerView] Component.onCompleted called - Using MpvQuickItem for GPU rendering")
    playerRoot.forceActiveFocus()
  }
}
