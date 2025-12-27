# BluePlayer Test Architecture Blueprint

## Executive Summary

This document presents a world-class test architecture for BluePlayer, incorporating best practices from Google, Microsoft, and Qt testing guidelines.

---

## 1. Current State Analysis

### Strengths
- Qt Test Framework properly integrated
- Comprehensive mock infrastructure (`MockHttpClient`, `MockNetworkReply`)
- Good test data factories (`TwitchTestData`)
- Separate test executables per module
- Coverage support enabled
- Data-driven tests used in `TestTwitchService`, `TestError`

### Areas for Improvement
- No base test class hierarchy
- Duplicate setup/teardown code
- No test categorization (smoke, regression, performance)
- Missing Page Object pattern for UI tests
- No chaos/fault injection tests
- No performance benchmarks
- Inconsistent naming conventions
- Limited dependency injection

---

## 2. Test Pyramid Implementation

### Google Test Pyramid Ratios
```
         /\
        /  \   E2E Tests (5%)
       /----\  Integration Tests (15%)
      /------\ Unit Tests (80%)
     /________\
```

### Current vs. Target Distribution

| Test Type    | Current | Target | Gap   |
|--------------|---------|--------|-------|
| Unit Tests   | ~95%    | 80%    | Over  |
| Integration  | ~5%     | 15%    | Under |
| E2E/UI       | ~0%     | 5%     | Under |

---

## 3. Base Test Class Hierarchy

### 3.1 Core Base Classes

```cpp
// tests/base/TestBase.hpp
#pragma once

#include <QtTest/QtTest>
#include <QObject>
#include <QTemporaryDir>
#include <memory>

namespace blueplayer::test {

/**
 * @brief Abstract base class for all tests
 * 
 * Provides:
 * - Common setup/teardown lifecycle
 * - Temporary directory management
 * - Test timing and metrics
 * - Standard assertions and utilities
 */
class TestBase : public QObject {
  Q_OBJECT

protected:
  // Lifecycle hooks - override in derived classes
  virtual void onSetUp() {}
  virtual void onTearDown() {}
  virtual void onTestCaseSetUp() {}
  virtual void onTestCaseTearDown() {}

  // Utilities
  [[nodiscard]] QString tempPath() const { return m_tempDir->path(); }
  [[nodiscard]] QString tempFile(const QString& name) const;
  void createTempFile(const QString& name, const QByteArray& content);
  
  // Timing utilities
  void startTimer() { m_timer.start(); }
  [[nodiscard]] qint64 elapsedMs() const { return m_timer.elapsed(); }

  // Environment management
  void setEnv(const char* name, const QString& value);
  void unsetEnv(const char* name);
  void withEnv(const char* name, const QString& value, 
               std::function<void()> fn);

private slots:
  void initTestCase() final { onTestCaseSetUp(); }
  void cleanupTestCase() final { onTestCaseTearDown(); }
  void init() final;
  void cleanup() final;

private:
  std::unique_ptr<QTemporaryDir> m_tempDir;
  QElapsedTimer m_timer;
  QMap<QString, QString> m_originalEnv;
};

/**
 * @brief Base class for network-dependent tests
 */
class NetworkTestBase : public TestBase {
  Q_OBJECT

protected:
  void onSetUp() override;
  void onTearDown() override;

  [[nodiscard]] MockHttpClient& mockHttp() { return *m_mockHttp; }
  
  // Convenience methods
  void queueJsonResponse(const QByteArray& json, int status = 200);
  void queueTwitchResponse(const QVariantList& data);
  void queueTwitchError(int status, const QString& message);
  void queueNetworkError(QNetworkReply::NetworkError error);

private:
  std::unique_ptr<MockHttpClient> m_mockHttp;
};

/**
 * @brief Base class for async tests requiring event loop
 */
class AsyncTestBase : public TestBase {
  Q_OBJECT

protected:
  // Wait for signal with timeout
  template<typename Signal>
  bool waitForSignal(QObject* sender, Signal signal, int timeoutMs = 5000);
  
  // Wait for condition with timeout
  bool waitFor(std::function<bool()> condition, int timeoutMs = 5000);
  
  // Process events
  void processEvents(int ms = 100);
};

/**
 * @brief Base class for database/cache tests
 */
class PersistenceTestBase : public TestBase {
  Q_OBJECT

protected:
  void onSetUp() override;
  void onTearDown() override;

  [[nodiscard]] QString testDbPath() const;
  [[nodiscard]] QString testCachePath() const;
  void clearTestData();
};

/**
 * @brief Base class for QML/UI tests
 */
class QmlTestBase : public AsyncTestBase {
  Q_OBJECT

protected:
  void onSetUp() override;
  void onTearDown() override;

  void loadQml(const QString& qmlSource);
  QQuickItem* findItem(const QString& objectName);
  
  // Interactions
  void click(QQuickItem* item);
  void type(QQuickItem* item, const QString& text);
  void scroll(QQuickItem* item, int delta);

private:
  std::unique_ptr<QQmlEngine> m_engine;
  std::unique_ptr<QQuickView> m_view;
};

} // namespace blueplayer::test
```

### 3.2 Test Categories via Traits

```cpp
// tests/base/TestTraits.hpp
#pragma once

#include <QtTest/QtTest>

namespace blueplayer::test::traits {

/**
 * @brief Marks a test as a smoke test (fast, critical path)
 */
#define SMOKE_TEST \
  private: void runAsSmoke() { QVERIFY(true); }

/**
 * @brief Marks a test as performance-sensitive
 */
#define PERF_TEST \
  private: void runAsPerf() { QVERIFY(true); }

/**
 * @brief Marks a test as requiring network
 */
#define NETWORK_TEST \
  private: void requiresNetwork() { QVERIFY(true); }

/**
 * @brief Marks a test as slow (>1s execution time)
 */
#define SLOW_TEST \
  private: void isSlow() { QVERIFY(true); }

/**
 * @brief Skip test if condition not met
 */
#define SKIP_IF(condition, message) \
  if (condition) QSKIP(message)

/**
 * @brief Timeout decorator for async tests
 */
#define TEST_TIMEOUT(ms) \
  QTimer::singleShot(ms, []() { QFAIL("Test timed out"); })

} // namespace blueplayer::test::traits
```

---

## 4. Fixture Factories Pattern

### 4.1 Generic Factory Interface

```cpp
// tests/fixtures/FixtureFactory.hpp
#pragma once

#include <memory>
#include <functional>

namespace blueplayer::test {

/**
 * @brief Generic factory for creating test fixtures
 */
template<typename T>
class FixtureFactory {
public:
  using Customizer = std::function<void(T&)>;
  
  static std::unique_ptr<T> create(Customizer customizer = nullptr) {
    auto fixture = std::make_unique<T>();
    if (customizer) {
      customizer(*fixture);
    }
    return fixture;
  }
  
  static T createValue(Customizer customizer = nullptr) {
    T fixture;
    if (customizer) {
      customizer(fixture);
    }
    return fixture;
  }
};

} // namespace blueplayer::test
```

### 4.2 Domain-Specific Factories

```cpp
// tests/fixtures/TwitchFixtures.hpp
#pragma once

#include "helpers/TwitchTestData.hpp"
#include <QVariantMap>
#include <QVariantList>
#include <random>

namespace blueplayer::test {

/**
 * @brief Factory for creating Twitch stream fixtures
 */
class StreamFixture {
public:
  StreamFixture& withUserName(const QString& name) {
    m_data["user_name"] = name;
    m_data["user_login"] = name.toLower();
    return *this;
  }
  
  StreamFixture& withTitle(const QString& title) {
    m_data["title"] = title;
    return *this;
  }
  
  StreamFixture& withViewers(int count) {
    m_data["viewer_count"] = count;
    return *this;
  }
  
  StreamFixture& withGame(const QString& gameName, const QString& gameId = "") {
    m_data["game_name"] = gameName;
    m_data["game_id"] = gameId.isEmpty() ? QString::number(qHash(gameName) % 100000) : gameId;
    return *this;
  }
  
  StreamFixture& asLive() {
    m_data["type"] = "live";
    return *this;
  }
  
  StreamFixture& asMature() {
    m_data["is_mature"] = true;
    return *this;
  }
  
  StreamFixture& withLanguage(const QString& lang) {
    m_data["language"] = lang;
    return *this;
  }
  
  StreamFixture& withThumbnail(const QString& url) {
    m_data["thumbnail_url"] = url;
    return *this;
  }
  
  [[nodiscard]] QVariantMap build() const {
    QVariantMap result = defaultStream();
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
      result[it.key()] = it.value();
    }
    return result;
  }
  
  static QVariantMap defaultStream() {
    static int idCounter = 1;
    QVariantMap stream;
    stream["id"] = QString::number(idCounter++);
    stream["user_id"] = QString::number(1000 + idCounter);
    stream["user_name"] = "DefaultStreamer";
    stream["user_login"] = "defaultstreamer";
    stream["game_id"] = "12345";
    stream["game_name"] = "Just Chatting";
    stream["type"] = "live";
    stream["title"] = "Default Stream Title";
    stream["viewer_count"] = 100;
    stream["started_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    stream["language"] = "en";
    stream["thumbnail_url"] = "https://example.com/thumb-{width}x{height}.jpg";
    stream["is_mature"] = false;
    return stream;
  }

private:
  QVariantMap m_data;
};

/**
 * @brief Factory for creating Twitch category fixtures
 */
class CategoryFixture {
public:
  CategoryFixture& withName(const QString& name) {
    m_name = name;
    return *this;
  }
  
  CategoryFixture& withId(const QString& id) {
    m_id = id;
    return *this;
  }
  
  [[nodiscard]] QVariantMap build() const {
    return TwitchTestData::createCategory(
      m_name.isEmpty() ? "Default Category" : m_name,
      m_id
    );
  }

private:
  QString m_name;
  QString m_id;
};

/**
 * @brief Factory for creating complete API response fixtures
 */
class ApiResponseFixture {
public:
  ApiResponseFixture& withStreams(int count) {
    for (int i = 0; i < count; ++i) {
      m_data.append(StreamFixture()
        .withUserName(QString("Streamer%1").arg(i + 1))
        .withViewers(100 * (i + 1))
        .build());
    }
    return *this;
  }
  
  ApiResponseFixture& withStream(const StreamFixture& stream) {
    m_data.append(stream.build());
    return *this;
  }
  
  ApiResponseFixture& withPagination(const QString& cursor) {
    m_cursor = cursor;
    return *this;
  }
  
  [[nodiscard]] QString buildJson() const {
    if (m_cursor.isEmpty()) {
      return TwitchTestData::createApiResponse(m_data);
    }
    return TwitchTestData::createPaginatedResponse(m_data, m_cursor);
  }
  
  [[nodiscard]] MockResponse buildMockResponse(int status = 200) const {
    return MockResponse::json(buildJson().toUtf8(), status);
  }

private:
  QVariantList m_data;
  QString m_cursor;
};

} // namespace blueplayer::test
```

---

## 5. Builder Pattern for Test Data

### 5.1 Fluent Builder Implementation

```cpp
// tests/builders/TestDataBuilder.hpp
#pragma once

#include <QVariantMap>
#include <QVariantList>
#include <QString>
#include <QDateTime>

namespace blueplayer::test {

/**
 * @brief Fluent builder for Twitch stream test data
 */
class StreamBuilder {
public:
  StreamBuilder() { reset(); }
  
  StreamBuilder& id(const QString& value) { m_stream["id"] = value; return *this; }
  StreamBuilder& userId(const QString& value) { m_stream["user_id"] = value; return *this; }
  StreamBuilder& userName(const QString& value) { 
    m_stream["user_name"] = value; 
    m_stream["user_login"] = value.toLower();
    return *this; 
  }
  StreamBuilder& title(const QString& value) { m_stream["title"] = value; return *this; }
  StreamBuilder& viewers(int value) { m_stream["viewer_count"] = value; return *this; }
  StreamBuilder& game(const QString& name) { m_stream["game_name"] = name; return *this; }
  StreamBuilder& gameId(const QString& id) { m_stream["game_id"] = id; return *this; }
  StreamBuilder& language(const QString& lang) { m_stream["language"] = lang; return *this; }
  StreamBuilder& mature(bool value = true) { m_stream["is_mature"] = value; return *this; }
  StreamBuilder& thumbnail(const QString& url) { m_stream["thumbnail_url"] = url; return *this; }
  StreamBuilder& startedAt(const QDateTime& dt) { 
    m_stream["started_at"] = dt.toString(Qt::ISODate); 
    return *this; 
  }
  
  QVariantMap build() {
    QVariantMap result = m_stream;
    reset();
    return result;
  }
  
  static StreamBuilder aStream() { return StreamBuilder(); }
  
private:
  void reset() {
    static int counter = 0;
    m_stream.clear();
    m_stream["id"] = QString::number(++counter);
    m_stream["user_id"] = QString::number(1000 + counter);
    m_stream["user_name"] = "TestStreamer";
    m_stream["user_login"] = "teststreamer";
    m_stream["game_id"] = "12345";
    m_stream["game_name"] = "Just Chatting";
    m_stream["type"] = "live";
    m_stream["title"] = "Test Stream";
    m_stream["viewer_count"] = 100;
    m_stream["started_at"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    m_stream["language"] = "en";
    m_stream["thumbnail_url"] = "https://cdn.twitch.tv/thumb-{width}x{height}.jpg";
    m_stream["is_mature"] = false;
  }
  
  QVariantMap m_stream;
};

/**
 * @brief Fluent builder for Twitch video/VOD test data
 */
class VideoBuilder {
public:
  VideoBuilder() { reset(); }
  
  VideoBuilder& id(const QString& value) { m_video["id"] = value; return *this; }
  VideoBuilder& title(const QString& value) { m_video["title"] = value; return *this; }
  VideoBuilder& userName(const QString& value) { 
    m_video["user_name"] = value;
    m_video["user_login"] = value.toLower();
    return *this; 
  }
  VideoBuilder& views(int value) { m_video["view_count"] = value; return *this; }
  VideoBuilder& duration(const QString& value) { m_video["duration"] = value; return *this; }
  VideoBuilder& durationSeconds(int seconds) {
    int h = seconds / 3600;
    int m = (seconds % 3600) / 60;
    int s = seconds % 60;
    m_video["duration"] = QString("%1h%2m%3s").arg(h).arg(m).arg(s);
    return *this;
  }
  VideoBuilder& type(const QString& value) { m_video["type"] = value; return *this; }
  VideoBuilder& asArchive() { return type("archive"); }
  VideoBuilder& asHighlight() { return type("highlight"); }
  VideoBuilder& asUpload() { return type("upload"); }
  
  QVariantMap build() {
    QVariantMap result = m_video;
    reset();
    return result;
  }
  
  static VideoBuilder aVideo() { return VideoBuilder(); }
  
private:
  void reset();
  QVariantMap m_video;
};

/**
 * @brief Builder for API error responses
 */
class ErrorResponseBuilder {
public:
  ErrorResponseBuilder& status(int code) { m_status = code; return *this; }
  ErrorResponseBuilder& error(const QString& err) { m_error = err; return *this; }
  ErrorResponseBuilder& message(const QString& msg) { m_message = msg; return *this; }
  
  // Common error presets
  ErrorResponseBuilder& unauthorized() { 
    return status(401).error("Unauthorized").message("Invalid access token"); 
  }
  ErrorResponseBuilder& notFound() { 
    return status(404).error("Not Found").message("Resource not found"); 
  }
  ErrorResponseBuilder& rateLimited() { 
    return status(429).error("Too Many Requests").message("Rate limit exceeded"); 
  }
  ErrorResponseBuilder& serverError() { 
    return status(500).error("Internal Server Error").message("An error occurred"); 
  }
  
  QString buildJson() const {
    return TwitchTestData::createErrorResponse(m_error, m_status, m_message);
  }
  
  MockResponse buildMockResponse() const {
    return MockResponse::json(buildJson().toUtf8(), m_status);
  }
  
private:
  int m_status = 400;
  QString m_error = "Bad Request";
  QString m_message = "Invalid request";
};

// Convenience functions
inline StreamBuilder aStream() { return StreamBuilder::aStream(); }
inline VideoBuilder aVideo() { return VideoBuilder::aVideo(); }
inline ErrorResponseBuilder anError() { return ErrorResponseBuilder(); }

} // namespace blueplayer::test
```

---

## 6. Page Object Pattern for UI Tests

### 6.1 Base Page Object

```cpp
// tests/ui/pages/PageObject.hpp
#pragma once

#include <QQuickItem>
#include <QQuickView>
#include <QString>
#include <QTest>
#include <memory>

namespace blueplayer::test::ui {

/**
 * @brief Base class for Page Objects in UI testing
 * 
 * Implements the Page Object pattern for QML UI testing.
 * Each page/view has a corresponding PageObject that encapsulates
 * all UI interactions.
 */
class PageObject {
public:
  explicit PageObject(QQuickView* view) : m_view(view) {}
  virtual ~PageObject() = default;
  
  // Navigation
  [[nodiscard]] virtual bool isVisible() const = 0;
  virtual void waitUntilLoaded(int timeoutMs = 5000);
  
protected:
  // Element finders
  [[nodiscard]] QQuickItem* findByObjectName(const QString& name) const;
  [[nodiscard]] QQuickItem* findByProperty(const QString& property, const QVariant& value) const;
  [[nodiscard]] QList<QQuickItem*> findAllByObjectName(const QString& name) const;
  
  // Interactions
  void click(QQuickItem* item);
  void click(const QString& objectName);
  void doubleClick(QQuickItem* item);
  void type(QQuickItem* item, const QString& text);
  void type(const QString& objectName, const QString& text);
  void clearAndType(QQuickItem* item, const QString& text);
  void pressKey(Qt::Key key, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
  void scroll(QQuickItem* item, int deltaY);
  
  // Property access
  [[nodiscard]] QVariant getProperty(QQuickItem* item, const QString& property) const;
  [[nodiscard]] QVariant getProperty(const QString& objectName, const QString& property) const;
  void setProperty(QQuickItem* item, const QString& property, const QVariant& value);
  
  // Waits
  bool waitForProperty(QQuickItem* item, const QString& property, 
                       const QVariant& expectedValue, int timeoutMs = 5000);
  bool waitForVisible(const QString& objectName, int timeoutMs = 5000);
  bool waitForHidden(const QString& objectName, int timeoutMs = 5000);
  
  // View access
  [[nodiscard]] QQuickView* view() const { return m_view; }
  [[nodiscard]] QQuickItem* rootItem() const { return m_view->rootObject(); }

private:
  QQuickView* m_view;
};

} // namespace blueplayer::test::ui
```

### 6.2 Concrete Page Objects

```cpp
// tests/ui/pages/HomePageObject.hpp
#pragma once

#include "PageObject.hpp"

namespace blueplayer::test::ui {

/**
 * @brief Page Object for HomeView.qml
 */
class HomePageObject : public PageObject {
public:
  using PageObject::PageObject;
  
  // Visibility
  [[nodiscard]] bool isVisible() const override;
  
  // Sections
  [[nodiscard]] bool hasFollowedStreamsSection() const;
  [[nodiscard]] bool hasRecommendedStreamsSection() const;
  [[nodiscard]] bool hasCategoriesSection() const;
  [[nodiscard]] int streamCardCount() const;
  [[nodiscard]] int categoryCardCount() const;
  
  // Interactions
  void clickStreamCard(int index);
  void clickCategoryCard(int index);
  void scrollToSection(const QString& sectionName);
  void refreshContent();
  
  // Stream cards
  [[nodiscard]] QString streamCardTitle(int index) const;
  [[nodiscard]] QString streamCardStreamer(int index) const;
  [[nodiscard]] int streamCardViewers(int index) const;
  
  // Search
  void openSearch();
  void search(const QString& query);
  void clearSearch();
  
  // Navigation
  PlayerPageObject clickFirstStream();
  CategoryPageObject clickFirstCategory();
};

/**
 * @brief Page Object for PlayerView.qml
 */
class PlayerPageObject : public PageObject {
public:
  using PageObject::PageObject;
  
  [[nodiscard]] bool isVisible() const override;
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] bool isPaused() const;
  [[nodiscard]] bool isBuffering() const;
  
  // Controls
  void play();
  void pause();
  void togglePlayPause();
  void setVolume(int percent);
  void mute();
  void unmute();
  void seekTo(double position);
  void seekForward(int seconds = 10);
  void seekBackward(int seconds = 10);
  void toggleFullscreen();
  
  // Quality
  void openQualitySelector();
  void selectQuality(const QString& quality);
  [[nodiscard]] QString currentQuality() const;
  [[nodiscard]] QStringList availableQualities() const;
  
  // Chat
  void toggleChat();
  [[nodiscard]] bool isChatVisible() const;
  
  // Back navigation
  HomePageObject goBack();
};

/**
 * @brief Page Object for LoginView.qml
 */
class LoginPageObject : public PageObject {
public:
  using PageObject::PageObject;
  
  [[nodiscard]] bool isVisible() const override;
  [[nodiscard]] bool isLoading() const;
  [[nodiscard]] bool hasError() const;
  [[nodiscard]] QString errorMessage() const;
  
  // Actions
  void clickLoginButton();
  void waitForAuthComplete(int timeoutMs = 30000);
  
  // Navigation
  HomePageObject afterSuccessfulLogin();
};

} // namespace blueplayer::test::ui
```

---

## 7. Test Categorization

### 7.1 Test Tags and Categories

```cpp
// tests/categories/TestCategories.hpp
#pragma once

namespace blueplayer::test {

/**
 * Test categories for filtering and CI/CD pipelines
 */
enum class TestCategory {
  Smoke,        // Fast, critical path tests (<100ms each)
  Unit,         // Standard unit tests
  Integration,  // Tests with real dependencies
  Performance,  // Benchmark tests
  Chaos,        // Fault injection tests
  E2E,          // End-to-end UI tests
  Regression    // Full regression suite
};

/**
 * CTest labels mapping (used in CMakeLists.txt)
 */
// set_tests_properties(test_name PROPERTIES LABELS "smoke;unit")

} // namespace blueplayer::test
```

### 7.2 CMake Test Organization

```cmake
# tests/CMakeLists.txt additions

# Define test categories
set(SMOKE_TESTS
  test_config
  test_error
  test_result
  test_input_validator
)

set(UNIT_TESTS
  test_twitch_api_client
  test_twitch_auth_manager
  test_twitch_service
  test_cache_manager
  test_http_client
  test_api_client_base
  test_network_cache
  test_secure_storage
  test_state_machine_live_replay
  test_home_view_model
  test_cache_manager_view_model
  test_logger
  test_file_logger
  test_curl_http_client
  test_twitch_chat_client
  test_hls_ad_filter
  test_watch_history
)

set(INTEGRATION_TESTS
  test_twitch_integration
)

set(PERFORMANCE_TESTS
  # Future: test_network_perf
  # Future: test_cache_perf
)

set(CHAOS_TESTS
  # Future: test_network_chaos
  # Future: test_auth_chaos
)

set(E2E_TESTS
  # Future: test_home_e2e
  # Future: test_player_e2e
)

# Apply labels
foreach(test ${SMOKE_TESTS})
  set_tests_properties(${test} PROPERTIES LABELS "smoke;fast")
endforeach()

foreach(test ${UNIT_TESTS})
  set_tests_properties(${test} PROPERTIES LABELS "unit")
endforeach()

foreach(test ${INTEGRATION_TESTS})
  set_tests_properties(${test} PROPERTIES LABELS "integration;slow")
endforeach()

# Custom targets for running test categories
add_custom_target(test-smoke
  COMMAND ${CMAKE_CTEST_COMMAND} -L smoke --output-on-failure
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(test-unit
  COMMAND ${CMAKE_CTEST_COMMAND} -L unit --output-on-failure
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(test-integration
  COMMAND ${CMAKE_CTEST_COMMAND} -L integration --output-on-failure
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(test-all
  COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
```

### 7.3 Smoke Tests (Critical Path)

```cpp
// tests/smoke/SmokeTests.cpp
/**
 * @brief Smoke tests for critical application paths
 * 
 * These tests should:
 * - Execute in < 100ms total
 * - Cover critical initialization paths
 * - Verify basic functionality works
 * - Run on every commit
 */

#include "base/TestBase.hpp"
#include "core/Config.hpp"
#include "core/Error.hpp"
#include "core/Result.hpp"

namespace blueplayer::test {

class SmokeTests : public TestBase {
  Q_OBJECT

private slots:
  // Config smoke tests
  void testConfigSingletonExists() {
    startTimer();
    Config& config = Config::instance();
    QVERIFY(&config != nullptr);
    QVERIFY(elapsedMs() < 10);
  }
  
  // Error handling smoke tests
  void testErrorCanBeCreated() {
    Error error(ErrorCode::Unknown);
    QVERIFY(!error.hasError() || error.code() != ErrorCode::Unknown);
  }
  
  // Result type smoke tests
  void testResultSuccessWorks() {
    auto result = Result<int>::success(42);
    QVERIFY(result.isSuccess());
    QCOMPARE(result.value(), 42);
  }
  
  void testResultFailureWorks() {
    auto result = Result<int>::failure(Error(ErrorCode::NetworkError));
    QVERIFY(result.isFailure());
  }
};

} // namespace blueplayer::test

QTEST_MAIN(blueplayer::test::SmokeTests)
#include "SmokeTests.moc"
```

---

## 8. Performance Tests (Benchmarks)

### 8.1 Benchmark Framework

```cpp
// tests/performance/Benchmark.hpp
#pragma once

#include <QtTest/QtTest>
#include <QElapsedTimer>
#include <functional>
#include <vector>
#include <algorithm>
#include <numeric>

namespace blueplayer::test {

/**
 * @brief Benchmark results container
 */
struct BenchmarkResult {
  QString name;
  int iterations;
  qint64 totalMs;
  qint64 minMs;
  qint64 maxMs;
  double avgMs;
  double medianMs;
  double stdDevMs;
  
  void print() const {
    qDebug() << "Benchmark:" << name;
    qDebug() << "  Iterations:" << iterations;
    qDebug() << "  Total:" << totalMs << "ms";
    qDebug() << "  Min:" << minMs << "ms";
    qDebug() << "  Max:" << maxMs << "ms";
    qDebug() << "  Avg:" << avgMs << "ms";
    qDebug() << "  Median:" << medianMs << "ms";
    qDebug() << "  StdDev:" << stdDevMs << "ms";
  }
};

/**
 * @brief Benchmark runner utility
 */
class Benchmark {
public:
  static BenchmarkResult run(const QString& name, 
                             std::function<void()> fn,
                             int iterations = 100,
                             int warmupIterations = 10) {
    // Warmup
    for (int i = 0; i < warmupIterations; ++i) {
      fn();
    }
    
    // Actual measurements
    std::vector<qint64> times;
    times.reserve(iterations);
    
    QElapsedTimer timer;
    for (int i = 0; i < iterations; ++i) {
      timer.start();
      fn();
      times.push_back(timer.elapsed());
    }
    
    // Calculate statistics
    BenchmarkResult result;
    result.name = name;
    result.iterations = iterations;
    result.totalMs = std::accumulate(times.begin(), times.end(), 0LL);
    result.minMs = *std::min_element(times.begin(), times.end());
    result.maxMs = *std::max_element(times.begin(), times.end());
    result.avgMs = static_cast<double>(result.totalMs) / iterations;
    
    std::sort(times.begin(), times.end());
    result.medianMs = times[iterations / 2];
    
    double variance = 0;
    for (qint64 t : times) {
      variance += (t - result.avgMs) * (t - result.avgMs);
    }
    result.stdDevMs = std::sqrt(variance / iterations);
    
    return result;
  }
  
  // Assertion helpers
  static void assertMaxTime(const BenchmarkResult& result, qint64 maxMs) {
    QVERIFY2(result.avgMs <= maxMs, 
             qPrintable(QString("Benchmark %1 exceeded max time: %2ms > %3ms")
                       .arg(result.name)
                       .arg(result.avgMs)
                       .arg(maxMs)));
  }
};

} // namespace blueplayer::test
```

### 8.2 Performance Test Examples

```cpp
// tests/performance/TestNetworkPerformance.cpp
#include "base/NetworkTestBase.hpp"
#include "performance/Benchmark.hpp"
#include "api/twitch/TwitchApiClient.hpp"

namespace blueplayer::test {

class TestNetworkPerformance : public NetworkTestBase {
  Q_OBJECT

private slots:
  void benchmarkJsonParsing() {
    // Create large JSON response
    QString largeJson = createLargeStreamResponse(1000);
    
    auto result = Benchmark::run("JSON Parsing (1000 streams)", [&]() {
      QJsonDocument::fromJson(largeJson.toUtf8());
    });
    
    result.print();
    Benchmark::assertMaxTime(result, 50); // Max 50ms average
  }
  
  void benchmarkStreamTransformation() {
    QVariantList streams;
    for (int i = 0; i < 100; ++i) {
      streams.append(StreamBuilder::aStream()
                    .userName(QString("Streamer%1").arg(i))
                    .viewers(i * 100)
                    .build());
    }
    
    auto result = Benchmark::run("Stream Transformation (100 streams)", [&]() {
      // Transform streams through ViewModel
      HomeViewModel vm;
      vm.transformTwitchStreams(streams);
    });
    
    result.print();
    Benchmark::assertMaxTime(result, 10); // Max 10ms average
  }
  
private:
  QString createLargeStreamResponse(int count);
};

} // namespace blueplayer::test
```

---

## 9. Chaos Tests (Fault Injection)

### 9.1 Chaos Testing Framework

```cpp
// tests/chaos/ChaosEngine.hpp
#pragma once

#include <functional>
#include <random>
#include <QNetworkReply>

namespace blueplayer::test {

/**
 * @brief Chaos engineering utilities for fault injection testing
 */
class ChaosEngine {
public:
  enum class FaultType {
    NetworkTimeout,
    NetworkError,
    SlowResponse,
    PartialResponse,
    InvalidJson,
    EmptyResponse,
    ServerError,
    RateLimitExceeded,
    AuthFailure
  };
  
  /**
   * @brief Injects a random fault based on probability
   */
  static bool shouldInjectFault(double probability = 0.1) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) < probability;
  }
  
  /**
   * @brief Gets a random fault type
   */
  static FaultType randomFault() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 8);
    return static_cast<FaultType>(dis(gen));
  }
  
  /**
   * @brief Creates a MockResponse for a given fault type
   */
  static MockResponse createFaultyResponse(FaultType fault) {
    switch (fault) {
      case FaultType::NetworkTimeout:
        return MockResponse::timeout();
      case FaultType::NetworkError:
        return MockResponse::connectionRefused();
      case FaultType::SlowResponse:
        return MockResponse::json("{}").withDelay(5000);
      case FaultType::PartialResponse:
        return MockResponse::json("{\"data\":");  // Incomplete JSON
      case FaultType::InvalidJson:
        return MockResponse::json("{not valid json}");
      case FaultType::EmptyResponse:
        return MockResponse::json("");
      case FaultType::ServerError:
        return MockResponse::serverError();
      case FaultType::RateLimitExceeded:
        return ErrorResponseBuilder().rateLimited().buildMockResponse();
      case FaultType::AuthFailure:
        return MockResponse::unauthorized();
    }
    return MockResponse::json("{}");
  }
};

/**
 * @brief Mock HTTP client that randomly injects faults
 */
class ChaoticHttpClient : public MockHttpClient {
public:
  explicit ChaoticHttpClient(double faultProbability = 0.1)
    : m_faultProbability(faultProbability) {}
  
  void setFaultProbability(double p) { m_faultProbability = p; }
  
  MockNetworkReply* get(const QUrl& url, 
                        const QHash<QString, QString>& headers = {}) override {
    if (ChaosEngine::shouldInjectFault(m_faultProbability)) {
      queueResponse(ChaosEngine::createFaultyResponse(ChaosEngine::randomFault()));
    }
    return MockHttpClient::get(url, headers);
  }
  
private:
  double m_faultProbability;
};

} // namespace blueplayer::test
```

### 9.2 Chaos Test Examples

```cpp
// tests/chaos/TestNetworkChaos.cpp
#include "base/NetworkTestBase.hpp"
#include "chaos/ChaosEngine.hpp"
#include "api/twitch/TwitchService.hpp"

namespace blueplayer::test {

class TestNetworkChaos : public NetworkTestBase {
  Q_OBJECT

private slots:
  void testServiceSurvivesRandomFaults() {
    // Create service with chaotic client
    auto chaoticClient = std::make_unique<ChaoticHttpClient>(0.3);
    // Inject client... (would need DI support)
    
    TwitchService service;
    QSignalSpy errorSpy(&service, &TwitchService::errorOccurred);
    
    // Hammer the service with requests
    for (int i = 0; i < 100; ++i) {
      service.refreshStreams();
      service.refreshCategories();
      service.search("test");
      
      // Process events
      QCoreApplication::processEvents();
    }
    
    // Service should not crash
    QVERIFY(&service != nullptr);
    
    // Some errors expected
    qDebug() << "Errors encountered:" << errorSpy.count();
  }
  
  void testGracefulDegradationOnTimeout() {
    queueNetworkError(QNetworkReply::TimeoutError);
    
    TwitchService service;
    QSignalSpy errorSpy(&service, &TwitchService::errorOccurred);
    
    service.refreshStreams();
    processEvents(1000);
    
    // Should emit error
    QVERIFY(errorSpy.count() > 0);
    // Data should be empty (not crash)
    QVERIFY(service.streams().isEmpty());
  }
  
  void testRecoveryAfterNetworkRestore() {
    // First request fails
    queueNetworkError(QNetworkReply::TimeoutError);
    // Second request succeeds
    queueTwitchResponse(TwitchTestData::createStreams(5));
    
    TwitchService service;
    
    service.refreshStreams();
    processEvents(500);
    QVERIFY(service.streams().isEmpty());
    
    service.refreshStreams();
    processEvents(500);
    QVERIFY(!service.streams().isEmpty());
  }
};

} // namespace blueplayer::test
```

---

## 10. Test Isolation Strategy

### 10.1 Dependency Injection Pattern

```cpp
// src/core/DependencyContainer.hpp
#pragma once

#include <memory>
#include <functional>
#include <typeindex>
#include <unordered_map>

namespace blueplayer::core {

/**
 * @brief Lightweight dependency injection container
 */
class DependencyContainer {
public:
  static DependencyContainer& instance() {
    static DependencyContainer container;
    return container;
  }
  
  template<typename Interface, typename Implementation>
  void registerType() {
    m_factories[typeid(Interface)] = []() {
      return std::make_shared<Implementation>();
    };
  }
  
  template<typename Interface>
  void registerInstance(std::shared_ptr<Interface> instance) {
    m_instances[typeid(Interface)] = instance;
  }
  
  template<typename Interface>
  void registerFactory(std::function<std::shared_ptr<Interface>()> factory) {
    m_factories[typeid(Interface)] = [factory]() {
      return factory();
    };
  }
  
  template<typename Interface>
  std::shared_ptr<Interface> resolve() {
    // Check for registered instance first
    auto instIt = m_instances.find(typeid(Interface));
    if (instIt != m_instances.end()) {
      return std::static_pointer_cast<Interface>(instIt->second);
    }
    
    // Fall back to factory
    auto factIt = m_factories.find(typeid(Interface));
    if (factIt != m_factories.end()) {
      return std::static_pointer_cast<Interface>(factIt->second());
    }
    
    return nullptr;
  }
  
  void reset() {
    m_instances.clear();
    m_factories.clear();
  }

private:
  std::unordered_map<std::type_index, std::shared_ptr<void>> m_instances;
  std::unordered_map<std::type_index, std::function<std::shared_ptr<void>()>> m_factories;
};

} // namespace blueplayer::core
```

### 10.2 Mock Boundaries

```
+--------------------------------------------------+
|                    APPLICATION                    |
+--------------------------------------------------+
|                                                   |
|   +------------+     +------------+               |
|   | ViewModels |     |  Services  |               |
|   +-----+------+     +------+-----+               |
|         |                   |                     |
|   +-----v-------------------v-----+               |
|   |         API Clients           | <- MOCK HERE  |
|   +---------------+---------------+               |
|                   |                               |
+-------------------v-------------------------------+
                    |
          +---------v---------+
          |  Network Layer    | <- MOCK HERE
          +---------+---------+
                    |
          +---------v---------+
          |  External APIs    | <- REAL IN INTEGRATION
          +-------------------+

MOCK BOUNDARIES:
1. MockHttpClient      - Replace network layer
2. MockTwitchApiClient - Replace API client
3. MockCacheManager    - Replace file system
4. MockSecureStorage   - Replace keychain
```

### 10.3 Test Database/Cache Strategy

```cpp
// tests/fixtures/TestEnvironment.hpp
#pragma once

#include <QTemporaryDir>
#include <QStandardPaths>
#include <memory>

namespace blueplayer::test {

/**
 * @brief Manages isolated test environment
 */
class TestEnvironment {
public:
  static TestEnvironment& instance() {
    static TestEnvironment env;
    return env;
  }
  
  void setUp() {
    m_tempDir = std::make_unique<QTemporaryDir>();
    
    // Override standard paths for testing
    QStandardPaths::setTestModeEnabled(true);
    
    // Set environment variables
    qputenv("BLUEPLAYER_CONFIG_DIR", configPath().toUtf8());
    qputenv("BLUEPLAYER_CACHE_DIR", cachePath().toUtf8());
    qputenv("BLUEPLAYER_DATA_DIR", dataPath().toUtf8());
  }
  
  void tearDown() {
    QStandardPaths::setTestModeEnabled(false);
    m_tempDir.reset();
    
    qunsetenv("BLUEPLAYER_CONFIG_DIR");
    qunsetenv("BLUEPLAYER_CACHE_DIR");
    qunsetenv("BLUEPLAYER_DATA_DIR");
  }
  
  [[nodiscard]] QString configPath() const { 
    return m_tempDir->filePath("config"); 
  }
  [[nodiscard]] QString cachePath() const { 
    return m_tempDir->filePath("cache"); 
  }
  [[nodiscard]] QString dataPath() const { 
    return m_tempDir->filePath("data"); 
  }
  
  void clearAll() {
    QDir(configPath()).removeRecursively();
    QDir(cachePath()).removeRecursively();
    QDir(dataPath()).removeRecursively();
  }

private:
  std::unique_ptr<QTemporaryDir> m_tempDir;
};

} // namespace blueplayer::test
```

---

## 11. Naming and Organization Conventions

### 11.1 Test File Naming

```
tests/
  base/                          # Base classes
    TestBase.hpp
    TestTraits.hpp
    
  builders/                      # Builder pattern
    TestDataBuilder.hpp
    
  fixtures/                      # Fixture factories
    TwitchFixtures.hpp
    FixtureFactory.hpp
    TestEnvironment.hpp
    
  mocks/                         # Mock implementations
    MockHttpClient.hpp
    MockNetworkReply.hpp
    MockCacheManager.hpp
    MockSecureStorage.hpp
    
  helpers/                       # Test utilities
    TwitchTestData.hpp
    TestHelpers.hpp
    
  ui/pages/                      # Page Objects
    PageObject.hpp
    HomePageObject.hpp
    PlayerPageObject.hpp
    
  smoke/                         # Smoke tests
    SmokeTests.cpp
    
  performance/                   # Performance tests
    Benchmark.hpp
    TestNetworkPerformance.cpp
    
  chaos/                         # Chaos tests
    ChaosEngine.hpp
    TestNetworkChaos.cpp
    
  # Domain tests (mirroring src/ structure)
  api/twitch/
    TestTwitchApiClient.cpp
    TestTwitchAuthManager.cpp
    TestTwitchService.cpp
    
  core/
    TestConfig.cpp
    TestError.cpp
    TestResult.cpp
    network/
      TestHttpClient.cpp
      TestApiClientBase.cpp
      
  media/
    TestHlsAdFilter.cpp
    
  chat/
    TestTwitchChatClient.cpp
    
  ui/
    TestHomeViewModel.cpp
    TestPlayerE2E.cpp           # E2E tests
    
  integration/
    TestTwitchFlow.cpp
```

### 11.2 Test Method Naming Convention

```cpp
/**
 * Pattern: test<Method>_<Scenario>_<ExpectedBehavior>
 * 
 * Examples:
 * - testParseJson_ValidResponse_ReturnsData
 * - testParseJson_MalformedInput_ReturnsError
 * - testLogin_WithValidCredentials_EmitsAuthenticatedSignal
 * - testRefreshStreams_WhenNotAuthenticated_ReturnsEmpty
 */

class TestTwitchService : public QObject {
  Q_OBJECT

private slots:
  // Good naming
  void testRefreshStreams_WhenAuthenticated_EmitsStreamsChanged();
  void testRefreshStreams_WhenNotAuthenticated_DoesNotCrash();
  void testSearch_WithEmptyQuery_ReturnsEmpty();
  void testPlayStream_WithInvalidIndex_EmitsError();
  
  // Alternative: Given-When-Then style
  void givenAuthenticated_whenRefreshStreams_thenStreamsChanged();
  void givenNotAuthenticated_whenRefreshStreams_thenEmptyResult();
};
```

### 11.3 Test Class Organization

```cpp
class TestTwitchApiClient : public NetworkTestBase {
  Q_OBJECT

private slots:
  // === Lifecycle (inherited from base) ===
  // init(), cleanup(), initTestCase(), cleanupTestCase()
  
  // === Section 1: Initialization Tests ===
  void testConstructor();
  void testSetAccessToken();
  void testSetAccessToken_Empty();
  void testSetAccessToken_Null();
  
  // === Section 2: Core Functionality Tests ===
  void testListStreams_Success();
  void testListStreams_Empty();
  void testListStreams_WithPagination();
  
  // === Section 3: Error Handling Tests ===
  void testListStreams_NetworkError();
  void testListStreams_Unauthorized();
  void testListStreams_RateLimited();
  
  // === Section 4: Edge Cases ===
  void testListStreams_NegativeLimit();
  void testListStreams_ZeroLimit();
  void testListStreams_LargeLimit();
  
  // === Section 5: Signal Tests ===
  void testStreamsReadySignal();
  void testErrorSignal();
  void testTokenInvalidatedSignal();
  
  // === Section 6: Data-Driven Tests ===
  void testErrorCodes_data();
  void testErrorCodes();

private:
  // Helper methods
  void queueStreamResponse(int count);
  void verifyStreamData(const QVariantList& streams);
};
```

---

## 12. Implementation Roadmap

### Phase 1: Foundation (Week 1-2)
- [ ] Create `tests/base/` directory with base classes
- [ ] Implement `TestBase`, `NetworkTestBase`, `AsyncTestBase`
- [ ] Create test environment isolation
- [ ] Add CMake infrastructure for test categories

### Phase 2: Fixtures & Builders (Week 2-3)
- [ ] Implement builder pattern classes
- [ ] Create fixture factories
- [ ] Refactor existing tests to use builders

### Phase 3: UI Testing (Week 3-4)
- [ ] Implement Page Object base class
- [ ] Create page objects for main views
- [ ] Add first E2E tests

### Phase 4: Performance & Chaos (Week 4-5)
- [ ] Implement benchmark framework
- [ ] Add performance tests for critical paths
- [ ] Implement chaos engine
- [ ] Add fault injection tests

### Phase 5: CI/CD Integration (Week 5-6)
- [ ] Configure smoke tests for every commit
- [ ] Configure nightly full regression
- [ ] Add performance regression detection
- [ ] Create test reporting dashboard

---

## 13. Summary Metrics

| Metric | Current | Target |
|--------|---------|--------|
| Unit Test Coverage | ~60% | 85% |
| Integration Test Coverage | ~10% | 50% |
| E2E Test Coverage | 0% | 20% |
| Smoke Test Execution Time | N/A | <5s |
| Full Suite Execution Time | ~30s | <60s |
| Flaky Test Rate | Unknown | <1% |
| Test/Code Ratio | 0.4:1 | 1:1 |

---

## Appendix A: Qt Test Best Practices Checklist

- [x] Use `QCOMPARE` instead of `QVERIFY` for value comparisons
- [x] Use `QSignalSpy` for async signal testing
- [ ] Use data-driven tests with `_data()` suffix
- [ ] Use `QBENCHMARK` for performance testing
- [ ] Avoid `QTest::qWait()` - use signal-based waiting
- [x] Clean up resources in `cleanup()` not destructor
- [x] Use `QTemporaryDir` for file-based tests
- [ ] Use `QStandardPaths::setTestModeEnabled(true)`

---

## Appendix B: References

1. Google Testing Blog - Test Pyramid
2. Microsoft Testing Guidelines for C++
3. Qt Test Framework Documentation
4. Martin Fowler - Page Object Pattern
5. Netflix Chaos Engineering Principles
