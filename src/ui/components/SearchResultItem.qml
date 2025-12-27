import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

Rectangle {
  id: itemRoot
  
  property string itemText: ""
  property string itemSubtext: ""
  property string thumbnailUrl: ""
  property bool isLive: false
  property bool isCategory: false
  property bool isSelected: false
  
  signal clicked()
  
  implicitHeight: 48
  color: isSelected ? BlueTheme.surfaceSoft : (mouseArea.containsMouse ? "#1a2230" : "transparent")
  radius: 8
  
  Behavior on color {
    ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
  }
  
  RowLayout {
    anchors.fill: parent
    anchors.leftMargin: 8
    anchors.rightMargin: 8
    spacing: 12
    
    // Thumbnail/Avatar
    Rectangle {
      Layout.preferredWidth: isCategory ? 32 : 36
      Layout.preferredHeight: isCategory ? 42 : 36
      radius: isCategory ? 4 : 18
      color: BlueTheme.surfaceSoft
      clip: true
      
      Image {
        id: thumbnailImage
        anchors.fill: parent
        source: itemRoot.thumbnailUrl
        fillMode: isCategory ? Image.PreserveAspectCrop : Image.PreserveAspectCrop
        asynchronous: true
        cache: true
        visible: status === Image.Ready
        
        opacity: status === Image.Ready ? 1 : 0
        Behavior on opacity {
          NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
        }
      }
      
      // Placeholder
      Text {
        anchors.centerIn: parent
        text: isCategory ? "🎮" : "👤"
        font.pixelSize: 16
        color: BlueTheme.mutedText
        visible: thumbnailImage.status !== Image.Ready
        opacity: 0.5
      }
    }
    
    // Text content
    ColumnLayout {
      Layout.fillWidth: true
      spacing: 2
      
      RowLayout {
        Layout.fillWidth: true
        spacing: 6
        
        Text {
          text: itemRoot.itemText
          font.family: BlueTheme.fontFamily
          font.pixelSize: 14
          font.bold: true
          color: BlueTheme.primaryText
          elide: Text.ElideRight
          Layout.fillWidth: true
        }
        
        // Live badge
        Rectangle {
          visible: itemRoot.isLive && !itemRoot.isCategory
          width: 36
          height: 16
          radius: 8
          color: BlueTheme.statusNegative
          
          Text {
            anchors.centerIn: parent
            text: "LIVE"
            font.family: BlueTheme.fontFamily
            font.pixelSize: 9
            font.bold: true
            color: "#ffffff"
          }
        }
      }
      
      // Subtext (game name for live channels)
      Text {
        visible: itemRoot.itemSubtext !== ""
        text: itemRoot.itemSubtext
        font.family: BlueTheme.fontFamily
        font.pixelSize: 12
        color: BlueTheme.secondaryText
        elide: Text.ElideRight
        Layout.fillWidth: true
      }
    }
    
    // Arrow indicator
    Text {
      text: "→"
      font.pixelSize: 14
      color: BlueTheme.mutedText
      opacity: mouseArea.containsMouse || itemRoot.isSelected ? 1 : 0
      
      Behavior on opacity {
        NumberAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
      }
    }
  }
  
  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true
    cursorShape: Qt.PointingHandCursor
    onClicked: itemRoot.clicked()
  }
}
