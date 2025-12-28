.pragma library

var windowBackground = "#03050b"
var gradientStart = "#040b15"
var gradientEnd = "#0b1727"
var overlayTint = "#0c111b"
var surface = "#141c2a"
var surfaceSoft = "#1d2533"
var accent = "#5bc0ff"
var accentSubtle = "#3da2ff"
var primaryText = "#f6f7fa"
var secondaryText = "#aeb9c9"
var mutedText = "#7d89a4"
var divider = "#222b37"
var statusPositive = "#4ef57a"
var statusWarning = "#f7c114"
var statusNegative = "#f46969"
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
var cardHighlight = "#171f2f"
var buttonSurface = "#1c2232"
var buttonBorder = "#2b3450"
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
