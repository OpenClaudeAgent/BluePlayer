#include <QtTest/QtTest>
#include "core/SecureStorage.hpp"

using namespace blueplayer::core;

class TestSecureStorage : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  
  void testStoreAndRetrieve();
  void testRetrieveNonExistent();
  void testRetrieveWithDefault();
  void testRemove();
  void testContains();
  void testClear();
  void testMultipleKeys();
};

void TestSecureStorage::initTestCase() {
}

void TestSecureStorage::cleanupTestCase() {
  // Nettoyer les données de test
  SecureStorage storage;
  storage.clear();
}

void TestSecureStorage::testStoreAndRetrieve() {
  SecureStorage storage;
  
  QString key = "test_key";
  QString value = "test_value_123";
  
  QVERIFY(storage.store(key, value));
  QCOMPARE(storage.retrieve(key), value);
}

void TestSecureStorage::testRetrieveNonExistent() {
  SecureStorage storage;
  
  QString key = "non_existent_key";
  QCOMPARE(storage.retrieve(key), QString(""));
}

void TestSecureStorage::testRetrieveWithDefault() {
  SecureStorage storage;
  
  QString key = "non_existent_key";
  QString defaultValue = "default_value";
  QCOMPARE(storage.retrieve(key, defaultValue), defaultValue);
}

void TestSecureStorage::testRemove() {
  SecureStorage storage;
  
  QString key = "test_key_remove";
  QString value = "test_value";
  
  storage.store(key, value);
  QVERIFY(storage.contains(key));
  
  storage.remove(key);
  QVERIFY(!storage.contains(key));
  QCOMPARE(storage.retrieve(key), QString(""));
}

void TestSecureStorage::testContains() {
  SecureStorage storage;
  
  QString key = "test_key_contains";
  QString value = "test_value";
  
  QVERIFY(!storage.contains(key));
  
  storage.store(key, value);
  QVERIFY(storage.contains(key));
  
  storage.remove(key);
  QVERIFY(!storage.contains(key));
}

void TestSecureStorage::testClear() {
  SecureStorage storage;
  
  storage.store("key1", "value1");
  storage.store("key2", "value2");
  storage.store("key3", "value3");
  
  QVERIFY(storage.contains("key1"));
  QVERIFY(storage.contains("key2"));
  QVERIFY(storage.contains("key3"));
  
  storage.clear();
  
  QVERIFY(!storage.contains("key1"));
  QVERIFY(!storage.contains("key2"));
  QVERIFY(!storage.contains("key3"));
}

void TestSecureStorage::testMultipleKeys() {
  SecureStorage storage;
  
  storage.store("key1", "value1");
  storage.store("key2", "value2");
  storage.store("key3", "value3");
  
  QCOMPARE(storage.retrieve("key1"), QString("value1"));
  QCOMPARE(storage.retrieve("key2"), QString("value2"));
  QCOMPARE(storage.retrieve("key3"), QString("value3"));
  
  // Modifier une valeur
  storage.store("key2", "new_value2");
  QCOMPARE(storage.retrieve("key2"), QString("new_value2"));
  
  // Les autres valeurs doivent rester inchangées
  QCOMPARE(storage.retrieve("key1"), QString("value1"));
  QCOMPARE(storage.retrieve("key3"), QString("value3"));
}

QTEST_MAIN(TestSecureStorage)
#include "TestSecureStorage.moc"



