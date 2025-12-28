import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import "themes/BlueTheme.js" as BlueTheme
import "components"

Item {
  id: loginRoot

  // Accès au service Twitch (disponible globalement via setContextProperty)
  // Quand chargé via Loader avec source, le contexte global est disponible
  // Utiliser une fonction pour éviter les problèmes de binding
  property var twitchServiceRef: (function() {
    var service = typeof twitchService !== "undefined" ? twitchService : null
    console.log("[LoginView] twitchServiceRef initialization - service:", service ? "EXISTS" : "NULL")
    return service
  })()
  
  Component.onCompleted: {
    console.log("[LoginView] Component.onCompleted - twitchServiceRef:", twitchServiceRef ? "EXISTS" : "NULL")
    console.log("[LoginView] Component.onCompleted - typeof twitchService:", typeof twitchService)
    // Essayer d'accéder directement aussi
    if (typeof twitchService !== "undefined") {
      console.log("[LoginView] Direct twitchService access:", twitchService ? "EXISTS" : "NULL")
      if (twitchService) {
        console.log("[LoginView] twitchService.authenticated:", twitchService.authenticated)
      }
    }
    if (twitchServiceRef) {
      console.log("[LoginView] twitchServiceRef.authenticated:", twitchServiceRef.authenticated)
    }
  }

  ColumnLayout {
    anchors.centerIn: parent
    anchors.verticalCenterOffset: -50
    spacing: BlueTheme.spacingLarge * 4
    width: Math.min(520, parent.width - BlueTheme.spacingLarge * 4)

    // Logo ou titre
    ColumnLayout {
      Layout.alignment: Qt.AlignHCenter
      spacing: BlueTheme.spacingMedium

      Text {
        text: "BluePlayer"
        font.family: BlueTheme.fontFamily
        font.pixelSize: 56
        font.weight: 600
        color: BlueTheme.primaryText
        horizontalAlignment: Text.AlignHCenter
        Layout.fillWidth: true
        Layout.bottomMargin: BlueTheme.spacingSmall
      }

      Text {
        text: qsTr("Native Twitch player")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 16
        font.weight: 400
        color: BlueTheme.mutedText
        horizontalAlignment: Text.AlignHCenter
        Layout.fillWidth: true
      }
    }

    // Carte de connexion
    Rectangle {
      Layout.fillWidth: true
      Layout.topMargin: BlueTheme.spacingLarge * 2
      Layout.minimumHeight: 300
      clip: true
      
      color: "#1b2130"
      radius: 8
      border.color: "#2a324e"
      border.width: 1
      
      property real cardPadding: BlueTheme.spacingLarge * 1.5
      
      ColumnLayout {
        id: cardContent
        anchors.fill: parent
        anchors.leftMargin: parent.cardPadding
        anchors.rightMargin: parent.cardPadding
        anchors.topMargin: parent.cardPadding
        anchors.bottomMargin: parent.cardPadding
        spacing: 0

        // Section titre simplifiée
        Text {
          text: qsTr("Login required")
          font.family: BlueTheme.fontFamily
          font.pixelSize: 22
          font.weight: 500
          color: BlueTheme.primaryText
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignHCenter
          horizontalAlignment: Text.AlignHCenter
          wrapMode: Text.WordWrap
        }

        // Texte descriptif avec meilleur espacement
        Text {
          text: qsTr("To access your followed streams and enjoy all BluePlayer features, please sign in with your Twitch account.")
          font.family: BlueTheme.fontFamily
          font.pixelSize: 15
          font.weight: 400
          color: BlueTheme.secondaryText
          wrapMode: Text.WordWrap
          lineHeight: 1.6
          Layout.fillWidth: true
          Layout.topMargin: BlueTheme.spacingMedium
          Layout.alignment: Qt.AlignHCenter
          horizontalAlignment: Text.AlignHCenter
        }

        Item {
          Layout.fillHeight: true
          Layout.minimumHeight: BlueTheme.spacingLarge
        }

        // Bouton avec accent color
        Rectangle {
          Layout.fillWidth: true
          Layout.preferredHeight: 50
          color: BlueTheme.accent
          radius: 14
          
          MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              // Essayer d'abord twitchServiceRef, puis accès direct
              var service = loginRoot.twitchServiceRef
              if (!service && typeof twitchService !== "undefined") {
                service = twitchService
              }
              if (service) {
                console.log("[LoginView] Logging in...")
                service.login()
              } else {
                console.warn("[LoginView] WARNING: twitchService is not available when button clicked")
              }
            }
          }
          
          Text {
            anchors.centerIn: parent
            text: qsTr("Sign in with Twitch")
            font.family: BlueTheme.fontFamily
            font.pixelSize: 15
            font.weight: 600
            color: "#03050b"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
          }
        }

        // Message d'erreur si nécessaire
        Text {
          id: errorText
          visible: false
          text: ""
          font.family: BlueTheme.fontFamily
          font.pixelSize: 12
          color: BlueTheme.statusNegative
          wrapMode: Text.WordWrap
          Layout.fillWidth: true
          Layout.topMargin: BlueTheme.spacingMedium
          horizontalAlignment: Text.AlignHCenter
        }
      }
    }

    // Connexion pour gérer les erreurs - seulement si twitchServiceRef est disponible
    Connections {
      id: twitchConnections
      target: loginRoot.twitchServiceRef
      enabled: loginRoot.twitchServiceRef !== null && loginRoot.twitchServiceRef !== undefined
      function onErrorOccurred(message) {
        console.log("[LoginView] Error:", message)
        errorText.text = message
        errorText.visible = true
      }
      function onAuthenticatedChanged(authenticated) {
        if (authenticated) {
          console.log("[LoginView] User authenticated, view will be switched")
          errorText.visible = false
        }
      }
    }
  }
}

