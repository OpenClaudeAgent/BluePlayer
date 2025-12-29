import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

BaseCard {
  id: cardRoot

  // Theme access
  readonly property var tm: typeof themeManager !== "undefined" ? themeManager : null

  property string videoTitle: ""
  property string userName: ""
  property string viewCount: ""
  property string duration: ""
  property string thumbnailUrl: ""
  property bool hasProgress: false
  property int watchPosition: 0

  onCardClicked: {
    // TODO: Play video - will log when implemented
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 12

    Rectangle {
      Layout.fillWidth: true
      Layout.preferredHeight: 120
      radius: 8
      color: cardRoot.isPlaceholder 
             ? (tm ? tm.cardHighlight : "#1a2230")
             : (tm ? tm.surfaceSoft : BlueTheme.surfaceSoft)
      border.color: tm ? tm.divider : BlueTheme.divider
      border.width: 1
      clip: true

      Image {
        id: thumbnailImage
        anchors.fill: parent
        source: cardRoot.isPlaceholder ? "" : cardRoot.thumbnailUrl
        fillMode: Image.PreserveAspectCrop
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
        color: cardRoot.isPlaceholder 
               ? (tm ? tm.cardHighlight : "#1a2230")
               : (tm ? tm.surfaceSoft : BlueTheme.surfaceSoft)
        visible: thumbnailImage.status !== Image.Ready || cardRoot.isPlaceholder

        Text {
          anchors.centerIn: parent
          text: cardRoot.isPlaceholder ? "⋯" : (thumbnailImage.status === Image.Loading ? "⏳" : "📹")
          font.pixelSize: 32
          color: tm ? tm.mutedText : BlueTheme.mutedText
          opacity: 0.5
        }
      }

      // Duration badge
      Rectangle {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 6
        width: durationText.implicitWidth + 8
        height: 20
        radius: 10
        color: "#00000080"
        visible: !cardRoot.isPlaceholder && cardRoot.duration !== "" && thumbnailImage.status === Image.Ready

        Text {
          id: durationText
          anchors.centerIn: parent
          text: cardRoot.duration
          font.pixelSize: 10
          font.bold: true
          color: "#fff"
        }
      }

      // Resume badge
      Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 6
        width: resumeBadge.implicitWidth + 12
        height: 24
        radius: 12
        color: tm ? tm.accent : BlueTheme.accent
        visible: cardRoot.hasProgress && !cardRoot.isPlaceholder && thumbnailImage.status === Image.Ready

        Text {
          id: resumeBadge
          anchors.centerIn: parent
          text: "▶ Reprendre"
          font.pixelSize: 10
          font.bold: true
          color: "#fff"
        }
      }
    }

    ColumnLayout {
      Layout.fillWidth: true
      spacing: 4

      Text {
        text: cardRoot.videoTitle
        font.family: BlueTheme.fontFamily
        font.pixelSize: 12
        font.bold: true
        color: tm ? tm.primaryText : BlueTheme.primaryText
        elide: Text.ElideRight
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        Layout.fillWidth: true
      }

      Text {
        text: cardRoot.userName
        font.family: BlueTheme.fontFamily
        font.pixelSize: 11
        color: tm ? tm.secondaryText : BlueTheme.secondaryText
        elide: Text.ElideRight
        Layout.fillWidth: true
      }

      Text {
        text: cardRoot.viewCount !== "" ? cardRoot.viewCount + " vues" : ""
        font.family: BlueTheme.fontFamily
        font.pixelSize: 10
        color: tm ? tm.accent : BlueTheme.accent
        Layout.fillWidth: true
      }
    }
  }
}
