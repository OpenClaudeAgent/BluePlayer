import QtQuick 2.15
import QtQuick.Controls 6.5
import QtQuick.Layouts 1.15

import "themes/BlueTheme.js" as BlueTheme
import "components"

Item {
    id: preferencesRoot

    // Theme access
    readonly property var tm: typeof themeManager !== "undefined" ? themeManager : null

    signal closeRequested()

    // Default quality preference - connected to backend TwitchService
    property string defaultQuality: {
        var service = getTwitchService()
        return service ? service.defaultQuality : "Auto"
    }

    // Quality options
    readonly property var qualityOptions: ["Auto", "1080p60", "1080p", "720p60", "720p", "480p", "360p"]

    // Cache refresh trigger (incremented to force binding refresh)
    property int cacheRefreshTrigger: 0

    // Language preference - connected to backend LanguageManager
    property string currentLanguage: {
        var mgr = getLanguageManager()
        return mgr ? mgr.currentLanguage : "system"
    }

    function getLanguageManager() {
        return typeof languageManager !== "undefined" ? languageManager : null
    }

    function setLanguage(lang) {
        var mgr = getLanguageManager()
        if (mgr) {
            mgr.setLanguage(lang)
            console.info("[Preferences] Language changed:", lang)
        }
    }

    // Theme preference - connected to backend ThemeManager
    property string currentThemePreference: {
        var mgr = getThemeManager()
        return mgr ? mgr.themePreference : "auto"
    }

    function getThemeManager() {
        return typeof themeManager !== "undefined" ? themeManager : null
    }

    function setThemePreference(pref) {
        var mgr = getThemeManager()
        if (mgr) {
            mgr.themePreference = pref
            console.info("[Preferences] Theme changed:", pref)
        }
    }

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
            GradientStop { position: 0.0; color: tm ? tm.gradientStart : BlueTheme.gradientStart }
            GradientStop { position: 1.0; color: tm ? tm.gradientEnd : BlueTheme.gradientEnd }
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
                    // SECTION: Apparence (Theme)
                    // ═══════════════════════════════════════════════════════════
                    PreferencesSection {
                        Layout.fillWidth: true
                        title: qsTr("Appearance")
                        icon: "\uD83C\uDFA8"

                        RowLayout {
                            width: parent.width
                            spacing: BlueTheme.spacingMedium

                            Text {
                                text: qsTr("Theme")
                                font.family: BlueTheme.fontFamily
                                font.pixelSize: 14
                                color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
                                Layout.fillWidth: true
                            }

                            // Theme dropdown using BlueDropdown component
                            BlueDropdown {
                                id: themeDropdown
                                Layout.preferredWidth: 140
                                Layout.preferredHeight: 36
                                model: [qsTr("Automatic"), qsTr("Light"), qsTr("Dark")]
                                selectedValue: {
                                    var pref = preferencesRoot.currentThemePreference
                                    if (pref === "light") return qsTr("Light")
                                    if (pref === "dark") return qsTr("Dark")
                                    return qsTr("Automatic")
                                }
                                placeholder: qsTr("Theme")
                                
                                onValueSelected: function(value) {
                                    var prefCode = "auto"
                                    if (value === qsTr("Light")) prefCode = "light"
                                    else if (value === qsTr("Dark")) prefCode = "dark"
                                    preferencesRoot.setThemePreference(prefCode)
                                }
                            }
                        }
                    }

                    // ═══════════════════════════════════════════════════════════
                    // SECTION: Lecture
                    // ═══════════════════════════════════════════════════════════
                    PreferencesSection {
                        Layout.fillWidth: true
                        title: qsTr("Playback")
                        icon: "\uD83C\uDFAC"

                        RowLayout {
                            width: parent.width
                            spacing: BlueTheme.spacingMedium

                            Text {
                                text: qsTr("Default quality")
                                font.family: BlueTheme.fontFamily
                                font.pixelSize: 14
                                color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
                                Layout.fillWidth: true
                            }

                            // Quality dropdown using BlueDropdown component
                            BlueDropdown {
                                id: qualityDropdown
                                Layout.preferredWidth: 120
                                Layout.preferredHeight: 36
                                model: preferencesRoot.qualityOptions
                                selectedValue: preferencesRoot.defaultQuality
                                placeholder: qsTr("Quality")
                                
                                onValueSelected: function(value) {
                                    // Save to backend (persisted via QSettings)
                                    var service = getTwitchService()
                                    if (service) {
                                        service.setDefaultQuality(value)
                                        console.info("[Preferences] Default quality changed:", value)
                                    }
                                }
                            }
                        }
                    }

                    // ═══════════════════════════════════════════════════════════
                    // SECTION: Language
                    // ═══════════════════════════════════════════════════════════
                    PreferencesSection {
                        Layout.fillWidth: true
                        title: qsTr("Language")
                        icon: "\uD83C\uDF10"

                        ColumnLayout {
                            width: parent.width
                            spacing: BlueTheme.spacingMedium

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: BlueTheme.spacingMedium

                                Text {
                                    text: qsTr("App language")
                                    font.family: BlueTheme.fontFamily
                                    font.pixelSize: 14
                                    color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
                                    Layout.fillWidth: true
                                }

                                BlueDropdown {
                                    id: languageDropdown
                                    Layout.preferredWidth: 140
                                    Layout.preferredHeight: 36
                                    model: [qsTr("System"), "English", "Français"]
                                    selectedValue: {
                                        var lang = preferencesRoot.currentLanguage
                                        if (lang === "en") return "English"
                                        if (lang === "fr") return "Français"
                                        return qsTr("System")
                                    }
                                    placeholder: qsTr("Language")

                                    onValueSelected: function(value) {
                                        var langCode = "system"
                                        if (value === "English") langCode = "en"
                                        else if (value === "Français") langCode = "fr"
                                        preferencesRoot.setLanguage(langCode)
                                    }
                                }
                            }
                        }
                    }

                    // ═══════════════════════════════════════════════════════════
                    // SECTION: Compte Twitch
                    // ═══════════════════════════════════════════════════════════
                    PreferencesSection {
                        Layout.fillWidth: true
                        title: qsTr("Twitch Account")
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
                                               (preferencesRoot.tm ? preferencesRoot.tm.statusPositive : BlueTheme.statusPositive) : 
                                               (preferencesRoot.tm ? preferencesRoot.tm.statusNegative : BlueTheme.statusNegative)
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
                                                qsTr("Connected as @%1").arg(userName) : 
                                                qsTr("Connected to Twitch")
                                        }
                                        return qsTr("Not connected")
                                    }
                                    font.family: BlueTheme.fontFamily
                                    font.pixelSize: 14
                                    color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
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
                                    color: {
                                        var accentColor = preferencesRoot.tm ? preferencesRoot.tm.accent : BlueTheme.accent
                                        var negativeColor = preferencesRoot.tm ? preferencesRoot.tm.statusNegative : BlueTheme.statusNegative
                                        if (twitchActionArea.containsMouse) {
                                            return isLoggedIn ? Qt.rgba(negativeColor.r, negativeColor.g, negativeColor.b, 0.2) : 
                                                               Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.2)
                                        }
                                        return isLoggedIn ? "transparent" : accentColor
                                    }
                                    border.color: isLoggedIn ? (preferencesRoot.tm ? preferencesRoot.tm.statusNegative : BlueTheme.statusNegative) : (preferencesRoot.tm ? preferencesRoot.tm.accent : BlueTheme.accent)
                                    border.width: isLoggedIn ? 1 : 0

                                    Behavior on color {
                                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                                    }

                                    Text {
                                        id: twitchActionText
                                        anchors.centerIn: parent
                                        text: twitchActionButton.isLoggedIn ? qsTr("Sign out") : qsTr("Sign in")
                                        font.family: BlueTheme.fontFamily
                                        font.pixelSize: 13
                                        font.weight: Font.Medium
                                        color: twitchActionButton.isLoggedIn ? (preferencesRoot.tm ? preferencesRoot.tm.statusNegative : BlueTheme.statusNegative) : "#FFFFFF"
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
                                                    console.info("[Preferences] Sign out requested")
                                                    service.logout()
                                                } else {
                                                    console.info("[Preferences] Sign in requested")
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
                        title: qsTr("Cache & Storage")
                        icon: "\uD83D\uDCBE"

                        ColumnLayout {
                            width: parent.width
                            spacing: BlueTheme.spacingMedium

                            // Cache stats
                            Text {
                                text: {
                                    var cm = getCacheManager()
                                    if (cm) {
                                        return qsTr("%1 replays - %2 used")
                                            .arg(cm.vodCount)
                                            .arg(cm.formattedTotalSize())
                                    }
                                    return qsTr("Cache unavailable")
                                }
                                font.family: BlueTheme.fontFamily
                                font.pixelSize: 14
                                color: preferencesRoot.tm ? preferencesRoot.tm.secondaryText : BlueTheme.secondaryText
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
                                    color: preferencesRoot.tm ? Qt.rgba(preferencesRoot.tm.divider.r, preferencesRoot.tm.divider.g, preferencesRoot.tm.divider.b, 0.5) : Qt.rgba(1, 1, 1, 0.1)

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
                                        color: percentage > 90 ? (preferencesRoot.tm ? preferencesRoot.tm.statusNegative : BlueTheme.statusNegative) :
                                               percentage > 70 ? (preferencesRoot.tm ? preferencesRoot.tm.statusWarning : BlueTheme.statusWarning) : (preferencesRoot.tm ? preferencesRoot.tm.accent : BlueTheme.accent)

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
                                    color: preferencesRoot.tm ? preferencesRoot.tm.secondaryText : BlueTheme.secondaryText
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
                                    text: qsTr("Maximum size")
                                    font.family: BlueTheme.fontFamily
                                    font.pixelSize: 14
                                    color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
                                }

                                Item { Layout.fillWidth: true }

                                // Stepper control
                                Rectangle {
                                    Layout.preferredWidth: 130
                                    Layout.preferredHeight: 36
                                    radius: 10
                                    color: {
                                        var surfaceColor = preferencesRoot.tm ? preferencesRoot.tm.surfaceSoft : BlueTheme.surfaceSoft
                                        return Qt.rgba(surfaceColor.r, surfaceColor.g, surfaceColor.b, 0.5)
                                    }
                                    border.color: preferencesRoot.tm ? preferencesRoot.tm.divider : BlueTheme.divider
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
                                            color: minusArea.containsMouse ? (preferencesRoot.tm ? preferencesRoot.tm.cardHighlight : "#1a2230") : "transparent"

                                            Text {
                                                anchors.centerIn: parent
                                                text: "\u2212"
                                                font.pixelSize: 16
                                                font.weight: Font.Medium
                                                color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
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
                                                        console.info("[Preferences] Max cache size changed:", newGB, "GB")
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
                                                color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
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
                                            color: plusArea.containsMouse ? (preferencesRoot.tm ? preferencesRoot.tm.cardHighlight : "#1a2230") : "transparent"

                                            Text {
                                                anchors.centerIn: parent
                                                text: "+"
                                                font.pixelSize: 16
                                                font.weight: Font.Medium
                                                color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
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
                                                        console.info("[Preferences] Max cache size changed:", newGB, "GB")
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
                                    color: preferencesRoot.tm ? preferencesRoot.tm.secondaryText : BlueTheme.secondaryText
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
                                    color: {
                                        var negativeColor = preferencesRoot.tm ? preferencesRoot.tm.statusNegative : BlueTheme.statusNegative
                                        return clearCacheArea.containsMouse ? Qt.rgba(negativeColor.r, negativeColor.g, negativeColor.b, 0.15) : "transparent"
                                    }
                                    border.color: preferencesRoot.tm ? preferencesRoot.tm.statusNegative : BlueTheme.statusNegative
                                    border.width: 1

                                    Behavior on color {
                                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                                    }

                                    Text {
                                        id: clearCacheText
                                        anchors.centerIn: parent
                                        text: qsTr("Clear cache")
                                        font.family: BlueTheme.fontFamily
                                        font.pixelSize: 13
                                        font.weight: Font.Medium
                                        color: preferencesRoot.tm ? preferencesRoot.tm.statusNegative : BlueTheme.statusNegative
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
            color: {
                var surfaceColor = preferencesRoot.tm ? preferencesRoot.tm.surface : BlueTheme.surface
                return Qt.rgba(surfaceColor.r, surfaceColor.g, surfaceColor.b, 0.98)
            }
            border.color: preferencesRoot.tm ? preferencesRoot.tm.divider : BlueTheme.divider
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
                text: qsTr("Clear all cache?")
                font.family: BlueTheme.fontFamily
                font.pixelSize: 18
                font.weight: Font.DemiBold
                color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: {
                    var cm = getCacheManager()
                    if (cm) {
                        return qsTr("%1 replays will be deleted.\nSpace freed: %2")
                            .arg(cm.vodCount)
                            .arg(cm.formattedTotalSize())
                    }
                    return ""
                }
                font.family: BlueTheme.fontFamily
                font.pixelSize: 14
                color: preferencesRoot.tm ? preferencesRoot.tm.secondaryText : BlueTheme.secondaryText
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
                    color: cancelArea.containsMouse ? (preferencesRoot.tm ? preferencesRoot.tm.cardHighlight : Qt.rgba(1, 1, 1, 0.1)) : "transparent"
                    border.color: preferencesRoot.tm ? preferencesRoot.tm.divider : BlueTheme.divider
                    border.width: 1

                    Behavior on color {
                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Cancel")
                        font.family: BlueTheme.fontFamily
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
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
                    color: {
                        var negativeColor = preferencesRoot.tm ? preferencesRoot.tm.statusNegative : BlueTheme.statusNegative
                        return confirmArea.containsMouse ? Qt.darker(negativeColor, 1.1) : negativeColor
                    }

                    Behavior on color {
                        ColorAnimation { duration: BlueTheme.animHoverDuration; easing.type: Easing.OutCubic }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Clear")
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
                                                console.info("[Preferences] Cache cleared, freed:", cm.formattedTotalSize())
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
    // Default quality sync with backend
    // ═══════════════════════════════════════════════════════════════════════════
    Connections {
        target: getTwitchService()
        enabled: getTwitchService() !== null
        function onDefaultQualityChanged() {
            // Re-evaluate the binding to update UI
            var service = getTwitchService()
            if (service) {
                qualityDropdown.selectedValue = service.defaultQuality
            }
        }
    }

    // ═══════════════════════════════════════════════════════════════════════════
    // Theme preference sync with backend
    // ═══════════════════════════════════════════════════════════════════════════
    Connections {
        target: getThemeManager()
        enabled: getThemeManager() !== null
        function onThemePreferenceChanged(preference) {
            // Force re-evaluation of currentThemePreference binding
            preferencesRoot.currentThemePreference = preference
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
        color: {
            var surfaceColor = preferencesRoot.tm ? preferencesRoot.tm.surface : BlueTheme.surface
            return Qt.rgba(surfaceColor.r, surfaceColor.g, surfaceColor.b, 0.6)
        }
        border.color: preferencesRoot.tm ? preferencesRoot.tm.divider : BlueTheme.divider
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
                    color: preferencesRoot.tm ? preferencesRoot.tm.primaryText : BlueTheme.primaryText
                }

                Item { Layout.fillWidth: true }
            }

            // Divider
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: preferencesRoot.tm ? preferencesRoot.tm.divider : BlueTheme.divider
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
