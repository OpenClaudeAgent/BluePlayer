import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/AppleTheme.js" as AppleTheme

Item {
  id: cardRoot
  property string streamerName: ""
  property string streamTitle: ""
  property string viewerCount: ""
  property bool isPlaceholder: false
  property string previewImage: ""
  property string streamerLogin: ""
  
  signal clicked(string streamerLogin, string streamerName, string streamTitle)

  implicitWidth: 180
  implicitHeight: 220

  Rectangle {
    id: cardBackground
    anchors.fill: parent
    radius: 12
    color: cardRoot.isPlaceholder ? AppleTheme.surfaceSoft : AppleTheme.surface
    border.color: AppleTheme.divider
    border.width: 1

    // Hover effect
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

    // Ombre subtile
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

        // Zone image preview avec chargement asynchrone et cache
        Rectangle {
          Layout.fillWidth: true
          Layout.preferredHeight: 120
          radius: 8
          color: cardRoot.isPlaceholder ? "#1a2230" : AppleTheme.surfaceSoft
          border.color: AppleTheme.divider
          border.width: 1
          clip: true

          // Image avec chargement asynchrone et cache
          Image {
            id: previewImage
            anchors.fill: parent
            source: cardRoot.isPlaceholder ? "" : cardRoot.previewImage
            fillMode: Image.PreserveAspectCrop
            asynchronous: true  // Chargement asynchrone pour ne pas bloquer l'UI
            cache: true  // Utiliser le cache Qt pour éviter les rechargements
            visible: status === Image.Ready && !cardRoot.isPlaceholder
            
            // Animation de fade-in lors du chargement
            opacity: status === Image.Ready ? 1 : 0
            Behavior on opacity {
              NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
            }
          }

          // Placeholder pendant le chargement ou si pas d'image
          Rectangle {
            anchors.fill: parent
            color: cardRoot.isPlaceholder ? "#1a2230" : AppleTheme.surfaceSoft
            visible: previewImage.status !== Image.Ready || cardRoot.isPlaceholder
            
            Text {
              anchors.centerIn: parent
              text: cardRoot.isPlaceholder ? "⋯" : (previewImage.status === Image.Loading ? "⏳" : "📺")
              font.pixelSize: 32
              color: AppleTheme.mutedText
              opacity: 0.5
            }
          }

          // Badge "LIVE" si nécessaire
          Rectangle {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 6
            width: 40
            height: 20
            radius: 10
            color: AppleTheme.statusNegative
            visible: !cardRoot.isPlaceholder && previewImage.status === Image.Ready

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
          font.family: AppleTheme.fontFamily
          font.pixelSize: 14
          font.bold: true
          color: AppleTheme.primaryText
          elide: Text.ElideRight
          Layout.fillWidth: true
        }

        Text {
          text: cardRoot.streamTitle
          font.family: AppleTheme.fontFamily
          font.pixelSize: 12
          color: AppleTheme.secondaryText
          elide: Text.ElideRight
          wrapMode: Text.WordWrap
          maximumLineCount: 2
          Layout.fillWidth: true
        }

        Text {
          text: cardRoot.viewerCount
          font.family: AppleTheme.fontFamily
          font.pixelSize: 11
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
      if (!cardRoot.isPlaceholder && cardRoot.streamerLogin) {
        console.log("Clicked on stream:", cardRoot.streamerName, "login:", cardRoot.streamerLogin)
        cardRoot.clicked(cardRoot.streamerLogin, cardRoot.streamerName, cardRoot.streamTitle)
      }
    }
  }
}

