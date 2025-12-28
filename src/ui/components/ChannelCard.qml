import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

BaseCard {
  id: cardRoot

  // Dimensions adaptées comme pour les catégories
  cardHeight: 260

  property string channelName: ""
  property string displayName: ""
  property string thumbnailUrl: ""
  property bool isLive: false
  property string gameName: ""

  onCardClicked: {
    console.log("Clicked on channel:", channelName)
    // TODO: Navigate to channel
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 12

    // Zone image preview adaptée comme pour les catégories
    Rectangle {
      Layout.fillWidth: true
      Layout.preferredHeight: 200
      radius: 8
      color: cardRoot.isPlaceholder ? "#1a2230" : BlueTheme.surfaceSoft
      border.color: BlueTheme.divider
      border.width: 1
      clip: true

      Image {
        id: avatarImage
        anchors.fill: parent
        source: cardRoot.isPlaceholder ? "" : cardRoot.thumbnailUrl
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        cache: true
        visible: status === Image.Ready && !cardRoot.isPlaceholder

        opacity: status === Image.Ready ? 1 : 0
        Behavior on opacity {
          NumberAnimation { duration: BlueTheme.animContentFadeDuration; easing.type: Easing.OutCubic }
        }
      }

      Rectangle {
        anchors.fill: parent
        color: cardRoot.isPlaceholder ? "#1a2230" : BlueTheme.surfaceSoft
        visible: avatarImage.status !== Image.Ready || cardRoot.isPlaceholder

        Text {
          anchors.centerIn: parent
          text: cardRoot.isPlaceholder ? "⋯" : (avatarImage.status === Image.Loading ? "⏳" : "👤")
          font.pixelSize: 32
          color: BlueTheme.mutedText
          opacity: 0.5
        }
      }

      // Badge LIVE
      Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 6
        width: 40
        height: 20
        radius: 10
        color: BlueTheme.statusNegative
        visible: cardRoot.isLive && !cardRoot.isPlaceholder && avatarImage.status === Image.Ready

        Text {
          anchors.centerIn: parent
          text: "LIVE"
          font.pixelSize: 10
          font.bold: true
          color: "#fff"
        }
      }
    }

    // Nom de la chaîne (centré comme pour les catégories)
    Text {
      text: cardRoot.displayName !== "" ? cardRoot.displayName : cardRoot.channelName
      font.family: BlueTheme.fontFamily
      font.pixelSize: 14
      font.bold: true
      color: BlueTheme.primaryText
      elide: Text.ElideRight
      Layout.fillWidth: true
      horizontalAlignment: Text.AlignHCenter
    }

    // Statut (seulement si en direct avec nom du jeu)
    Text {
      text: cardRoot.isLive && cardRoot.gameName !== "" ? cardRoot.gameName : ""
      font.family: BlueTheme.fontFamily
      font.pixelSize: 11
      color: BlueTheme.accent
      elide: Text.ElideRight
      Layout.fillWidth: true
      horizontalAlignment: Text.AlignHCenter
      visible: text !== ""
    }
  }
}
