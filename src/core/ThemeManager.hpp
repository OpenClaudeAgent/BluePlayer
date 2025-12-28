#pragma once

#include <QColor>
#include <QObject>
#include <QSettings>
#include <QString>

namespace blueplayer::core {

/**
 * @brief Manages application theme (light/dark mode) with reactive color properties
 *
 * All color properties are reactive - they update automatically when the theme changes.
 * QML components should bind to these properties directly:
 *   color: themeManager.primaryText
 *
 * Supports three modes:
 * - "auto": Follows system theme (macOS appearance)
 * - "light": Forces light theme
 * - "dark": Forces dark theme
 */
class ThemeManager : public QObject {
  Q_OBJECT

  // ========================================================================
  // THEME CONTROL PROPERTIES
  // ========================================================================

  Q_PROPERTY(QString themePreference READ themePreference WRITE
                 setThemePreference NOTIFY themePreferenceChanged)
  Q_PROPERTY(QString currentTheme READ currentTheme NOTIFY currentThemeChanged)
  Q_PROPERTY(bool isDark READ isDark NOTIFY currentThemeChanged)

  // ========================================================================
  // REACTIVE COLOR PROPERTIES - Window & Backgrounds
  // ========================================================================

  Q_PROPERTY(QColor windowBackground READ windowBackground NOTIFY colorsChanged)
  Q_PROPERTY(QColor gradientStart READ gradientStart NOTIFY colorsChanged)
  Q_PROPERTY(QColor gradientEnd READ gradientEnd NOTIFY colorsChanged)
  Q_PROPERTY(QColor overlayTint READ overlayTint NOTIFY colorsChanged)

  // ========================================================================
  // REACTIVE COLOR PROPERTIES - Surfaces
  // ========================================================================

  Q_PROPERTY(QColor surface READ surface NOTIFY colorsChanged)
  Q_PROPERTY(QColor surfaceSoft READ surfaceSoft NOTIFY colorsChanged)
  Q_PROPERTY(QColor cardHighlight READ cardHighlight NOTIFY colorsChanged)
  Q_PROPERTY(QColor buttonSurface READ buttonSurface NOTIFY colorsChanged)
  Q_PROPERTY(QColor buttonBorder READ buttonBorder NOTIFY colorsChanged)

  // ========================================================================
  // REACTIVE COLOR PROPERTIES - Text
  // ========================================================================

  Q_PROPERTY(QColor primaryText READ primaryText NOTIFY colorsChanged)
  Q_PROPERTY(QColor secondaryText READ secondaryText NOTIFY colorsChanged)
  Q_PROPERTY(QColor mutedText READ mutedText NOTIFY colorsChanged)

  // ========================================================================
  // REACTIVE COLOR PROPERTIES - Accent & Brand
  // ========================================================================

  Q_PROPERTY(QColor accent READ accent NOTIFY colorsChanged)
  Q_PROPERTY(QColor accentSubtle READ accentSubtle NOTIFY colorsChanged)

  // ========================================================================
  // REACTIVE COLOR PROPERTIES - Borders & Dividers
  // ========================================================================

  Q_PROPERTY(QColor divider READ divider NOTIFY colorsChanged)

  // ========================================================================
  // REACTIVE COLOR PROPERTIES - Status
  // ========================================================================

  Q_PROPERTY(QColor statusPositive READ statusPositive NOTIFY colorsChanged)
  Q_PROPERTY(QColor statusWarning READ statusWarning NOTIFY colorsChanged)
  Q_PROPERTY(QColor statusNegative READ statusNegative NOTIFY colorsChanged)

 public:
  explicit ThemeManager(QObject* parent = nullptr);
  ~ThemeManager() override;

  // Theme control
  QString themePreference() const;
  void setThemePreference(const QString& preference);
  QString currentTheme() const;
  bool isDark() const;

  // Color accessors - Window & Backgrounds
  QColor windowBackground() const;
  QColor gradientStart() const;
  QColor gradientEnd() const;
  QColor overlayTint() const;

  // Color accessors - Surfaces
  QColor surface() const;
  QColor surfaceSoft() const;
  QColor cardHighlight() const;
  QColor buttonSurface() const;
  QColor buttonBorder() const;

  // Color accessors - Text
  QColor primaryText() const;
  QColor secondaryText() const;
  QColor mutedText() const;

  // Color accessors - Accent & Brand
  QColor accent() const;
  QColor accentSubtle() const;

  // Color accessors - Borders & Dividers
  QColor divider() const;

  // Color accessors - Status
  QColor statusPositive() const;
  QColor statusWarning() const;
  QColor statusNegative() const;

  Q_INVOKABLE QString detectSystemTheme() const;

  void startListeningForSystemChanges();
  void stopListeningForSystemChanges();

 signals:
  void themePreferenceChanged(const QString& preference);
  void currentThemeChanged(const QString& theme);
  void colorsChanged();

 private:
  void updateCurrentTheme();
  void updateColors();
  void onSystemThemeChanged();

  QString m_themePreference;  // "auto", "light", "dark"
  QString m_currentTheme;     // "light" or "dark"
  bool m_systemIsDark;
  QSettings m_settings;

  // Current color values (updated when theme changes)
  struct ColorPalette {
    // Window & Backgrounds
    QColor windowBackground;
    QColor gradientStart;
    QColor gradientEnd;
    QColor overlayTint;

    // Surfaces
    QColor surface;
    QColor surfaceSoft;
    QColor cardHighlight;
    QColor buttonSurface;
    QColor buttonBorder;

    // Text
    QColor primaryText;
    QColor secondaryText;
    QColor mutedText;

    // Accent
    QColor accent;
    QColor accentSubtle;

    // Dividers
    QColor divider;

    // Status
    QColor statusPositive;
    QColor statusWarning;
    QColor statusNegative;
  };

  ColorPalette m_colors;

  // Platform-specific observer handle
  void* m_observer = nullptr;
};

}  // namespace blueplayer::core
