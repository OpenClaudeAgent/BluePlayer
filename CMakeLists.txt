cmake_minimum_required(VERSION 3.24)
project(BluePlayer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Définir les chemins pour Vulkan (utilise les variables d'environnement ou chemins système)
if(NOT DEFINED Vulkan_INCLUDE_DIR)
  # Essayer de trouver Vulkan via pkg-config ou chemins standards
  find_path(Vulkan_INCLUDE_DIR 
    NAMES vulkan/vulkan.h
    PATHS 
      ${CMAKE_SYSTEM_PREFIX_PATH}/include
      /usr/include
      /usr/local/include
      /opt/homebrew/include
      ENV VULKAN_SDK
    PATH_SUFFIXES Vulkan
  )
endif()

if(NOT DEFINED Vulkan_LIBRARY)
  find_library(Vulkan_LIBRARY
    NAMES vulkan
    PATHS
      ${CMAKE_SYSTEM_PREFIX_PATH}/lib
      /usr/lib
      /usr/local/lib
      /opt/homebrew/lib
      ENV VULKAN_SDK
    PATH_SUFFIXES lib
  )
endif()

if(Vulkan_INCLUDE_DIR AND Vulkan_LIBRARY)
  set(Vulkan_FOUND TRUE)
endif()

# Activer AUTOMOC, AUTORCC, AUTOUIC pour une meilleure gestion des fichiers Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

# Paramètres de compilation globaux
if(CMAKE_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

option(BLUEPLAYER_ENABLE_TESTS "Build unit and integration tests" ON)
option(BLUEPLAYER_ENABLE_PROFILING "Enable profiling instrumentation" OFF)

# Inclure CompilerWarnings.cmake directement
include(cmake/CompilerWarnings.cmake OPTIONAL)

# Suppression de l'inclusion du chemin du module cmake pour éviter le FindFFmpeg.cmake personnalisé
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt6 6.5 COMPONENTS Core Quick Gui Multimedia Widgets REQUIRED)

# Configuration FFmpeg - utilise les variables d'environnement ou chemins standards
if(DEFINED ENV{FFmpeg_DIR})
  set(FFmpeg_DIR $ENV{FFmpeg_DIR})
endif()

# Essayer de trouver FFmpeg via le module personnalisé
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FFmpeg QUIET)

if(NOT FFmpeg_FOUND)
  # Fallback: chercher manuellement dans les chemins standards
  find_path(FFmpeg_INCLUDE_DIRS
    NAMES libavcodec/avcodec.h libavformat/avformat.h
    PATHS
      ${CMAKE_SYSTEM_PREFIX_PATH}/include
      /usr/include
      /usr/local/include
      /opt/homebrew/include
      ENV FFMPEG_INCLUDE_DIR
  )
  
  if(FFmpeg_INCLUDE_DIRS)
    find_library(FFmpeg_avcodec_LIBRARY avcodec
      PATHS
        ${CMAKE_SYSTEM_PREFIX_PATH}/lib
        /usr/lib
        /usr/local/lib
        /opt/homebrew/lib
        ENV FFMPEG_LIB_DIR
    )
    find_library(FFmpeg_avformat_LIBRARY avformat PATHS ${CMAKE_SYSTEM_PREFIX_PATH}/lib /usr/lib /usr/local/lib /opt/homebrew/lib ENV FFMPEG_LIB_DIR)
    find_library(FFmpeg_avutil_LIBRARY avutil PATHS ${CMAKE_SYSTEM_PREFIX_PATH}/lib /usr/lib /usr/local/lib /opt/homebrew/lib ENV FFMPEG_LIB_DIR)
    find_library(FFmpeg_swscale_LIBRARY swscale PATHS ${CMAKE_SYSTEM_PREFIX_PATH}/lib /usr/lib /usr/local/lib /opt/homebrew/lib ENV FFMPEG_LIB_DIR)
    find_library(FFmpeg_swresample_LIBRARY swresample PATHS ${CMAKE_SYSTEM_PREFIX_PATH}/lib /usr/lib /usr/local/lib /opt/homebrew/lib ENV FFMPEG_LIB_DIR)
    
    # Vérifier que toutes les bibliothèques requises sont trouvées avant de les ajouter à la liste
    # Évite l'inclusion de bibliothèques *-NOTFOUND qui causeraient des erreurs de linkage
    if(FFmpeg_avcodec_LIBRARY AND FFmpeg_avformat_LIBRARY AND FFmpeg_avutil_LIBRARY AND FFmpeg_swscale_LIBRARY AND FFmpeg_swresample_LIBRARY)
      set(FFmpeg_LIBRARIES ${FFmpeg_avcodec_LIBRARY} ${FFmpeg_avformat_LIBRARY} ${FFmpeg_avutil_LIBRARY} ${FFmpeg_swscale_LIBRARY} ${FFmpeg_swresample_LIBRARY})
      set(FFmpeg_FOUND TRUE)
    else()
      # Réinitialiser FFmpeg_INCLUDE_DIRS si les bibliothèques ne sont pas toutes trouvées
      # pour éviter un état incohérent où les headers sont définis mais pas les bibliothèques
      unset(FFmpeg_INCLUDE_DIRS CACHE)
    endif()
  endif()
endif()

if(NOT FFmpeg_FOUND)
  message(WARNING "FFmpeg not found. Please set FFmpeg_DIR or install FFmpeg.")
endif()

add_subdirectory(src)

if(BLUEPLAYER_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Installation (optionnel, pour déployer l'application)
install(TARGETS BluePlayer
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION . # Pour les bundles macOS, installe dans le répertoire courant de l'installation
)

