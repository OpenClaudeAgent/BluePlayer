import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import "themes/AppleTheme.js" as AppleTheme
import "components"

Item {
  id: homeRoot

  // Accès au service Twitch (disponible globalement)
  // Utiliser une fonction pour éviter les boucles de binding
  function getTwitchService() {
    return typeof twitchService !== "undefined" ? twitchService : null
  }

  // Génération de 20 cartes placeholder
  property var placeholderCards: (function() {
    var cards = [];
    for (var i = 0; i < 20; i++) {
      cards.push({ isPlaceholder: true });
    }
    console.log("[HomeView] Generated", cards.length, "placeholder cards")
    return cards;
  })()

  // Transformation des données Twitch en format compatible avec StreamCard
  function transformTwitchStreams(twitchStreams) {
    console.log("[HomeView] transformTwitchStreams() called with", twitchStreams ? twitchStreams.length : 0, "streams")
    if (!twitchStreams || twitchStreams.length === 0) {
      console.log("[HomeView] No streams to transform")
      return [];
    }
    
    var transformed = [];
    for (var i = 0; i < twitchStreams.length; i++) {
      var stream = twitchStreams[i];
      var viewerCount = stream.viewer_count || 0;
      var viewerText = viewerCount.toLocaleString(Qt.locale(), 'f', 0) + " viewers";
      
      transformed.push({
        name: stream.user_name || "",
        detail: stream.title || "",
        viewers: viewerText,
        previewImage: stream.thumbnail_url || "",
        streamUrl: stream.stream_url || ""
      });
      console.log("[HomeView] Transformed stream:", stream.user_name || "UNKNOWN")
    }
    console.log("[HomeView] Transformed", transformed.length, "streams")
    return transformed;
  }

  // Streams suivis transformés depuis l'API Twitch
  property var followedStreams: []
  
  function updateFollowedStreams() {
    var service = getTwitchService()
    var streams = []
    if (service && service.streams) {
      console.log("[HomeView] updateFollowedStreams: twitchService.streams length =", service.streams.length)
      streams = transformTwitchStreams(service.streams)
    } else {
      console.log("[HomeView] updateFollowedStreams: No twitchService or no streams")
    }
    console.log("[HomeView] updateFollowedStreams =", streams.length, "streams")
    followedStreams = streams
  }

  // Mise à jour des sections avec les données réelles ou les placeholders
  property var sectionsData: {
    var cards = followedStreams.length > 0 ? followedStreams : placeholderCards
    console.log("[HomeView] sectionsData binding: using", cards.length, "cards (followedStreams:", followedStreams.length, ", placeholders:", placeholderCards.length, ")")
    return [
      {
        title: qsTr("Vos streamers suivis"),
        subtitle: qsTr("Reprenez là où vous vous êtes arrêtés"),
        cards: cards
      },
    {
      title: qsTr("Recommandé pour vous"),
      subtitle: qsTr("Basé sur vos préférences"),
      cards: [
        { name: "AuroraPlay", detail: qsTr("Aventure narrative"), viewers: qsTr("310 viewers") },
        { name: "ZenGarden", detail: qsTr("ASMR & mindfulness"), viewers: qsTr("480 viewers") },
        { name: "NeoArena", detail: qsTr("Jeux compétitifs"), viewers: qsTr("1 050 viewers") },
        { name: "FluxLuxe", detail: qsTr("Talk-show premium"), viewers: qsTr("690 viewers") },
        { name: "PixelCraft", detail: qsTr("Création de jeux"), viewers: qsTr("520 viewers") },
        { name: "RetroWave", detail: qsTr("Musique rétro"), viewers: qsTr("380 viewers") }
      ]
    },
    {
      title: qsTr("En direct maintenant"),
      subtitle: qsTr("Les streams les plus populaires"),
      cards: [
        { name: "EpicGamer", detail: qsTr("Tournoi esport"), viewers: qsTr("5 240 viewers") },
        { name: "CreativeHub", detail: qsTr("Design & illustration"), viewers: qsTr("3 890 viewers") },
        { name: "MusicLive", detail: qsTr("Concert en direct"), viewers: qsTr("2 670 viewers") },
        { name: "TechTalk", detail: qsTr("Débat technologique"), viewers: qsTr("1 950 viewers") },
        { name: "FoodieStream", detail: qsTr("Cuisine en direct"), viewers: qsTr("1 420 viewers") }
      ]
    },
    {
      title: qsTr("Populaire cette semaine"),
      subtitle: qsTr("Les tendances du moment"),
      cards: [
        { name: "GamingPro", detail: qsTr("Speedrun record"), viewers: qsTr("8 500 viewers") },
        { name: "ArtStudio", detail: qsTr("Création en temps réel"), viewers: qsTr("6 200 viewers") },
        { name: "MusicFest", detail: qsTr("Festival virtuel"), viewers: qsTr("4 800 viewers") },
        { name: "TechReview", detail: qsTr("Tests produits"), viewers: qsTr("3 100 viewers") },
        { name: "CookingShow", detail: qsTr("Recettes gourmandes"), viewers: qsTr("2 600 viewers") }
      ]
    }
    ]
  }

  // Gestion du focus : perdre le focus quand on clique ailleurs
  Keys.onPressed: function(event) {
    if (event.key === Qt.Key_Escape && searchField.activeFocus) {
      searchField.focus = false
      event.accepted = true
    }
  }
  
  focus: true

  // Connexion au service Twitch pour mettre à jour les streams
  Connections {
    id: twitchConnections
    target: (function() {
      return typeof twitchService !== "undefined" ? twitchService : null
    })()
    enabled: typeof twitchService !== "undefined" && twitchService !== null
    function onStreamsChanged() {
      console.log("[HomeView] onStreamsChanged() signal received")
      homeRoot.updateFollowedStreams()
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
      homeRoot.updateFollowedStreams()
    }
    console.log("[HomeView] followedStreams length:", followedStreams.length)
    console.log("[HomeView] placeholderCards length:", placeholderCards.length)
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
              onAccepted: {
                console.log("Recherche :", text)
              }
              Keys.onEscapePressed: {
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
