import QtQuick 2.15
import QtQuick.Controls 6.5

import "../themes/BlueTheme.js" as BlueTheme

Button {
  id: root
  property color tone: BlueTheme.buttonSurface
  property color borderTone: BlueTheme.buttonBorder
  property bool filled: true

  font.family: BlueTheme.fontFamily
  font.pixelSize: 13
  padding: BlueTheme.spacingMedium

  background: Rectangle {
    radius: 14
    border.width: 1
    border.color: borderTone
    color: root.enabled ? (filled ? tone : BlueTheme.surface) : BlueTheme.overlayTint
  }

  contentItem: Label {
    text: root.text
    color: root.enabled ? BlueTheme.primaryText : BlueTheme.mutedText
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    font.bold: root.filled
  }
}

