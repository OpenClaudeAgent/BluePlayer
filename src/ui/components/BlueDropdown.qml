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
        color: root.hovered || root.popup.visible ? 
               Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(1, 1, 1, 0.05)
        border.color: root.popup.visible ? BlueTheme.accent : BlueTheme.divider
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
            color: root.displayText ? BlueTheme.primaryText : BlueTheme.secondaryText
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
        color: BlueTheme.secondaryText
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
            color: BlueTheme.surface
            border.color: BlueTheme.divider
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
    // Delegate for each option
    // =========================================================================
    delegate: ItemDelegate {
        id: delegateItem
        width: root.width - 16
        height: 32

        required property var model
        required property int index

        background: Rectangle {
            radius: 6
            color: delegateItem.hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent"

            Behavior on color {
                ColorAnimation { 
                    duration: BlueTheme.animHoverDuration
                    easing.type: Easing.OutCubic 
                }
            }
        }

        contentItem: RowLayout {
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            Text {
                text: delegateItem.model[root.textRole] || delegateItem.model.modelData || ""
                font.family: BlueTheme.fontFamily
                font.pixelSize: 13
                font.weight: root.currentIndex === delegateItem.index ? Font.DemiBold : Font.Normal
                color: root.currentIndex === delegateItem.index ? BlueTheme.accent : BlueTheme.primaryText
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            Text {
                text: root.currentIndex === delegateItem.index ? "\u2713" : ""
                font.pixelSize: 12
                font.weight: Font.Bold
                color: BlueTheme.accent
            }
        }

        highlighted: root.highlightedIndex === index
    }
}
