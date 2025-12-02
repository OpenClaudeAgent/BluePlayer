set(BLUEPLAYER_CORE_SOURCES
  core/Application.cpp
  core/Application.hpp
  core/Config.cpp
  core/Config.hpp
  core/Error.cpp
  core/Error.hpp
  core/Logger.cpp
  core/Logger.hpp
  core/NetworkCache.cpp
  core/NetworkCache.hpp
  core/SecureStorage.cpp
  core/SecureStorage.hpp
  core/InputValidator.cpp
  core/InputValidator.hpp
  api/twitch/TwitchApiClient.cpp
  api/twitch/TwitchApiClient.hpp
  api/twitch/TwitchAuthManager.cpp
  api/twitch/TwitchAuthManager.hpp
  api/twitch/TwitchService.cpp
  api/twitch/TwitchService.hpp
  ui/HomeViewModel.cpp
  ui/HomeViewModel.hpp
)

set(BLUEPLAYER_MEDIA_SOURCES
  media/FFmpegBridge.cpp
  media/FFmpegBridge.hpp
  media/FFmpegMediaService.cpp
  media/FFmpegMediaService.hpp
  media/FFmpegMediaSource.cpp
  media/FFmpegMediaSource.hpp
)

# blueplayer_core doit être créé avant blueplayer_media car blueplayer_media en dépend
# Mais blueplayer_core utilise aussi blueplayer_media (Application utilise FFmpegMediaService)
# On résout cela en liant blueplayer_media après sa création
add_library(blueplayer_core STATIC ${BLUEPLAYER_CORE_SOURCES})
target_include_directories(blueplayer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(blueplayer_core
  PUBLIC
    Qt6::Core
    Qt6::Quick
    Qt6::Network
    Qt6::Multimedia  # Nécessaire car Application.hpp inclut FFmpegMediaService.hpp
)
setup_compiler_warnings(blueplayer_core)

add_library(blueplayer_media STATIC ${BLUEPLAYER_MEDIA_SOURCES})
target_include_directories(blueplayer_media PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${FFmpeg_INCLUDE_DIRS}
)
target_link_libraries(blueplayer_media
  PUBLIC
    Qt6::Core
    Qt6::Multimedia
    avcodec
    avformat
    avutil
    swscale
    swresample
  PRIVATE
    blueplayer_core  # Nécessaire pour Logger, LogCategory, InputValidator
)
target_link_directories(blueplayer_media PRIVATE /opt/homebrew/lib)
setup_compiler_warnings(blueplayer_media)

# blueplayer_core utilise blueplayer_media (Application utilise FFmpegMediaService)
# On doit lier blueplayer_media après sa création pour éviter la dépendance circulaire
target_link_libraries(blueplayer_core PRIVATE blueplayer_media)

qt_add_executable(BluePlayer
  main.mm
)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_add_qml_module(BluePlayer
  URI BluePlayer
  VERSION 1.0
  QML_FILES
    ui/main.qml
    ui/HomeView.qml
    ui/LoginView.qml
    ui/PreferencesView.qml
    ui/VideoPlayerApple.qml
    ui/themes/AppleTheme.js
    ui/components/AppleCard.qml
    ui/components/AppleButton.qml
    ui/components/CategoriesScroller.qml
    ui/components/LocalPlaybackCard.qml
    ui/components/HorizontalRowSection.qml
    ui/components/StreamCard.qml
)

target_link_libraries(BluePlayer
  PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Widgets
    Qt6::Multimedia
    blueplayer_core
    blueplayer_media  # Nécessaire car blueplayer_core ne lie plus blueplayer_media
    avcodec
    avformat
    avutil
    swscale
    swresample
)
target_link_directories(BluePlayer PRIVATE /opt/homebrew/lib)

set_target_properties(BluePlayer PROPERTIES
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
)

setup_compiler_warnings(BluePlayer)

