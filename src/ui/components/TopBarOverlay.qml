import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

/**
 * TopBarOverlay - Barre supérieure avec gradient, bouton retour et infos stream
 * 
 * Propriétés:
 *   - streamerName: Nom du streamer
 *   - streamerLogin: Login du streamer  
 *   - streamTitle: Titre du stream
 *   - statusText: Texte de statut
 *   - controlsVisible: Visibilité des contrôles
 * 
 * Signaux:
 *   - backClicked: Émis quand le bouton retour est cliqué
 */
Rectangle {
    id: topBar
    
    // Properties
    property string streamerName: ""
    property string streamerLogin: ""
    property string streamTitle: ""
    property string statusText: ""
    property bool controlsVisible: true
    
    // Signals
    signal backClicked()
    
    height: 80
    
    gradient: Gradient {
        GradientStop { position: 0.0; color: "#CC000000" }
        GradientStop { position: 1.0; color: "transparent" }
    }
    
    // Visibility Animation
    opacity: controlsVisible ? 1.0 : 0.0
    visible: opacity > 0
    Behavior on opacity { 
        NumberAnimation { 
            duration: BlueTheme.animControlBarDuration
            easing.type: Easing.InOutCubic 
        } 
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        spacing: 16
        
        // Back Button
        Rectangle {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 40
            radius: 20
            color: backButtonMouseArea.containsMouse ? "#4DFFFFFF" : "#1AFFFFFF"
            
            Text {
                anchors.centerIn: parent
                text: "\u2190" // ←
                font.pixelSize: 22
                color: "#FFFFFF"
            }
            
            MouseArea {
                id: backButtonMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: topBar.backClicked()
            }
        }
        
        // Stream Info
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            
            Text {
                text: streamerName || streamerLogin
                font.family: BlueTheme.fontFamily
                font.pixelSize: 18
                font.bold: true
                color: "#FFFFFF"
                style: Text.Outline
                styleColor: "#80000000"
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            
            Text {
                text: streamTitle || qsTr("Live stream")
                font.family: BlueTheme.fontFamily
                font.pixelSize: 13
                color: "#DDFFFFFF"
                style: Text.Outline
                styleColor: "#80000000"
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }
        
        // Status Badge
        Rectangle {
            Layout.preferredHeight: 24
            Layout.preferredWidth: statusLabel.width + 16
            radius: 12
            color: "#4D000000"
            visible: statusText.length > 0
            
            Text {
                id: statusLabel
                anchors.centerIn: parent
                text: statusText
                font.pixelSize: 11
                color: "#FFFFFF"
            }
        }
    }
}
