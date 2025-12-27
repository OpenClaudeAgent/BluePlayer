import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

/**
 * LoadingOverlay - Indicateur de chargement centré
 * 
 * Propriétés:
 *   - loading: Si true, l'overlay est visible
 */
Rectangle {
    id: loadingOverlay
    
    // Properties
    property bool loading: false
    
    width: 120
    height: 120
    radius: 20
    color: "#80000000"
    visible: loading
    
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 16
        
        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            running: loadingOverlay.loading
            palette.dark: "#FFFFFF" // Force white indicator
        }
    }
}
