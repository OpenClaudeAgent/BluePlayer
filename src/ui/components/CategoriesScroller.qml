import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

Item {
  id: root
  property string sectionTitle
  property string sectionSubtitle
  property var cardsModel
  property real availableWidth

  ColumnLayout {
    width: availableWidth
    spacing: BlueTheme.spacingSmall

    Text {
      text: sectionTitle
      color: BlueTheme.primaryText
      font.bold: true
      font.pixelSize: 14
    }
    Text {
      text: sectionSubtitle
      color: BlueTheme.secondaryText
      font.pixelSize: 12
    }

    Flickable {
      width: parent.width
      height: 150
      contentWidth: cardsModel ? cardsModel.length * 210 : 0
      clip: true
      flickableDirection: Flickable.HorizontalFlick
      Row {
        id: cardsRow
        spacing: 10
        Repeater {
          model: cardsModel
          delegate: BlueCard {
            Layout.preferredWidth: 200
            ColumnLayout {
              anchors.fill: parent
              spacing: BlueTheme.spacingSmall
              Label { text: name; color: BlueTheme.primaryText; font.pixelSize: 12; font.bold: true }
              Label { text: detail; color: BlueTheme.secondaryText; font.pixelSize: 11; wrapMode: Text.WordWrap }
              Label { text: viewers; color: BlueTheme.accent; font.pixelSize: 11 }
            }
          }
        }
      }
    }
  }
}

