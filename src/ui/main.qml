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

  Rectangle {
    id: preferencesIcon
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.margins: AppleTheme.spacingSmall
    z: 10
    width: 38
    height: 38
    radius: width / 2
    color: preferencesActive ? AppleTheme.overlayTint : AppleTheme.surface
    border.color: preferencesActive ? AppleTheme.accent : AppleTheme.buttonBorder
    border.width: AppleTheme.borderWidth
    // Masquer l'icône de préférences sur la vue de connexion ou si l'utilisateur n'est pas authentifié
    visible: {
      // Si twitchService n'existe pas ou n'est pas authentifié, masquer (vue de connexion)
      var service = root.getTwitchService()
      if (!service || !service.authenticated) {
        return false
      }
      // Sinon, afficher uniquement sur home et preferences
      return currentView === "home" || currentView === "preferences"
    }

    Label {
      anchors.centerIn: parent
      text: "\u2699"
      font.pixelSize: 18
      color: AppleTheme.primaryText
      horizontalAlignment: Text.AlignHCenter
      verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor
      onClicked: currentView = preferencesActive ? "home" : "preferences"
    }

    ToolTip {
      text: qsTr("Préférences")
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
        var service = root.getTwitchService()
        item.twitchService = service
        item.streamerLogin = root.playerStreamerLogin
        item.streamerName = root.playerStreamerName
        item.streamTitle = root.playerStreamTitle
        item.backRequested.connect(function() {
          console.log("[main.qml] Back requested, returning to home")
          root.currentView = "home"
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
          return currentView === "home" ? "HomeView.qml" : "PreferencesView.qml"
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
              item.openStreamPlayer.connect(function(login, name, title) {
                console.log("[main.qml] openStreamPlayer signal received:")
                console.log("[main.qml]   login:", login)
                console.log("[main.qml]   name:", name)
                console.log("[main.qml]   title:", title)
                root.playerStreamerLogin = login
                root.playerStreamerName = name
                root.playerStreamTitle = title
                root.currentView = "player"
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
