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
add_library(test_helpers STATIC
  TestHelpers.cpp
)
target_link_libraries(test_helpers PRIVATE 
  ${TEST_LIBS}
  Qt6::Network  # Nécessaire pour QNetworkAccessManager dans TestHelpers
)
target_include_directories(test_helpers PUBLIC
  ${TEST_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
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

# Tests unitaires pour les modules network
add_executable(test_http_client
  core/network/TestHttpClient.cpp
)
target_link_libraries(test_http_client PRIVATE
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
  ${TEST_LIBS}
  blueplayer_core
)
target_include_directories(test_api_client_base PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_api_client_base PRIVATE /opt/homebrew/lib)
apply_coverage_flags(test_api_client_base)
add_test(NAME ApiClientBase COMMAND test_api_client_base)

# Tests unitaires pour les modules media


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
