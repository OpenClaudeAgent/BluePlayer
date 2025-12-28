#include <QtTest/QtTest>
#include "mocks/MockSecureStorage.hpp"
#include "core/ISecureStorage.hpp"

using namespace blueplayer::test;
using namespace blueplayer::core;

/**
 * @brief Tests pour MockSecureStorage
 * 
 * Ces tests vérifient que le mock implémente correctement l'interface ISecureStorage.
 * Le vrai SecureStorage utilise le Keychain macOS qui demande des autorisations
 * interactives, donc on teste le mock à la place.
 * 
 * Sprint 7 - Plan 24: Tests complets de l'interface ISecureStorage
 */
class TestSecureStorage : public QObject {
  Q_OBJECT

private slots:
  void init();
  void cleanup();
  
  // Tests de base
  void testStoreAndRetrieve();
  void testRetrieveNonExistent();
  void testRetrieveWithDefault();
  void testRemove();
  void testContains();
  void testClear();
  void testMultipleKeys();
  void testStoreEmptyKey();
  void testCount();
  
  // Sprint 7 - Nouveaux tests pour interface ISecureStorage
  void testInterfaceCompliance();
  void testStoreEmptyValue();
  void testRetrieveEmptyKey();
  void testUpdateExistingKey();
  void testRemoveNonExistentKey();
  void testContainsEmptyKey();
  void testKeysMethod();
  void testSpecialCharactersInKey();
  void testSpecialCharactersInValue();
  void testLongValues();

private:
  MockSecureStorage* m_storage = nullptr;
};

void TestSecureStorage::init() {
  m_storage = new MockSecureStorage();
}

void TestSecureStorage::cleanup() {
  delete m_storage;
  m_storage = nullptr;
}

void TestSecureStorage::testStoreAndRetrieve() {
  QString key = "test_key";
  QString value = "test_value_123";
  
  QVERIFY(m_storage->store(key, value));
  QCOMPARE(m_storage->retrieve(key), value);
}

void TestSecureStorage::testRetrieveNonExistent() {
  QString key = "non_existent_key";
  QCOMPARE(m_storage->retrieve(key), QString(""));
}

void TestSecureStorage::testRetrieveWithDefault() {
  QString key = "non_existent_key";
  QString defaultValue = "default_value";
  QCOMPARE(m_storage->retrieve(key, defaultValue), defaultValue);
}

void TestSecureStorage::testRemove() {
  QString key = "test_key_remove";
  QString value = "test_value";
  
  m_storage->store(key, value);
  QVERIFY(m_storage->contains(key));
  
  m_storage->remove(key);
  QVERIFY(!m_storage->contains(key));
  QCOMPARE(m_storage->retrieve(key), QString(""));
}

void TestSecureStorage::testContains() {
  QString key = "test_key_contains";
  QString value = "test_value";
  
  QVERIFY(!m_storage->contains(key));
  
  m_storage->store(key, value);
  QVERIFY(m_storage->contains(key));
  
  m_storage->remove(key);
  QVERIFY(!m_storage->contains(key));
}

void TestSecureStorage::testClear() {
  m_storage->store("key1", "value1");
  m_storage->store("key2", "value2");
  m_storage->store("key3", "value3");
  
  QVERIFY(m_storage->contains("key1"));
  QVERIFY(m_storage->contains("key2"));
  QVERIFY(m_storage->contains("key3"));
  
  m_storage->clear();
  
  QVERIFY(!m_storage->contains("key1"));
  QVERIFY(!m_storage->contains("key2"));
  QVERIFY(!m_storage->contains("key3"));
  QCOMPARE(m_storage->count(), 0);
}

void TestSecureStorage::testMultipleKeys() {
  m_storage->store("key1", "value1");
  m_storage->store("key2", "value2");
  m_storage->store("key3", "value3");
  
  QCOMPARE(m_storage->retrieve("key1"), QString("value1"));
  QCOMPARE(m_storage->retrieve("key2"), QString("value2"));
  QCOMPARE(m_storage->retrieve("key3"), QString("value3"));
  
  // Modifier une valeur
  m_storage->store("key2", "new_value2");
  QCOMPARE(m_storage->retrieve("key2"), QString("new_value2"));
  
  // Les autres valeurs doivent rester inchangées
  QCOMPARE(m_storage->retrieve("key1"), QString("value1"));
  QCOMPARE(m_storage->retrieve("key3"), QString("value3"));
}

void TestSecureStorage::testStoreEmptyKey() {
  // Stocker avec une clé vide doit échouer
  QVERIFY(!m_storage->store("", "value"));
  QVERIFY(!m_storage->contains(""));
}

void TestSecureStorage::testCount() {
  QCOMPARE(m_storage->count(), 0);
  
  m_storage->store("key1", "value1");
  QCOMPARE(m_storage->count(), 1);
  
  m_storage->store("key2", "value2");
  QCOMPARE(m_storage->count(), 2);
  
  m_storage->remove("key1");
  QCOMPARE(m_storage->count(), 1);
  
  m_storage->clear();
  QCOMPARE(m_storage->count(), 0);
}

// ===== Sprint 7 - Nouveaux tests pour interface ISecureStorage =====

void TestSecureStorage::testInterfaceCompliance() {
  // Vérifie que MockSecureStorage implémente correctement ISecureStorage
  ISecureStorage* iface = m_storage;
  
  // Toutes les méthodes de l'interface doivent fonctionner via le pointeur d'interface
  QVERIFY(iface->store("interface_key", "interface_value"));
  QCOMPARE(iface->retrieve("interface_key"), QString("interface_value"));
  QVERIFY(iface->contains("interface_key"));
  
  iface->remove("interface_key");
  QVERIFY(!iface->contains("interface_key"));
  
  iface->clear();
  QCOMPARE(m_storage->count(), 0);
}

void TestSecureStorage::testStoreEmptyValue() {
  // Stocker une valeur vide doit réussir
  QVERIFY(m_storage->store("key_with_empty_value", ""));
  QCOMPARE(m_storage->retrieve("key_with_empty_value"), QString(""));
  QVERIFY(m_storage->contains("key_with_empty_value"));
}

void TestSecureStorage::testRetrieveEmptyKey() {
  // Récupérer avec une clé vide doit retourner la valeur par défaut
  QCOMPARE(m_storage->retrieve(""), QString(""));
  QCOMPARE(m_storage->retrieve("", "default"), QString("default"));
}

void TestSecureStorage::testUpdateExistingKey() {
  // Mettre à jour une clé existante doit remplacer la valeur
  m_storage->store("update_key", "original_value");
  QCOMPARE(m_storage->retrieve("update_key"), QString("original_value"));
  
  m_storage->store("update_key", "updated_value");
  QCOMPARE(m_storage->retrieve("update_key"), QString("updated_value"));
  
  // Le nombre de clés ne doit pas changer
  QCOMPARE(m_storage->count(), 1);
}

void TestSecureStorage::testRemoveNonExistentKey() {
  // Supprimer une clé inexistante ne doit pas crasher
  int countBefore = m_storage->count();
  m_storage->remove("definitely_does_not_exist");
  QCOMPARE(m_storage->count(), countBefore);
}

void TestSecureStorage::testContainsEmptyKey() {
  // Vérifier une clé vide doit retourner false
  QVERIFY(!m_storage->contains(""));
}

void TestSecureStorage::testKeysMethod() {
  // Tester la méthode keys() utilitaire
  m_storage->store("alpha", "value_a");
  m_storage->store("beta", "value_b");
  m_storage->store("gamma", "value_c");
  
  QStringList keys = m_storage->keys();
  QCOMPARE(keys.size(), 3);
  QVERIFY(keys.contains("alpha"));
  QVERIFY(keys.contains("beta"));
  QVERIFY(keys.contains("gamma"));
}

void TestSecureStorage::testSpecialCharactersInKey() {
  // Clés avec caractères spéciaux
  QString specialKey = "key_with-special.chars:123/456";
  QVERIFY(m_storage->store(specialKey, "value"));
  QCOMPARE(m_storage->retrieve(specialKey), QString("value"));
  QVERIFY(m_storage->contains(specialKey));
}

void TestSecureStorage::testSpecialCharactersInValue() {
  // Valeurs avec caractères spéciaux (tokens OAuth, JSON, etc.)
  QString jsonValue = R"({"access_token":"abc123","refresh_token":"xyz789"})";
  QVERIFY(m_storage->store("json_key", jsonValue));
  QCOMPARE(m_storage->retrieve("json_key"), jsonValue);
  
  QString tokenValue = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIn0";
  QVERIFY(m_storage->store("token_key", tokenValue));
  QCOMPARE(m_storage->retrieve("token_key"), tokenValue);
}

void TestSecureStorage::testLongValues() {
  // Tester le stockage de valeurs longues (comme de longs tokens)
  QString longValue = QString(10000, 'x'); // 10000 caractères
  QVERIFY(m_storage->store("long_key", longValue));
  QCOMPARE(m_storage->retrieve("long_key"), longValue);
}

QTEST_MAIN(TestSecureStorage)
#include "TestSecureStorage.moc"
