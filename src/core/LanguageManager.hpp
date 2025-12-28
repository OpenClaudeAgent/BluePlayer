#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QTranslator>
#include <memory>

namespace blueplayer::core {

/**
 * @brief Manages application language and translations at runtime.
 *
 * This class allows changing the application language without restarting.
 * It handles loading/unloading QTranslator and triggering QML retranslation.
 */
class LanguageManager final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString currentLanguage READ currentLanguage NOTIFY languageChanged)

 public:
  explicit LanguageManager(QObject* parent = nullptr);
  ~LanguageManager() override = default;

  /**
   * @brief Sets the QML engine reference (required for retranslation).
   */
  void setEngine(QQmlEngine* engine);

  /**
   * @brief Gets the current language code.
   * @return "system", "en", "fr", etc.
   */
  [[nodiscard]] QString currentLanguage() const;

  /**
   * @brief Available language codes.
   */
  Q_INVOKABLE QStringList availableLanguages() const;

 public slots:
  /**
   * @brief Changes the application language at runtime.
   * @param languageCode "system", "en", "fr", etc.
   *
   * This will:
   * 1. Save the preference to QSettings
   * 2. Load the appropriate translation file
   * 3. Trigger QML retranslation (all qsTr() calls are re-evaluated)
   */
  void setLanguage(const QString& languageCode);

 signals:
  /**
   * @brief Emitted when the language changes.
   */
  void languageChanged();

 private:
  void loadTranslation(const QString& locale);
  QString resolveLocale(const QString& languageCode) const;

  QQmlEngine* m_engine = nullptr;
  std::unique_ptr<QTranslator> m_translator;
  QString m_currentLanguage;
};

}  // namespace blueplayer::core
