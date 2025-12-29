import QtQuick 2.15
import QtQuick.Controls 2.15
import "../themes/BlueTheme.js" as BlueTheme

Rectangle {
  id: searchResultsRoot
  objectName: "searchResultsPopup"  // For E2E testing
  
  // Theme access
  readonly property var tm: typeof themeManager !== "undefined" ? themeManager : null
  
  property var channelResults: []      // Chaînes LIVE uniquement
  property var cacheResults: []        // VODs en cache
  property bool isVisible: false
  property bool hasSearchQuery: false  // True si une recherche a été effectuée
  property int selectedIndex: -1
  property int totalCount: (channelResults ? channelResults.length : 0) + (cacheResults ? cacheResults.length : 0)
  
  signal channelClicked(string broadcasterLogin, string displayName, bool isLive, string thumbnailUrl)
  signal cacheVodClicked(string vodId, string filePath, string streamerName)
  signal closeRequested()
  
  // Visible si: recherche effectuée ET (résultats OU pas de résultats à montrer)
  visible: isVisible && hasSearchQuery
  color: tm ? tm.surface : BlueTheme.surface
  border.color: tm ? tm.divider : BlueTheme.divider
  border.width: 1
  radius: 16
  clip: true
  
  implicitHeight: totalCount > 0 ? Math.min(contentColumn.height + 16, 400) : 60
  
  function navigateUp() {
    if (selectedIndex > 0) selectedIndex--
    else selectedIndex = totalCount - 1
  }
  
  function navigateDown() {
    if (selectedIndex < totalCount - 1) selectedIndex++
    else selectedIndex = 0
  }
  
  function selectCurrent() {
    if (selectedIndex < 0) return
    var channelCount = channelResults ? channelResults.length : 0
    if (selectedIndex < channelCount) {
      var channel = channelResults[selectedIndex]
      channelClicked(channel.broadcaster_login || "", channel.display_name || "", channel.is_live || false, channel.thumbnail_url || "")
    } else {
      var cacheIndex = selectedIndex - channelCount
      var vod = cacheResults[cacheIndex]
      cacheVodClicked(vod.id || "", vod.filePath || "", vod.streamerName || "")
    }
  }
  
  onChannelResultsChanged: selectedIndex = -1
  onCacheResultsChanged: selectedIndex = -1
  
  Flickable {
    anchors.fill: parent
    anchors.margins: BlueTheme.spacingSmall
    contentHeight: contentColumn.height
    clip: true
    
    Column {
      id: contentColumn
      width: parent.width
      spacing: BlueTheme.spacingSmall
      
      // Constantes d'espacement
      readonly property int sectionLabelHeight: 24
      readonly property int sectionSpacing: 8
      readonly property int itemSpacing: 2
      
      // ==================== LIVE CHANNELS ====================
      Item {
        visible: channelResults && channelResults.length > 0
        width: contentColumn.width
        height: contentColumn.sectionLabelHeight
        
        Text {
          anchors.left: parent.left
          anchors.leftMargin: 8
          anchors.bottom: parent.bottom
          anchors.bottomMargin: 4
          text: qsTr("Live")
          font.family: BlueTheme.fontFamily
          font.pixelSize: 11
          font.bold: true
          font.letterSpacing: 0.3
          color: tm ? tm.mutedText : BlueTheme.mutedText
        }
      }
      
      Repeater {
        model: channelResults
        
        delegate: Rectangle {
          id: channelDelegate
          objectName: "searchResult_" + index  // For E2E testing
          width: contentColumn.width
          height: {
            var info = channelDelegate.channelInfo
            if (info && info.is_live && info.title) return 72
            if (info && info.is_live && info.game_name) return 56
            return 52
          }
          radius: BlueTheme.spacingSmall
          
          property var channelInfo: modelData
          property int itemIndex: index
          property bool isSelected: searchResultsRoot.selectedIndex === itemIndex
          property bool isLive: channelInfo && channelInfo.is_live ? true : false
          property bool hasTitle: channelInfo && channelInfo.title ? true : false
          property bool hasGame: channelInfo && channelInfo.game_name ? true : false
          
          color: channelMouse.containsMouse ? (tm ? tm.cardHighlight : BlueTheme.cardHighlight) : "transparent"
          border.color: isSelected ? (tm ? tm.accent : BlueTheme.accent) : "transparent"
          border.width: isSelected ? 1 : 0
          
          Behavior on color { ColorAnimation { duration: BlueTheme.animHoverDuration } }
          Behavior on border.color { ColorAnimation { duration: BlueTheme.animHoverDuration } }
          
          // Avatar
          Rectangle {
            id: channelAvatar
            x: 8
            y: (parent.height - 36) / 2
            width: 36
            height: 36
            radius: 18
            color: tm ? tm.surfaceSoft : BlueTheme.surfaceSoft
            clip: true
            
            Image {
              anchors.fill: parent
              source: channelDelegate.channelInfo ? (channelDelegate.channelInfo.thumbnail_url || "") : ""
              fillMode: Image.PreserveAspectCrop
              asynchronous: true
            }
          }
          
          // Channel name
          Text {
            id: channelNameText
            x: 56
            y: {
              if (channelDelegate.isLive && channelDelegate.hasTitle) return 8
              if (channelDelegate.isLive && channelDelegate.hasGame) return 10
              return (parent.height - height) / 2
            }
            width: channelDelegate.width - 120
            text: channelDelegate.channelInfo ? (channelDelegate.channelInfo.display_name || channelDelegate.channelInfo.broadcaster_login || "Unknown") : "Unknown"
            font.family: BlueTheme.fontFamily
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: tm ? tm.primaryText : BlueTheme.primaryText
            elide: Text.ElideRight
          }
          
          // Game name
          Text {
            id: gameNameText
            x: 56
            y: channelNameText.y + 18
            width: channelDelegate.width - 120
            visible: channelDelegate.isLive && channelDelegate.hasGame
            text: channelDelegate.channelInfo ? (channelDelegate.channelInfo.game_name || "") : ""
            font.family: BlueTheme.fontFamily
            font.pixelSize: 12
            color: tm ? tm.accent : BlueTheme.accent
            elide: Text.ElideRight
          }
          
          // Stream title
          Text {
            x: 56
            y: gameNameText.visible ? gameNameText.y + 16 : channelNameText.y + 18
            width: channelDelegate.width - 68
            visible: channelDelegate.isLive && channelDelegate.hasTitle
            text: channelDelegate.channelInfo ? (channelDelegate.channelInfo.title || "") : ""
            font.family: BlueTheme.fontFamily
            font.pixelSize: 11
            color: tm ? tm.mutedText : BlueTheme.mutedText
            elide: Text.ElideRight
            maximumLineCount: 1
          }
          
          // Live badge
          Rectangle {
            id: liveBadge
            x: channelDelegate.width - 54
            y: 8
            visible: channelDelegate.isLive
            width: 42
            height: 18
            radius: 9
            color: tm ? tm.statusNegative : BlueTheme.statusNegative
            
            Text {
              anchors.centerIn: parent
              text: "LIVE"
              font.pixelSize: 10
              font.bold: true
              color: "#ffffff"
            }
          }
          
          MouseArea {
            id: channelMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              if (channelDelegate.channelInfo) {
                console.info("[Search] Channel selected:", channelDelegate.channelInfo.display_name || channelDelegate.channelInfo.broadcaster_login, "(live)")
                searchResultsRoot.channelClicked(
                  channelDelegate.channelInfo.broadcaster_login || "",
                  channelDelegate.channelInfo.display_name || "",
                  channelDelegate.channelInfo.is_live || false,
                  channelDelegate.channelInfo.thumbnail_url || ""
                )
              }
            }
          }
        }
      }
      
      // ==================== SEPARATOR ====================
      Item {
        visible: channelResults && channelResults.length > 0 && cacheResults && cacheResults.length > 0
        width: contentColumn.width
        height: contentColumn.sectionSpacing * 2
        
        Rectangle {
          anchors.centerIn: parent
          width: parent.width - 16
          height: 1
          color: tm ? tm.divider : BlueTheme.divider
        }
      }
      
      // ==================== CACHED VODS ====================
      Item {
        visible: cacheResults && cacheResults.length > 0
        width: contentColumn.width
        height: contentColumn.sectionLabelHeight
        
        Text {
          anchors.left: parent.left
          anchors.leftMargin: 8
          anchors.bottom: parent.bottom
          anchors.bottomMargin: 4
          text: qsTr("In cache")
          font.family: BlueTheme.fontFamily
          font.pixelSize: 11
          font.bold: true
          font.letterSpacing: 0.3
          color: tm ? tm.mutedText : BlueTheme.mutedText
        }
      }
      
      Repeater {
        model: cacheResults
        
        delegate: Rectangle {
          id: cacheDelegate
          width: contentColumn.width
          height: 64
          radius: BlueTheme.spacingSmall
          
          property var vodInfo: modelData
          property int itemIndex: index
          property int globalIndex: (channelResults ? channelResults.length : 0) + itemIndex
          property bool isSelected: searchResultsRoot.selectedIndex === globalIndex
          
          color: cacheMouse.containsMouse ? (tm ? tm.cardHighlight : BlueTheme.cardHighlight) : "transparent"
          border.color: isSelected ? (tm ? tm.accent : BlueTheme.accent) : "transparent"
          border.width: isSelected ? 1 : 0
          
          Behavior on color { ColorAnimation { duration: BlueTheme.animHoverDuration } }
          Behavior on border.color { ColorAnimation { duration: BlueTheme.animHoverDuration } }
          
          // Thumbnail
          Rectangle {
            id: vodThumb
            x: 8
            y: 8
            width: 48
            height: 48
            radius: 8
            color: tm ? tm.surfaceSoft : BlueTheme.surfaceSoft
            clip: true
            
            Image {
              anchors.fill: parent
              source: cacheDelegate.vodInfo ? (cacheDelegate.vodInfo.thumbnailPath || "") : ""
              fillMode: Image.PreserveAspectCrop
              asynchronous: true
            }
            
            // Fallback icon
            Text {
              anchors.centerIn: parent
              text: "📼"
              font.pixelSize: 20
              visible: parent.children[0].status !== Image.Ready
              opacity: 0.5
            }
          }
          
          // Streamer name
          Text {
            x: 68
            y: 10
            width: cacheDelegate.width - 140
            text: cacheDelegate.vodInfo ? (cacheDelegate.vodInfo.streamerName || "Unknown") : "Unknown"
            font.family: BlueTheme.fontFamily
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: tm ? tm.primaryText : BlueTheme.primaryText
            elide: Text.ElideRight
          }
          
          // Stream title
          Text {
            x: 68
            y: 28
            width: cacheDelegate.width - 140
            text: cacheDelegate.vodInfo ? (cacheDelegate.vodInfo.streamTitle || "") : ""
            font.family: BlueTheme.fontFamily
            font.pixelSize: 11
            color: tm ? tm.secondaryText : BlueTheme.secondaryText
            elide: Text.ElideRight
          }
          
          // Date, duration & size
          Text {
            x: 68
            y: 44
            width: cacheDelegate.width - 140
            text: {
              var info = cacheDelegate.vodInfo
              if (!info) return ""
              var parts = []
              if (info.recordedAtFormatted) parts.push(info.recordedAtFormatted)
              if (info.durationFormatted) parts.push(info.durationFormatted)
              if (info.fileSizeFormatted) parts.push(info.fileSizeFormatted)
              return parts.join(" • ")
            }
            font.family: BlueTheme.fontFamily
            font.pixelSize: 10
            color: tm ? tm.mutedText : BlueTheme.mutedText
          }
          
          // Cache badge
          Rectangle {
            x: cacheDelegate.width - 62
            y: 8
            width: 50
            height: 18
            radius: 9
            color: tm ? tm.accent : BlueTheme.accent
            opacity: 0.8
            
            Text {
              anchors.centerIn: parent
              text: "CACHE"
              font.pixelSize: 9
              font.bold: true
              color: "#ffffff"
            }
          }
          
          MouseArea {
            id: cacheMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
              if (cacheDelegate.vodInfo) {
                console.info("[Search] Cached VOD selected:", cacheDelegate.vodInfo.streamerName || "Unknown")
                searchResultsRoot.cacheVodClicked(
                  cacheDelegate.vodInfo.id || "",
                  cacheDelegate.vodInfo.filePath || "",
                  cacheDelegate.vodInfo.streamerName || ""
                )
              }
            }
          }
        }
      }
      
      // No results
      Text {
        visible: searchResultsRoot.totalCount === 0
        text: qsTr("No results")
        font.family: BlueTheme.fontFamily
        font.pixelSize: 13
        color: tm ? tm.mutedText : BlueTheme.mutedText
        width: contentColumn.width
        horizontalAlignment: Text.AlignHCenter
        topPadding: 16
        bottomPadding: 16
      }
    }
  }
}
