import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/AppleTheme.js" as AppleTheme

Item {
  id: root
  property string sectionTitle
  property string sectionSubtitle: ""
  property var cardsModel: []
  property real rowHeight: 220
  property real cardWidth: 180
  property real cardSpacing: 16

  implicitHeight: (titleSection.visible ? titleSection.height + AppleTheme.spacingMedium : 0) + flickableSection.height

  ColumnLayout {
    id: container
    anchors.fill: parent
    spacing: AppleTheme.spacingMedium

    // Titre et sous-titre de la section
    ColumnLayout {
      id: titleSection
      Layout.fillWidth: true
      spacing: 4
      visible: root.sectionTitle !== ""

      Text {
        text: root.sectionTitle
        font.family: AppleTheme.fontFamily
        font.pixelSize: 20
        font.bold: true
        color: AppleTheme.primaryText
      }

      Text {
        visible: root.sectionSubtitle !== ""
        text: root.sectionSubtitle
        font.family: AppleTheme.fontFamily
        font.pixelSize: 14
        color: AppleTheme.secondaryText
      }
    }

    // Zone scrollable horizontale avec virtualisation pour optimiser les performances
    ListView {
      id: flickableSection
      Layout.fillWidth: true
      Layout.preferredHeight: root.rowHeight
      orientation: ListView.Horizontal
      spacing: root.cardSpacing
      clip: true
      interactive: true
      
      // Optimisation: cacheBuffer pour précharger les éléments hors écran
      // Cache 2 écrans supplémentaires de chaque côté pour une navigation fluide
      cacheBuffer: Math.max(parent.width * 2, root.cardWidth * 4)
      
      // Modèle avec lazy loading: ne charge que les éléments visibles
      model: root.cardsModel
      
      
      delegate: Loader {
        id: cardLoader
        width: root.cardWidth
        height: root.rowHeight
        property var cardData: modelData
        property bool isCategory: cardData ? (cardData.boxArtUrl !== undefined || cardData.id !== undefined) : false
        sourceComponent: isCategory ? categoryCardComponent : streamCardComponent
        
        // Passer cardData à l'item chargé
        onItemChanged: {
          if (item && item.hasOwnProperty('card')) {
            item.card = Qt.binding(function() { return cardData })
          }
        }
      }
      
      Component {
        id: streamCardComponent
        StreamCard {
          id: streamCard
          width: root.cardWidth
          height: root.rowHeight
          property var card: null  // Sera assigné par le Loader parent via onItemChanged
          streamerName: card ? (card.name || "") : ""
          streamTitle: card ? (card.detail || "") : ""
          viewerCount: card ? (card.viewers || "") : ""
          previewImage: card ? (card.previewImage || "") : ""
          isPlaceholder: card ? (card.isPlaceholder || false) : true
        }
      }
      
      Component {
        id: categoryCardComponent
        CategoryCard {
          id: categoryCard
          width: root.cardWidth
          height: root.rowHeight
          property var card: null  // Sera assigné par le Loader parent via onItemChanged
          categoryName: card ? (card.name || "") : ""
          categoryId: card ? (card.id || "") : ""
          boxArtUrl: card ? (card.boxArtUrl || "") : ""
          isPlaceholder: card ? (card.isPlaceholder || false) : true
        }
      }
    }
  }
}

