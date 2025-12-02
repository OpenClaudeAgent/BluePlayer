import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtMultimedia 6.5

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

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: AppleTheme.spacingLarge
    spacing: AppleTheme.spacingMedium

    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true

      Loader {
        id: pageLoader
        anchors.fill: parent
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

  Connections {
    target: ffmpegService
    function onErrorOccurred(message) {
      statusText = message
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
