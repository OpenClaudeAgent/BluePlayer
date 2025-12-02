import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/AppleTheme.js" as AppleTheme

Item {
  id: cardRoot
  property string channelName: ""
  property string displayName: ""
  property string thumbnailUrl: ""
  property bool isLive: false
  property string gameName: ""
  property bool isPlaceholder: false

  // Dimensions adaptées comme pour les catégories
  implicitWidth: 180
  implicitHeight: 260

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

      // Zone image preview adaptée comme pour les catégories
      Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 200  // Augmenté de 120 à 200 comme pour les catégories
        radius: 8
        color: cardRoot.isPlaceholder ? "#1a2230" : AppleTheme.surfaceSoft
        border.color: AppleTheme.divider
        border.width: 1
        clip: true

        // Image avec chargement asynchrone et cache
        Image {
          id: avatarImage
          anchors.fill: parent
          source: cardRoot.isPlaceholder ? "" : cardRoot.thumbnailUrl
          fillMode: Image.PreserveAspectFit  // Comme pour les catégories
          asynchronous: true
          cache: true
          visible: status === Image.Ready && !cardRoot.isPlaceholder
          
          // Animation de fade-in lors du chargement
          opacity: status === Image.Ready ? 1 : 0
          Behavior on opacity {
            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
          }
        }

        Rectangle {
          anchors.fill: parent
          color: cardRoot.isPlaceholder ? "#1a2230" : AppleTheme.surfaceSoft
          visible: avatarImage.status !== Image.Ready || cardRoot.isPlaceholder
          
          Text {
            anchors.centerIn: parent
            text: cardRoot.isPlaceholder ? "⋯" : (avatarImage.status === Image.Loading ? "⏳" : "👤")
            font.pixelSize: 32
            color: AppleTheme.mutedText
            opacity: 0.5
          }
        }

        Rectangle {
          anchors.top: parent.top
          anchors.right: parent.right
          anchors.margins: 6
          width: 40
          height: 20
          radius: 10
          color: AppleTheme.statusNegative
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
        font.family: AppleTheme.fontFamily
        font.pixelSize: 14
        font.bold: true
        color: AppleTheme.primaryText
        elide: Text.ElideRight
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
      }
      
      // Statut (seulement si en direct avec nom du jeu)
      Text {
        text: cardRoot.isLive && cardRoot.gameName !== "" ? cardRoot.gameName : ""
        font.family: AppleTheme.fontFamily
        font.pixelSize: 11
        color: AppleTheme.accent
        elide: Text.ElideRight
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
        visible: text !== ""
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
        console.log("Clicked on channel:", cardRoot.channelName)
        // TODO: Navigate to channel
      }
    }
  }
}


