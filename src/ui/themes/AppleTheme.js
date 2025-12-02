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
