# Inspired by community FindFFmpeg modules. Provides imported targets:
#   FFmpeg::AVCODEC, FFmpeg::AVFORMAT, FFmpeg::AVUTIL, FFmpeg::SWSCALE, FFmpeg::SWRESAMPLE

include(FindPackageHandleStandardArgs)
include(CMakeFindDependencyMacro)

set(_FFMPEG_COMPONENTS ${FFmpeg_FIND_COMPONENTS})
if(_FFMPEG_COMPONENTS STREQUAL "")
  set(_FFMPEG_COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE SWRESAMPLE)
endif()

set(_FFMPEG_SEARCH_PATHS
  ${FFmpeg_ROOT}
  $ENV{FFmpeg_ROOT}
  $ENV{FFmpeg_DIR}
  /usr/local
  /opt/homebrew
  /opt/homebrew/lib # Ajouté pour les bibliothèques Homebrew
  /usr
)

find_package(PkgConfig QUIET)

foreach(_component IN LISTS _FFMPEG_COMPONENTS)
  string(TOLOWER "${_component}" _component_lower)

  if(PkgConfig_FOUND)
    pkg_check_modules("PC_FFMPEG_${_component}" QUIET "lib${_component_lower}")
  endif()

  find_library(FFmpeg_${_component}_LIBRARY
    NAMES ${_component_lower}
    HINTS
      ${PC_FFMPEG_${_component}_LIBDIR}
      ${PC_FFMPEG_${_component}_LIBRARY_DIRS}
      ${_FFMPEG_SEARCH_PATHS}
    PATH_SUFFIXES lib lib64
  )

  list(APPEND FFmpeg_LIBRARIES "${FFmpeg_${_component}_LIBRARY}")
endforeach()

find_path(FFmpeg_INCLUDE_DIR
  NAMES libavcodec/avcodec.h
  HINTS
    ${PC_FFMPEG_AVCODEC_INCLUDEDIR}
    ${PC_FFMPEG_AVCODEC_INCLUDE_DIRS}
    ${_FFMPEG_SEARCH_PATHS}
    /opt/homebrew/include # Ajouté pour les en-têtes Homebrew
  PATH_SUFFIXES include include/ffmpeg
)

find_package_handle_standard_args(
  FFmpeg
  REQUIRED_VARS FFmpeg_INCLUDE_DIR ${FFmpeg_LIBRARIES}
  HANDLE_COMPONENTS
)

if(NOT FFmpeg_FOUND)
  return()
endif()

set(FFmpeg_INCLUDE_DIRS ${FFmpeg_INCLUDE_DIR})

foreach(_component IN LISTS _FFMPEG_COMPONENTS)
  if(NOT TARGET FFmpeg::${_component})
    add_library(FFmpeg::${_component} UNKNOWN IMPORTED)
    set_target_properties(FFmpeg::${_component} PROPERTIES
      IMPORTED_LOCATION "${FFmpeg_${_component}_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${FFmpeg_INCLUDE_DIRS}"
    )
  endif()
endforeach()

