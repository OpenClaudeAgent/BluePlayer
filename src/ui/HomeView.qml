import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import BluePlayer.UI 1.0

import "themes/AppleTheme.js" as AppleTheme
import "components"

Item {
  id: homeRoot

  // Accès au service Twitch (disponible globalement)
  // Utiliser une fonction pour éviter les boucles de binding
  function getTwitchService() {
    return typeof twitchService !== "undefined" ? twitchService : null
  }

  // HomeViewModel pour gérer la logique métier
  HomeViewModel {
    id: viewModel
  }

  // Utiliser sectionsData du ViewModel au lieu de la propriété locale
  property var sectionsData: viewModel.sectionsData

  // Gestion du focus : perdre le focus quand on clique ailleurs
  Keys.onPressed: function(event) {
    if (event.key === Qt.Key_Escape && searchField.activeFocus) {
      searchField.focus = false
      event.accepted = true
    }
  }
  
  focus: true

  // Timer pour debouncing de la recherche (300ms)
  Timer {
    id: searchDebounceTimer
    interval: 300
    onTriggered: {
      if (searchField.text.length > 0) {
        console.log("[HomeView] Recherche déclenchée après debounce:", searchField.text)
        // TODO: Implémenter la recherche réelle ici
        // Exemple: twitchService.searchStreams(searchField.text)
      }
    }
  }

  // Connexion au service Twitch pour mettre à jour les streams via le ViewModel
  Connections {
    id: twitchConnections
    target: (function() {
      return typeof twitchService !== "undefined" ? twitchService : null
    })()
    enabled: typeof twitchService !== "undefined" && twitchService !== null
    function onStreamsChanged() {
      console.log("[HomeView] onStreamsChanged() signal received")
      var service = getTwitchService()
      if (service && service.streams) {
        viewModel.updateFollowedStreams(service.streams)
      }
    }
    function onErrorOccurred(message) {
      console.log("[HomeView] ERROR Twitch:", message)
    }
    function onAuthenticatedChanged(authenticated) {
      console.log("[HomeView] onAuthenticatedChanged() signal received, authenticated:", authenticated)
    }
  }

  Component.onCompleted: {
    console.log("[HomeView] Component.onCompleted()")
    var service = getTwitchService()
    console.log("[HomeView] twitchService:", service ? "EXISTS" : "NULL")
    if (service) {
      console.log("[HomeView] twitchService.authenticated:", service.authenticated)
      console.log("[HomeView] twitchService.streams:", service.streams ? service.streams.length + " streams" : "null")
      if (service.streams) {
        viewModel.updateFollowedStreams(service.streams)
      }
    }
    console.log("[HomeView] ViewModel followedStreams length:", viewModel.followedStreams.length)
    console.log("[HomeView] ViewModel placeholderCards length:", viewModel.placeholderCards.length)
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 0

    // Barre de recherche fixe en haut
    Rectangle {
      id: searchBarContainer
      Layout.fillWidth: true
      Layout.preferredHeight: 88
      color: "transparent"
      z: 10

      Rectangle {
        id: searchBarBackground
        width: Math.min(400, (parent.width - AppleTheme.spacingLarge * 2) / 2)
        height: 52
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        radius: 26
        color: searchField.activeFocus ? "#1a2330" : AppleTheme.surfaceSoft
        border.color: searchField.activeFocus ? "#5a6578" : AppleTheme.divider
        border.width: searchField.activeFocus ? 1.5 : AppleTheme.borderWidth

        // Effet de glow au focus (gris subtil)
        Rectangle {
          anchors.fill: parent
          anchors.margins: -3
          radius: parent.radius + 3
          color: "transparent"
          border.color: searchField.activeFocus ? Qt.rgba(0.35, 0.39, 0.47, 0.15) : "transparent"
          border.width: 3
          z: -1
          opacity: searchField.activeFocus ? 1 : 0
          Behavior on opacity {
            NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
          }
        }

        // Ombre subtile
        Rectangle {
          anchors.fill: parent
          anchors.margins: -1
          radius: parent.radius + 1
          color: "transparent"
          border.color: "#00000025"
          border.width: 1
          z: -2
        }

        Behavior on color {
          ColorAnimation { duration: 200; easing.type: Easing.OutCubic }
        }
        Behavior on border.color {
          ColorAnimation { duration: 200; easing.type: Easing.OutCubic }
        }

        RowLayout {
          anchors.fill: parent
          anchors.leftMargin: 22
          anchors.rightMargin: 22
          anchors.topMargin: 14
          anchors.bottomMargin: 14
          spacing: 18

          // Icône de loupe
          Text {
            text: "🔍"
            font.pixelSize: 17
            color: searchField.activeFocus ? "#5a6578" : AppleTheme.secondaryText
            Layout.alignment: Qt.AlignVCenter
            Behavior on color {
              ColorAnimation { duration: 200; easing.type: Easing.OutCubic }
            }
          }

          // Item wrapper pour mieux contrôler l'espacement vertical
          Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            TextField {
              id: searchField
              anchors.left: parent.left
              anchors.right: parent.right
              anchors.verticalCenter: parent.verticalCenter
              height: 24
              verticalAlignment: Text.AlignVCenter
              placeholderText: qsTr("Rechercher...")
              placeholderTextColor: searchField.activeFocus ? Qt.rgba(0.49, 0.54, 0.64, 0.6) : AppleTheme.mutedText
              font.family: AppleTheme.fontFamily
              font.pixelSize: 15
              color: AppleTheme.primaryText
              cursorVisible: activeFocus
              background: Rectangle { 
                color: "transparent"
                anchors.fill: parent
              }
              selectByMouse: true
              leftPadding: 0
              rightPadding: 0
              topPadding: text.length > 0 ? 6 : 0
              bottomPadding: 0
              Behavior on topPadding {
                NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
              }
              onTextChanged: {
                // Annuler le timer précédent si l'utilisateur tape encore
                searchDebounceTimer.stop()
                // Redémarrer le timer pour attendre 300ms après la dernière frappe
                if (text.length > 0) {
                  searchDebounceTimer.start()
                }
              }
              onAccepted: {
                // Recherche immédiate si l'utilisateur appuie sur Entrée
                searchDebounceTimer.stop()
                console.log("[HomeView] Recherche immédiate:", text)
                // TODO: Implémenter la recherche réelle ici
                // Exemple: twitchService.searchStreams(text)
              }
              Keys.onEscapePressed: {
                searchDebounceTimer.stop()
                focus = false
              }
            }
          }
        }
      }
    }

    // Zone de contenu scrollable verticalement
    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
      ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

      ColumnLayout {
        id: contentLayout
        width: parent.width
        spacing: 32
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: AppleTheme.spacingLarge
        
        // MouseArea pour perdre le focus sans bloquer le scroll
        MouseArea {
          anchors.fill: parent
          z: -1
          hoverEnabled: false
          propagateComposedEvents: true
          onClicked: function(mouse) {
            if (searchField.activeFocus) {
              searchField.focus = false
            }
            mouse.accepted = false
          }
        }

        // Répéter pour chaque section
        Repeater {
          model: homeRoot.sectionsData
          delegate: HorizontalRowSection {
            Layout.fillWidth: true
            Layout.topMargin: Repeater.index === 0 ? AppleTheme.spacingLarge * 2 : 0
            sectionTitle: modelData.title
            sectionSubtitle: modelData.subtitle
            cardsModel: modelData.cards
            rowHeight: 220
            cardWidth: 180
            cardSpacing: 16
          }
        }

        // Espace en bas pour le scroll
        Item {
          Layout.preferredHeight: AppleTheme.spacingLarge * 2
        }
      }
    }
  }
}
