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

# Chemins d'inclusion communs
set(TEST_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${FFmpeg_INCLUDE_DIRS}
)

# Bibliothèques communes pour les tests
set(TEST_LIBS
  Qt6::Core
  Qt6::Test
  Qt6::Network  # Pour QNetworkAccessManager, QSslConfiguration
  Qt6::Multimedia  # Pour QVideoSink
  ${FFmpeg_LIBRARIES}
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
add_test(NAME TwitchAuthManager COMMAND test_twitch_auth_manager)

# Tests unitaires pour FFmpegMediaService
add_executable(test_ffmpeg_media_service
  media/TestFFmpegMediaService.cpp
)
target_link_libraries(test_ffmpeg_media_service PRIVATE
  test_helpers
  ${TEST_LIBS}
  blueplayer_media  # Nécessaire pour FFmpegMediaService
)
target_include_directories(test_ffmpeg_media_service PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_ffmpeg_media_service PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
add_test(NAME FFmpegMediaService COMMAND test_ffmpeg_media_service)

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
add_test(NAME TwitchIntegration COMMAND test_twitch_integration)

# Tests d'intégration pour le pipeline média
add_executable(test_media_pipeline
  integration/TestMediaPipeline.cpp
)
target_link_libraries(test_media_pipeline PRIVATE
  test_helpers
  ${TEST_LIBS}
  blueplayer_media  # Nécessaire pour FFmpegMediaService
)
target_include_directories(test_media_pipeline PRIVATE ${TEST_INCLUDE_DIRS})
target_link_directories(test_media_pipeline PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
add_test(NAME MediaPipeline COMMAND test_media_pipeline)

include(CTest)
enable_testing()
