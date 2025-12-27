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
  
  // Signal pour ouvrir le player de stream
  signal openStreamPlayer(string streamerLogin, string streamerName, string streamTitle, string thumbnailUrl)
  
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
        console.log("[HomeView] Recherche déclenchée après debounce:", searchField.text)
        // Rechercher dans Twitch
        var service = getTwitchService()
        if (service) {
          service.search(searchField.text)
        }
        // Rechercher dans le cache local
        var cache = getCacheManager()
        if (cache) {
          searchResultsPopup.cacheResults = cache.searchVods(searchField.text)
          console.log("[HomeView] Cache search results:", searchResultsPopup.cacheResults.length)
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
      console.log("[DEBUG HomeView] onPopularClipsChanged() called")
      var service = getTwitchService()
      console.log("[DEBUG HomeView] service:", service ? "exists" : "null")
      if (service) {
        console.log("[DEBUG HomeView] service.popularClips:", service.popularClips ? "exists" : "null", "length:", service.popularClips ? service.popularClips.length : 0)
        if (service.popularClips && service.popularClips.length > 0) {
          console.log("[DEBUG HomeView] Calling viewModel.updatePopularClips()")
          viewModel.updatePopularClips(service.popularClips)
        } else {
          console.log("[DEBUG HomeView] No popular clips to update")
        }
      }
    }
    function onFollowedClipsChanged() {
      console.log("[DEBUG HomeView] onFollowedClipsChanged() called")
      var service = getTwitchService()
      if (service && service.followedClips && service.followedClips.length > 0) {
        console.log("[DEBUG HomeView] Calling viewModel.updateFollowedClips() with", service.followedClips.length, "clips")
        viewModel.updateFollowedClips(service.followedClips)
      } else {
        console.log("[DEBUG HomeView] No followed clips to update")
      }
    }
    function onVideosChanged() {
      console.log("[DEBUG HomeView] onVideosChanged() called")
      var service = getTwitchService()
      if (service && service.videos && service.videos.length > 0) {
        console.log("[DEBUG HomeView] Calling viewModel.updateVideos() with", service.videos.length, "videos")
        viewModel.updateVideos(service.videos)
      } else {
        console.log("[DEBUG HomeView] No videos to update")
      }
    }
    function onFollowedChannelsChanged() {
      console.log("[DEBUG HomeView] onFollowedChannelsChanged() called")
      var service = getTwitchService()
      if (service && service.followedChannels && service.followedChannels.length > 0) {
        console.log("[DEBUG HomeView] Calling viewModel.updateFollowedChannels() with", service.followedChannels.length, "channels")
        viewModel.updateFollowedChannels(service.followedChannels)
      } else {
        console.log("[DEBUG HomeView] No followed channels to update")
      }
    }
    function onNewStreamersChanged() {
      var service = getTwitchService()
      if (service && service.newStreamers && service.newStreamers.length > 0) {
        viewModel.updateNewStreamers(service.newStreamers)
      }
    }
    function onCategoryStreamsChanged() {
      console.log("[DEBUG HomeView] onCategoryStreamsChanged() called")
      var service = getTwitchService()
      console.log("[DEBUG HomeView] service:", service ? "exists" : "null")
      console.log("[DEBUG HomeView] service.categoryStreams:", service && service.categoryStreams ? "exists" : "null", "length:", service && service.categoryStreams ? service.categoryStreams.length : 0)
      if (service && service.categoryStreams && service.categoryStreams.length > 0) {
        console.log("[DEBUG HomeView] Calling viewModel.updateCategoryStreams() with", service.categoryStreams.length, "streams")
        viewModel.updateCategoryStreams(service.categoryStreams)
      } else {
        console.log("[DEBUG HomeView] No category streams to update - service:", service ? "exists" : "null", "categoryStreams:", service && service.categoryStreams ? "exists" : "null", "length:", service && service.categoryStreams ? service.categoryStreams.length : 0)
      }
    }
    function onErrorOccurred(message) {
      console.log("[HomeView] ERROR Twitch:", message)
    }
    function onSearchChannelResultsChanged() {
      console.log("[HomeView] Search channel results changed")
      // Force update of SearchResults binding
      searchResultsPopup.channelResults = getTwitchService() ? getTwitchService().searchChannelResults : []
    }
    function onSearchCategoryResultsChanged() {
      // Categories are no longer displayed in search results
      console.log("[HomeView] Search category results changed (ignored)")
    }
    function onUserIdChanged() {
      console.log("[DEBUG HomeView] onUserIdChanged() called")
      var service = getTwitchService()
      if (service && service.userId) {
        console.log("[DEBUG HomeView] UserId available:", service.userId, "- user-specific data should be loaded")
      }
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
        // Charger les streams recommandés même sans authentification
        service.refreshRecommendedStreams()
      }
      if (service.categories && service.categories.length > 0) {
        viewModel.updateCategories(service.categories)
      } else {
        // Charger les catégories même sans authentification
        service.refreshCategories()
      }
      // Charger les sections publiques même sans authentification
      console.log("[DEBUG HomeView] Loading public sections")
      service.refreshPopularClips()
      
      // Charger les sections nécessitant authentification si l'utilisateur est authentifié
      if (service.authenticated) {
        console.log("[DEBUG HomeView] User authenticated, will load user-specific data after userId is available")
        // Les données spécifiques à l'utilisateur seront chargées dans TwitchService::onUserInfoReady()
        // après que userId soit disponible
      }
    }
  }

  // Overlay pour fermer les résultats de recherche quand on clique ailleurs
  MouseArea {
    id: searchOverlay
    anchors.fill: parent
    visible: searchResultsPopup.visible
    z: 5  // Au-dessus du contenu mais en-dessous de la barre de recherche
    onClicked: {
      console.log("[HomeView] Click outside search results - closing")
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
        color: searchField.activeFocus ? "#161d28" : "#0d1117"
        border.color: searchField.activeFocus ? AppleTheme.accent : "transparent"
        border.width: searchField.activeFocus ? 1 : 0

        Behavior on color {
          ColorAnimation { duration: 150; easing.type: Easing.OutCubic }
        }
        Behavior on border.color {
          ColorAnimation { duration: 150; easing.type: Easing.OutCubic }
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
            color: searchField.activeFocus ? AppleTheme.accent : AppleTheme.mutedText
            Layout.alignment: Qt.AlignVCenter
            opacity: 0.7
            Behavior on color {
              ColorAnimation { duration: 150 }
            }
          }

          TextField {
            id: searchField
            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            placeholderText: qsTr("Rechercher...")
            placeholderTextColor: AppleTheme.mutedText
            font.family: AppleTheme.fontFamily
            font.pixelSize: 14
            font.weight: Font.Normal
            color: AppleTheme.primaryText
            cursorVisible: activeFocus
            background: Item {}
            selectByMouse: true
            leftPadding: 0
            rightPadding: 0
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
                  console.log("[HomeView] Recherche immédiate:", text)
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
      
      // Popup des résultats de recherche
      SearchResults {
        id: searchResultsPopup
        anchors.top: searchBarBackground.bottom
        anchors.topMargin: 8
        anchors.horizontalCenter: searchBarBackground.horizontalCenter
        width: searchBarBackground.width
        
        isVisible: homeRoot.searchResultsVisible
        channelResults: {
          var service = getTwitchService()
          return service ? service.searchChannelResults : []
        }
        cacheResults: []  // Updated via searchDebounceTimer
        
        onChannelClicked: function(broadcasterLogin, displayName, isLive, thumbnailUrl) {
          console.log("[HomeView] Search result channel clicked:", displayName, "login:", broadcasterLogin, "isLive:", isLive)
          searchField.text = ""
          clearSearchResults()
          searchField.focus = false
          // Ouvrir le player pour le stream en direct (only live channels are shown)
          homeRoot.openStreamPlayer(broadcasterLogin, displayName, "", thumbnailUrl)
        }
        
        onCacheVodClicked: function(vodId, filePath, streamerName) {
          console.log("[HomeView] Cache VOD clicked:", streamerName, "path:", filePath)
          searchField.text = ""
          clearSearchResults()
          searchField.focus = false
          // Ouvrir le player pour la VOD en cache
          // Le signal openStreamPlayer peut être réutilisé avec le filePath comme "login"
          // On utilise "file://" + filePath comme URL de stream
          homeRoot.openStreamPlayer("cache:" + vodId, streamerName, "VOD en cache", "")
        }
        
        onCloseRequested: {
          closeSearchResults()
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
              console.log("[DEBUG HomeView] Category clicked:", categoryName, "ID:", categoryId)
              const service = getTwitchService()
              if (service && categoryId) {
                console.log("[DEBUG HomeView] Calling service.refreshCategoryStreams() with gameId:", categoryId)
                service.refreshCategoryStreams(categoryId)
              } else {
                console.log("[DEBUG HomeView] Cannot refresh category streams - service:", service ? "exists" : "null", "categoryId:", categoryId)
              }
            }
            
            onStreamClicked: function(streamerLogin, streamerName, streamTitle, thumbnailUrl) {
              console.log("[DEBUG HomeView] Stream clicked:", streamerName, "login:", streamerLogin)
              console.log("[DEBUG HomeView] thumbnailUrl received:", thumbnailUrl)
              // Émettre un signal pour ouvrir le player
              homeRoot.openStreamPlayer(streamerLogin, streamerName, streamTitle, thumbnailUrl)
            }
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
