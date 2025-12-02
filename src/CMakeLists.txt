set(BLUEPLAYER_CORE_SOURCES
  core/Application.cpp
  core/Application.hpp
  api/twitch/TwitchApiClient.cpp
  api/twitch/TwitchApiClient.hpp
  api/twitch/TwitchAuthManager.cpp
  api/twitch/TwitchAuthManager.hpp
  api/twitch/TwitchService.cpp
  api/twitch/TwitchService.hpp
)

set(BLUEPLAYER_MEDIA_SOURCES
  media/FFmpegBridge.cpp
  media/FFmpegBridge.hpp
  media/FFmpegMediaService.cpp
  media/FFmpegMediaService.hpp
  media/FFmpegMediaSource.cpp
  media/FFmpegMediaSource.hpp
)

add_library(blueplayer_media STATIC ${BLUEPLAYER_MEDIA_SOURCES})
target_include_directories(blueplayer_media PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${FFmpeg_INCLUDE_DIRS}
)
target_link_libraries(blueplayer_media
  PUBLIC
    Qt6::Core
    Qt6::Multimedia
    ${FFmpeg_LIBRARIES}
)
target_link_directories(blueplayer_media PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})
setup_compiler_warnings(blueplayer_media)

add_library(blueplayer_core STATIC ${BLUEPLAYER_CORE_SOURCES})
target_include_directories(blueplayer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(blueplayer_core
  PUBLIC
    Qt6::Core
    blueplayer_media
    Qt6::Network
)
setup_compiler_warnings(blueplayer_core)

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
    Qt6::Widgets
    Qt6::Multimedia
    blueplayer_core
    ${FFmpeg_LIBRARIES}
)
target_link_directories(BluePlayer PRIVATE /opt/homebrew/lib ${FFMPEG_LIBS_LIBRARY_DIRS})

set_target_properties(BluePlayer PROPERTIES
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
)

setup_compiler_warnings(BluePlayer)

