#include "LanguageManager.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QLocale>
#include <QSettings>

namespace blueplayer::core {

LanguageManager::LanguageManager(QObject* parent)
    : QObject(parent), m_translator(std::make_unique<QTranslator>()) {
  // Load saved preference
  QSettings settings;
  m_currentLanguage = settings.value("i18n/language", "system").toString();
}

void LanguageManager::setEngine(QQmlEngine* engine) { m_engine = engine; }

QString LanguageManager::currentLanguage() const { return m_currentLanguage; }

QStringList LanguageManager::availableLanguages() const {
  return {"system", "en", "fr"};
}

void LanguageManager::setLanguage(const QString& languageCode) {
  if (m_currentLanguage == languageCode) {
    return;
  }

  qDebug() << "[LanguageManager] Changing language from" << m_currentLanguage
           << "to" << languageCode;

  // Save preference
  QSettings settings;
  settings.setValue("i18n/language", languageCode);
  m_currentLanguage = languageCode;

  // Load new translation
  QString locale = resolveLocale(languageCode);
  loadTranslation(locale);

  // Trigger QML retranslation
  if (m_engine != nullptr) {
    m_engine->retranslate();
    qDebug() << "[LanguageManager] QML retranslation triggered";
  }

  emit languageChanged();
}

QString LanguageManager::resolveLocale(const QString& languageCode) const {
  if (languageCode.isEmpty() || languageCode == "system") {
    return QLocale::system().name();  // e.g., "fr_FR", "en_US"
  }
  return languageCode;
}

void LanguageManager::loadTranslation(const QString& locale) {
  // Remove current translator
  if (m_translator->isEmpty() == false) {
    QCoreApplication::removeTranslator(m_translator.get());
  }

  // Try to load new translation
  bool loaded = m_translator->load("blueplayer_" + locale, ":/i18n");

  // Fallback for French variants
  if (!loaded && locale.startsWith("fr")) {
    loaded = m_translator->load("blueplayer_fr", ":/i18n");
  }

  if (loaded) {
    QCoreApplication::installTranslator(m_translator.get());
    qDebug() << "[LanguageManager] Loaded translation for:" << locale;
  } else {
    qDebug() << "[LanguageManager] Using default English (no translation for:"
             << locale << ")";
  }
}

}  // namespace blueplayer::core
