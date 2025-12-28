import QtQuick 2.15
import "../themes/BlueTheme.js" as BlueTheme

Item {
  id: baseCard

  // Props communes
  property bool isPlaceholder: false
  property int cardWidth: 180
  property int cardHeight: 220
  property int cardRadius: 12
  property int contentMargins: 12

  // Alias pour permettre aux enfants d'accéder au hover state
  readonly property alias hovered: mouseArea.containsMouse

  // Signal générique
  signal cardClicked()

  // Alias pour le contenu
  default property alias content: contentContainer.data

  implicitWidth: cardWidth
  implicitHeight: cardHeight

  Rectangle {
    id: cardBackground
    anchors.fill: parent
    radius: baseCard.cardRadius
    color: baseCard.isPlaceholder ? BlueTheme.surfaceSoft : BlueTheme.surface
    border.color: BlueTheme.divider
    border.width: 1

    // Hover effect
    states: [
      State {
        name: "hovered"
        when: mouseArea.containsMouse
        PropertyChanges {
          target: cardBackground
          color: baseCard.isPlaceholder ? "#252d3d" : "#1a2330"
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
        duration: BlueTheme.animCardDuration
        easing.type: Easing.OutCubic
      }
      ColorAnimation {
        duration: BlueTheme.animCardDuration
        easing.type: Easing.OutCubic
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

    Item {
      id: contentContainer
      anchors.fill: parent
      anchors.margins: baseCard.contentMargins
    }
  }

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true
    cursorShape: Qt.PointingHandCursor
    onClicked: {
      if (!baseCard.isPlaceholder) {
        baseCard.cardClicked()
      }
    }
  }
}
