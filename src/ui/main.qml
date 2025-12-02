import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15
import QtMultimedia 6.5

import "themes/AppleTheme.js" as AppleTheme
import "components"

ApplicationWindow {
  id: root
  visible: true
  width: 1200
  height: 780
  color: AppleTheme.windowBackground
  font.family: AppleTheme.fontFamily
  title: qsTr("BluePlayer")
  property string currentView: "home"
  property string statusText: qsTr("Sélectionnez un stream ou une vidéo locale pour commencer.")

  background: Rectangle {
    anchors.fill: parent
    gradient: Gradient {
      GradientStop { position: 0; color: AppleTheme.gradientStart }
      GradientStop { position: 1; color: AppleTheme.gradientEnd }
    }
  }

  property bool preferencesActive: currentView === "preferences"

  Rectangle {
    id: preferencesIcon
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.margins: AppleTheme.spacingSmall
    z: 10
    width: 38
    height: 38
    radius: width / 2
    color: preferencesActive ? AppleTheme.overlayTint : AppleTheme.surface
    border.color: preferencesActive ? AppleTheme.accent : AppleTheme.buttonBorder
    border.width: AppleTheme.borderWidth

    Label {
      anchors.centerIn: parent
      text: "\u2699"
      font.pixelSize: 18
      color: AppleTheme.primaryText
      horizontalAlignment: Text.AlignHCenter
      verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
      anchors.fill: parent
      cursorShape: Qt.PointingHandCursor
      onClicked: currentView = preferencesActive ? "home" : "preferences"
    }

    ToolTip {
      text: qsTr("Préférences")
    }
  }

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: AppleTheme.spacingLarge
    spacing: AppleTheme.spacingMedium

    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true

      Loader {
        id: pageLoader
        anchors.fill: parent
        sourceComponent: currentView === "home" ? homeComponent : preferencesComponent
        Behavior on opacity {
          NumberAnimation { duration: 220 }
        }
      }
    }
  }

  Component {
    id: homeComponent
    HomeView {
    }
  }

  Component {
    id: preferencesComponent
    PreferencesView {
      onCloseRequested: currentView = "home"
    }
  }

  Connections {
    target: ffmpegService
    function onErrorOccurred(message) {
      statusText = message
    }
  }
}
