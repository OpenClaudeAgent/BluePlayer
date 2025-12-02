import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import "themes/AppleTheme.js" as AppleTheme
import "components"

Item {
  id: preferencesRoot
  signal closeRequested()

  Rectangle {
    anchors.fill: parent
    color: AppleTheme.windowBackground
    opacity: 0.98
  }

  ScrollView {
    anchors.fill: parent
    ColumnLayout {
      id: prefsLayout
      anchors.fill: parent
      anchors.margins: AppleTheme.spacingLarge
      spacing: AppleTheme.spacingLarge

      RowLayout {
        Layout.fillWidth: true
        spacing: AppleTheme.spacingSmall

        Label {
          text: qsTr("Préférences")
          font.pixelSize: 24
          font.bold: true
          color: AppleTheme.primaryText
        }

        Item { Layout.fillWidth: true }

        AppleButton {
          text: qsTr("Revenir à l'accueil")
          onClicked: closeRequested()
          filled: false
        }
      }

      AppleCard {
        Layout.fillWidth: true
        ColumnLayout {
          spacing: AppleTheme.spacingMedium

          Label {
            text: qsTr("Confort visuel")
            font.pixelSize: 16
            font.bold: true
            color: AppleTheme.primaryText
          }

          RowLayout {
            spacing: AppleTheme.spacingMedium
            Switch { checked: true }
            Label {
              text: qsTr("Activer l’apparence sombre")
              color: AppleTheme.secondaryText
            }
          }

          RowLayout {
            spacing: AppleTheme.spacingMedium
            Slider {
              Layout.fillWidth: true
              from: 0
              to: 100
              value: 32
            }
            Label {
              text: qsTr("Réactivité des animations")
              color: AppleTheme.secondaryText
            }
          }
        }
      }

      AppleCard {
        Layout.fillWidth: true
        ColumnLayout {
          spacing: AppleTheme.spacingMedium

          Label {
            text: qsTr("Lecture")
            font.pixelSize: 16
            font.bold: true
            color: AppleTheme.primaryText
          }

          RowLayout {
            spacing: AppleTheme.spacingMedium
            Switch { checked: true }
            Label {
              text: qsTr("Notifications automatiques")
              color: AppleTheme.secondaryText
            }
          }

          Text {
            text: qsTr("Les réglages ici sont symboliques pour la vue préférences.")
            color: AppleTheme.mutedText
            font.pixelSize: 12
            wrapMode: Text.WordWrap
          }
        }
      }

      AppleButton {
        text: qsTr("Fermer et revenir à l’accueil")
        onClicked: closeRequested()
        Layout.fillWidth: true
      }
    }
  }
}

