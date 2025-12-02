import QtQuick 2.15
import QtQuick.Controls 6.5

import "../themes/AppleTheme.js" as AppleTheme

Button {
  id: root
  property color tone: AppleTheme.buttonSurface
  property color borderTone: AppleTheme.buttonBorder
  property bool filled: true

  font.family: AppleTheme.fontFamily
  font.pixelSize: 13
  padding: AppleTheme.spacingMedium

  background: Rectangle {
    radius: 14
    border.width: 1
    border.color: borderTone
    color: root.enabled ? (filled ? tone : AppleTheme.surface) : AppleTheme.overlayTint
  }

  contentItem: Label {
    text: root.text
    color: root.enabled ? AppleTheme.primaryText : AppleTheme.mutedText
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    font.bold: root.filled
  }
}

