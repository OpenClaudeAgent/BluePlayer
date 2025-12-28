.pragma library

// ============================================================================
// THEME SYSTEM - Light & Dark Mode Support
// ============================================================================

// Current theme mode - will be bound to ThemeManager from C++
// Values: "dark", "light"
var currentTheme = "dark"

// ============================================================================
// THEME DEFINITIONS
// ============================================================================

var themes = {
    dark: {
        // Window & Background
        windowBackground: "#03050b",
        gradientStart: "#040b15",
        gradientEnd: "#0b1727",
        overlayTint: "#0c111b",
        
        // Surfaces
        surface: "#141c2a",
        surfaceSoft: "#1d2533",
        cardHighlight: "#171f2f",
        buttonSurface: "#1c2232",
        buttonBorder: "#2b3450",
        
        // Text
        primaryText: "#f6f7fa",
        secondaryText: "#aeb9c9",
        mutedText: "#7d89a4",
        
        // Accent (same for both themes - brand color)
        accent: "#5bc0ff",
        accentSubtle: "#3da2ff",
        
        // Borders & Dividers
        divider: "#222b37",
        
        // Status colors
        statusPositive: "#4ef57a",
        statusWarning: "#f7c114",
        statusNegative: "#f46969"
    },
    
    light: {
        // Window & Background
        windowBackground: "#F2F2F7",
        gradientStart: "#FFFFFF",
        gradientEnd: "#F2F2F7",
        overlayTint: "#E5E5EA",
        
        // Surfaces
        surface: "#FFFFFF",
        surfaceSoft: "#F9F9FB",
        cardHighlight: "#F5F5F7",
        buttonSurface: "#EBEBF0",
        buttonBorder: "#D1D1D6",
        
        // Text
        primaryText: "#1C1C1E",
        secondaryText: "#6C6C70",
        mutedText: "#8E8E93",
        
        // Accent (same for both themes - brand color)
        accent: "#007AFF",
        accentSubtle: "#0A84FF",
        
        // Borders & Dividers
        divider: "#E5E5EA",
        
        // Status colors
        statusPositive: "#34C759",
        statusWarning: "#FF9500",
        statusNegative: "#FF3B30"
    }
}

// ============================================================================
// DYNAMIC THEME ACCESSORS
// ============================================================================

// These functions return the current theme value
// QML components should call these or use the direct properties below

function getThemeValue(key) {
    return themes[currentTheme][key]
}

function setTheme(themeName) {
    if (themeName === "dark" || themeName === "light") {
        currentTheme = themeName
        return true
    }
    return false
}

function isDarkTheme() {
    return currentTheme === "dark"
}

// ============================================================================
// EXPORTED THEME PROPERTIES (for backward compatibility)
// These are updated when setTheme() is called
// ============================================================================

// Window & Background
var windowBackground = themes.dark.windowBackground
var gradientStart = themes.dark.gradientStart
var gradientEnd = themes.dark.gradientEnd
var overlayTint = themes.dark.overlayTint

// Surfaces
var surface = themes.dark.surface
var surfaceSoft = themes.dark.surfaceSoft
var cardHighlight = themes.dark.cardHighlight
var buttonSurface = themes.dark.buttonSurface
var buttonBorder = themes.dark.buttonBorder

// Text
var primaryText = themes.dark.primaryText
var secondaryText = themes.dark.secondaryText
var mutedText = themes.dark.mutedText

// Accent
var accent = themes.dark.accent
var accentSubtle = themes.dark.accentSubtle

// Borders & Dividers
var divider = themes.dark.divider

// Status colors
var statusPositive = themes.dark.statusPositive
var statusWarning = themes.dark.statusWarning
var statusNegative = themes.dark.statusNegative

// ============================================================================
// STATIC PROPERTIES (same for all themes)
// ============================================================================

var cornerRadius = 22
var borderWidth = 1
var elevation = 28

function selectFontFamily() {
  var preferredFonts = [
    "SF Pro Display",
    "Helvetica Neue",
    "Inter",
    "Arial",
    "Segoe UI"
  ];
  var families = Qt.fontFamilies();
  for (var idx = 0; idx < preferredFonts.length; ++idx) {
    if (families.indexOf(preferredFonts[idx]) !== -1) {
      return preferredFonts[idx];
    }
  }
  return families.length > 0 ? families[0] : "Sans Serif";
}
var fontFamily = selectFontFamily()
var spacing = 14
var spacingSmall = 8
var spacingMedium = 16
var spacingLarge = 24
var responsiveBreakpoint = 1024
var cardElevation = 10
var heroHeight = 260
var heroCornerRadius = 30
var headlineFont = fontFamily

// ============================================================================
// ANIMATION SYSTEM - Apple-Inspired Motion Design
// ============================================================================

// Duration tokens (milliseconds)
var animDurationInstant = 80        // Immediate feedback (press state)
var animDurationFast = 150          // Micro-interactions (hover, focus)
var animDurationStandard = 250      // Standard transitions (cards, overlays)
var animDurationEmphasis = 350      // Important transitions (navigation, panels)
var animDurationDramatic = 500      // Dramatic changes (mode switches)

// Animation patterns - Pre-defined durations for common UI patterns
var animPressDuration = 80          // Button press, tap feedback
var animHoverDuration = 150         // Hover on buttons, interactive elements
var animFocusDuration = 150         // Keyboard navigation focus
var animCardDuration = 200          // Card hover (scale, shadow, color)
var animContentFadeDuration = 200   // Lazy loading, skeleton to content
var animOverlayDuration = 250       // Dropdowns, tooltips, popovers
var animPanelDuration = 300         // Side panels, drawers, chat
var animControlBarDuration = 300    // Show/hide player controls
var animNavigationDuration = 350    // Page/view transitions
var animPulseDuration = 600         // Loading pulse indicators

// Toast/Notification durations
var animToastEnterDuration = 200
var animToastDisplayDuration = 1400
var animToastExitDuration = 250

// Scale transforms for depth perception
var scaleHover = 1.02               // Slight lift on hover
var scalePress = 0.98               // Slight press down
var scaleSelected = 1.05            // Emphasis on selection

// Easing reference (use in QML as easing.type: Easing.XXX)
// - Easing.OutCubic    → Most animations (responsive feel)
// - Easing.InOutCubic  → Symmetrical movements (navigation, control bar)
// - Easing.OutQuart    → Snappy responses (press feedback)
// - Easing.OutBack     → Spring/bounce effects (emphasis)
// - Easing.InOutSine   → Smooth pulse animations

// ============================================================================
// QUALITY DETECTION & FORMATTING
// ============================================================================

/**
 * Checks if a quality string represents audio-only mode.
 * 
 * Examples:
 *   "audio_only" → true
 *   "Audio Only" → true
 *   "1080p60" → false
 */
function isAudioQuality(quality) {
    if (!quality) return false
    return quality.toLowerCase().indexOf("audio") >= 0
}

/**
 * Formats a quality label for display.
 * Normalizes various quality formats to consistent display names.
 * 
 * Examples:
 *   "audio_only" → "Audio"
 *   "chunked" → "Source"
 *   "1080p60" → "1080p 60fps"
 *   "720p30" → "720p"
 *   "Source (1080p)" → "1080p"
 */
function formatQuality(quality) {
    if (!quality) return ""
    
    var q = quality.toLowerCase().trim()
    
    // Audio mode
    if (q.indexOf("audio") >= 0) {
        return "Audio"
    }
    
    // Source/chunked → Source
    if (q === "chunked" || q === "source") {
        return "Source"
    }
    
    // Remove "Source" prefix if followed by resolution (e.g., "Source (1080p)" → "1080p")
    var sourceMatch = quality.match(/source\s*\(?(\d+p\d*)\)?/i)
    if (sourceMatch) {
        q = sourceMatch[1].toLowerCase()
    }
    
    // Format resolution with fps (e.g., "1080p60" → "1080p 60fps", "720p30" → "720p")
    var resMatch = q.match(/^(\d+p)(\d+)?$/)
    if (resMatch) {
        var res = resMatch[1]
        var fps = resMatch[2]
        
        // Only show fps if it's 60 (skip 30fps as it's default)
        if (fps && parseInt(fps) >= 60) {
            return res + " " + fps + "fps"
        }
        return res
    }
    
    // Return original if no transformation needed
    return quality
}

// ============================================================================
// THEME UPDATE FUNCTION
// Called from QML when theme changes
// ============================================================================

function applyTheme(themeName) {
    if (!setTheme(themeName)) {
        return false
    }
    
    var t = themes[themeName]
    
    // Update all exported properties
    windowBackground = t.windowBackground
    gradientStart = t.gradientStart
    gradientEnd = t.gradientEnd
    overlayTint = t.overlayTint
    surface = t.surface
    surfaceSoft = t.surfaceSoft
    cardHighlight = t.cardHighlight
    buttonSurface = t.buttonSurface
    buttonBorder = t.buttonBorder
    primaryText = t.primaryText
    secondaryText = t.secondaryText
    mutedText = t.mutedText
    accent = t.accent
    accentSubtle = t.accentSubtle
    divider = t.divider
    statusPositive = t.statusPositive
    statusWarning = t.statusWarning
    statusNegative = t.statusNegative
    
    return true
}
