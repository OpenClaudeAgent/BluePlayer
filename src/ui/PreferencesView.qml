import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import "themes/AppleTheme.js" as AppleTheme
import "components"

Item {
  id: preferencesRoot
  signal closeRequested()

  Rectangle {
    anchors.fill: parent
    color: AppleTheme.windowBackground
    opacity: 0.98
  }

  ScrollView {
    anchors.fill: parent
    contentWidth: availableWidth
    ColumnLayout {
      id: prefsLayout
      width: parent.width
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.margins: AppleTheme.spacingLarge
      spacing: AppleTheme.spacingLarge

      Label {
        text: qsTr("Préférences")
        font.pixelSize: 24
        font.bold: true
        color: AppleTheme.primaryText
      }

      // Section Authentification Twitch
      RowLayout {
        Layout.fillWidth: true
        Item { Layout.fillWidth: true }
        AppleCard {
          Layout.maximumWidth: 600
          Layout.fillWidth: false
          ColumnLayout {
            spacing: AppleTheme.spacingMedium
            anchors.margins: 0

            Label {
              text: qsTr("Authentification Twitch")
              font.pixelSize: 16
              font.bold: true
              color: AppleTheme.primaryText
            }

            // Accès au service Twitch (disponible globalement)
            function getTwitchService() {
              return typeof twitchService !== "undefined" ? twitchService : null
            }

            Text {
              text: {
                var service = getTwitchService()
                if (service && service.authenticated) {
                  var userName = service.userName || ""
                  return userName ? 
                    qsTr("Vous êtes connecté à Twitch en tant que %1.").arg(userName) : 
                    qsTr("Vous êtes connecté à Twitch.")
                } else {
                  return qsTr("Vous n'êtes pas connecté à Twitch.")
                }
              }
              color: {
                var service = getTwitchService()
                return service && service.authenticated ? 
                  AppleTheme.statusPositive : 
                  AppleTheme.secondaryText
              }
              font.pixelSize: 14
              wrapMode: Text.WordWrap
              Layout.fillWidth: true
            }

            RowLayout {
              Layout.fillWidth: true
              Item { Layout.fillWidth: true }
              AppleButton {
                text: {
                  var service = getTwitchService()
                  return service && service.authenticated ? 
                    qsTr("Se déconnecter") : 
                    qsTr("Se connecter à Twitch")
                }
                onClicked: {
                  var service = getTwitchService()
                  if (service) {
                    if (service.authenticated) {
                      console.log("[PreferencesView] Logging out...")
                      service.logout()
                      // Rediriger vers l'accueil après déconnexion
                      preferencesRoot.closeRequested()
                    } else {
                      console.log("[PreferencesView] Logging in...")
                      service.login()
                    }
                  } else {
                    console.log("[PreferencesView] ERROR: twitchService is not available")
                  }
                }
                filled: {
                  var service = getTwitchService()
                  return service && service.authenticated ? false : true
                }
              }
            }
            
            // Espace en bas pour le padding
            Item {
              Layout.preferredHeight: AppleTheme.spacingMedium
            }

            // Connexion pour fermer automatiquement les préférences après déconnexion
            Connections {
              target: (function() {
                return typeof twitchService !== "undefined" ? twitchService : null
              })()
              enabled: typeof twitchService !== "undefined" && twitchService !== null
              function onAuthenticatedChanged(authenticated) {
                if (!authenticated) {
                  console.log("[PreferencesView] User logged out, closing preferences")
                  preferencesRoot.closeRequested()
                }
              }
            }
          }
        }
        Item { Layout.fillWidth: true }
      }

      RowLayout {
        Layout.fillWidth: true
        Item { Layout.fillWidth: true }
        AppleButton {
          text: qsTr("Fermer et revenir à l'accueil")
          onClicked: closeRequested()
        }
        Item { Layout.fillWidth: true }
      }
    }
  }
}

