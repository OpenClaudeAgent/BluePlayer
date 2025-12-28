import QtQuick 2.15

/**
 * ThemeProvider - Bridge to C++ ThemeManager
 * 
 * This component provides easy access to the reactive theme colors
 * exposed by the C++ ThemeManager. All color properties update
 * automatically when the theme changes.
 * 
 * Usage in main.qml:
 *   ThemeProvider { id: theme }
 *   color: theme.primaryText
 * 
 * The ThemeManager is exposed via context property from main.mm
 */
Item {
    id: root
    visible: false
    width: 0
    height: 0
    
    // Direct access to C++ ThemeManager
    readonly property var manager: typeof themeManager !== "undefined" ? themeManager : null
    
    // Theme state
    readonly property bool isDark: manager ? manager.isDark : true
    readonly property string currentTheme: manager ? manager.currentTheme : "dark"
    readonly property string preference: manager ? manager.themePreference : "auto"
    
    // ========================================================================
    // REACTIVE COLORS - Direct bindings to C++ properties
    // ========================================================================
    
    // Window & Backgrounds
    readonly property color windowBackground: manager ? manager.windowBackground : "#03050b"
    readonly property color gradientStart: manager ? manager.gradientStart : "#040b15"
    readonly property color gradientEnd: manager ? manager.gradientEnd : "#0b1727"
    readonly property color overlayTint: manager ? manager.overlayTint : "#0c111b"
    
    // Surfaces
    readonly property color surface: manager ? manager.surface : "#141c2a"
    readonly property color surfaceSoft: manager ? manager.surfaceSoft : "#1d2533"
    readonly property color cardHighlight: manager ? manager.cardHighlight : "#171f2f"
    readonly property color buttonSurface: manager ? manager.buttonSurface : "#1c2232"
    readonly property color buttonBorder: manager ? manager.buttonBorder : "#2b3450"
    
    // Text
    readonly property color primaryText: manager ? manager.primaryText : "#f6f7fa"
    readonly property color secondaryText: manager ? manager.secondaryText : "#aeb9c9"
    readonly property color mutedText: manager ? manager.mutedText : "#7d89a4"
    
    // Accent
    readonly property color accent: manager ? manager.accent : "#5bc0ff"
    readonly property color accentSubtle: manager ? manager.accentSubtle : "#3da2ff"
    
    // Borders & Dividers
    readonly property color divider: manager ? manager.divider : "#222b37"
    
    // Status
    readonly property color statusPositive: manager ? manager.statusPositive : "#4ef57a"
    readonly property color statusWarning: manager ? manager.statusWarning : "#f7c114"
    readonly property color statusNegative: manager ? manager.statusNegative : "#f46969"
    
    // ========================================================================
    // HELPER FUNCTIONS
    // ========================================================================
    
    function setPreference(pref) {
        if (manager) {
            manager.themePreference = pref
        }
    }
    
    // ========================================================================
    // INITIALIZATION
    // ========================================================================
    
    Component.onCompleted: {
        if (manager) {
            console.log("[ThemeProvider] Connected to ThemeManager - theme:", currentTheme)
        } else {
            console.warn("[ThemeProvider] ThemeManager not available, using defaults")
        }
    }
}
