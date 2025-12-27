#include <QtTest/QtTest>
#include "mocks/MockSecureStorage.hpp"

using namespace blueplayer::test;

/**
 * @brief Tests pour MockSecureStorage
 * 
 * Ces tests vérifient que le mock implémente correctement l'interface ISecureStorage.
 * Le vrai SecureStorage utilise le Keychain macOS qui demande des autorisations
 * interactives, donc on teste le mock à la place.
 */
class TestSecureStorage : public QObject {
  Q_OBJECT

private slots:
  void init();
  void cleanup();
  
  void testStoreAndRetrieve();
  void testRetrieveNonExistent();
  void testRetrieveWithDefault();
  void testRemove();
  void testContains();
  void testClear();
  void testMultipleKeys();
  void testStoreEmptyKey();
  void testCount();

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

QTEST_MAIN(TestSecureStorage)
#include "TestSecureStorage.moc"
