#include "ThemeManager.hpp"

#include <QDebug>

#if defined(Q_OS_MAC)
#import <AppKit/AppKit.h>
#endif

namespace blueplayer::core {

// ============================================================================
// COLOR PALETTES - Carefully designed for each theme
// ============================================================================

namespace {

// Dark theme - Current BluePlayer theme (refined)
constexpr struct {
  const char* windowBackground = "#03050b";
  const char* gradientStart = "#040b15";
  const char* gradientEnd = "#0b1727";
  const char* overlayTint = "#0c111b";
  const char* surface = "#141c2a";
  const char* surfaceSoft = "#1d2533";
  const char* cardHighlight = "#171f2f";
  const char* buttonSurface = "#1c2232";
  const char* buttonBorder = "#2b3450";
  const char* primaryText = "#f6f7fa";
  const char* secondaryText = "#aeb9c9";
  const char* mutedText = "#7d89a4";
  const char* accent = "#5bc0ff";
  const char* accentSubtle = "#3da2ff";
  const char* divider = "#222b37";
  const char* statusPositive = "#4ef57a";
  const char* statusWarning = "#f7c114";
  const char* statusNegative = "#f46969";
} kDarkPalette;

// Light theme - Carefully designed for readability and aesthetics
constexpr struct {
  const char* windowBackground = "#F5F5F7";      // Apple-style warm gray
  const char* gradientStart = "#FFFFFF";         // Pure white top
  const char* gradientEnd = "#F0F0F5";           // Subtle purple-gray bottom
  const char* overlayTint = "#E8E8ED";           // Soft overlay
  const char* surface = "#FFFFFF";               // White cards
  const char* surfaceSoft = "#FAFAFA";           // Slightly off-white
  const char* cardHighlight = "#F0F2F5";         // Card hover state
  const char* buttonSurface = "#E8E8ED";         // Button background
  const char* buttonBorder = "#D1D1D6";          // Subtle borders
  const char* primaryText = "#1D1D1F";           // Apple dark text
  const char* secondaryText = "#636366";         // Medium gray text
  const char* mutedText = "#8E8E93";             // Light gray text
  const char* accent = "#0A84FF";                // iOS blue (brighter for light)
  const char* accentSubtle = "#007AFF";          // Standard iOS blue
  const char* divider = "#D1D1D6";               // Visible but subtle
  const char* statusPositive = "#30D158";        // iOS green
  const char* statusWarning = "#FF9F0A";         // iOS orange
  const char* statusNegative = "#FF453A";        // iOS red
} kLightPalette;

}  // namespace

// ============================================================================
// CONSTRUCTOR / DESTRUCTOR
// ============================================================================

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent),
      m_themePreference("auto"),
      m_currentTheme("dark"),
      m_systemIsDark(true),
      m_settings("BluePlayer", "BluePlayer") {
  // Load saved preference
  m_themePreference =
      m_settings.value("appearance/theme", "auto").toString();

  // Detect initial system theme
  m_systemIsDark = (detectSystemTheme() == "dark");

  // Calculate initial effective theme
  updateCurrentTheme();
  
  // Initialize colors
  updateColors();

  // Start listening for system changes
  startListeningForSystemChanges();
  
  qDebug() << "[ThemeManager] Initialized - preference:" << m_themePreference
           << "current:" << m_currentTheme;
}

ThemeManager::~ThemeManager() { 
  stopListeningForSystemChanges(); 
}

// ============================================================================
// THEME CONTROL
// ============================================================================

QString ThemeManager::themePreference() const { 
  return m_themePreference; 
}

void ThemeManager::setThemePreference(const QString& preference) {
  if (preference != "auto" && preference != "light" && preference != "dark") {
    qWarning() << "[ThemeManager] Invalid theme preference:" << preference;
    return;
  }

  if (m_themePreference != preference) {
    m_themePreference = preference;
    m_settings.setValue("appearance/theme", preference);
    m_settings.sync();

    emit themePreferenceChanged(preference);
    updateCurrentTheme();

    qDebug() << "[ThemeManager] Theme preference changed to:" << preference;
  }
}

QString ThemeManager::currentTheme() const { 
  return m_currentTheme; 
}

bool ThemeManager::isDark() const { 
  return m_currentTheme == "dark"; 
}

// ============================================================================
// COLOR ACCESSORS
// ============================================================================

QColor ThemeManager::windowBackground() const { return m_colors.windowBackground; }
QColor ThemeManager::gradientStart() const { return m_colors.gradientStart; }
QColor ThemeManager::gradientEnd() const { return m_colors.gradientEnd; }
QColor ThemeManager::overlayTint() const { return m_colors.overlayTint; }
QColor ThemeManager::surface() const { return m_colors.surface; }
QColor ThemeManager::surfaceSoft() const { return m_colors.surfaceSoft; }
QColor ThemeManager::cardHighlight() const { return m_colors.cardHighlight; }
QColor ThemeManager::buttonSurface() const { return m_colors.buttonSurface; }
QColor ThemeManager::buttonBorder() const { return m_colors.buttonBorder; }
QColor ThemeManager::primaryText() const { return m_colors.primaryText; }
QColor ThemeManager::secondaryText() const { return m_colors.secondaryText; }
QColor ThemeManager::mutedText() const { return m_colors.mutedText; }
QColor ThemeManager::accent() const { return m_colors.accent; }
QColor ThemeManager::accentSubtle() const { return m_colors.accentSubtle; }
QColor ThemeManager::divider() const { return m_colors.divider; }
QColor ThemeManager::statusPositive() const { return m_colors.statusPositive; }
QColor ThemeManager::statusWarning() const { return m_colors.statusWarning; }
QColor ThemeManager::statusNegative() const { return m_colors.statusNegative; }

// ============================================================================
// SYSTEM THEME DETECTION
// ============================================================================

QString ThemeManager::detectSystemTheme() const {
#if defined(Q_OS_MAC)
  if (@available(macOS 10.14, *)) {
    NSAppearance* appearance = [NSApp effectiveAppearance];
    NSAppearanceName name = [appearance
        bestMatchFromAppearancesWithNames:@[
          NSAppearanceNameAqua, NSAppearanceNameDarkAqua
        ]];
    if ([name isEqualToString:NSAppearanceNameDarkAqua]) {
      return "dark";
    }
    return "light";
  }
#endif
  // Default to dark on older systems or non-macOS
  return "dark";
}

// ============================================================================
// INTERNAL UPDATES
// ============================================================================

void ThemeManager::updateCurrentTheme() {
  QString newTheme;

  if (m_themePreference == "auto") {
    newTheme = m_systemIsDark ? "dark" : "light";
  } else {
    newTheme = m_themePreference;
  }

  if (m_currentTheme != newTheme) {
    m_currentTheme = newTheme;
    updateColors();
    emit currentThemeChanged(newTheme);
    qDebug() << "[ThemeManager] Current theme changed to:" << newTheme;
  }
}

void ThemeManager::updateColors() {
  if (m_currentTheme == "dark") {
    m_colors.windowBackground = QColor(kDarkPalette.windowBackground);
    m_colors.gradientStart = QColor(kDarkPalette.gradientStart);
    m_colors.gradientEnd = QColor(kDarkPalette.gradientEnd);
    m_colors.overlayTint = QColor(kDarkPalette.overlayTint);
    m_colors.surface = QColor(kDarkPalette.surface);
    m_colors.surfaceSoft = QColor(kDarkPalette.surfaceSoft);
    m_colors.cardHighlight = QColor(kDarkPalette.cardHighlight);
    m_colors.buttonSurface = QColor(kDarkPalette.buttonSurface);
    m_colors.buttonBorder = QColor(kDarkPalette.buttonBorder);
    m_colors.primaryText = QColor(kDarkPalette.primaryText);
    m_colors.secondaryText = QColor(kDarkPalette.secondaryText);
    m_colors.mutedText = QColor(kDarkPalette.mutedText);
    m_colors.accent = QColor(kDarkPalette.accent);
    m_colors.accentSubtle = QColor(kDarkPalette.accentSubtle);
    m_colors.divider = QColor(kDarkPalette.divider);
    m_colors.statusPositive = QColor(kDarkPalette.statusPositive);
    m_colors.statusWarning = QColor(kDarkPalette.statusWarning);
    m_colors.statusNegative = QColor(kDarkPalette.statusNegative);
  } else {
    m_colors.windowBackground = QColor(kLightPalette.windowBackground);
    m_colors.gradientStart = QColor(kLightPalette.gradientStart);
    m_colors.gradientEnd = QColor(kLightPalette.gradientEnd);
    m_colors.overlayTint = QColor(kLightPalette.overlayTint);
    m_colors.surface = QColor(kLightPalette.surface);
    m_colors.surfaceSoft = QColor(kLightPalette.surfaceSoft);
    m_colors.cardHighlight = QColor(kLightPalette.cardHighlight);
    m_colors.buttonSurface = QColor(kLightPalette.buttonSurface);
    m_colors.buttonBorder = QColor(kLightPalette.buttonBorder);
    m_colors.primaryText = QColor(kLightPalette.primaryText);
    m_colors.secondaryText = QColor(kLightPalette.secondaryText);
    m_colors.mutedText = QColor(kLightPalette.mutedText);
    m_colors.accent = QColor(kLightPalette.accent);
    m_colors.accentSubtle = QColor(kLightPalette.accentSubtle);
    m_colors.divider = QColor(kLightPalette.divider);
    m_colors.statusPositive = QColor(kLightPalette.statusPositive);
    m_colors.statusWarning = QColor(kLightPalette.statusWarning);
    m_colors.statusNegative = QColor(kLightPalette.statusNegative);
  }
  
  emit colorsChanged();
}

void ThemeManager::onSystemThemeChanged() {
  bool wasDark = m_systemIsDark;
  m_systemIsDark = (detectSystemTheme() == "dark");

  if (wasDark != m_systemIsDark) {
    qDebug() << "[ThemeManager] System theme changed to:"
             << (m_systemIsDark ? "dark" : "light");

    // Update effective theme if in auto mode
    if (m_themePreference == "auto") {
      updateCurrentTheme();
    }
  }
}

// ============================================================================
// SYSTEM THEME LISTENERS
// ============================================================================

void ThemeManager::startListeningForSystemChanges() {
#if defined(Q_OS_MAC)
  if (m_observer) {
    return;  // Already listening
  }

  ThemeManager* rawSelf = this;

  // Observe appearance changes via distributed notification
  if (@available(macOS 10.14, *)) {
    m_observer = [[NSDistributedNotificationCenter defaultCenter]
        addObserverForName:@"AppleInterfaceThemeChangedNotification"
                    object:nil
                     queue:[NSOperationQueue mainQueue]
                usingBlock:^(NSNotification* note) {
                  Q_UNUSED(note)
                  if (rawSelf) {
                    QMetaObject::invokeMethod(
                        rawSelf,
                        [rawSelf]() { rawSelf->onSystemThemeChanged(); },
                        Qt::QueuedConnection);
                  }
                }];
  }

  qDebug() << "[ThemeManager] Started listening for system theme changes";
#endif
}

void ThemeManager::stopListeningForSystemChanges() {
#if defined(Q_OS_MAC)
  if (m_observer) {
    [[NSDistributedNotificationCenter defaultCenter] 
        removeObserver:(__bridge id)m_observer];
    m_observer = nullptr;
  }
  qDebug() << "[ThemeManager] Stopped listening for system theme changes";
#endif
}

}  // namespace blueplayer::core
