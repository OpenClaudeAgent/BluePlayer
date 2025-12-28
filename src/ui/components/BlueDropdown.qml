import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import "../themes/BlueTheme.js" as BlueTheme

/**
 * BlueDropdown - Styled ComboBox for BluePlayer design system
 * 
 * Uses Qt's native ComboBox which handles z-index and positioning correctly.
 */
ComboBox {
    id: root

    // Theme access
    readonly property var tm: typeof themeManager !== "undefined" ? themeManager : null

    // Size
    implicitWidth: 140
    implicitHeight: 36

    // Custom properties
    property string placeholder: ""

    // Selected value (use selectedValue instead of currentValue to avoid FINAL property conflict)
    property string selectedValue: ""
    signal valueSelected(string value)

    // Sync currentIndex when selectedValue changes
    onSelectedValueChanged: {
        if (model) {
            for (var i = 0; i < model.length; i++) {
                if (model[i] === selectedValue) {
                    currentIndex = i
                    break
                }
            }
        }
    }

    Component.onCompleted: {
        if (selectedValue && model) {
            for (var i = 0; i < model.length; i++) {
                if (model[i] === selectedValue) {
                    currentIndex = i
                    break
                }
            }
        }
    }

    onActivated: function(index) {
        selectedValue = textAt(index)
        valueSelected(textAt(index))
    }

    // =========================================================================
    // Button background
    // =========================================================================
    background: Rectangle {
        radius: 10
        color: {
            var surfaceColor = root.tm ? root.tm.surfaceSoft : BlueTheme.surfaceSoft
            if (root.hovered || root.popup.visible) {
                return Qt.rgba(surfaceColor.r, surfaceColor.g, surfaceColor.b, 0.8)
            }
            return Qt.rgba(surfaceColor.r, surfaceColor.g, surfaceColor.b, 0.5)
        }
        border.color: root.popup.visible ? (root.tm ? root.tm.accent : BlueTheme.accent) : (root.tm ? root.tm.divider : BlueTheme.divider)
        border.width: 1

        Behavior on color {
            ColorAnimation { 
                duration: BlueTheme.animHoverDuration
                easing.type: Easing.OutCubic 
            }
        }
        Behavior on border.color {
            ColorAnimation { 
                duration: BlueTheme.animHoverDuration
                easing.type: Easing.OutCubic 
            }
        }
    }

    // =========================================================================
    // Content item (selected text)
    // =========================================================================
    contentItem: Item {
        anchors.fill: parent

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            anchors.rightMargin: 28  // Space for indicator
            text: root.displayText || root.placeholder
            font.family: BlueTheme.fontFamily
            font.pixelSize: 13
            font.weight: Font.Medium
            color: root.displayText ? (root.tm ? root.tm.primaryText : BlueTheme.primaryText) : (root.tm ? root.tm.secondaryText : BlueTheme.secondaryText)
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
    }

    // =========================================================================
    // Dropdown indicator
    // =========================================================================
    indicator: Text {
        x: root.width - width - 10
        y: (root.height - height) / 2
        text: "\u25BC"
        font.pixelSize: 8
        color: root.tm ? root.tm.secondaryText : BlueTheme.secondaryText
        rotation: root.popup.visible ? 180 : 0

        Behavior on rotation {
            NumberAnimation { 
                duration: BlueTheme.animHoverDuration
                easing.type: Easing.OutCubic 
            }
        }
    }

    // =========================================================================
    // Popup styling
    // =========================================================================
    popup: Popup {
        y: root.height + 4
        width: root.width
        implicitHeight: contentItem.implicitHeight + 16
        padding: 8

        background: Rectangle {
            radius: 12
            color: root.tm ? root.tm.surface : BlueTheme.surface
            border.color: root.tm ? root.tm.divider : BlueTheme.divider
            border.width: 1

            // Shadow effect
            layer.enabled: true
            layer.effect: null
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds

            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }

    // =========================================================================
    // Delegate for each option (simple Rectangle pour contrôle total)
    // =========================================================================
    delegate: Rectangle {
        id: delegateItem
        width: root.width - 16
        height: 32
        radius: 6
        color: delegateMouseArea.containsMouse ? (root.tm ? root.tm.cardHighlight : BlueTheme.cardHighlight) : "transparent"

        required property var model
        required property int index

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Text {
                text: delegateItem.model[root.textRole] || delegateItem.model.modelData || ""
                font.family: BlueTheme.fontFamily
                font.pixelSize: 13
                font.weight: root.currentIndex === delegateItem.index ? Font.DemiBold : Font.Normal
                color: root.currentIndex === delegateItem.index ? (root.tm ? root.tm.accent : BlueTheme.accent) : (root.tm ? root.tm.primaryText : BlueTheme.primaryText)
                Layout.fillWidth: true
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Text {
                text: root.currentIndex === delegateItem.index ? "\u2713" : ""
                font.pixelSize: 12
                font.weight: Font.Bold
                color: root.tm ? root.tm.accent : BlueTheme.accent
                verticalAlignment: Text.AlignVCenter
            }
        }

        MouseArea {
            id: delegateMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.currentIndex = delegateItem.index
                root.activated(delegateItem.index)
                root.popup.close()
            }
        }
    }
}
