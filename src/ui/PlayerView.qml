import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Window 2.2
import Qt.labs.settings 1.1

import BluePlayer.Media 1.0
import BluePlayer.Chat 1.0
import "themes/BlueTheme.js" as BlueTheme
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
  property string statusText: qsTr("Loading stream...")
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
  
  // Picture-in-Picture properties
  property bool pipActive: false
  property bool pipEnabled: true  // Disabled when fullscreen
  
  // Quality selector properties
  property var availableQualities: []
  property string currentQuality: "Auto"
  
  // Audio-only mode detection
  property bool isAudioOnly: BlueTheme.isAudioQuality(currentQuality)
  
  // Watch history properties
  property int lastSavedPosition: 0  // Track last saved position to avoid redundant saves
  property int pendingSeekPosition: 0  // Position to seek to when media is ready
  
  signal backRequested()
  signal pipRequested()
  signal pipReturnRequested()
  
  // Timer pour la sauvegarde automatique de la progression (toutes les 30s)
  Timer {
    id: autoSaveTimer
    interval: 30000  // 30 secondes
    repeat: true
    running: playerRoot.isVodMode && playerRoot.playing && !playerRoot.paused
    onTriggered: {
      saveWatchProgress()
    }
  }
  
  // Fonction pour sauvegarder la progression de visionnage
  function saveWatchProgress() {
    if (!isVodMode || !vodId || vodId.length === 0) return
    if (position <= 0 || duration <= 0) return
    
    // Éviter les sauvegardes redondantes (moins de 5s de différence)
    if (Math.abs(position - lastSavedPosition) < 5) return
    
    lastSavedPosition = position
    
    console.log("[PlayerView] Saving watch progress for VOD", vodId, "at position", Math.floor(position))
    
    // Mettre à jour le CacheManager avec la position
    if (typeof cacheManager !== "undefined" && cacheManager) {
      cacheManager.updateWatchPosition(vodId, Math.floor(position))
    }
  }
  
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
    if (seconds >= 0) {
      mpvPlayer.seek(seconds)
    }
  }

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
      updateStatus(qsTr("No stream selected"))
      return
    }
    
    updateStatus(qsTr("Fetching stream URL..."))
    console.log("[PlayerView] Calling twitchService.getStreamHlsUrl() with login:", streamerLogin)
    
    // Obtenir l'URL HLS via le service Twitch
    if (twitchService) {
      twitchService.getStreamHlsUrl(streamerLogin)
    } else {
      console.log("[PlayerView] ERROR: twitchService is null")
      updateStatus(qsTr("Twitch service unavailable"))
    }
  }
  
  function requestStop() {
    // Déconnecter le chat
    if (chatClient) {
      chatClient.disconnect()
    }
    chatVisible = false
    
    // Sauvegarder la progression de visionnage VOD à la fermeture
    if (isVodMode && vodId && vodId.length > 0 && position > 0) {
      console.log("[PlayerView] Saving watch progress on stop:", Math.floor(position))
      saveWatchProgress()
    }
    
    // Sauvegarder l'enregistrement si en cours
    saveRecordingIfNeeded()
    mpvPlayer.stop()
    statusText = qsTr("Playback stopped")
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
    
    console.log("[PlayerView] startAutoRecording called")
    
    // Utiliser CacheManager pour préparer l'enregistrement
    if (typeof cacheManager !== "undefined" && cacheManager) {
      var result = cacheManager.prepareRecording(streamerLogin, streamThumbnailUrl)
      
      if (result.recordingPath) {
        currentRecordingPath = result.recordingPath
        currentThumbnailPath = result.thumbnailPath || ""
        recordingStartTime = new Date(result.startTime)
        recordingGameCategory = ""
        
        console.log("[PlayerView] Starting auto-recording to:", currentRecordingPath)
        mpvPlayer.startRecording(currentRecordingPath)
      }
    } else {
      console.log("[PlayerView] Cannot start recording - cacheManager not available")
    }
  }
  
  function saveRecordingIfNeeded() {
    if (!currentRecordingPath || currentRecordingPath.length === 0) return
    if (isVodMode) return
    
    console.log("[PlayerView] Stopping recording and saving metadata")
    mpvPlayer.stopRecording()
    
    // Utiliser CacheManager pour finaliser l'enregistrement
    if (typeof cacheManager !== "undefined" && cacheManager && recordingStartTime) {
      var startMs = recordingStartTime.getTime()
      cacheManager.finalizeRecording(
        currentRecordingPath,
        streamerLogin,
        streamerName || streamerLogin,
        streamTitle || qsTr("Recorded stream"),
        currentThumbnailPath,
        startMs,
        playerRoot.currentQuality || "Auto"
      )
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
      updateStatus(qsTr("No file selected"))
      return
    }
    
    isVodMode = true
    liveMode = false
    updateStatus(qsTr("Loading video..."))
    
    // Appliquer les settings
    mpvPlayer.hardwareDecoding = hardwareDecodingEnabled
    mpvPlayer.cropVideo = cropMode
    mpvPlayer.playbackRate = playbackRate
    
    // Sauvegarder la position de reprise (sera appliquée quand le média est prêt)
    pendingSeekPosition = (vodMetadata && vodMetadata.watchPosition > 5) ? vodMetadata.watchPosition : 0
    
    console.log("[PlayerView] Playing VOD file:", vodFilePath)
    if (pendingSeekPosition > 0) {
      console.log("[PlayerView] Will resume at position:", pendingSeekPosition)
    }
    mpvPlayer.play(vodFilePath)
    controlsVisible = true
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
    toast.show(qsTr("Speed %1x").arg(clamped.toFixed(2)))
  }

  function toggleHardwareDecoding() {
    hardwareDecodingEnabled = !hardwareDecodingEnabled
    playerSettings.hardwareDecoding = hardwareDecodingEnabled
    if (mpvPlayer) {
      mpvPlayer.hardwareDecoding = hardwareDecodingEnabled
    }
    toast.show(hardwareDecodingEnabled ? qsTr("Hardware decoding") : qsTr("Software decoding"))
  }

  function toggleCropMode() {
    cropMode = !cropMode
    playerSettings.cropVideo = cropMode
    if (mpvPlayer) {
      mpvPlayer.cropVideo = cropMode
    }
    toast.show(cropMode ? qsTr("Crop mode enabled") : qsTr("Fit mode enabled"))
  }
  
  // PiP functions
  function togglePip() {
    if (!pipEnabled) return
    playerRoot.pipRequested()
  }
  
  function getVideoPlayer() {
    return mpvPlayer
  }
  
  function getVideoContainer() {
    return videoContainer
  }
  
  function returnVideoToPlayer() {
    if (mpvPlayer) {
      mpvPlayer.parent = videoContainer
      mpvPlayer.anchors.fill = videoContainer
    }
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
      anchors.right: chatPanel.left  // Always anchored to chat panel (which animates its width)
      
      // The player is defined below and reparented dynamically
    }

    // Chat Panel (right side) - z:100 to be above mouse area
    ChatPanel {
      id: chatPanel
      anchors.top: parent.top
      anchors.right: parent.right
      anchors.bottom: parent.bottom  // Full height for better integration
      width: chatVisible ? chatPanel.currentWidth : 0
      visible: chatVisible || chatCloseAnimation.running
      z: 100  // Above mouse interaction layer
      chatClient: chatClient
      channelName: playerRoot.streamerLogin
      
      onCloseRequested: {
        playerRoot.chatVisible = false
        chatClient.disconnect()
      }

      Behavior on width {
        NumberAnimation { 
          id: chatCloseAnimation
          duration: BlueTheme.animPanelDuration
          easing.type: Easing.OutCubic 
        }
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

        if (isPlaying) {
          playerRoot.buffering = false
          playerRoot.updateStatus(qsTr("Playing"))
          playerRoot.forceActiveFocus()
          // Démarrer l'enregistrement automatique pour les streams live
          if (!playerRoot.isVodMode && playerRoot.currentRecordingPath.length === 0) {
            playerRoot.startAutoRecording()
          }
        } else {
          if (playerRoot.hlsUrl.length > 0) {
            playerRoot.updateStatus(qsTr("Stopped"))
          }
        }
      }
      
      onPausedChanged: function(isPaused) {
        playerRoot.paused = isPaused
        if (isPaused) {
          playerRoot.updateStatus(qsTr("Paused"))
          playerRoot.controlsVisible = true
        } else if (playerRoot.playing) {
          playerRoot.updateStatus(qsTr("Playing"))
        }
      }
      
      onVolumeChanged: function(newVolume) { playerRoot.volume = newVolume }
      onMutedChanged: function(isMuted) { playerRoot.muted = isMuted }
      onDurationChanged: function(dur) {
        playerRoot.duration = dur
        // Appliquer le seek de reprise quand le média est prêt
        if (dur > 0 && playerRoot.pendingSeekPosition > 0) {
          console.log("[PlayerView] Media ready, seeking to:", playerRoot.pendingSeekPosition)
          mpvPlayer.seek(playerRoot.pendingSeekPosition)
          playerRoot.pendingSeekPosition = 0
        }
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
      }
      onBufferingChanged: function(isBuffering) { playerRoot.buffering = isBuffering }
      onPlaybackRateChanged: function(rate) { playerRoot.playbackRate = rate }
      onErrorOccurred: function(message) {
        playerRoot.updateStatus(qsTr("Error: %1").arg(message))
        errorToast.show(message)
      }
      onSpeedAutoReset: function(reason) {
        toast.show(reason)
        playerRoot.playbackRate = 1.0
      }
      onLeftLiveEdge: {
        toast.show(qsTr("Replay mode (stream in progress)"))
      }
    }
    
    // Audio-Only Placeholder Overlay
    Rectangle {
      id: audioOnlyPlaceholder
      anchors.fill: parent
      visible: playerRoot.isAudioOnly && playerRoot.playing
      color: BlueTheme.windowBackground
      
      // Background with thumbnail (blurred effect simulated with darker overlay)
      Image {
        id: audioThumbnail
        anchors.fill: parent
        source: playerRoot.streamThumbnailUrl
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: true
        opacity: 0.3
      }
      
      // Dark overlay for contrast
      Rectangle {
        anchors.fill: parent
        color: "#80000000"
      }
      
      // Center content
      Column {
        anchors.centerIn: parent
        spacing: BlueTheme.spacingMedium
        
        // Audio wave icon
        Rectangle {
          anchors.horizontalCenter: parent.horizontalCenter
          width: 80
          height: 80
          radius: 40
          color: "#33FFFFFF"
          border.color: BlueTheme.accent
          border.width: 2
          
          Text {
            anchors.centerIn: parent
            text: "\uD83C\uDFB5"  // 🎵
            font.pixelSize: 36
          }
          
          // Pulsing animation
          SequentialAnimation on scale {
            running: audioOnlyPlaceholder.visible
            loops: Animation.Infinite
            NumberAnimation { to: 1.05; duration: 1000; easing.type: Easing.InOutSine }
            NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutSine }
          }
        }
        
        // "Audio Only" label
        Text {
          anchors.horizontalCenter: parent.horizontalCenter
          text: qsTr("Audio Only")
          color: "#FFFFFF"
          font.pixelSize: 24
          font.family: BlueTheme.fontFamily
          font.weight: Font.DemiBold
        }
        
        // Streamer info
        Column {
          anchors.horizontalCenter: parent.horizontalCenter
          spacing: 4
          topPadding: BlueTheme.spacingSmall
          
          Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: playerRoot.streamerName || playerRoot.streamerLogin
            color: BlueTheme.primaryText
            font.pixelSize: 16
            font.family: BlueTheme.fontFamily
            font.weight: Font.Medium
            visible: text.length > 0
          }
          
          Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: playerRoot.streamTitle
            color: BlueTheme.secondaryText
            font.pixelSize: 13
            font.family: BlueTheme.fontFamily
            maximumLineCount: 2
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            width: Math.min(implicitWidth, 400)
            visible: text.length > 0
          }
        }
      }
      
      // Fade in animation
      opacity: visible ? 1.0 : 0.0
      Behavior on opacity {
        NumberAnimation { duration: BlueTheme.animOverlayDuration; easing.type: Easing.OutCubic }
      }
    }
    
    // PiP Placeholder Overlay - Shown when video is in PiP window
    PipPlaceholder {
      id: pipPlaceholder
      anchors.fill: parent
      visible: playerRoot.pipActive
      z: 50  // Above video but below controls
      
      onReturnRequested: {
        playerRoot.pipReturnRequested()
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
      anchors.right: chatPanel.left  // Always anchored to chat panel (which animates its width)
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
    
    // Top Bar Overlay (extracted component)
    TopBarOverlay {
      id: topBar
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      
      streamerName: playerRoot.streamerName
      streamerLogin: playerRoot.streamerLogin
      streamTitle: playerRoot.streamTitle
      statusText: playerRoot.statusText
      controlsVisible: playerRoot.controlsVisible
      
      onBackClicked: {
        requestStop()
        playerRoot.backRequested()
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
            NumberAnimation { target: centerFeedback; property: "opacity"; to: 1; duration: BlueTheme.animPressDuration }
            NumberAnimation { target: centerFeedback; property: "scale"; from: 0.8; to: 1.1; duration: BlueTheme.animHoverDuration }
        }
        PauseAnimation { duration: BlueTheme.animOverlayDuration }
        ParallelAnimation {
            NumberAnimation { target: centerFeedback; property: "opacity"; to: 0; duration: BlueTheme.animOverlayDuration }
            NumberAnimation { target: centerFeedback; property: "scale"; to: 1.5; duration: BlueTheme.animOverlayDuration }
        }
      }
    }

    // Loading Overlay (extracted component)
    LoadingOverlay {
      anchors.centerIn: parent
      loading: !playing && (statusText.indexOf("Loading") >= 0 || statusText.indexOf("Fetching") >= 0 || statusText.indexOf("Connecting") >= 0)
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
          text: qsTr("Ad playing (%1)").arg(adSegments)
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
      anchors.right: chatPanel.left  // Always anchored to chat panel (which animates its width)
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
      availableQualities: playerRoot.availableQualities
      currentQuality: playerRoot.currentQuality
      pipActive: playerRoot.pipActive
      pipEnabled: playerRoot.pipEnabled
      
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
      onQualitySelected: function(quality) {
        console.log("[PlayerView] Quality selected:", quality)
        if (twitchService) {
          twitchService.setStreamQuality(quality)
        }
      }
      onPipClicked: {
        playerRoot.togglePip()
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
      Behavior on opacity { NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic } }

      RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        BusyIndicator { running: true; Layout.preferredWidth: 20; Layout.preferredHeight: 20 }
        Text { text: qsTr("Buffering..."); color: "#FFFFFF"; font.pixelSize: 12; font.family: BlueTheme.fontFamily }
      }
    }

    // Toast pour toggles rapides - Design moderne avec taille dynamique
    Rectangle {
      id: toast
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: playerControlBar.top
      anchors.bottomMargin: BlueTheme.spacingMedium
      
      // Taille dynamique basée sur le contenu
      width: toastContent.width + BlueTheme.spacingLarge
      height: toastContent.height + BlueTheme.spacingMedium
      
      // Style moderne
      radius: height / 2  // Pill shape
      color: "#BF000000"  // Noir 75% opacité
      border.color: "#1AFFFFFF"  // Bordure très subtile
      border.width: 1
      
      visible: opacity > 0
      opacity: 0
      
      // Animation d'apparition fluide
      Behavior on opacity { 
        NumberAnimation { 
          duration: BlueTheme.animContentFadeDuration
          easing.type: Easing.OutCubic 
        } 
      }
      
      property string text: ""

      function show(msg) {
        text = msg
        toastAnim.restart()
      }

      Row {
        id: toastContent
        anchors.centerIn: parent
        spacing: BlueTheme.spacingSmall
        
        // Icône info avec style
        Rectangle {
          width: 18
          height: 18
          radius: 9
          color: BlueTheme.accent
          anchors.verticalCenter: parent.verticalCenter
          
          Text {
            anchors.centerIn: parent
            text: "i"
            color: "#FFFFFF"
            font.pixelSize: 11
            font.family: BlueTheme.fontFamily
            font.weight: Font.Bold
          }
        }
        
        // Texte du toast
        Text { 
          text: toast.text
          color: BlueTheme.primaryText
          font.pixelSize: 13
          font.family: BlueTheme.fontFamily
          font.weight: Font.Medium
          anchors.verticalCenter: parent.verticalCenter
        }
      }

      SequentialAnimation {
        id: toastAnim
        running: false
        PropertyAnimation { target: toast; property: "opacity"; to: 1; duration: BlueTheme.animToastEnterDuration }
        PauseAnimation { duration: BlueTheme.animToastDisplayDuration }
        PropertyAnimation { target: toast; property: "opacity"; to: 0; duration: BlueTheme.animToastExitDuration }
      }
    }

    // Error Toast (extracted component)
    ErrorToast {
      id: errorToast
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: playerControlBar.top
      anchors.bottomMargin: 60
      message: playerRoot.errorMessage
      showError: playerRoot.showError
      onShowErrorChanged: playerRoot.showError = showError
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
        // Priority: PiP → Fullscreen → Quit player
        if (playerRoot.pipActive) {
          // If PiP active, close it and return video
          playerRoot.pipReturnRequested()
        } else {
          var winEsc = Window.window
          if (winEsc && winEsc.visibility === Window.FullScreen) {
            winEsc.showNormal()
          } else {
            requestStop()
            playerRoot.backRequested()
          }
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
      case Qt.Key_Q:
        // Toggle quality selector (only if available)
        if (playerRoot.availableQualities.length > 0 && !isVodMode) {
          // Quality popup is handled by PlayerControlBar
          toast.show(qsTr("Use the quality button"))
        }
        event.accepted = true
        break
      case Qt.Key_P:
        // Toggle Picture-in-Picture
        togglePip()
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
        updateStatus(adsActive ? qsTr("Ads detected - Connecting...") : qsTr("Connecting to stream..."))
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
        updateStatus(qsTr("Failed to fetch stream URL"))
      }
    }
    function onErrorOccurred(message) {
      console.log("[PlayerView] Twitch error:", message)
      updateStatus(qsTr("Twitch error: %1").arg(message))
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
    function onAvailableQualitiesChanged() {
      playerRoot.availableQualities = twitchService.availableQualities
      console.log("[PlayerView] Available qualities:", playerRoot.availableQualities.length)
    }
    function onCurrentQualityChanged() {
      playerRoot.currentQuality = twitchService.currentQuality
      console.log("[PlayerView] Current quality:", playerRoot.currentQuality)
    }
    function onQualityChanged(url) {
      console.log("[PlayerView] Quality changed, new URL:", url.substring(0, 80) + "...")
      // Switch mpv to the new quality URL while preserving playback state
      if (mpvPlayer && url) {
        // Store current position to resume after quality switch
        var currentPos = playerRoot.position
        var wasPlaying = playerRoot.playing && !playerRoot.paused
        
        mpvPlayer.play(url)
        
        // Seek to previous position after a short delay
        if (currentPos > 5) {
          Qt.callLater(function() {
            mpvPlayer.seek(currentPos)
          })
        }
        
        toast.show(qsTr("Quality: %1").arg(BlueTheme.formatQuality(playerRoot.currentQuality)))
      }
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
