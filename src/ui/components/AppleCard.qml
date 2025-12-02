import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
  id: appleCard
  width: 220
  height: 140
  // Allow consumers to inject arbitrary content inside this card
  default property alias content: contentContainer.children

  Rectangle {
    anchors.fill: parent
    color: "#1b2130"
    radius: 8
    border.color: "#2a324e"
    border.width: 1
    // content holder
    ColumnLayout {
      id: contentContainer
      anchors.fill: parent
      anchors.margins: 16
    }
  }
}

