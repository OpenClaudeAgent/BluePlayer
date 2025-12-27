import QtQuick 2.15
import QtQuick.Layouts 1.15
import "../themes/BlueTheme.js" as BlueTheme

/**
 * ErrorToast - Toast d'erreur avec timer auto-hide
 * 
 * Propriétés:
 *   - message: Message d'erreur à afficher
 *   - visible: Contrôle la visibilité
 *   - autoHideDuration: Durée avant masquage auto (ms), 0 = pas d'auto-hide
 */
Rectangle {
    id: errorToast
    
    // Properties
    property string message: ""
    property bool showError: false
    property int autoHideDuration: 4000
    
    radius: 10
    color: "#CCB00020"
    border.color: "#FF5252"
    visible: showError
    opacity: showError ? 1.0 : 0.0
    
    // Auto-size based on content
    implicitWidth: contentRow.width + 24
    implicitHeight: contentRow.height + 24
    
    Behavior on opacity { 
        NumberAnimation { 
            duration: BlueTheme.animToastEnterDuration
            easing.type: Easing.OutCubic 
        } 
    }

    Row {
        id: contentRow
        anchors.centerIn: parent
        spacing: 8
        
        Text { 
            text: "\u26A0" // Warning icon
            color: "#FFFFFF"
            font.pixelSize: 13 
        }
        
        Text { 
            text: errorToast.message
            color: "#FFFFFF"
            font.pixelSize: 13
            wrapMode: Text.WordWrap
            maximumLineCount: 3
        }
    }
    
    Timer {
        id: hideTimer
        interval: errorToast.autoHideDuration
        running: errorToast.showError && errorToast.autoHideDuration > 0
        repeat: false
        onTriggered: errorToast.showError = false
    }
    
    // Public function to show error
    function show(errorMessage) {
        message = errorMessage
        showError = true
    }
    
    // Public function to hide
    function hide() {
        showError = false
    }
}
