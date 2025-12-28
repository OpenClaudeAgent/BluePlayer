import QtQuick 2.15
import "../themes/BlueTheme.js" as BlueTheme
import "."  // Pour CircleButton

/**
 * PanelHeader.qml
 * 
 * Header standard pour tous les panels (Preferences, CacheManager, etc.).
 * Titre aligné à gauche, bouton close (X) en haut à droite.
 * 
 * Usage:
 *   PanelHeader {
 *       title: qsTr("Preferences")
 *       onBackClicked: preferencesVisible = false
 *   }
 *   
 *   // Sans bouton close :
 *   PanelHeader {
 *       title: qsTr("Settings")
 *       showBackButton: false
 *   }
 */
Item {
    id: root

    // Theme access
    readonly property var tm: typeof themeManager !== "undefined" ? themeManager : null

    // ─────────────────────────────────────────────────────────────────────────
    // Public Properties
    // ─────────────────────────────────────────────────────────────────────────
    
    /** Titre affiché dans le header */
    property string title: ""
    
    /** Afficher le bouton close */
    property bool showBackButton: true
    
    /** Couleur de fond (avec transparence par défaut) */
    property color backgroundColor: {
        var surfaceColor = tm ? tm.surface : BlueTheme.surface
        return Qt.rgba(surfaceColor.r, surfaceColor.g, surfaceColor.b, 0.6)
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Signals
    // ─────────────────────────────────────────────────────────────────────────
    
    signal backClicked()

    // ─────────────────────────────────────────────────────────────────────────
    // Dimensions
    // ─────────────────────────────────────────────────────────────────────────
    
    implicitHeight: 64
    implicitWidth: parent ? parent.width : 400

    // ─────────────────────────────────────────────────────────────────────────
    // Background
    // ─────────────────────────────────────────────────────────────────────────
    
    Rectangle {
        id: headerBackground
        anchors.fill: parent
        color: root.backgroundColor
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Title (aligned left)
    // ─────────────────────────────────────────────────────────────────────────
    
    Text {
        id: titleText
        
        anchors {
            left: parent.left
            leftMargin: BlueTheme.spacingLarge
            verticalCenter: parent.verticalCenter
        }
        
        text: root.title
        font.pixelSize: 20
        font.weight: Font.DemiBold
        font.family: BlueTheme.fontFamily
        color: tm ? tm.primaryText : BlueTheme.primaryText
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Close Button (X) - Top Right - Uses CircleButton for consistent style
    // ─────────────────────────────────────────────────────────────────────────
    
    CircleButton {
        id: closeButton
        visible: root.showBackButton
        
        anchors {
            right: parent.right
            rightMargin: BlueTheme.spacingMedium
            verticalCenter: parent.verticalCenter
        }
        
        iconText: "\u2715"  // ✕
        iconSize: 16
        tooltipText: ""
        
        onClicked: root.backClicked()
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Bottom Divider
    // ─────────────────────────────────────────────────────────────────────────
    
    Rectangle {
        id: bottomDivider
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: 1
        color: tm ? tm.divider : BlueTheme.divider
    }
}
