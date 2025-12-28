import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15


import "themes/BlueTheme.js" as BlueTheme
import "components"

ApplicationWindow {
  id: root
  visible: true
  width: 1200
  height: 780
  color: BlueTheme.windowBackground
  font.family: BlueTheme.fontFamily
  title: qsTr("BluePlayer")
  property string currentView: "home"
  property string statusText: qsTr("Sélectionnez un stream ou une vidéo locale pour commencer.")
  property string playerStreamerLogin: ""
  property string playerStreamerName: ""
  property string playerStreamTitle: ""
  property string playerStreamThumbnailUrl: ""
  // VOD playback properties
  property string vodId: ""
  property string vodFilePath: ""
  property var vodMetadata: null

  // Accès au service Twitch (disponible globalement via setContextProperty)
  // Ne pas créer de propriété locale pour éviter de masquer la variable globale
  function getTwitchService() {
    return typeof twitchService !== "undefined" ? twitchService : null
  }

  background: Rectangle {
    anchors.fill: parent
    gradient: Gradient {
      GradientStop { position: 0; color: BlueTheme.gradientStart }
      GradientStop { position: 1; color: BlueTheme.gradientEnd }
    }
  }

  property bool preferencesActive: currentView === "preferences"
  property bool cacheActive: currentView === "cache"

  // ==========================================================================
  // NAVIGATION GLOBALE - Boutons en haut à droite
  // Option B : Cachés quand un panel est ouvert (pas d'overlap)
  // ==========================================================================
  Row {
    id: globalNavigation
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.margins: BlueTheme.spacingSmall
    z: 10
    spacing: 8
    
    // Visible UNIQUEMENT sur Home (pas dans les panels)
    visible: {
      var service = root.getTwitchService()
      if (!service || !service.authenticated) {
        return false
      }
      return currentView === "home"
    }
    
    // Fade animation
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity {
      NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
    }

    // Bouton Replays - Utilise CircleButton
    CircleButton {
      iconText: "\u21BA"
      active: cacheActive
      tooltipText: {
        if (typeof cacheManager !== "undefined" && cacheManager) {
          return qsTr("Mes Replays (%1)").arg(cacheManager.vodCount)
        }
        return qsTr("Mes Replays")
      }
      onClicked: currentView = "cache"
    }

    // Bouton Préférences - Utilise CircleButton
    CircleButton {
      iconText: "\u2699"
      iconSize: 18
      active: preferencesActive
      tooltipText: qsTr("Préférences")
      onClicked: currentView = "preferences"
    }
  }

  // Player view - full screen, no margins (outside ColumnLayout)
  // Z-index 5 to overlay on top of home content (which stays rendered behind)
  Loader {
    id: playerLoader
    anchors.fill: parent
    visible: opacity > 0
    active: currentView === "player"
    opacity: currentView === "player" ? 1.0 : 0.0
    z: 5  // Above home content, below header buttons (z:10)
    source: "PlayerView.qml"
    
    Behavior on opacity {
      NumberAnimation { duration: BlueTheme.animContentFadeDuration; easing.type: Easing.OutCubic }
    }
    
    onItemChanged: {
      if (item && currentView === "player") {
        console.log("[main.qml] Setting PlayerView properties")
        console.log("[main.qml] root.playerStreamThumbnailUrl =", root.playerStreamThumbnailUrl)
        var service = root.getTwitchService()
        item.twitchService = service
        // Set thumbnail URL FIRST before streamerLogin triggers loading
        item.streamThumbnailUrl = root.playerStreamThumbnailUrl
        item.streamerName = root.playerStreamerName
        item.streamTitle = root.playerStreamTitle
        item.streamerLogin = root.playerStreamerLogin  // This triggers onStreamerLoginChanged - set last
        // VOD properties
        item.vodId = root.vodId
        item.vodFilePath = root.vodFilePath
        item.vodMetadata = root.vodMetadata
        item.isVodMode = root.vodFilePath.length > 0
        item.backRequested.connect(function() {
          console.log("[main.qml] Back requested, returning to previous view")
          // Si on venait du cache manager, y retourner
          if (root.vodFilePath.length > 0) {
            root.currentView = "cache"
            // Reset VOD properties
            root.vodId = ""
            root.vodFilePath = ""
            root.vodMetadata = null
          } else {
            root.currentView = "home"
          }
        })
      }
    }
  }

  // ==========================================================================
  // VIEWS ARCHITECTURE: Separate loaders to preserve state
  // - HomeView: Always active (never destroyed) to keep thumbnails loaded
  // - Preferences/Cache: Overlay on top of home, destroyed when closed
  // - Player: Full overlay with fade animation
  // ==========================================================================

  // Helper to check if authenticated
  property bool isAuthenticated: {
    var service = root.getTwitchService()
    return service && service.authenticated
  }

  // --------------------------------------------------------------------------
  // LOGIN VIEW - Shown when not authenticated
  // --------------------------------------------------------------------------
  ColumnLayout {
    id: loginContainer
    anchors.fill: parent
    anchors.margins: BlueTheme.spacingLarge
    spacing: BlueTheme.spacingMedium
    visible: !isAuthenticated
    
    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
      
      Loader {
        id: loginLoader
        anchors.fill: parent
        active: !isAuthenticated
        source: "LoginView.qml"
      }
    }
  }

  // --------------------------------------------------------------------------
  // HOME VIEW - Always active when authenticated (never destroyed)
  // --------------------------------------------------------------------------
  ColumnLayout {
    id: homeContainer
    anchors.fill: parent
    anchors.margins: BlueTheme.spacingLarge
    spacing: BlueTheme.spacingMedium
    visible: isAuthenticated && currentView !== "player"
    
    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
      
      Loader {
        id: homeLoader
        anchors.fill: parent
        // ALWAYS active when authenticated - never destroyed
        active: isAuthenticated
        source: "HomeView.qml"
        
        onItemChanged: {
          if (item) {
            console.log("[main.qml] HomeView loaded, connecting signals")
            
            item.openStreamPlayer.connect(function(login, name, title, thumbnailUrl) {
              console.log("[main.qml] openStreamPlayer:", login)
              root.playerStreamerLogin = login
              root.playerStreamerName = name
              root.playerStreamTitle = title
              root.playerStreamThumbnailUrl = thumbnailUrl
              root.currentView = "player"
            })
            
            item.openCacheManager.connect(function() {
              console.log("[main.qml] Opening cache manager")
              root.currentView = "cache"
            })
            
            item.playVodRequested.connect(function(id, filePath, metadata) {
              console.log("[main.qml] HomeView playVodRequested:", id, filePath)
              root.vodId = id
              root.vodFilePath = filePath
              root.vodMetadata = metadata
              root.playerStreamerLogin = ""
              root.playerStreamerName = metadata.streamerName || ""
              root.playerStreamTitle = metadata.streamTitle || ""
              if (cacheManager) {
                cacheManager.markAsPlayed(id)
              }
              root.currentView = "player"
            })
          }
        }
      }
    }
  }

  // --------------------------------------------------------------------------
  // PREFERENCES VIEW - Overlay on top of home
  // --------------------------------------------------------------------------
  ColumnLayout {
    id: preferencesContainer
    anchors.fill: parent
    anchors.margins: BlueTheme.spacingLarge
    spacing: BlueTheme.spacingMedium
    visible: currentView === "preferences"
    z: 2  // Above home
    
    // Background to cover home
    Rectangle {
      Layout.fillWidth: true
      Layout.fillHeight: true
      color: "transparent"
      
      // Gradient background matching app theme
      Rectangle {
        anchors.fill: parent
        gradient: Gradient {
          GradientStop { position: 0; color: BlueTheme.gradientStart }
          GradientStop { position: 1; color: BlueTheme.gradientEnd }
        }
      }
      
      ScrollView {
        anchors.fill: parent
        clip: true
        
        Loader {
          id: preferencesLoader
          anchors.fill: parent
          active: currentView === "preferences"
          source: "PreferencesView.qml"
          
          onItemChanged: {
            if (item && item.hasOwnProperty("closeRequested")) {
              item.closeRequested.connect(function() {
                root.currentView = "home"
              })
            }
          }
        }
      }
    }
  }

  // --------------------------------------------------------------------------
  // CACHE MANAGER VIEW - Overlay on top of home
  // --------------------------------------------------------------------------
  ColumnLayout {
    id: cacheContainer
    anchors.fill: parent
    anchors.margins: BlueTheme.spacingLarge
    spacing: BlueTheme.spacingMedium
    visible: currentView === "cache"
    z: 2  // Above home
    
    // Background to cover home
    Rectangle {
      Layout.fillWidth: true
      Layout.fillHeight: true
      color: "transparent"
      
      // Gradient background matching app theme
      Rectangle {
        anchors.fill: parent
        gradient: Gradient {
          GradientStop { position: 0; color: BlueTheme.gradientStart }
          GradientStop { position: 1; color: BlueTheme.gradientEnd }
        }
      }
      
      ScrollView {
        anchors.fill: parent
        clip: true
        
        Loader {
          id: cacheLoader
          anchors.fill: parent
          active: currentView === "cache"
          source: "CacheManagerView.qml"
          
          onItemChanged: {
            if (item) {
              if (item.hasOwnProperty("cacheManager")) {
                item.cacheManager = cacheManager
              }
              if (item.hasOwnProperty("backRequested")) {
                item.backRequested.connect(function() {
                  root.currentView = "home"
                })
              }
              if (item.hasOwnProperty("playVodRequested")) {
                item.playVodRequested.connect(function(id, filePath, metadata) {
                  console.log("[main.qml] CacheManager playVodRequested:", id, filePath)
                  root.vodId = id
                  root.vodFilePath = filePath
                  root.vodMetadata = metadata
                  root.playerStreamerLogin = ""
                  root.playerStreamerName = metadata.streamerName || ""
                  root.playerStreamTitle = metadata.streamTitle || ""
                  if (cacheManager) {
                    cacheManager.markAsPlayed(id)
                  }
                  root.currentView = "player"
                })
              }
            }
          }
        }
      }
    }
  }

  // --------------------------------------------------------------------------
  // Refresh home data when returning from player
  // --------------------------------------------------------------------------
  property string previousView: "home"
  onCurrentViewChanged: {
    if (previousView === "player" && currentView === "home") {
      console.log("[main.qml] Returned from player, refreshing home data")
      var service = root.getTwitchService()
      if (service && service.authenticated) {
        service.refreshFollowedStreams()
        service.refreshRecommendedStreams()
      }
    }
    previousView = currentView
  }

  // Connexion pour rediriger vers home après déconnexion
  Connections {
    target: root.getTwitchService()
    enabled: root.getTwitchService() !== null
    function onAuthenticatedChanged(authenticated) {
      if (!authenticated) {
        console.log("[main.qml] User logged out, isAuthenticated will update automatically")
        currentView = "home"
      }
    }
  }
}
