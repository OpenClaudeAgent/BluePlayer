cmake_minimum_required(VERSION 3.24)
project(BluePlayerTests LANGUAGES CXX)

# Utiliser Qt Test Framework pour les tests unitaires et d'intégration
find_package(Qt6 6.5 COMPONENTS Core Quick Gui Multimedia Test Network REQUIRED)

# FFmpeg est trouvé par le CMakeLists.txt principal, nous utilisons ses variables directement.
# Assurez-vous que FFmpeg_LIBRARIES et FFmpeg_INCLUDE_DIRS sont définis par le parent.
# Les bibliothèques blueplayer_core et blueplayer_media sont créées par le CMakeLists.txt principal.

# Configuration globale pour tous les tests
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Fonction helper pour appliquer les flags de couverture aux exécutables de test
function(apply_coverage_flags target)
    if(BLUEPLAYER_COVERAGE_ENABLED)
        target_compile_options(${target} PRIVATE ${BLUEPLAYER_COVERAGE_FLAGS})
        target_link_options(${target} PRIVATE ${BLUEPLAYER_COVERAGE_FLAGS})
    endif()
endfunction()

# Chemins d'inclusion communs
set(TEST_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/mocks
  ${FFmpeg_INCLUDE_DIRS}
)

# Bibliothèques communes pour les tests
# Note: FFmpeg_LIBRARIES n'est pas inclus ici car elles sont déjà liées via blueplayer_media
set(TEST_LIBS
  Qt6::Core
  Qt6::Test
  Qt6::Network  # Pour QNetworkAccessManager, QSslConfiguration
  Qt6::Multimedia  # Pour QVideoSink
)

# Helpers et fixtures pour les tests
# Note: MockNetworkReply needs Q_OBJECT moc generation
add_library(test_helpers STATIC
  TestHelpers.cpp
  mocks/MockNetworkReply.cpp
)
target_link_libraries(test_helpers PUBLIC 
  ${TEST_LIBS}
  blueplayer_core
  Qt6::Network  # Nécessaire pour QNetworkAccessManager dans TestHelpers
)
target_include_directories(test_helpers PUBLIC
  ${TEST_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/mocks
  ${CMAKE_CURRENT_SOURCE_DIR}/helpers
)

# Tests unitaires pour les services API Twitch
add_executable(test_twitch_api_client
  api/twitch/TestTwitchApiClient.cpp
)
target_link_libraries(test_twitch_api_client PRIVATE
  test_helpers
  ${TEST_LIBS}
  blueplayer_core  # Nécessaire pour TwitchApiClient
)
target_include_directories(test_twitch_api_client PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_twitch_api_client PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
apply_coverage_flags(test_twitch_api_client)
add_test(NAME TwitchApiClient COMMAND test_twitch_api_client)

# Tests unitaires pour TwitchAuthManager
add_executable(test_twitch_auth_manager
  api/twitch/TestTwitchAuthManager.cpp
)
target_link_libraries(test_twitch_auth_manager PRIVATE
  test_helpers
  ${TEST_LIBS}
  blueplayer_core  # Nécessaire pour TwitchAuthManager
)
target_include_directories(test_twitch_auth_manager PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_twitch_auth_manager PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
apply_coverage_flags(test_twitch_auth_manager)
add_test(NAME TwitchAuthManager COMMAND test_twitch_auth_manager)



# Tests d'intégration pour le flux Twitch complet
add_executable(test_twitch_integration
  integration/TestTwitchFlow.cpp
)
target_link_libraries(test_twitch_integration PRIVATE
  test_helpers
  ${TEST_LIBS}
  blueplayer_core  # Nécessaire pour TwitchService
)
target_include_directories(test_twitch_integration PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_twitch_integration PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
apply_coverage_flags(test_twitch_integration)
add_test(NAME TwitchIntegration COMMAND test_twitch_integration)



# Tests unitaires pour les modules core
add_executable(test_config
  core/TestConfig.cpp
)
target_link_libraries(test_config PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_config PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_config PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_config)
add_test(NAME Config COMMAND test_config)

add_executable(test_error_handler
  core/TestErrorHandler.cpp
)
target_link_libraries(test_error_handler PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_error_handler PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_error_handler PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_error_handler)
add_test(NAME ErrorHandler COMMAND test_error_handler)

add_executable(test_input_validator
  core/TestInputValidator.cpp
)
target_link_libraries(test_input_validator PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_input_validator PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_input_validator PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_input_validator)
add_test(NAME InputValidator COMMAND test_input_validator)

add_executable(test_network_cache
  core/TestNetworkCache.cpp
)
target_link_libraries(test_network_cache PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_network_cache PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_network_cache PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_network_cache)
add_test(NAME NetworkCache COMMAND test_network_cache)

add_executable(test_secure_storage
  core/TestSecureStorage.cpp
)
target_link_libraries(test_secure_storage PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_secure_storage PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_secure_storage PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_secure_storage)
add_test(NAME SecureStorage COMMAND test_secure_storage)

# Tests unitaires pour LanguageManager
add_executable(test_language_manager
  core/TestLanguageManager.cpp
)
target_link_libraries(test_language_manager PRIVATE
  ${TEST_LIBS}
  Qt6::Qml
  blueplayer_core
)
target_include_directories(test_language_manager PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_language_manager PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_language_manager)
add_test(NAME LanguageManager COMMAND test_language_manager)

# Tests unitaires pour les modules network
add_executable(test_http_client
  core/network/TestHttpClient.cpp
)
target_link_libraries(test_http_client PRIVATE
  test_helpers
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_http_client PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_http_client PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_http_client)
add_test(NAME HttpClient COMMAND test_http_client)

add_executable(test_api_client_base
  core/network/TestApiClientBase.cpp
)
target_link_libraries(test_api_client_base PRIVATE
  test_helpers
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_api_client_base PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_api_client_base PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_api_client_base)
add_test(NAME ApiClientBase COMMAND test_api_client_base)

# Tests unitaires pour les modules media

# Tests unitaires pour StateMachineLiveReplay
add_executable(test_state_machine_live_replay
  core/TestStateMachineLiveReplay.cpp
)
target_link_libraries(test_state_machine_live_replay PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_state_machine_live_replay PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_state_machine_live_replay PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_state_machine_live_replay)
add_test(NAME StateMachineLiveReplay COMMAND test_state_machine_live_replay)

# Tests unitaires pour CacheManager
add_executable(test_cache_manager
  core/TestCacheManager.cpp
)
target_link_libraries(test_cache_manager PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_cache_manager PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_cache_manager PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_cache_manager)
add_test(NAME CacheManager COMMAND test_cache_manager)

# Tests unitaires pour les ViewModels
add_executable(test_home_view_model
  ui/TestHomeViewModel.cpp
)
target_link_libraries(test_home_view_model PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_home_view_model PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_home_view_model PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_home_view_model)
add_test(NAME HomeViewModel COMMAND test_home_view_model)

include(CTest)
enable_testing()

# ===== Nouveaux tests ajoutés =====

# Tests unitaires pour HlsAdFilter
add_executable(test_hls_ad_filter
  media/TestHlsAdFilter.cpp
)
target_link_libraries(test_hls_ad_filter PRIVATE
  ${TEST_LIBS}
  blueplayer_core
  blueplayer_media
)
target_include_directories(test_hls_ad_filter PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_hls_ad_filter PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
apply_coverage_flags(test_hls_ad_filter)
add_test(NAME HlsAdFilter COMMAND test_hls_ad_filter)

# Tests unitaires pour PlaybackSpeedLogic
add_executable(test_playback_speed_logic
  media/TestPlaybackSpeedLogic.cpp
)
target_link_libraries(test_playback_speed_logic PRIVATE
  ${TEST_LIBS}
  blueplayer_core
  blueplayer_media
)
target_include_directories(test_playback_speed_logic PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_playback_speed_logic PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
apply_coverage_flags(test_playback_speed_logic)
add_test(NAME PlaybackSpeedLogic COMMAND test_playback_speed_logic)

# Tests unitaires pour WatchHistory
add_executable(test_watch_history
  core/TestWatchHistory.cpp
)
target_link_libraries(test_watch_history PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_watch_history PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_watch_history PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_watch_history)
add_test(NAME WatchHistory COMMAND test_watch_history)

# Tests unitaires pour CacheManagerViewModel
add_executable(test_cache_manager_view_model
  ui/TestCacheManagerViewModel.cpp
)
target_link_libraries(test_cache_manager_view_model PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_cache_manager_view_model PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_cache_manager_view_model PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_cache_manager_view_model)
add_test(NAME CacheManagerViewModel COMMAND test_cache_manager_view_model)

# Tests unitaires pour Logger
add_executable(test_logger
  core/TestLogger.cpp
)
target_link_libraries(test_logger PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_logger PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_logger PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_logger)
add_test(NAME Logger COMMAND test_logger)

# Tests unitaires pour FileLogger
add_executable(test_file_logger
  core/TestFileLogger.cpp
)
target_link_libraries(test_file_logger PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_file_logger PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_file_logger PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_file_logger)
add_test(NAME FileLogger COMMAND test_file_logger)

# Tests unitaires pour CurlHttpClient
add_executable(test_curl_http_client
  core/network/TestCurlHttpClient.cpp
)
target_link_libraries(test_curl_http_client PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_curl_http_client PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_curl_http_client PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_curl_http_client)
add_test(NAME CurlHttpClient COMMAND test_curl_http_client)

# Tests unitaires pour TwitchChatClient
add_executable(test_twitch_chat_client
  chat/TestTwitchChatClient.cpp
)
target_link_libraries(test_twitch_chat_client PRIVATE
  ${TEST_LIBS}
  Qt6::WebSockets
  blueplayer_core
)
target_include_directories(test_twitch_chat_client PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_twitch_chat_client PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_twitch_chat_client)
add_test(NAME TwitchChatClient COMMAND test_twitch_chat_client)

# Tests unitaires pour TwitchService (complet)
add_executable(test_twitch_service
  api/twitch/TestTwitchService.cpp
)
target_link_libraries(test_twitch_service PRIVATE
  test_helpers
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_twitch_service PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_twitch_service PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
apply_coverage_flags(test_twitch_service)
add_test(NAME TwitchService COMMAND test_twitch_service)

# Tests unitaires pour Error
add_executable(test_error
  core/TestError.cpp
)
target_link_libraries(test_error PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_error PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_error PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_error)
add_test(NAME Error COMMAND test_error)

# Tests unitaires pour Result
add_executable(test_result
  core/TestResult.cpp
)
target_link_libraries(test_result PRIVATE
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_result PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_result PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_result)
add_test(NAME Result COMMAND test_result)

# ===== Contract Tests =====

# Include directories for contract tests
set(CONTRACT_TEST_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/contracts
)

# Helix API Contract Tests
add_executable(test_helix_contracts
  contracts/TestHelixContracts.cpp
)
target_link_libraries(test_helix_contracts PRIVATE
  ${TEST_LIBS}
)
target_include_directories(test_helix_contracts PRIVATE ${CONTRACT_TEST_INCLUDE_DIRS})
target_link_directories(test_helix_contracts PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_helix_contracts)
add_test(NAME HelixContracts COMMAND test_helix_contracts)

# OAuth Contract Tests
add_executable(test_oauth_contracts
  contracts/TestOAuthContracts.cpp
)
target_link_libraries(test_oauth_contracts PRIVATE
  ${TEST_LIBS}
)
target_include_directories(test_oauth_contracts PRIVATE ${CONTRACT_TEST_INCLUDE_DIRS})
target_link_directories(test_oauth_contracts PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_oauth_contracts)
add_test(NAME OAuthContracts COMMAND test_oauth_contracts)

# IRC Contract Tests
add_executable(test_irc_contracts
  contracts/TestIrcContracts.cpp
)
target_link_libraries(test_irc_contracts PRIVATE
  ${TEST_LIBS}
)
target_include_directories(test_irc_contracts PRIVATE ${CONTRACT_TEST_INCLUDE_DIRS})
target_link_directories(test_irc_contracts PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_irc_contracts)
add_test(NAME IrcContracts COMMAND test_irc_contracts)

# HLS Contract Tests
add_executable(test_hls_contracts
  contracts/TestHlsContracts.cpp
)
target_link_libraries(test_hls_contracts PRIVATE
  ${TEST_LIBS}
)
target_include_directories(test_hls_contracts PRIVATE ${CONTRACT_TEST_INCLUDE_DIRS})
target_link_directories(test_hls_contracts PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_hls_contracts)
add_test(NAME HlsContracts COMMAND test_hls_contracts)

# ===== Tests Fonctionnels UI (Qt Quick Test) =====
# Split into 4 parallel targets for faster execution with ctest -j4

# Find Qt Quick Test module
find_package(Qt6 6.5 COMPONENTS QuickTest QUIET)

if(Qt6QuickTest_FOUND)
  message(STATUS "Qt6 QuickTest found - enabling functional UI tests (4 parallel targets)")

  # Common libraries for all functional test targets
  set(FUNCTIONAL_TEST_LIBS
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::QuickTest
    Qt6::Test
  )

  # Helper function to create a functional test target
  function(add_functional_test_target TARGET_NAME SOURCE_DIR TEST_NAME LABEL)
    add_executable(${TARGET_NAME}
      functional/${SOURCE_DIR}/main.cpp
    )

    target_link_libraries(${TARGET_NAME} PRIVATE ${FUNCTIONAL_TEST_LIBS})

    target_compile_definitions(${TARGET_NAME} PRIVATE
      QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/functional/${SOURCE_DIR}"
    )

    target_include_directories(${TARGET_NAME} PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/../src
    )

    target_link_directories(${TARGET_NAME} PRIVATE /opt/homebrew/lib)
    apply_coverage_flags(${TARGET_NAME})

    add_test(NAME ${TEST_NAME} COMMAND ${TARGET_NAME})

    # Use offscreen platform for faster execution (6-7x speedup)
    # Some mouse-related tests may fail in offscreen mode
    set_tests_properties(${TEST_NAME} PROPERTIES
      LABELS "functional;ui;${LABEL}"
      TIMEOUT 180
      ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
  endfunction()

  # Target 1: Card components (8 files, ~300 tests)
  # BaseCard, BlueCard, CategoryCard, ChannelCard, ClipCard, LocalPlaybackCard, StreamCard, VideoCard
  add_functional_test_target(test_functional_cards cards FunctionalCards cards)

  # Target 2: Control components (8 files, ~280 tests)
  # BlueButton, BlueDropdown, CircleButton, ControlButton, QualityControl, QualitySelector, SeekBar, VolumeControl
  add_functional_test_target(test_functional_controls controls FunctionalControls controls)

  # Target 3: Panel components (7 files, ~250 tests)
  # ErrorToast, LoadingOverlay, PanelHeader, PlayerControlBar, SearchResultItem, SearchResults, TopBarOverlay
  add_functional_test_target(test_functional_panels panels FunctionalPanels panels)

  # Target 4: View/container components (4 files, ~240 tests)
  # CategoriesScroller, ChatMessage, ChatPanel, HorizontalRowSection
  add_functional_test_target(test_functional_views views FunctionalViews views)

  # Add a convenience target to build all functional tests
  add_custom_target(test_functional_all
    DEPENDS test_functional_cards test_functional_controls test_functional_panels test_functional_views
  )

  message(STATUS "Functional UI tests configured:")
  message(STATUS "  - test_functional_cards (8 files)")
  message(STATUS "  - test_functional_controls (8 files)")
  message(STATUS "  - test_functional_panels (7 files)")
  message(STATUS "  - test_functional_views (4 files)")
  message(STATUS "Run with: ctest -j4 -L functional")

else()
  message(WARNING "Qt6 QuickTest not found - functional UI tests disabled. Install Qt6 QuickTest module to enable.")
endif()

# ===== E2E Tests =====
# End-to-end tests with mock Twitch/HLS servers
add_subdirectory(e2e)
