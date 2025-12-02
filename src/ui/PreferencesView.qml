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
    ColumnLayout {
      id: prefsLayout
      anchors.fill: parent
      anchors.margins: AppleTheme.spacingLarge
      spacing: AppleTheme.spacingLarge

      RowLayout {
        Layout.fillWidth: true
        spacing: AppleTheme.spacingSmall

        Label {
          text: qsTr("Préférences")
          font.pixelSize: 24
          font.bold: true
          color: AppleTheme.primaryText
        }

        Item { Layout.fillWidth: true }

        AppleButton {
          text: qsTr("Revenir à l'accueil")
          onClicked: closeRequested()
          filled: false
        }
      }

      // Section Authentification Twitch
      AppleCard {
        Layout.fillWidth: true
        ColumnLayout {
          spacing: AppleTheme.spacingMedium

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
          
          property var twitchService: getTwitchService()

          Text {
            text: {
              var service = getTwitchService()
              return service && service.authenticated ? 
                qsTr("Vous êtes connecté à Twitch.") : 
                qsTr("Vous n'êtes pas connecté à Twitch.")
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
            Layout.fillWidth: true
            filled: {
              var service = getTwitchService()
              return service && service.authenticated ? false : true
            }
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

      AppleCard {
        Layout.fillWidth: true
        ColumnLayout {
          spacing: AppleTheme.spacingMedium

          Label {
            text: qsTr("Confort visuel")
            font.pixelSize: 16
            font.bold: true
            color: AppleTheme.primaryText
          }

          RowLayout {
            spacing: AppleTheme.spacingMedium
            Switch { checked: true }
            Label {
              text: qsTr("Activer l’apparence sombre")
              color: AppleTheme.secondaryText
            }
          }

          RowLayout {
            spacing: AppleTheme.spacingMedium
            Slider {
              Layout.fillWidth: true
              from: 0
              to: 100
              value: 32
            }
            Label {
              text: qsTr("Réactivité des animations")
              color: AppleTheme.secondaryText
            }
          }
        }
      }

      AppleCard {
        Layout.fillWidth: true
        ColumnLayout {
          spacing: AppleTheme.spacingMedium

          Label {
            text: qsTr("Lecture")
            font.pixelSize: 16
            font.bold: true
            color: AppleTheme.primaryText
          }

          RowLayout {
            spacing: AppleTheme.spacingMedium
            Switch { checked: true }
            Label {
              text: qsTr("Notifications automatiques")
              color: AppleTheme.secondaryText
            }
          }

          Text {
            text: qsTr("Les réglages ici sont symboliques pour la vue préférences.")
            color: AppleTheme.mutedText
            font.pixelSize: 12
            wrapMode: Text.WordWrap
          }
        }
      }

      AppleButton {
        text: qsTr("Fermer et revenir à l’accueil")
        onClicked: closeRequested()
        Layout.fillWidth: true
      }
    }
  }
}

