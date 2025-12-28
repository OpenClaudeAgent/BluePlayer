# BluePlayer - Architecture des tests

## Résumé

Ce document présente une architecture de tests de classe mondiale pour BluePlayer, intégrant les meilleures pratiques de Google, Microsoft et les directives de test Qt.

---

## 1. Analyse de l'état actuel

### Points forts
- Framework Qt Test correctement intégré
- Infrastructure de mock complète (`MockHttpClient`, `MockNetworkReply`)
- Bonnes factories de données de test (`TwitchTestData`)
- Exécutables de test séparés par module
- Support de la couverture de code activé
- Tests data-driven utilisés dans `TestTwitchService`, `TestError`

### Axes d'amélioration
- Pas de hiérarchie de classes de test de base
- Code de setup/teardown dupliqué
- Pas de catégorisation des tests (smoke, régression, performance)
- Pattern Page Object manquant pour les tests UI
- Pas de tests de chaos/injection de fautes
- Pas de benchmarks de performance
- Conventions de nommage incohérentes
- Injection de dépendances limitée

---

## 2. Implémentation de la pyramide des tests

### Ratios de la pyramide des tests Google
```
         /\
        /  \   Tests E2E (5%)
       /----\  Tests d'intégration (15%)
      /------\ Tests unitaires (80%)
     /________\
```

### Distribution actuelle vs. cible

| Type de test     | Actuel | Cible  | Écart      |
|------------------|--------|--------|------------|
| Tests unitaires  | ~95%   | 80%    | Supérieur  |
| Intégration      | ~5%    | 15%    | Inférieur  |
| E2E/UI           | ~0%    | 5%     | Inférieur  |

---

## 3. Hiérarchie des classes de test de base

### 3.1 Classes de base principales

```cpp
// tests/base/TestBase.hpp
#pragma once

#include <QtTest/QtTest>
#include <QObject>
#include <QTemporaryDir>
#include <memory>

namespace blueplayer::test {

/**
 * @brief Classe de base abstraite pour tous les tests
 * 
 * Fournit :
 * - Cycle de vie commun setup/teardown
 * - Gestion des répertoires temporaires
 * - Timing et métriques des tests
 * - Assertions et utilitaires standards
 */
class TestBase : public QObject {
  Q_OBJECT

protected:
  // Hooks de cycle de vie - à surcharger dans les classes dérivées
  virtual void onSetUp() {}
  virtual void onTearDown() {}
  virtual void onTestCaseSetUp() {}
  virtual void onTestCaseTearDown() {}

  // Utilitaires
  [[nodiscard]] QString tempPath() const { return m_tempDir->path(); }
  [[nodiscard]] QString tempFile(const QString& name) const;
  void createTempFile(const QString& name, const QByteArray& content);
  
  // Utilitaires de timing
  void startTimer() { m_timer.start(); }
  [[nodiscard]] qint64 elapsedMs() const { return m_timer.elapsed(); }

  // Gestion de l'environnement
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
 * @brief Classe de base pour les tests dépendant du réseau
 */
class NetworkTestBase : public TestBase {
  Q_OBJECT

protected:
  void onSetUp() override;
  void onTearDown() override;

  [[nodiscard]] MockHttpClient& mockHttp() { return *m_mockHttp; }
  
  // Méthodes de commodité
  void queueJsonResponse(const QByteArray& json, int status = 200);
  void queueTwitchResponse(const QVariantList& data);
  void queueTwitchError(int status, const QString& message);
  void queueNetworkError(QNetworkReply::NetworkError error);

private:
  std::unique_ptr<MockHttpClient> m_mockHttp;
};

/**
 * @brief Classe de base pour les tests asynchrones nécessitant une boucle d'événements
 */
class AsyncTestBase : public TestBase {
  Q_OBJECT

protected:
  // Attendre un signal avec timeout
  template<typename Signal>
  bool waitForSignal(QObject* sender, Signal signal, int timeoutMs = 5000);
  
  // Attendre une condition avec timeout
  bool waitFor(std::function<bool()> condition, int timeoutMs = 5000);
  
  // Traiter les événements
  void processEvents(int ms = 100);
};

/**
 * @brief Classe de base pour les tests de base de données/cache
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
 * @brief Classe de base pour les tests QML/UI
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

### 3.2 Catégories de tests via traits

```cpp
// tests/base/TestTraits.hpp
#pragma once

#include <QtTest/QtTest>

namespace blueplayer::test::traits {

/**
 * @brief Marque un test comme test smoke (rapide, chemin critique)
 */
#define SMOKE_TEST \
  private: void runAsSmoke() { QVERIFY(true); }

/**
 * @brief Marque un test comme sensible aux performances
 */
#define PERF_TEST \
  private: void runAsPerf() { QVERIFY(true); }

/**
 * @brief Marque un test comme nécessitant le réseau
 */
#define NETWORK_TEST \
  private: void requiresNetwork() { QVERIFY(true); }

/**
 * @brief Marque un test comme lent (temps d'exécution >1s)
 */
#define SLOW_TEST \
  private: void isSlow() { QVERIFY(true); }

/**
 * @brief Sauter le test si la condition n'est pas remplie
 */
#define SKIP_IF(condition, message) \
  if (condition) QSKIP(message)

/**
 * @brief Décorateur de timeout pour les tests asynchrones
 */
#define TEST_TIMEOUT(ms) \
  QTimer::singleShot(ms, []() { QFAIL("Test timed out"); })

} // namespace blueplayer::test::traits
```

---

## 4. Pattern Factory pour les fixtures

### 4.1 Interface générique de factory

```cpp
// tests/fixtures/FixtureFactory.hpp
#pragma once

#include <memory>
#include <functional>

namespace blueplayer::test {

/**
 * @brief Factory générique pour créer des fixtures de test
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

### 4.2 Factories spécifiques au domaine

```cpp
// tests/fixtures/TwitchFixtures.hpp
#pragma once

#include "helpers/TwitchTestData.hpp"
#include <QVariantMap>
#include <QVariantList>
#include <random>

namespace blueplayer::test {

/**
 * @brief Factory pour créer des fixtures de stream Twitch
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
 * @brief Factory pour créer des fixtures de catégorie Twitch
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
 * @brief Factory pour créer des fixtures de réponse API complètes
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

## 5. Builder Pattern pour les données de test

### 5.1 Implémentation du builder fluent

```cpp
// tests/builders/TestDataBuilder.hpp
#pragma once

#include <QVariantMap>
#include <QVariantList>
#include <QString>
#include <QDateTime>

namespace blueplayer::test {

/**
 * @brief Builder fluent pour les données de test de stream Twitch
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
 * @brief Builder fluent pour les données de test de vidéo/VOD Twitch
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
 * @brief Builder pour les réponses d'erreur API
 */
class ErrorResponseBuilder {
public:
  ErrorResponseBuilder& status(int code) { m_status = code; return *this; }
  ErrorResponseBuilder& error(const QString& err) { m_error = err; return *this; }
  ErrorResponseBuilder& message(const QString& msg) { m_message = msg; return *this; }
  
  // Préréglages d'erreurs courantes
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

// Fonctions de commodité
inline StreamBuilder aStream() { return StreamBuilder::aStream(); }
inline VideoBuilder aVideo() { return VideoBuilder::aVideo(); }
inline ErrorResponseBuilder anError() { return ErrorResponseBuilder(); }

} // namespace blueplayer::test
```

---

## 6. Pattern Page Object pour les tests UI

### 6.1 Page Object de base

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
 * @brief Classe de base pour les Page Objects dans les tests UI
 * 
 * Implémente le pattern Page Object pour les tests UI QML.
 * Chaque page/vue a un PageObject correspondant qui encapsule
 * toutes les interactions UI.
 */
class PageObject {
public:
  explicit PageObject(QQuickView* view) : m_view(view) {}
  virtual ~PageObject() = default;
  
  // Navigation
  [[nodiscard]] virtual bool isVisible() const = 0;
  virtual void waitUntilLoaded(int timeoutMs = 5000);
  
protected:
  // Recherche d'éléments
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
  
  // Accès aux propriétés
  [[nodiscard]] QVariant getProperty(QQuickItem* item, const QString& property) const;
  [[nodiscard]] QVariant getProperty(const QString& objectName, const QString& property) const;
  void setProperty(QQuickItem* item, const QString& property, const QVariant& value);
  
  // Attentes
  bool waitForProperty(QQuickItem* item, const QString& property, 
                       const QVariant& expectedValue, int timeoutMs = 5000);
  bool waitForVisible(const QString& objectName, int timeoutMs = 5000);
  bool waitForHidden(const QString& objectName, int timeoutMs = 5000);
  
  // Accès à la vue
  [[nodiscard]] QQuickView* view() const { return m_view; }
  [[nodiscard]] QQuickItem* rootItem() const { return m_view->rootObject(); }

private:
  QQuickView* m_view;
};

} // namespace blueplayer::test::ui
```

### 6.2 Page Objects concrets

```cpp
// tests/ui/pages/HomePageObject.hpp
#pragma once

#include "PageObject.hpp"

namespace blueplayer::test::ui {

/**
 * @brief Page Object pour HomeView.qml
 */
class HomePageObject : public PageObject {
public:
  using PageObject::PageObject;
  
  // Visibilité
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
  
  // Cartes de stream
  [[nodiscard]] QString streamCardTitle(int index) const;
  [[nodiscard]] QString streamCardStreamer(int index) const;
  [[nodiscard]] int streamCardViewers(int index) const;
  
  // Recherche
  void openSearch();
  void search(const QString& query);
  void clearSearch();
  
  // Navigation
  PlayerPageObject clickFirstStream();
  CategoryPageObject clickFirstCategory();
};

/**
 * @brief Page Object pour PlayerView.qml
 */
class PlayerPageObject : public PageObject {
public:
  using PageObject::PageObject;
  
  [[nodiscard]] bool isVisible() const override;
  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] bool isPaused() const;
  [[nodiscard]] bool isBuffering() const;
  
  // Contrôles
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
  
  // Qualité
  void openQualitySelector();
  void selectQuality(const QString& quality);
  [[nodiscard]] QString currentQuality() const;
  [[nodiscard]] QStringList availableQualities() const;
  
  // Chat
  void toggleChat();
  [[nodiscard]] bool isChatVisible() const;
  
  // Navigation retour
  HomePageObject goBack();
};

/**
 * @brief Page Object pour LoginView.qml
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

## 7. Catégorisation des tests

### 7.1 Tags et catégories de tests

```cpp
// tests/categories/TestCategories.hpp
#pragma once

namespace blueplayer::test {

/**
 * Catégories de tests pour le filtrage et les pipelines CI/CD
 */
enum class TestCategory {
  Smoke,        // Tests rapides, chemin critique (<100ms chacun)
  Unit,         // Tests unitaires standards
  Integration,  // Tests avec dépendances réelles
  Performance,  // Tests de benchmark
  Chaos,        // Tests d'injection de fautes
  E2E,          // Tests end-to-end UI
  Regression    // Suite de régression complète
};

/**
 * Mapping des labels CTest (utilisé dans CMakeLists.txt)
 */
// set_tests_properties(test_name PROPERTIES LABELS "smoke;unit")

} // namespace blueplayer::test
```

### 7.2 Organisation CMake des tests

```cmake
# Ajouts à tests/CMakeLists.txt

# Définir les catégories de tests
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
  # Futur: test_network_perf
  # Futur: test_cache_perf
)

set(CHAOS_TESTS
  # Futur: test_network_chaos
  # Futur: test_auth_chaos
)

set(E2E_TESTS
  # Futur: test_home_e2e
  # Futur: test_player_e2e
)

# Appliquer les labels
foreach(test ${SMOKE_TESTS})
  set_tests_properties(${test} PROPERTIES LABELS "smoke;fast")
endforeach()

foreach(test ${UNIT_TESTS})
  set_tests_properties(${test} PROPERTIES LABELS "unit")
endforeach()

foreach(test ${INTEGRATION_TESTS})
  set_tests_properties(${test} PROPERTIES LABELS "integration;slow")
endforeach()

# Cibles personnalisées pour exécuter les catégories de tests
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

### 7.3 Tests smoke (chemin critique)

```cpp
// tests/smoke/SmokeTests.cpp
/**
 * @brief Tests smoke pour les chemins critiques de l'application
 * 
 * Ces tests doivent :
 * - S'exécuter en < 100ms au total
 * - Couvrir les chemins d'initialisation critiques
 * - Vérifier que les fonctionnalités de base fonctionnent
 * - S'exécuter à chaque commit
 */

#include "base/TestBase.hpp"
#include "core/Config.hpp"
#include "core/Error.hpp"
#include "core/Result.hpp"

namespace blueplayer::test {

class SmokeTests : public TestBase {
  Q_OBJECT

private slots:
  // Tests smoke de configuration
  void testConfigSingletonExists() {
    startTimer();
    Config& config = Config::instance();
    QVERIFY(&config != nullptr);
    QVERIFY(elapsedMs() < 10);
  }
  
  // Tests smoke de gestion d'erreurs
  void testErrorCanBeCreated() {
    Error error(ErrorCode::Unknown);
    QVERIFY(!error.hasError() || error.code() != ErrorCode::Unknown);
  }
  
  // Tests smoke du type Result
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

## 8. Tests de performance (benchmarks)

### 8.1 Framework de benchmark

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
 * @brief Conteneur de résultats de benchmark
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
    qDebug() << "  Itérations:" << iterations;
    qDebug() << "  Total:" << totalMs << "ms";
    qDebug() << "  Min:" << minMs << "ms";
    qDebug() << "  Max:" << maxMs << "ms";
    qDebug() << "  Moyenne:" << avgMs << "ms";
    qDebug() << "  Médiane:" << medianMs << "ms";
    qDebug() << "  Écart-type:" << stdDevMs << "ms";
  }
};

/**
 * @brief Utilitaire d'exécution de benchmarks
 */
class Benchmark {
public:
  static BenchmarkResult run(const QString& name, 
                             std::function<void()> fn,
                             int iterations = 100,
                             int warmupIterations = 10) {
    // Préchauffage
    for (int i = 0; i < warmupIterations; ++i) {
      fn();
    }
    
    // Mesures réelles
    std::vector<qint64> times;
    times.reserve(iterations);
    
    QElapsedTimer timer;
    for (int i = 0; i < iterations; ++i) {
      timer.start();
      fn();
      times.push_back(timer.elapsed());
    }
    
    // Calcul des statistiques
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
  
  // Helpers d'assertion
  static void assertMaxTime(const BenchmarkResult& result, qint64 maxMs) {
    QVERIFY2(result.avgMs <= maxMs, 
             qPrintable(QString("Le benchmark %1 a dépassé le temps maximum : %2ms > %3ms")
                       .arg(result.name)
                       .arg(result.avgMs)
                       .arg(maxMs)));
  }
};

} // namespace blueplayer::test
```

### 8.2 Exemples de tests de performance

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
    // Créer une grande réponse JSON
    QString largeJson = createLargeStreamResponse(1000);
    
    auto result = Benchmark::run("Parsing JSON (1000 streams)", [&]() {
      QJsonDocument::fromJson(largeJson.toUtf8());
    });
    
    result.print();
    Benchmark::assertMaxTime(result, 50); // Max 50ms en moyenne
  }
  
  void benchmarkStreamTransformation() {
    QVariantList streams;
    for (int i = 0; i < 100; ++i) {
      streams.append(StreamBuilder::aStream()
                    .userName(QString("Streamer%1").arg(i))
                    .viewers(i * 100)
                    .build());
    }
    
    auto result = Benchmark::run("Transformation de streams (100 streams)", [&]() {
      // Transformer les streams via le ViewModel
      HomeViewModel vm;
      vm.transformTwitchStreams(streams);
    });
    
    result.print();
    Benchmark::assertMaxTime(result, 10); // Max 10ms en moyenne
  }
  
private:
  QString createLargeStreamResponse(int count);
};

} // namespace blueplayer::test
```

---

## 9. Tests de chaos (injection de fautes)

### 9.1 Framework de tests de chaos

```cpp
// tests/chaos/ChaosEngine.hpp
#pragma once

#include <functional>
#include <random>
#include <QNetworkReply>

namespace blueplayer::test {

/**
 * @brief Utilitaires d'ingénierie du chaos pour les tests d'injection de fautes
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
   * @brief Injecte une faute aléatoire selon une probabilité
   */
  static bool shouldInjectFault(double probability = 0.1) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(gen) < probability;
  }
  
  /**
   * @brief Obtient un type de faute aléatoire
   */
  static FaultType randomFault() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 8);
    return static_cast<FaultType>(dis(gen));
  }
  
  /**
   * @brief Crée une MockResponse pour un type de faute donné
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
        return MockResponse::json("{\"data\":");  // JSON incomplet
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
 * @brief Client HTTP mock qui injecte des fautes aléatoirement
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

### 9.2 Exemples de tests de chaos

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
    // Créer un service avec un client chaotique
    auto chaoticClient = std::make_unique<ChaoticHttpClient>(0.3);
    // Injecter le client... (nécessiterait le support DI)
    
    TwitchService service;
    QSignalSpy errorSpy(&service, &TwitchService::errorOccurred);
    
    // Marteler le service avec des requêtes
    for (int i = 0; i < 100; ++i) {
      service.refreshStreams();
      service.refreshCategories();
      service.search("test");
      
      // Traiter les événements
      QCoreApplication::processEvents();
    }
    
    // Le service ne doit pas planter
    QVERIFY(&service != nullptr);
    
    // Des erreurs sont attendues
    qDebug() << "Erreurs rencontrées:" << errorSpy.count();
  }
  
  void testGracefulDegradationOnTimeout() {
    queueNetworkError(QNetworkReply::TimeoutError);
    
    TwitchService service;
    QSignalSpy errorSpy(&service, &TwitchService::errorOccurred);
    
    service.refreshStreams();
    processEvents(1000);
    
    // Doit émettre une erreur
    QVERIFY(errorSpy.count() > 0);
    // Les données doivent être vides (pas de crash)
    QVERIFY(service.streams().isEmpty());
  }
  
  void testRecoveryAfterNetworkRestore() {
    // La première requête échoue
    queueNetworkError(QNetworkReply::TimeoutError);
    // La deuxième requête réussit
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

## 10. Stratégie d'isolation des tests

### 10.1 Pattern d'injection de dépendances

```cpp
// src/core/DependencyContainer.hpp
#pragma once

#include <memory>
#include <functional>
#include <typeindex>
#include <unordered_map>

namespace blueplayer::core {

/**
 * @brief Conteneur d'injection de dépendances léger
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
    // Vérifier d'abord l'instance enregistrée
    auto instIt = m_instances.find(typeid(Interface));
    if (instIt != m_instances.end()) {
      return std::static_pointer_cast<Interface>(instIt->second);
    }
    
    // Se rabattre sur la factory
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

### 10.2 Frontières de mock

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
|   |        Clients API            | <- MOCK ICI   |
|   +---------------+---------------+               |
|                   |                               |
+-------------------v-------------------------------+
                    |
          +---------v---------+
          |  Couche réseau    | <- MOCK ICI
          +---------+---------+
                    |
          +---------v---------+
          |   APIs externes   | <- RÉEL EN INTÉGRATION
          +-------------------+

FRONTIÈRES DE MOCK :
1. MockHttpClient      - Remplace la couche réseau
2. MockTwitchApiClient - Remplace le client API
3. MockCacheManager    - Remplace le système de fichiers
4. MockSecureStorage   - Remplace le keychain
```

### 10.3 Stratégie de base de données/cache de test

```cpp
// tests/fixtures/TestEnvironment.hpp
#pragma once

#include <QTemporaryDir>
#include <QStandardPaths>
#include <memory>

namespace blueplayer::test {

/**
 * @brief Gère l'environnement de test isolé
 */
class TestEnvironment {
public:
  static TestEnvironment& instance() {
    static TestEnvironment env;
    return env;
  }
  
  void setUp() {
    m_tempDir = std::make_unique<QTemporaryDir>();
    
    // Surcharger les chemins standards pour les tests
    QStandardPaths::setTestModeEnabled(true);
    
    // Définir les variables d'environnement
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

## 11. Conventions de nommage et d'organisation

### 11.1 Nommage des fichiers de test

```
tests/
  base/                          # Classes de base
    TestBase.hpp
    TestTraits.hpp
    
  builders/                      # Builder pattern
    TestDataBuilder.hpp
    
  fixtures/                      # Fixture factories
    TwitchFixtures.hpp
    FixtureFactory.hpp
    TestEnvironment.hpp
    
  mocks/                         # Implémentations mock
    MockHttpClient.hpp
    MockNetworkReply.hpp
    MockCacheManager.hpp
    MockSecureStorage.hpp
    
  helpers/                       # Utilitaires de test
    TwitchTestData.hpp
    TestHelpers.hpp
    
  ui/pages/                      # Page Objects
    PageObject.hpp
    HomePageObject.hpp
    PlayerPageObject.hpp
    
  smoke/                         # Tests smoke
    SmokeTests.cpp
    
  performance/                   # Tests de performance
    Benchmark.hpp
    TestNetworkPerformance.cpp
    
  chaos/                         # Tests de chaos
    ChaosEngine.hpp
    TestNetworkChaos.cpp
    
  # Tests par domaine (miroir de la structure src/)
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
    TestPlayerE2E.cpp           # Tests E2E
    
  integration/
    TestTwitchFlow.cpp
```

### 11.2 Convention de nommage des méthodes de test

```cpp
/**
 * Pattern: test<Méthode>_<Scénario>_<ComportementAttendu>
 * 
 * Exemples :
 * - testParseJson_ValidResponse_ReturnsData
 * - testParseJson_MalformedInput_ReturnsError
 * - testLogin_WithValidCredentials_EmitsAuthenticatedSignal
 * - testRefreshStreams_WhenNotAuthenticated_ReturnsEmpty
 */

class TestTwitchService : public QObject {
  Q_OBJECT

private slots:
  // Bon nommage
  void testRefreshStreams_WhenAuthenticated_EmitsStreamsChanged();
  void testRefreshStreams_WhenNotAuthenticated_DoesNotCrash();
  void testSearch_WithEmptyQuery_ReturnsEmpty();
  void testPlayStream_WithInvalidIndex_EmitsError();
  
  // Alternative : style Given-When-Then
  void givenAuthenticated_whenRefreshStreams_thenStreamsChanged();
  void givenNotAuthenticated_whenRefreshStreams_thenEmptyResult();
};
```

### 11.3 Organisation des classes de test

```cpp
class TestTwitchApiClient : public NetworkTestBase {
  Q_OBJECT

private slots:
  // === Cycle de vie (hérité de la base) ===
  // init(), cleanup(), initTestCase(), cleanupTestCase()
  
  // === Section 1 : Tests d'initialisation ===
  void testConstructor();
  void testSetAccessToken();
  void testSetAccessToken_Empty();
  void testSetAccessToken_Null();
  
  // === Section 2 : Tests de fonctionnalités principales ===
  void testListStreams_Success();
  void testListStreams_Empty();
  void testListStreams_WithPagination();
  
  // === Section 3 : Tests de gestion d'erreurs ===
  void testListStreams_NetworkError();
  void testListStreams_Unauthorized();
  void testListStreams_RateLimited();
  
  // === Section 4 : Cas limites ===
  void testListStreams_NegativeLimit();
  void testListStreams_ZeroLimit();
  void testListStreams_LargeLimit();
  
  // === Section 5 : Tests de signaux ===
  void testStreamsReadySignal();
  void testErrorSignal();
  void testTokenInvalidatedSignal();
  
  // === Section 6 : Tests data-driven ===
  void testErrorCodes_data();
  void testErrorCodes();

private:
  // Méthodes helper
  void queueStreamResponse(int count);
  void verifyStreamData(const QVariantList& streams);
};
```

---

## 12. Roadmap d'implémentation

### Phase 1 : Fondations (Semaine 1-2)
- [ ] Créer le répertoire `tests/base/` avec les classes de base
- [ ] Implémenter `TestBase`, `NetworkTestBase`, `AsyncTestBase`
- [ ] Créer l'isolation de l'environnement de test
- [ ] Ajouter l'infrastructure CMake pour les catégories de tests

### Phase 2 : Fixtures et builders (Semaine 2-3)
- [ ] Implémenter les classes du builder pattern
- [ ] Créer les fixture factories
- [ ] Refactoriser les tests existants pour utiliser les builders

### Phase 3 : Tests UI (Semaine 3-4)
- [ ] Implémenter la classe de base Page Object
- [ ] Créer les Page Objects pour les vues principales
- [ ] Ajouter les premiers tests E2E

### Phase 4 : Performance et chaos (Semaine 4-5)
- [ ] Implémenter le framework de benchmark
- [ ] Ajouter des tests de performance pour les chemins critiques
- [ ] Implémenter le moteur de chaos
- [ ] Ajouter des tests d'injection de fautes

### Phase 5 : Intégration CI/CD (Semaine 5-6)
- [ ] Configurer les tests smoke pour chaque commit
- [ ] Configurer la régression complète nocturne
- [ ] Ajouter la détection de régression de performance
- [ ] Créer un tableau de bord de reporting des tests

---

## 13. Métriques récapitulatives

| Métrique | Actuel | Cible |
|----------|--------|-------|
| Couverture des tests unitaires | ~60% | 85% |
| Couverture des tests d'intégration | ~10% | 50% |
| Couverture des tests E2E | 0% | 20% |
| Temps d'exécution des tests smoke | N/A | <5s |
| Temps d'exécution de la suite complète | ~30s | <60s |
| Taux de tests flaky | Inconnu | <1% |
| Ratio test/code | 0.4:1 | 1:1 |

---

## Annexe A : Checklist des bonnes pratiques Qt Test

- [x] Utiliser `QCOMPARE` au lieu de `QVERIFY` pour les comparaisons de valeurs
- [x] Utiliser `QSignalSpy` pour les tests de signaux asynchrones
- [ ] Utiliser les tests data-driven avec le suffixe `_data()`
- [ ] Utiliser `QBENCHMARK` pour les tests de performance
- [ ] Éviter `QTest::qWait()` - utiliser l'attente basée sur les signaux
- [x] Nettoyer les ressources dans `cleanup()` et non dans le destructeur
- [x] Utiliser `QTemporaryDir` pour les tests basés sur les fichiers
- [ ] Utiliser `QStandardPaths::setTestModeEnabled(true)`

---

## Annexe B : Références

1. Google Testing Blog - Pyramide des tests
2. Directives de test Microsoft pour C++
3. Documentation du framework Qt Test
4. Martin Fowler - Pattern Page Object
5. Principes d'ingénierie du chaos Netflix
