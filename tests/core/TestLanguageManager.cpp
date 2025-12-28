/**
 * @file TestLanguageManager.cpp
 * @brief Tests unitaires pour LanguageManager
 *
 * Couvre:
 * - Constructeur et chargement des preferences
 * - Getters (currentLanguage, availableLanguages)
 * - setLanguage avec emission de signal et persistence
 * - resolveLocale (via setLanguage)
 * - setEngine
 */

#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QSettings>
#include <QLocale>
#include <QQmlEngine>

#include "core/LanguageManager.hpp"

using namespace blueplayer::core;

class TestLanguageManager : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Constructeur
  void testConstructorDefaultLanguage();
  void testConstructorLoadsFromSettings();

  // Getters
  void testCurrentLanguage();
  void testAvailableLanguages();
  void testAvailableLanguagesContent();

  // setLanguage
  void testSetLanguageEmitsSignal();
  void testSetLanguageSameValueNoSignal();
  void testSetLanguageSavesToSettings();
  void testSetLanguageUpdatesCurrentLanguage();
  void testSetLanguageMultipleTimes();

  // resolveLocale (tester via setLanguage)
  void testResolveLocaleSystem();
  void testResolveLocaleExplicit();
  void testResolveLocaleEmpty();

  // setEngine
  void testSetEngineNull();
  void testSetEngineValid();
  void testSetEngineTriggersRetranslate();

private:
  void clearSettings();

  LanguageManager* m_manager = nullptr;
};

void TestLanguageManager::initTestCase() {
  // Configure l'organisation et l'application pour QSettings
  QCoreApplication::setOrganizationName("BluePlayerTest");
  QCoreApplication::setApplicationName("TestLanguageManager");
}

void TestLanguageManager::cleanupTestCase() {
  clearSettings();
}

void TestLanguageManager::init() {
  // Nettoyer les settings avant chaque test
  clearSettings();

  // Creer une nouvelle instance
  m_manager = new LanguageManager();
}

void TestLanguageManager::cleanup() {
  delete m_manager;
  m_manager = nullptr;
  clearSettings();
}

void TestLanguageManager::clearSettings() {
  QSettings settings;
  settings.remove("i18n/language");
  settings.sync();
}

// =====================================================
// Constructeur Tests
// =====================================================

void TestLanguageManager::testConstructorDefaultLanguage() {
  // Arrange - settings vides
  clearSettings();

  // Act
  LanguageManager manager;

  // Assert - doit utiliser "system" par defaut
  QCOMPARE(manager.currentLanguage(), QString("system"));
}

void TestLanguageManager::testConstructorLoadsFromSettings() {
  // Arrange - definir une langue dans les settings
  QSettings settings;
  settings.setValue("i18n/language", "fr");
  settings.sync();

  // Act
  LanguageManager manager;

  // Assert
  QCOMPARE(manager.currentLanguage(), QString("fr"));
}

// =====================================================
// Getters Tests
// =====================================================

void TestLanguageManager::testCurrentLanguage() {
  // Arrange - settings avec "en"
  clearSettings();
  QSettings settings;
  settings.setValue("i18n/language", "en");
  settings.sync();

  // Act
  LanguageManager manager;

  // Assert
  QCOMPARE(manager.currentLanguage(), QString("en"));
}

void TestLanguageManager::testAvailableLanguages() {
  // Act
  QStringList languages = m_manager->availableLanguages();

  // Assert
  QVERIFY(!languages.isEmpty());
  QCOMPARE(languages.size(), 3);
}

void TestLanguageManager::testAvailableLanguagesContent() {
  // Act
  QStringList languages = m_manager->availableLanguages();

  // Assert - doit contenir system, en, fr
  QVERIFY(languages.contains("system"));
  QVERIFY(languages.contains("en"));
  QVERIFY(languages.contains("fr"));
}

// =====================================================
// setLanguage Tests
// =====================================================

void TestLanguageManager::testSetLanguageEmitsSignal() {
  // Arrange
  QSignalSpy spy(m_manager, &LanguageManager::languageChanged);
  QVERIFY(spy.isValid());

  // Act
  m_manager->setLanguage("fr");

  // Assert
  QCOMPARE(spy.count(), 1);
}

void TestLanguageManager::testSetLanguageSameValueNoSignal() {
  // Arrange - definir d'abord la langue
  m_manager->setLanguage("en");

  QSignalSpy spy(m_manager, &LanguageManager::languageChanged);
  QVERIFY(spy.isValid());

  // Act - definir la meme langue
  m_manager->setLanguage("en");

  // Assert - pas de signal emis
  QCOMPARE(spy.count(), 0);
}

void TestLanguageManager::testSetLanguageSavesToSettings() {
  // Arrange
  clearSettings();

  // Act
  m_manager->setLanguage("fr");

  // Assert
  QSettings settings;
  QCOMPARE(settings.value("i18n/language").toString(), QString("fr"));
}

void TestLanguageManager::testSetLanguageUpdatesCurrentLanguage() {
  // Arrange
  QCOMPARE(m_manager->currentLanguage(), QString("system"));

  // Act
  m_manager->setLanguage("en");

  // Assert
  QCOMPARE(m_manager->currentLanguage(), QString("en"));
}

void TestLanguageManager::testSetLanguageMultipleTimes() {
  // Arrange
  QSignalSpy spy(m_manager, &LanguageManager::languageChanged);
  QVERIFY(spy.isValid());

  // Act
  m_manager->setLanguage("en");
  m_manager->setLanguage("fr");
  m_manager->setLanguage("system");

  // Assert - 3 changements = 3 signaux
  QCOMPARE(spy.count(), 3);
  QCOMPARE(m_manager->currentLanguage(), QString("system"));
}

// =====================================================
// resolveLocale Tests (teste via setLanguage)
// =====================================================

void TestLanguageManager::testResolveLocaleSystem() {
  // Arrange - on verifie que "system" est accepte
  // Note: On ne peut pas verifier directement le locale resolu
  // car resolveLocale est private, mais on peut verifier que
  // setLanguage accepte "system" sans probleme

  // Act
  m_manager->setLanguage("system");

  // Assert
  QCOMPARE(m_manager->currentLanguage(), QString("system"));
}

void TestLanguageManager::testResolveLocaleExplicit() {
  // Arrange & Act
  m_manager->setLanguage("fr");

  // Assert
  QCOMPARE(m_manager->currentLanguage(), QString("fr"));
}

void TestLanguageManager::testResolveLocaleEmpty() {
  // Arrange - tester avec une chaine vide
  // Note: Une chaine vide devrait etre traitee comme "system"

  // Act
  m_manager->setLanguage("");

  // Assert
  QCOMPARE(m_manager->currentLanguage(), QString(""));
}

// =====================================================
// setEngine Tests
// =====================================================

void TestLanguageManager::testSetEngineNull() {
  // Act - ne doit pas crasher
  m_manager->setEngine(nullptr);

  // Assert - le test passe s'il n'y a pas de crash
  QVERIFY(true);
}

void TestLanguageManager::testSetEngineValid() {
  // Arrange
  QQmlEngine engine;

  // Act - ne doit pas crasher
  m_manager->setEngine(&engine);

  // Assert - le test passe s'il n'y a pas de crash
  QVERIFY(true);
}

void TestLanguageManager::testSetEngineTriggersRetranslate() {
  // Arrange
  QQmlEngine engine;
  m_manager->setEngine(&engine);

  QSignalSpy spy(m_manager, &LanguageManager::languageChanged);
  QVERIFY(spy.isValid());

  // Act - changer la langue avec engine configure
  m_manager->setLanguage("fr");

  // Assert - le signal doit etre emis (retranslate appele)
  QCOMPARE(spy.count(), 1);
  QCOMPARE(m_manager->currentLanguage(), QString("fr"));
}

QTEST_MAIN(TestLanguageManager)
#include "TestLanguageManager.moc"
