import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import "themes/BlueTheme.js" as BlueTheme
import "components"

Item {
    id: preferencesRoot

    signal closeRequested()

    // Default quality preference (will be connected to backend later)
    property string defaultQuality: "Auto"

    // Quality options
    readonly property var qualityOptions: ["Auto", "1080p60", "1080p", "720p60", "720p", "480p", "360p"]

    // Cache refresh trigger (incremented to force binding refresh)
    property int cacheRefreshTrigger: 0

    // Access helpers
    function getCacheManager() {
        return typeof cacheManager !== "undefined" ? cacheManager : null
    }

    function getTwitchService() {
        return typeof twitchService !== "undefined" ? twitchService : null
    }

    // Background with subtle gradient
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: BlueTheme.gradientStart }
            GradientStop { position: 1.0; color: BlueTheme.gradientEnd }
        }
    }

    // Main content
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        // Header - Utilise le composant PanelHeader unifié
        PanelHeader {
            Layout.fillWidth: true
            title: qsTr("Preferences")
            onBackClicked: preferencesRoot.closeRequested()
        }

        // Scrollable content
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: BlueTheme.spacingLarge

                Item { Layout.preferredHeight: BlueTheme.spacingMedium }

                // Center container for sections
                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: Math.min(560, preferencesRoot.width - 48)
                    spacing: BlueTheme.spacingLarge

                    // ═══════════════════════════════════════════════════════════
                    // SECTION: Lecture
                    // ═══════════════════════════════════════════════════════════
                    PreferencesSection {
                        Layout.fillWidth: true
                        title: qsTr("Lecture")
                        icon: "\uD83C\uDFAC"

                        RowLayout {
                            width: parent.width
                            spacing: BlueTheme.spacingMedium

                            Text {
                                text: qsTr("Qualite par defaut")
                                font.family: BlueTheme.fontFamily
                                font.pixelSize: 14
                                color: BlueTheme.primaryText
                                Layout.fillWidth: true
                            }

                            // Quality dropdown using BlueDropdown component
                            BlueDropdown {
                                id: qualityDropdown
                                Layout.preferredWidth: 120
                                Layout.preferredHeight: 36
                                model: preferencesRoot.qualityOptions
                                selectedValue: preferencesRoot.defaultQuality
                                placeholder: qsTr("Qualite")
                                
                                onValueSelected: function(value) {
                                    preferencesRoot.defaultQuality = value
                                }
                            }
                        }
                    }

                    // ═══════════════════════════════════════════════════════════
                    // SECTION: Compte Twitch
                    // ═══════════════════════════════════════════════════════════
                    PreferencesSection {
                        Layout.fillWidth: true
                        title: qsTr("Compte Twitch")
                        icon: "\uD83D\uDC64"

                        ColumnLayout {
                            width: parent.width
                            spacing: BlueTheme.spacingMedium

                            // Connection status
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: BlueTheme.spacingSmall

                                // Status indicator
                                Rectangle {
                                    Layout.preferredWidth: 8
                                    Layout.preferredHeight: 8
                                    radius: 4
                                    color: {
                                        var service = getTwitchService()
                                        return service && service.authenticated ? 
                                               BlueTheme.statusPositive : BlueTheme.statusNegative
                                    }

                                    // Pulse animation when connected
                                    SequentialAnimation on opacity {
                                        loops: Animation.Infinite
                                        running: {
                                            var service = getTwitchService()
                                            return service && service.authenticated
                                        }
                                        NumberAnimation { to: 0.5; duration: 1000; easing.type: Easing.InOutSine }
                                        NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutSine }
                                    }
                                }

                                Text {
                                    text: {
                                        var service = getTwitchService()
                                        if (service && service.authenticated) {
                                            var userName = service.userName || ""
                                            return userName ? 
                                                qsTr("Connecte en tant que @%1").arg(userName) : 
                                                qsTr("Connecte a Twitch")
                                        }
                                        return qsTr("Non connecte")
                                    }
                                    font.family: BlueTheme.fontFamily
                                    font.pixelSize: 14
                                    color: BlueTheme.primaryText
                                    Layout.fillWidth: true
                                }
                            }

                            // Action button
                            RowLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: BlueTheme.spacingSmall

                                Item { Layout.fillWidth: true }

                                Rectangle {
                                    id: twitchActionButton
                                    property bool isLoggedIn: {
                                        var service = getTwitchService()
                                        return service && service.authenticated
                                    }

                                    Layout.preferredWidth: twitchActionText.width + 32
                                    Layout.preferredHeight: 36
                                    radius: 18
                                    color: twitchActionArea.containsMouse ? 
                                           (isLoggedIn ? Qt.rgba(BlueTheme.statusNegative.r, BlueTheme.statusNegative.g, BlueTheme.statusNegative.b, 0.2) : 
                                                        Qt.rgba(BlueTheme.accent.r, BlueTheme.accent.g, BlueTheme.accent.b, 0.2)) :
                                           (isLoggedIn ? "transparent" : BlueTheme.accent)
                                    border.color: isLoggedIn ? BlueTheme.statusNegative : BlueTheme.accent
                                    border.width: isLoggedIn ? 1 : 0

                                    Behavior on color {
                                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                                    }

                                    Text {
                                        id: twitchActionText
                                        anchors.centerIn: parent
                                        text: twitchActionButton.isLoggedIn ? qsTr("Se deconnecter") : qsTr("Se connecter")
                                        font.family: BlueTheme.fontFamily
                                        font.pixelSize: 13
                                        font.weight: Font.Medium
                                        color: twitchActionButton.isLoggedIn ? BlueTheme.statusNegative : "#FFFFFF"
                                    }

                                    MouseArea {
                                        id: twitchActionArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: {
                                            var service = getTwitchService()
                                            if (service) {
                                                if (service.authenticated) {
                                                    console.log("[PreferencesView] Logging out...")
                                                    service.logout()
                                                } else {
                                                    console.log("[PreferencesView] Logging in...")
                                                    service.login()
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // ═══════════════════════════════════════════════════════════
                    // SECTION: Cache & Stockage
                    // ═══════════════════════════════════════════════════════════
                    PreferencesSection {
                        Layout.fillWidth: true
                        title: qsTr("Cache & Stockage")
                        icon: "\uD83D\uDCBE"

                        ColumnLayout {
                            width: parent.width
                            spacing: BlueTheme.spacingMedium

                            // Cache stats
                            Text {
                                text: {
                                    var cm = getCacheManager()
                                    if (cm) {
                                        return qsTr("%1 replays - %2 utilises")
                                            .arg(cm.vodCount)
                                            .arg(cm.formattedTotalSize())
                                    }
                                    return qsTr("Cache non disponible")
                                }
                                font.family: BlueTheme.fontFamily
                                font.pixelSize: 14
                                color: BlueTheme.secondaryText
                            }

                            // Progress bar with percentage
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: BlueTheme.spacingSmall

                                // Progress bar
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 8
                                    radius: 4
                                    color: Qt.rgba(1, 1, 1, 0.1)

                                    Rectangle {
                                        id: cacheProgressBar
                                        property real percentage: {
                                            // Depend on cacheRefreshTrigger to force refresh
                                            var trigger = preferencesRoot.cacheRefreshTrigger
                                            var cm = getCacheManager()
                                            return cm ? Math.min(100, cm.cacheUsagePercent()) : 0
                                        }

                                        width: parent.width * (percentage / 100)
                                        height: parent.height
                                        radius: 4
                                        color: percentage > 90 ? BlueTheme.statusNegative :
                                               percentage > 70 ? BlueTheme.statusWarning : BlueTheme.accent

                                        Behavior on width {
                                            NumberAnimation { duration: BlueTheme.animCardDuration; easing.type: Easing.OutCubic }
                                        }
                                        Behavior on color {
                                            ColorAnimation { duration: BlueTheme.animCardDuration; easing.type: Easing.OutCubic }
                                        }
                                    }
                                }

                                // Percentage text
                                Text {
                                    text: {
                                        // Depend on cacheRefreshTrigger to force refresh
                                        var trigger = preferencesRoot.cacheRefreshTrigger
                                        var cm = getCacheManager()
                                        return cm ? Math.round(cm.cacheUsagePercent()) + "%" : "0%"
                                    }
                                    font.family: BlueTheme.fontFamily
                                    font.pixelSize: 12
                                    font.weight: Font.Medium
                                    color: BlueTheme.secondaryText
                                    Layout.preferredWidth: 40
                                    horizontalAlignment: Text.AlignRight
                                }
                            }

                            // Max cache size control
                            RowLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: BlueTheme.spacingSmall
                                spacing: BlueTheme.spacingSmall

                                Text {
                                    text: qsTr("Taille maximale")
                                    font.family: BlueTheme.fontFamily
                                    font.pixelSize: 14
                                    color: BlueTheme.primaryText
                                }

                                Item { Layout.fillWidth: true }

                                // Stepper control
                                Rectangle {
                                    Layout.preferredWidth: 130
                                    Layout.preferredHeight: 36
                                    radius: 10
                                    color: Qt.rgba(1, 1, 1, 0.05)
                                    border.color: BlueTheme.divider
                                    border.width: 1

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 3
                                        spacing: 0

                                        // Minus button
                                        Rectangle {
                                            Layout.preferredWidth: 32
                                            Layout.fillHeight: true
                                            radius: 7
                                            color: minusArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"

                                            Text {
                                                anchors.centerIn: parent
                                                text: "\u2212"
                                                font.pixelSize: 16
                                                font.weight: Font.Medium
                                                color: BlueTheme.primaryText
                                            }

                                            MouseArea {
                                                id: minusArea
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    var cm = getCacheManager()
                                                    if (cm) {
                                                        var currentGB = cm.maxCacheSize / (1024 * 1024 * 1024)
                                                        var newGB = Math.max(1, currentGB - 10)
                                                        cm.setMaxCacheSize(newGB * 1024 * 1024 * 1024)
                                                    }
                                                }
                                            }
                                        }

                                        // Value input
                                        Item {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true

                                            TextInput {
                                                id: cacheSizeInput
                                                anchors.centerIn: parent
                                                width: parent.width
                                                horizontalAlignment: Text.AlignHCenter
                                                font.family: BlueTheme.fontFamily
                                                font.pixelSize: 14
                                                font.weight: Font.DemiBold
                                                color: BlueTheme.primaryText
                                                text: {
                                                    var cm = getCacheManager()
                                                    return cm ? Math.round(cm.maxCacheSize / (1024 * 1024 * 1024)).toString() : "10"
                                                }
                                                validator: IntValidator { bottom: 1; top: 9999 }
                                                selectByMouse: true
                                                onEditingFinished: {
                                                    var cm = getCacheManager()
                                                    if (cm) {
                                                        var newGB = parseInt(text) || 10
                                                        newGB = Math.max(1, newGB)
                                                        cm.setMaxCacheSize(newGB * 1024 * 1024 * 1024)
                                                    }
                                                }
                                            }
                                        }

                                        // Plus button
                                        Rectangle {
                                            Layout.preferredWidth: 32
                                            Layout.fillHeight: true
                                            radius: 7
                                            color: plusArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"

                                            Text {
                                                anchors.centerIn: parent
                                                text: "+"
                                                font.pixelSize: 16
                                                font.weight: Font.Medium
                                                color: BlueTheme.primaryText
                                            }

                                            MouseArea {
                                                id: plusArea
                                                anchors.fill: parent
                                                hoverEnabled: true
                                                cursorShape: Qt.PointingHandCursor
                                                onClicked: {
                                                    var cm = getCacheManager()
                                                    if (cm) {
                                                        var currentGB = cm.maxCacheSize / (1024 * 1024 * 1024)
                                                        var newGB = currentGB + 10
                                                        cm.setMaxCacheSize(newGB * 1024 * 1024 * 1024)
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                Text {
                                    text: "GB"
                                    font.family: BlueTheme.fontFamily
                                    font.pixelSize: 14
                                    font.weight: Font.Medium
                                    color: BlueTheme.secondaryText
                                }
                            }

                            // Clear cache button
                            RowLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: BlueTheme.spacingSmall
                                visible: {
                                    var cm = getCacheManager()
                                    return cm && cm.vodCount > 0
                                }

                                Item { Layout.fillWidth: true }

                                Rectangle {
                                    id: clearCacheButton
                                    Layout.preferredWidth: clearCacheText.width + 32
                                    Layout.preferredHeight: 36
                                    radius: 18
                                    color: clearCacheArea.containsMouse ? 
                                           Qt.rgba(BlueTheme.statusNegative.r, BlueTheme.statusNegative.g, BlueTheme.statusNegative.b, 0.15) : 
                                           "transparent"
                                    border.color: BlueTheme.statusNegative
                                    border.width: 1

                                    Behavior on color {
                                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                                    }

                                    Text {
                                        id: clearCacheText
                                        anchors.centerIn: parent
                                        text: qsTr("Vider le cache")
                                        font.family: BlueTheme.fontFamily
                                        font.pixelSize: 13
                                        font.weight: Font.Medium
                                        color: BlueTheme.statusNegative
                                    }

                                    MouseArea {
                                        id: clearCacheArea
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: clearCacheDialog.open()
                                    }
                                }
                            }
                        }
                    }

                    // Bottom spacer
                    Item { Layout.preferredHeight: BlueTheme.spacingLarge * 2 }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Clear Cache Dialog
    // ═══════════════════════════════════════════════════════════════════════════
    Popup {
        id: clearCacheDialog
        modal: true
        anchors.centerIn: parent
        width: 340
        padding: BlueTheme.spacingLarge

        background: Rectangle {
            radius: 20
            color: Qt.rgba(BlueTheme.surface.r, BlueTheme.surface.g, BlueTheme.surface.b, 0.98)
            border.color: BlueTheme.divider
            border.width: 1

            // Subtle shadow effect
            layer.enabled: true
            layer.effect: null
        }

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: BlueTheme.animOverlayDuration; easing.type: Easing.OutCubic }
            NumberAnimation { property: "scale"; from: 0.95; to: 1; duration: BlueTheme.animOverlayDuration; easing.type: Easing.OutCubic }
        }

        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0; duration: BlueTheme.animHoverDuration; easing.type: Easing.InCubic }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: BlueTheme.spacingMedium

            // Warning icon
            Text {
                text: "\u26A0\uFE0F"
                font.pixelSize: 32
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: qsTr("Vider tout le cache ?")
                font.family: BlueTheme.fontFamily
                font.pixelSize: 18
                font.weight: Font.DemiBold
                color: BlueTheme.primaryText
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: {
                    var cm = getCacheManager()
                    if (cm) {
                        return qsTr("%1 replays seront supprimes.\nEspace libere: %2")
                            .arg(cm.vodCount)
                            .arg(cm.formattedTotalSize())
                    }
                    return ""
                }
                font.family: BlueTheme.fontFamily
                font.pixelSize: 14
                color: BlueTheme.secondaryText
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            Item { Layout.preferredHeight: BlueTheme.spacingSmall }

            RowLayout {
                Layout.fillWidth: true
                spacing: BlueTheme.spacingMedium

                // Cancel button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    radius: 22
                    color: cancelArea.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                    border.color: BlueTheme.divider
                    border.width: 1

                    Behavior on color {
                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Annuler")
                        font.family: BlueTheme.fontFamily
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        color: BlueTheme.primaryText
                    }

                    MouseArea {
                        id: cancelArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: clearCacheDialog.close()
                    }
                }

                // Confirm button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    radius: 22
                    color: confirmArea.containsMouse ? 
                           Qt.darker(BlueTheme.statusNegative, 1.1) : BlueTheme.statusNegative

                    Behavior on color {
                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Vider")
                        font.family: BlueTheme.fontFamily
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        color: "#FFFFFF"
                    }

                    MouseArea {
                        id: confirmArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var cm = getCacheManager()
                            if (cm) {
                                cm.clearAllVods()
                            }
                            clearCacheDialog.close()
                        }
                    }
                }
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Auto-close on logout
    // ═══════════════════════════════════════════════════════════════════════════
    Connections {
        target: getTwitchService()
        enabled: getTwitchService() !== null
        function onAuthenticatedChanged(authenticated) {
            if (!authenticated) {
                console.log("[PreferencesView] User logged out, closing preferences")
                preferencesRoot.closeRequested()
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Cache refresh on maxCacheSize change
    // ═══════════════════════════════════════════════════════════════════════════
    Connections {
        target: getCacheManager()
        enabled: getCacheManager() !== null
        function onMaxCacheSizeChanged() {
            preferencesRoot.cacheRefreshTrigger++
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // PreferencesSection Component (inline)
    // ═══════════════════════════════════════════════════════════════════════════
    component PreferencesSection: Rectangle {
        id: sectionRoot

        property string title: ""
        property string icon: ""
        default property alias content: sectionContent.children

        implicitHeight: sectionColumn.height + 2 * BlueTheme.spacingLarge
        radius: 16
        color: Qt.rgba(BlueTheme.surface.r, BlueTheme.surface.g, BlueTheme.surface.b, 0.6)
        border.color: BlueTheme.divider
        border.width: 1

        ColumnLayout {
            id: sectionColumn
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: BlueTheme.spacingLarge
            spacing: BlueTheme.spacingMedium

            // Section header
            RowLayout {
                Layout.fillWidth: true
                spacing: BlueTheme.spacingSmall

                Text {
                    text: sectionRoot.icon
                    font.pixelSize: 18
                    visible: sectionRoot.icon !== ""
                }

                Text {
                    text: sectionRoot.title
                    font.family: BlueTheme.fontFamily
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    color: BlueTheme.primaryText
                }

                Item { Layout.fillWidth: true }
            }

            // Divider
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: BlueTheme.divider
            }

            // Content
            Column {
                id: sectionContent
                Layout.fillWidth: true
                spacing: BlueTheme.spacingMedium
            }
        }
    }
}
