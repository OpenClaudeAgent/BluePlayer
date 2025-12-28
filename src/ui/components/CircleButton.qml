import QtQuick 2.15
import QtQuick.Controls 2.15
import "../themes/BlueTheme.js" as BlueTheme

/**
 * CircleButton.qml
 * 
 * Bouton circulaire réutilisable pour les actions globales de navigation.
 * Utilisé pour les boutons ↺ (history), ⚙️ (settings), etc.
 * 
 * Usage:
 *   CircleButton {
 *       iconText: "\u21BA"  // Symbole Unicode
 *       tooltipText: qsTr("Watch History")
 *       active: historyPanelVisible
 *       onClicked: historyPanelVisible = !historyPanelVisible
 *   }
 */
Item {
    id: root

    // Theme access
    readonly property var tm: typeof themeManager !== "undefined" ? themeManager : null

    // ─────────────────────────────────────────────────────────────────────────
    // Public Properties
    // ─────────────────────────────────────────────────────────────────────────
    
    /** Texte/icône Unicode à afficher (ex: "\u21BA", "\u2699") */
    property string iconText: ""
    
    /** État actif (panel ouvert) */
    property bool active: false
    
    /** Texte du tooltip (optionnel, vide = pas de tooltip) */
    property string tooltipText: ""
    
    /** Taille de la police de l'icône */
    property int iconSize: 20
    
    /** Couleur de l'icône (par défaut: primaryText) */
    property color iconColor: tm ? tm.primaryText : BlueTheme.primaryText

    // ─────────────────────────────────────────────────────────────────────────
    // Signals
    // ─────────────────────────────────────────────────────────────────────────
    
    signal clicked()

    // ─────────────────────────────────────────────────────────────────────────
    // Dimensions
    // ─────────────────────────────────────────────────────────────────────────
    
    implicitWidth: 38
    implicitHeight: 38

    // ─────────────────────────────────────────────────────────────────────────
    // Visual Components
    // ─────────────────────────────────────────────────────────────────────────
    
    Rectangle {
        id: buttonBackground
        anchors.fill: parent
        radius: width / 2
        
        // Couleur selon l'état
        color: {
            if (root.active) {
                return tm ? tm.overlayTint : BlueTheme.overlayTint
            } else if (mouseArea.containsMouse) {
                return tm ? tm.surfaceSoft : BlueTheme.surfaceSoft
            } else {
                return tm ? tm.surface : BlueTheme.surface
            }
        }
        
        border.color: root.active 
                      ? (tm ? tm.accent : BlueTheme.accent)
                      : (tm ? tm.buttonBorder : BlueTheme.buttonBorder)
        border.width: BlueTheme.borderWidth

        // Animation de couleur fluide
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

        // Icône
        Text {
            id: iconLabel
            anchors.centerIn: parent
            text: root.iconText
            font.pixelSize: root.iconSize
            font.family: BlueTheme.fontFamily
            color: root.iconColor
            
            // Légère mise à l'échelle au hover
            scale: mouseArea.containsMouse ? 1.05 : 1.0
            
            Behavior on scale {
                NumberAnimation {
                    duration: BlueTheme.animHoverDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Interaction
    // ─────────────────────────────────────────────────────────────────────────
    
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        
        onClicked: root.clicked()
        
        // Effet de pression
        onPressed: buttonBackground.scale = BlueTheme.scalePress
        onReleased: buttonBackground.scale = 1.0
    }
    
    // Animation de l'effet de pression
    Behavior on scale {
        NumberAnimation {
            duration: BlueTheme.animPressDuration
            easing.type: Easing.OutQuart
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Tooltip (optionnel)
    // ─────────────────────────────────────────────────────────────────────────
    
    ToolTip {
        id: tooltip
        visible: root.tooltipText !== "" && mouseArea.containsMouse
        text: root.tooltipText
        delay: 500
        
        background: Rectangle {
            color: tm ? tm.surface : BlueTheme.surface
            border.color: tm ? tm.divider : BlueTheme.divider
            border.width: 1
            radius: 6
        }
        
        contentItem: Text {
            text: tooltip.text
            font.pixelSize: 12
            font.family: BlueTheme.fontFamily
            color: tm ? tm.primaryText : BlueTheme.primaryText
        }
    }
}
