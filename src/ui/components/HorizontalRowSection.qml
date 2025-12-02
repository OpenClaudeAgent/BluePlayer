import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/AppleTheme.js" as AppleTheme

Item {
  id: root
  property string sectionTitle
  property string sectionSubtitle: ""
  property var cardsModel: []
  property real rowHeight: 220
  property real cardWidth: 180
  property real cardSpacing: 16

  implicitHeight: titleSection.height + flickableSection.height + AppleTheme.spacingMedium

  ColumnLayout {
    id: container
    anchors.fill: parent
    spacing: AppleTheme.spacingMedium

    // Titre et sous-titre de la section
    ColumnLayout {
      id: titleSection
      Layout.fillWidth: true
      spacing: 4

      Text {
        text: root.sectionTitle
        font.family: AppleTheme.fontFamily
        font.pixelSize: 20
        font.bold: true
        color: AppleTheme.primaryText
      }

      Text {
        visible: root.sectionSubtitle !== ""
        text: root.sectionSubtitle
        font.family: AppleTheme.fontFamily
        font.pixelSize: 14
        color: AppleTheme.secondaryText
      }
    }

    // Zone scrollable horizontale
    Flickable {
      id: flickableSection
      Layout.fillWidth: true
      Layout.preferredHeight: root.rowHeight
      contentWidth: Math.max(parent.width, cardsRow.implicitWidth + AppleTheme.spacingLarge)
      clip: true
      flickableDirection: Flickable.HorizontalFlick
      interactive: true

      Row {
        id: cardsRow
        anchors.left: parent.left
        anchors.leftMargin: 0
        anchors.verticalCenter: parent.verticalCenter
        spacing: root.cardSpacing
        height: parent.height

        property real implicitWidth: root.cardsModel.length * (root.cardWidth + root.cardSpacing) - root.cardSpacing

        Repeater {
          model: root.cardsModel
          delegate: StreamCard {
            width: root.cardWidth
            height: root.rowHeight
            streamerName: modelData.name || ""
            streamTitle: modelData.detail || ""
            viewerCount: modelData.viewers || ""
            isPlaceholder: modelData.isPlaceholder || false
          }
        }
      }
    }
  }
}

