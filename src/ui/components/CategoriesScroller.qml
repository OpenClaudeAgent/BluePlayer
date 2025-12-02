import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import "../themes/AppleTheme.js" as AppleTheme

Item {
  id: root
  property string sectionTitle
  property string sectionSubtitle
  property var cardsModel
  property real availableWidth

  ColumnLayout {
    width: availableWidth
    spacing: AppleTheme.spacingSmall

    Text {
      text: sectionTitle
      color: AppleTheme.primaryText
      font.bold: true
      font.pixelSize: 14
    }
    Text {
      text: sectionSubtitle
      color: AppleTheme.secondaryText
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
          delegate: AppleCard {
            Layout.preferredWidth: 200
            ColumnLayout {
              anchors.fill: parent
              spacing: AppleTheme.spacingSmall
              Label { text: name; color: AppleTheme.primaryText; font.pixelSize: 12; font.bold: true }
              Label { text: detail; color: AppleTheme.secondaryText; font.pixelSize: 11; wrapMode: Text.WordWrap }
              Label { text: viewers; color: AppleTheme.accent; font.pixelSize: 11 }
            }
          }
        }
      }
    }
  }
}

