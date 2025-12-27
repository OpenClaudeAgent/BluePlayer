import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

Item {
  id: cardRoot
  property string categoryName: ""
  property string categoryId: ""
  property string boxArtUrl: ""
  property bool isPlaceholder: false

  // Dimensions adaptées pour les box art Twitch (ratio 285x380 ≈ 0.75)
  // Largeur standard, hauteur augmentée pour mieux afficher les box art
  implicitWidth: 180
  implicitHeight: 260

  Rectangle {
    id: cardBackground
    anchors.fill: parent
    radius: 12
    color: cardRoot.isPlaceholder ? BlueTheme.surfaceSoft : BlueTheme.surface
    border.color: BlueTheme.divider
    border.width: 1

    // Hover effect
    states: [
      State {
        name: "hovered"
        when: mouseArea.containsMouse
        PropertyChanges {
          target: cardBackground
          color: cardRoot.isPlaceholder ? "#252d3d" : "#1a2330"
          scale: 1.02
        }
        PropertyChanges {
          target: cardShadow
          opacity: 0.3
        }
      }
    ]

    transitions: Transition {
      NumberAnimation {
        properties: "scale, opacity"
        duration: BlueTheme.animCardDuration
        easing.type: Easing.OutCubic
      }
      ColorAnimation {
        duration: BlueTheme.animCardDuration
        easing.type: Easing.OutCubic
      }
    }

    // Ombre subtile
    Rectangle {
      id: cardShadow
      anchors.fill: parent
      anchors.margins: -2
      radius: parent.radius + 2
      color: "transparent"
      border.color: "#00000020"
      border.width: 1
      opacity: 0
    }

    ColumnLayout {
      anchors.fill: parent
      anchors.margins: 12
      spacing: 12

      // Zone image preview adaptée pour les box art Twitch (ratio 285x380)
      // Hauteur augmentée pour éviter de couper l'image
      Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 200  // Augmenté de 120 à 200 pour mieux afficher les box art
        radius: 8
        color: cardRoot.isPlaceholder ? "#1a2230" : BlueTheme.surfaceSoft
        border.color: BlueTheme.divider
        border.width: 1
        clip: true

        // Image avec chargement asynchrone et cache
        Image {
          id: categoryImage
          anchors.fill: parent
          source: cardRoot.isPlaceholder ? "" : cardRoot.boxArtUrl
          fillMode: Image.PreserveAspectFit  // Changé de PreserveAspectCrop à PreserveAspectFit pour voir toute l'image
          asynchronous: true
          cache: true
          visible: status === Image.Ready && !cardRoot.isPlaceholder
          
          // Animation de fade-in lors du chargement
          opacity: status === Image.Ready ? 1 : 0
          Behavior on opacity {
            NumberAnimation { duration: BlueTheme.animContentFadeDuration; easing.type: Easing.OutCubic }
          }
        }

        // Placeholder pendant le chargement ou si pas d'image
        Rectangle {
          anchors.fill: parent
          color: cardRoot.isPlaceholder ? "#1a2230" : BlueTheme.surfaceSoft
          visible: categoryImage.status !== Image.Ready || cardRoot.isPlaceholder
          
          Text {
            anchors.centerIn: parent
            text: cardRoot.isPlaceholder ? "⋯" : (categoryImage.status === Image.Loading ? "⏳" : "🎮")
            font.pixelSize: 32
            color: BlueTheme.mutedText
            opacity: 0.5
          }
        }
      }

      // Nom de la catégorie
      Text {
        text: cardRoot.categoryName
        font.family: BlueTheme.fontFamily
        font.pixelSize: 14
        font.bold: true
        color: BlueTheme.primaryText
        elide: Text.ElideRight
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
      }
    }
  }

  // Signal émis quand une catégorie est cliquée
  signal categoryClicked(string categoryId, string categoryName)
  
  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true
    cursorShape: Qt.PointingHandCursor
    onClicked: {
      if (!cardRoot.isPlaceholder) {
        console.log("[DEBUG CategoryCard] Clicked on category:", cardRoot.categoryName, "ID:", cardRoot.categoryId)
        cardRoot.categoryClicked(cardRoot.categoryId, cardRoot.categoryName)
      }
    }
  }
}

