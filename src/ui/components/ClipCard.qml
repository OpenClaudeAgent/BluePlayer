import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/AppleTheme.js" as AppleTheme

Item {
  id: cardRoot
  property string clipTitle: ""
  property string broadcasterName: ""
  property string viewCount: ""
  property string duration: ""
  property string thumbnailUrl: ""
  property bool isPlaceholder: false

  implicitWidth: 180
  implicitHeight: 220

  Rectangle {
    id: cardBackground
    anchors.fill: parent
    radius: 12
    color: cardRoot.isPlaceholder ? AppleTheme.surfaceSoft : AppleTheme.surface
    border.color: AppleTheme.divider
    border.width: 1

    states: [
      State {
        name: "hovered"
        when: mouseArea.containsMouse
        PropertyChanges {
          target: cardBackground
          color: cardRoot.isPlaceholder ? "#252d3d" : "#1a2330"
          scale: 1.02
        }
        PropertyChanges {
          target: cardShadow
          opacity: 0.3
        }
      }
    ]

    transitions: Transition {
      NumberAnimation {
        properties: "scale, opacity"
        duration: 200
        easing.type: Easing.OutCubic
      }
      ColorAnimation {
        duration: 200
      }
    }

    Rectangle {
      id: cardShadow
      anchors.fill: parent
      anchors.margins: -2
      radius: parent.radius + 2
      color: "transparent"
      border.color: "#00000020"
      border.width: 1
      opacity: 0
    }

    ColumnLayout {
      anchors.fill: parent
      anchors.margins: 12
      spacing: 12

      Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 120
        radius: 8
        color: cardRoot.isPlaceholder ? "#1a2230" : AppleTheme.surfaceSoft
        border.color: AppleTheme.divider
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
            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
          }
        }

        Rectangle {
          anchors.fill: parent
          color: cardRoot.isPlaceholder ? "#1a2230" : AppleTheme.surfaceSoft
          visible: thumbnailImage.status !== Image.Ready || cardRoot.isPlaceholder
          
          Text {
            anchors.centerIn: parent
            text: cardRoot.isPlaceholder ? "⋯" : (thumbnailImage.status === Image.Loading ? "⏳" : "🎬")
            font.pixelSize: 32
            color: AppleTheme.mutedText
            opacity: 0.5
          }
        }

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
      }

      ColumnLayout {
        Layout.fillWidth: true
        spacing: 4

        Text {
          text: cardRoot.clipTitle
          font.family: AppleTheme.fontFamily
          font.pixelSize: 12
          font.bold: true
          color: AppleTheme.primaryText
          elide: Text.ElideRight
          wrapMode: Text.WordWrap
          maximumLineCount: 2
          Layout.fillWidth: true
        }

        Text {
          text: cardRoot.broadcasterName
          font.family: AppleTheme.fontFamily
          font.pixelSize: 11
          color: AppleTheme.secondaryText
          elide: Text.ElideRight
          Layout.fillWidth: true
        }

        Text {
          text: cardRoot.viewCount !== "" ? cardRoot.viewCount + " vues" : ""
          font.family: AppleTheme.fontFamily
          font.pixelSize: 10
          color: AppleTheme.accent
          Layout.fillWidth: true
        }
      }
    }
  }

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true
    cursorShape: Qt.PointingHandCursor
    onClicked: {
      if (!cardRoot.isPlaceholder) {
        console.log("Clicked on clip:", cardRoot.clipTitle)
        // TODO: Play clip
      }
    }
  }
}





