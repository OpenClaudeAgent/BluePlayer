import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import "themes/AppleTheme.js" as AppleTheme
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
    spacing: AppleTheme.spacingLarge * 4
    width: Math.min(520, parent.width - AppleTheme.spacingLarge * 4)

    // Logo ou titre
    ColumnLayout {
      Layout.alignment: Qt.AlignHCenter
      spacing: AppleTheme.spacingMedium

      Text {
        text: "BluePlayer"
        font.family: AppleTheme.fontFamily
        font.pixelSize: 56
        font.weight: 600
        color: AppleTheme.primaryText
        horizontalAlignment: Text.AlignHCenter
        Layout.fillWidth: true
        Layout.bottomMargin: AppleTheme.spacingSmall
      }

      Text {
        text: qsTr("Lecteur Twitch natif")
        font.family: AppleTheme.fontFamily
        font.pixelSize: 16
        font.weight: 400
        color: AppleTheme.mutedText
        horizontalAlignment: Text.AlignHCenter
        Layout.fillWidth: true
      }
    }

    // Carte de connexion
    Rectangle {
      Layout.fillWidth: true
      Layout.topMargin: AppleTheme.spacingLarge * 2
      Layout.minimumHeight: 300
      clip: true
      
      color: "#1b2130"
      radius: 8
      border.color: "#2a324e"
      border.width: 1
      
      property real cardPadding: AppleTheme.spacingLarge * 1.5
      
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
          text: qsTr("Connexion requise")
          font.family: AppleTheme.fontFamily
          font.pixelSize: 22
          font.weight: 500
          color: AppleTheme.primaryText
          Layout.fillWidth: true
          Layout.alignment: Qt.AlignHCenter
          horizontalAlignment: Text.AlignHCenter
          wrapMode: Text.WordWrap
        }

        // Texte descriptif avec meilleur espacement
        Text {
          text: qsTr("Pour accéder à vos streams suivis et profiter de toutes les fonctionnalités de BluePlayer, connectez-vous avec votre compte Twitch.")
          font.family: AppleTheme.fontFamily
          font.pixelSize: 15
          font.weight: 400
          color: AppleTheme.secondaryText
          wrapMode: Text.WordWrap
          lineHeight: 1.6
          Layout.fillWidth: true
          Layout.topMargin: AppleTheme.spacingMedium
          Layout.alignment: Qt.AlignHCenter
          horizontalAlignment: Text.AlignHCenter
        }

        Item {
          Layout.fillHeight: true
          Layout.minimumHeight: AppleTheme.spacingLarge
        }

        // Bouton avec accent color
        Rectangle {
          Layout.fillWidth: true
          Layout.preferredHeight: 50
          color: AppleTheme.accent
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
            text: qsTr("Se connecter à Twitch")
            font.family: AppleTheme.fontFamily
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
          font.family: AppleTheme.fontFamily
          font.pixelSize: 12
          color: AppleTheme.statusNegative
          wrapMode: Text.WordWrap
          Layout.fillWidth: true
          Layout.topMargin: AppleTheme.spacingMedium
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

