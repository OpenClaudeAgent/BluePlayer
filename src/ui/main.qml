import QtQuick
import QtQuick.Controls

ApplicationWindow {
  id: root
  visible: true
  width: 960
  height: 540
  title: qsTr("BluePlayer")

  Rectangle {
    anchors.fill: parent
    color: "#111315"

    Column {
      anchors.centerIn: parent
      spacing: 12

      Label {
        text: qsTr("BluePlayer – Prototype")
        font.pixelSize: 26
        color: "white"
      }

      Label {
        text: qsTr("Pipeline Twitch en cours de construction…")
        font.pixelSize: 16
        color: "#cccccc"
      }
    }
  }
}

