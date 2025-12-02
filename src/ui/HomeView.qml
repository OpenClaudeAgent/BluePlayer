import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import "themes/AppleTheme.js" as AppleTheme
import "components"

Item {
  id: homeRoot

  property var sectionsData: [
    {
      title: qsTr("Vos streamers suivis"),
      subtitle: qsTr("Reprenez là où vous vous êtes arrêtés"),
      cards: [
        { name: "LumaCloud", detail: qsTr("Speedrun & chill"), viewers: qsTr("1 240 viewers") },
        { name: "NoraPixel", detail: qsTr("Créations artistiques"), viewers: qsTr("845 viewers") },
        { name: "BoraTech", detail: qsTr("Tech & hardware"), viewers: qsTr("620 viewers") },
        { name: "DeepSeaTV", detail: qsTr("Découverte sous-marine"), viewers: qsTr("410 viewers") },
        { name: "StreamMaster", detail: qsTr("Gaming compétitif"), viewers: qsTr("2 150 viewers") },
        { name: "ArtVibes", detail: qsTr("Création digitale"), viewers: qsTr("890 viewers") }
      ]
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

  ColumnLayout {
    anchors.fill: parent
    spacing: 0

    // Barre de recherche fixe en haut
    Rectangle {
      id: searchBarContainer
      Layout.fillWidth: true
      Layout.preferredHeight: 80
      color: "transparent"
      z: 10

      Rectangle {
        id: searchBarBackground
        width: Math.min(800, parent.width - AppleTheme.spacingLarge * 2)
        height: 56
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        radius: 28
        color: AppleTheme.surfaceSoft
        border.color: "#2d3644"
        border.width: AppleTheme.borderWidth

        // Effet de blur subtil (simulé avec gradient)
        gradient: Gradient {
          GradientStop { position: 0; color: "#1f2735" }
          GradientStop { position: 1; color: AppleTheme.surfaceSoft }
        }

        // Ombre subtile
        Rectangle {
          anchors.fill: parent
          anchors.margins: -1
          radius: parent.radius + 1
          color: "transparent"
          border.color: "#00000015"
          border.width: 1
          z: -1
        }

        RowLayout {
          anchors.fill: parent
          anchors.margins: 8
          spacing: AppleTheme.spacingMedium

          // Icône de recherche
          Text {
            text: "🔍"
            font.pixelSize: 20
            Layout.leftMargin: AppleTheme.spacingMedium
          }

          TextField {
            id: searchField
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height
            verticalAlignment: Text.AlignVCenter
            placeholderText: qsTr("Rechercher un streamer, un tag, un jeu...")
            placeholderTextColor: AppleTheme.secondaryText
            font.family: AppleTheme.fontFamily
            font.pixelSize: 16
            color: AppleTheme.primaryText
            cursorVisible: true
            background: Rectangle { color: "transparent" }
            onAccepted: {
              console.log("Recherche :", text)
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

        // Répéter pour chaque section
        Repeater {
          model: homeRoot.sectionsData
          delegate: HorizontalRowSection {
            Layout.fillWidth: true
            Layout.topMargin: Repeater.index === 0 ? AppleTheme.spacingMedium : 0
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
