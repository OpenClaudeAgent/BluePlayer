# BluePlayer Test Code Quality Standards

## Table of Contents
1. [Executive Summary](#executive-summary)
2. [Current State Analysis](#current-state-analysis)
3. [Test Code Quality Standards](#test-code-quality-standards)
4. [Test Smells Identified](#test-smells-identified)
5. [Test Review Checklist](#test-review-checklist)
6. [Refactoring Patterns](#refactoring-patterns)
7. [Test Documentation Standards](#test-documentation-standards)

---

## Executive Summary

This document analyzes the test suite in `/tests/` and establishes quality standards for test code. The codebase contains **26 test files** covering core functionality, API clients, UI view models, and integration tests.

### Overall Assessment

| Category | Rating | Notes |
|----------|--------|-------|
| Test Organization | Good | Clear structure, proper Qt Test Framework usage |
| Setup/Teardown | Good | Consistent init/cleanup patterns |
| Assertion Quality | Fair | Many weak "no crash" assertions |
| DRY Principles | Fair | Duplication in signal validity tests |
| Test Naming | Fair | Inconsistent conventions |
| Test Data Management | Good | TwitchTestData builder pattern |
| Mock Infrastructure | Excellent | Well-designed MockHttpClient |

---

## Current State Analysis

### Strengths

#### 1. Well-Structured Test Framework Usage
```cpp
// Good: Consistent Qt Test lifecycle
class TestCacheManager : public QObject {
  Q_OBJECT
private slots:
  void initTestCase();   // Global setup
  void cleanupTestCase(); // Global teardown
  void init();           // Per-test setup
  void cleanup();        // Per-test teardown
```

#### 2. Data-Driven Tests (Exemplary)
Found in: `TestError.cpp`, `TestLogger.cpp`, `TestTwitchService.cpp`, `TestNetworkCache.cpp`

```cpp
// Good: Data-driven testing reduces duplication
void TestTwitchService::testPropertyInitiallyEmpty_data() {
  QTest::addColumn<QString>("propertyName");
  QTest::addColumn<bool>("requiresLogout");
  
  QTest::newRow("streams") << "streams" << false;
  QTest::newRow("categories") << "categories" << false;
  // ... consolidates 14 individual tests
}

void TestTwitchService::testPropertyInitiallyEmpty() {
  QFETCH(QString, propertyName);
  QFETCH(bool, requiresLogout);
  // Single test logic handles all variations
}
```

#### 3. Test Data Builders (Exemplary)
File: `tests/helpers/TwitchTestData.hpp`

```cpp
// Good: Factory methods for consistent test data
class TwitchTestData {
public:
  static QVariantMap createStream(const QString& userName,
                                  const QString& title,
                                  int viewers,
                                  const QString& gameName);
  static QVariantList createStreams(int count);
  static MockResponse twitchApiResponse(const QJsonArray& data);
};
```

#### 4. Comprehensive Mock Infrastructure
File: `tests/mocks/MockHttpClient.hpp`

```cpp
// Good: Fluent API for mock configuration
MockHttpClient client;
client.whenUrl("/api/users").respond(MockResponse::json(userData));
client.queueResponse(MockResponse::timeout());

// Verification
QCOMPARE(client.requestCount(), 1);
QVERIFY(client.lastRequest().urlContains("/users"));
```

### Weaknesses

#### 1. Weak "No Crash" Assertions
Found in: 15+ test files

```cpp
// Bad: Only tests that code doesn't crash
void TestTwitchApiClient::testListStreamsInvokable() {
  m_client->listStreams(10);
  QVERIFY(m_client != nullptr);  // Weak - only checks no crash
}

// Bad: Comment acknowledges weakness
void TestTwitchChatClient::testParseSimpleMessage() {
  // Les methodes de parsing sont privees, on teste via le comportement
  QVERIFY(m_client != nullptr);  // Empty test
}
```

#### 2. Empty/Placeholder Tests
Found in: `TestTwitchChatClient.cpp`

```cpp
// Bad: Test does nothing meaningful
void TestTwitchChatClient::testParseMessageWithBadges() {
  // Test de parsing de badges - teste via comportement
  QVERIFY(m_client != nullptr);
}
```

#### 3. Duplicated Signal Validity Tests
Found in: Multiple API test files

```cpp
// Duplicated pattern across 20+ tests
void TestTwitchApiClient::testErrorSignalConnection() {
  QSignalSpy spy(m_client, &TwitchApiClient::errorOccurred);
  QVERIFY(spy.isValid());
}

void TestTwitchApiClient::testStreamsReadySignalConnection() {
  QSignalSpy spy(m_client, &TwitchApiClient::streamsReady);
  QVERIFY(spy.isValid());
}
// ... repeated for every signal
```

#### 4. Conditional Logic in Tests
Found in: `TestFileLogger.cpp`, `TestStateMachineLiveReplay.cpp`

```cpp
// Bad: Conditional logic obscures test intent
void TestFileLogger::testNewSessionCreatesNewFile() {
  // ...
  if (firstPath.isEmpty() || secondPath.isEmpty()) {
    QSKIP("Could not create log files");
  }
  if (firstPath != secondPath) {
    QVERIFY(firstPath != secondPath);  // Tautology
  }
}
```

---

## Test Code Quality Standards

### 1. Single Responsibility per Test

**Standard:** Each test should verify ONE specific behavior.

```cpp
// Bad: Testing multiple behaviors
void testUserLogin() {
  user.login("name", "pass");
  QVERIFY(user.isLoggedIn());
  QVERIFY(user.hasSession());
  QCOMPARE(user.lastLoginTime(), QDateTime::currentDateTime());
}

// Good: One behavior per test
void testLogin_ValidCredentials_SetsLoggedInTrue() {
  user.login("name", "pass");
  QVERIFY(user.isLoggedIn());
}

void testLogin_ValidCredentials_CreatesSession() {
  user.login("name", "pass");
  QVERIFY(user.hasSession());
}
```

### 2. Naming Convention: Given_When_Then

**Standard:** Use descriptive names that document behavior.

```cpp
// Pattern: test[Method]_[Condition]_[ExpectedResult]

// Bad
void testLogin();
void testLoginFails();

// Good
void testLogin_ValidCredentials_ReturnsSuccess();
void testLogin_EmptyPassword_ReturnsInvalidCredentialsError();
void testLogin_ExpiredToken_RefreshesAutomatically();
```

### 3. Arrange-Act-Assert Structure

**Standard:** Every test should have clear AAA sections.

```cpp
void TestCacheManager::testUpdateCacheEnd_ValidTime_UpdatesEndTime() {
  // Arrange
  m_cacheManager->startCaching(0.0);
  QSignalSpy endSpy(m_cacheManager, &CacheManager::cacheEndTimeChanged);
  
  // Act
  m_cacheManager->updateCacheEnd(30.0);
  
  // Assert
  QCOMPARE(m_cacheManager->cacheEndTime(), 30.0);
  QCOMPARE(endSpy.count(), 1);
}
```

### 4. No Logic in Tests

**Standard:** Avoid if/else, loops, and complex expressions in test bodies.

```cpp
// Bad: Logic in test
void testProcessItems() {
  for (int i = 0; i < items.size(); ++i) {
    if (items[i].isValid()) {
      processor.process(items[i]);
      QVERIFY(processor.hasResult(i));
    }
  }
}

// Good: Use data-driven tests or separate test methods
void testProcessItems_data() {
  QTest::addColumn<Item>("item");
  QTest::addColumn<bool>("expectedResult");
  
  QTest::newRow("valid") << validItem << true;
  QTest::newRow("invalid") << invalidItem << false;
}

void testProcessItems() {
  QFETCH(Item, item);
  QFETCH(bool, expectedResult);
  
  processor.process(item);
  QCOMPARE(processor.hasResult(), expectedResult);
}
```

### 5. Meaningful Assertions

**Standard:** Assert specific expected values, not just "no crash."

```cpp
// Bad: Weak assertion
void testSetAccessToken() {
  m_client->setAccessToken("new_token");
  QVERIFY(m_client != nullptr);  // Only checks no crash
}

// Good: Verify actual behavior
void testSetAccessToken_ValidToken_StoresToken() {
  m_client->setAccessToken("new_token_12345");
  QCOMPARE(m_client->accessToken(), QString("new_token_12345"));
}

void testSetAccessToken_ValidToken_EmitsTokenChangedSignal() {
  QSignalSpy spy(m_client, &ApiClient::accessTokenChanged);
  m_client->setAccessToken("new_token");
  QCOMPARE(spy.count(), 1);
}
```

### 6. DRY Without Sacrificing Clarity

**Standard:** Extract common setup, but keep test intent clear.

```cpp
// Good: Helper method for common setup
private:
  QVariantMap createTestStream(int viewers = 100) {
    return TwitchTestData::createStream("user", "title", viewers, "game");
  }

// Good: Data-driven for variations
void testViewerCount_data() {
  QTest::addColumn<int>("viewers");
  QTest::addColumn<QString>("expectedFormatted");
  
  QTest::newRow("zero") << 0 << "0";
  QTest::newRow("hundreds") << 500 << "500";
  QTest::newRow("thousands") << 5000 << "5K";
  QTest::newRow("millions") << 1500000 << "1.5M";
}
```

### 7. Isolation and Independence

**Standard:** Tests must not depend on each other or shared mutable state.

```cpp
// Bad: Tests share state
static TwitchService* sharedService;

void testA() {
  sharedService->login();  // Modifies shared state
}

void testB() {
  // Fails if testA didn't run first
  QVERIFY(sharedService->isLoggedIn());
}

// Good: Each test creates its own instance
void init() {
  m_service = new TwitchService(this);
}

void cleanup() {
  delete m_service;
  m_service = nullptr;
}
```

---

## Test Smells Identified

### 1. Eager Tests (Testing Too Much)
**Found in:** `TestTwitchFlow::testMultipleOperationsWithoutAuth`

```cpp
// Smell: Tests 8 different operations in one test
void testMultipleOperationsWithoutAuth() {
  m_service->refreshStreams();
  m_service->refreshRecommendedStreams();
  m_service->refreshCategories();
  // ... 5 more operations
  QVERIFY(m_service->streams().isEmpty());
  // ... 7 more assertions
}
```

**Fix:** Split into individual tests or use data-driven approach.

### 2. Mystery Guests (Unclear Dependencies)
**Found in:** Multiple files using environment variables

```cpp
// Smell: Hidden dependency on environment
void initTestCase() {
  qputenv("TWITCH_CLIENT_ID", "test_client_id");  // Mystery guest
}
```

**Fix:** Make dependencies explicit in test setup.

### 3. Conditional Test Logic
**Found in:** `TestFileLogger.cpp`, `TestStateMachineLiveReplay.cpp`

```cpp
// Smell: Conditionals hide test flow
if (seekSpy.count() > 0) {
  double targetTime = seekSpy.first().at(0).toDouble();
  QCOMPARE(targetTime, 40.0);
}
```

**Fix:** Use `QTRY_COMPARE` or ensure conditions are always met.

### 4. Commented-Out Tests
**None found** - This is good!

### 5. Shared State Between Tests
**Found in:** `TwitchTestData.hpp` with static counters

```cpp
// Smell: Static state can cause flaky tests
static int streamIdCounter = 1;  // Shared across tests
```

**Fix:** Reset counters in test setup or use per-test instance.

### 6. Test Code Duplication
**Severity: High**

Signal validity tests are duplicated across 20+ methods:

```cpp
// Repeated pattern
void testSignalX() {
  QSignalSpy spy(m_obj, &Class::signalX);
  QVERIFY(spy.isValid());
}
```

**Fix:** Use data-driven test for signal validity:

```cpp
void testSignalValidity_data() {
  QTest::addColumn<QString>("signalName");
  QTest::newRow("errorOccurred") << "errorOccurred";
  QTest::newRow("streamsReady") << "streamsReady";
  // ...
}
```

### 7. Empty Tests (Placeholder Tests)
**Found in:** `TestTwitchChatClient.cpp`

```cpp
void testParseSimpleMessage() {
  QVERIFY(m_client != nullptr);  // Does nothing useful
}
```

**Fix:** Either implement the test or remove it with a TODO comment.

---

## Test Review Checklist

### Before Submitting Tests for Review

| # | Check | Requirement |
|---|-------|-------------|
| 1 | **Single Responsibility** | Each test verifies ONE behavior |
| 2 | **Descriptive Name** | Name describes Given/When/Then |
| 3 | **AAA Structure** | Clear Arrange, Act, Assert sections |
| 4 | **No Logic** | No if/else, loops, or try/catch in test body |
| 5 | **Meaningful Assertions** | Tests verify behavior, not just "no crash" |
| 6 | **Independence** | Test doesn't depend on other tests |
| 7 | **Deterministic** | Test always produces same result |
| 8 | **Fast** | Test completes in < 100ms (unit), < 1s (integration) |
| 9 | **No External Dependencies** | Unit tests don't hit network/filesystem |
| 10 | **Clear Failure Message** | Assertion failure is self-explanatory |

### Assertion Best Practices

```cpp
// Prefer specific assertions
QCOMPARE(actual, expected);        // Better than QVERIFY(actual == expected)
QCOMPARE(list.size(), 5);          // Clear expected value
QCOMPARE(error.code(), ErrorCode::NetworkError);  // Specific enum comparison

// Use QVERIFY2 for context
QVERIFY2(result.isSuccess(), 
         qPrintable("Expected success but got: " + result.error().message()));

// Use QTRY_ macros for async operations
QTRY_COMPARE(spy.count(), 1);      // Waits for condition
QTRY_VERIFY(client.isConnected());
```

### Common Anti-Patterns to Reject

| Anti-Pattern | Example | Fix |
|--------------|---------|-----|
| **Crash-only test** | `QVERIFY(obj != nullptr)` | Assert actual behavior |
| **Magic numbers** | `QCOMPARE(result, 42)` | Use named constants |
| **Test pollution** | Static state without reset | Reset in cleanup() |
| **Flaky async** | No wait for signal | Use QTRY_* or QSignalSpy::wait() |
| **Hidden setup** | Dependencies in initTestCase | Make explicit or document |
| **Copy-paste tests** | Identical test structure | Use data-driven tests |
| **Commented code** | `// QCOMPARE(...)` | Remove or re-enable |

---

## Refactoring Patterns

### 1. Extract Method for Common Setup

**Before:**
```cpp
void testUpdateStreams() {
  QVariantList streams;
  QVariantMap s1;
  s1["user_name"] = "User1";
  s1["title"] = "Stream 1";
  s1["viewer_count"] = 100;
  streams.append(s1);
  // ... more setup
}
```

**After:**
```cpp
private:
  QVariantMap createStream(const QString& name, int viewers) {
    return TwitchTestData::createStream(name, "Title", viewers, "Game");
  }

void testUpdateStreams() {
  QVariantList streams = { createStream("User1", 100) };
  // ... test logic
}
```

### 2. Parameterized Tests for Variations

**Before:**
```cpp
void testLoginEmptyUsername() { /* ... */ }
void testLoginEmptyPassword() { /* ... */ }
void testLoginInvalidUsername() { /* ... */ }
void testLoginInvalidPassword() { /* ... */ }
```

**After:**
```cpp
void testLoginValidation_data() {
  QTest::addColumn<QString>("username");
  QTest::addColumn<QString>("password");
  QTest::addColumn<ErrorCode>("expectedError");
  
  QTest::newRow("empty_username") << "" << "pass" << ErrorCode::InvalidUsername;
  QTest::newRow("empty_password") << "user" << "" << ErrorCode::InvalidPassword;
  QTest::newRow("invalid_username") << "a" << "pass" << ErrorCode::InvalidUsername;
}

void testLoginValidation() {
  QFETCH(QString, username);
  QFETCH(QString, password);
  QFETCH(ErrorCode, expectedError);
  
  auto result = auth.login(username, password);
  QCOMPARE(result.error().code(), expectedError);
}
```

### 3. Custom Assertions for Domain Concepts

**Before:**
```cpp
void testStreamReady() {
  auto stream = service.getStream(0);
  QVERIFY(!stream["user_name"].toString().isEmpty());
  QVERIFY(stream["viewer_count"].toInt() >= 0);
  QVERIFY(!stream["thumbnail_url"].toString().isEmpty());
  QVERIFY(stream["thumbnail_url"].toString().startsWith("https://"));
}
```

**After:**
```cpp
// In TestHelpers.hpp
#define QVERIFY_STREAM_VALID(stream) \
  do { \
    QVERIFY2(!stream["user_name"].toString().isEmpty(), "Stream missing user_name"); \
    QVERIFY2(stream["viewer_count"].toInt() >= 0, "Invalid viewer count"); \
    QVERIFY2(stream["thumbnail_url"].toString().startsWith("https://"), \
             "Invalid thumbnail URL"); \
  } while (0)

void testStreamReady() {
  auto stream = service.getStream(0);
  QVERIFY_STREAM_VALID(stream);
}
```

### 4. Test Data Builders (Already Implemented)

The `TwitchTestData` class is an excellent example:

```cpp
// Usage example
auto stream = TwitchTestData::createStream("Ninja", "Playing Fortnite", 50000, "Fortnite");
auto response = TwitchTestData::createApiResponse(streams);
auto error = TwitchTestData::createErrorResponse("Not Found", 404, "Stream not found");
```

### 5. Signal Testing Pattern

**Before (duplicated):**
```cpp
void testErrorSignal() {
  QSignalSpy spy(obj, &Class::error);
  QVERIFY(spy.isValid());
}
void testSuccessSignal() {
  QSignalSpy spy(obj, &Class::success);
  QVERIFY(spy.isValid());
}
```

**After (consolidated):**
```cpp
void testSignalExists_data() {
  QTest::addColumn<const char*>("signalSignature");
  QTest::newRow("error") << SIGNAL(error(QString));
  QTest::newRow("success") << SIGNAL(success());
}

void testSignalExists() {
  QFETCH(const char*, signalSignature);
  
  int signalIndex = obj->metaObject()->indexOfSignal(
    QMetaObject::normalizedSignature(signalSignature + 1));
  QVERIFY2(signalIndex >= 0, 
           qPrintable(QString("Signal not found: %1").arg(signalSignature)));
}
```

---

## Test Documentation Standards

### When to Document Tests

| Situation | Documentation Required |
|-----------|----------------------|
| Complex setup/preconditions | Yes - explain why |
| Non-obvious assertions | Yes - explain what's being verified |
| Integration test with external deps | Yes - document requirements |
| Edge case/boundary test | Yes - explain the edge case |
| Simple unit test | No - name should be sufficient |
| Data-driven test | Document data generation logic |

### Documentation Format

```cpp
/**
 * @brief Tests that cache duration only increases
 *
 * The cache manager must never allow the end time to decrease,
 * as this would invalidate cached segments. This test verifies
 * that updateCacheEnd ignores values less than current end time.
 *
 * @see CacheManager::updateCacheEnd
 */
void TestCacheManager::testUpdateCacheEnd_SmallerValue_IgnoresUpdate() {
  // Arrange
  m_cacheManager->startCaching(0.0);
  m_cacheManager->updateCacheEnd(50.0);
  
  // Act - try to set a smaller value
  m_cacheManager->updateCacheEnd(30.0);
  
  // Assert - original value preserved
  QCOMPARE(m_cacheManager->cacheEndTime(), 50.0);
}
```

### Complex Scenario Documentation

```cpp
/**
 * @brief Integration test: Full replay workflow
 *
 * This test verifies the complete workflow for entering replay mode:
 * 1. Start with live stream at position 100
 * 2. User requests seekback to earliest cached position
 * 3. State machine transitions: Live -> SeekbackSeeking -> ReplayPlaying
 * 4. Player seeks to the calculated target time
 *
 * Prerequisites:
 * - Cache must have at least 60 seconds of content
 * - Live edge must be ahead of cache start
 *
 * Expected signals:
 * - stateChanged (2 times)
 * - seekRequested (1 time with target = liveEdge - cacheDuration)
 * - replayModeRequested (1 time)
 */
void TestStateMachineLiveReplay::testSeekBackFromLive() {
  // ...
}
```

### Living Documentation from Tests

Tests serve as executable documentation. To maximize this:

1. **Use descriptive test names** as documentation
2. **Group related tests** with section comments
3. **Use `_data()` rows** as scenario documentation

```cpp
// Section comment documents test group
// ===== Tests de validation temporelle =====

void testIsTimeInCache_data() {
  QTest::addColumn<double>("cacheStart");
  QTest::addColumn<double>("cacheEnd");
  QTest::addColumn<double>("queryTime");
  QTest::addColumn<bool>("expected");
  
  // Row names document scenarios
  QTest::newRow("at_start_boundary") << 10.0 << 60.0 << 10.0 << true;
  QTest::newRow("at_end_boundary") << 10.0 << 60.0 << 60.0 << true;
  QTest::newRow("in_middle") << 10.0 << 60.0 << 35.0 << true;
  QTest::newRow("before_cache") << 10.0 << 60.0 << 5.0 << false;
  QTest::newRow("after_cache") << 10.0 << 60.0 << 100.0 << false;
  QTest::newRow("slightly_before") << 10.0 << 60.0 << 9.99 << false;
}
```

---

## Priority Refactoring Actions

### High Priority

1. **Strengthen weak assertions** in `TestTwitchApiClient.cpp`, `TestTwitchChatClient.cpp`
   - Replace `QVERIFY(m_client != nullptr)` with actual behavior verification
   
2. **Consolidate signal validity tests** using data-driven pattern
   - Affects 8+ test files, ~40 duplicated tests

3. **Remove empty placeholder tests** in `TestTwitchChatClient.cpp`
   - Either implement or mark with TODO issue

### Medium Priority

4. **Standardize test naming** to Given_When_Then convention
   - Create naming guide and apply incrementally

5. **Extract test data builders** for common objects
   - Extend `TwitchTestData` pattern to other domains

6. **Add missing assertions** to async tests
   - Use `QSignalSpy::wait()` or `QTRY_*` macros

### Low Priority

7. **Document complex integration tests**
   - Add docstrings to `TestTwitchFlow.cpp`, `TestStateMachineLiveReplay.cpp`

8. **Translate French comments** to English for consistency

---

## Metrics and Goals

| Metric | Current | Target |
|--------|---------|--------|
| Tests with weak assertions | ~25% | < 5% |
| Duplicated test patterns | ~15% | < 5% |
| Empty/placeholder tests | 3 | 0 |
| Tests with conditional logic | ~8% | < 2% |
| Tests following naming convention | ~60% | > 95% |
| Data-driven test coverage | Good | Maintain |
| Mock infrastructure quality | Excellent | Maintain |

---

*Document Version: 1.0*  
*Generated: 2025*  
*Applies to: `/tests/` directory*
