/**
 * @file TestResult.cpp
 * @brief Tests unitaires pour la classe Result<T>
 */

#include <QtTest/QtTest>
#include "core/Result.hpp"

using namespace blueplayer::core;

class TestResult : public QObject {
  Q_OBJECT

private slots:
  // ===== Tests Result<T> - création =====
  void testSuccessCreation();
  void testFailureCreation();

  // ===== Tests Result<T> - isSuccess/isFailure =====
  void testIsSuccessOnSuccess();
  void testIsSuccessOnFailure();
  void testIsFailureOnSuccess();
  void testIsFailureOnFailure();

  // ===== Tests Result<T> - value() =====
  void testValueOnSuccess();
  void testValueOnSuccessString();
  void testValueOrOnSuccess();
  void testValueOrOnFailure();

  // ===== Tests Result<T> - error() =====
  void testErrorOnFailure();
  void testErrorCodeOnFailure();

  // ===== Tests Result<T> - map() =====
  void testMapOnSuccess();
  void testMapOnFailure();
  void testMapChaining();

  // ===== Tests Result<T> - flatMap() =====
  void testFlatMapOnSuccess();
  void testFlatMapOnFailure();
  void testFlatMapChaining();

  // ===== Tests Result<void> =====
  void testVoidSuccessCreation();
  void testVoidFailureCreation();
  void testVoidIsSuccessOnSuccess();
  void testVoidIsSuccessOnFailure();
  void testVoidIsFailureOnSuccess();
  void testVoidIsFailureOnFailure();
  void testVoidErrorOnFailure();

  // ===== Tests avec types complexes =====
  void testWithQString();
  void testWithQStringList();
  void testWithCustomStruct();
};

// Helper struct for testing
struct TestData {
  int id;
  QString name;
  bool operator==(const TestData& other) const {
    return id == other.id && name == other.name;
  }
};

// ===== Tests Result<T> - création =====

void TestResult::testSuccessCreation() {
  auto result = Result<int>::success(42);
  QVERIFY(result.isSuccess());
  QCOMPARE(result.value(), 42);
}

void TestResult::testFailureCreation() {
  auto result = Result<int>::failure(Error(ErrorCode::NetworkError));
  QVERIFY(result.isFailure());
  QCOMPARE(result.error().code(), ErrorCode::NetworkError);
}

// ===== Tests Result<T> - isSuccess/isFailure =====

void TestResult::testIsSuccessOnSuccess() {
  auto result = Result<int>::success(10);
  QVERIFY(result.isSuccess());
}

void TestResult::testIsSuccessOnFailure() {
  auto result = Result<int>::failure(Error(ErrorCode::Unknown));
  QVERIFY(!result.isSuccess());
}

void TestResult::testIsFailureOnSuccess() {
  auto result = Result<int>::success(10);
  QVERIFY(!result.isFailure());
}

void TestResult::testIsFailureOnFailure() {
  auto result = Result<int>::failure(Error(ErrorCode::Unknown));
  QVERIFY(result.isFailure());
}

// ===== Tests Result<T> - value() =====

void TestResult::testValueOnSuccess() {
  auto result = Result<int>::success(123);
  QCOMPARE(result.value(), 123);
}

void TestResult::testValueOnSuccessString() {
  auto result = Result<QString>::success("Hello World");
  QCOMPARE(result.value(), QString("Hello World"));
}

void TestResult::testValueOrOnSuccess() {
  auto result = Result<int>::success(42);
  QCOMPARE(result.valueOr(0), 42);
}

void TestResult::testValueOrOnFailure() {
  auto result = Result<int>::failure(Error(ErrorCode::Unknown));
  QCOMPARE(result.valueOr(99), 99);
}

// ===== Tests Result<T> - error() =====

void TestResult::testErrorOnFailure() {
  Error err(ErrorCode::NetworkTimeout, "Connection timed out");
  auto result = Result<int>::failure(err);
  QCOMPARE(result.error().message(), QString("Connection timed out"));
}

void TestResult::testErrorCodeOnFailure() {
  auto result = Result<QString>::failure(Error(ErrorCode::InvalidArgument));
  QCOMPARE(result.error().code(), ErrorCode::InvalidArgument);
}

// ===== Tests Result<T> - map() =====

void TestResult::testMapOnSuccess() {
  auto result = Result<int>::success(10);
  auto mapped = result.map([](int x) { return x * 2; });
  
  QVERIFY(mapped.isSuccess());
  QCOMPARE(mapped.value(), 20);
}

void TestResult::testMapOnFailure() {
  auto result = Result<int>::failure(Error(ErrorCode::NetworkError));
  auto mapped = result.map([](int x) { return x * 2; });
  
  QVERIFY(mapped.isFailure());
  QCOMPARE(mapped.error().code(), ErrorCode::NetworkError);
}

void TestResult::testMapChaining() {
  auto result = Result<int>::success(5);
  auto chained = result
    .map([](int x) { return x + 10; })
    .map([](int x) { return x * 2; })
    .map([](int x) { return QString::number(x); });
  
  QVERIFY(chained.isSuccess());
  QCOMPARE(chained.value(), QString("30"));
}

// ===== Tests Result<T> - flatMap() =====

void TestResult::testFlatMapOnSuccess() {
  auto result = Result<int>::success(10);
  auto flatMapped = result.flatMap([](int x) {
    return Result<QString>::success(QString::number(x));
  });
  
  QVERIFY(flatMapped.isSuccess());
  QCOMPARE(flatMapped.value(), QString("10"));
}

void TestResult::testFlatMapOnFailure() {
  auto result = Result<int>::failure(Error(ErrorCode::InvalidState));
  auto flatMapped = result.flatMap([](int x) {
    return Result<QString>::success(QString::number(x));
  });
  
  QVERIFY(flatMapped.isFailure());
  QCOMPARE(flatMapped.error().code(), ErrorCode::InvalidState);
}

void TestResult::testFlatMapChaining() {
  auto divide = [](int a, int b) -> Result<int> {
    if (b == 0) {
      return Result<int>::failure(Error(ErrorCode::InvalidArgument, "Division by zero"));
    }
    return Result<int>::success(a / b);
  };
  
  // Test successful chain
  auto result1 = Result<int>::success(100)
    .flatMap([&divide](int x) { return divide(x, 2); })
    .flatMap([&divide](int x) { return divide(x, 5); });
  
  QVERIFY(result1.isSuccess());
  QCOMPARE(result1.value(), 10);
  
  // Test failure in chain
  auto result2 = Result<int>::success(100)
    .flatMap([&divide](int x) { return divide(x, 0); })
    .flatMap([&divide](int x) { return divide(x, 5); });
  
  QVERIFY(result2.isFailure());
  QCOMPARE(result2.error().code(), ErrorCode::InvalidArgument);
}

// ===== Tests Result<void> =====

void TestResult::testVoidSuccessCreation() {
  auto result = Result<void>::success();
  QVERIFY(result.isSuccess());
}

void TestResult::testVoidFailureCreation() {
  auto result = Result<void>::failure(Error(ErrorCode::ConfigInvalid));
  QVERIFY(result.isFailure());
}

void TestResult::testVoidIsSuccessOnSuccess() {
  auto result = Result<void>::success();
  QVERIFY(result.isSuccess());
}

void TestResult::testVoidIsSuccessOnFailure() {
  auto result = Result<void>::failure(Error(ErrorCode::Unknown));
  QVERIFY(!result.isSuccess());
}

void TestResult::testVoidIsFailureOnSuccess() {
  auto result = Result<void>::success();
  QVERIFY(!result.isFailure());
}

void TestResult::testVoidIsFailureOnFailure() {
  auto result = Result<void>::failure(Error(ErrorCode::Unknown));
  QVERIFY(result.isFailure());
}

void TestResult::testVoidErrorOnFailure() {
  Error err(ErrorCode::MediaDecodeError, "Failed to decode");
  auto result = Result<void>::failure(err);
  QCOMPARE(result.error().code(), ErrorCode::MediaDecodeError);
  QCOMPARE(result.error().message(), QString("Failed to decode"));
}

// ===== Tests avec types complexes =====

void TestResult::testWithQString() {
  auto result = Result<QString>::success("Test String");
  QVERIFY(result.isSuccess());
  QCOMPARE(result.value(), QString("Test String"));
  
  auto mapped = result.map([](const QString& s) { return s.toUpper(); });
  QCOMPARE(mapped.value(), QString("TEST STRING"));
}

void TestResult::testWithQStringList() {
  QStringList list = {"one", "two", "three"};
  auto result = Result<QStringList>::success(list);
  
  QVERIFY(result.isSuccess());
  QCOMPARE(result.value().size(), 3);
  QCOMPARE(result.value().at(0), QString("one"));
  
  auto mapped = result.map([](const QStringList& l) { return l.size(); });
  QCOMPARE(mapped.value(), 3);
}

void TestResult::testWithCustomStruct() {
  TestData data{42, "Test"};
  auto result = Result<TestData>::success(data);
  
  QVERIFY(result.isSuccess());
  QCOMPARE(result.value().id, 42);
  QCOMPARE(result.value().name, QString("Test"));
  
  auto mapped = result.map([](const TestData& d) { return d.id; });
  QCOMPARE(mapped.value(), 42);
}

QTEST_MAIN(TestResult)
#include "TestResult.moc"
