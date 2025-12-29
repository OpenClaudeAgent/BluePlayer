import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

BaseCard {
  id: cardRoot

  // Dimensions adaptées pour les box art Twitch (ratio 285x380 ≈ 0.75)
  cardHeight: 260

  property string categoryName: ""
  property string categoryId: ""
  property string boxArtUrl: ""

  signal categoryClicked(string categoryId, string categoryName)

  onCardClicked: {
    categoryClicked(categoryId, categoryName)
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 12

    // Zone image preview adaptée pour les box art Twitch
    Rectangle {
      Layout.fillWidth: true
      Layout.preferredHeight: 200
      radius: 8
      color: cardRoot.isPlaceholder ? "#1a2230" : BlueTheme.surfaceSoft
      border.color: BlueTheme.divider
      border.width: 1
      clip: true

      Image {
        id: categoryImage
        anchors.fill: parent
        source: cardRoot.isPlaceholder ? "" : cardRoot.boxArtUrl
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
