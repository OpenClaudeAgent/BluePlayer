import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

Item {
  id: root
  
  // Theme access
  readonly property var tm: typeof themeManager !== "undefined" ? themeManager : null
  
  property string sectionTitle
  property string sectionSubtitle: ""
  property var cardsModel: []
  property real rowHeight: 220  // Hauteur par défaut pour les streams
  property real cardWidth: 180
  property real cardSpacing: 16
  
  // Détecter le type de section pour ajuster la hauteur et le composant
  property string sectionType: ""  // "clips", "videos", "channels", "streams", ou vide pour catégories
  
  // Signal émis quand une catégorie est cliquée
  signal categoryClicked(string categoryId, string categoryName)
  // Signal émis quand un stream est cliqué
  signal streamClicked(string streamerLogin, string streamerName, string streamTitle, string thumbnailUrl)
  readonly property bool isCategorySection: sectionTitle === "Parcourir" || (sectionType === "" && cardsModel.length > 0 && cardsModel[0] && cardsModel[0].boxArtUrl !== undefined)
  
  // Masquer la section "Recommandations par catégorie" si elle ne contient que des placeholders
  readonly property bool shouldShowSection: {
    if (sectionTitle === "Recommandations par catégorie") {
      // Afficher seulement si on a des données réelles (pas seulement des placeholders)
      if (cardsModel.length === 0) return false
      // Vérifier si au moins une carte n'est pas un placeholder
      for (var i = 0; i < cardsModel.length; i++) {
        if (cardsModel[i] && !cardsModel[i].isPlaceholder) {
          return true
        }
      }
      return false  // Toutes les cartes sont des placeholders
    }
    return true  // Afficher toutes les autres sections
  }
  readonly property bool isClipSection: sectionType === "clips" || (cardsModel.length > 0 && cardsModel[0] && cardsModel[0].clipTitle !== undefined)
  readonly property bool isVideoSection: sectionType === "videos" || (cardsModel.length > 0 && cardsModel[0] && cardsModel[0].videoTitle !== undefined)
  readonly property bool isChannelSection: sectionType === "channels" || (cardsModel.length > 0 && cardsModel[0] && cardsModel[0].channelName !== undefined)
  readonly property real actualRowHeight: (isCategorySection || isChannelSection) ? 260 : rowHeight

  implicitHeight: shouldShowSection ? ((titleSection.visible ? titleSection.height + BlueTheme.spacingMedium : 0) + flickableSection.height) : 0
  visible: shouldShowSection

  ColumnLayout {
    id: container
    anchors.fill: parent
    spacing: BlueTheme.spacingMedium

    // Titre et sous-titre de la section
    ColumnLayout {
      id: titleSection
      Layout.fillWidth: true
      spacing: 4
      visible: root.sectionTitle !== ""

      Text {
        text: root.sectionTitle
        font.family: BlueTheme.fontFamily
        font.pixelSize: 20
        font.bold: true
        color: tm ? tm.primaryText : BlueTheme.primaryText
      }

      Text {
        visible: root.sectionSubtitle !== ""
        text: root.sectionSubtitle
        font.family: BlueTheme.fontFamily
        font.pixelSize: 14
        color: tm ? tm.secondaryText : BlueTheme.secondaryText
      }
    }

    // Zone scrollable horizontale avec virtualisation pour optimiser les performances
    // Wrapper pour permettre l'effet scale sans crop
    Item {
      Layout.fillWidth: true
      Layout.preferredHeight: root.actualRowHeight + BlueTheme.spacingSmall * 2
      clip: true

      ListView {
        id: flickableSection
        anchors.fill: parent
        anchors.topMargin: BlueTheme.spacingSmall
        anchors.bottomMargin: BlueTheme.spacingSmall
        anchors.leftMargin: BlueTheme.spacingSmall
        anchors.rightMargin: BlueTheme.spacingSmall
        orientation: ListView.Horizontal
        spacing: root.cardSpacing
        clip: false
        interactive: true
      
      // Optimisation: cacheBuffer pour précharger les éléments hors écran
      // Cache 2 écrans supplémentaires de chaque côté pour une navigation fluide
      cacheBuffer: Math.max(parent.width * 2, root.cardWidth * 4)
      
      // Modèle avec lazy loading: ne charge que les éléments visibles
      model: root.cardsModel
      
      
      delegate: Loader {
        id: cardLoader
        width: root.cardWidth
        height: root.actualRowHeight
        property int cardIndex: index  // For E2E testing
        property var cardData: modelData
        property bool isCategory: cardData ? (cardData.boxArtUrl !== undefined || cardData.id !== undefined) : false
        property bool isClip: cardData ? (cardData.clipTitle !== undefined) : false
        property bool isVideo: cardData ? (cardData.videoTitle !== undefined) : false
        property bool isChannel: cardData ? (cardData.channelName !== undefined) : false
        
        sourceComponent: {
          if (isCategory) return categoryCardComponent
          if (isClip) return clipCardComponent
          if (isVideo) return videoCardComponent
          if (isChannel) return channelCardComponent
          return streamCardComponent
        }
        
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
          objectName: "streamCard_" + (parent ? parent.cardIndex : -1)  // For E2E testing
          width: root.cardWidth
          height: root.rowHeight
          property var card: null  // Sera assigné par le Loader parent via onItemChanged
          streamerName: card ? (card.name || "") : ""
          streamTitle: card ? (card.detail || "") : ""
          viewerCount: card ? (card.viewers || "") : ""
          previewImage: card ? (card.previewImage || "") : ""
          streamerLogin: card ? (card.streamerLogin || card.userLogin || "") : ""
          isPlaceholder: card ? (card.isPlaceholder || false) : true
          
          onClicked: function(login, name, title, thumbnailUrl) {
            root.streamClicked(login, name, title, thumbnailUrl)
          }
        }
      }
      
      Component {
        id: categoryCardComponent
        CategoryCard {
          id: categoryCard
          width: root.cardWidth
          height: root.actualRowHeight
          property var card: null  // Sera assigné par le Loader parent via onItemChanged
          categoryName: card ? (card.name || "") : ""
          categoryId: card ? (card.id || "") : ""
          boxArtUrl: card ? (card.boxArtUrl || "") : ""
          isPlaceholder: card ? (card.isPlaceholder || false) : true
          
          onCategoryClicked: function(categoryId, categoryName) {
            root.categoryClicked(categoryId, categoryName)
          }
        }
      }
      
      Component {
        id: clipCardComponent
        ClipCard {
          id: clipCard
          width: root.cardWidth
          height: root.rowHeight
          property var card: null
          clipTitle: card ? (card.clipTitle || "") : ""
          broadcasterName: card ? (card.broadcasterName || "") : ""
          viewCount: card ? (card.viewCount || "") : ""
          duration: card ? (card.duration || "") : ""
          thumbnailUrl: card ? (card.thumbnailUrl || "") : ""
          isPlaceholder: card ? (card.isPlaceholder || false) : true
        }
      }
      
      Component {
        id: videoCardComponent
        VideoCard {
          id: videoCard
          width: root.cardWidth
          height: root.rowHeight
          property var card: null
          videoTitle: card ? (card.videoTitle || "") : ""
          userName: card ? (card.userName || "") : ""
          viewCount: card ? (card.viewCount || "") : ""
          duration: card ? (card.duration || "") : ""
          thumbnailUrl: card ? (card.thumbnailUrl || "") : ""
          hasProgress: card ? (card.hasProgress || false) : false
          watchPosition: card ? (card.watchPosition || 0) : 0
          isPlaceholder: card ? (card.isPlaceholder || false) : true
        }
      }
      
      Component {
        id: channelCardComponent
        ChannelCard {
          id: channelCard
          width: root.cardWidth
          height: root.actualRowHeight
          property var card: null
          channelName: card ? (card.channelName || "") : ""
          displayName: card ? (card.displayName || "") : ""
          thumbnailUrl: card ? (card.thumbnailUrl || "") : ""
          isLive: card ? (card.isLive || false) : false
          gameName: card ? (card.gameName || "") : ""
          isPlaceholder: card ? (card.isPlaceholder || false) : true
        }
      }
    }
    }  // Item wrapper
  }
}
