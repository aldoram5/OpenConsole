# - Try to find VLC library
# Once done this will define
#
#  VLC_FOUND - system has VLC
#  VLC_INCLUDE_DIR - The VLC include directory
#  VLC_LIBRARIES - The libraries needed to use VLC
#  VLC_DEFINITIONS - Compiler switches required for using VLC
#
# Copyright (C) 2008, Tanguy Krotoff <tkrotoff@gmail.com>
# Copyright (C) 2008, Lukas Durfina <lukas.durfina@gmail.com>
# Copyright (c) 2009, Fathi Boudra <fboudra@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if(VLC_INCLUDE_DIR AND VLC_LIBRARIES)
   # in cache already
   set(VLC_FIND_QUIETLY TRUE)
endif(VLC_INCLUDE_DIR AND VLC_LIBRARIES)

# use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
if(NOT WIN32)
  find_package(PkgConfig)
  pkg_check_modules(VLC libvlc>=3.0.0)
  set(VLC_DEFINITIONS ${VLC_CFLAGS})
  if(VLC_LDFLAGS)
    set(VLC_LIBRARIES ${VLC_LDFLAGS})
  endif()
endif(NOT WIN32)

# macOS-specific: Check common locations if pkg-config didn't find VLC
if(APPLE AND NOT VLC_VERSION)
  # Check Homebrew locations
  # First find the vlc.h file with PATH_SUFFIXES vlc
  find_path(VLC_INCLUDE_DIR_INTERNAL
            NAMES vlc.h
            PATHS
              /usr/local/include
              /opt/homebrew/include
              /Applications/VLC.app/Contents/MacOS/include
            PATH_SUFFIXES vlc)

  # VLC_INCLUDE_DIR should be the parent (so #include <vlc/vlc.h> works)
  if(VLC_INCLUDE_DIR_INTERNAL)
    get_filename_component(VLC_INCLUDE_DIR "${VLC_INCLUDE_DIR_INTERNAL}" DIRECTORY)
  endif()

  find_library(VLC_LIBRARY_PATH
               NAMES vlc
               PATHS
                 /usr/local/lib
                 /opt/homebrew/lib
                 /Applications/VLC.app/Contents/MacOS/lib)

  if(VLC_INCLUDE_DIR AND VLC_LIBRARY_PATH)
    set(VLC_FOUND TRUE)
    set(VLC_LIBRARIES ${VLC_LIBRARY_PATH})
    # Assume version is OK if found via Homebrew (version check not available)
    set(VLC_VERSION_OK TRUE)
    message(STATUS "VLC library found via direct search (macOS)")
    message(STATUS "  Include: ${VLC_INCLUDE_DIR}")
    message(STATUS "  Library: ${VLC_LIBRARY_PATH}")
  else()
    set(VLC_FOUND FALSE)
    set(VLC_VERSION_OK FALSE)
    message(FATAL_ERROR "VLC library not found. On macOS, install with: brew install libvlc")
  endif()
else()
  # Non-macOS or pkg-config found VLC
  if(VLC_VERSION)
    # TODO add argument support to pass version on find_package
    include(MacroEnsureVersion)
    macro_ensure_version(3.0.0 ${VLC_VERSION} VLC_VERSION_OK)
    if(VLC_VERSION_OK)
      set(VLC_FOUND TRUE)
      message(STATUS "VLC library found via pkg-config (version ${VLC_VERSION})")
    else(VLC_VERSION_OK)
      set(VLC_FOUND FALSE)
      message(FATAL_ERROR "VLC library version ${VLC_VERSION} is too old (need >= 3.0.0)")
    endif(VLC_VERSION_OK)
  else()
    set(VLC_FOUND FALSE)
    message(FATAL_ERROR "VLC library not found via pkg-config")
  endif()

  find_path(VLC_INCLUDE_DIR
            NAMES vlc.h
            PATHS ${VLC_INCLUDE_DIRS}
            PATH_SUFFIXES vlc)

  find_library(VLC_LIBRARIES
               NAMES vlc
               PATHS ${VLC_LIBRARY_DIRS})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VLC DEFAULT_MSG VLC_INCLUDE_DIR VLC_LIBRARIES)

# show the VLC_INCLUDE_DIR and VLC_LIBRARIES variables only in the advanced view
mark_as_advanced(VLC_INCLUDE_DIR VLC_LIBRARIES)
