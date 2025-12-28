set(BLUEPLAYER_CORE_SOURCES
  core/Application.cpp
  core/Application.hpp
  core/Config.cpp
  core/Config.hpp
  core/Error.cpp
  core/Error.hpp
  core/ErrorHandler.cpp
  core/ErrorHandler.hpp
  core/Logger.cpp
  core/Logger.hpp
  core/NetworkCache.cpp
  core/NetworkCache.hpp
  core/SecureStorage.cpp
  core/SecureStorage.hpp
  core/WatchHistory.cpp
  core/WatchHistory.hpp
  core/InputValidator.cpp
  core/InputValidator.hpp
  core/StateMachineLiveReplay.cpp
  core/StateMachineLiveReplay.hpp
  core/CacheManager.cpp
  core/CacheManager.hpp
  core/FileLogger.cpp
  core/FileLogger.hpp
  core/LanguageManager.cpp
  core/LanguageManager.hpp
  core/network/HttpClient.cpp
  core/network/HttpClient.hpp
  core/network/ApiClientBase.cpp
  core/network/ApiClientBase.hpp
  core/network/CurlHttpClient.cpp
  core/network/CurlHttpClient.hpp
  api/twitch/TwitchApiClient.cpp
  api/twitch/TwitchApiClient.hpp
  api/twitch/TwitchAuthManager.cpp
  api/twitch/TwitchAuthManager.hpp
  api/twitch/TwitchService.cpp
  api/twitch/TwitchService.hpp
  chat/TwitchChatClient.cpp
  chat/TwitchChatClient.hpp
  chat/ChatMessage.hpp
  ui/HomeViewModel.cpp
  ui/HomeViewModel.hpp
  ui/CacheManagerViewModel.cpp
  ui/CacheManagerViewModel.hpp
)

set(BLUEPLAYER_MEDIA_SOURCES

  media/HlsAdFilter.cpp
  media/HlsAdFilter.hpp

  media/MpvQuickItem.cpp
  media/MpvQuickItem.hpp

  media/PlaybackSpeedLogic.cpp
  media/PlaybackSpeedLogic.hpp
)

# blueplayer_core et blueplayer_media ont une dépendance circulaire résolue via forward declarations
# blueplayer_media dépend de blueplayer_core (Logger, InputValidator)
# blueplayer_core utilise blueplayer_media (Application utilise FFmpegMediaService via forward declarations)
add_library(blueplayer_core STATIC ${BLUEPLAYER_CORE_SOURCES})
target_include_directories(blueplayer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# Find libcurl
find_package(CURL REQUIRED)

target_link_libraries(blueplayer_core
  PUBLIC
    Qt6::Core
    Qt6::Quick
    Qt6::Network
    Qt6::WebSockets
    CURL::libcurl
    # Qt6::Multimedia retiré car Application utilise maintenant des forward declarations
)

# macOS Security framework for Keychain access
if(APPLE)
  target_link_libraries(blueplayer_core PUBLIC "-framework Security")
endif()
# Appliquer les flags de couverture si activés
if(BLUEPLAYER_COVERAGE_ENABLED)
    target_compile_options(blueplayer_core PRIVATE ${BLUEPLAYER_COVERAGE_FLAGS})
    target_link_options(blueplayer_core PRIVATE ${BLUEPLAYER_COVERAGE_FLAGS})
endif()
setup_compiler_warnings(blueplayer_core)

add_library(blueplayer_media STATIC ${BLUEPLAYER_MEDIA_SOURCES})
target_include_directories(blueplayer_media PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${HOMEBREW_PREFIX}/include  # For libmpv
)
target_link_libraries(blueplayer_media
  PUBLIC
    Qt6::Core
    Qt6::Quick
    Qt6::Multimedia
    Qt6::OpenGL
    mpv  # libmpv for video playback
  PRIVATE
    blueplayer_core  # Nécessaire pour Logger, LogCategory, InputValidator
)
target_link_directories(blueplayer_media PRIVATE ${HOMEBREW_PREFIX}/lib)
# Appliquer les flags de couverture si activés
if(BLUEPLAYER_COVERAGE_ENABLED)
    target_compile_options(blueplayer_media PRIVATE ${BLUEPLAYER_COVERAGE_FLAGS})
    target_link_options(blueplayer_media PRIVATE ${BLUEPLAYER_COVERAGE_FLAGS})
endif()
setup_compiler_warnings(blueplayer_media)

# blueplayer_core utilise blueplayer_media (Application utilise FFmpegMediaService)
# La dépendance circulaire est résolue via forward declarations dans Application.hpp
# On lie blueplayer_media après sa création pour permettre la résolution des symboles
target_link_libraries(blueplayer_core PRIVATE blueplayer_media)

# ============================================================================
# Internationalization (i18n)
# ============================================================================
set(BLUEPLAYER_TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/blueplayer_fr.ts
)

# Generate .qm files from .ts files
qt_add_translations(blueplayer_core
    TS_FILES ${BLUEPLAYER_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE BLUEPLAYER_QM_FILES
    LUPDATE_OPTIONS -no-obsolete
)

qt_add_executable(BluePlayer
  main.mm
)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

# Add .qm files as resources
qt_add_resources(BluePlayer "translations"
    PREFIX "/i18n"
    BASE ${CMAKE_CURRENT_BINARY_DIR}
    FILES ${BLUEPLAYER_QM_FILES}
)

qt_add_qml_module(BluePlayer
  URI BluePlayer
  VERSION 1.0
  QML_FILES
    ui/main.qml
    ui/HomeView.qml
    ui/LoginView.qml
    ui/PreferencesView.qml
    ui/PlayerView.qml
    ui/CacheManagerView.qml
    ui/VideoPlayer.qml
    ui/PipWindow.qml
    ui/themes/BlueTheme.js
    ui/components/BaseCard.qml
    ui/components/BlueCard.qml
    ui/components/BlueButton.qml
    ui/components/BlueDropdown.qml
    ui/components/CategoriesScroller.qml
    ui/components/CategoryCard.qml
    ui/components/ClipCard.qml
    ui/components/VideoCard.qml
    ui/components/ChannelCard.qml
    ui/components/LocalPlaybackCard.qml
    ui/components/HorizontalRowSection.qml
    ui/components/StreamCard.qml
    ui/components/PlayerControlBar.qml
    ui/components/SearchResults.qml
    ui/components/SearchResultItem.qml
    ui/components/ChatPanel.qml
    ui/components/ChatMessage.qml
    ui/components/TopBarOverlay.qml
    ui/components/LoadingOverlay.qml
    ui/components/ErrorToast.qml
    ui/components/ControlButton.qml
    ui/components/SeekBar.qml
    ui/components/VolumeControl.qml
    ui/components/QualitySelector.qml
    ui/components/QualityControl.qml
    ui/components/CircleButton.qml
    ui/components/PanelHeader.qml
    ui/components/PipPlaceholder.qml
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

)
target_link_directories(BluePlayer PRIVATE ${HOMEBREW_PREFIX}/lib)

set_target_properties(BluePlayer PROPERTIES
  MACOSX_BUNDLE TRUE
  WIN32_EXECUTABLE TRUE
)

# Code signing for macOS - permet au Keychain de reconnaître l'app entre les builds
if(APPLE)
  # Utilise le certificat de développement disponible
  set_target_properties(BluePlayer PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "SKFL9W89L2"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
  )
  
  # Pour les builds Ninja/Make (non-Xcode), signer manuellement après le build
  # Utilise le hash SHA-1 du certificat pour éviter les problèmes d'échappement
  if(NOT CMAKE_GENERATOR MATCHES "Xcode")
    add_custom_command(TARGET BluePlayer POST_BUILD
      COMMAND codesign --force --deep --sign E8F938C579690A07F405E486C84C573D745B43DD 
              "$<TARGET_BUNDLE_DIR:BluePlayer>"
      COMMENT "Signing BluePlayer.app with developer certificate..."
    )
  endif()
endif()

setup_compiler_warnings(BluePlayer)

