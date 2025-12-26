import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15


import "themes/AppleTheme.js" as AppleTheme
import "components"

ApplicationWindow {
  id: root
  visible: true
  width: 1200
  height: 780
  color: AppleTheme.windowBackground
  font.family: AppleTheme.fontFamily
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
      GradientStop { position: 0; color: AppleTheme.gradientStart }
      GradientStop { position: 1; color: AppleTheme.gradientEnd }
    }
  }

  property bool preferencesActive: currentView === "preferences"
  property bool cacheActive: currentView === "cache"

  // Conteneur des boutons header (en haut à droite)
  Row {
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.margins: AppleTheme.spacingSmall
    z: 10
    spacing: 8
    visible: {
      var service = root.getTwitchService()
      if (!service || !service.authenticated) {
        return false
      }
      return currentView === "home" || currentView === "preferences" || currentView === "cache"
    }

    // Bouton Replays
    Rectangle {
      id: replaysIcon
      width: 38
      height: 38
      radius: width / 2
      color: cacheActive ? AppleTheme.overlayTint : (replaysMouseArea.containsMouse ? AppleTheme.surfaceSoft : AppleTheme.surface)
      border.color: cacheActive ? AppleTheme.accent : AppleTheme.buttonBorder
      border.width: AppleTheme.borderWidth

      Text {
        anchors.centerIn: parent
        text: "\u21BA"
        font.pixelSize: 20
        font.bold: true
        color: AppleTheme.primaryText
      }

      MouseArea {
        id: replaysMouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: currentView = cacheActive ? "home" : "cache"
      }

      ToolTip {
        visible: replaysMouseArea.containsMouse
        text: {
          if (typeof cacheManager !== "undefined" && cacheManager) {
            return qsTr("Mes Replays (%1)").arg(cacheManager.vodCount)
          }
          return qsTr("Mes Replays")
        }
        delay: 500
      }
    }

    // Bouton Préférences
    Rectangle {
      id: preferencesIcon
      width: 38
      height: 38
      radius: width / 2
      color: preferencesActive ? AppleTheme.overlayTint : (prefsMouseArea.containsMouse ? AppleTheme.surfaceSoft : AppleTheme.surface)
      border.color: preferencesActive ? AppleTheme.accent : AppleTheme.buttonBorder
      border.width: AppleTheme.borderWidth

      Label {
        anchors.centerIn: parent
        text: "\u2699"
        font.pixelSize: 18
        color: AppleTheme.primaryText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
      }

      MouseArea {
        id: prefsMouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: currentView = preferencesActive ? "home" : "preferences"
      }

      ToolTip {
        visible: prefsMouseArea.containsMouse
        text: qsTr("Préférences")
        delay: 500
      }
    }
  }

  // Player view - full screen, no margins (outside ColumnLayout)
  Loader {
    id: playerLoader
    anchors.fill: parent
    visible: currentView === "player"
    active: currentView === "player"
    source: "PlayerView.qml"
    
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

  // Other views - with margins and scroll
  ColumnLayout {
    anchors.fill: parent
    anchors.margins: AppleTheme.spacingLarge
    spacing: AppleTheme.spacingMedium
    visible: currentView !== "player"

    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true

      Loader {
        id: pageLoader
        anchors.fill: parent
        active: currentView !== "player"
        source: {
          // Si twitchService n'existe pas ou si l'utilisateur n'est pas authentifié, afficher la vue de connexion
          var service = root.getTwitchService()
          console.log("[main.qml] pageLoader.source - twitchService:", service ? "EXISTS" : "NULL")
          if (!service || !service.authenticated) {
            console.log("[main.qml] Loading LoginView.qml")
            return "LoginView.qml"
          }
          // Sinon, afficher la vue normale selon currentView
          console.log("[main.qml] Loading view:", currentView)
          if (currentView === "home") return "HomeView.qml"
          if (currentView === "preferences") return "PreferencesView.qml"
          if (currentView === "cache") return "CacheManagerView.qml"
          return "HomeView.qml"
        }
        Behavior on opacity {
          NumberAnimation { duration: 220 }
        }
        
        // Passer les propriétés au composant chargé
        onItemChanged: {
          console.log("[main.qml] onItemChanged - currentView:", currentView)
          if (item) {
            if (item.hasOwnProperty("openStreamPlayer")) {
              console.log("[main.qml] Connecting openStreamPlayer signal")
              item.openStreamPlayer.connect(function(login, name, title, thumbnailUrl) {
                console.log("[main.qml] openStreamPlayer signal received:")
                console.log("[main.qml]   login:", login)
                console.log("[main.qml]   name:", name)
                console.log("[main.qml]   title:", title)
                console.log("[main.qml]   thumbnailUrl:", thumbnailUrl)
                root.playerStreamerLogin = login
                root.playerStreamerName = name
                root.playerStreamTitle = title
                root.playerStreamThumbnailUrl = thumbnailUrl
                root.currentView = "player"
              })
            }
            // CacheManagerView connections
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
                console.log("[main.qml] playVodRequested:", id, filePath)
                root.vodId = id
                root.vodFilePath = filePath
                root.vodMetadata = metadata
                // Set player info from VOD metadata
                root.playerStreamerLogin = ""
                root.playerStreamerName = metadata.streamerName || ""
                root.playerStreamTitle = metadata.streamTitle || ""
                // Mark as played
                if (cacheManager) {
                  cacheManager.markAsPlayed(id)
                }
                // Navigate to player
                root.currentView = "player"
              })
            }
            // HomeView - connect to open cache view
            if (item.hasOwnProperty("openCacheManager")) {
              item.openCacheManager.connect(function() {
                console.log("[main.qml] Opening cache manager")
                root.currentView = "cache"
              })
            }
          }
        }
      }
    }
  }

  Component {
    id: loginComponent
    LoginView {
      // Passer twitchService explicitement au Component pour éviter les problèmes de contexte
      twitchServiceRef: {
        console.log("[main.qml] loginComponent - twitchService:", twitchService ? "EXISTS" : "NULL")
        return twitchService
      }
    }
  }

  Component {
    id: homeComponent
    HomeView {
    }
  }

  Component {
    id: preferencesComponent
    PreferencesView {
      onCloseRequested: currentView = "home"
    }
  }



  // Connexion pour rediriger vers LoginView après déconnexion
  Connections {
    target: root.getTwitchService()
    enabled: root.getTwitchService() !== null
    function onAuthenticatedChanged(authenticated) {
      if (!authenticated) {
        console.log("[main.qml] User logged out, redirecting to LoginView")
        // Mettre currentView à "home" pour forcer le rechargement
        // Le Loader chargera automatiquement LoginView car authenticated est false
        currentView = "home"
      }
    }
  }
}
