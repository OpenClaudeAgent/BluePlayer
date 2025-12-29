import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import BluePlayer.UI 1.0

import "themes/BlueTheme.js" as BlueTheme
import "components"

Item {
  id: homeRoot
  objectName: "homeRoot"  // For E2E testing

  // Theme access
  readonly property var tm: typeof themeManager !== "undefined" ? themeManager : null

  // Accès au service Twitch (disponible globalement)
  // Utiliser une fonction pour éviter les boucles de binding
  function getTwitchService() {
    return typeof twitchService !== "undefined" ? twitchService : null
  }
  
  // Signal pour ouvrir le player de stream
  signal openStreamPlayer(string streamerLogin, string streamerName, string streamTitle, string thumbnailUrl)
  
  // Signal pour ouvrir une VOD en cache
  signal playVodRequested(string id, string filePath, var metadata)
  
  // Signal pour ouvrir le gestionnaire de cache
  signal openCacheManager()
  
  // Accès au cache manager
  function getCacheManager() {
    return typeof cacheManager !== "undefined" ? cacheManager : null
  }

  // HomeViewModel pour gérer la logique métier
  HomeViewModel {
    id: viewModel
    onSectionsDataChanged: {
      homeRoot.sectionsData = viewModel.sectionsData
    }
  }

  // Refresh sections when language changes
  Connections {
    target: typeof languageManager !== "undefined" ? languageManager : null
    enabled: target !== null
    function onLanguageChanged() {
      viewModel.refreshTranslations()
    }
  }

  // Utiliser sectionsData du ViewModel, mis à jour via le signal
  property var sectionsData: viewModel.sectionsData

  // Gestion du focus : perdre le focus quand on clique ailleurs
  Keys.onPressed: function(event) {
    if (event.key === Qt.Key_Escape && searchField.activeFocus) {
      searchField.focus = false
      event.accepted = true
    }
  }
  
  focus: true

  // Timer pour debouncing de la recherche (400ms comme spécifié)
  Timer {
    id: searchDebounceTimer
    interval: 400
    onTriggered: {
      if (searchField.text.length > 0) {
        // Rechercher dans Twitch
        var service = getTwitchService()
        if (service) {
          service.search(searchField.text)
        }
        // Rechercher dans le cache local
        var cache = getCacheManager()
        if (cache) {
          searchResultsPopup.cacheResults = cache.searchVods(searchField.text)
        }
      }
    }
  }
  
  // Propriétés pour les résultats de recherche
  property bool searchResultsVisible: searchField.activeFocus && searchField.text.length > 0
  
  // Fermer les résultats quand on clique ailleurs (sans effacer)
  function closeSearchResults() {
    searchField.focus = false
    // Ne pas effacer les résultats - ils réapparaîtront au refocus
  }
  
  // Effacer complètement les résultats (quand on vide le champ ou sélectionne un résultat)
  function clearSearchResults() {
    var service = getTwitchService()
    if (service) {
      service.clearSearchResults()
    }
    searchResultsPopup.cacheResults = []
  }

  // Connexion au service Twitch pour mettre à jour les streams via le ViewModel
  Connections {
    id: twitchConnections
    target: (function() {
      return typeof twitchService !== "undefined" ? twitchService : null
    })()
    enabled: typeof twitchService !== "undefined" && twitchService !== null
    function onStreamsChanged() {
      var service = getTwitchService()
      if (service && service.streams) {
        viewModel.updateFollowedStreams(service.streams)
      }
    }
    function onRecommendedStreamsChanged() {
      var service = getTwitchService()
      if (service && service.recommendedStreams && service.recommendedStreams.length > 0) {
        viewModel.updateRecommendedStreams(service.recommendedStreams)
      }
    }
    function onCategoriesChanged() {
      var service = getTwitchService()
      if (service && service.categories && service.categories.length > 0) {
        viewModel.updateCategories(service.categories)
      }
    }
    function onPopularClipsChanged() {
      var service = getTwitchService()
      if (service && service.popularClips && service.popularClips.length > 0) {
        viewModel.updatePopularClips(service.popularClips)
      }
    }
    function onFollowedClipsChanged() {
      var service = getTwitchService()
      if (service && service.followedClips && service.followedClips.length > 0) {
        viewModel.updateFollowedClips(service.followedClips)
      }
    }
    function onVideosChanged() {
      var service = getTwitchService()
      if (service && service.videos && service.videos.length > 0) {
        viewModel.updateVideos(service.videos)
      }
    }
    function onFollowedChannelsChanged() {
      var service = getTwitchService()
      if (service && service.followedChannels && service.followedChannels.length > 0) {
        viewModel.updateFollowedChannels(service.followedChannels)
      }
    }
    function onNewStreamersChanged() {
      var service = getTwitchService()
      if (service && service.newStreamers && service.newStreamers.length > 0) {
        viewModel.updateNewStreamers(service.newStreamers)
      }
    }
    function onCategoryStreamsChanged() {
      var service = getTwitchService()
      if (service && service.categoryStreams && service.categoryStreams.length > 0) {
        viewModel.updateCategoryStreams(service.categoryStreams)
      }
    }
    function onErrorOccurred(message) {
      console.warn("[Home] Twitch error:", message)
    }
    function onSearchChannelResultsChanged() {
      // Force update of SearchResults binding
      searchResultsPopup.channelResults = getTwitchService() ? getTwitchService().searchChannelResults : []
    }
    function onSearchCategoryResultsChanged() {
      // Categories are no longer displayed in search results
    }
    function onUserIdChanged() {
      // User ID changed - user-specific data will be loaded by TwitchService
    }
    function onAuthenticatedChanged(authenticated) {
      // Charger les streams recommandés et catégories quand l'utilisateur s'authentifie
      if (authenticated) {
        var service = getTwitchService()
        if (service) {
          service.refreshRecommendedStreams()
          service.refreshCategories()
        }
      }
    }
  }

  Component.onCompleted: {
    var service = getTwitchService()
    if (service) {
      if (service.streams) {
        viewModel.updateFollowedStreams(service.streams)
      }
      if (service.recommendedStreams && service.recommendedStreams.length > 0) {
        viewModel.updateRecommendedStreams(service.recommendedStreams)
      } else {
        service.refreshRecommendedStreams()
      }
      if (service.categories && service.categories.length > 0) {
        viewModel.updateCategories(service.categories)
      } else {
        service.refreshCategories()
      }
      // Charger les sections publiques
      service.refreshPopularClips()
    }
  }

  // Overlay pour fermer les résultats de recherche quand on clique ailleurs
  MouseArea {
    id: searchOverlay
    anchors.fill: parent
    visible: searchResultsPopup.visible || searchField.activeFocus
    z: 5  // Au-dessus du contenu mais en-dessous du popup de résultats
    onClicked: {
      closeSearchResults()
    }
  }
  
  // Popup des résultats de recherche - HORS du ColumnLayout pour z-index correct
  SearchResults {
    id: searchResultsPopup
    // Centré horizontalement, juste en dessous de la barre de recherche
    anchors.horizontalCenter: parent.horizontalCenter
    y: {
      var pos = searchBarBackground.mapToItem(homeRoot, 0, searchBarBackground.height)
      return pos.y + 20
    }
    width: searchBarBackground.width
    z: 10  // Au-dessus de l'overlay
    
    isVisible: homeRoot.searchResultsVisible
    hasSearchQuery: searchField.text.length > 0
    channelResults: {
      var service = getTwitchService()
      return service ? service.searchChannelResults : []
    }
    cacheResults: []  // Updated via searchDebounceTimer
    
    onChannelClicked: function(broadcasterLogin, displayName, isLive, thumbnailUrl) {
      console.info("[Search] Opening stream:", displayName)
      searchField.text = ""
      clearSearchResults()
      searchField.focus = false
      // Ouvrir le player pour le stream en direct
      homeRoot.openStreamPlayer(broadcasterLogin, displayName, "", thumbnailUrl)
    }
    
    onCacheVodClicked: function(vodId, filePath, streamerName) {
      console.info("[Search] Opening cached VOD:", streamerName)
      searchField.text = ""
      clearSearchResults()
      searchField.focus = false
      // Ouvrir le player pour la VOD en cache
      homeRoot.playVodRequested(vodId, filePath, { streamerName: streamerName, streamTitle: "VOD en cache" })
    }
    
    onCloseRequested: {
      closeSearchResults()
    }
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 0

    // Barre de recherche fixe en haut - Style minimaliste
    Rectangle {
      id: searchBarContainer
      Layout.fillWidth: true
      Layout.preferredHeight: 72
      color: "transparent"
      z: 100

      Rectangle {
        id: searchBarBackground
        width: Math.min(380, parent.width - 48)
        height: 44
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        radius: 12
        color: {
          var surfaceColor = homeRoot.tm ? homeRoot.tm.surfaceSoft : BlueTheme.surfaceSoft
          if (searchField.activeFocus) {
            return Qt.rgba(surfaceColor.r, surfaceColor.g, surfaceColor.b, 0.9)
          }
          return Qt.rgba(surfaceColor.r, surfaceColor.g, surfaceColor.b, 0.6)
        }
        border.color: searchField.activeFocus ? (homeRoot.tm ? homeRoot.tm.accent : BlueTheme.accent) : (homeRoot.tm ? homeRoot.tm.divider : BlueTheme.divider)
        border.width: 1

        Behavior on color {
          ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
        }
        Behavior on border.color {
          ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
        }

        RowLayout {
          anchors.fill: parent
          anchors.leftMargin: 14
          anchors.rightMargin: 14
          spacing: 10

          // Icône de loupe minimaliste
          Text {
            text: "\u2315"  // Loupe unicode
            font.pixelSize: 18
            font.weight: Font.Light
            color: searchField.activeFocus ? (homeRoot.tm ? homeRoot.tm.accent : BlueTheme.accent) : (homeRoot.tm ? homeRoot.tm.mutedText : BlueTheme.mutedText)
            Layout.alignment: Qt.AlignVCenter
            opacity: 0.7
            Behavior on color {
              ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
            }
          }

          TextField {
            id: searchField
            objectName: "searchField"  // For E2E testing
            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            placeholderText: qsTr("Search...")
            placeholderTextColor: homeRoot.tm ? homeRoot.tm.mutedText : BlueTheme.mutedText
            font.family: BlueTheme.fontFamily
            font.pixelSize: 14
            font.weight: Font.Normal
            color: homeRoot.tm ? homeRoot.tm.primaryText : BlueTheme.primaryText
            cursorVisible: activeFocus
            background: Item {}
            selectByMouse: true
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0
            clip: true
              onTextChanged: {
                // Annuler le timer précédent si l'utilisateur tape encore
                searchDebounceTimer.stop()
                // Redémarrer le timer pour attendre 400ms après la dernière frappe
                if (text.length > 0) {
                  searchDebounceTimer.start()
                } else {
                  // Effacer les résultats si le champ est vide
                  clearSearchResults()
                }
              }
              onAccepted: {
                // Recherche immédiate si l'utilisateur appuie sur Entrée
                // (seulement si aucun élément n'est sélectionné - sinon géré par Keys.onPressed)
                if (searchResultsPopup.selectedIndex < 0 && text.length > 0) {
                  searchDebounceTimer.stop()
                  console.info("[Search] Query submitted:", text)
                  var service = getTwitchService()
                  if (service) {
                    service.search(text)
                  }
                  // Aussi chercher dans le cache
                  var cache = getCacheManager()
                  if (cache) {
                    searchResultsPopup.cacheResults = cache.searchVods(text)
                  }
                }
              }
            Keys.priority: Keys.BeforeItem
            Keys.onPressed: function(event) {
              if (event.key === Qt.Key_Escape) {
                searchDebounceTimer.stop()
                closeSearchResults()
                event.accepted = true
              } else if (event.key === Qt.Key_Up) {
                if (searchResultsPopup.visible) {
                  searchResultsPopup.navigateUp()
                  event.accepted = true
                }
              } else if (event.key === Qt.Key_Down) {
                if (searchResultsPopup.visible) {
                  searchResultsPopup.navigateDown()
                  event.accepted = true
                }
              } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                if (searchResultsPopup.visible && searchResultsPopup.selectedIndex >= 0) {
                  searchResultsPopup.selectCurrent()
                  event.accepted = true
                }
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
      contentWidth: availableWidth  // Prevent horizontal scrolling

      ColumnLayout {
        id: contentLayout
        width: parent.width
        spacing: 32
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: BlueTheme.spacingLarge
        
        // MouseArea pour perdre le focus et fermer les résultats de recherche
        MouseArea {
          Layout.fillWidth: true
          Layout.fillHeight: true
          z: -1
          hoverEnabled: false
          propagateComposedEvents: true
          onClicked: function(mouse) {
            if (searchField.activeFocus || searchResultsPopup.visible) {
              closeSearchResults()
            }
            mouse.accepted = false
          }
        }

        // Répéter pour chaque section
        Repeater {
          model: homeRoot.sectionsData
          delegate: HorizontalRowSection {
            Layout.fillWidth: true
            Layout.topMargin: 0
            sectionTitle: modelData.title
            sectionSubtitle: modelData.subtitle
            sectionType: modelData.type || ""
            cardsModel: modelData.cards
            rowHeight: 220
            cardWidth: 180
            cardSpacing: 16
            
            onCategoryClicked: function(categoryId, categoryName) {
              const service = getTwitchService()
              if (service && categoryId) {
                service.refreshCategoryStreams(categoryId)
              }
            }
            
            onStreamClicked: function(streamerLogin, streamerName, streamTitle, thumbnailUrl) {
              homeRoot.openStreamPlayer(streamerLogin, streamerName, streamTitle, thumbnailUrl)
            }
          }
        }

        // Espace en bas pour le scroll
        Item {
          Layout.preferredHeight: BlueTheme.spacingLarge * 2
        }
      }
    }
  }
}
