import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Window 2.2

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
  property bool isFullscreen: false  // Safari-style fullscreen mode
  
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

    // 1. Video Layer Container - player will be reparented here when not fullscreen
    Item {
      id: videoContainer
      anchors.fill: parent
      
      // The player is defined below and reparented dynamically
    }

    MpvQuickItem {
      id: mpvPlayer
      // Parent changes between videoContainer (normal) and fsVideoContainer (fullscreen)
      parent: isFullscreen ? fsVideoContainer : videoContainer
      anchors.fill: parent
      
      onPlayingChanged: function(isPlaying) {
        playerRoot.playing = isPlaying
        // Trigger center animation
        centerFeedback.show(isPlaying ? "\u25B6" : "\u23F8") // Play or Pause icon
        
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
      
      onVolumeChanged: function(newVolume) { playerRoot.volume = newVolume }
      onMutedChanged: function(isMuted) { playerRoot.muted = isMuted }
      onDurationChanged: function(dur) { playerRoot.duration = dur }
      onPositionChanged: function(pos) { playerRoot.position = pos }
      onLiveOffsetChanged: function(offset) { playerRoot.liveOffset = offset }
      onIsLiveModeChanged: function(isLive) { playerRoot.liveMode = isLive }
      onBufferingChanged: function(isBuffering) { playerRoot.buffering = isBuffering }
      onErrorOccurred: function(message) { playerRoot.updateStatus(qsTr("Erreur: %1").arg(message)) }
    }
    
    // 2. Mouse Interaction Layer (Background)
    // Placed here so it is BEHIND interface overlays (TopBar, ControlBar)
    MouseArea {
      id: backgroundMouseArea
      anchors.fill: parent
      hoverEnabled: true
      propagateComposedEvents: true 
      
      onPositionChanged: {
        playerRoot.controlsVisible = true
        hideControlsTimer.restart()
      }
      
      onClicked: {
          togglePlayPause()
          playerRoot.controlsVisible = true
          hideControlsTimer.restart()
      }
      
      onDoubleClicked: {
          var win = Window.window
          if (win) {
              if (win.visibility === Window.FullScreen)
                  win.showNormal()
              else
                  win.showFullScreen()
          }
      }
    }
    
    Timer {
      id: hideControlsTimer
      interval: 3000
      onTriggered: {
        if (playing && !paused) {
          playerRoot.controlsVisible = false
        }
      }
    }

    // 3. Interface Layer (Overlays)
    
    // Top Bar Overlay (Gradient)
    Rectangle {
      id: topBar
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      height: 80
      
      gradient: Gradient {
        GradientStop { position: 0.0; color: "#CC000000" }
        GradientStop { position: 1.0; color: "transparent" }
      }
      
      // Visibility Animation
      opacity: playerRoot.controlsVisible ? 1.0 : 0.0
      visible: opacity > 0
      Behavior on opacity { NumberAnimation { duration: 300 } }
      
      RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        spacing: 16
        
        // Back Button
        Rectangle {
          Layout.preferredWidth: 40
          Layout.preferredHeight: 40
          radius: 20
          color: backButtonMouseArea.containsMouse ? "#4DFFFFFF" : "#1AFFFFFF"
          
          Text {
            anchors.centerIn: parent
            text: "←"
            font.pixelSize: 22
            color: "#FFFFFF"
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
        
        // Stream Info
        ColumnLayout {
          Layout.fillWidth: true
          spacing: 2
          
          Text {
            text: streamerName || streamerLogin
            font.family: AppleTheme.fontFamily
            font.pixelSize: 18
            font.bold: true
            color: "#FFFFFF"
            style: Text.Outline; styleColor: "#80000000"
            elide: Text.ElideRight
            Layout.fillWidth: true
          }
          
          Text {
            text: streamTitle || qsTr("Stream en direct")
            font.family: AppleTheme.fontFamily
            font.pixelSize: 13
            color: "#DDFFFFFF"
            style: Text.Outline; styleColor: "#80000000"
            elide: Text.ElideRight
            Layout.fillWidth: true
          }
        }
        
        // Status Badge
        Rectangle {
          Layout.preferredHeight: 24
          Layout.preferredWidth: statusLabel.width + 16
          radius: 12
          color: "#4D000000"
          visible: statusText.length > 0
          
          Text {
            id: statusLabel
            anchors.centerIn: parent
            text: statusText
            font.pixelSize: 11
            color: "#FFFFFF"
          }
        }
      }
    }

    // Center Play/Pause Feedback Animation
    Item {
      id: centerFeedback
      anchors.centerIn: parent
      width: 100
      height: 100
      opacity: 0
      
      property string iconText: ""
      
      function show(icon) {
        iconText = icon
        feedbackAnim.restart()
      }
      
      Rectangle {
        anchors.fill: parent
        radius: 50
        color: "#80000000"
        
        Text {
          anchors.centerIn: parent
          text: centerFeedback.iconText
          color: "#FFFFFF"
          font.pixelSize: 48
        }
      }
      
      SequentialAnimation {
        id: feedbackAnim
        
        ParallelAnimation {
            NumberAnimation { target: centerFeedback; property: "opacity"; to: 1; duration: 100 }
            NumberAnimation { target: centerFeedback; property: "scale"; from: 0.8; to: 1.1; duration: 150 }
        }
        PauseAnimation { duration: 300 }
        ParallelAnimation {
            NumberAnimation { target: centerFeedback; property: "opacity"; to: 0; duration: 250 }
            NumberAnimation { target: centerFeedback; property: "scale"; to: 1.5; duration: 250 }
        }
      }
    }

    // Loading Overlay
    Rectangle {
      anchors.centerIn: parent
      width: 120; height: 120
      radius: 20
      color: "#80000000"
      visible: !playing && (statusText.indexOf("Chargement") >= 0 || statusText.indexOf("Connexion") >= 0)
      
      ColumnLayout {
        anchors.centerIn: parent
        spacing: 16
        
        BusyIndicator {
          Layout.alignment: Qt.AlignHCenter
          running: true
          palette.dark: "#FFFFFF" // Force white indicator
        }
      }
    }
    
    // Ad Badge (Top Left, under Top Bar)
    Rectangle {
      visible: adsActive
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.topMargin: 90 // Clear top bar
      anchors.leftMargin: 24
      width: adBadgeRow.width + 20
      height: 32
      radius: 16
      color: "#CC000000"
      border.color: "#FF9500"
      border.width: 1
      
      Row {
        id: adBadgeRow
        anchors.centerIn: parent
        spacing: 8
        
        Text {
          text: "AD"
          font.pixelSize: 11
          font.bold: true
          color: "#FF9500"
        }
        Text {
          text: qsTr("Pub en cours (%1)").arg(adSegments)
          font.pixelSize: 11
          font.bold: true
          color: "#FFFFFF"
        }
      }
    }
    
    // Bottom Control Bar
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
      onFullscreenClicked: {
          isFullscreen = !isFullscreen
          console.log("[PlayerView] Fullscreen toggled:", isFullscreen)
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
  
  // ========================================
  // FULLSCREEN WINDOW (Safari-style)
  // ========================================
  // A separate window that goes fullscreen independently
  // The main app window stays in normal mode
  
  Window {
    id: fullscreenWindow
    title: streamerName || streamerLogin
    color: "#000000"
    flags: Qt.Window
    
    // Controls visibility state
    property bool controlsShown: true
    
    // Only show when fullscreen is active
    visible: isFullscreen
    
    // Start in fullscreen when shown
    onVisibleChanged: {
      if (visible) {
        console.log("[Fullscreen] Window opened")
        showFullScreen()
      } else {
        console.log("[Fullscreen] Window closed")
        showNormal()
      }
    }
    
    // Container for the reparented player
    Item {
      id: fsVideoContainer
      anchors.fill: parent
    }
    
    // Mouse interaction
    MouseArea {
      id: fsMouseArea
      anchors.fill: parent
      hoverEnabled: true
      
      onPositionChanged: {
        fullscreenWindow.controlsShown = true
        fsControlsTimer.restart()
      }
      
      onDoubleClicked: {
        isFullscreen = false
      }
      
      onClicked: {
         // Toggle controls on click
         fullscreenWindow.controlsShown = !fullscreenWindow.controlsShown
         if (fullscreenWindow.controlsShown) fsControlsTimer.restart()
      }
    }
    
    // Auto-hide controls timer
    Timer {
      id: fsControlsTimer
      interval: 3000
      onTriggered: fullscreenWindow.controlsShown = false
    }
    
    // Minimal overlay controls
    Rectangle {
      id: fsControls
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      height: 80
      
      gradient: Gradient {
        GradientStop { position: 0.0; color: "transparent" }
        GradientStop { position: 1.0; color: "#CC000000" }
      }
      
      opacity: fullscreenWindow.controlsShown ? 1.0 : 0.0
      visible: opacity > 0
      Behavior on opacity { NumberAnimation { duration: 200 } }
      
      // Streamer name
      Text {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 20
        text: streamerName || streamerLogin
        font.family: AppleTheme.fontFamily
        font.pixelSize: 18
        font.bold: true
        color: "#FFFFFF"
      }
      
      // Exit button
      Rectangle {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        width: 100
        height: 36
        radius: 8
        color: exitFsBtnMouse.containsMouse ? "#FF4444" : "#80FFFFFF"
        
        Text {
          anchors.centerIn: parent
          text: "Exit ⎋"
          font.pixelSize: 14
          color: exitFsBtnMouse.containsMouse ? "#FFFFFF" : "#000000"
        }
        
        MouseArea {
          id: exitFsBtnMouse
          anchors.fill: parent
          hoverEnabled: true
          cursorShape: Qt.PointingHandCursor
          onClicked: isFullscreen = false
        }
      }
    }
    
    // Keyboard shortcuts
    Shortcut {
      sequence: "Escape"
      onActivated: isFullscreen = false
    }
    
    Shortcut {
      sequence: "F"
      onActivated: isFullscreen = false
    }
  }
}
