
cmake_minimum_required(VERSION 3.0)

if (NOT CMAKE_VERSION VERSION_LESS 3.3)
  # Allow symbol visibility settings also on static libraries. CMake < 3.3
  # only sets the property on a shared library build.
  cmake_policy(SET CMP0063 NEW)

  # Support new IN_LIST if() operator.
  cmake_policy(SET CMP0057 NEW)
endif ()

include(CheckIncludeFile)
include(CMakeDependentOption)

# CMAKE_TOOLCHAIN_FILE must be set before `project' is called, which
# configures the base build environment and references the toolchain file
project(freetype C)

set(VERSION_MAJOR "2")
set(VERSION_MINOR "13")
set(VERSION_PATCH "2")

# Generate LIBRARY_VERSION and LIBRARY_SOVERSION.
set(LIBTOOL_REGEX "version_info='([0-9]+):([0-9]+):([0-9]+)'")
file(STRINGS "${PROJECT_SOURCE_DIR}/builds/unix/configure.raw"
  VERSION_INFO
  REGEX ${LIBTOOL_REGEX})
string(REGEX REPLACE
  ${LIBTOOL_REGEX} "\\1"
  LIBTOOL_CURRENT "${VERSION_INFO}")
string(REGEX REPLACE
  ${LIBTOOL_REGEX} "\\2"
  LIBTOOL_REVISION "${VERSION_INFO}")
string(REGEX REPLACE
  ${LIBTOOL_REGEX} "\\3"
  LIBTOOL_AGE "${VERSION_INFO}")

# Disallow in-source builds
if ("${CMAKE_BINARY_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  message(FATAL_ERROR
    "In-source builds are not permitted!  Make a separate folder for"
    " building, e.g.,\n"
    "  cmake -E make_directory build\n"
    "  cmake -E chdir build cmake ..\n"
    "Before that, remove the files created by this failed run with\n"
    "  cmake -E remove CMakeCache.txt\n"
    "  cmake -E remove_directory CMakeFiles")
endif ()


# Add local cmake modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/builds/cmake)


file(GLOB PUBLIC_HEADERS "include/ft2build.h" "include/freetype/*.h")
file(GLOB PUBLIC_CONFIG_HEADERS "include/freetype/config/*.h")
file(GLOB PRIVATE_HEADERS "include/freetype/internal/*.h")


set(BASE_SRCS
    # Required
    src/base/ftsystem.c
    src/base/ftinit.c
    src/base/ftdebug.c
    src/base/ftbase.c
    
    # Recommended
    src/base/ftbbox.c
    src/base/ftglyph.c
    
    # Optional
    src/base/ftbitmap.c
    src/base/ftmm.c
    src/truetype/truetype.c
    src/sfnt/sfnt.c 
    src/psnames/psnames.c
    
    src/smooth/smooth.c
)


if (NOT DISABLE_FORCE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif ()


add_library(freetype
  ${PUBLIC_HEADERS}
  ${PUBLIC_CONFIG_HEADERS}
  ${PRIVATE_HEADERS}
  ${BASE_SRCS}
)

set_target_properties(
  freetype PROPERTIES
    C_VISIBILITY_PRESET hidden)

target_compile_definitions(
  freetype PRIVATE FT2_BUILD_LIBRARY)
  
message(WARNING "FT2: lvgl root: ${LVGL_ROOT_DIR}")
  
target_compile_definitions(
    freetype PRIVATE FT_CONFIG_OPTIONS_H=<lvgl/src/libs/freetype/ftoption.h>)
    
target_compile_definitions(
    freetype PRIVATE FT_CONFIG_MODULES_H=<lvgl/src/libs/freetype/ftmodule.h>)

if (BUILD_SHARED_LIBS)
  set_target_properties(freetype PROPERTIES
    VERSION ${LIBRARY_VERSION}
    SOVERSION ${LIBRARY_SOVERSION})
endif ()

if (1)
# Pick up ftconfig.h and ftoption.h generated above, first.
target_include_directories(
  freetype
    PUBLIC
      $<INSTALL_INTERFACE:include/freetype2>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include

      # Make <ftconfig.h> available for builds/unix/ftsystem.c.
      ${CMAKE_CURRENT_BINARY_DIR}/include/freetype/config
)
endif ()


# 'freetype-interface' is an interface library, to be accessed with
# `EXPORT_NAME Freetype::Freetype`.  This is the target name provided by
# CMake's `FindFreetype.cmake`, so we provide it for compatibility.
add_library(freetype-interface INTERFACE)
set_target_properties(freetype-interface PROPERTIES
  EXPORT_NAME Freetype::Freetype
  INTERFACE_LINK_LIBRARIES freetype)