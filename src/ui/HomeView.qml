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
          Layout.fillWidth: true
          Layout.fillHeight: true
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
