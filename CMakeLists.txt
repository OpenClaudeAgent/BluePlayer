cmake_minimum_required(VERSION 3.24)
project(BluePlayer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Définir les chemins pour Vulkan via Homebrew
set(Vulkan_INCLUDE_DIR "/opt/homebrew/include")
set(Vulkan_LIBRARY "/opt/homebrew/lib/libvulkan.dylib") # Ou libvulkan.a si statique
set(Vulkan_FOUND TRUE)

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
option(BLUEPLAYER_ENABLE_STATIC_ANALYSIS "Enable static analysis (clang-tidy, cppcheck)" OFF)

# Inclure CompilerWarnings.cmake directement
include(cmake/CompilerWarnings.cmake OPTIONAL)

# Suppression de l'inclusion du chemin du module cmake pour éviter le FindFFmpeg.cmake personnalisé
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt6 6.5 COMPONENTS Core Quick Gui Multimedia QuickControls2 Widgets REQUIRED)

# Désactiver pkg_search_module et FindFFmpeg.cmake pour FFmpeg
# et lier les bibliothèques manuellement.
set(FFmpeg_LIBRARIES avcodec avformat avutil swscale swresample)
set(FFmpeg_INCLUDE_DIRS /opt/homebrew/include) # Chemin des en-têtes Homebrew

add_subdirectory(src)

if(BLUEPLAYER_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Analyse statique avec clang-tidy
if(BLUEPLAYER_ENABLE_STATIC_ANALYSIS)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-15 clang-tidy-16 clang-tidy-17)
  if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=bugprone-*,cert-*,cppcoreguidelines-*,performance-*,readability-*,modernize-*")
  else()
    message(WARNING "clang-tidy not found. Static analysis disabled.")
  endif()
  
  # cppcheck optionnel
  find_program(CPPCHECK_EXE NAMES cppcheck)
  if(CPPCHECK_EXE)
    message(STATUS "Found cppcheck: ${CPPCHECK_EXE}")
    # cppcheck peut être ajouté comme target personnalisé si nécessaire
  endif()
endif()

# Installation (optionnel, pour déployer l'application)
install(TARGETS BluePlayer
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION . # Pour les bundles macOS, installe dans le répertoire courant de l'installation
)
