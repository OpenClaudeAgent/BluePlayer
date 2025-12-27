import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Window 2.2
import Qt.labs.settings 1.1

import BluePlayer.Media 1.0
import BluePlayer.Chat 1.0
import "themes/AppleTheme.js" as AppleTheme
import "components"

Item {
  id: playerRoot
  
  property var twitchService: null
  property string streamerLogin: ""
  property string streamerName: ""
  property string streamTitle: ""
  property string streamThumbnailUrl: ""  // URL du thumbnail Twitch
  // VOD mode properties
  property bool isVodMode: false
  property string vodFilePath: ""
  property string vodId: ""
  property var vodMetadata: null
  // Recording properties
  property string currentRecordingPath: ""
  property string currentThumbnailPath: ""
  property var recordingStartTime: null
  property string recordingGameCategory: ""
  property bool playing: false
  property bool paused: false
  property bool buffering: false
  property real volume: 1.0
  property bool muted: false
  property real duration: 0.0
  property real position: 0.0
  property real liveOffset: 0.0
  property bool loggingEnabled: false
  property bool loggingVerbose: false
  property bool liveMode: true
  property string statusText: qsTr("Chargement du flux...")
  property string hlsUrl: ""
  property bool adsActive: false
  property int adSegments: 0
  property bool controlsVisible: true
  // isFullscreen removed - now using native Window.showFullScreen()
  property bool hardwareDecodingEnabled: true
  property bool cropMode: false
  property real playbackRate: 1.0
  property string errorMessage: ""
  property bool showError: false
  property bool settingsApplied: false  // avoid double applying settings on load
  property bool chatVisible: false
  property bool chatEnabled: !isVodMode  // Chat disabled in VOD mode
  
  signal backRequested()
  
  Settings {
    id: playerSettings
    category: "player"
    property bool hardwareDecoding: true
    property bool cropVideo: false
    property double playbackRate: 1.0
  }
  
  function seekTo(seconds) {
    if (loggingEnabled) {
      console.log("[seekbar] seekTo seconds=" + seconds.toFixed(2))
    }
    // #region agent log
    console.log(JSON.stringify({location:'PlayerView.qml:55',message:'seekTo called',data:{seconds:seconds,liveMode:playerRoot.liveMode},timestamp:Date.now(),sessionId:'debug-session',runId:'run1',hypothesisId:'H1'}));
    // #endregion
    if (seconds >= 0) {
      mpvPlayer.seek(seconds)
    }
  }

  function updateStatus(message) {
    statusText = message
  }
  
  function loadStream() {
    // #region agent log
    console.log(JSON.stringify({location:'PlayerView.qml:66',message:'loadStream called',data:{streamerLogin:streamerLogin,liveMode:playerRoot.liveMode},timestamp:Date.now(),sessionId:'debug-session',runId:'run1',hypothesisId:'H1'}));
    // #endregion
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
    // Déconnecter le chat
    if (chatClient) {
      chatClient.disconnect()
    }
    chatVisible = false
    // Sauvegarder l'enregistrement si en cours
    saveRecordingIfNeeded()
    mpvPlayer.stop()
    statusText = qsTr("Lecture arrêtée")
  }
  
  function updateChatCredentials() {
    if (twitchService && twitchService.accessToken && twitchService.userName) {
      console.log("[PlayerView] Setting chat credentials for:", twitchService.userName)
      chatClient.setCredentials(twitchService.accessToken, twitchService.userName)
    } else {
      console.log("[PlayerView] Cannot set chat credentials - missing token or username")
    }
  }
  
  function startAutoRecording() {
    if (isVodMode || !streamerLogin) return
    
    // Générer un nom de fichier unique
    var timestamp = new Date().toISOString().replace(/[:.]/g, "-")
    var filename = streamerLogin + "_" + timestamp + ".ts"
    var thumbFilename = streamerLogin + "_" + timestamp + ".jpg"
    
    console.log("[PlayerView] startAutoRecording called")
    console.log("[PlayerView]   streamerLogin:", streamerLogin)
    console.log("[PlayerView]   streamThumbnailUrl:", streamThumbnailUrl)
    
    // Obtenir le chemin du cache
    if (typeof cacheManager !== "undefined" && cacheManager) {
      var cachePath = cacheManager.cacheDirectory + "/" + filename
      console.log("[PlayerView] Starting auto-recording to:", cachePath)
      
      currentRecordingPath = cachePath
      recordingStartTime = new Date()
      recordingGameCategory = ""  // TODO: obtenir depuis twitchService si disponible
      
      // Télécharger le thumbnail si disponible
      if (streamThumbnailUrl && streamThumbnailUrl.length > 0) {
        console.log("[PlayerView] Downloading thumbnail from:", streamThumbnailUrl)
        currentThumbnailPath = cacheManager.downloadThumbnail(streamThumbnailUrl, thumbFilename)
        console.log("[PlayerView] Thumbnail saved to:", currentThumbnailPath)
      } else {
        console.log("[PlayerView] No thumbnail URL available")
        currentThumbnailPath = ""
      }
      
      mpvPlayer.startRecording(cachePath)
    } else {
      console.log("[PlayerView] Cannot start recording - cacheManager not available")
    }
  }
  
  function saveRecordingIfNeeded() {
    if (!currentRecordingPath || currentRecordingPath.length === 0) return
    if (isVodMode) return
    
    console.log("[PlayerView] Stopping recording and saving metadata")
    mpvPlayer.stopRecording()
    
    // Calculer la durée
    var durationSecs = 0
    if (recordingStartTime) {
      durationSecs = Math.floor((new Date() - recordingStartTime) / 1000)
    }
    
    // Ne pas sauvegarder les enregistrements trop courts (< 30 secondes)
    if (durationSecs < 30) {
      console.log("[PlayerView] Recording too short (" + durationSecs + "s), not saving")
      currentRecordingPath = ""
      recordingStartTime = null
      return
    }
    
    // Ajouter au cache manager
    if (typeof cacheManager !== "undefined" && cacheManager) {
      console.log("[PlayerView] currentThumbnailPath:", currentThumbnailPath)
      var metadata = {
        "streamerLogin": streamerLogin,
        "streamerName": streamerName || streamerLogin,
        "streamTitle": streamTitle || qsTr("Stream enregistre"),
        "filePath": currentRecordingPath,
        "duration": durationSecs,
        "gameCategory": recordingGameCategory,
        "thumbnailPath": currentThumbnailPath
      }
      
      console.log("[PlayerView] Saving VOD metadata:", JSON.stringify(metadata))
      cacheManager.addVodFromQml(metadata)
    }
    
    // Reset
    currentRecordingPath = ""
    currentThumbnailPath = ""
    recordingStartTime = null
  }
  
  function loadVod() {
    console.log("[PlayerView] loadVod() called")
    console.log("[PlayerView] vodFilePath:", vodFilePath)
    
    if (!vodFilePath || vodFilePath.length === 0) {
      console.log("[PlayerView] ERROR: No VOD file path provided")
      updateStatus(qsTr("Aucun fichier selectionne"))
      return
    }
    
    isVodMode = true
    liveMode = false
    updateStatus(qsTr("Chargement de la video..."))
    
    // Appliquer les settings
    mpvPlayer.hardwareDecoding = hardwareDecodingEnabled
    mpvPlayer.cropVideo = cropMode
    mpvPlayer.playbackRate = playbackRate
    
    // Charger le fichier local
    console.log("[PlayerView] Playing VOD file:", vodFilePath)
    mpvPlayer.play(vodFilePath)
    controlsVisible = true
    
    // Reprendre à la position sauvegardée si disponible
    if (vodMetadata && vodMetadata.watchPosition > 0) {
      console.log("[PlayerView] Resuming from position:", vodMetadata.watchPosition)
      Qt.callLater(function() {
        mpvPlayer.seek(vodMetadata.watchPosition)
      })
    }
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

  function goLive() {
    if (loggingEnabled) {
      console.log("[seekbar] goLive")
    }
    mpvPlayer.seekToLive()
  }
  
  function applyPersistedSettings() {
    if (settingsApplied)
      return
    hardwareDecodingEnabled = playerSettings.hardwareDecoding
    cropMode = playerSettings.cropVideo
    playbackRate = playerSettings.playbackRate
    if (mpvPlayer) {
      mpvPlayer.hardwareDecoding = hardwareDecodingEnabled
      mpvPlayer.cropVideo = cropMode
      mpvPlayer.playbackRate = playbackRate
    }
    settingsApplied = true
  }

  function adjustPlaybackRate(targetRate) {
    var clamped = Math.max(0.25, Math.min(3.0, targetRate))
    playbackRate = clamped
    playerSettings.playbackRate = clamped
    if (mpvPlayer) {
      mpvPlayer.playbackRate = clamped
    }
    toast.show(qsTr("Vitesse %1x").arg(clamped.toFixed(2)))
  }

  function toggleHardwareDecoding() {
    hardwareDecodingEnabled = !hardwareDecodingEnabled
    playerSettings.hardwareDecoding = hardwareDecodingEnabled
    if (mpvPlayer) {
      mpvPlayer.hardwareDecoding = hardwareDecodingEnabled
    }
    toast.show(hardwareDecodingEnabled ? qsTr("Décodage matériel") : qsTr("Décodage logiciel"))
  }

  function toggleCropMode() {
    cropMode = !cropMode
    playerSettings.cropVideo = cropMode
    if (mpvPlayer) {
      mpvPlayer.cropVideo = cropMode
    }
    toast.show(cropMode ? qsTr("Rognage actif") : qsTr("Adaptation proportionnelle"))
  }
  
  
  Rectangle {
    anchors.fill: parent
    color: "#000000"

    // Chat client (created once, reused)
    TwitchChatClient {
      id: chatClient
    }

    // 1. Video Layer Container - player will be reparented here when not fullscreen
    Item {
      id: videoContainer
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.bottom: parent.bottom
      anchors.right: chatVisible ? chatPanel.left : parent.right
      
      // The player is defined below and reparented dynamically
    }

    // Chat Panel (right side) - z:100 to be above mouse area
    ChatPanel {
      id: chatPanel
      anchors.top: parent.top
      anchors.right: parent.right
      anchors.bottom: parent.bottom  // Full height for better integration
      width: chatVisible ? chatPanel.currentWidth : 0
      visible: chatVisible
      z: 100  // Above mouse interaction layer
      chatClient: chatClient
      channelName: playerRoot.streamerLogin
      
      onCloseRequested: {
        playerRoot.chatVisible = false
        chatClient.disconnect()
      }

      Behavior on width {
        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
      }
    }

    MpvQuickItem {
      id: mpvPlayer
      parent: videoContainer
      anchors.fill: parent
      
      onPlayingChanged: function(isPlaying) {
        playerRoot.playing = isPlaying
        // Trigger center animation
        centerFeedback.show(isPlaying ? "\u25B6" : "\u23F8") // Play or Pause icon
        
        // #region agent log
        console.log(JSON.stringify({location:'PlayerView.qml:175',message:'onPlayingChanged',data:{isPlaying:isPlaying,liveMode:playerRoot.liveMode},timestamp:Date.now(),sessionId:'debug-session',runId:'run1',hypothesisId:'H3'}));
        // #endregion

        if (isPlaying) {
          playerRoot.buffering = false
          playerRoot.updateStatus(qsTr("Lecture en cours"))
          playerRoot.forceActiveFocus()
          // Démarrer l'enregistrement automatique pour les streams live
          if (!playerRoot.isVodMode && playerRoot.currentRecordingPath.length === 0) {
            playerRoot.startAutoRecording()
          }
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
      onDurationChanged: function(dur) {
        playerRoot.duration = dur
      }
      onPositionChanged: function(pos) {
        playerRoot.position = pos
        if (playerRoot.duration > 0) {
          playerRoot.liveOffset = Math.max(0, playerRoot.duration - pos)
        }
      }
      onLiveOffsetChanged: function(offset) {
        playerRoot.liveOffset = offset
      }
      onIsLiveModeChanged: function(isLive) { 
        playerRoot.liveMode = isLive
        // #region agent log
        console.log(JSON.stringify({location:'PlayerView.qml:215',message:'onIsLiveModeChanged',data:{isLive:isLive,playerRootLiveMode:playerRoot.liveMode},timestamp:Date.now(),sessionId:'debug-session',runId:'run1',hypothesisId:'H3'}));
        // #endregion
      }
      onBufferingChanged: function(isBuffering) { playerRoot.buffering = isBuffering }
      onPlaybackRateChanged: function(rate) { playerRoot.playbackRate = rate }
      onErrorOccurred: function(message) {
        playerRoot.updateStatus(qsTr("Erreur: %1").arg(message))
        playerRoot.errorMessage = message
        playerRoot.showError = true
        errorHideTimer.restart()
      }
      onSpeedAutoReset: function(reason) {
        toast.show(reason)
        playerRoot.playbackRate = 1.0
      }
      onLeftLiveEdge: {
        toast.show(qsTr("Mode replay (stream en cours)"))
      }
    }
    
    // 2. Mouse Interaction Layer (Background)
    // Placed here so it is BEHIND interface overlays (TopBar, ControlBar)
    // Only covers video area, not chat panel
    MouseArea {
      id: backgroundMouseArea
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.bottom: parent.bottom
      anchors.right: chatVisible ? chatPanel.left : parent.right
      hoverEnabled: true
      propagateComposedEvents: true
      // Hide cursor in fullscreen when controls are hidden
      cursorShape: {
        var win = Window.window
        var isFullscreen = win && win.visibility === Window.FullScreen
        return (isFullscreen && !playerRoot.controlsVisible) ? Qt.BlankCursor : Qt.ArrowCursor
      }
      
      onPositionChanged: {
        playerRoot.controlsVisible = true
        hideControlsTimer.restart()
      }
      
      onClicked: {
          // Ne pas mettre en pause au clic - uniquement montrer les contrôles
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
    
    // Bottom Control Bar - adapts to chat panel
    PlayerControlBar {
      id: playerControlBar
      anchors.left: parent.left
      anchors.right: chatVisible ? chatPanel.left : parent.right
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
      isReplayMode: playerRoot.isVodMode
      controlsVisible: playerRoot.controlsVisible
      playbackRate: playerRoot.playbackRate
      hardwareDecoding: playerRoot.hardwareDecodingEnabled
      cropVideo: playerRoot.cropMode
      chatVisible: playerRoot.chatVisible
      chatEnabled: playerRoot.chatEnabled
      
      onPlayPauseClicked: togglePlayPause()
      onStopClicked: {
        requestStop()
        playerRoot.backRequested()
      }
      onVolumeRequested: function(newVolume) { setVolume(newVolume) }
      onMuteClicked: toggleMute()
      onSeekRequested: function(seconds) {
        seekTo(seconds)
      }
      onSeekDragStarted: {
        if (playerRoot.loggingEnabled) {
          console.log("[seekbar] drag_start")
        }
      }
      onSeekPreviewed: function(seconds) {
        if (playerRoot.loggingEnabled && playerRoot.loggingVerbose) {
          console.log("[seekbar] drag_preview seconds=", seconds.toFixed(2))
        }
      }
      onSeekDragEnded: function(seconds) {
        if (playerRoot.loggingEnabled) {
          console.log("[seekbar] drag_end seconds=", seconds.toFixed(2))
        }
      }
      onLiveRequested: goLive()
      onLiveClicked: goLive()
      onFullscreenClicked: {
          // Use same mechanism as double-click for native fullscreen
          var win = Window.window
          if (win) {
              if (win.visibility === Window.FullScreen)
                  win.showNormal()
              else
                  win.showFullScreen()
          }
      }
      onPlaybackRateRequested: function(rate) { adjustPlaybackRate(rate) }
      onHardwareToggleClicked: toggleHardwareDecoding()
      onCropToggleClicked: toggleCropMode()
      onChatToggleClicked: {
        playerRoot.chatVisible = !playerRoot.chatVisible
        if (playerRoot.chatVisible && playerRoot.streamerLogin) {
          updateChatCredentials()
          chatClient.connectToChannel(playerRoot.streamerLogin)
        } else {
          chatClient.disconnect()
        }
      }
    }

    // Buffering badge (when already en lecture)
    Rectangle {
      anchors.top: parent.top
      anchors.right: parent.right
      anchors.margins: 20
      visible: buffering
      opacity: buffering ? 1.0 : 0.0
      radius: 12
      color: "#AA000000"
      border.color: "#33FFFFFF"
      border.width: 1
      Behavior on opacity { NumberAnimation { duration: 150 } }

      RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        BusyIndicator { running: true; Layout.preferredWidth: 20; Layout.preferredHeight: 20 }
        Text { text: qsTr("Buffering..."); color: "#FFFFFF"; font.pixelSize: 12 }
      }
    }

    // Toast pour toggles rapides
    Rectangle {
      id: toast
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: playerControlBar.top
      anchors.bottomMargin: 12
      radius: 10
      color: "#CC000000"
      border.color: "#55FFFFFF"
      visible: opacity > 0
      opacity: 0
      Behavior on opacity { NumberAnimation { duration: 200 } }
      property string text: ""

      function show(msg) {
        text = msg
        toastAnim.restart()
      }

      Row {
        anchors.margins: 12
        anchors.fill: parent
        spacing: 8
        Text { text: "\u2139"; color: "#FFFFFF"; font.pixelSize: 13 }
        Text { text: toast.text; color: "#FFFFFF"; font.pixelSize: 13 }
      }

      SequentialAnimation {
        id: toastAnim
        running: false
        PropertyAnimation { target: toast; property: "opacity"; to: 1; duration: 120 }
        PauseAnimation { duration: 1400 }
        PropertyAnimation { target: toast; property: "opacity"; to: 0; duration: 200 }
      }
    }

    // Erreur toast
    Rectangle {
      id: errorToast
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: playerControlBar.top
      anchors.bottomMargin: 60
      radius: 10
      color: "#CCB00020"
      border.color: "#FF5252"
      visible: showError
      opacity: showError ? 1.0 : 0.0
      Behavior on opacity { NumberAnimation { duration: 180 } }

      Row {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8
        Text { text: "\u26A0"; color: "#FFFFFF"; font.pixelSize: 13 }
        Text { text: errorMessage; color: "#FFFFFF"; font.pixelSize: 13; wrapMode: Text.WordWrap; Layout.fillWidth: true }
      }
    }

    Timer {
      id: errorHideTimer
      interval: 4000
      running: false
      repeat: false
      onTriggered: showError = false
    }

  }
  
  // Gestion des raccourcis clavier
  Keys.onPressed: function(event) {
    switch (event.key) {
      case Qt.Key_Space:
        togglePlayPause()
        event.accepted = true
        break
      case Qt.Key_Left:
        seekTo(Math.max(0, position - 10))
        event.accepted = true
        break
      case Qt.Key_Right:
        seekTo(Math.min(duration, position + 10))
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
        // If fullscreen, exit fullscreen first; otherwise quit player
        var winEsc = Window.window
        if (winEsc && winEsc.visibility === Window.FullScreen) {
          winEsc.showNormal()
        } else {
          requestStop()
          playerRoot.backRequested()
        }
        event.accepted = true
        break
      case Qt.Key_Plus:
      case Qt.Key_Equal:
        adjustPlaybackRate(playbackRate + 0.1)
        event.accepted = true
        break
      case Qt.Key_Minus:
      case Qt.Key_Underscore:
        adjustPlaybackRate(playbackRate - 0.1)
        event.accepted = true
        break
      case Qt.Key_R:
        adjustPlaybackRate(1.0)
        event.accepted = true
        break
      case Qt.Key_F:
        // Toggle native fullscreen (same as double-click)
        var winF = Window.window
        if (winF) {
          if (winF.visibility === Window.FullScreen)
            winF.showNormal()
          else
            winF.showFullScreen()
        }
        event.accepted = true
        break
      case Qt.Key_L:
        goLive()
        event.accepted = true
        break
      case Qt.Key_H:
        toggleHardwareDecoding()
        event.accepted = true
        break
      case Qt.Key_C:
        // Toggle chat (only in live mode)
        if (chatEnabled) {
          playerRoot.chatVisible = !playerRoot.chatVisible
          if (playerRoot.chatVisible && playerRoot.streamerLogin) {
            updateChatCredentials()
            chatClient.connectToChannel(playerRoot.streamerLogin)
          } else {
            chatClient.disconnect()
          }
        }
        event.accepted = true
        break
      case Qt.Key_V:
        toggleCropMode()
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
        mpvPlayer.hardwareDecoding = hardwareDecodingEnabled
        mpvPlayer.cropVideo = cropMode
        mpvPlayer.playbackRate = playbackRate
        mpvPlayer.play(url)
        controlsVisible = true
        // Force live mode and seek to live edge when a new stream is loaded
        mpvPlayer.seekToLive()
        // #endregion
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
    if (streamerLogin && streamerLogin.length > 0 && !isVodMode) {
      playerRoot.liveMode = true // Force live mode for new streams
      loadStream()
    }
  }
  
  // Appeler loadVod() quand vodFilePath devient disponible
  onVodFilePathChanged: {
    console.log("[PlayerView] onVodFilePathChanged called, vodFilePath:", vodFilePath)
    if (vodFilePath && vodFilePath.length > 0) {
      loadVod()
    }
  }

  Component.onCompleted: {
    console.log("[PlayerView] Component.onCompleted called - Using MpvQuickItem for GPU rendering")
    applyPersistedSettings()
    playerRoot.forceActiveFocus()
  }
  
}
