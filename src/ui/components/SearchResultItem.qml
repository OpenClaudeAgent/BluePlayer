import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/AppleTheme.js" as AppleTheme

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
  color: isSelected ? AppleTheme.surfaceSoft : (mouseArea.containsMouse ? "#1a2230" : "transparent")
  radius: 8
  
  Behavior on color {
    ColorAnimation { duration: 150; easing.type: Easing.OutCubic }
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
      color: AppleTheme.surfaceSoft
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
          NumberAnimation { duration: 150 }
        }
      }
      
      // Placeholder
      Text {
        anchors.centerIn: parent
        text: isCategory ? "🎮" : "👤"
        font.pixelSize: 16
        color: AppleTheme.mutedText
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
          font.family: AppleTheme.fontFamily
          font.pixelSize: 14
          font.bold: true
          color: AppleTheme.primaryText
          elide: Text.ElideRight
          Layout.fillWidth: true
        }
        
        // Live badge
        Rectangle {
          visible: itemRoot.isLive && !itemRoot.isCategory
          width: 36
          height: 16
          radius: 8
          color: AppleTheme.statusNegative
          
          Text {
            anchors.centerIn: parent
            text: "LIVE"
            font.family: AppleTheme.fontFamily
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
        font.family: AppleTheme.fontFamily
        font.pixelSize: 12
        color: AppleTheme.secondaryText
        elide: Text.ElideRight
        Layout.fillWidth: true
      }
    }
    
    // Arrow indicator
    Text {
      text: "→"
      font.pixelSize: 14
      color: AppleTheme.mutedText
      opacity: mouseArea.containsMouse || itemRoot.isSelected ? 1 : 0
      
      Behavior on opacity {
        NumberAnimation { duration: 150 }
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
