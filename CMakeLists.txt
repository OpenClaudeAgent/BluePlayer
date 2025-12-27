cmake_minimum_required(VERSION 3.24)
project(BluePlayer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Détection automatique du chemin Homebrew (portable entre architectures Mac)
if(APPLE)
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT HOMEBREW_PREFIX)
        # Fallback pour les Mac ARM et Intel
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
            set(HOMEBREW_PREFIX "/opt/homebrew")
        else()
            set(HOMEBREW_PREFIX "/usr/local")
        endif()
    endif()
    message(STATUS "Homebrew prefix: ${HOMEBREW_PREFIX}")
endif()

# Définir les chemins pour Vulkan
set(Vulkan_INCLUDE_DIR "${HOMEBREW_PREFIX}/include")
set(Vulkan_LIBRARY "${HOMEBREW_PREFIX}/lib/libvulkan.dylib")
set(Vulkan_FOUND TRUE)

# Activer AUTOMOC, AUTORCC, AUTOUIC pour une meilleure gestion des fichiers Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

# Paramètres de compilation globaux
if(CMAKE_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Supprimer les warnings de duplication de bibliothèques lors du linkage
# Ces warnings sont normaux pour les bibliothèques statiques avec dépendances circulaires
if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-w")
endif()

# Déclarer les options avant de les utiliser
option(BLUEPLAYER_ENABLE_TESTS "Build unit and integration tests" ON)
option(BLUEPLAYER_ENABLE_PROFILING "Enable profiling instrumentation" OFF)
option(BLUEPLAYER_ENABLE_STATIC_ANALYSIS "Enable static analysis (clang-tidy, cppcheck)" OFF)
option(BLUEPLAYER_ENABLE_COVERAGE "Enable code coverage with llvm-cov" OFF)
option(BLUEPLAYER_ENABLE_MUTATION_TESTING "Enable mutation testing with Mull" OFF)

# Configuration de la couverture de code avec llvm-cov
if(BLUEPLAYER_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(STATUS "Enabling code coverage with llvm-cov")
        # Flags pour la génération de profilage et de couverture
        # Ces flags seront appliqués uniquement aux bibliothèques et tests via target_compile_options
        set(BLUEPLAYER_COVERAGE_ENABLED ON CACHE INTERNAL "")
        set(BLUEPLAYER_COVERAGE_FLAGS "-fprofile-instr-generate;-fcoverage-mapping")
    else()
        message(WARNING "Code coverage is only supported with Clang compiler. Current compiler: ${CMAKE_CXX_COMPILER_ID}")
        set(BLUEPLAYER_ENABLE_COVERAGE OFF)
    endif()
endif()

# Inclure CompilerWarnings.cmake directement
include(cmake/CompilerWarnings.cmake OPTIONAL)

# Suppression de l'inclusion du chemin du module cmake pour éviter le FindFFmpeg.cmake personnalisé
# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Qt6 6.5 COMPONENTS Core Quick Gui Multimedia QuickControls2 Widgets WebSockets REQUIRED)

# QuickTest is optional (for functional UI tests)
find_package(Qt6 6.5 COMPONENTS QuickTest QUIET)
if(Qt6QuickTest_FOUND)
  message(STATUS "Qt6 QuickTest found - functional UI tests will be available")
endif()

# FFmpeg dependencies removed


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
