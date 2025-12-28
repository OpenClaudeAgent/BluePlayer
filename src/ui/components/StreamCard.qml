import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

BaseCard {
  id: cardRoot

  property string streamerName: ""
  property string streamTitle: ""
  property string viewerCount: ""
  property string previewImage: ""
  property string streamerLogin: ""

  signal clicked(string streamerLogin, string streamerName, string streamTitle, string thumbnailUrl)

  onCardClicked: {
    if (streamerLogin) {
      console.log("[DEBUG StreamCard] Clicked on stream:", streamerName, "login:", streamerLogin)
      console.log("[DEBUG StreamCard] previewImage:", previewImage)
      clicked(streamerLogin, streamerName, streamTitle, previewImage)
    }
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 12

    // Zone image preview avec chargement asynchrone et cache
    Rectangle {
      Layout.fillWidth: true
      Layout.preferredHeight: 120
      radius: 8
      color: cardRoot.isPlaceholder ? "#1a2230" : BlueTheme.surfaceSoft
      border.color: BlueTheme.divider
      border.width: 1
      clip: true

      // Image avec chargement asynchrone et cache
      Image {
        id: thumbnailImage
        anchors.fill: parent
        source: cardRoot.isPlaceholder ? "" : cardRoot.previewImage
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: true
        visible: status === Image.Ready && !cardRoot.isPlaceholder

        opacity: status === Image.Ready ? 1 : 0
        Behavior on opacity {
          NumberAnimation { duration: BlueTheme.animContentFadeDuration; easing.type: Easing.OutCubic }
        }
      }

      // Placeholder pendant le chargement ou si pas d'image
      Rectangle {
        anchors.fill: parent
        color: cardRoot.isPlaceholder ? "#1a2230" : BlueTheme.surfaceSoft
        visible: thumbnailImage.status !== Image.Ready || cardRoot.isPlaceholder

        Text {
          anchors.centerIn: parent
          text: cardRoot.isPlaceholder ? "⋯" : (thumbnailImage.status === Image.Loading ? "⏳" : "📺")
          font.pixelSize: 32
          color: BlueTheme.mutedText
          opacity: 0.5
        }
      }

      // Badge "LIVE"
      Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 6
        width: 40
        height: 20
        radius: 10
        color: BlueTheme.statusNegative
        visible: !cardRoot.isPlaceholder && thumbnailImage.status === Image.Ready

        Text {
          anchors.centerIn: parent
          text: "LIVE"
          font.pixelSize: 10
          font.bold: true
          color: "#fff"
        }
      }
    }

    // Informations du stream
    ColumnLayout {
      Layout.fillWidth: true
      spacing: 4

      Text {
        text: cardRoot.streamerName
        font.family: BlueTheme.fontFamily
        font.pixelSize: 14
        font.bold: true
        color: BlueTheme.primaryText
        elide: Text.ElideRight
        Layout.fillWidth: true
      }

      Text {
        text: cardRoot.streamTitle
        font.family: BlueTheme.fontFamily
        font.pixelSize: 12
        color: BlueTheme.secondaryText
        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        Layout.fillWidth: true
      }

      Text {
        text: cardRoot.viewerCount
        font.family: BlueTheme.fontFamily
        font.pixelSize: 11
        color: BlueTheme.accent
        Layout.fillWidth: true
      }
    }
  }
}
