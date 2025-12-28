import QtQuick 2.15
import "../themes/BlueTheme.js" as BlueTheme

/**
 * PanelHeader.qml
 * 
 * Header standard pour tous les panels (Preferences, CacheManager, etc.).
 * Fournit un bouton retour et un titre avec un style uniforme.
 * 
 * Usage:
 *   PanelHeader {
 *       title: qsTr("Preferences")
 *       onBackClicked: preferencesVisible = false
 *   }
 *   
 *   // Ou sans bouton retour :
 *   PanelHeader {
 *       title: qsTr("Settings")
 *       showBackButton: false
 *   }
 */
Item {
    id: root

    // ─────────────────────────────────────────────────────────────────────────
    // Public Properties
    // ─────────────────────────────────────────────────────────────────────────
    
    /** Titre affiché dans le header */
    property string title: ""
    
    /** Afficher le bouton retour */
    property bool showBackButton: true
    
    /** Couleur de fond (avec transparence par défaut) */
    property color backgroundColor: Qt.rgba(
        BlueTheme.surface.r,
        BlueTheme.surface.g,
        BlueTheme.surface.b,
        0.6
    )

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
    // Back Button
    // ─────────────────────────────────────────────────────────────────────────
    
    Rectangle {
        id: backButton
        visible: root.showBackButton
        
        anchors {
            left: parent.left
            leftMargin: BlueTheme.spacingMedium
            verticalCenter: parent.verticalCenter
        }
        
        width: 36
        height: 36
        radius: 18
        
        color: backButtonArea.containsMouse ? "#1AFFFFFF" : "transparent"
        
        Behavior on color {
            ColorAnimation {
                duration: BlueTheme.animHoverDuration
                easing.type: Easing.OutCubic
            }
        }
        
        // Icône flèche retour
        Text {
            id: backIcon
            anchors.centerIn: parent
            text: "\u2190"  // ←
            font.pixelSize: 20
            font.family: BlueTheme.fontFamily
            color: BlueTheme.primaryText
            
            // Légère translation au hover
            x: backButtonArea.containsMouse ? -2 : 0
            
            Behavior on x {
                NumberAnimation {
                    duration: BlueTheme.animHoverDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
        
        MouseArea {
            id: backButtonArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            
            onClicked: root.backClicked()
            
            // Effet de pression
            onPressed: backButton.scale = BlueTheme.scalePress
            onReleased: backButton.scale = 1.0
        }
        
        Behavior on scale {
            NumberAnimation {
                duration: BlueTheme.animPressDuration
                easing.type: Easing.OutQuart
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Title
    // ─────────────────────────────────────────────────────────────────────────
    
    Text {
        id: titleText
        
        anchors {
            left: root.showBackButton ? backButton.right : parent.left
            leftMargin: root.showBackButton ? BlueTheme.spacingMedium : BlueTheme.spacingLarge
            verticalCenter: parent.verticalCenter
        }
        
        text: root.title
        font.pixelSize: 20
        font.weight: Font.DemiBold
        font.family: BlueTheme.fontFamily
        color: BlueTheme.primaryText
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
        color: BlueTheme.divider
    }
}
