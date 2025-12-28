#pragma once

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QFile>

// Forward declarations
namespace blueplayer::test::e2e {
class MockTwitchServer;
class MockHlsServer;
}

namespace blueplayer::test::e2e {

/**
 * @brief Base class for E2E tests providing mock server infrastructure
 *
 * Manages MockTwitchServer and MockHlsServer lifecycle and provides
 * convenient methods for setting up test fixtures.
 *
 * Usage:
 * @code
 *   class MyE2ETest : public E2ETestBase {
 *   protected:
 *     void onSetUp() override {
 *       loadDefaultFixtures();
 *     }
 *   };
 * @endcode
 */
class E2ETestBase : public QObject {
  Q_OBJECT

public:
  explicit E2ETestBase(QObject* parent = nullptr);
  ~E2ETestBase() override;

  // ===== Server Access =====

  /**
   * @brief Returns the mock Twitch API server
   */
  [[nodiscard]] MockTwitchServer* twitchServer() const;

  /**
   * @brief Returns the mock HLS server
   */
  [[nodiscard]] MockHlsServer* hlsServer() const;

  /**
   * @brief Returns the base URL for mock Twitch API
   */
  [[nodiscard]] QString twitchApiUrl() const;

  /**
   * @brief Returns the base URL for mock HLS server
   */
  [[nodiscard]] QString hlsServerUrl() const;

  // ===== Fixture Loading =====

  /**
   * @brief Loads all default fixtures from the fixtures directory
   */
  void loadDefaultFixtures();

  /**
   * @brief Loads a JSON fixture file
   * @param filename Filename relative to fixtures directory
   * @return The JSON document, or empty document on error
   */
  [[nodiscard]] QJsonDocument loadFixture(const QString& filename) const;

  /**
   * @brief Loads streams fixture and configures the mock server
   */
  void loadStreamsFixture(const QString& filename = "streams.json");

  /**
   * @brief Loads users fixture and configures the mock server
   */
  void loadUsersFixture(const QString& filename = "users.json");

  /**
   * @brief Loads auth token fixture
   */
  void loadAuthFixture(const QString& filename = "auth_token.json");

  // ===== Environment Setup =====

  /**
   * @brief Sets up environment variables for test mode
   */
  static void setupTestEnvironment(const QString& twitchApiUrl,
                                   const QString& hlsServerUrl,
                                   const QString& authToken);

  /**
   * @brief Clears test environment variables
   */
  static void clearTestEnvironment();

  /**
   * @brief Returns the path to the fixtures directory
   */
  [[nodiscard]] static QString fixturesPath();

protected:
  /**
   * @brief Called before each test - override for custom setup
   */
  virtual void onSetUp() {}

  /**
   * @brief Called after each test - override for custom teardown
   */
  virtual void onTearDown() {}

protected Q_SLOTS:
  /**
   * @brief Qt Test framework hook - starts servers
   */
  void initTestCase();

  /**
   * @brief Qt Test framework hook - stops servers
   */
  void cleanupTestCase();

  /**
   * @brief Qt Test framework hook - per-test setup
   */
  void init();

  /**
   * @brief Qt Test framework hook - per-test cleanup
   */
  void cleanup();

private:
  MockTwitchServer* m_twitchServer = nullptr;
  MockHlsServer* m_hlsServer = nullptr;
  QString m_authToken;
};

} // namespace blueplayer::test::e2e
